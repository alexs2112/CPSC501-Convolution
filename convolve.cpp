#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <fstream>

using namespace std;

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
    int     num_samples;
    short*  samples;
};

int int_from_buffer(char buf[]) {
    int value =
        ((buf[3] & 0xFF) << 24) |
        ((buf[2] & 0xFF) << 16) |
        ((buf[1] & 0xFF) << 8) |
        (buf[0] & 0xFF);
    return value;
}

short short_from_buffer(char buf[]) {
    short value =
        ((buf[1] & 0xFF) << 8) |
        (buf[0] & 0xFF);
    return value;
}

float float_from_buffer(char buf[], int size) {
    if (size == 2) {
        return
            ((buf[1] & 0xFF) << 8) |
            (buf[0] & 0xFF);
    } else {
        return
            ((buf[3] & 0xFF) << 24) |
            ((buf[2] & 0xFF) << 16) |
            ((buf[1] & 0xFF) << 8) |
            (buf[0] & 0xFF);
    }
}

void get_file_header(file_header &header, ifstream &file) {
    file.read(header.chunk_id, 4);

    char size[4];
    file.read(size, 4);
    header.chunk_size = int_from_buffer(size);

    file.read(header.format, 4);
}

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
        samples[i] = float_from_buffer(value, 2);
    }
    data.samples = samples;
}

void print_id(char buf[]) {
    for (int i = 0; i < 4; i++) {
        printf("%c", buf[i]);
    }
    printf("\n");
}

void print_file_data(file_header header, file_fmt fmt, file_data data) {
    printf("HEADER\n");
    printf("Chunk ID: ");
    print_id(header.chunk_id);
    printf("Chunk Size: %d\n", header.chunk_size);
    printf("Format: ");
    print_id(header.format);
    
    printf("\nFMT\n");
    printf("Subchunk ID: ");
    print_id(fmt.subchunk_id);
    printf("Subchunk Size: %d\n", fmt.subchunk_size);
    printf("Audio Format: %d\n", fmt.audio_format);
    printf("Num Channels: %d\n", fmt.num_channels);
    printf("Sample Rate: %d\n", fmt.sample_rate);
    printf("Byte Rate: %d\n", fmt.byte_rate);
    printf("Block Align: %d\n", fmt.block_align);
    printf("Bits Per Sample: %d\n", fmt.bits_per_sample);

    printf("\nDATA\n");
    printf("Subchunk ID: ");
    print_id(data.subchunk_id);
    printf("Subchunk Size: %d\n", data.subchunk_size);
    printf("Sample Count: %d\n", data.num_samples);
}

int main(int argc, char* argv[]) {
    file_header header;
    file_fmt fmt;
    file_data data;
    if (argc != 4) {
        printf("Usage: convolve <input_file> <IR_file> <output_file>\n");
        exit(EXIT_FAILURE);
    }

    ifstream file = ifstream(argv[1]);
    get_file_header(header, file);
    get_file_format(fmt, file);
    get_file_data(data, file, fmt.block_align);
    print_file_data(header, fmt, data);

    return EXIT_SUCCESS;
}
