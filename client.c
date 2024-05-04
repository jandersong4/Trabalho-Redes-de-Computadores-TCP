#include "common.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>

// programa cliente que recebe o ip do servidor e um porto
void usage(int argc, char **argv)
{
    // mensagem para avisar como roda o programa
    printf("usage:  %s <server IP> <server port>\n", argv[0]);
    // exemplo
    printf("example: %s 127.0.0.1  51511\n", argv[0]);
    exit(EXIT_FAILURE);
}

#define BUFSZ 1024

int main(int argc, char **argv)
{

    if (argc < 3) // verifica se o programa foi chamado corretamente
    {
        usage(argc, argv);
    }

    // PASSO 1: Programa vai se comunicar em rede
    // primeira coisa é criar o socket (ver slide da primeira aula de implementação desse codigo)

    // nosso socket
    // agora precisamos chamar a função connect
    // essa função sempre retorna 0 quando tudo funciona corretamente
    // logo, se não retornar 0 é por que nao tivemos nenhum erro

    struct sockaddr_storage storage;
    // fazer o parse, função retorna 0 quando funciona e -1 quando erro
    // passamos o endereço recebido do servidor (argv[1]) o porto (argv[2]) e o ponteiro para o sockaddr_storage
    if (0 != addrparse(argv[1], argv[2], &storage))
    {
        usage(argc, argv); // se der erro chamamos o usage()
    }

    int s;
    // AF_INET = Para abrir um socket da internet
    // SOCK_STREAM = Para abrir um socket  tcp
    s = socket(storage.ss_family, SOCK_STREAM, 0);

    // verificar se deu erro poir em redes temos diversas possíbilidades para um erro acontecer
    if (s == -1)
    {
        logexit("socket");
    }
    // ponteiro que iremos passar no socket
    struct sockaddr *addr = (struct sockaddr *)(&storage); // estamos pegando o ponteiro, fazendo a conversão do tipo do ponteiro e jogamos na variável
    // connect recebe um ponteiro para a struct addr
    // passamos como parametro o socket e o endereço do servidor juntamento com o tamanho da estrutura
    if (0 != connect(s, addr, sizeof(storage)))
    {
        // se houver algum erro printamos
        logexit("conect");
    }

    // imprimir o endereço nessa variável
    char addrstr[BUFSZ];
    // função para imprimir o endereço
    addrtostr(addr, addrstr, BUFSZ);

    printf("conected to %s\n", addrstr);

    // fazer comunicação do cliente com o servidor
    // buffer para armazenar o dado
    char buf[BUFSZ];
    // inicializar o buffer com 0
    memset(buf, 0, BUFSZ);
    printf("mensagem>");
    fgets(buf, BUFSZ - 1, stdin); // lendo do teclado o que mandaremos para o servidor
    // o count dira o número total de bytes que foram transmitidos na rede
    int count = send(s, buf, strlen(buf) + 1, 0); // mandamos o dado s -> socket , buf -> o dado que vamos mandar, e por fim o número de bytes que vamos mandar , 0 é ṕq nao temos mais nenhum opçãp adicional
    // se o número de bytes transmitidos for diferente do strlen  esperado é pq obtivemos um erro
    if (count != strlen(buf) + 1)
    {
        logexit("send");
    }

    // vamos receber o dado dnv
    memset(buf, 0, BUFSZ);
    // total de bytes que recebomos até o momento
    unsigned total = 0;
    // while 1 para ficar recebendo dados do servidor
    while (1)
    {
        // os dados podem chegar em parcelas , logo precisamos do total e das estratégias em seguida
        //  usamos agora o recieve para receber a resposta do servidor
        //  recebemos o dado do socket s, colocamos o dado no buf e o limite de dados que vamos receber é até BUFSZ, passamos 0 pois não vamos utilizar nenhuma opção adicional
        //  quando total é 0 colocamos o dado no inicio do buffer a medida que os dados vão chegando vamos colocando os dados mais para frente
        //  BUFSZ - total, pois ja utilizamos o espaço de bytes que recebemos antes
        count = recv(s, buf + total, BUFSZ - total, 0);
        if (count == 0) // count == 0 é por que não recebemos nada
        {               // so retorna do recieve sem nada se a conexão tiver fechado
            // se a conexão estiver fechada é porque estamos fechando o programa e damos um break
            //  conection terminated
            break;
        }
        total += count;
    }
    // recieve = 0 é porque o servidor terminou de mandar tudo que era necessário
    //  fechamos o socket depois que saímos do while
    close(s);
    printf("received %u bytes\n", total);
    puts(buf);
    exit(EXIT_SUCCESS);
}