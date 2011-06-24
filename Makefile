
CPP = g++ -Wall -ansi -g
LIBS = $(shell sdl-config --libs) -lSDL_gfx



all: sand

sand: sand_objects
	$(CPP) $(LIBS) -o sand *.o

sand_objects: CellData.o main.o common.o CellGrid.o Physics.o main.o


%o: %cpp
	$(CPP) -c -o $@ $<


clean:
	rm *.o sand *~ 2> /dev/zero || true

n: clean
a: all
na: n a

