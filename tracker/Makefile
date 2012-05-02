CC              := g++
CFLAGS          := -I/usr/local/include/opencv -I/usr/local/boost_1_49_0 -L/usr/local/lib
OBJECTS         := 
LIBRARIES       := `pkg-config opencv --cflags --libs`

.PHONY: all clean
	


main:	tracker.o TextDetection.o
	$(CC)  $(CFLAGS) -o main tracker.o TextDetection.o $(LIBRARIES)
tracker.o:	tracker.cpp TextDetection.h
	$(CC) -c $(CFLAGS) -o tracker.o tracker.cpp  $(LIBRARIES)
TextDetection.o:	TextDetection.cpp TextDetection.h
	$(CC) -c $(CFLAGS) -o TextDetection.o TextDetection.cpp $(LIBRARIES)
clean:
	rm -f *.o treck *~
