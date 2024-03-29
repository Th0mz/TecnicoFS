# Makefile, versao 1
# Sistemas Operativos, DEI/IST/ULisboa 2020-21

CC   = gcc
LD   = gcc
CFLAGS =-Wall -std=gnu99 -I../ -pthread
LDFLAGS=-lm -pthread

# A phony target is one that is not really the name of a file
# https://www.gnu.org/software/make/manual/html_node/Phony-Targets.html
.PHONY: all clean run

all: tecnicofs

tecnicofs: lock_functions.o fs/state.o fs/operations.o main.o timer.o
	$(LD) $(CFLAGS) $(LDFLAGS) -o tecnicofs lock_functions.o fs/state.o fs/operations.o main.o timer.o

lock_functions: lock_functions.c
	$(LD) $(CFLAGS) -o lock_functions.o -c lock_functions.c

fs/state.o: fs/state.c lock_functions.h fs/state.h tecnicofs-api-constants.h
	$(CC) $(CFLAGS) -o fs/state.o -c fs/state.c

fs/operations.o: fs/operations.c fs/operations.h fs/state.h tecnicofs-api-constants.h
	$(CC) $(CFLAGS) -o fs/operations.o -c fs/operations.c

timer.o: timer.c
	$(CC) $(CFLAGS) -o timer.o -c timer.c

main.o: main.c lock_functions.h fs/operations.h fs/state.h tecnicofs-api-constants.h timer.h
	$(CC) $(CFLAGS) -o main.o -c main.c

clean:
	@echo Cleaning...
	rm -f fs/*.o *.o tecnicofs

run: tecnicofs
	./tecnicofs
