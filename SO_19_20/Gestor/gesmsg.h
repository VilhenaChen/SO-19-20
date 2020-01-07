typedef struct mensagem msg, *pmsg;
typedef struct topicos top, *ptop;
struct mensagem
{
	char topico[50];
	char titulo[50];
	char corpo[1000];
	int duracao;
	pmsg prox;
};


struct topicos
{
	char topico[50];
	pmsg lmensagem;
	ptop prox;
};
