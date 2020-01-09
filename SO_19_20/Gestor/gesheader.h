#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "gesmsg.h" /* header responsavel pelas listas de msg, topicos titulos */
#include "gesusr.h" /* Header Responsavel pela lista de users */
#include "gesfuncverif.c"
#include "gescomandos.c"
#include "ges_func_msg.c"
#include "../Comum/estruturas_comunicacao.h"
#include "../Comum/utils.c"

#define INTERVALO_HEARTBEAT 1 // 1 Segundo
#define TEMPO_INATIVIDADE_CLIENTE 10 // 10 segundos

// Defs para o verificador
#define PROGRAMA_VERIFICADOR "Bin/Verificador"
#define DEFAULT_FORB_WORDS "Verificador/fwords.txt"
#define N_PROIB 3
#define MAXCOMANDO 31
#define FILTRO_LIGADO 1
#define FILTRO_DESLIGADO 0


