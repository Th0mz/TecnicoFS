# Makefile, versao 1
# Sistemas Operativos, DEI/IST/ULisboa 2020-21

# A phony target is one that is not really the name of a file
# https://www.gnu.org/software/make/manual/html_node/Phony-Targets.html
.PHONY: all clean

all: tecnicofs-server tecnicofs-client
clean: tecnicofs-client-clean tecnicofs-server-clean

tecnicofs-server:
	cd ./server/ && make && cd ..

tecnicofs-server-clean:
	cd ./server/ && make clean && cd ..

tecnicofs-client:
	cd ./client/ && make && cd ..

tecnicofs-client-clean:
	cd ./client/ && make clean && cd ..

