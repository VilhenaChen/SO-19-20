typedef struct cli_gest cg, *pcg;
struct cli_gest
{
	int tipoinfo; /*0-username, 1-Enviar msg, 2- Consultar topicos, 3- Consultar titulos de topico, 4-Consultar mensagem de topico, 5- Subscrever Topico, 6- Cancelar subs topico*/
    char username[30];
    char fifoclienvia[20];
    char fifoclirecebe[20];
    char corpo[1001];
    char topico[101];
    char titulo[101];
    int duracao;

	pcg prox;
};

typedef struct gest_cli gc, *pgc;
struct gest_cli
{
	int tipoinfo; /*0-username, 1-Mensagem aprovada/reprovada, 2- Devolve topicos, 3- Devolve titulos de topico, 4-Devolve mensagem de topico, 5- Resposta, 6- Informacao*/
    char resposta[101];
    char informacao[101];
    char username[30];
    char corpo[1001];
    char topicos[2001];
    char titulos[2001];
	pgc prox;
};