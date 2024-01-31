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
#include <chrono>
#include <cmath>
#include <cstring>
#include <functional>
#include <future>
#include <iomanip>
#include <iostream>
#include <numeric>

#define DEBUG false

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

void parallel_computation(
    char* mem, long long begin, long long end,
    promise<unordered_map<string, vector<double>>*> prom) {
  auto* mapped_values = new unordered_map<string, vector<double>>();

  char buffer_arr[100];
  string buffer_str;
  unsigned long long i = begin;
  unsigned long long j = begin;

  while (j <= end) {
    if (mem[j] == '\n') {
      memset(buffer_arr, 0, 100);
      std::memcpy(buffer_arr, mem + i, j - i);
      buffer_str = buffer_arr;

      string station = buffer_str.substr(0, buffer_str.find(';'));
      double temp =
          stof(buffer_str.substr(buffer_str.find(';') + 1, buffer_str.size()));

      if (mapped_values->find(station) == mapped_values->end()) {
        (*mapped_values)[station].push_back(temp);
      } else {
        (*mapped_values)[station] = vector<double>{temp};
      }

      j = i = j + 1;
    } else {
      j++;
    }
  }
  prom.set_value(mapped_values);
}

unordered_map<string, vector<double>>* parallel_read_computation(
    const unique_ptr<MappedFile>& mapped_file, unsigned int threads) {
  // TODO: main thread is just waiting around, give that thread a task as well.
  long long chunk = floor(mapped_file->fileInfo.st_size / threads);
  vector<future<unordered_map<string, vector<double>>*>> futures{};
  vector<thread> threadArr{};
  auto futureResults = unordered_map<string, vector<double>>{};

  long long begin = 0;
  long long end = chunk;

  for (size_t i = 1; i <= threads; ++i) {
    if (i == threads) {
      end = mapped_file->fileInfo.st_size - 1;
    } else {
      while (end < mapped_file->fileInfo.st_size) {
        if (mapped_file->map[end] == '\n') {
          break;
        }
        end++;
      }
    }

    promise<unordered_map<string, vector<double>>*> prom;

    futures.push_back(prom.get_future());
    threadArr.push_back(thread(parallel_computation, mapped_file->map, begin,
                               end, std::move(prom)));

    if (mapped_file->map[end + 1] == '\0') {
      break;  // handles spawning extra threads.
    }
    begin = end + 1;  // we know from like 123, that end is '\n'.

    end = ((end + chunk) < mapped_file->fileInfo.st_size)
              ? end + chunk
              : mapped_file->fileInfo.st_size - 1;

    // promises.push_back(prom);
  }

  for (int i = 0; i < threads; ++i) {
    // TODO: this could probably be a concurrent hash map
    auto* futureResult = futures[i].get();
    threadArr[i].join();

    for (auto& [key, vec] : *futureResult) {
      if (futureResults.find(key) == futureResults.end()) {
        futureResults.insert({key, vec});
      } else {
        auto& res_vec = futureResults[key];
        res_vec.insert(res_vec.end(), vec.begin(), vec.end());
      }
    }
    // delete futureResult;
  }

  auto* results = new unordered_map<string, vector<double>>{};
  for (auto& [key, vec] : futureResults) {
    double min = (double)*min_element(vec.begin(), vec.end());
    double avg =
        (double)accumulate(vec.begin(), vec.end(), 0) / (double)vec.size();
    double max = (double)*max_element(vec.begin(), vec.end());
    vector<double> stats{min, avg, max};
    results->insert({key, stats});
  };

  return results;
}

int main(void) {
  string filepath = "measurements.txt";
  // if (args > 1) {
  //    filepath = argv[1];
  // } else {
  //    perror("No file to load.\n");
  //    exit(0);
  // }

  auto mapped_file = map_file2mem(filepath.c_str());
  if (mapped_file == nullptr) {
    perror("File mapping to memory failed.");
    exit(1);
  }

  unordered_map<string, vector<double>>* results;

/**
 * macro instead of a boolean variable to reduce the computation time.
 * If variable then initialization and control flow check will add to the time
 */
#ifdef DEBUG
  auto start = std::chrono::high_resolution_clock::now();
  results =
      parallel_read_computation(mapped_file, thread::hardware_concurrency());
  // results = threaded_computation(mapped_file,
  // thread::hardware_concurrency()); results =
  // sequential_computation(mapped_file->map);
  auto end = chrono::high_resolution_clock::now();
  auto elapsed_time = end - start;
  cout << chrono::duration<double, milli>(elapsed_time).count() << " ms"
       << '\n';
#else
  results = sequential_computation(mapped_file);
#endif

  printf("{");
  for (auto& [k, v] : *results) {
    printf("%s=%.2f/%.2f/%.2f, ", k.c_str(), v[0], v[1], v[2]);
  }
  printf("%c%c}", char(0x08), char(0x08));

  // delete results;

  return 0;
}
