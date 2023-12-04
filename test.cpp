#include <stdio.h>
#include "headers/convolution.h"
#include "headers/fast_fourier.h"
#include "headers/misc.h"

#ifndef FILE_STRUCTS_H
#define FILE_STRUCTS_H
#include "headers/file_structs.h"
#endif

/* Test Definitions */
#define NUM_TESTS   3
int test_linear_convolution();
int test_fft_convolution();
int test_complex_multiplication();

/* Main Function */
int main(int argc, char* argv[]) {
    int (*tests[])() = {
        test_linear_convolution,
        test_fft_convolution,
        test_complex_multiplication,
    };

    // Run each test loaded into tests
    int res;
    int failures = 0;
    for (int i = 0; i < NUM_TESTS; i++) {
        res = (*tests[i])();
        if (res == 1) { failures++; }
    }

    printf("\nTest Run Complete.\nTests Run: %d\nTest Failed: %d\n", NUM_TESTS, failures);

    return 0;
}

/* List of Test Functions */
int test_linear_convolution() {
    int N = 3;
    float x[] = { 0.2, 0.3, 0.4 };
    int M = 2;
    float h[] = { 0.1, 0.2 };
    int P = N + M + 1;
    double y[P];
    linear_convolution(x, N, h, M, y, P);

    double expected[] = {
        0.020000,
        0.070000,
        0.100000,
        0.080000,
        0.000000,
        0.000000
    };
    for (int i = 0; i < P; i++) {
        if ((int)(y[i]*100) != (int)(expected[i]*100)) {
            printf("Expected array after linear convolution: [%.3f, %.3f, %.3f, %.3f, %.3f, %.3f]\tActual: [%.3f, %.3f, %.3f, %.3f, %.3f, %.3f]\n",
                expected[0], expected[1], expected[2], expected[3], expected[4], expected[5],
                y[0], y[1], y[2], y[3], y[4], y[5]);
            return 1;
        }
    }
    return 0;
}

int test_fft_convolution() {
    int N = 3;
    float x[] = { 0.2, 0.3, 0.4 };
    int M = 2;
    float h[] = { 0.1, 0.2 };
    int P = N + M - 1;
    double y[P];
    fft_convolution(x, N, h, M, y, P);

    double expected[] = {
        0.026,
        0.093,
        0.133,
        0.106
    };
    for (int i = 0; i < P; i++) {
        if ((int)(y[i]*1000) != (int)(expected[i]*1000)) {
            printf("Expected array after fft convolution: [%.3f, %.3f, %.3f, %.3f]\tActual: [%.3f, %.3f, %.3f, %.3f]\n",
                expected[0], expected[1], expected[2], expected[3],
                y[0], y[1], y[2], y[3]);
            return 1;
        }
    }
    return 0;
}

// Copied from fast_fourier.cpp
struct complex_param {
    double *x;
    double *h;
    double *output;
    int size;
};

int test_complex_multiplication() {
    double x[] = { 1, 2, 3, 4 };
    double h[] = { 8, 7, 6, 5 };
    double o[4];

    complex_param p;
    p.x = &x[0];
    p.h = &h[0];
    p.output = &o[0];
    p.size = 4;
    complex_multiply((void *)&p);

    double e[] = { -6.0, 23.0, -2.0, 39.0 };

    for (int i = 0; i < 4; i++) {
        if (o[i] != e[i]) {
            printf("Error in complex multiplication.\n\tExpected: [%.1f, %.1f, %.1f, %.1f]\n\tActual: [%.1f, %.1f, %.1f, %.1f]\n",
                e[0], e[1], e[2], e[3], o[0], o[1], o[2], o[3]);
            return 1;
        }
    }
    return 0;
}