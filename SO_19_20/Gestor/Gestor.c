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

// Estado do Filtro do Verificador
int EstadoFiltro = FILTRO_LIGADO;

int escreve_msg_no_fifo(char* nome_do_fifo, gc* msg_gc)
{
    int fd_fifo;

    pthread_mutex_lock(&acesso_fifo_escrita_mutex);

    fd_fifo = open (nome_do_fifo, O_WRONLY | O_NONBLOCK);
    if (fd_fifo == -1)
    {
        fprintf(stderr, "Erro: Nao foi possivel comunicar com o Cliente!");
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

int devolve_numero_titulos(char* topico)
{
    ptop aux_percorre_topicos;
    pmsg aux_percorre_titulos;
    int cont=0;
    for(aux_percorre_topicos=topicos_head;aux_percorre_topicos!=NULL;aux_percorre_topicos=aux_percorre_topicos->prox)
    {
        if(strcmp(aux_percorre_topicos->topico,aux_percorre_topicos->topico)==0)
        {
            for(aux_percorre_titulos=aux_percorre_topicos->lmensagem;aux_percorre_titulos!=NULL;aux_percorre_titulos=aux_percorre_titulos->prox)
            {
                cont++;
            }
        }
        break;
    }
    return cont;
}

int devolve_numero_topicos()
{
    ptop aux_percorre_topicos;
    int cont=0;
    for(aux_percorre_topicos=topicos_head;aux_percorre_topicos!=NULL;aux_percorre_topicos=aux_percorre_topicos->prox)
    {
        cont++;
    }
    return cont;
}

void elimina_lista_topicos_user(ptu lista)
{
    if(lista!=NULL)
    {
        elimina_lista_topicos_user(lista->prox);
    }
    free(lista);
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
                    if(aux_apaga!=aux_percorre_topicos->lmensagem)
                    {
                        for (aux_percorre_b = aux_percorre_topicos->lmensagem; aux_percorre_b != NULL; aux_percorre_b = aux_percorre_b = aux_percorre_b->prox)
                        {
                            if (aux_percorre_b->prox == aux_apaga)
                            {
                                aux_percorre_b->prox=aux_apaga->prox;
                                free(aux_apaga);
                                break;
                            }
                        }
                    }
                    else
                    {
                        aux_percorre_topicos->lmensagem=aux_apaga->prox;
                        free(aux_apaga);
                    }

                    if(aux_percorre_topicos==NULL)
                    {
                        break;
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
                if(aux_apaga!=users_head)
                {
                    for (aux_percorre_b = users_head; aux_percorre_b != NULL; aux_percorre_b = aux_percorre_b = aux_percorre_b->prox)
                    {
                        if (aux_percorre_b->prox == aux_apaga)
                        {
                            aux_percorre_b->prox=aux_apaga->prox;
                            aux_percorre_a=aux_percorre_b;
                            elimina_lista_topicos_user(aux_apaga->topcs);
                            free(aux_apaga);
                            break;
                        }
                    }
                }
                else
                {
                    users_head=aux_apaga->prox;
                    aux_percorre_a=users_head;
                    free(aux_apaga);
                }

                if(aux_percorre_a==NULL)
                {
                    break;
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

        //Percorrer lista de clientes e escrever dos fifos
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
                aux_username->topcs=NULL;
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
    cg msg_cg;
    gc msg_gc;
    pusr aux_percorre_users;
    pusr aux_percorre_users_b;
    ptop aux_percorre_topicos;
    pmsg aux_percorre_mensagens;
    ptu aux_insere_topico_user;
    ptu aux_apaga_topico_user;
    ptu aux_a_percorre_topico_user;
    ptu aux_b_percorre_topico_user;
    pusr aux_apaga_usr;
    pusr aux_percorre_users_para_apagar;
    pmsg aux_guarda_mensagem;
    ptop aux_guarda_topico;
    int encontrou_topico;
    int encontrou_titulo;
    char forb_words[50];
    int infp, outfp;
    char corpo[128];
    int num_proib;
    int resultado_verificacao;
    // FIFO para recepção: Gestor -> Cliente
    int fd_fifo;
    char nome_fifo_cli_w[20];
    char nome_fifo_cli_r[20];
    char topicos[2021];
    char titulos[2021];

    int verifica_leitura_fifo;
    pthread_t id = pthread_self();


    // Obter variaveis de ambiente ou usar as default
    if(getenv("WORDSNOT") == NULL)
        strcpy(forb_words, DEFAULT_FORB_WORDS);
    else
        strcpy(forb_words, getenv("WORDSNOT"));

    if(getenv("MAXNOT")==NULL)
        num_proib=N_PROIB;
    else
        sscanf(getenv("MAXNOT"), "%d", &num_proib);

    // Lança o verificador
    if(lanca_verifica_mensagem(PROGRAMA_VERIFICADOR, forb_words, &infp, &outfp) <= 0)
    {
        fprintf(stderr, "Erro: Falha em lancar o verificador!\n");
        fflush(stdout);
    }

    while (1)
    {
        //TODO: Percorrer lista de clientes e ler dos fifos
        for (aux_percorre_users = users_head; aux_percorre_users!= NULL; aux_percorre_users = aux_percorre_users->prox)
        {
            strcpy(topicos,"");
            strcpy(titulos,"");

            strcpy(nome_fifo_cli_w,aux_percorre_users->fifow);
            limpa_msg_cg(&msg_cg);
            fd_fifo = open(nome_fifo_cli_w, O_RDONLY);
            verifica_leitura_fifo=read(fd_fifo,&msg_cg,sizeof(msg_cg));
            if (verifica_leitura_fifo==0)
            {
                close(fd_fifo);
            }
            else if (verifica_leitura_fifo==-1) 
            { // error
                fprintf(stderr, "Erro: Não foi possivel ler do fifo (%s).\n", nome_fifo_cli_w);
                close(fd_fifo);
            } 
            else 
            { // available data
                close(fd_fifo);
                aux_percorre_users->tempo_inactividade=0;
                switch (msg_cg.tipoinfo)
                {
                    case MSG_ENVIAR_MENSAGEM:
                        if(EstadoFiltro==FILTRO_LIGADO)
                        {
                            resultado_verificacao = verifica_mensagem(infp, outfp, msg_cg.corpo, num_proib);
                        }
                        else
                        {
                            resultado_verificacao=1;
                        }
                        if (resultado_verificacao < 1)
                        {
                            msg_erro_mensagem(&msg_gc,msg_cg.topico,msg_cg.titulo,msg_cg.corpo);
                            escreve_msg_no_fifo(aux_percorre_users->fifor,&msg_gc);
                        }
                        else
                        {
                            aux_guarda_mensagem=malloc(sizeof(msg));
                            strcpy(aux_guarda_mensagem->topico,msg_cg.topico);
                            strcpy(aux_guarda_mensagem->titulo,msg_cg.titulo);
                            strcpy(aux_guarda_mensagem->corpo,msg_cg.corpo);
                            aux_guarda_mensagem->duracao=msg_cg.duracao;
                            encontrou_topico=0;
                            for(aux_percorre_topicos=topicos_head;aux_percorre_topicos!=NULL;aux_percorre_topicos=aux_percorre_topicos->prox)
                            {
                                if(strcmp(aux_percorre_topicos->topico,aux_guarda_mensagem->topico)==0)
                                {
                                    if(devolve_numero_titulos(aux_percorre_topicos->topico)<20)
                                    {
                                        aux_guarda_mensagem->prox=aux_percorre_topicos->lmensagem;
                                        aux_percorre_topicos->lmensagem=aux_guarda_mensagem;
                                        encontrou_topico=1;
                                        for(aux_percorre_users_b=users_head;aux_percorre_users_b!=NULL;aux_percorre_users_b=aux_percorre_users_b->prox)
                                        {
                                            for(aux_a_percorre_topico_user=aux_percorre_users_b->topcs;aux_a_percorre_topico_user!=NULL;aux_a_percorre_topico_user=aux_a_percorre_topico_user->prox)
                                            {
                                                if(strcmp(aux_a_percorre_topico_user->topico,msg_cg.topico)==0)
                                                {
                                                    msg_notificar_nova_mensagem_topico(&msg_gc,msg_cg.topico,msg_cg.titulo);
                                                    escreve_msg_no_fifo(aux_percorre_users_b->fifor,&msg_gc);
                                                }
                                            }
                                        }
                                        break;
                                    }
                                    else
                                    {
                                        msg_erro_mensagem(&msg_gc,msg_cg.topico,msg_cg.titulo,msg_cg.corpo);
                                        escreve_msg_no_fifo(aux_percorre_users->fifor,&msg_gc);                                 
                                    }
                                }
                            }
                            if(encontrou_topico!=1)
                            {
                                if(devolve_numero_topicos()<20)
                                {
                                    aux_guarda_topico=malloc(sizeof(top));
                                    strcpy(aux_guarda_topico->topico,aux_guarda_mensagem->topico);
                                    aux_guarda_topico->lmensagem=NULL;
                                    aux_guarda_mensagem->prox=aux_guarda_topico->lmensagem;
                                    aux_guarda_topico->lmensagem=aux_guarda_mensagem;
                                    aux_guarda_topico->prox=topicos_head;
                                    topicos_head=aux_guarda_topico;
                                }
                            }
                        }  
                        break;
                    case MSG_CONSULTAR_TOPICOS :
                        if(topicos_head!=NULL)
                        {
                            strcpy(topicos,topicos_head->topico);
                            for(aux_percorre_topicos=topicos_head->prox;aux_percorre_topicos!=NULL;aux_percorre_topicos=aux_percorre_topicos->prox)
                            {
                                strcat(topicos,"\n");
                                strcat(topicos,aux_percorre_topicos->topico);
                            }
                        }
                        else
                        {
                            strcpy(topicos,"A lista de topicos esta vazia\n");
                            fprintf(stderr,"ERRO: A lista de topicos esta vazia\n");
                        }
                        msg_enviar_topicos(&msg_gc,topicos);
                        escreve_msg_no_fifo(aux_percorre_users->fifor,&msg_gc);
                        break;

                    case MSG_CONSULTAR_TITULOS_TOPICO:
                        if(topicos_head!=NULL)
                        {
                            encontrou_topico=0;
                            for(aux_percorre_topicos=topicos_head;aux_percorre_topicos!=NULL;aux_percorre_topicos=aux_percorre_topicos->prox)
                            {   
                                if(strcmp(aux_percorre_topicos->topico,msg_cg.topico)==0)
                                {
                                    if(aux_percorre_topicos->lmensagem!=NULL)
                                    {
                                        
                                        strcpy(titulos,aux_percorre_topicos->lmensagem->titulo);
                                        for(aux_percorre_mensagens=aux_percorre_topicos->lmensagem->prox;aux_percorre_mensagens!=NULL;aux_percorre_mensagens=aux_percorre_mensagens->prox)
                                        {
                                            strcat(titulos,"\n");
                                            strcat(titulos,aux_percorre_mensagens->titulo);
                                        }
                                    }
                                    else
                                    {
                                        strcpy(titulos,"A lista de titulos do topico pretendido esta vazia\n");
                                        fprintf(stderr,"A lista de titulos do topico %s esta vazia\n",msg_cg.topico);
                                    }
                                    encontrou_topico=1;
                                    break;
                                }
                            }
                            if(encontrou_topico!=1)
                            {
                                strcpy(titulos,"O topico nao existe!\n");
                                fprintf(stderr,"ERRO: O topico nao existe!\n");
                            }
                        }
                        else
                        {
                            strcpy(titulos,"A lista de topicos esta vazia\n");
                            fprintf(stderr,"ERRO: A lista de topicos esta vazia\n");
                        }
                        msg_enviar_titulos_topico(&msg_gc,msg_cg.topico,titulos);
                        escreve_msg_no_fifo(aux_percorre_users->fifor,&msg_gc);
                        break;

                    case MSG_CONSULTAR_MENSAGEM_TOPICO:
                        if(topicos_head!=NULL)
                        {
                            encontrou_topico=0;
                            for(aux_percorre_topicos=topicos_head;aux_percorre_topicos!=NULL;aux_percorre_topicos=aux_percorre_topicos->prox)
                            {
                                if(strcmp(aux_percorre_topicos->topico,msg_cg.topico)==0)
                                {
                                    encontrou_titulo=0;
                                    for(aux_percorre_mensagens=aux_percorre_topicos->lmensagem;aux_percorre_mensagens!=NULL;aux_percorre_mensagens=aux_percorre_mensagens->prox)
                                    {
                                        if(strcmp(aux_percorre_mensagens->titulo,msg_cg.titulo)==0)
                                        {
                                            msg_enviar_mensagem_topico(&msg_gc,msg_cg.topico,msg_cg.titulo,aux_percorre_mensagens->corpo);
                                            encontrou_titulo=1;
                                            break;
                                        }
                                    }
                                    encontrou_topico=1;
                                    if(encontrou_titulo!=1)
                                    {
                                        msg_enviar_informacao(&msg_gc,"O titulo nao existe!\n");
                                        fprintf(stderr,"ERRO: O titulo nao existe!\n");
                                    }
                                    break;
                                }
                            }
                            if(encontrou_topico!=1)
                            {
                                msg_enviar_informacao(&msg_gc,"O topico nao existe!\n");
                                fprintf(stderr,"ERRO: O topico nao existe!\n");
                            }
                        }
                        else
                        {
                            msg_enviar_informacao(&msg_gc,"A lista de topicos esta vazia\n");
                            fprintf(stderr,"ERRO: A lista de topicos esta vazia\n");
                        }
                        escreve_msg_no_fifo(aux_percorre_users->fifor,&msg_gc);
                        break;

                    case MSG_SUBSCREVER_TOPICO:
                        if(topicos_head!=NULL)
                        {
                            encontrou_topico=0;
                            for(aux_percorre_topicos=topicos_head;aux_percorre_topicos!=NULL;aux_percorre_topicos=aux_percorre_topicos->prox)
                            {
                                if(strcmp(aux_percorre_topicos->topico,msg_cg.topico)==0)
                                {
                                    aux_insere_topico_user=malloc(sizeof(tu));
                                    strcpy(aux_insere_topico_user->topico,msg_cg.topico);
                                    aux_insere_topico_user->prox=aux_percorre_users->topcs;
                                    aux_percorre_users->topcs=aux_insere_topico_user;
                                    msg_confirmar_subscricao_de_topico(&msg_gc,aux_percorre_topicos->topico);
                                    encontrou_topico=1;
                                    break;
                                }
                            }
                            if(encontrou_topico!=1)
                            {
                                msg_enviar_informacao(&msg_gc,"O topico nao existe\n");
                                fprintf(stderr,"ERRO: O topico nao existe\n");
                            }

                        }
                        else
                        {
                           msg_enviar_informacao(&msg_gc,"A lista de topicos esta vazia\n");
                           fprintf(stderr,"ERRO: A lista de topicos esta vazia\n");
                        }
                        escreve_msg_no_fifo(aux_percorre_users->fifor,&msg_gc);
                        break;

                    case MSG_CANCELAR_SUBSCICAO_TOPICO:
                        if(aux_percorre_users->topcs!=NULL)
                        {
                            encontrou_topico=0;
                            for(aux_a_percorre_topico_user = aux_percorre_users->topcs;aux_a_percorre_topico_user!=NULL;aux_a_percorre_topico_user=aux_a_percorre_topico_user->prox)
                            {
                                if(strcmp(aux_a_percorre_topico_user->topico,msg_cg.topico)==0)
                                {
                                    aux_apaga_topico_user=aux_a_percorre_topico_user;
                                    if(aux_apaga_topico_user!=aux_percorre_users->topcs)
                                    {
                                        for(aux_b_percorre_topico_user=aux_percorre_users_b->topcs;aux_a_percorre_topico_user!=NULL;aux_a_percorre_topico_user=aux_a_percorre_topico_user->prox)
                                        {
                                            if(aux_b_percorre_topico_user->prox == aux_apaga_topico_user)
                                            {
                                                aux_b_percorre_topico_user->prox=aux_apaga_topico_user->prox;
                                                aux_a_percorre_topico_user=aux_b_percorre_topico_user;
                                                free(aux_apaga_topico_user);
                                                break;
                                            }
                                        }
                                    }
                                    else
                                    {
                                        aux_percorre_users->topcs=aux_apaga_topico_user->prox;
                                        aux_a_percorre_topico_user=aux_percorre_users->topcs;
                                        free(aux_percorre_users);
                                    }
                                    msg_confirmar_cancelamento_subscricao_de_topico(&msg_gc,aux_percorre_topicos->topico);
                                    encontrou_topico=1;
                                    break;
                                }
                            }
                            if(encontrou_topico!=1)
                            {
                                msg_enviar_informacao(&msg_gc,"O topico nao existe na sua lista de topicos subscrito\n");
                                fprintf(stderr,"ERRO: O topico nao existe na sua lista de topicos subscrito\n");
                            }
                        }
                        else
                        {
                            msg_enviar_informacao(&msg_gc,"Nao tem topicos subscritos\n");
                            fprintf(stderr,"ERRO: Nao tem topicos subscritos\n");
                        }
                        escreve_msg_no_fifo(aux_percorre_users->fifor,&msg_gc);
                        break;

                    case MSG_ENVIAR_HEARTBEAT_CLIENTE:
                        //Nao faz nada pois qualquer tipo de mensagem serve de heartbeat
                        break;

                    case MSG_NOTIFICAR_FIM_EXECUCAO_CLIENTE:
                        aux_apaga_usr=aux_percorre_users;
                        if(aux_apaga_usr!=users_head)
                        {
                            for(aux_percorre_users_para_apagar=users_head;aux_percorre_users_para_apagar!=NULL;aux_percorre_users_para_apagar=aux_percorre_users_para_apagar->prox)
                            {
                                if(aux_percorre_users_para_apagar->prox == aux_apaga_usr)
                                {
                                    aux_percorre_users_para_apagar->prox = aux_apaga_usr->prox;
                                    aux_percorre_users=aux_percorre_users_para_apagar;
                                    elimina_lista_topicos_user(aux_apaga_usr->topcs);
                                    free(aux_apaga_usr);
                                    break;
                                }
                            }
                        }
                        else
                        {
                            users_head = aux_apaga_usr->prox;
                            aux_percorre_users=users_head;
                            elimina_lista_topicos_user(aux_apaga_usr->topcs);
                            free(aux_apaga_usr);
                        }
                        break;
                        
                    default:
                        break;
                }
            }

            if(aux_percorre_users==NULL)
            {
                break;
            }
        }

    }

    return NULL;
}

// Thread para tratar da interacção com o utilizador
void *interaccao_com_utilizador(void *arg)
{
    gc msg_gc;
    char input[MAXCOMANDO];
    char comando[20];
    char com[20];
    char argumento[20];
    int found_topic;
    int found_msg;
    int found_usr;
    pusr aux_percorreusr;
    pusr aux_apaga_usr;
    pusr aux_percorreusr_b;
    ptu aux_percorre_b;
    ptu aux_top_usr;
    ptu aux_apaga_top_usr;
    ptu aux_top_usr_b;
    pmsg aux_percorremsg;
    ptop aux_percorretop;
    ptop aux_percorretop_b;
    ptop aux_apaga_top;
    ptop ux_percorretop_b;
    pthread_t id = pthread_self();

    while (1)
    {
        strcpy(com,"");
        strcpy(comando,"");
        strcpy(argumento,"");
        printf("Comando -> ");
        fgets(comando,MAXCOMANDO,stdin);
        retira_enter_final(comando);
        if (sscanf(comando, "%s %s", com, argumento) != 2)
        {
            argumento[0] = '\0';
        }
        //Filtro do verificador
        if (strcmp(com, "filter") == 0 && argumento[0] != '\0' && (strcmp(argumento,"on") == 0 || strcmp(argumento,"off") == 0))
        {
            if(strcmp(argumento,"on") == 0)
            {
                EstadoFiltro = FILTRO_LIGADO;
            }
            else
            {
                if (strcmp(argumento,"off") == 0)
                {
                    EstadoFiltro = FILTRO_DESLIGADO;
                }
            }
            
        }
        else
        {
            //Lista os users
            if(strcmp(com, "users") == 0)
            {
                if(users_head != NULL)
                {
                    printf("Lista de Users:\n");
                    for (aux_percorreusr = users_head; aux_percorreusr != NULL; aux_percorreusr = aux_percorreusr->prox)
                    {
                        printf("Username: %s\n", aux_percorreusr->nome);
                    }
                }
                else
                {
                    printf("A lista de Users esta vazia!!!");
                }
                
            }
            else
            {
                //Lista os topicos
                if(strcmp(com, "topics") == 0)
                {
                    if(topicos_head != NULL)
                    {
                        printf("Lista de Topicos:\n");
                        for(aux_percorretop = topicos_head; aux_percorretop != NULL; aux_percorretop = aux_percorretop->prox)
                        {
                            printf("%s\n", aux_percorretop->topico);
                        }
                    }
                    else
                    {
                        printf("A lista de Topicos esa vazia!!!");
                    }
                    
                }
                else
                {
                    //Lista as mensagens
                    if(strcmp(com, "msg") == 0)
                    {
                        if(topicos_head != NULL)
                        {
                            printf("Lista de mensagens\n");
                            for(aux_percorretop = topicos_head; aux_percorretop != NULL; aux_percorretop = aux_percorretop->prox)
                            {
                                for (aux_percorremsg = aux_percorretop->lmensagem; aux_percorremsg != NULL; aux_percorremsg = aux_percorremsg->prox)
                                {
                                    printf("%s\n", aux_percorremsg->titulo);
                                }
                            }
                        }
                        else
                        {
                            printf("A lista de Topicos esta vazia!!!");
                        }
                        

                    }
                    else
                    {
                        //Lista mensagens de um certo topico
                        if(strcmp(com, "topic") == 0 && argumento[0] != '\0')
                        {
                            if (topicos_head != NULL)
                            { 
                                found_topic=0;
                                for(aux_percorretop = topicos_head; aux_percorretop != NULL; aux_percorretop = aux_percorretop->prox)
                                {
                                    if (strcmp(argumento,aux_percorretop->topico) == 0)
                                    {
                                        found_topic = 1;
                                        printf("Lista de Mensagens do topico %s:\n", argumento);
                                        for(aux_percorremsg = aux_percorretop->lmensagem; aux_percorremsg != NULL; aux_percorremsg = aux_percorremsg->prox)
                                        {
                                            printf("%s\n", aux_percorremsg->titulo);
                                        }
                                        break;
                                    }

                                }
                                if (found_topic == 0)
                                {
                                    printf("O Topico nao existe!!!");
                                }
                            }
                            else
                            {
                                printf("A Lista de topicos esta vazia!!!");
                            }
                            

                        }
                        else
                        {
                            //Elimina uma mensagem
                            if(strcmp(com,"del") == 0 && argumento[0] != '\0')
                            {
                                if (topicos_head != NULL)
                                {
                                    found_msg=0;
                                    for(aux_percorretop = topicos_head; aux_percorretop != NULL; aux_percorretop = aux_percorretop->prox)
                                    {
                                        for(aux_percorremsg = aux_percorretop->lmensagem; aux_percorremsg != NULL; aux_percorremsg = aux_percorremsg->prox)
                                        {
                                            if(strcmp(argumento, aux_percorremsg->titulo) == 0)
                                            {
                                                found_msg = 1;
                                                aux_percorremsg->duracao = 0;
                                                break;
                                            }
                                        }
                                    }
                                    if (found_msg == 0)
                                    {
                                        printf("A Mensagem nao existe!!!");
                                    }
                                }
                                else
                                {
                                    printf("A lista de Topicos esta vazia!!!");
                                }
                                
                            }
                            else
                            {
                                //Kick um utilixador
                                if(strcmp(com,"kick") == 0 && argumento[0] != '\0')
                                {
                                    if (users_head != NULL)
                                    {
                                        found_msg=0;
                                        for(aux_percorreusr = users_head; aux_percorreusr != NULL; aux_percorreusr = aux_percorreusr->prox)
                                        {
                                            if(strcmp(argumento,aux_percorreusr->nome) == 0)
                                            {
                                                found_usr = 1;
                                                aux_apaga_usr = aux_percorreusr;
                                                if(aux_apaga_usr != users_head)
                                                {
                                                    for (aux_percorreusr_b = users_head; aux_percorreusr_b != NULL; aux_percorreusr_b = aux_percorreusr_b->prox)
                                                    {
                                                        if (aux_percorreusr_b->prox == aux_apaga_usr)
                                                        {
                                                            msg_cliente_banido(&msg_gc);
                                                            escreve_msg_no_fifo(aux_percorreusr->fifor, &msg_gc);
                                                            aux_percorreusr_b->prox=aux_apaga_usr->prox;
                                                            elimina_lista_topicos_user(aux_apaga_usr->topcs);
                                                            free(aux_apaga_usr);
                                                            break;
                                                        }
                                                    }
                                                }
                                                else
                                                {
                                                    msg_cliente_banido(&msg_gc);
                                                    escreve_msg_no_fifo(aux_percorreusr->fifor, &msg_gc);
                                                    users_head=aux_apaga_usr->prox;
                                                    elimina_lista_topicos_user(aux_apaga_usr->topcs);
                                                    free(aux_apaga_usr);
                                                }
                                            }
                                        }
                                        if (found_usr == 0)
                                        {
                                            printf("O utilizador nao existe!!!");
                                        }
                                    }
                                    else
                                    {
                                        printf("A lista de Users esta vazia!!!");
                                    }
                                }
                                else
                                {
                                    //Elimina os topicos vazios
                                    if(strcmp(com,"prune") == 0)
                                    {
                                        if (topicos_head != NULL)
                                        {
                                            for(aux_percorretop = topicos_head; aux_percorretop != NULL; aux_percorretop = aux_percorretop->prox)
                                            {
                                                if(aux_percorretop->lmensagem == NULL)
                                                {
                                                    aux_apaga_top = aux_percorretop;
                                                    if (users_head != NULL)
                                                    {
                                                        for(aux_percorreusr = users_head; aux_percorreusr != NULL; aux_percorreusr = aux_percorreusr->prox)
                                                        { 
                                                            if(aux_percorreusr->topcs != NULL)
                                                            {
                                                                for(aux_top_usr = aux_percorreusr->topcs; aux_top_usr != NULL; aux_top_usr = aux_top_usr->prox)
                                                                {
                                                                    if(strcmp(aux_top_usr->topico,aux_percorretop->topico) == 0)
                                                                    {
                                                                        aux_apaga_top_usr = aux_top_usr;
                                                                        if (aux_apaga_top_usr != aux_percorreusr->topcs)
                                                                        {
                                                                            for(aux_top_usr_b = aux_percorreusr->topcs; aux_top_usr_b != NULL; aux_top_usr_b = aux_top_usr_b->prox)
                                                                            {
                                                                                if(aux_top_usr_b->prox == aux_apaga_top_usr)
                                                                                {
                                                                                    aux_top_usr_b->prox = aux_apaga_top_usr->prox;
                                                                                    aux_top_usr = aux_top_usr_b;
                                                                                    free(aux_apaga_top_usr);
                                                                                    break;
                                                                                }
                                                                            }
                                                                        }
                                                                        else
                                                                        {
                                                                            aux_percorreusr->topcs = aux_apaga_top_usr->prox;
                                                                            aux_top_usr = aux_percorreusr->topcs;
                                                                            free(aux_apaga_top_usr);
                                                                        }
                                                                        break;
                                                                    }
                                                                }
                                                            }
                                                        }
                                                    }
                                                    if (aux_apaga_top != topicos_head)
                                                    {
                                                        for(aux_percorretop_b = topicos_head; aux_percorretop_b != NULL; aux_percorretop_b->prox)
                                                        {
                                                            if(aux_percorretop_b->prox == aux_apaga_top)
                                                            {
                                                                aux_percorretop_b->prox = aux_apaga_top->prox;
                                                                aux_percorretop = aux_percorretop_b;
                                                                free(aux_apaga_top);
                                                            }
                                                        }
                                                    }
                                                    else
                                                    {
                                                        aux_percorretop = aux_apaga_top->prox;
                                                        aux_percorretop = topicos_head;
                                                        free(aux_apaga_top);
                                                    }  
                                                    if(aux_percorretop==NULL)
                                                    {
                                                        break;
                                                    }                                                  
                                                }
                                            }
                                        }
                                        else
                                        {
                                            printf("A Lista de topicos esta vazia!!!");
                                        }
                                    }
                                    else
                                    {
                                        if (strcmp(com,"shutdown") == 0)
                                        {
                                            if(users_head != NULL)
                                            {
                                                for(aux_percorreusr = users_head; aux_percorreusr != NULL; aux_percorreusr = aux_percorreusr = aux_percorreusr->prox)
                                                {
                                                    msg_fim_de_execucao_gestor(&msg_gc);
                                                    escreve_msg_no_fifo(aux_percorreusr->fifor, &msg_gc);
                                                    exit (0);
                                                }
                                            }
                                        }
                                        else
                                        {
                                            printf("Comando Invalido!!!\n");
                                        }
                                        
                                    } 
                                }
                            }
                        }  
                    }  
                } 
            }  
        }
    }

    return NULL;
}


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

    // Lança a thread de expiração de mensagens
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
