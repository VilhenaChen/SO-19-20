#include "../Comum/estruturas_comunicacao.h"

void limpa_msg_gc(gc* msg_gc){
    memset(msg_gc, 0, sizeof(*msg_gc));
}

void limpa_msg_cg(cg* msg_cg){
    memset(msg_cg, 0, sizeof(*msg_cg));
}
// Criação de mensagens para enviar para os clientes


void msg_heartbeat_gestor(gc* msg_gc){
    msg_gc->tipoinfo = MSG_ENVIAR_HEARTBEAT_GESTOR;
    return;
}

void msg_confirmar_username(gc* msg_gc, char* username){
    msg_gc->tipoinfo = MSG_CONFIRMAR_USERNAME;
    strcpy(msg_gc->username, username);
    return;
}

void msg_erro_mensagem(gc* msg_gc, char* topico, char* titulo, char* corpo){
    msg_gc->tipoinfo = MSG_ERRO_MENSAGEM;
    strcpy(msg_gc->topico, topico);
    strcpy(msg_gc->titulo, titulo);
    strcpy(msg_gc->corpo, corpo);
    return;
}

void msg_enviar_topicos(gc* msg_gc, char* topicos)
{
    msg_gc->tipoinfo = MSG_ENVIAR_TOPICOS;
    strcpy(msg_gc->topicos, topicos);
    return;
}

void msg_enviar_titulos_topico(gc* msg_gc, char* topico, char* titulos)
{
    msg_gc->tipoinfo = MSG_ENVIAR_TITULOS_DE_TOPICO;
    strcpy(msg_gc->topico, topico);
    strcpy(msg_gc->titulos, titulos);
    return;
}

void msg_enviar_mensagem_topico(gc* msg_gc, char* topico, char* titulo, char* corpo)
{
    msg_gc->tipoinfo = MSG_ENVIAR_MENSAGEM_DE_TOPICO;
    strcpy(msg_gc->topico, topico);
    strcpy(msg_gc->titulo, titulo);
    strcpy(msg_gc->corpo, corpo);
    return;
}

void msg_confirmar_subscricao_de_topico(gc* msg_gc, char* topico){
    msg_gc->tipoinfo = MSG_CONFIRMAR_SUBSCRICAO_TOPICO;
    strcpy(msg_gc->topico, topico);
    return;
}

void msg_confirmar_cancelamento_subscricao_de_topico(gc* msg_gc, char* topico){
    msg_gc->tipoinfo = MSG_CONFIRMAR_CANCELAMENTO_SUBSCICAO_TOPICO;
    strcpy(msg_gc->topico, topico);
    return;
}

void msg_fim_de_execucao_gestor(gc* msg_gc){
    msg_gc->tipoinfo = MSG_NOTIFICAR_FIM_EXECUCAO_GESTOR;
    return;
}

void msg_enviar_informacao(gc* msg_gc,char* informacao){
    msg_gc->tipoinfo = MSG_ENVIAR_INFORMACAO;
    strcpy(msg_gc->informacao,informacao);
    return;
}

void msg_notificar_nova_mensagem_topico(gc* msg_gc, char* topico, char* titulo)
{
    msg_gc->tipoinfo = MSG_NOTIFICAR_NOVA_MENSAGEM_TOPICO;
    strcpy(msg_gc->topico, topico);
    strcpy(msg_gc->titulo, titulo);
    return;
}

void msg_cliente_banido(gc* msg_gc){
    msg_gc->tipoinfo = MSG_CLIENTE_BANIDO;
    return;
}