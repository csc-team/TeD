CC              := g++
CFLAGS          := -I/usr/local/include/opencv -I/usr/local/boost_1_49_0 -L/usr/local/lib
OBJECTS         := 
LIBRARIES       := `pkg-config opencv --cflags --libs`

.PHONY: all clean
all:	old test	
	
test:	test.o TextTracker.o TextDetection.o
	$(CC) $(CFLAGS) -o test test.o TextDetection.o TextTracker.o $(LIBRARIES)
old:	tracker.o TextDetection.o
	$(CC)  $(CFLAGS) -o old tracker.o TextDetection.o $(LIBRARIES)
test.o: 	test.cpp TextTracker.h
	$(CC) -c $(CFLAGS) -o test.o test.cpp $(LIBRARIES)
TextTracker.o:	TextTracker.cpp TextTracker.h TextDetection.h
	$(CC) -c $(CFLAGS) -o TextTracker.o TextTracker.cpp $(LIBRARIES)
tracker.o:	tracker.cpp TextDetection.h
	$(CC) -c $(CFLAGS) -o tracker.o tracker.cpp  $(LIBRARIES)
TextDetection.o:	TextDetection.cpp TextDetection.h
	$(CC) -c $(CFLAGS) -o TextDetection.o TextDetection.cpp $(LIBRARIES)
clean:
	rm -f *.o treck *~
