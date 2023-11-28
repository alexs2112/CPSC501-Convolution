#include <stdio.h>
#include "../headers/file_structs.h"

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
