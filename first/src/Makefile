OUTPUT = build

build/program.exe: main.cpp Solver.h test.inc devs.h data.inc
	g++ main.cpp  -o build/program.exe -std=c++11 -Wall -pedantic -D __VALKOVIC__ -pthread

build/release.exe: main.cpp Solver.h test.inc devs.h data.inc
	g++ main.cpp  -o build/release.exe -std=c++11 -Wall -pedantic -pthread

compile: build/program.exe
	chmod +x build/program.exe

compileRelease: build/release.exe
	chmod +x build/release.exe

run: compile
	build/program.exe

runRelease: compileRelease
	build/release.exe