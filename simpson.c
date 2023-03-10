/*
 *  simpsons rule
 *
 *  compile:
 *      mpicc ./simpson.c -o simpson
 *  run:
 *      ibrun -np <p> simpson <n>
 *
 *      <p> - # of processors
 *      <n> - # up to subintervals to use
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <mpi.h>

double fxn(double x);

int main(int argc, char* argv[]) {
    // VARIABLES //

    int a, b;       // where to start and stop integration
    int id;         // current processor's ID/rank
    int p;          // number of processors
    int n;          // number of subintervals to use
    int i;

    double step;    // length of the subintervals
    double start;   // start position of the processors interval
    int start_idx;
    int end_idx;
    double end;     // end position of the procesors interval
    double ct;      // number of subintervals in each processor
    double summation;
    double x;
    double final;

    // initialize MPI
    MPI_Init(&argc, &argv);
    // get number of processes
    MPI_Comm_size(MPI_COMM_WORLD, &p);
    // get ID for this process
    MPI_Comm_rank(MPI_COMM_WORLD, &id);

    // integration bounds:
    a = 0;
    b = 1;

    // read in <n>, number of subintervals
    n = atoi(argv[1]);

    // make sure <n> is even
    if (n%2 != 0) {
        printf("<n> must be even, exiting");
        return EXIT_FAILURE;
    }

    // length of subintervals
    step = (double) (1.0 / n);

    // start/end index of each processor
    start_idx = (n / p) * id;
    end_idx = (n / p) * (id + 1);

    // start/end value of each processor
    start = start_idx * step;
    end = end_idx * step;

    // first calculation - x at start of interval
    x = start;
    summation = fxn(x);
    x += step;

    // do the summation calculation
    for (i = 1; x < end; i++) {
        if ((i%2) == 1) {
            summation += 2*fxn(x);
        }
        else {
            summation += 4*fxn(x);
        }
        x += step;
    }

    // final calculation - xn
    x = end;
    summation += fxn(x);

    summation *= (1.0 / (3.0 * n));

    // collect it together  on p0
    MPI_Reduce(&summation, &final, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);

    // print out pi
    if (id == 0) {
        printf("\nValue of pi: %.8f\n\n", final);
    }

    // clean up and exit
    MPI_Finalize();
    return EXIT_SUCCESS;
}

double fxn(double x) {
    double answer;

    // this is the function we're integrating
    answer = (4.0 / (1.0 + pow(x, 2)));
    return answer;
}