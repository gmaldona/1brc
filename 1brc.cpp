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
 *
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
#include <iostream>
#include <iomanip>
#include <array>

using namespace std;

/**
 * A MappedFile is a structure that holds metadata about an memory mapped file.
 * The true purpose of containerizing these values was to take advantage of
 * a deconstruct function to unmap memory.
 */
struct MappedFile {
   int fd;
   struct stat fileInfo;
   char* map;
   ~MappedFile();
};

MappedFile::~MappedFile() {
   if (munmap(map, fileInfo.st_size) == -1) {
      close(fd); exit(1);
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
             stat(path, &mappedFile->fileInfo) == -1
       ) {
      return nullptr;
   }
   mappedFile->map = static_cast<char*>(mmap(0, mappedFile->fileInfo.st_size,
                          PROT_READ, MAP_SHARED,
                          mappedFile->fd, 0));
   if (mappedFile->map == MAP_FAILED) {
      return nullptr;
   }
   return mappedFile;
}

void
sequential_computation(unique_ptr<MappedFile>&& mapped_file,
                       const shared_ptr<unordered_map<string, vector<float>>>& results) {
   unordered_map<string, vector<float>> mapped_values = unordered_map<string, vector<float>>{};

   // i := start , j := end
   int i = 0, j = 0;
   char* mem = mapped_file->map;
   char buffer_arr[100]; string buffer_str;
   while (true) {
      if (mem[j] == '\0') { break; }
      if (mem[j] == '\n') {
         memcpy(buffer_arr, mem + i, j);
         buffer_str = buffer_arr;
         if (buffer_str.empty()) {
            break;
         }
         string station; float temp;
         station = buffer_str.substr(0, buffer_str.find(';'));
         temp = stof(buffer_str.substr(
                    buffer_str.find(';') + 1,
                      buffer_str.size() - 1)
                    );
         if (mapped_values.find(station) == mapped_values.end()) {
            mapped_values[station].push_back(temp);
         } else {
            mapped_values[station] = vector<float> { temp };
         }
         j++;
         i = j;

      } else { j++; }
   }

   for (auto [k, v] : mapped_values) {
      std::sort(v.begin(), v.end());
      (*results)[k] = vector<float> { v[0], v[ceil(v.size() / 2)], v[v.size()] };
   }
}

ostream& operator<<(ostream& os, unordered_map<string, vector<float>> map) {
   // e.g. {Bosaso=-15.0/1.3/20.0, Petropavlovsk-Kamchatsky=-9.5/0.0/9.5}
   os << "{";
   for (auto [k, v] : map) {
      os << k << "=";
      for (int i = 0; i < v.size(); ++i) {
         os << v[i];
         if (i == v.size() - 1) { os << ", "; } else { os << "/"; }
      }
   }
   os << char(0x08) << char(0x08) << "}";
   return os;
}

int main(int args, char** argv) {
   string filepath;
   filepath = (args > 1) ? argv[0] : "/Users/gregorymaldonado/bing/src/c++/cs547/1BRC/samples/measurements-1.txt";

   auto mapped_file = map_file2mem(filepath.c_str());
   if (mapped_file == nullptr) {
      perror("File mapping to memory failed."); exit(0);
   }

   // {Bosaso=-15.0/1.3/20.0, Petropavlovsk-Kamchatsky=-9.5/0.0/9.5}
   auto results = make_shared<unordered_map<string, vector<float>>>();
   sequential_computation(std::move(mapped_file), results);
   cout << *results;
   return 0;
}
