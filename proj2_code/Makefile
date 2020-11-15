# Makefile, versao 1
# Sistemas Operativos, DEI/IST/ULisboa 2020-21

CC   = gcc
LD   = gcc
CFLAGS =-Wall -std=gnu99 -I../ -pthread
LDFLAGS=-lm

# A phony target is one that is not really the name of a file
# https://www.gnu.org/software/make/manual/html_node/Phony-Targets.html
.PHONY: all clean run

all: tecnicofs

tecnicofs: fs/state.o fs/operations.o fs/lockedLocks.o main.o timer.o circularBuffer.o 
	$(LD) $(CFLAGS) $(LDFLAGS) -o tecnicofs fs/state.o fs/operations.o fs/lockedLocks.o main.o timer.o circularBuffer.o 

fs/state.o: fs/state.c fs/state.h tecnicofs-api-constants.h
	$(CC) $(CFLAGS) -o fs/state.o -c fs/state.c

fs/operations.o: fs/operations.c fs/operations.h fs/state.h fs/lockedLocks.h tecnicofs-api-constants.h
	$(CC) $(CFLAGS) -o fs/operations.o -c fs/operations.c

fs/lockedLocks.o: fs/lockedLocks.c fs/lockedLocks.h fs/state.c fs/state.h tecnicofs-api-constants.h
	$(CC) $(CFLAGS) -o fs/lockedLocks.o -c fs/lockedLocks.c

timer.o: timer.c
	$(CC) $(CFLAGS) -o timer.o -c timer.c

circularBuffer.o: circularBuffer.c
	$(CC) $(CFLAGS) -o circularBuffer.o -c circularBuffer.c

main.o: main.c fs/operations.h fs/state.h tecnicofs-api-constants.h timer.h circularBuffer.h
	$(CC) $(CFLAGS) -o main.o -c main.c

clean:
	@echo Cleaning...
	rm -f fs/*.o *.o tecnicofs

run: tecnicofs
	./tecnicofs
