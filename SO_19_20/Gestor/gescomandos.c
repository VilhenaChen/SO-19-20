int comandos_servidor(char *com,char *arg, int *filtro);
void lista_users();
int liga_desliga_filtro();
void lista_mensagens();
void lista_topicos();
void lista_mensagens_topico();
void apaga_mensagem(char * arg);
void exclui_utilizador(char * arg);
void elimina_topicos_vazios();

int comandos_servidor(char *com,char *arg, int *filtro)
{
        if(strcmp(com,"filter") == 0 && arg[0] != '\0' && (strcmp(arg,"on")==0||strcmp(arg,"off")==0))
        {
                *filtro=liga_desliga_filtro(arg);
        }
        else
        {
                if(strcmp(com,"users") == 0)
                {
                        lista_users();
                }
                else
                {
                        if(strcmp(com,"topics") == 0)
                        {
                                lista_topicos();
                        }
                        else
                        {
                                if(strcmp(com,"msg") == 0)
                                {
                                        lista_mensagens();
                                }
                                else
                                {
                                        if(strcmp(com,"topic") == 0 && arg[0] != '\0' && (strcmp(arg,"Perdidos-e-Achados")==0 || strcmp(arg,"Assuntos-Financeiros")==0 ||strcmp(arg,"Publicidade")==0 ||strcmp(arg,"Curriculo")==0))
                                        {
                                                lista_mensagens_topico(arg);
                                        }
                                        else
                                        {
                                                if(strcmp(com,"del") == 0 && arg[0] != '\0' && (strcmp(arg,"Cachecol-encontrado")==0 ||strcmp(arg,"Cartao-de-Cidadao-Perdido")==0 ||strcmp(arg,"Carteira-encontrada")==0||strcmp(arg,"Pagamento-de-Servico")==0 ||strcmp(arg,"Pedido-de-Reembolso")==0 ||strcmp(arg,"Sistema-de-Seguranca")==0 ||strcmp(arg,"Curriculo-Carolina-Carvalho")==0 ||strcmp(arg,"Curriculo-Jose-Manuel")==0))
                                                {
                                                        apaga_mensagem(arg);
                                                }
                                                else
                                                {
                                                        if(strcmp(com,"kick") == 0 && arg[0] != '\0' && (strcmp(arg,"Velindro")==0 ||strcmp(arg,"Manuel87")==0 ||strcmp(arg,"AlfaBeta")==0 ||strcmp(arg,"R2D2")==0 ||strcmp(arg,"Lindux")==0 ||strcmp(arg,"Briosa")==0))
                                                        {
                                                                exclui_utilizador(arg);
                                                        }
                                                        else
                                                        {
                                                                if(strcmp(com,"prune") == 0)
                                                                {
                                                                        elimina_topicos_vazios();
                                                                }
                                                                else
								{
                                                                        return 1;
                                                                }
                                                        }
                                                }
                                        }
                                }
                        }
                }
        }
        return 0;
}

int liga_desliga_filtro(char *arg)
{
        if(strcmp(arg,"on") == 0)
        {
                printf("\nFiltro Ligado!\n");
                return 1;
        }
        else
        {
                printf("\nFiltro Desligado!\n");
                return 0;
 }

}

void lista_users()
{
        printf("\nUtilizadores:\n");
        printf("Velindro\nManuel87\nAlfaBeta\nR2D2\nLindux\nBriosa\n");
}

void lista_topicos()
{
        printf("\n----Topicos:\n");
        printf("\n Perdidos e Achados");
        printf("\n Assuntos Financeiros");
        printf("\n Publicidade");
        printf("\n Curriculo\n");
}

void lista_mensagens()
{
        printf("\nMensagens:\n");
        printf("\nCachecol encontrado\n");
        printf("Cartao de Cidadao Perdido\n");
        printf("Carteira encontrada\n");
        printf("Pagamento de Servico\n");
        printf("Pedido de Reembolso\n");
        printf("Sistema de Seguranca\n");
        printf("Curriculo Carolina Carvalho\n");
        printf("Curriculo Jose Manuel\n");
}

void lista_mensagens_topico(char * arg)
{
        if(strcmp(arg,"Perdidos-e-Achados")==0)
        {
                printf("\nTopico: Perdidos e Achados\n");
                printf("\nCachecol encontrado\nCartao de Cidadao Perdido\nCarteira encontrada\n");
        }
        else
        {
                if(strcmp(arg,"Assuntos-Financeiros")==0)
                {
                        printf("\nTopico: Assuntos Financeiros\n");
                        printf("\nPagamento de Servico\nPedido de Reembolso\n");
                }
                else
                {
                        if(strcmp(arg,"Publicidade")==0)
                        {
                                printf("\nTopico: Publicidade\n");
                                printf("\nSistema de Seguranca\n");
                        }
                        else
                        {
                                printf("\nTopico: Curriculo\n");
                                printf("\nCurriculo Carolina Carvalho\nCurriculo Jose Manuel\n");
                        }
                }
        }
}

void apaga_mensagem(char * arg)
{
        printf("\nA mensagem <%s> foi eliminada com sucesso!\n",arg);
}
void exclui_utilizador(char * arg)
{
        printf("\nO utilizador <%s> foi excluido com sucesso!\n",arg);
}
void elimina_topicos_vazios()
{
        printf("\nOs topicos vazios foram eliminados!\n");
}
