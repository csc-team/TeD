CC              := g++
CFLAGS          := -I/usr/local/include/opencv -I/usr/local/boost_1_49_0 -L/usr/local/lib
OBJECTS         := 
LIBRARIES       := `pkg-config opencv --cflags --libs`

.PHONY: all clean
	

all: test test2 treck laplace treck12 show

show:	showimage.cpp TextDetection.h TextDetection.cpp
	$(CC) -g $(CFLAGS) -o show showimage.cpp TextDetection.cpp $(LIBRARIES)

treck12:treck12.cpp TextDetection.h TextDetection.cpp
	$(CC) -g $(CFLAGS) -o treck12 treck12.cpp TextDetection.cpp $(LIBRARIES)

test2: test2.cpp
	$(CC) $(CFLAGS) -o test2 test2.cpp $(LIBRARIES)
	
treck: treck.cpp
	$(CC) $(CFLAGS) -g -o treck treck.cpp $(LIBRARIES)
test: test.cpp 
	$(CC) $(CFLAGS) -o test test.cpp $(LIBRARIES)
laplace: laplace.cpp
	$(CC) $(CFLAGS) -o laplace laplace.cpp $(LIBRARIES)
clean:
	rm -f *.o treck *~
