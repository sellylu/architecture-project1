CC = g++

PROGRAM = single_cycle
CFILES = simulator.cpp instruction.cpp
HFILES = instruction.h
OFILES = simulator.o instruction.o

$(PROGRAM): $(OFILES)
	$(CC) -o $(PROGRAM) $(OFILES)
simulator.o: simulator.cpp instruction.o
	$(CC) -g -c simulator.cpp
instruction.o: instruction.cpp instruction.h
	$(CC) -g -c instruction.cpp

clean:
	rm -f $(OFILES) $(PROGRAM)

