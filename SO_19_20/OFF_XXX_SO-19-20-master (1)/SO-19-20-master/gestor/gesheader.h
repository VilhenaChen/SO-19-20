#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stringutils.c> 
#include "gesmsg.h" /* header responsavel pelas listas de msg, topicos titulos */
#include "gesusr.h" /* Header Responsavel pela lista de users */
#include "gesfuncverif.c"
#include "gescomandos.c"
#include "../stringutils.c"
#include "cli_gest_estruturas.h"

#define FIFO_GES "gest"
#define FIFO_CLIR "clir_%d"
#define FIFO_CLIW "cliw_%d"

#define MAXMSG 10
#define MAXCOM 30
#define N_PROIB 1
