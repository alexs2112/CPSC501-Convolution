# CPSC 501 - Assignment 4, Optimization
Alex Stevenson - 30073617

Github Repository: https://github.com/alexs2112/CPSC501-Convolution

### Baseline Program
 - Initial version where convolution is implemented directly on the time domain in a linear matter.
 - The code for this version is stored in the [baseline branch](https://github.com/alexs2112/CPSC501-Convolution/tree/baseline) of the repository.

**Run Time Performance**:

Length of `FluteDry.wav`: 60 seconds

Length of `tah_mahal.wav`: 3 seconds
```
>>> time ./convolve input/FluteDry.wav ir/taj_mahal.wav output/out.wav
real    21m49.301s
user    20m56.925s
sys     0m0.710s
>>> gprof convolve profiling/linear-flute.out
```

Length of `GuitarDry.wav`: 30 seconds

Length of `large_brite_hall.wav`: 2 seconds
 - There are 2 channels for this wav file, however the baseline program still works by treating it as a single channel.
```
>>> time ./convolve input/GuitarDry.wav ir/large_brite_hall.wav output/out.wav
real    9m4.331s
user    9m0.709s
sys     0m0.154s
>>> gprof convolve profiling/linear-guitar.out
```

### Algorithm Based Optimization:
 - Utilizing the FFT algorithm to re-implement the convolution using the frequency domain.
 - The code for this version before any further optimizations is stored in the [fft branch](https://github.com/alexs2112/CPSC501-Convolution/tree/fft) of the repository.
 - As you can see with the base run time performance below, the algorithm based optimization increased speed of the program by 145 to 185 times.

**Run Time Performance**:
```
>>> time ./convolve input/FluteDry.wav ir/taj_mahal.wav output/out.wav
real    0m9.000s
user    0m8.394s
sys     0m0.130s
>>> gprof convolve profiling/fft-flute.out
```
```
>>> time ./convolve input/GuitarDry.wav ir/large_brite_hall.wav output/out.wav
real    0m2.947s
user    0m2.562s
sys     0m0.075s
>>> gprof convolve profiling/fft-guitar.out
```

**Regression Testing**:
 - Audio files produced from FFT convolution are the same as the ones produced by linear convolution.

### Manual Code Tuning #1:
 - The `complex_multiply` function very consistently takes the most time as a function call, taking 60% of the total processing time of the program. This is twice as much as the next largest function.
```
  %   cumulative   self              self     total           
 time   seconds   seconds    calls   s/call   s/call  name    
 59.93      4.92     4.92                             complex_multiply(double*, double*, double*, int)
 30.21      7.40     2.48        2     1.24     1.24  fft_convolution(float*, int, float*, int, double*, int)
  8.04      8.06     0.66                             zero_padding(float*, int, double*, int)
```
 - As this complex multiplication is done linearly on rather large inputs, it takes a long time. This can be multithreaded to do the full complex multiplication in parallel across many threads.
 - This code tuning creates a new struct to be used as a single parameter, instead of a list of parameters. It then delegates segments of the input arrays across several threads. Each of those threads process their segments of arrays and enters their result into their segment of the output.

**Commit**: [ff275d26132e407ef5ddb82fb4d211b12c2e9d62](https://github.com/alexs2112/CPSC501-Convolution/commit/ff275d26132e407ef5ddb82fb4d211b12c2e9d62)

**Code Changes**:
```c
// Allow for 512 threads. This could realistically be larger as the input size is huge
#define COMPLEX_THREADS     512

// Input struct
struct complex_param {
    double *x;
    double *h;
    double *output;
    int size;
};

// Perform the complex multiplication on a delegated segment of input arrays
void *complex_multiply(void *v) {
    complex_param p = ((complex_param *)v)[0];
    for (int k = 0; k < p.size; k += 2) {
        p.output[k] = p.x[k] * p.h[k] - p.x[k+1] * p.h[k+1];
        p.output[k+1] = p.x[k+1] * p.h[k] + p.x[k] * p.h[k+1];
    }
    return 0;
}

// Break the input arrays into several chunks, give each thread a chunk to process
void multithread_multiply(double *x, double *h, double *output, int size) {
    pthread_t ids[COMPLEX_THREADS];
    int chunk = size / COMPLEX_THREADS;
    int i;
    for (i = 0; i < COMPLEX_THREADS; i++) {
        complex_param p;
        p.x = &x[i * chunk];
        p.h = &h[i * chunk];
        p.output = &output[i * chunk];
        p.size = chunk;
        pthread_create(&ids[i], NULL, complex_multiply, (void *)&p);
    }
    for (i = 0; i < COMPLEX_THREADS; i++) {
        pthread_join(ids[i], NULL);
    }
}
```

**Run Time Performance**:
```
>>> time ./convolve input/FluteDry.wav ir/taj_mahal.wav output/out.wav
real    0m6.662s
user    0m5.949s
sys     0m0.192s
>>> gprof convolve profiling/flute-manual-1.out
  %   cumulative   self              self     total           
 time   seconds   seconds    calls   s/call   s/call  name    
 94.22      5.38     5.38        3     1.79     1.79  four1(double*, int, int)
  1.75      5.48     0.10        2     0.05     0.05  zero_padding(float*, int, double*, int)
  1.75      5.58     0.10                             complex_multiply(void*)
```
```
>>> time ./convolve input/GuitarDry.wav ir/large_brite_hall.wav output/out.wav
real    0m2.812s
user    0m2.397s
sys     0m0.134s
>>> gprof convolve profiling/guitar-manual-1.out
  %   cumulative   self              self     total           
 time   seconds   seconds    calls   s/call   s/call  name    
 93.30      2.09     2.09        3     0.70     0.70  four1(double*, int, int)
  3.12      2.16     0.07                             complex_multiply(void*)
  2.23      2.21     0.05        2     0.03     0.03  zero_padding(float*, int, double*, int)
```
 - As you can see with the above profiling results, the new `complex_multiply` function takes a fraction of the time that it used to. 1.75-3% of the total program runtime instead of the previous result of 60%

**Regression Testing**:
 - This change added an additional test for complex multiplication.
 - This test also broke previous unit tests as multiplication does not happen if the number of threads is greater than the size of the input arrays. This has since been fixed.
 - Output files from manual regression tests on convolution are the same as before the change.

### Manual Code Tuning #2:
 - Zero padding and converting the input samples to complex arrays have been taking a relatively long time after previous optimizations. Some profiling taken from the result of the previous optimization:
```
>>> gprof convolve profiling/flute-manual-1.out
  %   cumulative   self              self     total           
 time   seconds   seconds    calls   s/call   s/call  name    
 94.22      5.38     5.38        3     1.79     1.79  four1(double*, int, int)
  1.75      5.48     0.10        2     0.05     0.05  zero_padding(float*, int, double*, int)

>>> gprof convolve profiling/guitar-manual-1.out
  %   cumulative   self              self     total           
 time   seconds   seconds    calls   s/call   s/call  name    
 93.30      2.09     2.09        3     0.70     0.70  four1(double*, int, int)
  3.12      2.16     0.07                             complex_multiply(void*)
  2.23      2.21     0.05        2     0.03     0.03  zero_padding(float*, int, double*, int)
```
 - This is because the `zero_padding` function iterates over the `output` array twice, first it zeroes the entire array, and then it copies values from the input array to it.
 - This can be optimized by only going over the array once by zeroing the imaginary part of the array at the same time as copying values from the input array.
    - *Note*: If the output array size is greater than twice the length of the input array then the remainder will still need to be zeroed. This is easy to accomplish by starting another `for` loop at the index of `2 * input_length + 1`.

**Commit**: [95c75c13fa76d35a41940a4d9794334dfae4f208](https://github.com/alexs2112/CPSC501-Convolution/commit/95c75c13fa76d35a41940a4d9794334dfae4f208)

**Code Changes**:
```c
void zero_padding(float *signal, int input_size, double *output, int output_size) {
    int i;
    for (i = 0; i < input_size; i++) {
        output[i*2] = (double)signal[i];
        output[i*2 + 1] = 0.0;
    }
    for (i = input_size * 2 + 1; i < output_size; i++) {
        output[i] = 0.0;
    }
}
```

**Run Time Performance**:
```
>>> time ./convolve input/FluteDry.wav ir/taj_mahal.wav output/out.wav
real    0m6.031s
user    0m5.411s
sys     0m0.215s
>>> gprof convolve profiling/flute-manual-2.out
  %   cumulative   self              self     total           
 time   seconds   seconds    calls   s/call   s/call  name    
 92.47      4.79     4.79        3     1.60     1.60  four1(double*, int, int)
  2.70      4.93     0.14                             complex_multiply(void*)
  2.32      5.05     0.12        2     0.06     0.06  zero_padding(float*, int, double*, int)
```
```
>>> time ./convolve input/GuitarDry.wav ir/large_brite_hall.wav output/out.wav
real    0m2.611s
user    0m2.244s
sys     0m0.139s
>>> gprof convolve profiling/guitar-manual-2.out
  %   cumulative   self              self     total           
 time   seconds   seconds    calls   s/call   s/call  name    
 90.82      1.88     1.88        3     0.63     0.63  four1(double*, int, int)
  4.83      1.98     0.10                             complex_multiply(void*)
  1.93      2.02     0.04        2     0.02     0.02  zero_padding(float*, int, double*, int)
```
 - The time it takes for the program to complete has dropped by a reasonable amount.
 - The relative time it takes for `zero_padding` to finish compared to the other functions of the program has also been dropped by a reasonable amount.

**Regression Testing**:
 - A minor bug with the `for` loop values was caught by the existing unit tests.
 - Manual convolution testing is successful and has expected results.
