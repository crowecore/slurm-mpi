# Matrix Multiplication with OpenMPI and Slurm

This repository contains a demonstrator application for distributed matrix multiplication using OpenMPI and Slurm on Ubuntu.

## Prerequisites

- Ubuntu Linux (Jammy or newer)
- C compiler (gcc/g++)
- Make build system

## Installation

Install the required dependencies:

```bash
sudo apt update
sudo apt install -y build-essential openmpi-bin openmpi-common libopenmpi-dev slurm-wlm
```

## Files

- `matrix_mult.c` - MPI-based matrix multiplication program
- `matrix_job.sh` - Slurm job submission script
- `Makefile` - Build automation script
- `slurm.conf` - Slurm configuration (see run instructions below)

## Building

To compile the application:

```bash
make
```

## Usage

### Running Locally with MPI

Test the application locally using MPI (without Slurm):

```bash
make run
```

This will run 4 MPI processes on your local machine.

### Running with Slurm

configure slurm:
```bash
# find how many nodes you can support in your local cluster configuration
# adjust matrix_job.sh as needed
#SBATCH --ntasks-per-node=4 or 2 tasks x 2 cpus... on my machine I have to do 4:1
#SBATCH --cpus-per-task=1

bashCopysinfo -p debug

# copy slurm configuration
sudo mkdir -p /etc/slurm
sudo cp ./slurm.conf /etc/slurm/

# make required directories
sudo mkdir -p /var/run/slurm-llnl
sudo mkdir -p /var/spool/slurm
sudo mkdir -p /var/spool/slurmd

# create Slurm user (if one doesn't already exist)
sudo adduser --system --group slurm
sudo chown slurm:slurm /var/run/slurm-llnl
sudo chown slurm:slurm /var/spool/slurm
sudo chown slurm:slurm /var/spool/slurmd

# start Slurm daemons
sudo systemctl start slurmd
sudo systemctl start slurmctld

# check Slurm status
sudo systemctl status slurmd
sudo systemctl status slurmctld
sinfo

```

Submit the job to Slurm, and check job status:

```bash
# runs matrix_mult it with slurm
make submit

# check the job was started, and find the job id
squeue

# show job status (my job id is 1, yours might be different)
scontrol show job 1

# check program output
cat matrix_mult_<your job id>.out

# if no *.out file your job might be pending or blocked because of a bad config. Look at job details for clues about the output file location
scontrol show job 1

```

This submits a job that runs on 2 nodes with 2 tasks per node (note! my machine only supports a 1:4 config - matrix_job.sh).
If you want to submit the job again with different parameters, you can modify the matrix_job.sh file to change:

- number of nodes
- number of tasks per node
- time limits
- other Slurm parameters

Error recovery... if you configure improperly you might end up having to cancel jobs (like I did). If you see something like this after running squeue (PartitionNodeLimit)... you will need to cancel it
```bash
$ squeue
JOBID PARTITION     NAME     USER ST       TIME  NODES NODELIST(REASON)
    1     debug matrix_m    crowe PD       0:00      2 (PartitionNodeLimit)
```
To cancel borked jobs, run...
```bash
scancel 1
```

### Cleaning Up

Remove compiled binaries:

```bash
make clean
```

## Customization

- Adjust matrix size by modifying the `N` constant in `matrix_mult.c`
- Modify node and process count in `matrix_job.sh`
- Change node features or resource requirements in the `#SBATCH` directives

## Notes

- This demonstrator runs in a single-node "cluster" mode when Slurm is installed on a standalone machine
- For true distributed performance, configure Slurm with multiple compute nodes
- The application prints detailed process status to help visualize the distributed execution