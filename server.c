#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <string.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#define DEFAULT_LISTEN_PORT 8889

#define STATE_WAITING 0
#define STATE_CMD 1

#define CMD_ADD 1
#define CMD_MULT 2
#define CMD_QUIT 0

#define handle_error(msg) do { perror(msg); exit(EXIT_FAILURE); } while (0)

/**
* Execute add cmd
*/
int do_add(char* data, int n) {
	int result = 0;
	char *token;

	token = strtok(data, " ");
	while (token != NULL) {
		result += atoi(token);

		token = strtok(NULL, " ");
	}

	return result;
}

/**
* Execute mult cmd
*/
int do_mult(char* data, int n) {
	int result = 1;
	char *token;

	token = strtok(data, " ");
	while (token != NULL) {
		result *= atoi(token);

		token = strtok(NULL, " ");
	}

	return result;
}

/**
* Parse cmd for client
*/
int handle_cmd(char *data, int *param) {
	
	int result;
	char *token;
	
	*param = 0;

	token = strtok(data, " ");
	
	if (strcmp(token, "ADD") == 0) {
		result = CMD_ADD;
		*param = atoi(strtok(NULL, " "));
	} else if (strcmp(token, "MULT") == 0) {
		result = CMD_MULT;
		*param = atoi(strtok(NULL, " "));
	}
	else if (strcmp(token, "FIN") == 0) {
		result = CMD_QUIT;
	} else {
		result = -1;
	}

	return result;
}

/**
* Execute cmd with data
*/
int execute_cmd(int cmd, int arg, char *data) {
	
	int result;

	switch (cmd) {
	
		case CMD_ADD:
			result = do_add(data, arg);

		break;

		case CMD_MULT:
			result = do_mult(data, arg);
		break;

		default:
			result = 0;
	}

	return result;
}

/**
* Send OK to client
*/
void sendOk(int soc) { 
	send(soc, "OK", sizeof(char)*2, 0); 
}

/**
* Send cmd result to client
*/
void sendResult(int soc, int result) {
	char buf[1500];

	sprintf(buf, "%d", result);

	send(soc, buf, sizeof(buf), 0);
}

/**
* Handle a client connection
*/
void handle_client(int soc) {
	
	int state = STATE_WAITING;
	int command; 
	int arg;

	char buf[1500];

	while (1) {
		
		bzero(buf, sizeof(buf));
		if (recv(soc, buf, sizeof(buf), 0) == -1) {
			handle_error("recv");
		}

		printf("[RECV]%s\n", buf);

		switch(state) {
	
			case STATE_WAITING:
				if ((command = handle_cmd(buf, &arg)) != -1) {
					if (command != CMD_QUIT) {
						sendOk(soc);
						state = STATE_CMD;
					}
				}
				break;

			case STATE_CMD:
				sendResult(soc, execute_cmd(command, arg, buf));
				state = STATE_WAITING;
				break;

			default:
				state = STATE_WAITING;
		}

		if (command == CMD_QUIT) {
			return; 
		}
	}
}

int main(int argc, char** argv) {

	int s, s_client, ret;
	
	struct sockaddr_in addr;
	struct sockaddr_storage recept;
	socklen_t recept_len = sizeof(recept);

	char data_buf[1500], renvoi[1500], host[1024], service[20], *token;
	int result;
	
	if (argc != 2) {
		fprintf(stderr, "Usage: server <port>\n");
		exit(EXIT_FAILURE);
	}

	if ((s = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		handle_error("socket");
	}
	
	addr.sin_family = AF_INET;
	addr.sin_port = htons(atoi(argv[1]));
	addr.sin_addr.s_addr=INADDR_ANY;

	if (bind(s, (struct sockaddr *)&addr, sizeof(struct sockaddr_in)) != 0) {
		handle_error("bind");
	}

	if (listen(s, 5) != 0) {
		handle_error("listen");
	}

	printf("Server listening on port %d\n", htons(addr.sin_port));

	while (1) {
		if ((s_client = accept(s, (struct sockaddr *)&recept, &recept_len)) == -1) {
			handle_error("accept");
		}
		
		if ((ret = getnameinfo((struct sockaddr *)&recept, sizeof(recept), host, sizeof(host), service, sizeof(service), 0)) != 0) {
			fprintf(stderr, "getnameinfo: %s \n", gai_strerror(ret));
			exit(EXIT_FAILURE);
		}
		
		printf("Connexion from %s on port %s\n", host, service);
		
		handle_client(s_client);
		close(s_client);
		puts("Client disconected");
	}
}

