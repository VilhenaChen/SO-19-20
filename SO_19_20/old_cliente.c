/* cliente.c */

#include "cli/cliheader.h"


int main(int argc, char** argv)
{
	int op=0;
	char username[30];
	char clientfifor[20]; // r-> read w->write
	char clientfifow[20];
	char usrnbuff[50];		
	char msgbuff[2240];
	char topbuffout [101];
	char titlebuffout[101];
	char corpbuffout[2001];
	int fd_fifoclir,fd_fifocliw,fd_fifoges;
	pgc read_struct;
	pcg write_struct;
	if (access(FIFO_GES, F_OK) != 0)
	{
		printf("\n[ERRO] O GESTOR NAO ESTA ATIVO!!!\n");
		return 1;
	}
	sprintf(clientfifor, FIFO_CLIR , getpid());
	sprintf(clientfifow, FIFO_CLIW , getpid());
	//Cria os fifos do cliente
	mkfifo(clientfifor, 0600);
	mkfifo(clientfifow, 0600);
	printf("\nIntroduza nome de utilizador:");
	scanf("%s",write_struct->username);
	write_struct->tipoinfo=0;
	strcpy(write_struct->fifoclienvia,clientfifor);
	strcpy(write_struct->fifoclirecebe,clientfifow);
	//Verifica o Username
	fd_fifoges = open (FIFO_GES, O_WRONLY);
	if (fd_fifoges == -1)
	{
		printf("[ERRO] Nao foi possivel comunicar com o Gestor!!!");
		return 1;
	}
	write(fd_fifoges,write_struct,sizeof(write_struct));
	close(fd_fifoges);
	fd_fifoclir = open(clientfifor, O_RDONLY);
	if (fd_fifoclir == -1)
	{
		printf("[ERRO] Nao foi possivel comunicar com o Gestor!!!");
		return 1;
	}
	read(fd_fifoclir, read_struct, sizeof(read_struct));

	close(fd_fifoclir);

	fflush(stdin);
	do
	{
		printf("\nMENU\n");
		printf("\nOpcao 1 - Escrever Nova Mensagem");
		printf("\nOpcao 2 - Consultar a lista de topicos atualmente existente");
		printf("\nOpcao 3 - Consultar a lista de titulos de mensagens de um determinado topico");
		printf("\nOpcao 4 - Consultar uma mensagem de um topico");
		printf("\nOpcao 5 - Subscrever / Cancelar subscricao de um topico");
		printf("\nOpcao 6 - Exit");
		printf("\nInsira a opcao pretendida: ");
		scanf("%d",&op);
		switch (op)
		{
			case 1:
				fd_fifocliw = open (clientfifow, O_WRONLY);
				if (fd_fifocliw == -1)
				{
					printf("[ERRO] Nao foi possivel comunicar com o Gestor!!!");
					return 1;
				}
				write_struct= Nova_MSG(write_struct);
				strcpy(write_struct->username,username);
				write_struct->tipoinfo=1;
				write(fd_fifocliw,write_struct,strlen(write_struct));
				close(fd_fifocliw);
				fd_fifoclir = open (clientfifow, O_RDONLY);
				if (fd_fifoclir == -1)
				{
					printf("[ERRO] Nao foi possivel comunicar com o Gestor!!!");
					return 1;
				}
				read(fd_fifoclir,read_struct,strlen(read_struct));
				printf("%s\n",read_struct->resposta);
				close(fd_fifoclir);
				break;
			case 2:     
				fd_fifocliw = open (clientfifow, O_WRONLY);
				if (fd_fifocliw == -1)
				{
					printf ("[ERRO] Nao foi possivel comunicar com o Gestor!!!");
					return 1;
				}
				strcpy(write_struct->username,username);
				write_struct->tipoinfo=2;
				write(fd_fifocliw,write_struct,sizeof(write_struct));			
				close(fd_fifocliw);
				fd_fifoclir = open (clientfifor, O_RDONLY);
				if (fd_fifoclir == -1)
				{
					printf("[ERRO] Nao foi possivel comunicar com o Gestor!!!");
					return 1;
				}
				read(fd_fifoclir, read_struct, strlen(read_struct));
				close (fd_fifoclir);
				printf("TOPICOS:\n%s\n", read_struct->topicos);			
				break;
			case 3:
				fd_fifocliw = open (clientfifow, O_WRONLY);
				if (fd_fifocliw == -1)
				{
					printf ("[ERRO] Nao foi possivel comunicar com o Gestor!!!");
					return 1;
				}
				write_struct=Consulta_L_Titulos(write_struct);
				strcpy(write_struct->username,username);
				write_struct->tipoinfo=3;
				write(fd_fifocliw,write_struct,sizeof(write_struct));			
				close(fd_fifocliw);
				fd_fifoclir = open (clientfifor, O_RDONLY);
				if (fd_fifoclir == -1)
				{
					printf("[ERRO] Nao foi possivel comunicar com o Gestor!!!");
					return 1;
				}
				read(fd_fifoclir, read_struct, strlen(read_struct));
				close (fd_fifoclir);
				printf("TOPICO: %s TITULOS:\n%s\n", write_struct->topico,read_struct->titulos);
				break;
			case 4:
				fd_fifocliw = open (clientfifow, O_WRONLY);
				if (fd_fifocliw == -1)
				{
					printf ("[ERRO] Nao foi possivel comunicar com o Gestor!!!");
					return 1;
				}
				write_struct=Consulta_Mensagem(write_struct);
				strcpy(write_struct->username,username);
				write_struct->tipoinfo=4;
				write(fd_fifocliw,write_struct,sizeof(write_struct));			
				close(fd_fifocliw);
				fd_fifoclir = open (clientfifor, O_RDONLY);
				if (fd_fifoclir == -1)
				{
					printf("[ERRO] Nao foi possivel comunicar com o Gestor!!!");
					return 1;
				}
				read(fd_fifoclir, read_struct, strlen(read_struct));
				close (fd_fifoclir);
				printf("TOPICO: %s TITULO: %s\n CORPO: \n%s\n", write_struct->topico,write_struct->titulo,read_struct->corpo);
				break;
			case 5:
fd_fifocliw = open (clientfifow, O_WRONLY);
				if (fd_fifocliw == -1)
				{
					printf ("[ERRO] Nao foi possivel comunicar com o Gestor!!!");
					return 1;
				}
				write_struct=Subscrever_Canc_Subs_Topico(write_struct);
				strcpy(write_struct->username,username);
				write(fd_fifocliw,write_struct,sizeof(write_struct));			
				close(fd_fifocliw);
				fd_fifoclir = open (clientfifor, O_RDONLY);
				if (fd_fifoclir == -1)
				{
					printf("[ERRO] Nao foi possivel comunicar com o Gestor!!!");
					return 1;
				}
				read(fd_fifoclir, read_struct, strlen(read_struct));
				close (fd_fifoclir);
				printf("%s\n", read_struct->resposta);
				break;
			case 6:
				break;
			default:
				printf("\nOpcao Invalida!\n");
		};
	}while(op != 6);
	return 0;
}
