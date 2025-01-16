all: main.exe

main.exe: func.o main.o
	g++ func.o main.o -o main.exe

func.o: func.cpp
	g++ -c func.cpp -o func.o

main.o: main.cpp
	g++ -c main.cpp -o main.o

