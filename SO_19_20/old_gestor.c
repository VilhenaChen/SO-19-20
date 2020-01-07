/* Gestor.c */


#include "Gestor/gesheader.h"

int main(int argc, char** argv)
{
	char input[MAXCOM+1];
	char comando[20];
	char argumento[20];
	int verifica_comando;
	int filtro=1;
	int infp,outfp;
	char buff[128];
	char lixo;
	int n_p_encontradas;
	char forb_words[20];
	int num_proib;
	//named pipes
	int sf;
	char nome [20];
	ptop list_topico=NULL;
	list_topico=malloc(sizeof(top));
	strcpy(list_topico->topico,"topzao");
	list_topico->lmensagem=malloc(sizeof(msg));
        strcpy(list_topico->lmensagem->topico,"topzao");
        strcpy(list_topico->lmensagem->titulo,"comidaaaa");
        strcpy(list_topico->lmensagem->corpo,"batata frita em cima da mesa com melao");
	list_topico->lmensagem->duracao = 20;
	list_topico->lmensagem->prox=NULL;
	list_topico->prox=NULL;
	//variaveis de ambiente
	if(getenv("WORDSNOT") == NULL)
		strcpy(forb_words,"fwords.txt");
	else
		strcpy(forb_words,getenv("WORDSNOT"));
	if(lanca_verifica_mensagem("Verificador",forb_words,&infp,&outfp) <= 0)
	{
		printf("\nFalha em lancar o verificador!\n");
		fflush(stdout);
	}
	if(access(FIFO_GES, F_OK) !=0)
	{
		mkfifo(FIFO_GES, 0666);
		printf("Inicio!");
		fflush(stdout);

		memset(buff, 0x0, sizeof(buff)); //limpa o buffer
		printf("corpo msg  %s", list_topico->lmensagem->corpo);
		fflush(stdout);
		write(infp,list_topico->lmensagem->corpo,strlen(list_topico->lmensagem->corpo));
		write(infp, "\n##MSGEND##\n",12);

		read(outfp, buff, 128);
		printf(" buffer %s",buff);
		fflush(stdout);
		if(strcmp(buff,"ERROR-1\n")==0||strcmp(buff,"ERROR-2\n")==0||strcmp(buff,"ERROR-3\n")==0)
		{
			printf("Falha ao verificar a mensagem %s", buff);
			fflush(stdout);
		}
		else
		{
			if(sscanf(buff,"%d%c",&n_p_encontradas,&lixo) != 2)
			{
				printf("Falha ao verificar mensagem! 2");
				fflush(stdout);
			}
			else
			{
				if(getenv("MAXNOT")==NULL)
					num_proib=N_PROIB;
				else
				{
					sscanf(getenv("MAXPROIBIDAS"),"%d",&num_proib);
				}
				if(n_p_encontradas < num_proib)
				{
					printf("A mensagem foi aceite pelo verificador!");
					fflush(stdout);
				}
				else
				{
					printf("A mensagem nao foi aceite pelo verificador!");
					fflush(stdout);
				}
			}
		}
		sleep(1);
		close(infp);
		do
		{
			verifica_comando=0;
			printf("\n Formato: comando opcoes-ou-argumentos-do-comando\n");
			fflush(stdout);
			printf("\n Comando ->");
			fflush(stdout);
			fgets(input,MAXCOM,stdin);
			if(sscanf(input,"%s %s",comando,argumento)!=2)
				argumento[0]='\0';
			verifica_comando=comandos_servidor(comando,argumento,&filtro);
			if(verifica_comando==1 && strcmp(comando,"shutdown") != 0)
			{
				printf("COMANDO INVALIDO\n Por favor insira um novo comando!");
				fflush(stdout);
			}
		}while(strcmp(comando,"shutdown")!=0);
		write(infp,"SIGUSR2\n",8);
		printf("\nO verificador foi desligado! O programa vai encerrar!\n");
		fflush(stdout);
		free(list_topico->lmensagem);
		free(list_topico);
		unlink(FIFO_GES);
		return 0;
	}
	else
	{
		printf("[ERRO] O GESTOR JA ESTA ATIVO");
		fflush(stdout);
		return 1;
	}
}
