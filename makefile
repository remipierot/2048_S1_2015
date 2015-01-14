CC = gcc
OPTIONS = -Wall
LIBRARY = -lm
PROG = 2048
OBJS = main.o saisieM.o terminalCouleur.o

all:: $(PROG) clean

$(PROG): $(OBJS)
	$(CC) $(OPTIONS) $(OBJS) -o $(PROG) $(LIBRARY)

%.o: %.c
	$(CC) $(OPTIONS) -c $<

clean::
	rm -rf *.o
	rm -rf *~
	clear
