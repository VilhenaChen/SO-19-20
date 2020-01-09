//Verificar mensagem
#define READ 0
#define WRITE 1

int verifica_mensagem(int infp, int outfp, char* corpo, int num_proib){
    char buff[128];
    char ignorar;
    int n_p_encontradas;

    memset(buff, 0x0, sizeof(buff)); //limpa o buffer
    write(infp, corpo, strlen(corpo));
    write(infp, "\n##MSGEND##\n", 12);

    read(outfp, buff, 128);
    fprintf(stderr, "Info: Resposta do verificador foi: %s\n", buff);
    fflush(stdout);
    if (strcmp(buff, "ERROR-1\n") == 0 || strcmp(buff, "ERROR-2\n") == 0 || strcmp(buff, "ERROR-3\n") == 0) {
        fprintf(stderr, "Erro: Falha ao verificar a mensagem (%s).\n", buff);
        return -1;
    } else {
        if (sscanf(buff, "%d%c", &n_p_encontradas, &ignorar) != 2) {
            fprintf(stderr, "Erro: Falha ao verificar a mensagem (%s).\n", buff);
            return -2;
        } else {
            if (n_p_encontradas < num_proib) {
                fprintf(stderr, "Info: Mensagem aceite pelo verificador.\n");
                return 1;
            } else {
                fprintf(stderr, "Error: Mensagem recusada pelo verificador.\n");
                return 0;
            }
        }
    }

}

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
        fprintf(stderr, "Erro: Não foi possivel fazer o fork para o verificador.\n");
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
            execl(n_verificador, "Verificador", forbwords, NULL);
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
