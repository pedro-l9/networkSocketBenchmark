// Programa servidor em C para demonstrar comportamento de socket
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <errno.h>
#include <time.h>
#include <sys/time.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define FILES_FOLDER "files/"

void initServerSocket(int *serverFd, int *opt, struct sockaddr_in *address);
void listenSocket(int socketFd, struct sockaddr_in *address, int *newSocket);
FILE *openFile(char *fileName, long *fileSize);
long sendFile(int *socket, FILE *filePointer, long fileSize, char *buffer, long bufferSize);

struct globalConfig_t
{
	int port;		 /* -p option */
	long bufferSize; /* -b option */
	int silent;		 /* -s option */
} globalConfig;

static const char *optString = "p:b:s?";

void displayUsage()
{
	printf("------------ TP01 - Server ------------\n\n");

	printf("Configuração Obrigatória:");
	printf("\n\t-b: tamanho do buffer\n");

	printf("Opcionais:");
	printf("\n\t-p: porta do servidor (default: 8080)");
	printf("\n\t-s: execução silenciosa, desabilita output para o terminal\n");
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
			globalConfig.bufferSize = atol(optarg);
			break;

		case 's':
			globalConfig.silent = 1;
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

int main(int argc, char *const argv[])
{
	FILE *filePointer;
	char *buffer;
	int server_fd, socket, opt = 1;
	long totalBytes = 0, fileSize, uploadTime;
	struct sockaddr_in address;

	struct timeval start, end;

	globalConfig.port = 8080;
	globalConfig.bufferSize = 0;
	globalConfig.silent = 0;

	//Configura o programa a partir dos argumentos de linha de comando
	getConfiguration(argc, argv);

	//Aloca espaço para o buffer
	buffer = malloc(globalConfig.bufferSize * sizeof(char));

	//Inicializa os dados do Socket
	initServerSocket(&server_fd, &opt, &address);

	if (!globalConfig.silent)
		printf("Aguardando conexões...\n");

	while (1)
	{
		//Cria socket para escutar conexões
		listenSocket(server_fd, &address, &socket);

		//Esvazia o buffer
		memset(buffer, 0, globalConfig.bufferSize);

		//Lê a primera requisição, que tem que ser o nome do arquivo
		read(socket, buffer, 200);
		if (!globalConfig.silent)
			printf("Nome do arquivo: %s\n", buffer);

		//Abre o arquivo para começar o streaming e mede o tamanho dele
		filePointer = openFile(buffer, &fileSize);
		if (!globalConfig.silent)
			printf("Arquivo aberto! Tamanho: %ld bytes\n", fileSize);

		//Envia o tamanho do arquivo que será enviado
		send(socket, &fileSize, sizeof(long), 0);

		//Marca o tempo do início da transmissão do arquivo
		gettimeofday(&start, NULL);

		//Envia o arquivo
		totalBytes = sendFile(&socket, filePointer, fileSize, buffer, globalConfig.bufferSize);

		//Aguarda confirmação de recebimento
		read(socket, buffer, 1);
		if (!globalConfig.silent)
			printf("Recebimento confirmado\n");

		//Marca o tempo do final da transmissão do arquivo
		gettimeofday(&end, NULL);

		//Fecha o arquivo
		fclose(filePointer);

		//Fecha o socket
		close(socket);

		//Calcula o tempo de transmissão do arquivo em microsegundos
		uploadTime = ((end.tv_sec * 1000000 + end.tv_usec) - (start.tv_sec * 1000000 + start.tv_usec));

		if (!globalConfig.silent)
			printf("Enviados um total de %ld bytes em %ld microsegundos\n", totalBytes, uploadTime);
	}
	return 0;
}

void initServerSocket(int *serverFd, int *opt, struct sockaddr_in *address)
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

void listenSocket(int socketFd, struct sockaddr_in *address, int *newSocket)
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
	char *uploadFile;
	uploadFile = malloc(sizeof(FILES_FOLDER) + sizeof(fileName));

	strcpy(uploadFile, FILES_FOLDER);
	strcat(uploadFile, fileName);

	if (!globalConfig.silent)
		printf("Abrindo %s\n", uploadFile);

	FILE *filePointer = fopen(uploadFile, "r");

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

long sendFile(int *socket, FILE *filePointer, long fileSize, char *buffer, long bufferSize)
{
	long totalBytes = 0;

	while (fileSize != 0)
	{
		long sentBytes;
		long biteSize = fileSize < bufferSize ? fileSize : bufferSize;

		//Faz a leitura do arquivo em "mordidas" do tamanho dos dados restantes ou até o limite do buffer
		fread(buffer, 1, biteSize, filePointer);

		//Faz o envio do arquivo em "mordidas" do tamanho no máximo igual ao do buffer
		sentBytes = send(*socket, buffer, biteSize, 0);
		totalBytes += sentBytes;

		fileSize -= sentBytes;

		if (!globalConfig.silent)
			printf("Enviado: %ld bytes\n", sentBytes);

		if (sentBytes < 0)
		{
			fprintf(stderr, "Erro no envio: %s\n", strerror(errno));
			exit(1);
		}
	}

	return totalBytes;
}
