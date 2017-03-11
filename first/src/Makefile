OUTPUT = build

build/program.exe: main.cpp Solver.h test.inc devs.h data.inc
	g++ main.cpp -o build/program.exe -std=c++11 -Wall -pedantic

compile: build/program.exe
	chmod +x build/program.exe

run: compile
	build/program.exe