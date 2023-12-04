convolve: convolve.cpp
	g++ -O3 -o convolve convolve.cpp modules/reader.cpp modules/writer.cpp modules/convolution.cpp modules/fast_fourier.cpp modules/misc.cpp

test: test.cpp
	g++ -g -O3 -o test test.cpp modules/convolution.cpp modules/fast_fourier.cpp modules/misc.cpp
