all : main.exe
	
main.exe : main.o Direct3DPlayer.o
	g++ -o main.exe main.o Direct3DPlayer.o -lgdi32 -ld3d9

main.o : main.cpp
	g++ -c main.cpp

Direct3DPlayer.o : Direct3DPlayer.cpp
	g++ -c Direct3DPlayer.cpp

clean : 
	rm *.exe *.o