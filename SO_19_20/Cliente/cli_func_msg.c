#include "../Comum/estruturas_comunicacao.h"
#include "../Comum/utils.c"

void limpa_msg_cg(cg* msg_cg){
    memset(msg_cg, 0, sizeof(*msg_cg));
}

// Criação de mensagens para enviar para o gestor

void msg_heartbeat_cliente(cg* msg_cg){
    msg_cg->tipoinfo = MSG_ENVIAR_HEARTBEAT_CLIENTE;
    return;
}

void msg_registar_username(cg* msg_cg, char* username, char* fifoclienvia, char* fifoclirecebe ){
    msg_cg->tipoinfo = MSG_PEDIR_USERNAME;
    strcpy(msg_cg->username, username);
    strcpy(msg_cg->fifoclienvia, fifoclienvia);
    strcpy(msg_cg->fifoclirecebe, fifoclirecebe);
    return;
}

void msg_consultar_topicos(cg* msg_cg){
    msg_cg->tipoinfo = MSG_CONSULTAR_TOPICOS;
    return;
}

void msg_consultar_lista_titulos_topico(cg* msg_cg){
    char topic[50];

    printf("\n[Consulta de titulos de tópico]\n");
    printf("\n> Insira o tópico referente ao qual pretende consultar os titulos: ");
    fgets(topic, sizeof(topic), stdin);
    retira_enter_final(topic);

    msg_cg->tipoinfo = MSG_CONSULTAR_TITULOS_TOPICO;
    strcpy(msg_cg->topico, topic);

    return;
}

void msg_consultar_lista_mensagens_topico(cg* msg_cg){
    char topic[50];
    char title[50];

    printf("\n[Consulta de mensagem de tópico]\n");
    printf("\n> Insira o tópico a que pertence a mensagem: ");
    fgets(topic,sizeof(topic),stdin);
    printf("\n> Insira o titulo da mensagem: ");
    fgets(title,sizeof(title),stdin);
    retira_enter_final(topic);
    retira_enter_final(title);

    msg_cg->tipoinfo = MSG_CONSULTAR_MENSAGEM_TOPICO;
    strcpy(msg_cg->topico, topic);
    strcpy(msg_cg->titulo, title);
    return;
}

void msg_subscrever_topico(cg* msg_cg){
    char topic[50];

    printf("\n[Subscrição de tópico]\n");
    printf("\n> Insira o tópico que pretende subscrever: ");
    fgets(topic, sizeof(topic), stdin);
    retira_enter_final(topic);

    msg_cg->tipoinfo = MSG_SUBSCREVER_TOPICO;
    strcpy(msg_cg->topico, topic);
    return;
}

void msg_cancelar_subscricao_topico(cg* msg_cg){
    char topic[50];

    printf("\n[Cancelamento de subscrição de tópico]\n");
    printf("\n> Insira o tópico do qual pretente cancelar a subscrição: ");
    fgets(topic, sizeof(topic), stdin);
    retira_enter_final(topic);

    msg_cg->tipoinfo = MSG_CANCELAR_SUBSCICAO_TOPICO;
    strcpy(msg_cg->topico, topic);
    return;
}

void msg_fim_de_execucao_cliente(cg* msg_cg){
    msg_cg->tipoinfo = MSG_NOTIFICAR_FIM_EXECUCAO_CLIENTE;
    return;
}

void msg_nova_mensagem(cg* msg_cg){

    char topic[50];
    char title[50];
    char corp[1000];
    int dur, resposta;

    do{
        printf("\n[Envio de mensagem]\n");
        printf("\n> Topico da Mensagem: ");
        fgets(topic,sizeof(topic),stdin);
        printf("\n> Titulo da Mensagem: ");
        fgets(title,sizeof(title),stdin);
        printf("\n> Corpo da Mensagem: ");
        fgets(corp,sizeof(corp),stdin);
        printf("\n> Duracao da Mensagem: ");
        scanf("%d", &dur);
        clean_stdin();
        retira_enter_final(topic);
        retira_enter_final(title);
        retira_enter_final(corp);
        printf("\n[Conteudo da mensagem]\n");
        printf("- Topico: %s\n", topic);
        printf("- Titulo: %s\n", title);
        printf("- Corpo: %s\n", corp);
        printf("- Duracao: %d\n", dur);
        do{
            printf("\n> Pretende enviar esta mensagem?");
            printf("\n 1 - Sim");
            printf("\n 2 - Nao");
            printf("\n Opção:");
            scanf("%d", &resposta);
            clean_stdin();
        }while(resposta < 1 || resposta > 2);
    }while(resposta == 2);

    msg_cg->tipoinfo = MSG_ENVIAR_MENSAGEM;
    //strcpy(write_struct->username, username);
    strcpy(msg_cg->topico, topic);
    strcpy(msg_cg->titulo, title);
    strcpy(msg_cg->corpo, corp);
    msg_cg->duracao = dur;
    return;
}
