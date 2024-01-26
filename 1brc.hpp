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
 */

#include <memory>
#include <string>
#include <vector>
#include <unordered_map>

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

std::unique_ptr<MappedFile> map_file2mem(const char* path);

std::unordered_map<std::string, std::vector<float>>*
sequential_computation(char* mem);

std::ostream&
operator<<(std::ostream& os,const std::unordered_map<std::string, std::vector<float>>& map);