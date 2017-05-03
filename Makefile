#
# Makefile ESQUELETO
#
# OBRIGATÓRIO ter uma regra "all" para geração da biblioteca e de uma
# regra "clean" para remover todos os objetos gerados.
#
# NECESSARIO adaptar este esqueleto de makefile para suas necessidades.
#  1. Cuidado com a regra "clean" para não apagar o "fila2.o"
#
# OBSERVAR que as variáveis de ambiente consideram que o Makefile está no diretótio "cthread"
#

CC=gcc
LIB_DIR=./lib/
INC_DIR=./include/
BIN_DIR=./bin/
SRC_DIR=./src/

all: libdropbox.a

libdropbox.a: dropboxClient.o dropboxServer.o
	ar crs libcthread.a $(BIN_DIR)dropboxClient.o $(BIN_DIR)dropboxServer.o
	mv libdropbox.a $(LIB_DIR)
dropboxClient.o:
	$(CC) -g -c $(SRC_DIR)dropboxClient.c -Iinclude -Wall
	mv ./dropboxClient.o $(BIN_DIR)
dropboxServer.o:
	$(CC) -g -c $(SRC_DIR)dropboxServer.c -Iinclude -Wall
	mv ./dropboxServer.o $(BIN_DIR)


clean:
	rm -rf $(LIB_DIR)/*.a $(SRC_DIR)/*~ $(INC_DIR)/*~ $(BIN_DIR)/cdata.o $(BIN_DIR)/cthread.o *~
