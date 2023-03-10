/*\
 *  SHIFT problem
 *
 *  compile:
 *      mpicc ./shift.c -o shift
 *  run:
 *      ibrun -np <p> shift <n> <s>
 *
 *      <p> - # of processors to use
 *      <n> - length of the array in each processor
 *              so if <n> = 4, each process gets an array
 *              with 4 elements
 *      <s> - shift factor
 */

#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <unistd.h>

void Shift(int* arr1, int* arr2, int sh, int arr_len);
void printArrays(int arr1[], int arr2[], int arr_len);

int main(int argc, char* argv[]) {
    /*  VARIABLES  */
    int p;
    int id;
    int arr_len;
    int sh;

    MPI_Status stat;

    /* MPI INIT */
    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &p);
    MPI_Comm_rank(MPI_COMM_WORLD, &id);

    // get array size
    arr_len = atoi(argv[1]);
    // get shift factor
    sh = atoi(argv[2]);

    // check for the right number of inputs
    if (argc != 3) {
        if (id == 0) {
            printf("Wrong number of inputs\n\n");
        }
        return EXIT_FAILURE;
    }

    // each processor needs an array
    int arr1[arr_len];
    int arr2[arr_len];
    for (int i = 0; i < arr_len; i++) {
        arr1[i] = 0;
        arr2[i] = 0;
    }

    // get inputs
    for (int i = 0; i < p; i++) {
        // sync up
        MPI_Barrier(MPI_COMM_WORLD);

        // p0 reads in values
        if (id == 0) {
            for (int j = 0; j < arr_len; j++) {
                printf("input p%d at arr[%d]: ", i, j);
                fflush(stdout);
                scanf("%d", &arr2[j]);
            }
        }

        // processors wait here for p0 to get input
        MPI_Barrier(MPI_COMM_WORLD);

        //p0 sends values to the proper processor
        if(i == 0) {
            // i = 0; p0 keeps this one
            if (id == 0) {
                for (int j = 0; j < arr_len; j++) {
                    arr1[j] = arr2[j];
                }
            }
        }
        else {
            // i != 0; p0 needs to send the array to another processor
            if (id == 0) {
                MPI_Send(arr2, arr_len, MPI_INT, i, 0, MPI_COMM_WORLD);
            }
            else if (id == i) {
                MPI_Recv(arr1, arr_len, MPI_INT, 0, 0, MPI_COMM_WORLD, &stat);
            }
        }
    }

    // perform shift
    Shift(arr1, arr2, sh, arr_len);

    // print results
    if (id == 0) {
        printf("\n\n ******** RESULTS *********\n");
        printf("Input array:\n");
    }

    MPI_Barrier(MPI_COMM_WORLD);

    for (int i = 0; i < p; i++) {
        if (id == i) {
            for (int j = 0; j < arr_len; j++) {
                printf("|%3d", arr1[j]);
            }
            printf("\n");
        }
        MPI_Barrier(MPI_COMM_WORLD);
    }

    if (id == 0) {
        sleep(1);
        printf("\n\nPress ENTER for shift results:");
        fflush(stdout);
        getchar();
        while(getchar() != '\n');
    }

    MPI_Barrier(MPI_COMM_WORLD);

    if (id == 0) {
        printf("\n\nArray after shift of %3d:\n", sh);
        fflush(stdout);
    }

    MPI_Barrier(MPI_COMM_WORLD);

    for (int i = 0; i < p; i++) {
        if (id == i) {
            for (int j = 0; j < arr_len; j++) {
                printf("|%3d", arr2[j]);
            }
            printf("\n");
        }
        MPI_Barrier(MPI_COMM_WORLD);
    }

    if (id == 0) {
        sleep(1);
        printf("\n");
    }

    // clean up n close out
    MPI_Finalize();
    return EXIT_SUCCESS;
}

void Shift(int* arr1, int* arr2, int sh, int arr_len) {
    int p;
    int id;
    int temp;

    MPI_Status stat;

    MPI_Comm_rank(MPI_COMM_WORLD, &id);
    MPI_Comm_size(MPI_COMM_WORLD, &p);

    // copy arr1 into arr2
    for (int i = 0; i < arr_len; i++) {
        arr2[i] = arr1[i];
    }

    if (sh == 0) {
        return;
    }

    // sh is positive
    if (sh > 0) {
        for (int i = 0; i < sh; i++) {
            MPI_Barrier(MPI_COMM_WORLD);

            // send the end of the array to the next processor
            if (id == 0) {
                MPI_Send(&arr2[arr_len - 1], 1, MPI_INT, (id + 1), 0, MPI_COMM_WORLD);
                MPI_Recv(&temp, 1, MPI_INT, (p - 1), 0, MPI_COMM_WORLD, &stat);
            } else if (id == (p - 1)) {
                MPI_Send(&arr2[arr_len - 1], 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
                MPI_Recv(&temp, 1, MPI_INT, (id - 1), 0, MPI_COMM_WORLD, &stat);
            } else {
                MPI_Send(&arr2[arr_len - 1], 1, MPI_INT, (id + 1), 0, MPI_COMM_WORLD);
                MPI_Recv(&temp, 1, MPI_INT, (id - 1), 0, MPI_COMM_WORLD, &stat);
            }

            // move the array up
            for (int j = arr_len - 1; j > 0; j--) {
                arr2[j] = arr2[j - 1];
            }

            // replace the first value
            arr2[0] = temp;
        }
    }
    else { // sh is negative
        for (int i = sh; i < 0; i++) {
            MPI_Barrier(MPI_COMM_WORLD);

            // send the start of the array to the previous processor
            if (id == 0) {
                MPI_Send(&arr2[0], 1, MPI_INT, (p - 1), 0, MPI_COMM_WORLD);
                MPI_Recv(&temp, 1, MPI_INT, (id + 1), 0, MPI_COMM_WORLD, &stat);
            } else if (id == (p - 1)) {
                MPI_Send(&arr2[0], 1, MPI_INT, (id - 1), 0, MPI_COMM_WORLD);
                MPI_Recv(&temp, 1, MPI_INT, (0), 0, MPI_COMM_WORLD, &stat);
            } else {
                MPI_Send(&arr2[0], 1, MPI_INT, (id - 1), 0, MPI_COMM_WORLD);
                MPI_Recv(&temp, 1, MPI_INT, (id + 1), 0, MPI_COMM_WORLD, &stat);
            }

            // move the array down
            for (int j = 0; j < arr_len-1; j++) {
                arr2[j] = arr2[j + 1];
            }

            // replace the last value
            arr2[arr_len-1] = temp;
        }
    }

    return;
}