// Client side C/C++ program to demonstrate Socket programming
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <time.h>
#include <sys/time.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define DOWNLOAD_FOLDER "download/"

void initClientSocket(struct sockaddr_in *address, int *newSocket);
void connectSocket(struct sockaddr_in *serverAddress, int *socket);
void fetchFile(int socket, long *fileSize, char *fileName, char *buffer);
void logDataToFile(int bufferSize, long downloadTime, long fileSize);

struct globalConfig_t
{
	char *serverIP; /* -h option */
	int serverPort; /* -p option */
	char *fileName; /* -f option */
	int bufferSize; /* -b option */
	int logData;	/* -l option */
	int silent;		/* -s option */
} globalConfig;

static const char *optString = "h:p:b:f:ls?";

void displayUsage()
{
	printf("------------ TP01 - Client ------------\n\n");

	printf("Configuração Obrigatória:");
	printf("\n\t-f: nome do arquivo a baixar");
	printf("\n\t-b: tamanho do buffer\n");

	printf("Opcionais:");
	printf("\n\t-h: endereço do servidor (default: 127.0.0.1)");
	printf("\n\t-p: porta do servidor (default: 8080)");
	printf("\n\t-l: habilita log dos dados para arquivo");
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
		case 'h':
			globalConfig.serverIP = optarg;
			break;

		case 'p':
			globalConfig.serverPort = atoi(optarg);
			break;

		case 'b':
			globalConfig.bufferSize = atoi(optarg);
			break;

		case 'f':
			globalConfig.fileName = optarg;
			break;

		case 'l':
			globalConfig.logData = 1;
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

	if (globalConfig.bufferSize == 0 || strcmp(globalConfig.fileName, ""))
	{
		displayUsage();
		exit(EXIT_FAILURE);
	}
}

int main(int argc, char *const argv[])
{

	int socket = 0;
	struct sockaddr_in serverAddress;
	struct timeval start, end;
	long fileSize;
	char *buffer;

	globalConfig.serverPort = 8080;
	globalConfig.serverIP = "127.0.0.1";
	globalConfig.silent = 0;
	globalConfig.logData = 0;
	globalConfig.fileName = "";
	globalConfig.bufferSize = 0;

	//Obtem a configuração a partir dos argumentos da linha de comando
	getConfiguration(argc, argv);

	//Aloca espaço para o buffer
	buffer = malloc(globalConfig.bufferSize * sizeof(char));

	//Inicializa os dados do Socket
	initClientSocket(&serverAddress, &socket);

	//Conecta o socket ao servidor
	connectSocket(&serverAddress, &socket);

	//Marca o tempo do início da transmissão do arquivo
	gettimeofday(&start, NULL);

	//Descobre o tamanho do arquivo em bytes e faz o download do servidor
	fetchFile(socket, &fileSize, globalConfig.fileName, buffer);

	//Marca o tempo do final da transmissão do arquivo
	gettimeofday(&end, NULL);

	//Calcula o tempo de transmissão do arquivo em microsegundos
	long downloadTime = ((end.tv_sec * 1000000 + end.tv_usec) - (start.tv_sec * 1000000 + start.tv_usec));

	if (!globalConfig.silent)
		printf("Buffer = %5u byte(s), %6.2lf kbps (%ld bytes em %lu.%06lu s)\n",
			   globalConfig.bufferSize,
			   ((fileSize / 1024.0) / (downloadTime / 1000000.0)),
			   fileSize,
			   end.tv_sec - start.tv_sec,
			   end.tv_usec - start.tv_usec);

	if (globalConfig.logData)
	{
		logDataToFile(globalConfig.bufferSize, downloadTime, fileSize);
	}

	return 0;
}

void initClientSocket(struct sockaddr_in *address, int *newSocket)
{
	if ((*newSocket = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		printf("\n Socket creation error \n");
		exit(EXIT_FAILURE);
	}

	memset(address, '0', sizeof(*address));

	address->sin_family = AF_INET;
	address->sin_port = htons(globalConfig.serverPort);

	// Converte IP para binário
	if (inet_pton(AF_INET, globalConfig.serverIP, &address->sin_addr) <= 0)
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

void fetchFile(int socket, long *fileSize, char *fileName, char *buffer)
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

	if (!globalConfig.silent)
	{
		printf("Buscando arquivo %s, tamanho: %ld bytes\n", fileName, *fileSize);
		printf("Escrevendo arquivo para %s\n", downloadFile);
	}

	//Faz a abertura do download a ser gravado
	FILE *file = fopen(downloadFile, "wa");

	long bytesLeft = *fileSize;

	//Faz a leitura do arquivo e a gravação do download
	while (bytesLeft != 0)
	{
		int sentBytes;
		int biteSize = bytesLeft < globalConfig.bufferSize ? bytesLeft : globalConfig.bufferSize;

		//Lê o arquivo, em "mordidas" do tamanho exato do dado restante ou o máximo que cabe no buffer
		read(socket, buffer, biteSize);
		bytesLeft -= biteSize;

		fwrite(buffer, sizeof(char), biteSize, file);
	}

	//Fecha o arquivo
	fclose(file);
}

void logDataToFile(int bufferSize, long downloadTime, long fileSize)
{
	FILE *file;
	int fileExists = 0;

	//Descobre se o arquivo já existe
	if ((file = fopen("clientData.txt", "r")))
	{
		fileExists = 1;
		fclose(file);
	}

	file = fopen("clientData.txt", "a");

	//Se o arquivo está sendo criado agora, insere o cabeçalho
	if (!fileExists)
	{
		fprintf(file, "bufferSize downloadTime fileSize\n");
	}

	//Grava os dados importantes no arquivo
	fprintf(file, "%i %ld %ld\n", bufferSize, downloadTime, fileSize);

	fclose(file);
}
