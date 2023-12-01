void fft_convolve(float *x, int N, float *h, int M, double *y, int P);

/****************************
 * DELETE AFTER CLEANING UP *
 ****************************/
#ifndef FILE_STRUCTS_H
#define FILE_STRUCTS_H
#include "./file_structs.h"
#endif
wav_file convolve_files_but_fast(wav_file input, wav_file ir);
