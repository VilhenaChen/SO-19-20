#ifndef ESTRUTURAS_COMUNICACAO_H
#define ESTRUTURAS_COMUNICACAO_H

// Constantes dos tipos de mensagens

// Cliente -> Gestor
#define MSG_PEDIR_USERNAME 0
#define MSG_ENVIAR_MENSAGEM 1
#define MSG_CONSULTAR_TOPICOS 2
#define MSG_CONSULTAR_TITULOS_TOPICO 3
#define MSG_CONSULTAR_MENSAGEM_TOPICO 4
#define MSG_SUBSCREVER_TOPICO 5
#define MSG_CANCELAR_SUBSCICAO_TOPICO 6
#define MSG_ENVIAR_HEARTBEAT_CLIENTE 7
#define MSG_NOTIFICAR_FIM_EXECUCAO_CLIENTE 8

// Gestor -> Cliente
#define MSG_CONFIRMAR_USERNAME 100
#define MSG_ERRO_MENSAGEM 101
#define MSG_ENVIAR_TOPICOS 102
#define MSG_ENVIAR_TITULOS_DE_TOPICO 103
#define MSG_ENVIAR_MENSAGEM_DE_TOPICO 104
#define MSG_CONFIRMAR_SUBSCRICAO_TOPICO 105
#define MSG_CONFIRMAR_CANCELAMENTO_SUBSCICAO_TOPICO 106
#define MSG_ENVIAR_HEARTBEAT_GESTOR 107
#define MSG_NOTIFICAR_FIM_EXECUCAO_GESTOR 108
#define MSG_NOTIFICAR_NOVA_MENSAGEM_TOPICO 109

// Fifos
#define FIFO_PASTA "Fifos"
#define FIFO_GES "%s/gest" // Para adição da pasta
#define FIFO_CLI_R "%s/cli_r_%d" // Para adição da pasta e do pid
#define FIFO_CLI_W "%s/cli_w_%d" // Para adição da pasta e do pid

#define TIPO_FIFO_CLI_W 'w'
#define TIPO_FIFO_CLI_R 'r'
#define TIPO_FIFO_GES 'g'

// Estruturas
typedef struct cli_gest cg, *pcg;
struct cli_gest
{
	int tipoinfo;
    char username[30];
    char fifoclienvia[20];
    char fifoclirecebe[20];
    char corpo[1001];
    char topico[101];
    char titulo[101];
    int duracao;
	pcg prox;
};

typedef struct gest_cli gc, *pgc;
struct gest_cli
{
	int tipoinfo;
    char resposta[101];
    char informacao[101];
    char username[30];
    char corpo[1001];
    char topico[101];
    char titulo[101];
    char topicos[2001];
    char titulos[2001];
	pgc prox;
};

#endif