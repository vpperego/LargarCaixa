
/*
 *	Programa de exemplo de uso da biblioteca cthread
 *
 *	Versão 1.0 - 14/04/2016
 *
 *	Sistemas Operacionais I - www.inf.ufrgs.br
 *
 */

#include "../include/support.h"
#include "../include/cthread.h"
#include <stdio.h>

csem_t mutex;
void* func0(void *arg) {
 	cwait(&mutex);
	printf("Eu sou a func0 fazendo cwait e cedendo a CPU...\n");
	cyield();
	printf("func0 apos ceder a CPU\n");
	csignal(&mutex);
	printf("func0 apos liberar recurso\n");
	return NULL;
}

void* func1(void *arg) {
	printf("Eu sou a func1 solicitando recurso bloqueado e sendo bloqueada \n");
 	cwait(&mutex);
	printf("Sou a func1 apos solicitar recurso do mutex\n");
	return NULL;
}

int main(int argc, char *argv[]) {

	int	id0;
	int i = 999;

	printf("Teste de exclusao mutua (mutex)...\n");	
	csem_init(&mutex, 1);
	id0 = ccreate(func0, (void *)&i);
	ccreate(func1, (void *)&i);
 	cjoin(id0);
	printf("Thread main encerrando o programa...\n");
	return 0;
}

