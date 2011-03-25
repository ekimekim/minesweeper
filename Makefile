# Master makefile for general projects

CC="gcc"
CFLAGS="-Wall" "-W"
TOOBJECT="-c"
DEBUG="-g" "-D_DEBUG"
MAIN=minesweeper

all: $(MAIN)

$(MAIN): $(MAIN).c *.h
	$(CC) $(CFLAGS) -o $(MAIN) $(MAIN).c

clean:
	-rm *.o $(MAIN) -f
