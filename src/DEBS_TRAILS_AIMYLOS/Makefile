#call:   make clean    to clean if needed


all: average popularity

average:
	g++ main_avg.cpp DEBS_IO_BUFFER.cpp -I . -o average -lpthread -O3 -s

popularity: 
	g++ main_popul.cpp DEBS_IO_BUFFER.cpp miniBINheap.cpp -I . -o popularity -lpthread -O3 -s

clean: 
	rm -rf average popularity

