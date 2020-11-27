# Makefile, versao 1
# Sistemas Operativos, DEI/IST/ULisboa 2020-21

MAKE = make
CD = cd

# A phony target is one that is not really the name of a file
# https://www.gnu.org/software/make/manual/html_node/Phony-Targets.html
.PHONY: all clean

all: tecnicofs-server tecnicofs-client
clean: tecnicofs-client-clean tecnicofs-server-clean

tecnicofs-server:
    @echo
    @echo Compiling Server Code : 
    @$(CD) ./server/ && $(MAKE) --no-print-directory && $(CD) ..

tecnicofs-server-clean:
    @echo
    @echo Server : 
    @$(CD) ./server/ && $(MAKE) --no-print-directory clean && $(CD) ..

tecnicofs-client:
    @echo
    @echo Compiling Client Code : 
    @$(CD) ./client/ && $(MAKE) --no-print-directory && $(CD) ..

tecnicofs-client-clean:
    @echo
    @echo Client : 
    @$(CD) ./client/ && $(MAKE) clean --no-print-directory && $(CD) ..

