#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include "helpers.h"

void usage(char *file)
{
	fprintf(stderr, "Usage: %s server_id server_address server_port\n", file);
	exit(0);
}

int main(int argc, char *argv[])
{
	setvbuf(stdout, NULL, _IONBF, BUFSIZ);
	int sockfd, n, ret;
	struct sockaddr_in serv_addr;
	char id_client[10];
	char buffer[2000];
	if (argc < 4) {
		usage(argv[0]);
	}

	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	DIE(sockfd < 0, "socket");

	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(atoi(argv[3]));
	ret = inet_aton(argv[2], &serv_addr.sin_addr);
	DIE(ret == 0, "inet_aton");
	strcpy(id_client, argv[1]);

	ret = connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(struct sockaddr));
	DIE(ret < 0, "connect");


	fd_set read_set;
	FD_ZERO(&read_set);
	FD_SET(STDIN_FILENO, &read_set);
	FD_SET(sockfd, &read_set);

	int fd_max = sockfd > STDIN_FILENO ? sockfd : STDIN_FILENO;
	send(sockfd, id_client, strlen(id_client)+1, 0);

	while(1) {
		fd_set tmp = read_set;
		select(fd_max + 1, &tmp, NULL, NULL, NULL);
		if(FD_ISSET(STDIN_FILENO, &tmp)) {
			char aux[2000];
			// Citesc de la tastatura
			// trimit catre server
			memset(buffer, 0, 2000);
			fgets(buffer, 2000, stdin);
			if (strncmp(buffer, "exit", 4) == 0) {
				break;
			}
			memcpy(aux, buffer, 2000);
			char *str = strtok(buffer, " ");
			if(strncmp(str, "subscribe", 9) == 0)
			{
				printf("Subscribed to topic.\n");
			}
			else if(strncmp(str, "unsubscribe", 11) == 0) {
				printf("Unsubscribed to topic.\n");
			}
			n = send(sockfd, aux, strlen(aux)+1, 0);
			DIE(n < 0, "send");
		}

		else if (FD_ISSET(sockfd, &tmp)) {
			// citesc de pe socket
			// afisez la ecran
			memset(buffer, 0, 2000);
			int citire = recv(sockfd, &buffer, 2000, 0);
			DIE(citire == -1, "eroare");
			if(citire == 0)
				break;
			if(citire > 0) {
				char mesaj_de_trimis[2000];
				strcpy(mesaj_de_trimis, "\0");
				strcat(mesaj_de_trimis, buffer+1552);
				strcat(mesaj_de_trimis, " - ");
				if(buffer[49] != 0)
					strncat(mesaj_de_trimis, buffer, 50);
				else
					strncat(mesaj_de_trimis, buffer, strlen(buffer));
				strcat(mesaj_de_trimis, " - ");
				if((int)buffer[50] == 0) {
					strcat(mesaj_de_trimis, "INT");
					strcat(mesaj_de_trimis, " - ");
					char integer[20];
					int semn = buffer[51];
					int value = ntohl(*(uint32_t*)(buffer + 52));
					if(semn == 1)
						value *= -1;
					sprintf(integer, "%d", value);
					strcat(mesaj_de_trimis, integer);
				}
				else if((int)buffer[50] == 1) {
					strcat(mesaj_de_trimis, "SHORT_REAL");
					strcat(mesaj_de_trimis, " - ");
					char uinteger[20];
					//float value = (float)ntohs(*(uint16_t*)(buf + 51)) / 100;
					sprintf(uinteger, "%0.2f", (float)ntohs(*(uint16_t*)(buffer + 51)) / 100);
					strcat(mesaj_de_trimis, uinteger);
				}
				else if((int)buffer[50] == 2) {
					strcat(mesaj_de_trimis, "FLOAT");
					strcat(mesaj_de_trimis, " - ");
					char floats[20];
					int semn = buffer[51];
					double value = ntohl(*(uint32_t*)(buffer + 52));
					char inmultit = buffer[56];
					for(int z = 0; z < inmultit; z++) {
						value = (float)value / 10;
					}
					if(semn == 1)
						value *= -1;
					sprintf(floats, "%f", value);
					strcat(mesaj_de_trimis, floats);
				}
				else if((int)buffer[50] == 3) {
					strcat(mesaj_de_trimis, "STRING");
					strcat(mesaj_de_trimis, " - ");
					strncat(mesaj_de_trimis, buffer+51, strlen(buffer+51));
				}
				printf("%s\n", mesaj_de_trimis);
				send(sockfd, "ACK", strlen("ACK")+1, 0);
			}
		}
	}

	close(sockfd);

	return 0;
}
