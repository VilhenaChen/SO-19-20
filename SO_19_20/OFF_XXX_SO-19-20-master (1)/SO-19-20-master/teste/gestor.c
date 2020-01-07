#include "util.h"

int main ()
{
	int fd;
	printf("OLA\n");
	if (access(FIFO_SERV, F_OK) !=0)
	{
		mkfifo(FIFO_SERV, 0666);
		sleep(20);
		unlink (FIFO_SERV);
		return 0;
	}
	else
	{
		printf("ERRO");
		return 1;
	}
}
