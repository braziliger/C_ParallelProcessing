/*
 *  sieve of erastothenes
 *
 *  compile:
 *      mpicc ./erastothenes.c -o erastothenes
 *  run:
 *      ibrun -np <p> erastothenes <n>
 *
 *      <p> - # of processors to use
 *      <n> - find primes up to this number
 */

#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <math.h>

int main(int argc, char* argv[]) {
    // VARIABLES //
    int p;
    int id;
    int n;
    int arr_len;
    int k;
    int start;
    int end;
    int i;
    int next;

    double start_time;
    double end_time;

    // MPI //
    // init MPI
    MPI_Init(&argc, &argv);
    // get <p>
    MPI_Comm_size(MPI_COMM_WORLD, &p);
    // get the ID for this processor
    MPI_Comm_rank(MPI_COMM_WORLD, &id);

    // find all the prime numbers up to n:
    n = atoi(argv[1]);

    // make an array long enough to hold <n>, so that each processor controls an equal range
    arr_len = n - 1;

    if ((arr_len) % p == 0) {
        // length is evenly divisible by p - dont do anything
    }
    else {
        // if it isnt, extend it a tad
        arr_len = (arr_len / p) + 1;
        arr_len = arr_len * p;
    }

    int numbers[arr_len];

    int final[arr_len];

    for (i = 0; i < arr_len; i++) {
        numbers[i] = 0;
        final[i] = 0;
    }


    // set start/end ranges for each processor
    start = (arr_len / p) * id;
    end = (arr_len / p) * (id + 1);

    // set initial value of k
    k = 2;

    // set start time
    if (id == 0)
        start_time = MPI_Wtime();

    // main algorithm
    do {
        for (i = start; i < end; i++) {
            if ((i+2) <= k)
                continue;
            else if ((i+2) % k == 0) {
                numbers[i]++;
            }
        }

        MPI_Reduce(&numbers, &final, arr_len, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);

        if (id == 0) {
            int idx = k - 1;
            while (final[idx] != 0)
                idx++;
            k = idx + 2;
        }

        MPI_Bcast(&k, 1, MPI_INT, 0, MPI_COMM_WORLD);
        MPI_Barrier(MPI_COMM_WORLD);
    } while (pow(k, 2) <= n);


    if (id == 0) {
        end_time = MPI_Wtime();

        // print the primes out
        for (int i = 0; i+2 < n; i++) {
            if (final[i] == 0) {
                printf("%d\n", i+2);
            }
        }

        // print elapsed time
        printf("\n\nTIME ELAPSED:\t%f seconds\n\n", (end_time - start_time));
    }

    MPI_Finalize();

    return EXIT_SUCCESS;
}