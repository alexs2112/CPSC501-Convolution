#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <sys/stat.h>
#include "../headers/file_structs.h"

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

short double_to_short(double i) {
    short s;
    if (i < 0) { s = rint(i * 32768.0); }
    else { s = rint(i * 32767.0); }
    return s;
}
