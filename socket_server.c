#include <stdio.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <unistd.h> // read(), write(), close()
#include "socket_server.h"
#define MAXLEN 100
#define PORT 8080
#define SA struct sockaddr

int sockfd;
void* send_m_server(void* pack)
{
	GtkWidget *message_inp = GTK_WIDGET((((struct packer*)(pack))->data)[0]);
	const gchar *message = gtk_entry_get_text((GtkEntry *)message_inp);
	write(sockfd, message, sizeof(message));
	printf("You: %s", message);
	return NULL;
}

void* recieve_m_server(void* pack)
{
	int sockfd = *(int *)(((struct packer*)pack)->sockf);
	char buff[MAXLEN];
	int n;
	for (;;)
	{
		bzero(buff, sizeof(buff));
		read(sockfd, buff, sizeof(buff));
		printf("Client : %s", buff);
		if ((strncmp(buff, "exit", 4)) == 0)
		{
			printf("Exit...\n");
			break;
		}
	}
	return NULL;
}

// Driver function
void *begin_server(void* connfd)
{
	pthread_t send_thread, recieve_thread;

	struct sockaddr_in servaddr, cli;

	// socket create and verification
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd == -1)
	{
		printf("socket creation failed...\n");
		exit(0);
	}
	else
		printf("Socket successfully created..\n");
	bzero(&servaddr, sizeof(servaddr));

	// assign IP, PORT
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port = htons(PORT);

	// Binding newly created socket to given IP and verification
	if ((bind(sockfd, (SA *)&servaddr, sizeof(servaddr))) != 0)
	{
		printf("Socket bind failed...\n");
		exit(0);
	}
	else
		printf("Socket successfully binded..\n");

	// Now server is ready to listen and verification
	if ((listen(sockfd, 5)) != 0)
	{
		printf("Listen failed...\n");
		exit(0);
	}
	else
		printf("Server listening..\n");
	socklen_t len;
	len = sizeof(cli);

	// Accept the data packet from client and verification
	connfd = accept(sockfd, (SA *)&cli, &len);
	if (connfd < 0)
	{
		printf("server accept failed...\n");
		exit(0);
	}
	else
	{
		// Retrieve client's IP address
		char client_ip[25];
		struct sockaddr_in *client_addr = (struct sockaddr_in *)&cli;
		inet_ntop(AF_INET, &(client_addr->sin_addr), client_ip, INET_ADDRSTRLEN);
		printf("Connected with the client\n");
	}

	pthread_create(&send_thread, NULL, send_m_server, &connfd);
	pthread_create(&recieve_thread, NULL, recieve_m_server, &connfd);

	pthread_join(send_thread, NULL);
	pthread_join(recieve_thread, NULL);

	// After chatting close the socket
	close(sockfd);
	return 0;
}
