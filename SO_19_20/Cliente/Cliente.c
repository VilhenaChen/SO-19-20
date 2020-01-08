#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include "cliheader.h"
#include "../Comum/utils.c"

// Ponteiro para os Mutex
pthread_mutex_t acesso_fifo_escrita_mutex;

char username[30] = "";
int gestor_inactivo = 0;

int escreve_msg_no_fifo(char* nome_do_fifo, cg* msg_cg)
{
    int fd_fifo;

    pthread_mutex_lock(&acesso_fifo_escrita_mutex);

    if (access(nome_do_fifo, F_OK) != 0)
    {
        fprintf(stderr, "Erro: Nao foi possivel comunicar com o Gestor!");
        return 0;
    }

    fd_fifo = open (nome_do_fifo, O_WRONLY);
    if (fd_fifo == -1)
    {
        fprintf(stderr, "Erro: Nao foi possivel comunicar com o Gestor!");
        return 0;
    }
    write(fd_fifo, msg_cg, sizeof(*msg_cg));
    close(fd_fifo);

    pthread_mutex_unlock(&acesso_fifo_escrita_mutex);

    return 1;
}

int cria_fifos_cliente()
{
    // FIFO para transmissão Gestor -> Cliente
    char nome_fifo_cli_r[50];
    // FIFO para transmissão Cliente -> Gestor
    char nome_fifo_cli_w[50];

    get_nome_fifo(nome_fifo_cli_r, TIPO_FIFO_CLI_R);
    get_nome_fifo(nome_fifo_cli_w, TIPO_FIFO_CLI_W);

    // Cria os fifos
    if (mkfifo(nome_fifo_cli_r, 0600) < 0){
        fprintf(stderr, "Erro: Não foi possivel criar o FIFO \"%s\".\n", nome_fifo_cli_r);
        return 0;
    }

    if (mkfifo(nome_fifo_cli_w, 0600) < 0){
        fprintf(stderr, "Erro: Não foi possivel criar o FIFO \"%s\".\n", nome_fifo_cli_w);
        return 0;
    }

    return 1;
}

void envia_registo_de_username(){
    cg msg_cg;
    char username[30];
    // FIFO para transmissão Gestor -> Cliente
    char nome_fifo_cli_r[50];
    // FIFO para transmissão Cliente -> Gestor
    char nome_fifo_cli_w[50];
    // FIFO para registo inicial Cliente ->Gestor
    char nome_fifo_ges[50];

    get_nome_fifo(nome_fifo_cli_r, TIPO_FIFO_CLI_R);
    get_nome_fifo(nome_fifo_cli_w, TIPO_FIFO_CLI_W);
    get_nome_fifo(nome_fifo_ges, TIPO_FIFO_GES);

    printf("\nIntroduza nome de utilizador:");
    scanf("%s", username);
    clean_stdin();
    msg_registar_username(&msg_cg, username, nome_fifo_cli_w, nome_fifo_cli_r);
    escreve_msg_no_fifo(nome_fifo_ges, &msg_cg);
}


// Thread para manter o envio de heartbeat
void *envio_de_heartbeat(void *arg)
{
    cg msg_cg;

    // FIFO para transmissão Cliente -> Gestor
    char nome_fifo_cli_w[20];
    get_nome_fifo(nome_fifo_cli_w, TIPO_FIFO_CLI_W);

    pthread_t id = pthread_self();

    while (1)
    {
        fprintf(stderr, "Info: Envio de Heartbeat.\n");

        limpa_msg_cg(&msg_cg);
        msg_heartbeat_cliente(&msg_cg);
        escreve_msg_no_fifo(nome_fifo_cli_w, &msg_cg);

        sleep(INTERVALO_HEARTBEAT);
    }

    return NULL;
}

// Thread para aumentar o tempo de inactividade do Gestor
void *inactividade_gestor(void *arg)
{
    while (1)
    {
        fprintf(stderr, "Info: Aumento de tempo de inactividade do gestor para (%d).\n", ++gestor_inactivo);

        if (gestor_inactivo>TEMPO_INACTIVIDADE_GESTOR)
        {
            fprintf(stderr, "Info: Atingido o tempo de inactividade do Gestor.\n");

            // Terminar o programa
            fprintf(stdout, "\n-------------------------------------------------\n");
            fprintf(stdout, "O Gestor encontra-se inactivo há mais de (%d) segundos, pelo que o cliente vai ser terminado.\n", TEMPO_INACTIVIDADE_GESTOR);
            fprintf(stdout, "-------------------------------------------------\n");
            fflush(stdout);

            fprintf(stderr, "Info: Programa cliente a terminar.\n");

            exit(1);
        }
        sleep(1);
    }
    return NULL;
}

// Thread para receber e processar as mensagens do FIFO (vindas do gestor)
void *leitura_do_fifo(void *arg)
{
    gc msg_gc; // passar a gc

    // FIFO para transmissão Gestor -> Cliente
    int fd_fifo;
    char nome_fifo_cli_r[20];
    get_nome_fifo(nome_fifo_cli_r, TIPO_FIFO_CLI_R); // passar a _R

    pthread_t id = pthread_self();

    while (1)
    {
        fd_fifo = open(nome_fifo_cli_r, O_RDONLY);

        // Ler do fifo
        read(fd_fifo, &msg_gc, sizeof(msg_gc));

        // imprimir log
        fprintf(stderr, "Info: Recebida mensagem do tipo (%d).\n", msg_gc.tipoinfo);

        // reset do tempo de inactividade do gestor
        gestor_inactivo = 0;

        switch (msg_gc.tipoinfo){
            case MSG_CONFIRMAR_USERNAME:
                strcpy(username,msg_gc.username);
                break;

            case  MSG_ERRO_MENSAGEM:
                fprintf(stdout, "\n-------------------------------------------------\n");
                fprintf(stdout, "A seguinte mensagem foi recusada pelo gestor:\n");
                fprintf(stdout, "Topico: %s\n", msg_gc.topico);
                fprintf(stdout, "Título: %s\n", msg_gc.titulo);
                fprintf(stdout, "Corpo: %s\n", msg_gc.corpo);
                fprintf(stdout, "-------------------------------------------------\n");
                fflush(stdout);
                break;

            case  MSG_ENVIAR_TOPICOS:
                fprintf(stdout, "\n-------------------------------------------------\n");
                fprintf(stdout, "Os tópicos existentes são:\n");
                fprintf(stdout, "%s\n", msg_gc.topicos);
                fprintf(stdout, "-------------------------------------------------\n");
                fflush(stdout);
                break;

            case  MSG_ENVIAR_TITULOS_DE_TOPICO:
                fprintf(stdout, "\n-------------------------------------------------\n");
                fprintf(stdout, "Os títulos do tópico (%s) são:\n", msg_gc.topico);
                fprintf(stdout, "%s\n", msg_gc.titulos);
                fprintf(stdout, "-------------------------------------------------\n");
                fflush(stdout);
                break;

            case  MSG_ENVIAR_MENSAGEM_DE_TOPICO:
                fprintf(stdout, "\n-------------------------------------------------\n");
                fprintf(stdout, "A mensagem solicitada é: \n");
                fprintf(stdout, "Topico: %s\n", msg_gc.topico);
                fprintf(stdout, "Título: %s\n", msg_gc.titulo);
                fprintf(stdout, "Corpo: %s\n", msg_gc.corpo);
                fprintf(stdout, "-------------------------------------------------\n");
                fflush(stdout);
                break;

            case  MSG_CONFIRMAR_SUBSCRICAO_TOPICO:
                fprintf(stdout, "\n-------------------------------------------------\n");
                fprintf(stdout, "O tópico (%s) foi subscrito:\n", msg_gc.topico);
                fprintf(stdout, "-------------------------------------------------\n");
                fflush(stdout);
                break;

            case  MSG_CONFIRMAR_CANCELAMENTO_SUBSCICAO_TOPICO:
                fprintf(stdout, "\n-------------------------------------------------\n");
                fprintf(stdout, "A subscrição do tópico (%s) foi cancelada:\n", msg_gc.topico);
                fprintf(stdout, "-------------------------------------------------\n");
                fflush(stdout);
                break;

            case  MSG_ENVIAR_HEARTBEAT_GESTOR:
                // Não é necessário fazer nada pois o reset do tempo de inactividade é feito independentemente do tipo de mensagem.
                break;

            case  MSG_NOTIFICAR_FIM_EXECUCAO_GESTOR:
                fprintf(stdout, "\n-------------------------------------------------\n");
                fprintf(stdout, "O Gestor foi encerrado, pelo que o cliente vai ser terminado.\n");
                fprintf(stdout, "-------------------------------------------------\n");
                fflush(stdout);

                // Terminar o programa
                fprintf(stderr, "Info: Programa cliente a terminar.\n");
                exit(1);
                break;

            case  MSG_NOTIFICAR_NOVA_MENSAGEM_TOPICO:
                fprintf(stdout, "\n-------------------------------------------------\n");
                fprintf(stdout, "Existe uma nova mensagem com os seguintes dados:\n");
                fprintf(stdout, "Tópico: %s\n", msg_gc.topico);
                fprintf(stdout, "Título: %s\n", msg_gc.titulo);
                fprintf(stdout, "-------------------------------------------------\n");
                fflush(stdout);
                break;

            default:
                fprintf(stderr, "Erro: Recebida mensagem de tipo inválido (%d).\n", msg_gc.tipoinfo);
        }

        close(fd_fifo);

    }

    return NULL;
}

// Thread para interacção com o utilizador (menus, inputs, etc)
void *interaccao_com_utilizador(void *arg)
{
    cg msg_cg;
    int op;

    pthread_t id = pthread_self();

    // FIFO para transmissão Cliente -> Gestor
    char nome_fifo_cli_w[20];
    get_nome_fifo(nome_fifo_cli_w, TIPO_FIFO_CLI_W);

    do
    {
        printf("\nMENU: \n");
        printf(" 1 - Escrever Nova Mensagem \n");
        printf(" 2 - Consultar a lista de topicos existentes \n");
        printf(" 3 - Consultar a lista de titulos de mensagens de um determinado topico \n");
        printf(" 4 - Consultar uma mensagem de um topico \n");
        printf(" 5 - Subscrever um topico \n");
        printf(" 6 - Cancelar subscricao de um topico \n");
        printf(" 0 - Sair \n");
        printf("Insira a opcao pretendida: ");

        scanf("%d", &op);
        clean_stdin();

        switch (op)
        {
            case 0:
                // Notificar o gestor de que o cliente vai terminar
                limpa_msg_cg(&msg_cg);
                msg_fim_de_execucao_cliente(&msg_cg);
                escreve_msg_no_fifo(nome_fifo_cli_w, &msg_cg);

                // Terminar o programa
                fprintf(stderr, "Info: Programa cliente a terminar.\n");
                exit(0);
                break;

            case 1:
                // Enviar uma mensagem ao Gestor
                limpa_msg_cg(&msg_cg);
                msg_nova_mensagem(&msg_cg);
                escreve_msg_no_fifo(nome_fifo_cli_w, &msg_cg);
                break;

            case 2:
                // Consultar a lista de topicos
                limpa_msg_cg(&msg_cg);
                //strcpy(msg_cg.username, ....
                msg_consultar_topicos(&msg_cg);
                escreve_msg_no_fifo(nome_fifo_cli_w, &msg_cg);
                break;

            case 3:
                // Consultar a lista de titulos_de um topico
                limpa_msg_cg(&msg_cg);
                msg_consultar_lista_titulos_topico(&msg_cg);
                escreve_msg_no_fifo(nome_fifo_cli_w, &msg_cg);
                break;

            case 4:
                // Consultar uma mensagem de um topico
                limpa_msg_cg(&msg_cg);
                msg_consultar_lista_mensagens_topico(&msg_cg);
                escreve_msg_no_fifo(nome_fifo_cli_w, &msg_cg);
                break;

            case 5:
                // Subscrever um topico
                limpa_msg_cg(&msg_cg);
                msg_subscrever_topico(&msg_cg);
                escreve_msg_no_fifo(nome_fifo_cli_w, &msg_cg);
                break;

            case 6:
                // Cancelar a subscricao_de um topico
                limpa_msg_cg(&msg_cg);
                msg_cancelar_subscricao_topico(&msg_cg);
                escreve_msg_no_fifo(nome_fifo_cli_w, &msg_cg);
                break;

            default:
                // Opção invalida
                break;
        }
        sleep(1);
    }while(op != 0);

    return NULL;
}


int main(void)
{
    pthread_t envio_heartbeat_thread;
    pthread_t leitura_do_fifo_thread;
    pthread_t interaccao_com_utilizador_thread;
    pthread_t inactividade_de_gestor_thread;

    int erro;

    // Tenta criar os FIFOS de cliente
    erro = cria_fifos_cliente();
    if (erro != 1)
    {
        fprintf(stderr, "Info: Terminando a execução do Cliente por erro de criação dos fifos.\n");
        return -1;
    }

    // Lança a thread de leituta do fifo
    erro = pthread_create(&leitura_do_fifo_thread, NULL, &leitura_do_fifo, NULL);
    if (erro != 0)
        fprintf(stderr, "Erro: Não foi possivel criar a thread de leitura do fifo. Erro: %s.\n", strerror(erro));
    else
        fprintf(stderr, "Info: Thread de leitura do fifo criada.\n");

    // Regista o username
    envia_registo_de_username();

    // Aguarda pela confirmação do username
    fprintf(stdout, "Enviado pedido de registo de username.\n");
    while (strlen(username)==0){
        sleep(1);
    }
    fprintf(stdout, "Recebida a confirmação de registo do username (%s).\n", username);

    // Lança a thread de heartbeat
    erro = pthread_create(&envio_heartbeat_thread, NULL, &envio_de_heartbeat, NULL);
    if (erro != 0)
        fprintf(stderr, "Erro: Não foi possivel criar a thread de envio de heartbeat. Erro: %s.\n", strerror(erro));
    else
        fprintf(stderr, "Info: Thread de envio de heartbeat criada.\n");

    // Lança a thread de aumento de inactividade do gestor
    erro = pthread_create(&inactividade_de_gestor_thread, NULL, &inactividade_gestor, NULL);
    if (erro != 0)
        fprintf(stderr, "Erro: Não foi possivel criar a thread de aumento do tempo de inactividade do gestor. Erro: %s.\n", strerror(erro));
    else
        fprintf(stderr, "Info: Thread de aumento do tempo de inactividade do gestor criada.\n");

    // Lança a thread de interacção com o utilizador
    erro = pthread_create(&interaccao_com_utilizador_thread, NULL, &interaccao_com_utilizador, NULL);
    if (erro != 0)
        fprintf(stderr, "Erro: Não foi possivel criar a thread de interacção com o utilizador. Erro: %s.\n", strerror(erro));
    else
        fprintf(stderr, "Info: Thread de interacção com o utilizador criada.\n");

    while (1){
        sleep(10);
    }

}
