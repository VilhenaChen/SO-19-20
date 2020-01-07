#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include "gesheader.h"

// Mutex's
pthread_mutex_t acesso_fifo_escrita_mutex;

// Ponteiro para o hHead da lista de users
pusr users_head = NULL;

// Head da lista de topicos
ptop topicos_head = NULL;


int escreve_msg_no_fifo(char* nome_do_fifo, gc* msg_gc)
{
    int fd_fifo;

    pthread_mutex_lock(&acesso_fifo_escrita_mutex);

    fd_fifo = open (nome_do_fifo, O_WRONLY | O_NONBLOCK);
    if (fd_fifo == -1)
    {
        fprintf(stderr, "Erro: Nao foi possivel comunicar com o Gestor!");
        return 0;
    }
    write(fd_fifo, msg_gc, sizeof(*msg_gc));
    close(fd_fifo);

    pthread_mutex_unlock(&acesso_fifo_escrita_mutex);

    return 1;
}

int cria_fifo_gestor()
{
    // FIFO para transmissão inicial Cliente -> Gestor
    char nome_fifo_ges[50];

    get_nome_fifo(nome_fifo_ges, TIPO_FIFO_GES);

    // Cria os fifos
    if (mkfifo(nome_fifo_ges, 0600) < 0){
        fprintf(stderr, "Erro: Não foi possivel criar o FIFO \"%s\".\n", nome_fifo_ges);
        return 0;
    }

    return 1;
}

// Thread para decrementar a duração das mensagens e elimina-las se expiraram
void *expiracao_de_mensagens(void *arg) {
    pthread_t id = pthread_self();
    pmsg aux_percorre_a = NULL;
    pmsg aux_percorre_b = NULL;
    ptop aux_percorre_topicos=NULL;
    pmsg aux_apaga = NULL;

    while (1) {
        fprintf(stderr, "Info: Decrementando o tempo de vida das mensagens.\n");
        /*
        Percorrer todas as mensagens existentes e decrementar o tempo de vida.
            Sempre que o tempo de vida chegar a zero, remover a mensagem.
        */
        for (aux_percorre_topicos = topicos_head; aux_percorre_topicos != NULL; aux_percorre_topicos = aux_percorre_topicos->prox)
        {
            for (aux_percorre_a = aux_percorre_topicos->lmensagem; aux_percorre_a != NULL; aux_percorre_a = aux_percorre_a->prox)
            {
                if(aux_percorre_a->duracao > 0)
                {
                    aux_percorre_a->duracao = aux_percorre_a->duracao - 1;
                }
                else
                {
                    aux_apaga = aux_percorre_a;
                    for (aux_percorre_b = aux_percorre_topicos->lmensagem; aux_percorre_b != NULL; aux_percorre_b = aux_percorre_b = aux_percorre_b->prox)
                    {
                        if (aux_percorre_b->prox == aux_apaga)
                        {
                            if (aux_percorre_b == aux_percorre_topicos->lmensagem)
                            {
                                aux_percorre_topicos->lmensagem = aux_percorre_b->prox;
                            }
                            else
                            {
                                aux_percorre_b->prox = aux_apaga->prox;
                                free(aux_apaga);
                            }
                        }
                    }
                } 
            }
        }

        sleep(1);
    }
    return NULL;
}

// Thread para incrementar o tempo de inactividade dos clientes e eliminar os que atingirem o tempo máximo de inactividade
void *inactividade_de_clientes(void *arg) {
    pthread_t id = pthread_self();
    pusr aux_percorre_a = NULL;
    pusr aux_percorre_b = NULL;
    pusr aux_apaga = NULL;
    while (1) {
        fprintf(stderr, "Info: Aumentando o tempo de inactividade dos clientes.\n");
        /*
        Percorrer todos os clientes e aumentar o tempo de inactividade.
            Sempre que o tempo de inactividade chegar a zero, remover o cliente.
        */
       
        for (aux_percorre_a = users_head; aux_percorre_a!= NULL; aux_percorre_a = aux_percorre_a->prox)
        {
            if(aux_percorre_a->tempo_inactividade < TEMPO_INATIVIDADE_CLIENTE)
            {
                aux_percorre_a->tempo_inactividade = aux_percorre_a->tempo_inactividade + 1;
            }
            else
            {
                aux_apaga = aux_percorre_a;
                for (aux_percorre_b = users_head; aux_percorre_b != NULL; aux_percorre_b = aux_percorre_b = aux_percorre_b->prox)
                {
                    if (aux_percorre_b->prox == aux_apaga)
                    {
                       if (aux_percorre_b == users_head)
                       {
                            users_head = aux_percorre_b->prox;
                       }
                       else
                       {
                           aux_percorre_b->prox = aux_apaga->prox;
                           free(aux_apaga);
                       }
                    }
                }
            } 
        }
        sleep(1);
    }
    return NULL;
}

// Thread para enviar o heartbeat a todos os clientes
void *envio_de_heartbeat(void *arg)
{
    gc msg_gc;
    pusr aux_percorre_a = NULL;
    // FIFO para transmissão Gestor -> Clientes
    char nome_fifo_cli_r[20];

    pthread_t id = pthread_self();

    while (1)
    {
        fprintf(stderr, "Info: Envio de Heartbeat.\n");

        //TODO: Percorrer lista de clientes e ler dos fifos
        for (aux_percorre_a = users_head; aux_percorre_a!= NULL; aux_percorre_a = aux_percorre_a->prox)
        {
            strcpy(nome_fifo_cli_r, aux_percorre_a->fifor);
            limpa_msg_gc(&msg_gc);
            msg_heartbeat_gestor(&msg_gc);
            escreve_msg_no_fifo(nome_fifo_cli_r, &msg_gc);
        }

        sleep(INTERVALO_HEARTBEAT);
    }

    return NULL;
}

// Thread para receber e processar as mensagens do FIFO do Gestor (vindas dos clientes)
void *leitura_do_fifo_do_gestor(void *arg)
{
    cg msg_cg;
    gc msg_gc;
    // FIFO para recepção: Cliente -> Gestor
    int fd_fifo;
    char nome_fifo_ges[20];
    int indice;
    pusr aux_username;
    pusr aux_percorre_a;
    pusr aux_percorre_b;

    get_nome_fifo(nome_fifo_ges, TIPO_FIFO_GES);

    pthread_t id = pthread_self();

    while (1)
    {
        fd_fifo = open(nome_fifo_ges, O_RDONLY);

        // Ler do fifo
        read(fd_fifo, &msg_cg, sizeof(msg_cg));

        // imprimir log
        fprintf(stderr, "Info: Recebida mensagem do tipo (%d).\n", msg_cg.tipoinfo);

        switch (msg_cg.tipoinfo){
            case MSG_PEDIR_USERNAME:
                //Fazer o registo do username e responder através do fifo do respectivo Cliente
                aux_username=malloc(sizeof(usr));
                strcpy(aux_username->nome,msg_cg.username);
                strcpy(aux_username->fifor,msg_cg.fifoclirecebe);
                strcpy(aux_username->fifow,msg_cg.fifoclienvia);
                aux_username->tempo_inactividade=0;
                aux_username->indice=0;
                for(aux_percorre_a=users_head;aux_percorre_a!=NULL;aux_percorre_a=aux_percorre_a->prox)
                {
                    if(strcmp(aux_username->nome,aux_percorre_a->nome)==0)
                    {
                        indice=aux_username->indice;
                        indice++;
                        sprintf(aux_username->nome,"%s%d",aux_percorre_a->nome,indice);
                        aux_username->indice=indice;
                        for(aux_percorre_b=users_head;aux_percorre_b!=NULL;aux_percorre_b=aux_percorre_b->prox)
                        {
                            if(strcmp(aux_percorre_a->nome,aux_percorre_b->nome)==0)
                            {
                                aux_percorre_b->indice=indice;
                            }
                        }
                        break;
                    }
                }
                msg_confirmar_username(&msg_gc,aux_username->nome);
                escreve_msg_no_fifo(aux_username->fifor,&msg_gc);
                aux_username->prox=users_head;
                users_head=aux_username;
                break;
            default:
                fprintf(stderr, "Erro: Recebida mensagem de tipo invlálido (%d).\n", msg_cg.tipoinfo);
        }

        close(fd_fifo);
    }
    return NULL;
}

// Thread para receber e processar as mensagens dos FIFOs (vindas dos clientes)
void *leitura_dos_fifos_dos_clientes(void *arg)
{
    cg msg_gc; // passar a gc

    // FIFO para recepção: Gestor -> Cliente
    int fd_fifo;
    char nome_fifo_cli_w[20];

    pthread_t id = pthread_self();

    while (1)
    {
        //TODO: Percorrer lista de clientes e ler dos fifos
        /*while ()
        {
            fd_fifo = open(nome_fifo_cli_r, O_RDONLY);

            // Ler do fifo
            read(fd_fifo, &msg_gc, sizeof(msg_gc));

            // imprimir log
            fprintf(stderr, "Info: Recebida mensagem do tipo (%d).\n", msg_gc.tipoinfo);

            switch (msg_gc.tipoinfo){
                case MSG_CONFIRMAR_USERNAME:

                    break;
                default:
                    fprintf(stderr, "Erro: Recebida mensagem de tipo invlálido (%d).\n", msg_gc.tipoinfo);
            }

            close(fd_fifo);
        }*/

    }

    return NULL;
}

// TODO: Thread para tratar da interacção com o utilizador
void *interaccao_com_utilizador(void *arg)
{
     pthread_t id = pthread_self();

    while (1)
    {
        //TODO: Percorrer lista de clientes e ler dos fifos
        /*while ()
        {
            fd_fifo = open(nome_fifo_cli_r, O_RDONLY);

            // Ler do fifo
            read(fd_fifo, &msg_gc, sizeof(msg_gc));

            // imprimir log
            fprintf(stderr, "Info: Recebida mensagem do tipo (%d).\n", msg_gc.tipoinfo);

            switch (msg_gc.tipoinfo){
                case MSG_CONFIRMAR_USERNAME:

                    break;
                default:
                    fprintf(stderr, "Erro: Recebida mensagem de tipo invlálido (%d).\n", msg_gc.tipoinfo);
            }

            close(fd_fifo);
        }*/

    }

    return NULL;
}

// TODO: Thread para lançar o verificador

int main(void)
{
    pthread_t envio_heartbeat_thread;
    pthread_t leitura_dos_fifos_dos_clientes_thread;
    pthread_t leitura_do_fifo_do_gestor_thread;
    pthread_t interaccao_com_utilizador_thread;
    pthread_t expiracao_de_mensagens_thread;
    pthread_t inactividade_de_clientes_thread;


    int erro;

    // Tenta criar o FIFO do gestor
    erro = cria_fifo_gestor();
    if (erro != 1)
    {
        fprintf(stderr, "Info: Terminando a execução do Gestor por erro de criação do fifo.\n");
        return -1;
    }

    // Lança a thread de heartbeat
    erro = pthread_create(&envio_heartbeat_thread, NULL, &envio_de_heartbeat, NULL);
    if (erro != 0)
        fprintf(stderr, "Erro: Não foi possivel criar a thread de envio de heartbeat. Erro: %s.\n", strerror(erro));
    else
        fprintf(stderr, "Info: Thread de envio de heartbeat criada.\n");

    // Lança a thread de leituta do fifo do gestor
    erro = pthread_create(&leitura_do_fifo_do_gestor_thread, NULL, &leitura_do_fifo_do_gestor, NULL);
    if (erro != 0)
        fprintf(stderr, "Erro: Não foi possivel criar a thread de leitura do fifo do gestor. Erro: %s.\n", strerror(erro));
    else
        fprintf(stderr, "Info: Thread de leitura do fifo do gestor criada.\n");

    // Lança a thread de leituta dos fifos dos clientes
    erro = pthread_create(&leitura_dos_fifos_dos_clientes_thread, NULL, &leitura_dos_fifos_dos_clientes, NULL);
    if (erro != 0)
        fprintf(stderr, "Erro: Não foi possivel criar a thread de leitura dos fifos dos clientes. Erro: %s.\n", strerror(erro));
    else
        fprintf(stderr, "Info: Thread de leitura dos fifos dos clientes criada.\n");

    /*// Lança a thread de expiração de mensagens
    erro = pthread_create(&expiracao_de_mensagens_thread, NULL, &expiracao_de_mensagens, NULL);
    if (erro != 0)
        fprintf(stderr, "Erro: Não foi possivel criar a thread de expiração de mensagens. Erro: %s.\n", strerror(erro));
    else
        fprintf(stderr, "Info: Thread de expiração de mensagens criada.\n");

    // Lança a thread de expiração de clientes por inactividade
    erro = pthread_create(&inactividade_de_clientes_thread, NULL, &inactividade_de_clientes, NULL);
    if (erro != 0)
        fprintf(stderr, "Erro: Não foi possivel criar a thread de expiração de clientes. Erro: %s.\n", strerror(erro));
    else
        fprintf(stderr, "Info: Thread de expiração de clientes criada.\n");
*/
    
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