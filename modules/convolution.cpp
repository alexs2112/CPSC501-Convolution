#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <cstring>
#include "../headers/file_structs.h"

/***************
 * Convolution *
 ***************/
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

/* Using a slightly modified version of the input-side algorithm presented in lecture */
void convolve(float* x, int N, float* h, int M, float* y, int P) {
    int n, m;

	// Clear output buffer y[]
	for (n = 0; n < P; n++) {
		y[n] = 0.0;
	}

	// Outer Loop: Process each input value x[n] in turn
	for (n = 0; n < N; n++) {
		// Inner Loop: process x[n] with each sample of h[n]
		for (m = 0; m < M; m++) {
			y[n+m] += x[n] * h[m];
		}
	}
}

/* Some conversion functions */
float short_to_float(short i) {
    float f;
    if (i < 0) { f = ((float)i) / 32768.0f; }
    else { f = ((float)i) / 32767.0f; }
    return f;
}

short float_to_short(float i) {
    short s;
    if (i < 0) { s = rint(i * 32768.0f); }
    else { s = rint(i * 32767.0f); }
    return s;
}

/* Take the input and ir wav_files, format their data as floats and perform the convolution */
wav_file convolve_files(wav_file input, wav_file ir) {
    int N = input.data.num_samples;
    int M = ir.data.num_samples;
    int P = N + M + 1;

    int f = sizeof(float);
    float *x = (float *)malloc(N * f);
    float *h = (float *)malloc(M * f);
    float *y = (float *)malloc(P * f);

    // Convert the stored sample values as floats from -1.0 to 1.0
    int i;
    for (i = 0; i < N; i++) {
        x[i] = short_to_float(input.data.samples[i]);
    }
    for (i = 0; i < M; i++) {
        h[i] = short_to_float(ir.data.samples[i]);
    }

    // Perform the convolution
    convolve(x, N, h, M, y, P);

    // The convolution can result in values > 1.0, make sure to scale them
    float largest = 1.0;
    for (i = 0; i < P; i++) {
        if (abs(y[i]) > largest) { largest = abs(y[i]); }
    }
    printf("\nScaling output by %f\n\n", largest);

    // Convert the floats now stored in y back into sample values
    short *samples = (short *)malloc(P * input.fmt.block_align);
    for (i = 0; i < P; i++) {
        samples[i] = float_to_short(y[i] / largest);
    }

    // Clean up the float arrays
    free(x);
    free(h);
    free(y);

    // Create the output wav file using the above data
    return create_wav(samples, P, input);
}
