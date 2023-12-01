#include <stdio.h>
#include <stdlib.h>

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

    wav_file output = convolve_files_but_fast(input, ir);
    printf("\nOUTPUT FILE:\n");
    print_file_data(output);

    write_to_file(output, argv[3]);

    free(input.data.samples);
    free(ir.data.samples);
    free(output.data.samples);

    return EXIT_SUCCESS;
}
