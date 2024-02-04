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
#include <future>
#include <iostream>
#include <numeric>
#include <thread>

using namespace std;

MappedFile::~MappedFile() {
  if (munmap(map, fileInfo.st_size) == -1) {
    close(fd);
    exit(1);
  }
}

/*
 * Map a file to memory to faster reads.
 *
 * @param path: string representation of a path to a file.
 * @returns: a pointer to the file contents.
 */
unique_ptr<MappedFile> map_file2mem(const char* path) {
  auto mappedFile = make_unique<MappedFile>();

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


unordered_map<string, vector<int>>* OBRC_worker(
        char* mem, long long begin, long long end) {
    auto* mapped_values = new unordered_map<string, vector<int>>();
    long long i = begin;
    long long j = begin;
    while (j <= end) {
        if (mem[j] == '\n') {
            string buffer_str(mem + i, mem + j);

            unsigned long delimiter = buffer_str.find(';');
            string station = buffer_str.substr(0, delimiter);
            string temp_str = buffer_str.substr(delimiter + 1);

            temp_str.erase(remove(temp_str.begin(), temp_str.end(), '.'), temp_str.end());
            int temp = 2;   // = stoi(temp_str); // Reduces by 25% without stoi

            if (mapped_values->find(station) != mapped_values->end()) {
                vector<int> vec {temp};
                vec.reserve(10'000);
                (*mapped_values)[station] = vec;
            }
            else {
                (*mapped_values)[station].push_back(temp);
            }
            j = i = j + 1;
        } else {
            j++;
        }
    }

    return mapped_values;
}


void OBRC_futureworker(
    char* mem, long long begin, long long end,
    std::promise<std::unordered_map<std::string, std::vector<int>>*> prom) {
  prom.set_value(OBRC_worker(mem, begin, end));
}

std::unordered_map<std::string, std::vector<float>>* OBRC_futures(
    const std::unique_ptr<MappedFile>& mapped_file, unsigned int hw_threads) {
  long long chunk = floor(mapped_file->fileInfo.st_size / hw_threads);
  vector<future<unordered_map<string, vector<int>>*>> futures{};
  vector<thread> threads{};
  auto futureResults = unordered_map<string, vector<int>>{};

  long long begin = 0;
  long long end = chunk;

    auto insertResults = [&futureResults](unordered_map<string, vector<int>>* futureResult) -> void {
        for (auto& [key, vec] : *futureResult) {
            if (futureResults.find(key) == futureResults.end()) {
                futureResults.insert({key, vec});
            } else {
                auto& res_vec = futureResults[key];
                res_vec.insert(res_vec.end(), vec.begin(), vec.end());
            }
        }
    };

  for (size_t i = 1; i <= hw_threads; ++i) {
      if (i == hw_threads + 1) {
          end = mapped_file->fileInfo.st_size - 1;
      } else {
          while (end < mapped_file->fileInfo.st_size) {
              if (mapped_file->map[end] == '\n') {
                  break;
              }
              end++;
          }
          if (end == mapped_file->fileInfo.st_size - 1) {
              break;
          }
      }

      promise<unordered_map<string, vector<int>> *> prom;
      futures.push_back(prom.get_future());
      threads.push_back(thread(OBRC_futureworker, mapped_file->map, begin, end,
                               std::move(prom)));

      // sequential
//      auto result = OBRC_worker(mapped_file->map, begin, end);
//      insertResults(result);
//      delete result;

      if (mapped_file->map[end + 1] == '\0') {
          break;  // handles spawning extra threads.
      }
      begin = end + 1;  // we know that end is '\n'.

      end = ((end + chunk) < mapped_file->fileInfo.st_size)
            ? end + chunk
            : mapped_file->fileInfo.st_size - 1;
  }

    auto result = OBRC_worker(mapped_file->map, begin, mapped_file->fileInfo.st_size - 1);
    insertResults(result);
    delete result;

    int i = 0;
    for (auto& t : threads) {
        t.join();
        auto* futureResult = futures[i].get();
        insertResults(futureResult);
        delete futureResult;
        i++;
    }

  auto* results = new unordered_map<string, vector<float>>{};
  for (auto& [key, vec] : futureResults) {
    float min = *min_element(vec.begin(), vec.end()) / 10.0f;
    float avg = (accumulate(vec.begin(), vec.end(), 0.0)  / 10.0f ) / (double)vec.size();
    float max = *max_element(vec.begin(), vec.end()) / 10.0f;
    vector<float> stats{min, avg, max};
    results->insert({key, stats});
  };

  return results;
}

int main(int args, char** argv) {
  string filepath;
   // "/home/gmaldonado/one-billion-row-challenge-gmaldona/1brc/data/measurements.txt";
   // "/home/gmaldonado/t/one-billion-row-challenge-gmaldona/1brc/data/measurements.txt";

   if (args > 1) {
      filepath = argv[1];
   } else {
      perror("No file to load.\n");
      exit(0);
   }

  auto mapped_file = map_file2mem(filepath.c_str());
  if (mapped_file == nullptr) {
    perror("File mapping to memory failed.");
    exit(1);
  }

  unordered_map<string, vector<float>>* results;
  results = OBRC_futures(mapped_file);

  printf("{");
  for (auto& [k, v] : *results) {
    printf("%s=%.1f/%.1f/%.1f, ", k.c_str(), v[0], v[1], v[2]);
  }
  printf("%c%c}", char(0x08), char(0x08));

  delete results;

  return 0;
}
