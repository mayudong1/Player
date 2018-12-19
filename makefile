CC=g++
CFLAG=
LDFLAG=-lavformat -lavcodec -lglfw -framework opengl

player : player.o
	$(CC) -o a.out player.o $(LDFLAG)

player.o : player.cpp
	$(CC) -c player.cpp

all : main.o
	$(CC) -o a.out main.o $(LDFLAG)

main.o : main.cpp
	$(CC) -c main.cpp

.PHONY: clean
clean:
	rm a.out *.o