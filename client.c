// Client side C/C++ program to demonstrate Socket programming
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#define DOWNLOAD_FOLDER "download/"

void initSocket(struct sockaddr_in *address, int *newSocket);
void connectSocket(struct sockaddr_in *serverAddress, int *socket);
void fetchFile(int socket, long *fileSize, char *fileName);

char *buffer, *serverAddress;
int bufferSize, serverPort;
struct timeval start, end;

int main(int argc, char const *argv[])
{

	if (argc < 5)
	{
		printf("Argumentos insuficientes\n");
		printf("Execute no formato ./client [endereço_do_servidor] [porta] [nome_do_arquivo] [tamanho_do_buffer]\n");
		exit(1);
	}

	serverAddress = argv[1];
	serverPort = atoi(argv[2]);
	char *fileName = argv[3];
	bufferSize = atoi(argv[4]);

	int socket = 0;
	struct sockaddr_in serverAddress;
	long fileSize;

	buffer = malloc(bufferSize * sizeof(char));

	//Inicializa os dados do Socket
	initSocket(&serverAddress, &socket);

	//Conecta o socket ao servidor
	connectSocket(&serverAddress, &socket);

	//Descobre o tamanho do arquivo em bytes e faz o download do servidor
	fetchFile(socket, &fileSize, fileName);

	//Calcula o tempo de transmissão do arquivo em microsegundos
	long downloadTime = ((end.tv_sec * 1000000 + end.tv_usec) - (start.tv_sec * 1000000 + start.tv_usec));

	printf("Buffer = %5u byte(s), %6.2lf kbps (%ld bytes em %lu.%06lu s)\n",
		   bufferSize,
		   ((fileSize / 1024.0) / (downloadTime / 1000000.0)),
		   fileSize,
		   end.tv_sec - start.tv_sec,
		   end.tv_usec - start.tv_usec);

	return 0;
}

void initSocket(struct sockaddr_in *address, int *newSocket)
{
	if ((*newSocket = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		printf("\n Socket creation error \n");
		exit(EXIT_FAILURE);
	}

	memset(address, '0', sizeof(*address));

	address->sin_family = AF_INET;
	address->sin_port = htons(serverPort);

	// Converte IP para binário
	if (inet_pton(AF_INET, serverAddress, &address->sin_addr) <= 0)
	{
		printf("\nInvalid address/ Address not supported \n");
		exit(EXIT_FAILURE);
	}
}

void connectSocket(struct sockaddr_in *serverAddress, int *socket)
{
	if (connect(*socket, (struct sockaddr *)serverAddress, sizeof(*serverAddress)) < 0)
	{
		printf("\nConnection Failed \n");
		exit(EXIT_FAILURE);
	}
}

void fetchFile(int socket, long *fileSize, char *fileName)
{
	//Define o lugar onde colocar o arquivo a partir do nome dele e da pasta de downloads
	char *downloadFile;
	downloadFile = malloc(sizeof(DOWNLOAD_FOLDER) + sizeof(fileName));

	strcpy(downloadFile, DOWNLOAD_FOLDER);
	strcat(downloadFile, fileName);

	//Envia a requisição do arquivo ao servidor pelo nome
	send(socket, fileName, strlen(fileName), 0);

	//Lê a resposta do servidor, que deve ser o tamanho do arquivo
	read(socket, fileSize, sizeof(long));

	printf("Buscando arquivo %s, tamanho: %ld bytes\n", fileName, *fileSize);

	printf("Escrevendo arquivo para %s\n", downloadFile);

	//Marca o tempo do início da transmissão do arquivo
	gettimeofday(&start, NULL);

	//Faz a abertura do download a ser gravado
	FILE *file = fopen(downloadFile, "wb");

	long bytesLeft = *fileSize;

	//Faz a leitura do arquivo e a gravação do download
	while (bytesLeft != 0)
	{
		int sentBytes;
		int biteSize = fileSize < bufferSize ? bytesLeft : bufferSize;

		//Lê o arquivo, em "mordidas" do tamanho exato do dado restante ou o máximo que cabe no buffer
		read(socket, buffer, biteSize);
		bytesLeft -= biteSize;

		fwrite(buffer, sizeof(char), biteSize, file);
	}

	//Fecha o arquivo
	fclose(file);

	//Marca o tempo do final da transmissão do arquivo
	gettimeofday(&end, NULL);
}
