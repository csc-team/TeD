main: main.o TextDetection.o
	g++ main.o TextDetection.o -o main `pkg-config opencv --cflags --libs` 
main.o: main.cpp TextDetection.h
	gcc -c main.cpp `pkg-config opencv --cflags --libs` 
TextDetection.o: TextDetection.cpp TextDetection.h
	gcc -c TextDetection.cpp `pkg-config opencv --cflags --libs` 
