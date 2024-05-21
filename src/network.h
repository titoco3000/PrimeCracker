/*
Define structs para receber e enviar mensagens na rede
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <pthread.h>
#include <ctype.h>
#include "log.h"

#define TAMANHO_BUFFER_REBIMENTO 1024

// Ultilidades

int obter_alguma_porta()
{
    int sockfd;
    struct sockaddr_in addr;
    socklen_t addrlen = sizeof(addr);

    // Cria um socket
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("socket");
        return -1;
    }

    // Setup sockaddr_in
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(0); // Usa porta 0 pra deixar que o sistema escolha porta disponivel

    // Bind o socket ao addr e porta 0
    if (bind(sockfd, (struct sockaddr *)&addr, sizeof(addr)) < 0)
    {
        perror("bind");
        close(sockfd);
        return -1;
    }

    // Obtem numero da porta obtida
    if (getsockname(sockfd, (struct sockaddr *)&addr, &addrlen) < 0)
    {
        perror("getsockname");
        close(sockfd);
        return -1;
    }

    int available_port = ntohs(addr.sin_port);
    close(sockfd);

    return available_port;
}

static int str2int(int *out, char *s)
{
    char *end;
    if (s[0] == '\0' || isspace(s[0]))
        return 0;
    long l = strtol(s, &end, 10);
    *out = l;
    return 1;
}

char *obter_ip_local()
{
    char hostbuffer[256];
    char *IPbuffer;
    struct hostent *host_entry;
    int hostname;

    // To retrieve hostname
    hostname = gethostname(hostbuffer, sizeof(hostbuffer));
    if (hostname == -1)
        return 0;

    // To retrieve host information
    host_entry = gethostbyname(hostbuffer);
    if (!host_entry)
        return 0;

    // To convert an Internet network
    // address into ASCII string
    IPbuffer = inet_ntoa(*((struct in_addr *)
                               host_entry->h_addr_list[0]));

    return IPbuffer;
}

// NetworkDestination

typedef struct NetworkDestination
{
    char ipv4[30];
    int porta;
} NetworkDestination;

int NetworkDestination_parse(char *str, NetworkDestination *net)
{
    char porta_buf[5];
    int ipv4_bufocup = 0;
    int porta_bufocup = 0;

    int lendo_porta = 0;
    for (; str != 0; str++)
    {
        if (lendo_porta)
        {
            if (isdigit(*str))
            {
                porta_buf[porta_bufocup++] = *str;
            }
            else
            {
                porta_buf[porta_bufocup] = 0;
                return str2int(&net->porta, porta_buf);
            }
        }
        else
        {
            if (*str == ':')
            {
                net->ipv4[ipv4_bufocup] = 0;
                // verifica se ipv4 é valido
                struct sockaddr_in sa;
                int result = inet_pton(AF_INET, net->ipv4, &(sa.sin_addr));
                if (result != 1)
                    return 0;
                lendo_porta = 1;
            }
            else
                net->ipv4[ipv4_bufocup++] = *str;
        }
    }
    return 0;
}

void NetworkDestination_print(NetworkDestination *net)
{
    printf("%s:%d\n", net->ipv4, net->porta);
}

int NetworkDestination_cmp(NetworkDestination *a,NetworkDestination *b){
    int ip_comp = strcmp(a->ipv4, b->ipv4);
    if(ip_comp == 0)
        return b->porta - a->porta;
    return ip_comp;
}
int NetworkDestination_equals(NetworkDestination *a,NetworkDestination *b)
{
    return strcmp(a->ipv4, b->ipv4) == 0 && a->porta == b->porta;
}

// Network

typedef struct Network
{
    int porta_local;
    void (*ao_receber)(char *, NetworkDestination *origem);
    pthread_t thread;
} Network;

static void *internal_network_listener(void *arg)
{
    Network *net = (Network *)arg;

    int server_fd, new_socket;
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    char buffer[TAMANHO_BUFFER_REBIMENTO] = {0};
    int opt = 1;

    // Creating socket file descriptor
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
    {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // Forcefully attaching socket to the port 5000
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)))
    {
        perror("setsockopt");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(net->porta_local);

    // Binding the socket to the network address and port
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0)
    {
        perror("bind failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    // Start listening for incoming connections
    if (listen(server_fd, 3) < 0)
    {
        perror("listen");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    print_log(LL_ALL, "Server is listening on port %d\n", net->porta_local);

    while (1)
    {
        // Accept a connection from a client
        if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen)) < 0)
        {
            perror("accept");
            close(server_fd);
            exit(EXIT_FAILURE);
        }

        print_log(LL_ALL, "Connection accepted from %s:%d\n",
                  inet_ntoa(address.sin_addr), ntohs(address.sin_port));

        // Read message from the client
        int valread = read(new_socket, buffer, TAMANHO_BUFFER_REBIMENTO);
        if (valread < 0)
        {
            perror("read");
            close(new_socket);
            continue;
        }

        // Null-terminate the received data
        buffer[valread] = '\0';

        NetworkDestination origem;
        strcpy(origem.ipv4, inet_ntoa(address.sin_addr));
        origem.porta = ntohs(address.sin_port);

        net->ao_receber(buffer, &origem);

        // Close the client socket
        close(new_socket);
    }

    // Close the server socket
    close(server_fd);
}

Network *Network_new(void (*ao_receber)(char *, NetworkDestination *), int porta)
{
    Network *net = (Network *)malloc(sizeof(Network));

    net->porta_local = porta;
    net->ao_receber = ao_receber;

    pthread_create(&net->thread, NULL, internal_network_listener, (void *)net);
    return net;
}

int Network_enviar_mensagem(Network *net, NetworkDestination *destino, char *msg)
{
    print_log(LL_ALL, "Sending \"%s\" to ", msg);
    maybe_log(LL_ALL)
    {
        NetworkDestination_print(destino);
        puts("");
    }
    int sock = 0;
    struct sockaddr_in serv_addr;
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        print_log(LL_CRITICAL, "\n Socket creation error \n");
        return 0;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(destino->porta);

    // Convert IPv4 and IPv6 addresses from text to binary form
    if (inet_pton(AF_INET, destino->ipv4, &serv_addr.sin_addr) <= 0)
    {
        print_log(LL_CRITICAL, "\nInvalid address/ Address not supported \n");
        return 0;
    }

    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        print_log(LL_ALL, "\nConnection Failed \n");
        return 0;
    }

    send(sock, msg, strlen(msg), 0);
    print_log(LL_ALL, "Message sent: %s\n", msg);

    close(sock);
    return 1;
}

NetworkDestination Network_obter_endereco(Network *net)
{
    NetworkDestination dest;
    strcpy(dest.ipv4, obter_ip_local());
    dest.porta = net->porta_local;
    return dest;
}

void NetworkDestination_copy(NetworkDestination *dest, NetworkDestination src)
{
    strcpy(dest->ipv4, src.ipv4);
    dest->porta = src.porta;
}