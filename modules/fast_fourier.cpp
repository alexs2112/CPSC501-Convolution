#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define SIZE        8
#define PI          3.141592653589793
#define TWO_PI      (2.0 * PI)
#define SWAP(a, b)  tempr=(a);(a)=(b);(b)=tempr

// Replaces data[1..2*nn] by its discrete Fourier transform if isign is input as 1
// or replaces data[1..2*nn] by nn times its inverse discrete Fourier transform if isign is input as -1
// Params:
//    data pointer to data array, this should be subtracted by 1 as our arrays are 0 aligned (ie. four1(data - 1, 1024, 1))
//    nn must be a power of 2, it is the number of complex data points in data (ie. half the length of data)
//    isign = +1 for FFT, -1 for IFFT
void four1(double *data, int nn, int isign) {
    unsigned long n, mmax, m, j, istep, i;
    double wtemp, wr, wpr, wpi, wi, theta;
    double tempr, tempi;

    n = nn << 1;
    j = 1;

    for (i = 1; i < n; i += 2) {
        if (j > i) {
            SWAP(data[j], data[i]);
            SWAP(data[j+1], data[i+1]);
        }
        m = nn;
        while (m >= 2 && j > m) {
            j -= m;
            m >>= 1;
        }
        j += m;
    }

    mmax = 2;
    while (n > mmax) {
        istep = mmax << 1;
        theta = isign * (TWO_PI / mmax);
        wtemp = sin(0.5 * theta);
        wpr = -2.0 * wtemp * wtemp;
        wpi = sin(theta);
        wr = 1.0;
        wi = 0.0;

        for (m = 1; m < mmax; m += 2) {
            for (i = m; i <= n; i += istep) {
                j =i + mmax;
                tempr = wr * data[j] - wi * data[j+1];
                tempi = wr * data[j+1] + wi * data[j];
                data[j] = data[i] - tempr;
                data[j+1] = data[i+1] - tempi;
                data[i] += tempr;
                data[i+1] += tempi;
            }
            wr = (wtemp = wr) * wpr - wi * wpi + wr;
            wi = wi * wpr + wtemp * wpi + wi;
        }
        mmax = istep;
    }
}

// Determine the next highest power of 2 for an integer value
// https://graphics.stanford.edu/%7Eseander/bithacks.html#RoundUpPowerOf2
unsigned int next_power(unsigned int v) {
    v--;
    v |= v >> 1;
    v |= v >> 2;
    v |= v >> 4;
    v |= v >> 8;
    v |= v >> 16;
    v++;
    return v;
}

// Output size should be 2x the padded size to account for complex numbers
void zero_padding(float *signal, int input_size, double *output, int output_size) {
    int i;
    for (i = 0; i < output_size; i++) {
        output[i] = 0.0;
    }

    for (i = 0; i < input_size; i++) {
        output[i*2] = (double)signal[i];
    }
}

void complex_multiply(double *x, double *h, double *output, int size) {
    // Perform complex multiplication
    for (int k = 0; k < size; k += 2) {
        // Re Y[k] = Re X[k] Re H[k] - Im X[k] Im H[k]
        // Im Y[k] = Im X[k] Re H[k] + Re X[k] Im H[k]
        output[k] = x[k] * h[k] - x[k+1] * h[k+1];
        output[k+1] = x[k+1] * h[k] + x[k] * h[k+1];
    }
}

// Perform convolution using the FFT algorithm
// Params:
//    x: input signal samples between -1.0 and 1.0
//    N: length of x
//    h: ir signal samples between -1.0 and 1.0
//    M: length of h
//    y: output signal samples, scaled to be between -1.0 and 1.0
//    P: length of y, this should equal N + M - 1
void fft_convolution(float *x, int N, float *h, int M, double *y, int P) {
    int padded_size = next_power(P);

    double* padded_x = new double[2 * padded_size];
    double* padded_h = new double[2 * padded_size];
    double* padded_out = new double[2 * padded_size];

    zero_padding(x, N, padded_x, 2 * padded_size);
    zero_padding(h, M, padded_h, 2 * padded_size);

    four1(padded_x - 1, padded_size, 1);
    four1(padded_h - 1, padded_size, 1);

    complex_multiply(padded_x, padded_h, padded_out, 2 * padded_size);

    four1(padded_out - 1, padded_size, -1);

    for (int i = 0; i < P; i++)
        y[i] = padded_out[i * 2] / (double)N;
    
    delete(padded_x);
    delete(padded_h);
    delete(padded_out);
}
