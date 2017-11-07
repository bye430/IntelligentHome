#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <mqueue.h>
#include <errno.h>
#define SOCK_BUF_SZ 30
#define SHM_BUF_SZ 2048
#define  BT_PATH "./bt"
#define SERVCLIENT_PATH "./serveclient"
void err_handling(char* message);
static void read_childproc(int sig);

 int main(int argc, char *argv[]) {
	int serv_sock, clnt_sock;
	struct sockaddr_in serv_adr, clnt_adr;
	int shm_id; /*共享内存标识符*/
	mqd_t mqd;     //posix消息队列标识符
	pid_t pid, pid2;
	struct sigaction act;
	socklen_t adr_sz;
	int strlen, state;
	char sock_buf[SOCK_BUF_SZ];
	if (argc != 2) {
		printf("Usage:%s <port>", argv[0]);
		exit(1);
	}
	//注册信号处理僵尸进程
	act.sa_handler = read_childproc;
	sigemptyset(&act.sa_mask);
	act.sa_flags = 0;
	state = sigaction(SIGCHLD, &act, 0);

	//创建共享内存
	shm_id = shmget(IPC_PRIVATE, SHM_BUF_SZ, 0666);
	if (shm_id < 0) {
		printf("shmget failed!\n");
		exit(1);
	}
	printf("create a shared memory segment successfully: %d \n", shm_id);
	char shmstring[25];
	sprintf(shmstring,"%d",shm_id);
	//创建消息队列
	mqd = mq_open("/contrl_message", O_RDONLY|O_CREAT,0666,NULL);
	if (mqd == (mqd_t) -1)
		err_handling("create message queue error");
	//创建子进程
	pid = fork();
	if (pid == -1)
		err_handling("create process error");
	else if (pid == 0) {
		execl(BT_PATH,shmstring, "/contrl_message",NULL);
	} else {     //父进程

		serv_sock = socket(PF_INET, SOCK_STREAM, 0);
		memset(&serv_adr, 0, sizeof(serv_adr));
		serv_adr.sin_addr.s_addr = htonl(INADDR_ANY);
		serv_adr.sin_family = AF_INET;
		serv_adr.sin_port = htons(atoi(argv[1]));
		if (-1== bind(serv_sock, (struct socckaddr*) &serv_adr,
						sizeof(serv_adr)))
			{
				printf("errno:%d\n",errno);
				err_handling("bind()error");
			}
		while (1) {
			adr_sz = sizeof(clnt_adr);
			clnt_sock = accept(serv_sock, &clnt_adr, &adr_sz);
			if (clnt_sock == -1)
				continue;
			else
				puts("new client connected");

			pid2 = fork();
			if (pid2 == -1) {
				close(clnt_sock);
				continue;
			}
			if (pid2 == 0) {
				close(serv_sock);
				execl(SERVCLIENT_PATH,(char)&shm_id, (char*) &clnt_sock,
						"/contrl_message",NULL);
			} else {
				close(clnt_sock);
			}
			close(serv_sock);
			return 0;
		}     //while
	}     // 父进程
}
void read_childproc(int sig) {
	pid_t pid;
	int status;
	pid = waitpid(-1, &status, WNOHANG);
	printf("removed process id:%d\n", pid);
}
void err_handling(char *message) {
	fputs(message, stderr);
	fputc('/n', stderr);
	exit(1);
}
