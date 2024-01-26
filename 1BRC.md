# 1BRC @ Binghamton University (CS 547)
[1 Billion Row Contest](https://github.com/gunnarmorling/1brc)
for Binghamton University CS 547.

## What is the fastest way possible to read a file in C++?
* `std::ifstream`: Read line by line. 
* `mmap`: map or unmap files or devices into memory ([man7.org](https://man7.org/linux/man-pages/man2/mmap.2.html))
  * References:
    * https://eric-lo.gitbook.io/memory-mapped-io/shared-memory

## Is parallelization even needed? And if it is needed, what is the difference in performance? 
The first question you should always ask before pre-mature parallelization,
is, can I perform the same task sequentially? There are cases where 
sequential computing is more than enough and pre-mature parallelization can 
cost you performance. Hardware threads are expensive, so know when/where to 
use them. 

## Sequential Computation

## Parallelization Computation