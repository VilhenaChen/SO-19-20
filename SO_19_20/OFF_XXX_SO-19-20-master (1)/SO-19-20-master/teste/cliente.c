#include "util.h"

int main()
{
	if (access(FIFO_SERV, F_OK) !=0)
	{
		printf("ERRO GESTOR NAO EXISTE\n");
		exit(2);
	}
	printf("OLA");
	exit(1);
}
