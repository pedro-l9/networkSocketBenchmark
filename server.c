// Programa servidor em C para demonstrar comportamento de socket
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <time.h>
#include <sys/time.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>

void initSocket(int *serverFd, int *opt, struct sockaddr_in *address);
void createSocket(int socketFd, struct sockaddr_in *address, int *newSocket);
FILE *openFile(char *fileName, long *fileSize);
void getConfiguration(int argc, char *const argv[]);
void displayUsage();

struct globalConfig_t
{
	int port;		/* -p option */
	int bufferSize; /* -b option */
} globalConfig;

static const char *optString = "p:b:?";

char *buffer;

int main(int argc, char *const argv[])
{
	FILE *filePointer;
	int server_fd, socket, opt = 1;
	long totalBytes = 0, fileSize, uploadTime;
	struct sockaddr_in address;

	struct timeval start, end;

	globalConfig.port = 8080;
	globalConfig.bufferSize = 0;

	//Configura o programa a partir dos argumentos de linha de comando
	getConfiguration(argc, argv);

	//Aloca espaço para o buffer
	buffer = malloc(globalConfig.bufferSize * sizeof(char));

	//Inicializa os dados do Socket
	initSocket(&server_fd, &opt, &address);

	while (1)
	{
		//Cria socket para escutar conexões
		createSocket(server_fd, &address, &socket);

		//Esvazia o buffer
		memset(buffer, 0, globalConfig.bufferSize);

		//Lê a primera requisição, que tem que ser o nome do arquivo
		read(socket, buffer, 200);
		printf("Nome do arquivo: %s\n", buffer);

		//Abre o arquivo para começar o streaming e mede o tamanho dele
		filePointer = openFile(buffer, &fileSize);
		printf("Arquivo aberto! Tamanho: %ld bytes\n", fileSize);

		//Envia o tamanho do arquivo que será enviado
		send(socket, &fileSize, sizeof(long), 0);

		//Marca o tempo do início da transmissão do arquivo
		gettimeofday(&start, NULL);

		totalBytes = 0;
		while (fileSize != 0)
		{
			int sentBytes;
			int biteSize = fileSize < globalConfig.bufferSize ? fileSize : globalConfig.bufferSize;

			//Faz a leitura do arquivo em "mordidas" do tamanho dos dados restantes ou até o limite do buffer
			fread(buffer, 1, biteSize, filePointer);
			fileSize -= biteSize;

			//Faz o envio do arquivo em "mordidas" do tamanho no máximo igual ao do buffer
			sentBytes = send(socket, buffer, biteSize, 0);
			totalBytes += sentBytes;

			printf("Enviado: %d bytes\n", sentBytes);
		}

		//Marca o tempo do final da transmissão do arquivo
		gettimeofday(&end, NULL);

		//Fecha o arquivo
		fclose(filePointer);

		//Fecha o socket
		close(socket);

		//Calcula o tempo de transmissão do arquivo em microsegundos
		uploadTime = ((end.tv_sec * 1000000 + end.tv_usec) - (start.tv_sec * 1000000 + start.tv_usec));

		printf("Enviados um total de %ld bytes em %ld microsegundos\n", totalBytes, uploadTime);
	}
	return 0;
}

void initSocket(int *serverFd, int *opt, struct sockaddr_in *address)
{

	int addrlen = sizeof(*address);

	// Cria file descriptor do socket
	if ((*serverFd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
	{
		perror("socket failed");
		exit(EXIT_FAILURE);
	}

	// Configura parâmetros do socket
	if (setsockopt(*serverFd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT,
				   opt, sizeof(*opt)))
	{
		perror("setsockopt");
		exit(EXIT_FAILURE);
	}
	address->sin_family = AF_INET;
	address->sin_addr.s_addr = INADDR_ANY;
	address->sin_port = htons(globalConfig.port);

	// Faz o bind do socket ao endereço e à porta configurada
	if (bind(*serverFd, (struct sockaddr *)address,
			 addrlen) < 0)
	{
		perror("bind failed");
		exit(EXIT_FAILURE);
	}
	if (listen(*serverFd, 3) < 0)
	{
		perror("listen");
		exit(EXIT_FAILURE);
	}
}

void createSocket(int socketFd, struct sockaddr_in *address, int *newSocket)
{
	int addrlen = sizeof(*address);

	if ((*newSocket = accept(socketFd, (struct sockaddr *)address,
							 (socklen_t *)&addrlen)) < 0)
	{
		perror("accept");
		exit(EXIT_FAILURE);
	}
}

FILE *openFile(char *fileName, long *fileSize)
{
	FILE *filePointer = fopen(fileName, "r");

	if (filePointer == NULL)
	{
		printf("Error opening file!\n");
		exit(1);
	}

	//Move o ponteiro para o final do arquivo para descobrir o tamanho dele
	fseek(filePointer, 0L, SEEK_END);
	*fileSize = ftell(filePointer);

	//Volta o ponteiro para o começo do arquivo
	rewind(filePointer);

	return filePointer;
}

void getConfiguration(int argc, char *const argv[])
{
	int opt = 0;
	opt = getopt(argc, argv, optString);
	while (opt != -1)
	{
		switch (opt)
		{
		case 'p':
			globalConfig.port = atoi(optarg);
			break;

		case 'b':
			globalConfig.bufferSize = atoi(optarg);
			break;

		case '?':
			displayUsage();
			break;
		}

		opt = getopt(argc, argv, optString);
	}

	if (globalConfig.bufferSize == 0)
	{
		displayUsage();
		exit(EXIT_FAILURE);
	}
}

void displayUsage()
{
	printf("------------ TP01 - Server ------------\n\n");

	printf("Configuração Obrigatória:");
	printf("\n\t-b: tamanho do buffer\n");

	printf("Opcionais:");
	printf("\n\t-p: porta do servidor (default: 8080)\n");
}
