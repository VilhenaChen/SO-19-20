#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "clifuncmenu.c"
#include "../stringutils.c"
#include "cli_gest_estruturas.h"

#define FIFO_GES "gest"
#define FIFO_CLIR "clir_%d"
#define FIFO_CLIW "cliw_%d"

void clean_stdin();
pcg Consulta_L_Titulos(pcg topico);
pcg Consulta_Mensagem(pcg Consulta_Mensagem);
void Subscrever_Canc_Subs_Topico();
