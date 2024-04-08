#include "Common.h"
#include <termio.h>
char *SERVERIP = (char *)"127.0.0.1";
#define SERVERPORT 9000
#define BUFSIZE    512

char buf[BUFSIZE + 1];
typedef struct {
    SOCKET server_sock;
    struct sockaddr_in serveraddr;
} UDPServer;

int getch(void) {
    int ch;
    
    struct termios buff;
    struct termios save;
    
    tcgetattr(0, &save);
    buff = save;
    
    buff.c_lflag &= ~(ICANON|ECHO);
    buff.c_cc[VMIN] = 1;
    buff.c_cc[VTIME] = 0;
    
    tcsetattr(0, TCSAFLUSH, &buff);
    
    ch = getchar();
    
    tcsetattr(0, TCSAFLUSH, &save);
    
    return ch;
}

void *udp_recv(void * arg) {
    UDPServer *udpserver = (UDPServer *)arg;
    SOCKET sock = udpserver -> server_sock;
    
    while (1) {
            
            // 데이터 받기
            buf[0] = '\0';
            socklen_t addrlen = sizeof(udpserver->serveraddr);
            int retval = recvfrom(sock, buf, BUFSIZE, 0, (struct sockaddr *)&udpserver->serveraddr, &addrlen);
            if (retval == SOCKET_ERROR) {
                err_display("recvfrom()");
                break;
            }
            else if (retval == 0)
                break;
            
            // 받은 데이터 출력
            buf[retval] = '\0';
            printf("server: %s\n", buf);

        }
        pthread_exit(NULL);
}

void *udp_send(void * arg) {
    UDPServer *udpserver = (UDPServer *)arg;
    SOCKET sock = udpserver -> server_sock;
    
    while(1){
        
        char car[2];
        buf[0] = '\0';
        
        while((car[0] = getch()) != '\n') {
            strcat(buf, car);
        }
        
        printf("client: %s\n", buf);
        
        
        // '\n' 문자 제거
        int len = (int)strlen(buf);
        if (buf[len - 1] == '\n')
            buf[len - 1] = '\0';
        if (strlen(buf) == 0)
            break;
        
        
        // 데이터 보내기
        int retval = sendto(sock , buf, len, 0, (struct sockaddr *)&udpserver->serveraddr, sizeof(udpserver->serveraddr));
        if (retval == SOCKET_ERROR) {
            err_display("sendto()");
            break;
        }
    }
    pthread_exit(NULL);
}

int main(int argc, char *argv[])
{
    int retval;

    // 명령행 인수가 있으면 IP 주소로 사용
    if (argc > 1) SERVERIP = argv[1];    

    // 소켓 생성
    SOCKET sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock == INVALID_SOCKET) err_quit("socket()");

    // 서버 주소 설정
    struct sockaddr_in serveraddr;
    memset(&serveraddr, 0, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    inet_pton(AF_INET, SERVERIP, &serveraddr.sin_addr);
    serveraddr.sin_port = htons(SERVERPORT);

    // 데이터 통신에 사용할 변수
    int len;
    UDPServer udpserver;
    udpserver.server_sock = sock;
    udpserver.serveraddr = serveraddr;
    
    pthread_t recvudp;
    pthread_t sendudp;
    
    pthread_create(&sendudp, NULL, udp_send, (void *)&udpserver);
    pthread_create(&recvudp, NULL, udp_recv, (void *)&udpserver);
        
    pthread_join(sendudp, NULL);
    pthread_join(recvudp, NULL);
    
    // 소켓 닫기
    close(sock);
    return 0;
}
