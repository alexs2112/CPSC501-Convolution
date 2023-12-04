#include <stdio.h>
#include "headers/convolution.h"
#include "headers/fast_fourier.h"
#include "headers/misc.h"

#ifndef FILE_STRUCTS_H
#define FILE_STRUCTS_H
#include "headers/file_structs.h"
#endif

/****************************************************************************
 * Custom unit testing framework                                            *
 * To add a new test:                                                       *
 *  Add the test function and test definition                               *
 *  Increment NUM_TESTS                                                     *
 *  Add a pointer to the new test to the `tests` array at the start of main *
 ****************************************************************************/

/* Test Definitions */
#define NUM_TESTS   4
int test_linear_convolution();
int test_fft_convolution();
int test_complex_multiplication();
int test_zero_padding();

/* Main Function */
int main(int argc, char* argv[]) {
    int (*tests[])() = {
        test_linear_convolution,
        test_fft_convolution,
        test_complex_multiplication,
        test_zero_padding,
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
    float y[P];
    linear_convolution(x, N, h, M, y, P);

    float expected[] = {
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
    float y[P];
    fft_convolution(x, N, h, M, y, P);

    float expected[] = {
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
    float *x;
    float *h;
    float *output;
    int size;
};

int test_complex_multiplication() {
    float x[] = { 1, 2, 3, 4 };
    float h[] = { 8, 7, 6, 5 };
    float o[4];

    complex_param p;
    p.x = &x[0];
    p.h = &h[0];
    p.output = &o[0];
    p.size = 4;
    complex_multiply((void *)&p);

    float e[] = { -6.0, 23.0, -2.0, 39.0 };

    for (int i = 0; i < 4; i++) {
        if (o[i] != e[i]) {
            printf("Error in complex multiplication.\n\tExpected: [%.1f, %.1f, %.1f, %.1f]\n\tActual: [%.1f, %.1f, %.1f, %.1f]\n",
                e[0], e[1], e[2], e[3], o[0], o[1], o[2], o[3]);
            return 1;
        }
    }
    return 0;
}

int test_zero_padding() {
    float signal[] = { 0.5, 0.6, 0.7 };
    float output[8];
    zero_padding(signal, 3, output, 8);
    float expected[] = { 0.5, 0.0, 0.6, 0.0, 0.7, 0.0, 0.0, 0.0 };
    for (int i = 0; i < 8; i++) {
        if ((float)(output[i]) != (float)(expected[i])) {
            printf("Error in zero padding.\n\tExpected: [%.1f, %.1f, %.1f, %.1f, %.1f, %.1f, %.1f, %.1f]\n\tActual:   [%.1f, %.1f, %.1f, %.1f, %.1f, %.1f, %.1f, %.1f]\n",
                expected[0], expected[1], expected[2], expected[3], expected[4], expected[5], expected[6], expected[7],
                output[0], output[1], output[2], output[3], output[4], output[5], output[6], output[7]);
            return 1;
        }
    }
    return 0;
}