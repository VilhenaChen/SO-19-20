#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>

#define FIFO_SERV "canal"
#define FIFO_CLI "cli%d"

typedef struct 
{
	char nome[20];
	int idade;
}INFO;
