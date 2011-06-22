

CPP = g++ -Wall -ansi -g
LIBS = $(shell sdl-config --libs) -lSDL_gfx

all: CellData.o main.o
	$(CPP) $(LIBS) -o sand *.o

%o: %cpp
	$(CPP) -c -o $@ $<

main.o: main.cpp

clean:
	rm *.o sand *~ 2> /dev/zero || true

na: clean all


