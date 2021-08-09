#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "helpers.h"
#include <netinet/tcp.h>

void usage(char *file)
{
	fprintf(stderr, "Usage: %s server_port\n", file);
	exit(0);
}

typedef struct Sf_messages {
	char message[2000];
	struct Sf_messages *next;
}Sf_messages;

typedef struct Client_subscribe {
	int socket;
	int sf;
	int online_status;
	char id[10];
	Sf_messages *head_messages;
	struct Client_subscribe* next;
} Client_subscribe;

typedef struct node_Topics {
	Client_subscribe *lista_clienti_head;
	int nr_de_clienti;
	char nume[52];
	struct node_Topics* next;
} node_Topics;

typedef struct node_Clienti_logati {
	int socket;
	char id[10];
	int online_status;
	struct node_Clienti_logati* next;
} node_Clienti_logati;

typedef struct Topics {
	node_Topics *head;
}Topics;

typedef struct Clienti_logati {
	node_Clienti_logati* head;
} Clienti_logati;

void clienti_logati_init(Clienti_logati **list, int socket, char *id) {
	(*list) = malloc(sizeof(Clienti_logati));
	(*list)->head = malloc(sizeof(node_Clienti_logati));
	(*list)->head->socket = socket;
	strcpy((*list)->head->id, id);
	(*list)->head->online_status = 1;
	(*list)->head->next = NULL;
}

int clienti_logati_add(Clienti_logati *list, int socket, char *id) {
	node_Clienti_logati *aux = list->head;
	while(aux != NULL) {
		if(strcmp(aux->id, id) == 0)
			return 0;
		if(aux->next == NULL)
			break;
		aux = aux->next;
	}
	node_Clienti_logati *nou = malloc(sizeof(node_Clienti_logati));
	nou->online_status = 1;
	nou->socket = socket;
	nou->next = NULL;
	strcpy(nou->id, id);
	aux->next = nou;
	return 1;
}

char* clienti_logati_remove(Clienti_logati *list, int socket, char *id) {
	node_Clienti_logati *aux = list->head;
	node_Clienti_logati *prev = aux;
	if(aux != NULL && strcmp(aux->id, id) == 0) {
		list->head = aux->next;
		free(aux);
		return;
	}
	while(aux != NULL) {
		if(strcmp(id, aux->id) == 0)
			break;
		prev = aux;
		aux = aux->next;
	}
	if(aux == NULL)
		return;
	prev->next = aux->next;
	free(aux);
}

int topic_exists(Topics *list, char *nume) {
	node_Topics *aux = list->head;
	while(aux != NULL) {
		if(strcmp(nume, aux->nume) == 0)
			return 1;
		if(aux->next == NULL)
			break;
		aux = aux->next;
	}
	return 0;
}

void topics_init(Topics **list, char *nume, int socket, int sf, char *id) {
	(*list) = malloc(sizeof(Topics));
	(*list)->head = malloc(sizeof(node_Topics));
	(*list)->head->next = NULL;
	strcpy((*list)->head->nume, nume);
	(*list)->head->lista_clienti_head = malloc(sizeof(Client_subscribe));
	(*list)->head->lista_clienti_head->socket = socket;
	(*list)->head->lista_clienti_head->sf = sf;
	(*list)->head->lista_clienti_head->online_status = 1;
	strcpy((*list)->head->lista_clienti_head->id, id);
	(*list)->head->lista_clienti_head->next = NULL;
}
void topics_add(Topics *list, char *nume, int socket, int sf, char *id) {
	node_Topics *aux = list->head;
	while(aux->next != NULL) {
		aux = aux->next;
	}
	node_Topics *nou = malloc(sizeof(node_Topics));
	strcpy(nou->nume, nume);
	nou->nr_de_clienti = 1;
	nou->next = NULL;
	nou->lista_clienti_head = malloc(sizeof(Client_subscribe));
	nou->lista_clienti_head->next = NULL;
	nou->lista_clienti_head->socket = socket;
	nou->lista_clienti_head->sf = sf;
	strcpy(nou->lista_clienti_head->id, id);
	nou->lista_clienti_head->online_status = 1;
	aux->next = nou;
}

void add_subscriber(Topics *list, char *nume, int socket, int sf, char *id) {
	node_Topics *aux = list->head;
	while(aux != NULL) {
		if(strcmp(aux->nume, nume) == 0)
			break;
		if(aux->next == NULL)
			break;
		aux = aux->next;
	}
	Client_subscribe *aux2 = aux->lista_clienti_head;
	while(aux2->next != NULL)
		aux2 = aux2->next;
	Client_subscribe *nou = malloc(sizeof(Client_subscribe));
	nou->socket = socket;
	nou->sf = sf;
	strcpy(nou->id, id);
	nou->online_status = 1;
	nou->next = NULL;
	aux2->next = nou;
}

void remove_subscriber(Topics *list, char *nume, int socket, char *id) {
	node_Topics *aux = list->head;
	while(aux != NULL) {
		if(strcmp(aux->nume, nume) == 0)
			break;
		if(aux->next == NULL)
			break;
		aux = aux->next;
	}
	Client_subscribe *aux2 = aux->lista_clienti_head;
	if(aux2 != NULL && strcmp(aux2->id, id) == 0) {
		aux->lista_clienti_head = aux2->next;
		free(aux2);
		return;
	}
	Client_subscribe *prev = aux2;
	while(aux2 != NULL) {
		if(strcmp(aux2->id, id) == 0)
			break;
		prev = aux2;
		aux2 = aux2->next;
	}
	if(aux2 == NULL)
		return;
	prev->next = aux2->next;
	free(aux2);

}

int main(int argc, char *argv[])
{
	setvbuf(stdout, NULL, _IONBF, BUFSIZ);
	const int valoare = 1;
	Topics *topics;
	Clienti_logati *id;
	int sockfd, newsockfd, portno;
	int sockudp;
	char buffer[1551];
	struct sockaddr_in serv_addr;
	struct sockaddr_in cli_addr;
	int n, i, ret;
	socklen_t clilen;

	fd_set read_fds;	// multimea de citire folosita in select()
	fd_set tmp_fds;		// multime folosita temporar
	int fdmax;			// valoare maxima fd din multimea read_fds

	if (argc < 2) {
		usage(argv[0]);
	}

	// se goleste multimea de descriptori de citire (read_fds) si multimea temporara (tmp_fds)
	FD_ZERO(&read_fds);
	FD_ZERO(&tmp_fds);

	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	DIE(sockfd < 0, "socket");

	sockudp = socket(AF_INET, SOCK_DGRAM, 0);
	DIE(sockudp < 0, "socket");

	portno = atoi(argv[1]);
	DIE(portno == 0, "atoi");

	memset((char *) &serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(portno);
	serv_addr.sin_addr.s_addr = INADDR_ANY;

	ret = bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(struct sockaddr));
	DIE(ret < 0, "bind");

	ret = listen(sockfd, MAX_CLIENTS);
	DIE(ret < 0, "listen");

	
	ret = bind(sockudp, (struct sockaddr *) &serv_addr, sizeof(serv_addr));
	DIE(ret < 0, "bind");
	char buf[2000];

	// se adauga noul file descriptor (socketul pe care se asculta conexiuni) in multimea read_fds
	FD_SET(sockfd, &read_fds);
	FD_SET(sockudp, &read_fds);
	FD_SET(0, &read_fds);

	if(sockfd > sockudp)
		fdmax = sockfd;
	else
		fdmax = sockudp;

	while (1) {
		tmp_fds = read_fds; 
		

		ret = select(fdmax + 1, &tmp_fds, NULL, NULL, NULL);
		DIE(ret < 0, "select");

		for (i = 0; i <= fdmax; i++) {
			if (FD_ISSET(i, &tmp_fds)) {
				if (i == sockudp) {

					memset(buf, 0, 2000);
					socklen_t cli_len = sizeof(struct sockaddr_in);
					int len = recvfrom(sockudp, &buf, 1551, 0, (struct sockaddr *) &cli_addr ,&cli_len);
					DIE(len < 0, "recv");
					sprintf(buf+1551, " %s:%d", inet_ntoa(cli_addr.sin_addr), ntohs(cli_addr.sin_port));
					char c = 0;
					if(buf[49] != 0) {
						c = buf[50];
						buf[50] = 0;
					}
					if(topics != NULL) {
						node_Topics *aux = topics->head;
						while(aux != NULL) {
							if(strcmp(buf, aux->nume) == 0) {
								if(buf[49] != 0) {
									buf[50] = c;
								}
								Client_subscribe *aux2 = aux->lista_clienti_head;
								while(aux2 != NULL) {
									char ack[5];
									if(aux2->online_status == 1) {
										send(aux2->socket, buf, 2000, 0);
										recv(aux2->socket, &ack, sizeof(ack), 0);		
									}
									else if(aux2->online_status == 0 && aux2->sf == 1) {
										if(aux2->head_messages == NULL) {
											aux2->head_messages = malloc(sizeof(Sf_messages));
											memcpy(aux2->head_messages->message, buf, 2000);
											aux2->head_messages->next = NULL;
										}
										else {
											Sf_messages *aux_mess = aux2->head_messages;
											while(aux_mess->next != NULL)
												aux_mess = aux_mess->next;
											Sf_messages *nou_mes = malloc(sizeof(Sf_messages));
											memcpy(nou_mes->message, buf, 2000);
											nou_mes->next = NULL;
											aux_mess->next = nou_mes;
										}
										strcpy(ack, "ACK");
									}
									if(aux2->next == NULL)
										break;
									if(strncmp(ack, "ACK", 3) == 0)
										aux2 = aux2->next;
								}
								break;
							}
							if(aux->next == NULL)
								break;
							aux = aux->next;
						}
					}
				}
				else if (i == 0) {
					char inchis[100];
					fgets(inchis, 10, stdin);
					if(strncmp(inchis, "exit", 4) == 0) {
						for(int x = 0; x <= fdmax; x++) {
							close(x);
							close(sockfd);
							close(sockudp);
						}
						break;
					}
				}
				else if (i == sockfd) {

					// a venit o cerere de conexiune pe socketul inactiv (cel cu listen),
					// pe care serverul o accepta
					clilen = sizeof(cli_addr);
					newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
					DIE(newsockfd < 0, "accept");

					setsockopt(newsockfd, IPPROTO_TCP, TCP_NODELAY, (char *)&(valoare), sizeof(int));

					if(newsockfd == 0)
						close(newsockfd);
					
					// se adauga noul socket intors de accept() la multimea descriptorilor de citire
					
					char id2[10];
					int contor = 0;
					recv(newsockfd, id2, sizeof(id2), 0);
				

					if(id != NULL) {
						node_Clienti_logati *clienti = id->head;
						while(clienti != NULL) {
							if(strcmp(clienti->id, id2) == 0) {
								if(clienti->online_status == 1) {
									printf("Client %s already connected.\n", id2);
									close(newsockfd);
								}	
								break;
							}
							if(clienti->next == NULL) {
								contor = 1;
								break;
							}
							clienti = clienti->next;
						}
						if(contor == 1) {
							FD_SET(newsockfd, &read_fds);
							if (newsockfd > fdmax) { 
								fdmax = newsockfd;
							}
							clienti_logati_add(id,newsockfd,id2);
							printf("New client %s connected from%s:%d.\n", id2, inet_ntoa(cli_addr.sin_addr), ntohs(cli_addr.sin_port));
						}
						else if(contor == 0 && clienti->online_status == 0) {
							FD_SET(newsockfd, &read_fds);
							if (newsockfd > fdmax) { 
								fdmax = newsockfd;
							}
							clienti->online_status = 1;
							clienti->socket = newsockfd;
							node_Topics *aux = topics->head;
							while(aux != NULL) {

								Client_subscribe *aux2 = aux->lista_clienti_head;
								while(aux2 != NULL) {
									if(strcmp(aux2->id, id2) == 0) {
										
										aux2->socket = newsockfd;
										Sf_messages *aux_mess = aux2->head_messages;
										while(aux_mess != NULL) {
											send(newsockfd, aux_mess->message, 2000, 0);
											if(aux_mess->next == NULL)
												break;
											aux_mess = aux_mess->next;
										}
										aux2->online_status = 1;
										break;
									}
									if(aux2->next == NULL)
										break;
									aux2 = aux2->next;
								}
								if(aux->next == NULL)
									break;
								aux = aux->next;
							}
							printf("New client %s connected from%s:%d.\n", id2, inet_ntoa(cli_addr.sin_addr), ntohs(cli_addr.sin_port));
						}
					}
					else {
						FD_SET(newsockfd, &read_fds);
						if (newsockfd > fdmax) { 
							fdmax = newsockfd;
						}
						clienti_logati_init(&id, newsockfd ,id2);
						printf("New client %s connected from%s:%d.\n", id2, inet_ntoa(cli_addr.sin_addr), ntohs(cli_addr.sin_port));
					}
				} else {

					// s-au primit date pe unul din socketii de client,
					// asa ca serverul trebuie sa le receptioneze
					char string[2000];
					n = recv(i, &string, sizeof(string), 0);
					char * str = strtok(string, "\n");
					strcpy(buffer,str);
					DIE(n < 0, "recv");
					if (n == 0) {
						// conexiunea s-a inchis
						if(topics != NULL) {
							node_Topics *aux = topics->head;
							while(aux != NULL) {
								Client_subscribe *aux2 = aux->lista_clienti_head;
								while(aux2 != NULL) {
									if(aux2->socket == i)
										aux2->online_status = 0;
									if(aux2->next == NULL)
										break;
									aux2 = aux2->next;
								}
								if(aux->next == NULL)
									break;
								aux = aux->next;
							}
						}
						node_Clienti_logati *clienti = id->head;
						while(clienti != NULL) {
							if(clienti->socket == i) {
								clienti->online_status = 0;
								break;
							}
							if(clienti->next == NULL)
								break;
							clienti = clienti->next;
						}

						printf("Client %s disconnected.\n", clienti->id);
						close(i);
						// se scoate din multimea de citire socketul inchis 
						FD_CLR(i, &read_fds);
					} 
					else {
						char *str = strtok(buffer, " ");
						if(strncmp(str, "subscribe", 9) == 0) {

							str = strtok(NULL, " ");
							if(topics != NULL && topic_exists(topics, str) == 1) {
								node_Clienti_logati *cl = id->head;
								while(cl != NULL) {
									if(cl->socket == i)
										break;
									cl = cl->next;
								}
								int sf = atoi(strtok(NULL, "\n"));
							//	printf("sf este: %d\n", sf);
								add_subscriber(topics, str, i, sf, cl->id);
							}
							else {
								node_Clienti_logati *cl = id->head;
								while(cl != NULL) {
									if(cl->socket == i)
										break;
									cl = cl->next;
								}
								int sf = atoi(strtok(NULL, "\n"));
							//	printf("sf este: %d\n", sf);
								if(topics == NULL) {	
									topics_init(&topics, str, i, sf, cl->id);
								}
								else {
									topics_add(topics, str, i, sf, cl->id);
								}
							}

						}
						else if(strncmp(str, "unsubscribe", 11) == 0) {
							str = strtok(NULL, "\n");
							strcat(str, "\0");
							node_Clienti_logati *cl = id->head;
							while(cl != NULL) {
								if(cl->socket == i)
									break;
								cl = cl->next;
							}
							remove_subscriber(topics, str, i, cl->id);
						}
					}
				}
			}
		}
	}

	close(sockfd);
	return 0;
}
