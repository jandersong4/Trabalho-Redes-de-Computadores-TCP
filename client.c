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
    printf("example: %s 127.0.0.1  5151\n", argv[0]);
    exit(EXIT_FAILURE);
}

#define BUFSZ 1024

int main(int argc, char **argv)
{
    int driverNotFound = 1;

    while (driverNotFound)
    {

        int userMenuOption = -1;
        printf("0 - Sair\n");
        printf("1 - Solicitar Corrida\n");
        scanf("%d", &userMenuOption);
        if (userMenuOption == 0)
        {
            return 0;
        }
        else if (userMenuOption == 1)
        {
            while (getchar() != '\n')
                ;
        }

        struct sockaddr_storage storage;
        if (0 != addrparse(argv[1], argv[2], &storage))
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
                printf("%s\n", buf);
            }
            else
            {
                driverNotFound = 0;
                Coordinate coordCli = {-19.8679429, -43.9697909};
                memset(buf, 0, BUFSZ);
                count = send(s, (void *)&coordCli, sizeof(coordCli) + 1, 0);
                if (count != sizeof(coordCli) + 1)
                {
                    logexit("send");
                }

                memset(buf, 0, BUFSZ);
                double distance = 1;
                while (distance > 0)
                {
                    ssize_t count = recv(s, &distance, sizeof(distance), 0);
                    distance > 0 ? printf("Motorista a %dm \n", (int)distance) : printf("O motorista chegou.");
                    if (count == 0)
                    {
                        break;
                    }
                }

                close(s);
                puts(buf);
                exit(EXIT_SUCCESS);
            }
        }
    }
}