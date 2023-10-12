#include <iostream>
#include <fstream>
#include "mpi.h"
using namespace std;

void swap(int& a, int& b) {
    int temp = a;
    a = b;
    b = temp;
}

void bubbleSort(int* arr, int n) {
    for (int i = 0; i < n - 1; i++) {
        for (int j = 0; j < n - i - 1; j++) {
            if (arr[j] > arr[j + 1]) {
                swap(arr[j], arr[j + 1]);
            }
        }
    }
}

int* mergeTwoSortedArr(int* arr1, int n1, int* arr2, int n2)
{
    int i, j, k;
    int* result;

    result = new int[n1 + n2];
    i = 0;
    j = 0;
    k = 0;

    while (i < n1 && j < n2)
        if (arr1[i] < arr2[j]) {
            result[k] = arr1[i];
            i++;
            k++;
        }
        else {
            result[k] = arr2[j];
            j++;
            k++;
        }

    if (i == n1)
        while (j < n2) {
            result[k] = arr2[j];
            j++;
            k++;
        }
    if (j == n2)
        while (i < n1) {
            result[k] = arr1[i];
            i++;
            k++;
        }

    return result;
}

void main(int argc, char** argv)
{
    int* arr;
    int* sorted_subarr;
    int* sorted_arr;

    double start, stop;

    int n;
    int rank, size;
    int r;
    int sorted_size, sorted_subarr_size;
    MPI_Status status;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    n = 10;

    if (rank == 0) {
        cout << "Enter the size of array: ";
        cin >> n;
        cout << endl;
    }

    sorted_size = n / size;
    r = n % size;
    arr = new int[n + sorted_size - r];

    if (rank == 0) {
        srand(unsigned int(MPI_Wtime()));
        cout << "Array:" << endl;
        ofstream file("input.txt");

        for (int i = 0; i < n; i++)
        {
            arr[i] = rand() / 10;
            cout << arr[i] << " ";
            file << arr[i] << " ";
        }

        cout << "\n\n";
        file.close();

        if (r != 0) {
            for (int i = n; i < n + sorted_size - r; i++)
                arr[i] = INT_MAX;

            sorted_size = sorted_size + 1;
        }

        start = MPI_Wtime();
    }
   
    MPI_Bcast(&sorted_size, 1, MPI_INT, 0, MPI_COMM_WORLD);
    sorted_arr = new int[sorted_size];
    MPI_Scatter(arr, sorted_size, MPI_INT, sorted_arr, sorted_size, MPI_INT, 0, MPI_COMM_WORLD);
    bubbleSort(sorted_arr, sorted_size);

    if (rank == 0) {
        for (int i = 1; i < size; i++) {
            MPI_Recv(&sorted_subarr_size, 1, MPI_INT, i, 0, MPI_COMM_WORLD, &status);
            sorted_subarr = new int[sorted_subarr_size];
            MPI_Recv(sorted_subarr, sorted_subarr_size, MPI_INT, i, 0, MPI_COMM_WORLD, &status);
            sorted_arr = mergeTwoSortedArr(sorted_arr, sorted_size, sorted_subarr, sorted_subarr_size);
            sorted_size = sorted_size + sorted_subarr_size;
        }
    } else {
        MPI_Send(&sorted_size, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
        MPI_Send(sorted_arr, sorted_size, MPI_INT, 0, 0, MPI_COMM_WORLD);
    }

    if (rank == 0) {
        stop = MPI_Wtime();
        double parallel_time = stop - start;

        start = MPI_Wtime();
        bubbleSort(arr, n);
        stop = MPI_Wtime();

        double seq_time = stop - start;

        cout << "Sorted array:" << endl;
        ofstream file("output.txt");
        for (int i = 0; i < n; i++)
        {
            cout << sorted_arr[i] << " ";
            file << sorted_arr[i] << " ";
        }

        cout << "\n\n";
        file.close();
        
        cout << "Sequental algorithm time: " << seq_time << endl;
        cout << "Parallel algorithm time: " << parallel_time << endl;
        if (seq_time / parallel_time < 1) {
            cout << parallel_time / seq_time << " times slower" << endl;
        } else {
            cout << seq_time / parallel_time << " times faster" << endl;
        }
    }

    MPI_Finalize();
}