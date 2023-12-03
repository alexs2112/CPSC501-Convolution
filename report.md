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
