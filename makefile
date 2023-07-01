main: main.o
	g++ main.o -o main -Wpedantic -Wall -O3 -lm -lsfml-graphics -lsfml-window -lsfml-system  -I /libraries/
threaded: threaded.o
	g++ threaded.o -o threaded -Wpedantic -Wall -O3 -lm -lsfml-graphics -lsfml-window -lsfml-system -lpthread -I /libraries/
both: threaded main
main.o: main.cpp
	g++ -c main.cpp
clean:
	rm -f *.o
	rm -f main
	rm -f threaded
	clear
