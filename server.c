#include "common.h"

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/socket.h>
#include <sys/types.h>
#include <math.h>

#define BUFSZ 1024

void usage(int argc, char **argv)
{
    printf("usage: %s <v4|v6> <server port>\n", argv[0]);
    printf("example: %s v4 51511", argv[0]);
    exit(EXIT_FAILURE);
}

struct client_data
{
    int csock;
    struct sockaddr_storage storage;
};

double haversine(double lat1, double lon1,
                 double lat2, double lon2)
{
    // distance between latitudes
    // and longitudes
    double dLat = (lat2 - lat1) *
                  M_PI / 180.0;
    double dLon = (lon2 - lon1) *
                  M_PI / 180.0;

    // convert to radians
    lat1 = (lat1)*M_PI / 180.0;
    lat2 = (lat2)*M_PI / 180.0;

    // apply formulae
    double a = pow(sin(dLat / 2), 2) +
               pow(sin(dLon / 2), 2) *
                   cos(lat1) * cos(lat2);
    double rad = 6371;
    double c = 2 * asin(sqrt(a));
    return rad * c;
}

void *client_thread(void *data)
{
    Coordinate coordServ = {-19.9227,
                            -43.9451};

    struct client_data *cdata = (struct client_data *)data;
    struct sockaddr *caddr = (struct sockaddr *)(&cdata->storage);

    char caddrstr[BUFSZ];
    addrtostr(caddr, caddrstr, BUFSZ);
    printf("[log] connection from %s\n", caddrstr);

    // char buf[BUFSZ];
    // memset(buf, 0, BUFSZ);
    // size_t count = recv(cdata->csock, buf, BUFSZ - 1, 0);
    // printf("[msg] %s, %d bytes %s\n", caddrstr, (int)count, buf);

    Coordinate coordCli;
    memset(&coordCli, 0, sizeof(coordCli));
    ssize_t count = recv(cdata->csock, &coordCli, sizeof(coordCli), 0);
    if (count < 0)
    {
        perror("recv");
        exit(EXIT_FAILURE);
    }
    printf("[msg] Latitude: %f\n", coordCli.latitude);
    printf("[msg] Longitude: %f\n", coordCli.longitude);
    double distance = haversine(coordServ.latitude, coordServ.longitude, coordCli.latitude, coordCli.longitude) * 1000;
    printf("%f Metros \n", distance);
    while (distance > 0)
    {
        distance = distance - 400;
        int count = send(cdata->csock, (double *)&distance, sizeof(distance), 0);
        if (count != sizeof(distance))
        {
            logexit("send");
        }
        // close(cdata->csock);

        usleep(2000000);
    }
    printf("O motorista chegou!\n");
    // printf("[msg] %s, %d bytes %s\n", caddrstr, (int)count, buf);

    // sprintf(buf, "remote endpoint: %.1000s\n", caddrstr);
    // count = send(cdata->csock, buf, strlen(buf) + 1, 0);
    // if (count != strlen(buf) + 1)
    // {
    //     logexit("send");
    // }
    close(cdata->csock);
    pthread_exit(EXIT_SUCCESS);
}

int main(int argc, char **argv)
{

    // if (argc < 3) // verifica se o programa foi chamado corretamente
    // {
    //     usage(argc, argv);
    // }

    struct sockaddr_storage storage;
    // são passados a versão (ipv4 ou ipv6 e a porta em que o programa rodará)
    // if (0 != server_sockaddr_init(argv[1], argv[2], &storage))
    if (0 != server_sockaddr_init("v6", "5151", &storage)) // passamos como padrão IPV4 e a porta 5151
    {
        usage(argc, argv);
    }

    int s;
    // socket que recebe conexão
    s = socket(storage.ss_family, SOCK_STREAM, 0);
    if (s == -1)
    {
        logexit("socket");
    }

    int enable = 1;
    if (0 != setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)))
    {
        logexit("setsockopt");
    }

    struct sockaddr *addr = (struct sockaddr *)(&storage);
    if (0 != bind(s, addr, sizeof(storage)))
    {
        logexit("bind");
    }

    if (0 != listen(s, 10))
    {
        logexit("listen");
    }

    char addrstr[BUFSZ];
    addrtostr(addr, addrstr, BUFSZ);
    printf("bound to %s, Aguardando Solicitação.\n", addrstr);

    while (1)
    {
        struct sockaddr_storage cstorage;
        struct sockaddr *caddr = (struct sockaddr *)(&cstorage);
        socklen_t caddrlen = sizeof(cstorage);

        int csock = accept(s, caddr, &caddrlen); // socket que conversa com o cliente
        if (csock == -1)
        {
            logexit("accept");
        }
        int acceptClient = -1;
        printf("---------------------------------\n");
        printf("Corrida Disponivel: \n");
        printf("0 - Recusar\n");
        printf("1 - Aceitar\n");
        printf("-----------------------------------\n");
        scanf("%d", &acceptClient);
        if (acceptClient == 0)
        {
            char *msg = "Não foi encontrado um motorista";
            int count = send(csock, msg, strlen(msg) + 1, 0);
            if (count != strlen(msg) + 1)
            {
                logexit("send");
            }
            close(csock);
        }
        else if (acceptClient == 1)
        {
            // string vazia apenas como confirmação
            char *msg = "";
            int count = send(csock, msg, strlen(msg) + 1, 0);
            if (count != strlen(msg) + 1)
            {
                logexit("send");
            }

            while (getchar() != '\n')
                ;
            struct client_data *cdata = malloc(sizeof(*cdata));
            if (!cdata)
            {
                logexit("malloc");
            }
            cdata->csock = csock;
            memcpy(&(cdata->storage), &cstorage, sizeof(cstorage));

            pthread_t tid;
            pthread_create(&tid, NULL, client_thread, cdata);
        }
    }

    exit(EXIT_SUCCESS);
}
