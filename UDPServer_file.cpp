#include "Common.h"

#define SERVERPORT 9000
#define BUFSIZE    3072
#define FILENAME "novel.txt"

int main(int argc, char *argv[])
{
	int retval;

	// 소켓 생성
	SOCKET sock = socket(AF_INET, SOCK_DGRAM, 0);
	if (sock == INVALID_SOCKET) err_quit("socket()");

	// bind()
	struct sockaddr_in serveraddr;
	memset(&serveraddr, 0, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
	serveraddr.sin_port = htons(SERVERPORT);
	retval = bind(sock, (struct sockaddr *)&serveraddr, sizeof(serveraddr));
	if (retval == SOCKET_ERROR) err_quit("bind()");

	// 데이터 통신에 사용할 변수
	struct sockaddr_in clientaddr;
	socklen_t addrlen;
	char buf[BUFSIZE + 1];
	
	
	// 클라이언트와 데이터 통신
	while (1) {
		// 데이터 받기
		addrlen = sizeof(clientaddr);
		retval = recvfrom(sock, buf, BUFSIZE, 0,
			(struct sockaddr *)&clientaddr, &addrlen);
		if (retval == SOCKET_ERROR) {
			err_display("recvfrom()");
			break;
		}
		
		if (strncmp(buf, "request \"novel.txt\"", 20) == 0) {
			printf("The server received a request from a client. \n");
			
			FILE *file = fopen(FILENAME,"r");
			if (file == NULL) {
				err_quit("fopen()");
			}
			
			// 데이터 보내기
			retval = sendto(sock, buf, BUFSIZE, 0,
				(struct sockaddr *)&clientaddr, sizeof(clientaddr));
			if (retval == SOCKET_ERROR) {
				err_display("sendto()");
				fclose(file);
				break;
			}
			
			fclose(file);
			
			// 받은 데이터 출력
			buf[retval] = '\0';
			char addr[INET_ADDRSTRLEN];
			inet_ntop(AF_INET, &clientaddr.sin_addr, addr, sizeof(addr));
			printf("[UDP/%s:%d] %s\n The server sent \"novel.txt\" to the client \n", addr, ntohs(clientaddr.sin_port), buf);
			
			
		} else {
			// 데이터 보내기
			retval = sendto(sock, buf, retval, 0,
				(struct sockaddr *)&clientaddr, sizeof(clientaddr));
			if (retval == SOCKET_ERROR) {
				err_display("sendto()");
				break;
			}
			
			// 받은 데이터 출력
			buf[retval] = '\0';
			char addr[INET_ADDRSTRLEN];
			inet_ntop(AF_INET, &clientaddr.sin_addr, addr, sizeof(addr));
			printf("[UDP/%s:%d] %s\n", addr, ntohs(clientaddr.sin_port), buf);

		}
	}
	// 소켓 닫기
	close(sock);
	return 0;
}
