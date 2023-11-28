#include <stdio.h>
#include <stdlib.h>
#include "../headers/file_structs.h"

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
