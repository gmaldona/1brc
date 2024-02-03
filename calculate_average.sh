#!/bin/bash

stamp=$(date +"%y%m%dT%T")

#SBATCH -A gmaldonado
#SBATCH --job-name=gmaldonado-1brc
#SBATCH --output="gmaldonado-1brc-$stamp.out"
#SBATCH --time=24:00:00
#SBATCH --ntasks=1
#SBATCH --cpus-per-task=64
#SBATCH --mem-per-cpu=500

# module load gnu9/9.4.0
# srun make

module load intel/2023.0.0
srun $(pwd)/build/1brc