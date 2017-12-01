#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#include <netinet/in.h>
#include <netdb.h>
#include <sys/socket.h>
#include <string.h>



#define handle_error(msg) do { perror(msg); exit(EXIT_FAILURE); } while (0)

#define ADD 1
#define MULT 2
#define CMD_QUIT 0

int readint() {
	char buf[128];
	fgets(buf, 128, stdin);

	buf[strlen(buf) - 1] = '\0';

  return atoi(buf);
}

int send_cmd(int soc, unsigned short cmd, int *arg) {
	char buf[1500];
	char response[3];
	int cmp = -1;

	bzero(buf, sizeof(buf));

	switch (cmd) {
		
		case ADD:
			printf("Addition de combien de nombres ? : ");
			*arg = readint();
			sprintf(buf, "ADD %d", *arg);		
		break;

		case MULT:
			printf("Multiplication de combien de nombres ? : ");
			*arg = readint();
			sprintf(buf, "MULT %d", *arg);
		break;

		default:
			return -1;
	}

	if (send(soc, buf, sizeof(buf), 0) == -1) {
		handle_error("send");
	}

	if (recv(soc, response, sizeof(response), 0) == -1) {
		handle_error("recv");
	}

	if (strcmp("OK", response) == 0)
	{
		cmp = 1;
	}
	else {
		cmp = 0;
	}

	return cmp;
}

int send_data(int soc, int n) {
	int i;
	char buf_send[1500];
	char buf_recv[1500];
	char val[128];

	bzero(buf_send, sizeof(buf_send));
	bzero(buf_recv, sizeof(buf_recv));

	for (i = 0; i < n; ++i) {
		printf("Nombre %d : ", i+1);
		sprintf(val, "%d ", readint());
		strcat(buf_send, val);
	}

	send(soc, buf_send, sizeof(buf_send), 0);
	recv(soc, buf_recv, sizeof(buf_recv), 0);
	
	return atoi(buf_recv);
}

void send_quit(int soc) {
	char fin[3] = "FIN";

	send(soc, fin, sizeof(fin), 0);
}

int loop(int soc) {
	
	int choice, arg, ret;

	puts("\t1) ADD");
	puts("\t2) MULT");
	puts("\t0) QUIT");
	
	choice = readint();
	if (send_cmd(soc, choice, &arg) == 1) {	
		printf("Résultat = %d\n\n", send_data(soc, arg));
	}
	
	return choice;
}

int main(int argc, char **argv) {
	
	int s, ret_code;

	struct sockaddr_in addr;
	struct hostent *host;
	
	if (argc != 3) {
		fprintf(stderr, "Usage: client <hostname> <port>\n");
		exit(EXIT_FAILURE);
	}

	addr.sin_port = htons(atoi(argv[2]));
	addr.sin_family = AF_INET;
	
	host = (struct hostent *)gethostbyname(argv[1]);
	bcopy((char *)host->h_addr, (char*)&addr.sin_addr, host->h_length);

	if ((s = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		handle_error("socket");
	}

	if (connect(s, (struct sockaddr *)&addr, sizeof(struct sockaddr_in)) < 0) {
		handle_error("connect");
	}

	puts("Connected");

	do {
	} while (loop(s)); 
	
	send_quit(s);
	close(s);
}


