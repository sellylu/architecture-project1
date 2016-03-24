include Makefile.dep

CFLAGS = -g
LDFLAGS = -m32

CC = g++
LD = g++
RM = rm

INCPATH = -I ./ -I-

PROGRAM = simulator

HFILES = 

CFILES = 

C_OFILES = 

OFILES = $(C_OFILES)

$(PROGRAM): $(OFILES)
	$(LD) $(OFILES) $(LDFLAGS) -o $(PROGRAM)

$(C_OFILES): %.o:
	$(CC) $(CFLAGS) -c $<

clean:
	$(RM) -f $(OFILES)

distclean: clean
	$(RM) -f $(PROGRAMS)

