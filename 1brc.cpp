/*
 * 1 Billion Row Contest for Binghamton University CS 547.
 * see: https://github.com/gunnarmorling/1brc
 *
 * author: Gregory Maldonado
 * email : gmaldonado@cs.binghamton.edu
 * date  : 2024-01-25
 *
 * Graduate student @ Thomas J. Watson College of Engineering and Applied
 * Sciences, Binghamton University.
 *
 * Overview: The task for the 1BRC is to process a text file containing cities
 * and temperatures.
 * Goal: Grab all your (virtual) threads, reach out to SIMD, optimize your
 * GC, or pull any other trick, and create the fastest implementation for
 * solving this task!
 */

#include "1brc.hpp"

#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>

#include <algorithm>
#include <list>
#include <cmath>
#include <functional>
#include <iostream>
#include <thread>


//==================================================================== 80 =====

using namespace std;

MappedFile::~MappedFile() {
  if (munmap(map, fileInfo.st_size) == -1) {
    close(fd);
    exit(1);
  }
}

inline int StoI(const char *p) {
    int x = 0; int isNeg = 0;
    if (*p == '-') { isNeg = 1; ++p; }
    while (*p >= '0' && *p <= '9') {
        x = (x*10) + (*p - '0');
        ++p;
    }
    return (isNeg) ? -x : x;
}

/*
 * Map a file to memory to faster reads.
 *
 * @param path: string representation of a path to a file.
 * @returns: a pointer to the file contents.
 */
MappedFile* map_file2mem(const char* path) {
  auto mappedFile = new MappedFile();

  // try to open the file and if fail return nullptr for main to handle
  if ((mappedFile->fd = open(path, O_RDONLY)) == -1 ||
      stat(path, &mappedFile->fileInfo) == -1) {
    return nullptr;
  }
  mappedFile->map = (char*)mmap(0, mappedFile->fileInfo.st_size, PROT_READ,
                                MAP_SHARED, mappedFile->fd, 0);
  if (mappedFile->map == MAP_FAILED) {
    return nullptr;
  }
  return mappedFile;
}

//==================================================================== 80 =====

void OBRC_worker(char* mem,
                 long long begin,
                 long long end,
                 unordered_map<string, vector<int>*>* mapped_values) {
    long long i = begin;
    long long j = begin;

    while (j <= end) {
        if (mem[j] == '\n') {
            string buffer_str(mem + i, mem + j);

            unsigned long delimiter = buffer_str.find(';');
            string station = buffer_str.substr(0, delimiter);
            string temp_str = buffer_str.substr(delimiter + 1);
            unsigned long decimal = temp_str.find('.');

            if (decimal + 2 < temp_str.size()) {
                temp_str = temp_str.substr(0, decimal + 1);
            }

            temp_str.erase(remove(temp_str.begin(),
                           temp_str.end(), '.'),
                           temp_str.end()
                    );
            // Reduces by 25% without stoi
            int temp = StoI(temp_str.c_str());

            if (mapped_values->find(station) != mapped_values->end()) {
                auto& t = (*mapped_values)[station];
                if (temp < (*t)[0]) { (*t)[0] = temp; }
                (*t)[1] = (*t)[1] + temp;
                if (temp > (*t)[2]) { (*t)[2] = temp; }
                (*t)[3] = (*t)[3] + 1;
            }
            else {
                (*mapped_values).insert(
                        {station, new vector<int>{temp, temp, temp, 1}
                        }); // ----vvvvvvvvvvvvv
            } // extremely hacky:
              // [0] - min,
              // [1] - running total,
              // [2] - max,
              // [3] - size.
            j = i = j + 1;
        } else {
            j++;
        }
    }
}

//==================================================================== 80 =====

std::unordered_map<std::string, std::vector<float>*>* OBRC_futures(
    MappedFile* mapped_file, unsigned int hw_threads) {
  long long chunk = floor(mapped_file->fileInfo.st_size / hw_threads);
  vector<unordered_map<string, vector<int>*>*> futures{};
  vector<thread> threads{};
  auto futureResults = unordered_map<string, vector<int>>{};

  long long begin = 0;
  long long end = chunk;

    /*
     * Inserting a threaded result into the MEGA map.
     * This does NOT keep all the values. It does a loop through
     * and determines min, max, size, and running total on the
     * fly.
     * This was noted to be fast than doing a push_back()
     * on each value.
     */
    auto insertResults = [&futureResults]
            (unordered_map<string, vector<int>*>* futureResult) -> void {
        for (auto& [key, vec] : *futureResult) {
            if (futureResults.find(key) == futureResults.end()) {
                futureResults.insert({key, *vec});
            } else {
                auto& res_vec = futureResults[key];
                if ((*vec)[0] < res_vec[0]) { res_vec[0] = (*vec)[0]; }
                res_vec[1] = res_vec[1] + (*vec)[1];
                if ((*vec)[2] > res_vec[2]) {res_vec[2] = (*vec)[2]; }
                res_vec[3] = res_vec[3] + (*vec)[3];
            }
            delete vec;
        }
    };


    /**
     * The chunk algo.
     * Tries to divide each chunk evenly but if the chunk ends NOT on a '\n'
     * then it must find the closest one and use that. The next chunk will
     * start where the '\n'was found + 1.
     */
    for (size_t i = 1; i <= hw_threads; ++i) {
        // this will not run. I want to remove it but I won't.
        if (i == hw_threads + 1) {
            end = mapped_file->fileInfo.st_size - 1;
        } else {
            while (end < mapped_file->fileInfo.st_size) {
                if (mapped_file->map[end] == '\n') { break; }
                end++;
            }
            if (end == mapped_file->fileInfo.st_size - 1) {
                break;
          }
        }

//==================================================================== 80 =====


    // spawn the thread.
    auto* thread_map = new unordered_map<string, vector<int>*> {};
    futures.push_back(thread_map);
    threads.emplace_back(OBRC_worker, mapped_file->map, begin, end,
                               thread_map);

    if (mapped_file->map[end + 1] == '\0') {
        break;  // handles spawning extra threads.
    }
    begin = end + 1;  // we know that end is '\n'.

    end = ((end + chunk) < mapped_file->fileInfo.st_size)
            ? end + chunk
            : mapped_file->fileInfo.st_size - 1;
    }

    /* reserve the last hw_thread as the main thread and instead of
     * wasting cycles waiting, use the computation to calculate its
     * own chunk.
     */
    auto* result = new unordered_map<string, vector<int>*> {};
    OBRC_worker(mapped_file->map, begin,
                 mapped_file->fileInfo.st_size - 1,
                 result);
    insertResults(result);
    delete result;

    // join each thread and join into to MEGA results.
    int i = 0;
    for (auto& t : threads) {
        t.join();
        auto* r = futures[i];
        insertResults(r);
        delete r;
        i++;
    }

    // final computation on all the results.
    auto* results = new unordered_map<string, vector<float>*>{};
    for (auto& [key, vec] : futureResults) {
        float min = vec[0] / 10.0f;
        float avg = (((float)vec[1] ) / 10.0f) / (float)vec[3];
        float max = vec[2] / 10.0f;
        results->insert({key, new vector<float>{min, avg, max}});
    };
    return results;
}

//==================================================================== 80 =====

int main(int args, char** argv) {
  string filepath;

  if (args > 1) {
      filepath = argv[1];
  } else {
      perror("No file to load.\n");
      exit(0);
  }

  auto* mapped_file = map_file2mem(filepath.c_str());
  if (mapped_file == nullptr) {
      perror("File mapping to memory failed.");
      exit(1);
  }

  unordered_map<string, vector<float>*>* results;
  results = OBRC_futures(mapped_file);

  printf("{");
  for (auto& [k, v] : *results) {
      printf("%s=%.1f/%.1f/%.1f, ", k.c_str(), (*v)[0], (*v)[1], (*v)[2]);
      delete v;
  }
  printf("%c%c}", char(0x08), char(0x08));

  delete results;
  delete mapped_file;

  return 0;
}
//==================================================================== 80 =====
