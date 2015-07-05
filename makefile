CC:= g++
CFLAGS:= `pkg-config --cflags opencv` -g
LDFLAGS:= `pkg-config --libs opencv`

all: mocv_lpr.cpp detect.cpp segment.cpp recognize.cpp lpdetect.cpp
	$(CC) -c mocv_lpr.cpp $(CFLAGS) $(LDFLAGS)
	$(CC) -c detect.cpp $(CFLAGS) $(LDFLAGS)
	$(CC) -c segment.cpp $(CFLAGS) $(LDFLAGS)
	$(CC) -c recognize.cpp $(CFLAGS) $(LDFLAGS)
	$(CC) mocv_lpr.o detect.o segment.o recognize.o lpdetect.cpp -o lpdetect $(CFLAGS) $(LDFLAGS);

detect: mocv_lpr.cpp detect.cpp
	$(CC) -c mocv_lpr.cpp $(CFLAGS) $(LDFLAGS)
	$(CC) mocv_lpr.o detect.cpp -o detect $(CFLAGS) $(LDFLAGS);

segment: segment.cpp
	$(CC) segment.cpp -o segment $(CFLAGS) $(LDFLAGS)

recognize: mocv_lpr.cpp recognize.cpp
	$(CC) -c mocv_lpr.cpp $(CFLAGS) $(LDFLAGS)
	$(CC) mocv_lpr.o recognize.cpp -o recognize $(CFLAGS) $(LDFLAGS);

clean:
	rm -f *.o
	rm -f *.jpg
