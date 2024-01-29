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
#include <thread>
#include <unordered_map>

/**
* A MappedFile is a structure that holds metadata about an memory mapped file.
* The true purpose of containerizing these values was to take advantage of
 * a deconstruct function to unmap memory.
 */
struct MappedFile {
  int fd;
  struct stat fileInfo;
  char *map;
  ~MappedFile();
};

/**
 * Uses mmap to map a file to memory. This is the best way to do parallel computation
 * on a buffer due to the fact of divide and conquer on the memory.
 */
std::unique_ptr<MappedFile> map_file2mem(const char *path);

/**
 * Sequential Computation (First Attempt):
 *
 * Before parallellization, a sequential computation should be attempted. This
 * implementation is to show a simple solution on how to read and compute on a
 * dataset sequentially.
 *
 * Not the best solution because it reads the file, parses the file and does the
 * computation all sequentially. For small datasets, the sequential and
 * threaded computation share similar runtimes, sometimes even beating the
 * paralellized implemation due to the overhead of spinning up hardware threads.
 */
std::unordered_map<std::string, float[3]>
sequential_computation(char *mem);

/**
 * Parallelized Read, Sequential Computation (Second Attempt):
 *
 * This was a quick attempt at parallelizing the operations needed to read the
 * file. Using mmap, the memory is able to be divided amongst all of the threads
 * on the machine and reading and parsing the given block of memory.
 * A future holds the promised data and the main thread waits till the future is
 * finished on the parsing.
 * The computation of finding the min, med, and max is still sequential because
 * it requires all the futures to return their data and then can sum all of the
 * values together. This is still not as good as it __could__ be...
 * Parallelizing the computation would be most ideal.
 *
 * CONSIDER: switching away from STL and using arrays where possible.
 * STL containers have an overhead of spinning up.
 *
 */
std::unordered_map<std::string, float[3]>
parallel_read_sequential_computation(const std::unique_ptr<MappedFile> &mapped_file,
									 unsigned int threads = std::thread::hardware_concurrency());

/**
 * When a future is spawned, this is the function that is being performed within
 * the future.
 *
 * @param mem pointer to the mapped memory.
 * @param begin index to the starting position of the assigned chunk.
 * @param end index to the ending position of the assigned chunk.
 * @return pointer to the map of results of the assigned chunk.
 *
 * NOTE: Using a pointer because when returning, it returns a copy of the values
 * within the map. If a pointer is used, then no copy is needed and significantly
 * speeds up the performance.
 * 		One dry run: ~ 3.0e6 ns down to ~ 1.5e6 ns
 */
std::unordered_map<std::string, std::vector<float>> *
parallel_read_sequential_computation(const char *mem, const size_t &begin, const size_t &end);

/**
 * Parallel Read, Parallel Computation (Third Attempt):
 *
 * @param mapped_file
 * @param threads
 * @return
 */
//std::unordered_map<std::string, float[3]> *
//parallel_read_computation(const std::unique_ptr<MappedFile> &mapped_file,
//						  unsigned int threads = std::thread::hardware_concurrency());

std::ostream &
operator<<(std::ostream &os, const std::unordered_map<std::string, std::vector<float>> &map);