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

#include <sys/stat.h>

#include <future>
#include <memory>
#include <string>
#include <thread>
#include <unordered_map>
#include <utility>
#include <vector>

/**
 * A MappedFile is a structure that holds metadata about an memory mapped
 * file. The true purpose of containerizing these values was to take
 * advantage of a deconstruct function to unmap memory.
 */
struct MappedFile {
  int fd;
  struct stat fileInfo;
  char *map;
  ~MappedFile();
};

/**
 * Uses mmap to map a file to memory. This is the best way to do parallel
 * computation on a buffer due to the fact of divide and conquer on the
 * memory.
 */
MappedFile* map_file2mem(const char *path);

/**
 * Sequential Computation (First Attempt):
 *
 * Before parallellization, a sequential computation should be attempted.
 * This implementation is to show a simple solution on how to read and
 * compute on a dataset sequentially.
 *
 * Not the best solution because it reads the file, parses the file and does
 * the computation all sequentially. For small datasets, the sequential and
 * threaded computation share similar runtimes, sometimes even beating the
 * paralellized implemation due to the overhead of spinning up hardware
 * threads.
 */
//std::unordered_map<std::string, float[3]> *sequential_computation(char *mem);

/**
 * Parallelized Read, Sequential Computation (Second Attempt):
 *
 * This was a quick attempt at parallelizing the operations needed to read
 * the file. Using mmap, the memory is able to be divided amongst all of the
 * threads on the machine and reading and parsing the given block of memory.
 * A future holds the promised data and the main thread waits till the
 * future is finished on the parsing. The computation of finding the min,
 * med, and max is still sequential because it requires all the futures to
 * return their data and then can sum all of the values together. This is
 * still not as good as it __could__ be... Parallelizing the computation
 * would be most ideal.
 *
 */
std::unordered_map<std::string, std::vector<float>*> *OBRC_futures(
    MappedFile* mapped_file,
    unsigned int hw_threads = std::thread::hardware_concurrency() - 1);

/**
 * When a future is spawned, this is the function that is being performed
 * within the future.
 *
 * @param mem pointer to the mapped memory.
 * @param begin index to the starting position of the assigned chunk.
 * @param end index to the ending position of the assigned chunk.
 * @return pointer to the map of results of the assigned chunk.
 *
 * NOTE: Using a pointer because when returning, it returns a copy of the
 * values within the map. If a pointer is used, then no copy is needed and
 * significantly speeds up the performance. One dry run: ~ 3.0e6 ns down to
 * ~ 1.5e6 ns
 */
void OBRC_futureworker(
    char *mem, long long begin, long long end,
    std::unordered_map<std::string, std::vector<int>*> * mapped_values);

//unordered_map<string, vector<int>>*
void OBRC_worker(char* mem, long long begin, long long end,
    std::unordered_map<std::string, std::vector<int>*>* mapped_values);

// void OBRC_concurworker(
//     char *memmap, long long begin, long long end,
//     std::shared_ptr<concurrent_hash_map<std::string,
//     concurrent_vector<float>>>
//         res);

// std::shared_ptr<concurrent_hash_map<std::string,
// concurrent_vector<float>>> OBRC_concurmap(const struct OBRC_memmap
// &memmap,
//                unsigned int hw_threads =
//                std::thread::hardware_concurrency());

inline int StoI(const char *p);
