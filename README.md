# CPSC 501 - Convolution
*Usage*: convolve <input_file> <IR_file> <output_file>

*Note*: The baseline program that implements linear convolution is stored in the [baseline branch](https://github.com/alexs2112/CPSC501-Convolution/tree/baseline) for historical purposes.

### Timings:
Linear convolution:
```
>>> time ./convolve input/GuitarDry.wav ir/large_brite_hall.wav output/out.wav
real    9m4.331s
user    9m0.709s
sys     0m0.154s

>>> gprof convolve linear-guitar.out
```
```
>>> time ./convolve input/FluteDry.wav ir/taj_mahal.wav output/flute-mahal.wav
real    21m49.301s
user    20m56.925s
sys     0m0.710s

>>> gprof convolve linear-flute.out
```

### Unit Testing:
Unit tests are available through the `test.cpp` file. This can be compiled using `make test` and run as `./test`.
