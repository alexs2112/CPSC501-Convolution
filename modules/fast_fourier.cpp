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

// Convert our float arrays into complex double arrays
// Params:
//     data: input floats read from the wav files
//     data_len: length of the data array
//     output: should be at least twice the length as the data array padded to a length of power of 2
//     out_len: length of the output array
void convert_to_complex(float data[], int data_len, double output[], int out_len) {
    // Clear the output array
    for (int i = 0; i < out_len; i++) {
        output[i] = 0.0;
    }

    // As this is a real signal, the imaginary parts remain as 0
    for (int i = 0; i < data_len; i++) {
        output[i * 2] = (double)data[i];
    }
}

// Perform a convolution on input segment array and ir array
// input and ir should be complex arrays that have already gone through the four1 function
// in_length should be the length of input and ir, this should be identical and a power of 2 * 2
// output is the output buffer that is filled by the convolution
// out_length is the size of the output buffer, it should be equal to (in_length * 2 - 1)
// out_signals is the number of signals stored in the output buffer, this needs to be a power of 2
void convolution(double input[], double ir[], int in_length, double output[], int out_length, int out_signals) {
    // Clear the output buffer
    int k;
    for (k = 0; k < out_length; k++)
        output[k] = 0.0;

    // Perform complex multiplication
    for (k = 0; k < in_length; k += 2) {
        // Re Y[k] = Re X[k] Re H[k] - Im X[k] Im H[k]
        // Im Y[k] = Im X[k] Re H[k] + Re X[k] Im H[k]
        output[k] = input[k] * ir[k] - input[k+1] * ir[k+1];
        output[k+1] = input[k+1] * ir[k] + input[k] * ir[k+1];
    }

    // Perform the IFFT
    four1(output - 1, out_signals, -1);
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

// Break x into several chunks of length next_power(M*2)
// Perform FFT convolution on each chunk of x and h
// Use the Overlap Add method to stick those chunks together again to get y
// Params:
//    x: input signal samples between -1.0 and 1.0
//    N: length of x
//    h: ir signal samples between -1.0 and 1.0
//    M: length of h
//    y: output signal samples, scaled to be between -1.0 and 1.0
//    P: length of y, I think this should equal N + M - 1 (not sure about padding)
void fft_convolve(float *x, int N, float *h, int M, double *y, int P) {
    // Number of signals in a complex array, multiplied by 2 to avoid circular convolution
    unsigned int carr_signals = next_power(2 * M);

    // Number of values in a complex array as each signal makes 2 values
    unsigned int carr_length = carr_signals * 2;

    // Perform FFT on the IR signal
    double *complex_h = (double *)malloc(carr_length * sizeof(double));
    convert_to_complex(h, M, complex_h, carr_length);
    four1(complex_h-1, M, 1);

    // Break x[] into a bunch of segments of size (carr_signals / 2), perform convolution on each segment
    unsigned int segment_signals = carr_signals / 2;
    double *complex_x = (double *)malloc(carr_length * sizeof(double));
    double *output = (double *)malloc(carr_length * sizeof(double) * 2 - sizeof(double));
    int i;
    for (i = 0; i < N; i += segment_signals) {
        // Perform FFT on the new segment
        unsigned int signals;
        if ((i + segment_signals) > N) {
            // Truncate x if it would overflow the buffer
            signals = N - i;
        } else {
            signals = segment_signals;
        }
        convert_to_complex(&x[i], signals, complex_x, carr_length);
        four1(complex_x-1, signals, 1);

        // Do complex multiplication of complex_x and complex_h, then do the IFFT
        // Put the result into the output buffer
        convolution(complex_x, complex_h, carr_length, output, carr_length * 2 - 1, carr_signals);

        // Then overlap add the result back into y, assume that y is an empty array of doubles at this point
        for (int j = 0; j < carr_length; j++) {
            y[i * segment_signals + j] += output[j];
        }
    }

    printf("Free h\n");
    free(complex_h);
    printf("Free x\n");
    free(complex_x);
    printf("Free out\n");
    free(output);
}


/***************************************************************************************************
 * EVERYTHING BELOW THIS LINE SHOULD BE DELETED AND STUFF SHOULD BE ORGANIZED BETTER ONCE IT WORKS *
 ***************************************************************************************************/
#include "../headers/file_structs.h"
#include <cstring>

float short_to_float_pt2(short i) {
    float f;
    if (i < 0) { f = ((float)i) / 32768.0f; }
    else { f = ((float)i) / 32767.0f; }
    return f;
}

short double_to_short(double i) {
    short s;
    if (i < 0) { s = rint(i * 32768.0); }
    else { s = rint(i * 32767.0); }
    return s;
}

wav_file create_wav_pt2(short* samples, int num_samples, wav_file input) {
    wav_file output;

    // fmt subchunk
    strncpy(output.fmt.subchunk_id, "fmt ", 4);
    output.fmt.subchunk_size = 16;
    output.fmt.audio_format = 1;
    output.fmt.num_channels = input.fmt.num_channels;
    output.fmt.sample_rate = input.fmt.sample_rate;
    output.fmt.byte_rate = input.fmt.byte_rate;
    output.fmt.block_align = input.fmt.block_align;
    output.fmt.bits_per_sample = input.fmt.bits_per_sample;

    // data subchunk
    strncpy(output.data.subchunk_id, "data", 4);
    output.data.num_samples = num_samples;
    output.data.samples = samples;
    output.data.subchunk_size = num_samples * (output.fmt.num_channels) * (output.fmt.bits_per_sample)/8;

    // header chunk
    strncpy(output.header.chunk_id, "RIFF", 4);
    strncpy(output.header.format, "WAVE", 4);
    output.header.chunk_size = 4 + (8 + output.fmt.subchunk_size) + (8 + output.data.subchunk_size);

    return output;
}

wav_file convolve_files_but_fast(wav_file input, wav_file ir) {
    int N = input.data.num_samples;
    int M = ir.data.num_samples;
    int P = N + M + 1;

    int f = sizeof(float);
    float *x = (float *)malloc(N * f);
    float *h = (float *)malloc(M * f);
    double *y = (double *)malloc(P * sizeof(double));

    // Convert the stored sample values as floats from -1.0 to 1.0
    int i;
    for (i = 0; i < N; i++) {
        x[i] = short_to_float_pt2(input.data.samples[i]);
    }
    for (i = 0; i < M; i++) {
        h[i] = short_to_float_pt2(ir.data.samples[i]);
    }
    for (i = 0; i < P; i++) {
        y[i] = 0.0;
    }

    // Perform the convolution
    fft_convolve(x, N, h, M, y, P);

    // The convolution can result in values > 1.0, make sure to scale them
    float largest = 1.0;
    for (i = 0; i < P; i++) {
        if (abs(y[i]) > largest) { largest = abs(y[i]); }
    }
    printf("\nScaling output by %f\n\n", largest);

    // Convert the floats now stored in y back into sample values
    short *samples = (short *)malloc(P * input.fmt.block_align);
    for (i = 0; i < P; i++) {
        samples[i] = double_to_short(y[i] / largest);
    }

    // Clean up the float arrays
    free(x);
    free(h);
    free(y);

    // Create the output wav file using the above data
    return create_wav_pt2(samples, P, input);
}