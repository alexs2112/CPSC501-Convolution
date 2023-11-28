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
