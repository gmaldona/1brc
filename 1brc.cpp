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

#include <string>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unordered_map>
#include <unistd.h>
#include <memory>
#include <vector>
#include <algorithm>
#include <cmath>
#include <fstream>
#include <iomanip>
#include <thread>
#include <execution>
#include <future>
#include <chrono>
#include <iostream>
#include <algorithm>
#include "1brc.hpp"

#define DEBUG true

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
unique_ptr<MappedFile> map_file2mem(const char *path) {
   auto mappedFile = make_unique<MappedFile>();

   // try to open the file and if fail return nullptr for main to handle
   if ((mappedFile->fd = open(path, O_RDONLY)) == -1 ||
         stat(path, &mappedFile->fileInfo) == -1
         ) {
      return nullptr;
   }
   mappedFile->map = static_cast<char *>(mmap(0, mappedFile->fileInfo.st_size,
                                              PROT_READ, MAP_SHARED,
                                              mappedFile->fd, 0));
   if (mappedFile->map == MAP_FAILED) {
      return nullptr;
   }
   return mappedFile;
}

shared_ptr<unordered_multimap<string, float>>
threaded_computation(const char *mem, const size_t &begin, const size_t &end) {
   auto mapped_values = make_shared<unordered_multimap<string, float>>();
   string buffer_str;
   char buffer_arr[100];
   long long i = begin;
   long long j = begin;

   while (j <= end) {

      if (mem[j] == '\n') {

         // GRR n is an offset. Spent so much time debugging ...
         std::memcpy(buffer_arr, mem + i, j - i);

         buffer_str = buffer_arr;
         if (buffer_str.empty() || buffer_str == "\n") {
            break;
         }
         string station;
         float temp;
         station = buffer_str.substr(0, buffer_str.find(';'));

         temp = stof(buffer_str.substr(
               buffer_str.find(';') + 1,
               buffer_str.size()));

         mapped_values->insert({station, temp});
         i = ++j;

      } else { j++; }
   }
   return mapped_values;
}

unordered_map<string, vector<float>> *
threaded_computation(const unique_ptr<MappedFile> &mapped_file,
                     unsigned int threads) {
   size_t chunk = floor(mapped_file->fileInfo.st_size / (long long)(threads * 1.25));
   vector<future<shared_ptr<unordered_multimap<string, float>>>> futures{};
   auto futureResults = unordered_multimap<string, float>{};
   unsigned long long begin = 0;
   unsigned long long end = chunk;
   char block[100];
   string block_str;
   for (size_t i = 1; i <= threads; ++i) {
      auto &mem = mapped_file->map;

      if (i == threads) {
         end = mapped_file->fileInfo.st_size;
      } else {
         if (mem[end] != '\n') {
            while (mem[end] != '\n') {
               end++;
            }
         }
      }

      memset(block, 0, 100);
      memcpy(block, mem + begin, end - begin);
      block_str = string(block);

      futures.push_back(std::async(std::launch::async, [&mapped_file, begin, end] {
         auto test = threaded_computation(mapped_file->map, begin, end);
         return test;
      }));
      // end is a \n, so middle means +1 is the next char or if end then \0
      begin = end + 1;
      if ((end + chunk) < mapped_file->fileInfo.st_size) {
         end = end + chunk;
      } else {
         end = mapped_file->fileInfo.st_size;
      }
   }
   for (auto &future : futures) {
      future.wait();
   }

   for (auto &future : futures) {
      auto result = future.get();
      futureResults.merge(*result);
   }

   auto results = new unordered_map<string, vector<float>>{};

//   for (auto &[k, v] : futureResults) {
//      std::sort(v.begin(), v.end());
//      (*results)[k] = vector<float>{v[0], v[floor(v.size() / 2)], v[v.size() - 1]};
//   }

   return results;
}

/**
 * There's a few bad things with this implementation.
 * 1. Increased memory footprint. The file is loaded and then you are added the
 * same data into maps.
 * Sorting on the data. This is fine for small amount of data but once 1 B, then
 * not really reliable.
 */
unordered_map<string, vector<float>> *
sequential_computation(const unique_ptr<MappedFile> &mapped_file) {
   return sequential_computation(mapped_file->map);
}

unordered_map<string, vector<float>> *
sequential_computation(char *mem) {
   unordered_map<string, vector<float>> mapped_values = unordered_map<string, vector<float>>{};
   auto *results = new unordered_map<string, vector<float>>{};
   // i := start , j := end
   int i = 0, j = 0;
   string buffer_str;
   char buffer_arr[100];

   while (true) {

      if (mem[j] == '\0') { break; }
      if (mem[j] == '\n') {

         // GRR n is an offset. Spent so much time debugging ...
         std::memcpy(buffer_arr, mem + i, j - i);

         buffer_str = buffer_arr;
         if (buffer_str.empty() || buffer_str == "\n") {
            break;
         }
         string station;
         float temp;
         station = buffer_str.substr(0, buffer_str.find(';'));
         temp = stof(buffer_str.substr(
               buffer_str.find(';') + 1,
               buffer_str.size()));
         if (mapped_values.find(station) == mapped_values.end()) {
            mapped_values[station].push_back(temp);
         } else {
            mapped_values[station] = vector<float>{temp};
         }
         i = ++j;

      } else { j++; }
   }

   for (auto [k, v] : mapped_values) {
      std::sort(v.begin(), v.end());
      (*results)[k] = vector<float>{v[0], v[ceil(v.size() / 2)], v[v.size() - 1]};
   }

   return results;
}

ostream &operator<<(ostream &os, const unordered_map<string, vector<float>> &map) {
   os << "{";
   for (auto [k, v] : map) {
      os << k << "=";
      for (size_t i = 0; i < v.size(); ++i) {
         os << v[i];
         if (i == v.size() - 1) { os << ", "; } else { os << "/"; }
      }
   }
   os << char(0x08) << char(0x08) << "}";
   return os;
}

int main(int args, char **argv) {

   string filepath;
   filepath = (args > 1) ? argv[0]
                         : "/Users/gregorymaldonado/bing/src/c++/cs547/1BRC/samples/measurements-10.txt";

   auto mapped_file = map_file2mem(filepath.c_str());
   if (mapped_file == nullptr) {
      perror("File mapping to memory failed.");
      exit(0);
   }

   unordered_map<string, vector<float>> *results = nullptr;

   /**
    * macro instead of a boolean variable to reduce the computation time.
    * If variable then initialization and control flow check will add to the time
    */
   #ifdef DEBUG
   auto start = chrono::high_resolution_clock::now();
//   results = sequential_computation(mapped_file);
   results = threaded_computation(mapped_file, thread::hardware_concurrency());
   auto end = chrono::high_resolution_clock::now();
   #else
   results = sequential_computation(mapped_file);
   #endif

   cout << *results;
   delete results;

   return 0;
}
