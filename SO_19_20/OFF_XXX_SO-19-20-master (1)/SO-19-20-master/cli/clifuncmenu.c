void clean_stdin()
{
    int c;
    do {
        c = getchar();
    } while (c != '\n' && c != EOF);
}

pcg Nova_MSG(pcg mensagem)
{
        char topic[50];
        char title[50];
        char corp[1000];
        int dur, resposta;
        do{
                clean_stdin();
                printf("\n Insira uma nova mensagem:\n");
                printf("\nTopico da Mensagem: ");
                fgets(topic,sizeof(topic),stdin);
                printf("\nTitulo da Mensagem: ");
                fgets(title,sizeof(title),stdin);
                printf("\nCorpo  da Mensagem: ");
                fgets(corp,sizeof(corp),stdin);
                printf("\nDuracao da Mensagem: ");
                scanf("%d", &dur);
                printf("\nConteudo da mensagem:\n");
                printf("\nTopico: %s", topic);
                printf("\nTitulo: %s", title);
                printf("\nCorpo: %s", corp);
                printf("\nDuracao: %d", dur);
                do{
                        printf("\nPretende enviar esta mensagem? \n 1-Sim \n 2-Nao\n");
                        scanf("%d", &resposta);
                }while(resposta < 1 || resposta > 2);
        }while(resposta == 2);
        clean_stdin();
        strcpy(mensagem->topico,topic);
        strcpy(mensagem->titulo,title);
        strcpy(mensagem->corpo,corp);
        mensagem->duracao = dur;
        mensagem->prox=NULL;
	return mensagem;
}

pcg Consulta_L_Titulos(pcg topico)
{
        
        printf("Insira o Topico referente ao qual pretende consultar os titulos: ");
        gets(topico->topico,101,stdin);
        return topico;
}

pcg Consulta_Mensagem(pcg mensagem)
{
        printf("Insira o Topico referente ao qual pertence a mensagem: ");
        gets(mensagem->topico,101,stdin);
        printf("Insira o Titulo da mensagem: ");
        gets(mensagem->titulo,101,stdin);
        return mensagem;
}

pcg Subscrever_Canc_Subs_Topico(pcg mensagem)
{
        int op;
        printf("Insira o topico que pretende subscrever: ");
        gets(mensagem->topico,101,stdin);
        printf("Pretende: 1- Subscrever ou 2 - Cancelar Subscriçao?");
        scanf("%d",&op);
        if(op==1)
        {
                mensagem->tipoinfo=5;
        }
        else
        {
                mensagem->tipoinfo=6;
        }
        return mensagem;
}

