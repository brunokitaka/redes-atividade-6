#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <math.h>
#include <stdio_ext.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <wiringPi.h>
#include <stdio.h>
#include <stdint.h> 

#define ledPin 17

/*
 * Cliente TCP
 */
int main(int argc, char *argv[]) {
    unsigned short port;
    FILE *file;
    char sendbuf[150];
    char recvbuf[1500];
    struct hostent *hostnm;
    struct sockaddr_in server;
    int s, x;
    float temp = 0.0;
    char command[] = "cat /sys/bus/w1/devices/28-0516a46321ff/w1_slave | sed -n \'s/^.*\\(t=[^ ]*\\).*/\\1/p\' | sed \'s/t=//\' | awk \'{x=$1}END{print(x/1000)}\'";
    wiringPiSetupGpio(); 

    /*
     * O primeiro argumento (argv[1]) eh o hostname do servidor.
     * O segundo argumento (argv[2]) eh a porta do servidor.
     */
    if (argc != 3)
    {
        fprintf(stderr, "Use: %s hostname porta\n", argv[0]);
        exit(1);
    }
    /*
     * Obtendo o endereco IP do servidor
     */
    hostnm = gethostbyname(argv[1]);
    if (hostnm == (struct hostent *)0)
    {
        fprintf(stderr, "Gethostbyname failed\n");
        exit(2);
    }
    port = (unsigned short)atoi(argv[2]);

    /*
     * Define o endereco IP e a porta do servidor
     */
    server.sin_family = AF_INET;
    server.sin_port = htons(port);
    server.sin_addr.s_addr = *((unsigned long *)hostnm->h_addr);

    /*
     * Cria um socket TCP (stream)
     */
    if ((s = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
    {
        perror("\nERRO ao declarar o socket\n");
        exit(3);
    }

    /* Estabelece conexao com o servidor */
    if (connect(s, (struct sockaddr *)&server, sizeof(server)) < 0)
    {
        perror("\nERRO ao conectar com o servidor\n");
        exit(4);
    }

    pinMode(ledPin, OUTPUT);
    digitalWrite(ledPin, HIGH);

    while (1) {
        digitalWrite(ledPin, LOW);
        if((file = popen(command, "r")) != NULL){
			x = fread(sendbuf, sizeof(char), 2000, file);
			sendbuf[x] = '\0';
			printf("%s", sendbuf);
		}
        
        if (send(s, sendbuf, strlen(sendbuf) + 1, 0) < 0) {
            perror("\nERRO ao enviar a mensagem(1)\n");
            exit(5);
        }

        /* Recebe a mensagem do servidor no buffer de recep��o */
        if (recv(s, recvbuf, sizeof(recvbuf), 0) < 0) {
            perror("\nERRO ao receber a mensagem(1)\n");
            exit(6);
        }
        sleep(2);
	if(atoi(recvbuf) == 1){
	  digitalWrite(ledPin, HIGH);
	}
        sleep(2);
        printf("%s\n", recvbuf);
        
    }
    close(s);
    
}
