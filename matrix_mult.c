#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>  // For hostname

#define N 2000  // Matrix dimension

void initialize_matrix(int *matrix, int rows, int cols) {
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            matrix[i * cols + j] = rand() % 10;
        }
    }
}

void print_matrix(int *matrix, int rows, int cols) {
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            printf("%d ", matrix[i * cols + j]);
        }
        printf("\n");
    }
}

int main(int argc, char** argv) {
    MPI_Init(&argc, &argv);
    
    int world_rank, world_size;
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);
    
    // need hostname to identify which node the process is running on
    char hostname[256];
    gethostname(hostname, sizeof(hostname));
    
    // print initial status
    printf("[Node: %s, Rank: %d/%d] Process initialized and ready\n", 
           hostname, world_rank, world_size);
    fflush(stdout);  // display output immediately
    
    srand(time(NULL) + world_rank);
    
    int rows_per_process = N / world_size;
    
    // matrices
    // matrix A is distributed (scattered) across processes by rows
    // matrix B is replicated (broadcast) to every process in its entirety
    // each process calculates its portion of C
    // results are gathered to assemble the complete C matrix

    int *A = NULL, *B = NULL, *C = NULL;
    int *local_A = (int*)malloc(rows_per_process * N * sizeof(int));
    int *local_C = (int*)malloc(rows_per_process * N * sizeof(int));
    
    // status update before matrix initialization
    printf("[Node: %s, Rank: %d/%d] Allocated memory for local matrices\n", 
           hostname, world_rank, world_size);
    fflush(stdout);
    
    // rank 0 initializes matrices
    if (world_rank == 0) {
        printf("[Node: %s, Rank: %d/%d] Initializing matrices of size %dx%d\n", 
               hostname, world_rank, world_size, N, N);
        fflush(stdout);
        
        A = (int*)malloc(N * N * sizeof(int));
        B = (int*)malloc(N * N * sizeof(int));
        C = (int*)malloc(N * N * sizeof(int));
        
        initialize_matrix(A, N, N);
        initialize_matrix(B, N, N);
        
        printf("[Node: %s, Rank: %d/%d] Matrices initialized, distributing data to other processes\n", 
               hostname, world_rank, world_size);
        fflush(stdout);
    }
    
    // synchronize all processes before mpi distribution
    MPI_Barrier(MPI_COMM_WORLD);
    
    // distribute matrix A
    printf("[Node: %s, Rank: %d/%d] Waiting to receive portion of matrix A\n", 
           hostname, world_rank, world_size);
    fflush(stdout);
    
    MPI_Scatter(A, rows_per_process * N, MPI_INT, 
                local_A, rows_per_process * N, MPI_INT, 
                0, MPI_COMM_WORLD);
    
    printf("[Node: %s, Rank: %d/%d] Received %d rows of matrix A\n", 
           hostname, world_rank, world_size, rows_per_process);
    fflush(stdout);
    
    // broadcast matrix B to all processes
    if (world_rank != 0) {
        B = (int*)malloc(N * N * sizeof(int));
    }
    
    printf("[Node: %s, Rank: %d/%d] Waiting to receive matrix B\n", 
           hostname, world_rank, world_size);
    fflush(stdout);
    
    MPI_Bcast(B, N * N, MPI_INT, 0, MPI_COMM_WORLD);
    
    printf("[Node: %s, Rank: %d/%d] Received complete matrix B\n", 
           hostname, world_rank, world_size);
    fflush(stdout);
    
    // mark the start time
    double start_time = 0;
    if (world_rank == 0) {
        start_time = MPI_Wtime();
    }
    
    // do matrix multiplication
    printf("[Node: %s, Rank: %d/%d] Starting matrix multiplication\n", 
           hostname, world_rank, world_size);
    fflush(stdout);
    
    // calculate chunks and print progress updates
    int chunk_size = rows_per_process / 10 > 0 ? rows_per_process / 10 : 1;
    
    for (int chunk = 0; chunk < rows_per_process; chunk += chunk_size) {
        int end_row = chunk + chunk_size;
        if (end_row > rows_per_process) end_row = rows_per_process;
        
        for (int i = chunk; i < end_row; i++) {
            for (int j = 0; j < N; j++) {
                local_C[i * N + j] = 0;
                for (int k = 0; k < N; k++) {
                    local_C[i * N + j] += local_A[i * N + k] * B[k * N + j];
                }
            }
        }
        
        printf("[Node: %s, Rank: %d/%d] Completed rows %d-%d of %d (%.1f%%)\n", 
               hostname, world_rank, world_size, 
               chunk, end_row - 1, rows_per_process, 
               (float)end_row / rows_per_process * 100);
        fflush(stdout);
    }
    
    printf("[Node: %s, Rank: %d/%d] Local multiplication complete, gathering results\n", 
           hostname, world_rank, world_size);
    fflush(stdout);
    
    // gather results
    MPI_Gather(local_C, rows_per_process * N, MPI_INT,
               C, rows_per_process * N, MPI_INT,
               0, MPI_COMM_WORLD);
    
    printf("[Node: %s, Rank: %d/%d] Results sent to rank 0\n", 
           hostname, world_rank, world_size);
    fflush(stdout);
    
    // mark end time and print results
    if (world_rank == 0) {
        double end_time = MPI_Wtime();
        printf("[Node: %s, Rank: %d/%d] Matrix multiplication completed in %f seconds\n", 
               hostname, world_rank, world_size, end_time - start_time);
        
        // print small matrices for verification
        // if (N <= 10) {
        //     printf("Matrix A:\n");
        //     print_matrix(A, N, N);
        //     printf("Matrix B:\n");
        //     print_matrix(B, N, N);
        //     printf("Result Matrix C:\n");
        //     print_matrix(C, N, N);
        // }
    }
    
    // clean up !!!
    free(local_A);
    free(local_C);
    if (B) free(B);
    if (world_rank == 0) {
        free(A);
        free(C);
    }
    
    printf("[Node: %s, Rank: %d/%d] Process completed, cleaning up and exiting\n", 
           hostname, world_rank, world_size);
    fflush(stdout);
    
    MPI_Finalize();
    return 0;
}