main: main.o
	g++ main.o -o main -Wpedantic -Wall -O3 -lm -lsfml-graphics -lsfml-window -lsfml-system -I /libraries/
main.o: main.cpp
	g++ -c main.cpp
clean:
	rm *.o
	rm main
	clear