CC	 	= gcc
LD	 	= gcc
CFLAGS	 	= -Wall -g

LDFLAGS	 	= 
DEFS 	 	=

all:	population  

population: main.o PacWar.o sort.o
	$(CC) $(DEFS) $(CFLAGS) $(LIB) -o population main.o PacWar.o sort.o
 
clean:
	rm -f *.o
	rm -f *~
	rm -f core
	rm -f PacWar.o
	rm -f population
	rm -f main.o
	rm -f sort.o