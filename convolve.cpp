#include <stdio.h>
#include <stdlib.h>
#include <cstring>
#include <math.h>

#ifndef FILE_STRUCTS_H
#define FILE_STRUCTS_H
#include "headers/file_structs.h"
#endif

#include "headers/reader.h"
#include "headers/writer.h"
#include "headers/convolution.h"
#include "headers/fast_fourier.h"
#include "headers/misc.h"

using namespace std;

#define USE_FAST_FOURIER    60

float short_to_float(short i);
short float_to_short(float i);
short double_to_short(double i);
wav_file create_wav(short* samples, int num_samples, wav_file input);
wav_file convolution(wav_file input, wav_file ir);

/********
 * Main *
 ********/
int main(int argc, char* argv[]) {
    if (argc != 4) {
        printf("Usage: %s <input_file> <IR_file> <output_file>\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    exit_if_invalid(argv[1]);
    exit_if_invalid(argv[2]);

    wav_file input = read_file(argv[1]);
    // printf("INPUT FILE:\n");
    // print_file_data(input);

    wav_file ir = read_file(argv[2]);
    // printf("\nIMPULSE RESPONSE:\n");
    // print_file_data(ir);

    wav_file output = convolution(input, ir);
    // printf("\nOUTPUT FILE:\n");
    // print_file_data(output);

    write_to_file(output, argv[3]);

    free(input.data.samples);
    free(ir.data.samples);
    free(output.data.samples);

    return EXIT_SUCCESS;
}

/* Format and create the wav_file struct to be written to a file */
wav_file create_wav(short* samples, int num_samples, wav_file input) {
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

/* Take the input and ir wav_files, format their data as floats and perform the convolution */
wav_file convolution(wav_file input, wav_file ir) {
    int N = input.data.num_samples;
    int M = ir.data.num_samples;
    int P = N + M;
    if (M > USE_FAST_FOURIER) { P--; }
    else { P++; }
    int i;

    // Output samples after being converted back to floats
    short *samples = (short *)malloc(P * input.fmt.block_align);

    // x and h arrays are handled as decimals between -1.0 and 1.0
    float *x = new float[N];
    float *h = new float[M];
    float *y = new float[P];

    // Convert the stored sample values as floats from -1.0 to 1.0
    for (i = 0; i < N; i++)
        x[i] = short_to_float(input.data.samples[i]);
    for (i = 0; i < M; i++)
        h[i] = short_to_float(ir.data.samples[i]);
    for (i = 0; i < P; i++)
        y[i] = 0.0f;

    // Use FFT convolution when the size of M is greater than 60 samples, otherwise use linear convolution
    if (M > 60) {
        fft_convolution(x, N, h, M, y, P);
    } else {
        linear_convolution(x, N, h, M, y, P);
    }

    // The convolution can result in values > 1.0, make sure to scale them
    float largest = 1.0;
    for (i = 0; i < P; i++) {
        if (abs(y[i]) > largest) { largest = abs(y[i]); }
    }

    // Convert the doubles now stored in y back into sample values
    for (i = 0; i < P; i++) {
        samples[i] = double_to_short(y[i] / largest);
    }

    // Clean up the allocated arrays
    delete(x);
    delete(h);
    delete(y);

    // Create the output wav file using the above data
    return create_wav(samples, P, input);
}
