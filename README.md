# CPSC 501 - Convolution
*Usage*: convolve <input_file> <IR_file> <output_file>

*Note*: The baseline program that implements linear convolution is stored in the [baseline branch](https://github.com/alexs2112/CPSC501-Convolution/tree/baseline) for historical purposes.

A full report is available at [report.md](report.md).

### Timings:
Linear Convolution:
```
>>> time ./convolve input/FluteDry.wav ir/taj_mahal.wav output/flute-mahal.wav
real    21m49.301s
user    20m56.925s
sys     0m0.710s

>>> gprof convolve profiling/linear-flute.out
```
```
>>> time ./convolve input/GuitarDry.wav ir/large_brite_hall.wav output/out.wav
real    9m4.331s
user    9m0.709s
sys     0m0.154s

>>> gprof convolve profiling/linear-guitar.out
```

FFT Convolution:
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

Further Manual and Compiler-Level Optimizations:
```
>>> time ./convolve input/FluteDry.wav ir/taj_mahal.wav output/out.wav
real    0m2.766s
user    0m2.149s
sys     0m0.154s
```
```
>>> time ./convolve input/GuitarDry.wav ir/large_brite_hall.wav output/out.wav
real    0m1.205s
user    0m0.823s
sys     0m0.106s
```

### Unit Testing:
Unit tests are available through the `test.cpp` file that implements a custom unit testing framework. This can be compiled using `make test` and run as `./test`.
```
>>> ./test

Test Run Complete.
Tests Run: 5
Test Failed: 0
```
