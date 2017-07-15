#
# Makefile ESQUELETO
#
# OBRIGATÓRIO ter uma regra "all" para geração da biblioteca e de uma
# regra "clean" para remover todos os objetos gerados.
#
# NECESSARIO adaptar este esqueleto de makefile para suas necessidades.
#

CC=gcc
LIB_DIR=./lib/
INC_DIR=./include/
BIN_DIR=./bin/
SRC_DIR=./src/

all: dropboxServer dropboxClient

#libdropbox.a: dropboxClient.o dropboxClient.o
#	ar crs libcthread.a $(BIN_DIR)dropboxClient.o $(BIN_DIR)dropboxClient.o
#	mv libdropbox.a $(LIB_DIR)
dropboxClient:
	$(CC) -g -o dropboxClient $(SRC_DIR)dropboxClient.c $(SRC_DIR)dropboxUtil.c $(SRC_DIR)dropboxSharedSocket.c $(SRC_DIR)dropboxClientCommandHandler.c $(SRC_DIR)dropboxClientUI.c $(SRC_DIR)dropboxSynch.c $(SRC_DIR)dropboxReplicaManager.c -L/usr/local/opt/openssl/lib -I/usr/local/opt/openssl/include -Iinclude -lpthread -lssl -lcrypto -Wall
#	mv ./dropboxClient.o $(BIN_DIR)

dropboxServer:
	$(CC) -g -o dropboxServer $(SRC_DIR)dropboxServer.c $(SRC_DIR)dropboxUtil.c $(SRC_DIR)dropboxSharedSocket.c $(SRC_DIR)dropboxServerCommandHandler.c $(SRC_DIR)dropboxSynch.c $(SRC_DIR)dropboxReplicaManager.c -Iinclude -lpthread -lssl -lcrypto -I/usr/local/opt/openssl/include -L/usr/local/opt/openssl/lib -Wall
#	mv ./dropboxServer.o $(BIN_DIR)
#dropboxClient.o:
#	$(CC) -g -c $(SRC_DIR)dropboxClient.c -Iinclude -Wall
#	mv ./dropboxClient.o $(BIN_DIR)

clean:
	rm -rf dropboxClient dropboxServer
