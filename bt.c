#include<stdio.h>
#include<mqueue.h>
#include<sys/types.h>
#include <sys/shm.h>
#include <pthread.h>
#include<stdlib.h>
#include<errno.h>
#include<unistd.h>
#define BUFSIZ 512
#define TEXT_SZ 1024

struct shared_use_st {
	int ready; //作为一个标志，0：表示有数据 ,非 0:无数据
	char text[TEXT_SZ]; //记录写入和读取的文本
};
void* ReadControlMessageThread(void *arg);
extern void err_handling(char*);
void err_handling(char *message) {
	fputs(message, stderr);
	fputc('/n', stderr);
	exit(1);
}
int main(int argc, char* argv[]) {
	int shm_id; /*共享内存标识符*/
	struct shared_use_st *shared = NULL;
	char *result;
	
	//if (argc != 3)
		//err_handling("usage error");
	printf("argc:%d\n",argc);
        printf("argv:%s\n%s\n",argv[0],argv[1]);
	
	printf("I'm process%d\n",getpid());
	//创建一个线程
	int ntid;
	if (pthread_create(&ntid, NULL, ReadControlMessageThread,argv[1]) != 0)
		err_handling("thread create error");
	//向共享内存写
	shm_id =atoi(argv[0]);
	printf("child shmid:%d\n",shm_id);
	if((shared = shmat(shm_id, 0, 0))==(void*)-1)
		{
		printf("errno:%d\n",errno);
		err_handling("shmat error\n");
		}

	
	while (1) {
		result = "temperature:9";
		puts("update sharedmemery...\n");
		shared->ready = 0;
		//read(STDIN_FILENO,shared->text,TEXT_SZ);
		strncpy(shared->text, result, sizeof(result));
		puts("update sharedmemery  OK\n");
		sleep(5);
		}
	if (shmdt(shared) == -1) {
		err_handling("close shm failed\n");
	}
	return 0;
}
void* ReadControlMessageThread(void *argv) {
	struct mq_attr attr;
	int mqd ; //posix消息队列标识符
	if ((mqd = mq_open((char*)argv, O_RDONLY)) == -1)
      err_handling("mq_open error");
	if ((mq_getattr(mqd, &attr)) == -1)
		err_handling("mq_getattr");
	long len = attr.mq_msgsize;

	//从消息队列中取出消息
	//unsigned int prio;
	ssize_t n;
	char *buff = (char *) malloc(attr.mq_msgsize * sizeof(char));
while(1){	
	if ((n = mq_receive(mqd, buff, len, 0)) == -1)//没有消息就等待
		err_handling("mq_receive error");
	puts(buff);//输出控制消息
}
return (void *)0;
}
