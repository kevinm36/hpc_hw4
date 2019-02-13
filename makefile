all:
	g++-8 -o kmeans.exe Jpegfile.cpp hpc_hw4_parallel.cpp JpegLib/libjpeg.a -fopenmp
