CXX = g++
LD = g++
RM = rm
CP = install

LDFLAGS = `root-config --libs` -g -fopenmp
CFLAGS = `root-config --cflags` -g

all: multiplot.o
	$(LD) $(LDFLAGS) -o prog multiplot.o

multiplot.o: multiplot.cxx
	$(CXX) $(CFLAGS) -c -o multiplot.o multiplot.cxx

install: prog
	sudo $(CP) prog /usr/local/bin/multiplotter

clean:
	$(RM) *.o prog
