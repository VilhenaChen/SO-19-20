#ifndef UTILS_C
#define UTILS_C
#include "../Comum/estruturas_comunicacao.h"

void get_nome_fifo(char *nome_fifo, char tipo)
{
    switch (tipo){
        case TIPO_FIFO_CLI_W:
            sprintf(nome_fifo, FIFO_CLI_W, FIFO_PASTA, getpid());
            break;
        case TIPO_FIFO_CLI_R:
            sprintf(nome_fifo, FIFO_CLI_R, FIFO_PASTA, getpid());
            break;
        case TIPO_FIFO_GES:
            sprintf(nome_fifo, FIFO_GES, FIFO_PASTA);
            break;
    }
}

void clean_stdin()
{
    char c;
    do {
        c = getchar();
    } while (c != '\n' && c != EOF);
}
#endif