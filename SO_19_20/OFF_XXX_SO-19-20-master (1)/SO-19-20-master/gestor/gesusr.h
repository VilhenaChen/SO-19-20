typedef struct users usr, *pusr;
typedef struct top_user tu, *ptu;
struct users
{
	char nome;
	int indice;
	char fifor[20]; 
	char fifow[20];
	ptu topcs;
	pusr prox;
};

struct top_user
{
	char topico[50];
	ptu prox;
};
