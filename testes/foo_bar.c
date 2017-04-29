#include <stdio.h>
#include "../include/support.h"
#include "../include/cthread.h"
#define MAX 100
int i = 1;
void* foo(void *arg)
{
	for(;i <MAX;i++)
	{
		if(i%15==0)	
		{
			printf("FOO (i=%d)",i);
			cyield();
		}	
	}
	return NULL;
}
void* bar(void *arg)
{
	for(i++;i <MAX;i++)
	{
		if(i%15==0)	
		{
			printf(" BAR(i=%d)!\n",i);
			cyield();
		}	
	}
	return NULL;
}

int main()
{
	int	id0;
	id0 = ccreate(foo, (void *)&i);
	ccreate(bar, (void *)&i);
	cjoin(id0);
	return 0;
}
