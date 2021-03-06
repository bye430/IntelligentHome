#include <stdio.h>
#include <mqueue.h>
#include <sys/types.h>
#include <sys/shm.h>
#include <pthread.h>
#define BUF_SIZE  256
void err_handling(char *message) {
	fputs(message, stderr);
	fputc('/n', stderr);
	exit(1);
}
 int main(int argc, char* argv[]) {
	int mqd, clnt_sock, str_len = 0;
	char buf[BUF_SIZE];
	clnt_sock = (int) *argv[3];
        printf("I'm serveclient process %d\n",getpid());
	if ((mqd = mq_open(argv[4], O_WRONLY)) == -1)
		err_handling("mq_open error");
	while ((str_len+=read(clnt_sock,buf,BUF_SIZE))!=0)
	{

		if (mq_send(mqd, buf, BUF_SIZE, 0) == -1)
			err_handling("mqsend error");
	}
	exit(0);
}

