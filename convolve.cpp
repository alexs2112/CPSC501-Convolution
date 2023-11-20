#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <fstream>

using namespace std;

struct file_header{
    char    chunk_id[4];
    int     chunk_size;
    char    format[4];
};

struct file_fmt{
    char    subchunk_id[4];
    int     subchunk_size;
    short   audio_format;
    short   num_channels;
    int     sample_rate;
    int     byte_rate;
    short   block_align;
    short   bits_per_sample;
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

void print_file_data(file_header &header, file_fmt &fmt) {
    // For some reason this adds some extra stuff when printing id and
    // format, even though the length of each is 4
    printf("HEADER\n");
    printf("Chunk ID: %s\n", header.chunk_id);
    printf("Chunk Size: %d\n", header.chunk_size);
    printf("Format: %s\n", header.format);
    
    printf("\nFMT\n");
    printf("Subchunk ID: %s\n", fmt.subchunk_id);
    printf("Subchunk Size: %d\n", fmt.subchunk_size);
    printf("Audio Format: %d\n", fmt.audio_format);
    printf("Num Channels: %d\n", fmt.num_channels);
    printf("Sample Rate: %d\n", fmt.sample_rate);
    printf("Byte Rate: %d\n", fmt.byte_rate);
    printf("Block Align: %d\n", fmt.block_align);
    printf("Bits Per Sample: %d\n", fmt.bits_per_sample);
}

int main(int argc, char* argv[]) {
    file_header header;
    file_fmt fmt;
    if (argc != 4) {
        printf("Usage: convolve <input_file> <IR_file> <output_file>\n");
        exit(EXIT_FAILURE);
    }

    ifstream file = ifstream(argv[1]);
    get_file_header(header, file);
    get_file_format(fmt, file);
    print_file_data(header, fmt);

    return EXIT_SUCCESS;
}
