#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>

// #include <signal.h>
// #include <sys/ipc.h> /* for all IPC function calls */
// #include <sys/shm.h> /* for shmget(), shmat(), shmctl() */

// #define SHM_KEY 0x1434	/* SHM = Shared Memory */
// #define BUFFER_KEY 0x1435 /* Segunda Tarefa */

pthread_mutex_t job_queue_mutex = PTHREAD_MUTEX_INITIALIZER;

int maior;

int count;

void *atendeCliente(void* param);

/*
 * Servidor TCP
 */

int main(int argc, char *argv[])
{
	unsigned short port;
	struct sockaddr_in client;
	struct sockaddr_in server;
	int s;  /* Socket para aceitar conex�es       */
	int ns; /* Socket conectado ao cliente        */
	int namelen;
	int i = 0;
	int qtd = 0;
	int *param;
	pthread_t thread_id;

	/*
     * O primeiro argumento (argv[1]) eh a porta
     * onde o servidor aguardarah por conexoes
     */
	if (argc != 2)
	{
		fprintf(stderr, "Use: %s porta\n", argv[0]);
		exit(1);
	}

	port = (unsigned short)atoi(argv[1]);

	/*
     * Cria um socket TCP (stream) para aguardar conex�es
     */
	if ((s = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
	{
		perror("\nERRO ao atribuir o socket\n");
		exit(2);
	}

	/*
    * Define a qual endereco IP e porta o servidor estarah ligado.
    * IP = INADDDR_ANY -> faz com que o servidor se ligue em todos
    * os enderecos IP
    */
	server.sin_family = AF_INET;
	server.sin_port = htons(port);
	server.sin_addr.s_addr = INADDR_ANY;

	/*
     * Liga o servidor a porta definida anteriormente.
     */
	if (bind(s, (struct sockaddr *)&server, sizeof(server)) < 0)
	{
		perror("\nERRO ao dar bind na porta\n");
		exit(3);
	}

	/*
     * Prepara o socket para aguardar por conex�es e
     * cria uma fila de conex�es pendentes.
     */
	if (listen(s, 1) != 0)
	{
		perror("\nERRO ao dar listen\n");
		exit(4);
	}

	/*
     * Aceita uma conexao e cria um novo socket atraves do qual
     * ocorrera comunicacao com o cliente.
     */
	do
	{
		namelen = sizeof(client);
		if ((ns = accept(s, (struct sockaddr *)&client, (unsigned int *)&namelen)) == -1)
		{
			perror("Accept()");
			exit(5);
		}
		// printf("%i\n", ns);
		param = (int *)(long)ns;
		/* Criar thread */
		/* (id, null, function, params)*/
		pthread_create(&thread_id, NULL, atendeCliente, (void *)param);
		pthread_detach(thread_id);

		// printf("Thread criada: %li\n", (long int)thread_id);

		/* Fecha o socket conectado ao cliente */
		//pthread_join(thread_id[i], NULL);
		i++;
		//close(ns);
	} while (1);
	/* Fecha o socket aguardando por conex�es */
	close(s);
	/* Fecha o socket conectado ao cliente */
	close(ns);

	printf("\nServidor finalizado com sucesso.\n");
	pthread_exit(NULL);
}

void *atendeCliente(void *param)
{
	int z, i;
	char recvbuf[150];
	char sendbuf[1000];
	char *user;
	char *msg;
	char usuario[100];
	char mensagem[100];

	long int tid = (long int)pthread_self();

	user = NULL;
	msg = NULL;

	// printf("Dentro da thread %li\n", tid);
	// printf("%li\n", (long int)param);
	do
	{
		/* Recebe uma mensagem do cliente atraves do novo socket conectado */
		z = recv((long int)param, recvbuf, sizeof(recvbuf), 0);
		if (z != -1 && z != 0)
		{
			printf("Mensagem recebida do cliente: %s\n", recvbuf);
			pthread_mutex_lock (&job_queue_mutex); //LOCK
			if(maior < atoi(recvbuf)){
				maior = atoi(recvbuf);
				strcpy(sendbuf, "1");
			}
			else{
				strcpy(sendbuf, "\n\n");
			}
			pthread_mutex_unlock (&job_queue_mutex); //UNLOCK

			/* Envia uma mensagem ao cliente atraves do socket conectado */
			if (send((long int)param, sendbuf, strlen(sendbuf) + 1, 0) < 0)
			{
				perror("Send()");
				exit(7);
			}
			printf("Mensagem enviada ao cliente: %s\n", sendbuf);
			printf("=====================================================================\n");
		}
		else
		{
			perror("Recv()");
			//exit(6);
		}
	} while (z != -1 && z != 0);

	printf("suspender filho %li\n", tid);
	close((long int)param);
	pthread_exit(NULL);
}