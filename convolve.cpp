#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <math.h>
#include <cstring>

using namespace std;

/* Some structs to store values for a given file*/
struct file_header {
    char    chunk_id[4];
    int     chunk_size;
    char    format[4];
};

struct file_fmt {
    char    subchunk_id[4];
    int     subchunk_size;
    short   audio_format;
    short   num_channels;
    int     sample_rate;
    int     byte_rate;
    short   block_align;
    short   bits_per_sample;
};

struct file_data {
    char    subchunk_id[4];
    int     subchunk_size;
    int     num_samples;    // Not read from the file, just helpful to store here
    short*  samples;
};

struct wav_file {
    file_header header;
    file_fmt fmt;
    file_data data;
};

/*****************************************
 * Functions for reading from a wav file *
 *****************************************/
/* Get an int from a byte buffer of size 4 */
int int_from_buffer(char buf[]) {
    int value =
        ((buf[3] & 0xFF) << 24) |
        ((buf[2] & 0xFF) << 16) |
        ((buf[1] & 0xFF) << 8) |
        (buf[0] & 0xFF);
    return value;
}

/* Get a short from a byte buffer of size 2 */
short short_from_buffer(char buf[]) {
    short value =
        ((buf[1] & 0xFF) << 8) |
        (buf[0] & 0xFF);
    return value;
}

/* Use file to fill out the provided header */
void get_file_header(file_header &header, FILE* file) {
    fread(&header.chunk_id, 4, 1, file);
    
    char size[4];
    fread(&size, 4, 1, file);
    header.chunk_size = int_from_buffer(size);

    fread(&header.format, 4, 1, file);
}

/* Use file to fill out the provided format subchunk */
void get_file_format(file_fmt &fmt, FILE *file) {
    fread(fmt.subchunk_id, 4, 1, file);

    char size[4];
    fread(&size, 4, 1, file);
    fmt.subchunk_size = int_from_buffer(size);

    char audio_format[2];
    fread(&audio_format, 2, 1, file);
    fmt.audio_format = short_from_buffer(audio_format);

    char num_channels[2];
    fread(&num_channels, 2, 1, file);
    fmt.num_channels = short_from_buffer(num_channels);

    char sample_rate[4];
    fread(&sample_rate, 4, 1, file);
    fmt.sample_rate = int_from_buffer(sample_rate);

    char byte_rate[4];
    fread(&byte_rate, 4, 1, file);
    fmt.byte_rate = int_from_buffer(byte_rate);

    char block_align[2];
    fread(&block_align, 2, 1, file);
    fmt.block_align = short_from_buffer(block_align);

    char bits_per_sample[2];
    fread(&bits_per_sample, 2, 1, file);
    fmt.bits_per_sample = short_from_buffer(bits_per_sample);

    if (fmt.subchunk_size > 16) {
        // Sometimes the fmt of the ir's have a subchunk size that is randomly too big
        // Throw those away
        int l = fmt.subchunk_size - 16;
        char garbage[l];
        fread(&garbage, l, 1, file);
    }
}

/* Use file to fill out the provided data subchunk */
void get_file_data(file_data &data, FILE* file, int sample_size) {
    fread(&data.subchunk_id, 4, 1, file);

    char size[4];
    fread(&size, 4, 1, file);
    data.subchunk_size = int_from_buffer(size);

    int num_samples = data.subchunk_size / sample_size;
    data.num_samples = num_samples;
    data.samples = (short *)malloc(data.subchunk_size);
    for (int i = 0; i < num_samples; i++) {
        char value[sample_size];
        fread(&value, sample_size, 1, file);
        if (sample_size==2) {
            data.samples[i] = short_from_buffer(value);
        } else {
            data.samples[i] = int_from_buffer(value);
        }
    }
}

/* Read the filename given and store all wav file data into the returned wav_file */
wav_file read_file(char* filename) {
    FILE* file = fopen(filename, "rb");
    wav_file wav;
    get_file_header(wav.header, file);
    get_file_format(wav.fmt, file);
    get_file_data(wav.data, file, wav.fmt.block_align);
    fclose(file);
    return wav;
}

/***************************************
 * Functions for writing to a wav file *
 ***************************************/
/* Write an integer in little endian format to the file stream */
void fwriteIntLSB(int data, FILE *file) {
    char array[4];

    array[3] = (unsigned char)((data >> 24) & 0xFF);
    array[2] = (unsigned char)((data >> 16) & 0xFF);
    array[1] = (unsigned char)((data >> 8) & 0xFF);
    array[0] = (unsigned char)(data & 0xFF);
    fwrite((char *)array, 4, 1, file);
}

/* Write a short in little endian format to the file stream */
void fwriteShortLSB(short data, FILE *file) {
    char array[2];

    array[1] = (unsigned char)((data >> 8) & 0xFF);
    array[0] = (unsigned char)(data & 0xFF);
    fwrite((char *)array, 2, 1, file);
}

/* Write the fileheader to an open output filestream */
void write_file_header(file_header header, FILE *file) {
    fwrite(header.chunk_id, 4, 1, file);
    fwriteIntLSB(header.chunk_size, file);
    fwrite(header.format, 4, 1, file);
}

/* Write the file format subchunk to an open output filestream */
void write_file_format(file_fmt fmt, FILE *file) {
    fwrite(fmt.subchunk_id, 4, 1, file);
    fwriteIntLSB(fmt.subchunk_size, file);
    fwriteShortLSB(fmt.audio_format, file);
    fwriteShortLSB(fmt.num_channels, file);
    fwriteIntLSB(fmt.sample_rate, file);
    fwriteIntLSB(fmt.byte_rate, file);
    fwriteShortLSB(fmt.block_align, file);
    fwriteShortLSB(fmt.bits_per_sample, file);
}

/* Write the data subchunk to an open output filestream */
void write_file_data(file_data data, FILE *file, int block_align) {
    fwrite(data.subchunk_id, 4, 1, file);
    fwriteIntLSB(data.subchunk_size, file);
    
    for (int i = 0; i < data.num_samples; i++) {
        if (block_align == 2) {
            fwriteShortLSB(data.samples[i], file);
        } else {
            fwriteIntLSB(data.samples[i], file);
        }
    }
}

/* Write the wav_file struct to a given file */
void write_to_file(wav_file wav, char* filename) {
    FILE *file = fopen(filename, "wb");
    write_file_header(wav.header, file);
    write_file_format(wav.fmt, file);
    write_file_data(wav.data, file, wav.fmt.block_align);
    fclose(file);
}

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

    float x[N];
    float h[M];
    float y[P];

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

    // Create the output wav file using the above data
    return create_wav(samples, P, input);
}

/***************************
 * Miscellaneous functions *
 ***************************/
/* Some potentially helpful print statements for developing help */
void print_id(char buf[]) {
    for (int i = 0; i < 4; i++) {
        printf("%c", buf[i]);
    }
    printf("\n");
}
void print_file_data(wav_file wav) {
    printf("\tChunk ID: ");
    print_id(wav.header.chunk_id);
    printf("\tChunk Size: %d\n", wav.header.chunk_size);
    printf("\tFormat: ");
    print_id(wav.header.format);
    
    printf("\n\tSubchunk ID: ");
    print_id(wav.fmt.subchunk_id);
    printf("\tSubchunk Size: %d\n", wav.fmt.subchunk_size);
    printf("\tAudio Format: %d\n", wav.fmt.audio_format);
    printf("\tNum Channels: %d\n", wav.fmt.num_channels);
    printf("\tSample Rate: %d\n", wav.fmt.sample_rate);
    printf("\tByte Rate: %d\n", wav.fmt.byte_rate);
    printf("\tBlock Align: %d\n", wav.fmt.block_align);
    printf("\tBits Per Sample: %d\n", wav.fmt.bits_per_sample);

    printf("\n\tSubchunk ID: ");
    print_id(wav.data.subchunk_id);
    printf("\tSubchunk Size: %d\n", wav.data.subchunk_size);
    printf("\tSample Count: %d\n", wav.data.num_samples);
}
void exit_if_invalid(char* path) {
    struct stat sb;
    if (stat(path, &sb) != 0) {
        printf("Error: Could not find file: %s\n", path);
        exit(EXIT_FAILURE);
    }
}

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
    printf("INPUT FILE:\n");
    print_file_data(input);

    wav_file ir = read_file(argv[2]);
    printf("\nIMPULSE RESPONSE:\n");
    print_file_data(ir);

    wav_file output = convolve_files(input, ir);
    printf("\nOUTPUT FILE:\n");
    print_file_data(output);

    write_to_file(output, argv[3]);

    free(input.data.samples);
    free(ir.data.samples);
    free(output.data.samples);

    return EXIT_SUCCESS;
}
