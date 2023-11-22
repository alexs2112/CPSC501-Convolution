#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <fstream>

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
void get_file_header(file_header &header, ifstream &file) {
    file.read(header.chunk_id, 4);

    char size[4];
    file.read(size, 4);
    header.chunk_size = int_from_buffer(size);

    file.read(header.format, 4);
}

/* Use file to fill out the provided format subchunk */
void get_file_format(file_fmt &fmt, ifstream &file) {
    file.read(fmt.subchunk_id, 4);

    char size[4];
    file.read(size, 4);
    fmt.subchunk_size = int_from_buffer(size);

    char audio_format[2];
    file.read(audio_format, 2);
    fmt.audio_format = short_from_buffer(audio_format);

    char num_channels[2];
    file.read(num_channels, 2);
    fmt.num_channels = short_from_buffer(num_channels);

    char sample_rate[4];
    file.read(sample_rate, 4);
    fmt.sample_rate = int_from_buffer(sample_rate);

    char byte_rate[4];
    file.read(byte_rate, 4);
    fmt.byte_rate = int_from_buffer(byte_rate);

    char block_align[2];
    file.read(block_align, 2);
    fmt.block_align = short_from_buffer(block_align);

    char bits_per_sample[2];
    file.read(bits_per_sample, 2);
    fmt.bits_per_sample = short_from_buffer(bits_per_sample);
}

/* Use file to fill out the provided data subchunk */
void get_file_data(file_data &data, ifstream &file, int sample_size) {
    file.read(data.subchunk_id, 4);

    char size[4];
    file.read(size, 4);
    data.subchunk_size = int_from_buffer(size);

    int num_samples = data.subchunk_size / sample_size;
    data.num_samples = num_samples;
    short samples[num_samples];
    for (int i = 0; i < num_samples; i++) {
        char value[2];
        file.read(value, 2);
        samples[i] = short_from_buffer(value);
    }
    data.samples = samples;
}

/* Read the filename given and store all wav file data into the returned wav_file */
wav_file read_file(char* filename) {
    ifstream file = ifstream(filename);
    wav_file wav;
    get_file_header(wav.header, file);
    get_file_format(wav.fmt, file);
    get_file_data(wav.data, file, wav.fmt.block_align);
    file.close();
    return wav;
}

/***************************************
 * Functions for writing to a wav file *
 ***************************************/
/* Write an integer in little endian format to the file stream */
void fwriteIntLSB(int data, ofstream &file) {
    char array[4];

    array[3] = (unsigned char)((data >> 24) & 0xFF);
    array[2] = (unsigned char)((data >> 16) & 0xFF);
    array[1] = (unsigned char)((data >> 8) & 0xFF);
    array[0] = (unsigned char)(data & 0xFF);
    file.write((char *)array, 4);
}

/* Write a short in little endian format to the file stream */
void fwriteShortLSB(short data, ofstream &file) {
    char array[2];

    array[1] = (unsigned char)((data >> 8) & 0xFF);
    array[0] = (unsigned char)(data & 0xFF);
    file.write((char *)array, 2);
}

/* Write the fileheader to an open output filestream */
void write_file_header(file_header header, ofstream &file) {
    file.write(header.chunk_id, 4);
    fwriteIntLSB(header.chunk_size, file);
    file.write(header.format, 4);
}

/* Write the file format subchunk to an open output filestream */
void write_file_format(file_fmt fmt, ofstream &file) {
    file.write(fmt.subchunk_id, 4);
    fwriteIntLSB(fmt.subchunk_size, file);
    fwriteShortLSB(fmt.audio_format, file);
    fwriteShortLSB(fmt.num_channels, file);
    fwriteIntLSB(fmt.sample_rate, file);
    fwriteIntLSB(fmt.byte_rate, file);
    fwriteShortLSB(fmt.block_align, file);
    fwriteShortLSB(fmt.bits_per_sample, file);
}

/* Write the data subchunk to an open output filestream */
void write_file_data(file_data data, ofstream &file, int block_align) {
    file.write(data.subchunk_id, 4);
    fwriteIntLSB(data.subchunk_size, file);
    
    for (int i = 0; i < data.num_samples; i++) {
        if (block_align == 2) {
            fwriteShortLSB(data.samples[i], file);
        } else {
            fwriteIntLSB(data.samples[i], file);
        }
    }
}

void write_to_file(wav_file wav, char* filename) {
    ofstream file = ofstream(filename);
    write_file_header(wav.header, file);
    write_file_format(wav.fmt, file);
    write_file_data(wav.data, file, wav.fmt.block_align);
    file.close();
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
    printf("HEADER\n");
    printf("Chunk ID: ");
    print_id(wav.header.chunk_id);
    printf("Chunk Size: %d\n", wav.header.chunk_size);
    printf("Format: ");
    print_id(wav.header.format);
    
    printf("\nFMT\n");
    printf("Subchunk ID: ");
    print_id(wav.fmt.subchunk_id);
    printf("Subchunk Size: %d\n", wav.fmt.subchunk_size);
    printf("Audio Format: %d\n", wav.fmt.audio_format);
    printf("Num Channels: %d\n", wav.fmt.num_channels);
    printf("Sample Rate: %d\n", wav.fmt.sample_rate);
    printf("Byte Rate: %d\n", wav.fmt.byte_rate);
    printf("Block Align: %d\n", wav.fmt.block_align);
    printf("Bits Per Sample: %d\n", wav.fmt.bits_per_sample);

    printf("\nDATA\n");
    printf("Subchunk ID: ");
    print_id(wav.data.subchunk_id);
    printf("Subchunk Size: %d\n", wav.data.subchunk_size);
    printf("Sample Count: %d\n", wav.data.num_samples);
}

/********
 * Main *
 ********/
int main(int argc, char* argv[]) {
    if (argc != 4) {
        printf("Usage: %s <input_file> <IR_file> <output_file>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    wav_file wav = read_file(argv[1]);
    print_file_data(wav);
    write_to_file(wav, argv[3]);

    return EXIT_SUCCESS;
}
