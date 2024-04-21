# 1BRC @ Binghamton University (CS 547)
[1 Billion Row Contest](https://github.com/gunnarmorling/1brc) 

## Results

<img width="488" alt="Screenshot 2024-04-21 at 5 29 12 PM" src="https://github.com/bu-cs447-2024-1s/one-billion-row-challenge-gmaldona/assets/60359847/088dccb5-a0e1-4f49-8190-301d4d97f44a">

## Submission

* last commit into repo: [7099f37](https://github.com/bu-cs447-2024-1s/one-billion-row-challenge-gmaldona/commit/7099f37235c321b2ffc154dce7ddeb89b0fa460a)
   * 10:57 PM 2023-02-04

* DEADLINE: 11:59 PM 2023-02-04

### How to submit
* https://piazza.com/class/lrkxtzhdnc831h/post/42 <br>
```
> the instructors' answer:
Just push it to github.
```


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
