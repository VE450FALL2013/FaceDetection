all:
	g++ ocl_facedetection.cpp `pkg-config --cflags --libs opencv` -lcl  -o face -O3
clean:
	rm face
