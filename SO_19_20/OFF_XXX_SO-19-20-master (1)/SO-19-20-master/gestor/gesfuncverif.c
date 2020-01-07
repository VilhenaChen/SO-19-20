//Verificar mensagem
#define READ 0
#define WRITE 1

int lanca_verifica_mensagem(char * n_verificador, char * forbwords, int * infp, int * outfp);
int lanca_verifica_mensagem(char * n_verificador, char * forbwords, int * infp, int * outfp)
{
        int p_stdin[2];
        int pai_filho[2];
        int filho_pai[2];
        pid_t pid;

        if(pipe(pai_filho) != 0 || pipe(filho_pai) != 0)
                return -1;
        pid = fork();
        if(pid < 0)
        {
                printf("\nO fork falhou\n");
                return pid;
        }
        else
        {
                if(pid == 0)
                {
                        close(pai_filho[WRITE]);
                        dup2(pai_filho[READ],READ);
                        close(filho_pai[READ]);
                        dup2(filho_pai[WRITE],WRITE);
                        execl(n_verificador,n_verificador,forbwords,NULL);
                        perror("execl");
                        exit(1);
                }
        }
        if(infp == NULL)
                close(pai_filho[WRITE]);
        else
                *infp = pai_filho[WRITE];
        if(outfp == NULL)
                close(filho_pai[READ]);
        else
                *outfp = filho_pai[READ];
	return pid;
}
