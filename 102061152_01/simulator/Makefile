CC = g++

PROGRAM = single_cycle

OFILES = simulator.o instruction.o

all: $(PROGRAM)

$(PROGRAM): $(OFILES)
	$(CC) -o $(PROGRAM) $(OFILES)
simulator.o: simulator.cpp instruction.o
	$(CC) -c simulator.cpp
instructor.o: instruction.cpp instruction.h
	$(CC) -c instruction.cpp

clean:
	rm -f $(OFILES) $(PROGRAM)

