typedef struct users usr, *pusr;
typedef struct top_user tu, *ptu;
struct users
{
	char nome[30];
	int indice;
	char fifor[20]; 
	char fifow[20];
	int tempo_inactividade;
	ptu topcs;
	pusr prox;
};

struct top_user
{
	char topico[101];
	ptu prox;
};
