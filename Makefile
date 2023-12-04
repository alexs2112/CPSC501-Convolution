convolve: convolve.cpp
	g++ -g -p -o convolve convolve.cpp modules/reader.cpp modules/writer.cpp modules/convolution.cpp modules/fast_fourier.cpp modules/misc.cpp

test: test.cpp
	g++ -g -o test test.cpp modules/convolution.cpp modules/fast_fourier.cpp modules/misc.cpp
