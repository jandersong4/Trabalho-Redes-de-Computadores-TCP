#include "common.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <math.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>

void usage(int argc, char **argv)
{
    printf("usage:  %s <server IP> <server port>\n", argv[0]);
    printf("example: %s 127.0.0.1  51511\n", argv[0]);
    exit(EXIT_FAILURE);
}

#define BUFSZ 1024

int main(int argc, char **argv)
{
    int driverNotFound = 1;

    while (driverNotFound)
    {

        int userMenuOption = -1;
        printf("-----------------------------------\n");
        printf("0 - Sair\n");
        printf("1 - Solicitar Corrida\n");
        printf("---------------------------------\n");
        scanf("%d", &userMenuOption);
        if (userMenuOption == 0)
        {
            return 0;
        }
        else if (userMenuOption == 1)
        {
            // Limpar o buffer de entrada
            // necessário pois do contrario não é possível ler a mensagem para ser mandada.
            while (getchar() != '\n')
                ;
        }

        // if (argc < 3)
        // {
        //     usage(argc, argv);
        // }

        // socket de comunicação, passando o valor do ip e a porta
        struct sockaddr_storage storage;
        if (0 != addrparse("::1", "5151", &storage))
        {
            usage(argc, argv);
        }

        int s;
        s = socket(storage.ss_family, SOCK_STREAM, 0);

        if (s == -1)
        {
            logexit("socket");
        }
        struct sockaddr *addr = (struct sockaddr *)(&storage);
        if (0 != connect(s, addr, sizeof(storage)))
        {
            logexit("conect");
        }

        char addrstr[BUFSZ];
        addrtostr(addr, addrstr, BUFSZ);

        printf("conected to %s\n", addrstr);

        // recebendo resposta caso o motorista aceite ou não a corrida
        char buf[BUFSZ];
        memset(buf, 0, BUFSZ);
        int count = recv(s, buf, BUFSZ - 1, 0);
        if (count == 0)
        {
            printf("Conexão encerrada pelo servidor.\n");
            exit(EXIT_SUCCESS);
        }
        else
        {
            if (strcmp(buf, "") != 0)
            {
                printf("---------------------------\n");
                printf("%s\n", buf);
                printf("---------------------------\n");
            }
            else
            {
                driverNotFound = 0;
                Coordinate coordCli = {-19.8679429, -43.9697909};
                // char buf[BUFSZ];
                memset(buf, 0, BUFSZ);
                // printf("mensagem>");
                // fgets(buf, BUFSZ - 1, stdin);
                count = send(s, (void *)&coordCli, sizeof(coordCli) + 1, 0);
                if (count != sizeof(coordCli) + 1)
                {
                    logexit("send");
                }

                memset(buf, 0, BUFSZ);
                unsigned total = 0;
                double distance = 1;
                while (distance > 0)
                {
                    ssize_t count = recv(s, &distance, sizeof(distance), 0);
                    distance > 0 ? printf("Motorista a %dm \n", (int)distance) : printf("O motorista chegou.");
                    if (count == 0)
                    {
                        break;
                    }
                    total += count;
                }

                close(s);
                puts(buf);
                exit(EXIT_SUCCESS);
            }
        }
    }
}