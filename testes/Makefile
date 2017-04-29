#
# Makefile para programas de testes
# 

CC=gcc
LIB_DIR=../lib

all: exclusao_mutua foo_bar

exclusao_mutua: exclusao_mutua.c $(LIB_DIR)/libcthread.a
	$(CC) -o exclusao_mutua exclusao_mutua.c -L$(LIB_DIR) -lcthread -Wall

foo_bar: foo_bar.c $(LIB_DIR)/libcthread.a
	$(CC) -o foo_bar foo_bar.c -L$(LIB_DIR) -lcthread -Wall

clean:
	rm -rf exclusao_mutua  foo_bar *~




