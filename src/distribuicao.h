#include <stdio.h>
#include <pthread.h>

#include "lista.h"
#include "network.h"
#include "utils.h"
#include "log.h"
#include "primos.h"

Lista lista_de_nodes;
NetworkDestination *lider_addr;
Network *network;
pthread_t ping_loop_thread;

#define STATUS_CIVIL char
#define CANDIDATO 0
#define ABDICADO 1
#define CIVIL 2
#define LIDER 3

STATUS_CIVIL status_civil = CIVIL;
int candidato = 0;
int votos = 0;

int computacao_inicio;
int computacao_fim;
ull computacao_objetivo = 0;

int strstarts(const char *str, const char *prefix)
{
    return strncmp(str, prefix, strlen(prefix)) == 0;
}

void print_nodes()
{
    printf(" LISTA DE NODES\n");
    Lista_for_item_em((&lista_de_nodes))
    {
        NetworkDestination *dest = (NetworkDestination *)item->conteudo;
        printf("┼───────────────\n");
        printf("│%s:%d\n", dest->ipv4, dest->porta);
    }
    printf("└───────────────\n\n");
}

void registrar()
{
    if (status_civil == LIDER)
    {
        print_log(LL_CRITICAL, "Lider não deveria se registrar\n");
        exit(1);
    }
    char buffer[100];
    NetworkDestination addr_proprio = Network_obter_endereco(network);
    sprintf(buffer, "REGISTRAR %s:%d", addr_proprio.ipv4, addr_proprio.porta);
    Network_enviar_mensagem(network, lider_addr, buffer);
}
void pedir_lider()
{
    if (status_civil == LIDER)
    {
        print_log(LL_CRITICAL, "Lider não deveria pedir lider\n");
        exit(1);
    }
    char buffer[100];
    NetworkDestination addr_proprio = Network_obter_endereco(network);
    sprintf(buffer, "GETLIDER %s:%d", addr_proprio.ipv4, addr_proprio.porta);
    Network_enviar_mensagem(network, lider_addr, buffer);
}
int ping(NetworkDestination *addr)
{
    return Network_enviar_mensagem(network, addr, "PING");
}

void aviso_desconectado(NetworkDestination *addr){
    char buffer[100];
    sprintf(buffer, "DESCONECTADO %s:%d",addr->ipv4, addr->porta);
    Lista_for_item_em(&lista_de_nodes){
        NetworkDestination *outro = (NetworkDestination *)item->conteudo;
        //avisa para o outro da desconexão
        Network_enviar_mensagem(network, outro, buffer);                    
    }
}

void enviar_msg_eleicao(NetworkDestination *destino, NetworkDestination *candidato){
    char buffer[100];
    sprintf(buffer, "ELEICAO %s:%d",candidato->ipv4, candidato->porta);
    Network_enviar_mensagem(network, destino, buffer);                    

}
void candidatar(){
    candidato = 1;
    NetworkDestination addr = Network_obter_endereco(network);
    Lista_for_item_em(&lista_de_nodes){

        NetworkDestination *outro = (NetworkDestination *)item->conteudo;
        // convoca para eleição
        enviar_msg_eleicao(outro,&addr);
    }
}

void iniciar_computacao(NetworkDestination *destino, ull objetivo, int inicio, int fim){
    char buffer[100];
    sprintf(buffer, "COMPUTAR %llu %d %d",objetivo, inicio, fim);
    Network_enviar_mensagem(network, destino, buffer);  
}

void interpretar_mensagem(char *msg, NetworkDestination *origem)
{
    print_log(LL_ALL, "MENSAGEM: \"%s\" de ", msg);
    maybe_log (LL_ALL){
        NetworkDestination_print(origem);
        puts("\n");
    }

    if(strstarts(msg, "ELEICAO")){
        print_log(LL_ALL, "Recebi aviso de eleição\n");

        
        NetworkDestination *candidato = (NetworkDestination *)malloc(sizeof(NetworkDestination));
        if (NetworkDestination_parse(&msg[8], candidato))
        {
            NetworkDestination proprio = Network_obter_endereco(network);
            int cmp = NetworkDestination_cmp(&proprio, candidato);
            if(status_civil == CIVIL || status_civil == LIDER){
                votos = 0;
                if(cmp > 0){
                    status_civil = CANDIDATO;
                    enviar_msg_eleicao(candidato, &proprio);
                }
                else if(cmp < 0){
                    status_civil = ABDICADO;
                    enviar_msg_eleicao(candidato, candidato);   
                }
            }
            else if(status_civil == CANDIDATO){
                if(cmp == 0){
                    votos++;
                    printf("Votos: %d\n", votos);
                }
                else if(cmp < 0){
                    //se a comparação é negativa, abdica em favor do outro
                    status_civil = ABDICADO;
                    enviar_msg_eleicao(candidato, candidato);
                }
            }
            else{

            }
        }
        else
        {
            print_log(LL_CRITICAL, "Eleição invalida: %s\n", &msg[8]);
        }

    }
    // registrar novo nó
    else if (strstarts(msg, "REGISTRAR"))
    {
        print_log(LL_INFO, "Recebi novo pedido de registro\n");
        NetworkDestination *net = (NetworkDestination *)malloc(sizeof(NetworkDestination));
        if (NetworkDestination_parse(&msg[10], net))
        {
            print_log(LL_ALL, "Endereço é valido\n");
            // verifica se ja existe na lista
            ItemLista *item = Lista_procurar((&lista_de_nodes), strcmp(((NetworkDestination *)item->conteudo)->ipv4, net->ipv4) == 0 && ((NetworkDestination *)item->conteudo)->porta == net->porta);
            if (item)
                free(net);
            else
            {
                Lista_for_item_em(&lista_de_nodes){
                    char buffer[100];
                    NetworkDestination *outro = (NetworkDestination *)item->conteudo;
                    //avisa para o outro do novo
                    sprintf(buffer, "CONECTADO %s:%d",net->ipv4, net->porta);
                    Network_enviar_mensagem(network, outro, buffer);                    
                    //avisa para o novo do outro
                    sprintf(buffer, "CONECTADO %s:%d",outro->ipv4, outro->porta);
                    Network_enviar_mensagem(network, net, buffer);
                }
                print_log(LL_ALL, "Inserindo na lista\n");
                Lista_inserir(&lista_de_nodes, net);
            }
        }
        else
        {
            print_log(LL_WARNING, "Registro invalido: %s\n", &msg[10]);
        }
    }
    // ping
    else if (strstarts(msg, "PING"))
    {
        print_log(LL_ALL, "Recebi ping\n");
        // não responde nada, a própria confirmação TCP já faz o efeito
    }
    // obter endereço do lider
    else if (strstarts(msg, "GETLIDER"))
    {
        if (lider_addr)
        {
            NetworkDestination dest;
            if (NetworkDestination_parse(&msg[9], &dest))
            {
                char buffer[100];
                sprintf(buffer, "SETLIDER %s:%d", lider_addr->ipv4, lider_addr->porta);
                Network_enviar_mensagem(network, &dest, buffer);
            }
            else
            {
                print_log(LL_CRITICAL, "Destino invalido: %s\n", &msg[9]);
            }
        }
        else
            print_log(LL_CRITICAL, "Erro: nó sem lider recebendo pedido de lider\n");
    }
    else if (strstarts(msg, "SETLIDER"))
    {
        print_log(LL_INFO, "Recebi novo lider\n");
        NetworkDestination *net = malloc(sizeof(NetworkDestination));
        if (NetworkDestination_parse(&msg[9], net))
        {
            ItemLista *achado = Lista_procurar(&lista_de_nodes, NetworkDestination_equals((NetworkDestination*)item->conteudo,net));
            if(achado){
                lider_addr = (NetworkDestination*)achado->conteudo;
                free(net);
            }
            else{
                Lista_inserir(&lista_de_nodes, (void*)net);
                lider_addr = net;
            }
        }
        else
        {
            print_log(LL_CRITICAL, "Lider invalido: %s\n", &msg[9]);
            free(net);
        }
    }
    // aviso sobre nó
    else if (strstarts(msg, "CONECTADO"))
    {
        print_log(LL_INFO, "Recebi info de nova conexão\n");
        NetworkDestination *net = (NetworkDestination *)malloc(sizeof(NetworkDestination));
        if (NetworkDestination_parse(&msg[10], net))
        {
            // verifica se ja existe na lista
            ItemLista *item = Lista_procurar((&lista_de_nodes), NetworkDestination_equals((NetworkDestination *)item->conteudo, net));
            if (item)
                free(net);
            else
            {
                print_log(LL_INFO, "Inserindo na lista\n");
                Lista_inserir(&lista_de_nodes, net);
            }
        }
        else
        {
            print_log(LL_CRITICAL, "Conexao invalida: %s\n", &msg[10]);
        }
    }
    // aviso que nó desconectou
    else if (strstarts(msg, "DESCONECTADO"))
    {
        print_log(LL_INFO, "Recebi info de desconexão\n");
        NetworkDestination *net = (NetworkDestination *)malloc(sizeof(NetworkDestination));
        if (NetworkDestination_parse(&msg[13], net))
        {
            // verifica se ja existe na lista
            ItemLista *item = Lista_procurar((&lista_de_nodes), strcmp(((NetworkDestination *)item->conteudo)->ipv4, net->ipv4) == 0);
            if (item)
                Lista_remover(&lista_de_nodes, item);
            else
                print_log(LL_CRITICAL, "Erro: tentando remover item que não existe na lista\n");
        }
        else
        {
            print_log(LL_CRITICAL, "Conexao invalida: %s\n", &msg[13]);
        }
    }
    // aviso de novo calculo (lider->servo)
    else if (strstarts(msg, "COMPUTAR"))
    {
        char *str = &msg[9];
    
        // Pointer to keep track of the current position in the string
        char *endptr;

        // Convert the first number
        computacao_objetivo = strtoull(str, &endptr, 10);

        // Convert the second number
        computacao_inicio = (int)strtoull(endptr, &endptr, 10);

        // Convert the third number
        computacao_fim = (int)strtoull(endptr, NULL, 10);

        printf("computar com: %llu %d %d\n", computacao_objetivo, computacao_inicio, computacao_fim);
        sleep(2);
    }
    // aviso de novo calculo (servo->lider)
    else if (strstarts(msg, "RESPOSTA"))
    {
        if(status_civil == LIDER){
            char *str = &msg[9];
    
            // Pointer to keep track of the current position in the string
            char *endptr;

            // Convert the first number
            ull primeiro = strtoull(str, &endptr, 10);
            ull segundo = strtoull(endptr, &endptr, 10);

            printf("\nResposta: %llu x %llu \n\n",primeiro, segundo);

            Lista_for_item_em(&lista_de_nodes)
                Network_enviar_mensagem(network,(NetworkDestination*)item->conteudo, msg);
        }
        else{
            computacao_objetivo = 0;
        }
    }
}

void *ping_loop(){
    while (1)
    {
        if(status_civil == LIDER){
            NetworkDestination *removido = 0;
            Lista_for_item_em(&lista_de_nodes){
                if(!ping((NetworkDestination*)item->conteudo)){
                    removido = Lista_remover(&lista_de_nodes, item);
                    break;
                }
            }
            if(removido){
                aviso_desconectado(removido);
                free(removido);
            }

        }
        else if(status_civil==CIVIL){
            if(!ping(lider_addr)){
                //remove lider da lista de nodes
                printf("Não achou lider\n");
                ItemLista *lider_na_lista = Lista_procurar(&lista_de_nodes, NetworkDestination_equals((NetworkDestination*)item->conteudo,lider_addr));
                printf("Removendo lider da lista\n");
                NetworkDestination *d = Lista_remover(&lista_de_nodes, lider_na_lista);
                printf("Removido\n");
                
                candidatar();
            }
        }
        sleep(1);
    }
    
}

void iniciar_distribuicao(char *input)
{
    lista_de_nodes = Lista_new();
    lider_addr = (NetworkDestination *)malloc(sizeof(NetworkDestination));
    Primos *primos = Primos_abrir("encoded_primes.p");
    // para pegar primos aleatorios
    srand(time(NULL));

    pthread_create(&ping_loop_thread, NULL, ping_loop, 0);

    if (input[0])
    {
        status_civil = CIVIL;
        if (NetworkDestination_parse(input, lider_addr))
        {
            network = Network_new(&interpretar_mensagem, obter_alguma_porta());
            pedir_lider();
            registrar();

            char input_buffer[512];
            while (1)
            {

                if(computacao_objetivo == 0){
                    print_nodes();
                    sleep(1);
                }else{
                    if(computacao_inicio > computacao_fim){
                        int temp = computacao_inicio;
                        computacao_inicio = computacao_fim;
                        computacao_fim = temp;
                    }
                    printf("%d %d %llu\n", computacao_inicio, computacao_fim, computacao_objetivo);
                    for (int i = computacao_inicio; i < computacao_fim&& computacao_objetivo; i++)
                    {
                        ull produto = 0;
                        for (int j = 0; j < computacao_fim && computacao_objetivo && produto<computacao_objetivo; j++)
                        {
                            ull primeiro_fator = Primos_obter_no_index(primos, i);
                            ull segundo_fator = Primos_obter_no_index(primos, j);
                            produto = primeiro_fator*segundo_fator;
                            printf("%llu x %llu = %llu\n", primeiro_fator, segundo_fator, produto);
                            if(produto == computacao_objetivo){
                                printf("%llu x %llu = %llu\n",primeiro_fator, segundo_fator, computacao_objetivo);
                                computacao_objetivo = 0;
                                sprintf(input_buffer, "RESPOSTA %llu %llu",primeiro_fator, segundo_fator);
                                Network_enviar_mensagem(network, lider_addr, input_buffer);
                            }
                        }
                    }
                }
            }
        }
        else
        {
            printf("Endereço de referencia inválido\n");
        }
    }
    else
    {
        status_civil = LIDER;
        print_log(LL_INFO, "Eu sou o lider!\n");
        network = Network_new(interpretar_mensagem, obter_alguma_porta());
        NetworkDestination_copy(lider_addr, Network_obter_endereco(network));
        NetworkDestination_print(lider_addr);
        char buffer[100];
        sprintf(buffer, "%s:%d", lider_addr->ipv4, lider_addr->porta);
        copy_to_clipboard(buffer);
        char input_buffer[512];

        int rodando = 1;
        do
        {
            printf("LIDER\n");
            printf("a) Exibir nodes conectados\n");
            printf("b) Exibir numero primo de exemplo\n");
            printf("c) Calcular par de fatores primos\n");
            printf("d) Encerrar\n");
            printf(" > ");
            obter_input(input_buffer);
            if(input_buffer[0] == 'a'){
                print_nodes();
            }
            else if(input_buffer[0] == 'b'){
                ull num1 = Primos_obter_primo_aleatorio(primos);
                ull num2 = Primos_obter_primo_aleatorio(primos);
                printf("%llu x %llu = %llu\n", num1, num2, num1*num2);
            }
            else if(input_buffer[0] == 'c'){
                printf("Numero: ");
                ull num;
                scanf("%llu",&num);
                int qtd_nodes = Lista_contar(&lista_de_nodes);
                if(qtd_nodes>0){
                    int step = Primos_obter_index_aproximado(primos, num) / qtd_nodes + 1;
                    int i = 0;
                    Lista_for_item_em(&lista_de_nodes){
                        iniciar_computacao((NetworkDestination*)item->conteudo, num, i, i+step);
                        i+=step;
                    }
                }
                else{
                    printf("Isso funciona somente se tiver outros conectados");
                }
                    
            }
            else if(input_buffer[0] == 'd'){
                rodando = 0;
            }
            puts("");

        } while (rodando);
    }
}