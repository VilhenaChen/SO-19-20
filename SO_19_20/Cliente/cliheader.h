#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "cli_func_msg.c"
#include "../Comum/estruturas_comunicacao.h"
#include "../Comum/utils.c"

#define INTERVALO_HEARTBEAT 1 // 1 Segundo
#define TEMPO_INACTIVIDADE_GESTOR 10 //10 Segundos

void clean_stdin();
pcg Consulta_L_Titulos(pcg topico);
pcg Consulta_Mensagem(pcg Consulta_Mensagem);
//void Subscrever_Canc_Subs_Topico();
