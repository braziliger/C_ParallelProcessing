/*
 *  ping-pong algorithm
 *
 *  compile:
 *      mpicc ./pingpong.c -o pingpong
 *  run:
 *      ibrun -np <p> pingpong
 *
 *      <p> - # of processors to use - p MUST BE 2
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <mpi.h>

int main(int argc, char* argv[]) {
    /*  VARIABLES  */
    int p;
    int id;
    int loops;
    int len;

    double start;
    double end;
    double elapsed;
    double avg;

    MPI_Status stat;

    /*  MPI INIT  */
    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &p);
    MPI_Comm_rank(MPI_COMM_WORLD, &id);

    // make sure only 2 processors are being used
    if (p != 2) {
        if (id == 0) {
            printf("\nPing-pong must be run with only 2 processors!\n");
        }
        return EXIT_FAILURE;
    }

    /*  PING PONG  */
    loops = 2500;

    // this loop is from 0 - 512 bytes, w/ increments of 32 bytes
    for (int i = 0; i < 17; i++) {

        // type double has 4 bytes on Stampede2, so this makes sure the array is 0, 32, 64, ... bytes in size

        int len = i * 4;
        double arr[len];

        for (int i = 0; i < len; i++) {
            arr[i] = 0.0;
        }

        // sync up
        MPI_Barrier(MPI_COMM_WORLD);

        // start timer
        start = MPI_Wtime();

        // ping pong
        for (int i = 0; i < loops; i++) {
            if (id == 0) {
                MPI_Send(arr, len, MPI_DOUBLE, 1, 11, MPI_COMM_WORLD);
                MPI_Recv(arr, len, MPI_DOUBLE, 1, 22, MPI_COMM_WORLD, &stat);
            }
            else if (id == 1) {
                MPI_Recv(arr, len, MPI_DOUBLE, 0, 11, MPI_COMM_WORLD, &stat);
                MPI_Send(arr, len, MPI_DOUBLE, 0, 22, MPI_COMM_WORLD);
            }
        }

        // end timer
        end = MPI_Wtime();

        // get the average time to transmit across nodes
        elapsed = end - start;
        avg = elapsed / (2.0 * (double)loops);

        // get the size of the array in bytes
        size_t x = sizeof(arr);

        // print results
        if (id == 0) {
            printf("%3d bytes:\tavg: %.12f\n", x, avg);
        }
    }

    if (id == 0) {
        printf("\n\n");
    }

    // this loop is for 1 - 128 kb, in increments of 1 kb
    for (int i = 1; i <= 128; i++) {

        int len = i * 128;
        double arr[len];

        for (int i = 0; i < len; i++) {
            arr[i] = 0.0;
        }

        // sync up
        MPI_Barrier(MPI_COMM_WORLD);

        // start timer
        start = MPI_Wtime();

        // ping pong
        for (int i = 0; i < loops; i++) {
            if (id == 0) {
                MPI_Send(arr, len, MPI_DOUBLE, 1, 11, MPI_COMM_WORLD);
                MPI_Recv(arr, len, MPI_DOUBLE, 1, 22, MPI_COMM_WORLD, &stat);
            }
            else if (id == 1) {
                MPI_Recv(arr, len, MPI_DOUBLE, 0, 11, MPI_COMM_WORLD, &stat);
                MPI_Send(arr, len, MPI_DOUBLE, 0, 22, MPI_COMM_WORLD);
            }
        }

        // end timer
        end = MPI_Wtime();

        // get the average time to transmit across nodes
        elapsed = end - start;
        avg = elapsed / (2.0 * (double)loops);

        // get the size of the array in KB
        size_t x = sizeof(arr)/1024;

        // print results
        if (id == 0) {
            printf("%3d kb:\t\tavg: %.12f\n", x, avg);
        }
    }

    // clean up and exit
    MPI_Finalize();
    return EXIT_SUCCESS;
}