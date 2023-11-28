convolve: convolve.cpp
	g++ -g -o convolve convolve.cpp modules/reader.cpp modules/writer.cpp modules/convolution.cpp modules/misc.cpp
