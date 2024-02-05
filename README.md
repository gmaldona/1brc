# 1BRC @ Binghamton University (CS 547)
[1 Billion Row Contest](https://github.com/gunnarmorling/1brc) 

To do a straight run:

```bash
make && ./build/1brc <path_to_file>
```

To run on a SLURM job:

```bash
make && sbatch calculate_average.sh <path_to_file>
```

To benchmark:

```bash
make && time ./build/1brc <path_to_file>
```

## My testing files

* /home/gmaldonado/one-billion-row-challenge-gmaldona/1brc/data/measurements.txt
* /home/gmaldonado/t/one-billion-row-challenge-gmaldona/1brc/data/measurements.txt
* /home/gmaldonado/one-billion-row-challenge-gmaldona/1brc/src/test/resources/samples/measurements-10.txt