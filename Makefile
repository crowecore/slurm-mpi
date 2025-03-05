# Compiler and flags
CC = mpicc
CFLAGS = -Wall -O2

# Default target
all: matrix_mult

# Rule for building matrix_mult
matrix_mult: matrix_mult.c
	$(CC) $(CFLAGS) -o $@ $<

# Clean built programs
clean:
	rm -f matrix_mult *.o

# Run the matrix multiplication locally (for testing)
run: matrix_mult
	mpirun -n 4 ./matrix_mult

# Submit the matrix multiplication job to Slurm
submit: matrix_mult matrix_job.sh
	sbatch matrix_job.sh

.PHONY: all clean run submit