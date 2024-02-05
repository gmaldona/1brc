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

//==================================================================== 80 =====

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
// std::unordered_map<std::string, float[3]>
// *sequential_computation(char *mem);

/**
 * Threads (Second Attempt):
 *
 * This implementation calculates "chunks" (an range within the memmap that
 * will pass down to the thread to operate on) and spawn a thread to perform
 * the read. The amount of threads spawned is
 *      0 < x < std::thread::hardware_concurrency() - 1
 * The main thread will join() each thread and then combine all results into
 * a single unordered map to be do the final computations.
 * Note: pointers were used a bunch in this implementation. After MANY
 * iterations, it was noted that the speed increased dramatically after
 * using pointers. This is probably due to the fact, in previous
 * implementations, it was doing copies all over the place, where as pointers
 * do not need to do the copy.
 *
 */
std::unordered_map<std::string, std::vector<float>*> *OBRC_futures(
    MappedFile* mapped_file,
    unsigned int hw_threads = std::thread::hardware_concurrency() - 1);

//==================================================================== 80 =====

/**
 * This implementation is DEPRECATED.
 * This implementation was an attempt to use futures and then wait for
 * the future to return its value. The superseding implementation is
 * # OBRC_worker(char* mem, long long begin, long long end,
    std::unordered_map<std::string, std::vector<int>*>* mapped_values);
 * in which a pointer to a map is given as a parameter and the thread
 * operates on the parameter. This is almost like the implementation
 * with futures by passing in a promise but the overhead for the promise
 * was too costly so DEPRECATED :).
 */
void OBRC_futureworker(
    char *mem, long long begin, long long end,
    std::unordered_map<std::string, std::vector<int>*> * mapped_values);

/**
 * When a future is spawned, this is the function that is being performed
 * within the future.
 * The parsing is done using fixed point numbers and regexs. When the thread
 * is spawned, it is GUARENTEED that the chunk passed down will start at a
 * station name and end with a \n. This is to ensure splitting is done
 * evenly and correctly.
 *
 * @param mem pointer to the mapped memory.
 * @param begin index to the starting position of the assigned chunk.
 * @param end index to the ending position of the assigned chunk.
 * @param mapped_values the resulting computation of the worker.
 *
 * NOTE: Using a pointer because when returning, it returns a copy of the
 * values within the map. If a pointer is used, then no copy is needed and
 * significantly speeds up the performance. One dry run: ~ 3.0e6 ns down to
 * ~ 1.5e6 ns
 */
void OBRC_worker(char* mem, long long begin, long long end,
    std::unordered_map<std::string, std::vector<int>*>* mapped_values);

// ================================================ RIP TBB Implementaion =====
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

//==================================================================== 80 =====
