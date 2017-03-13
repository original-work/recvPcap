/*****************************************************************************
 * 版权所有(C)  2008-2018  LIANGJIANG TECH CO.,LTD.
 *----------------------------------------------------------------------------
 * 模 块 名 : TEST 
 * 文件名称 : Recv.cpp
 * 文件标识 : {[N/A]}
 * 内容摘要 : 从指定网卡收消息，然后在指定的时间间隔生成文件夹和文件
 * 注意事项 : {[N/A]}
 * 作    者 : 杨学荣，王欣鑫
 * 创建日期 : 2010-07-11 15:28
 * 当前版本 : Ver1.0
 *----------------------------------------------------------------------------
 * 变更记录 :
 *
 * $记录1
 * 变 更 单: $0000000(N/A)
 * 责 任 人: 杨学荣，王欣鑫
 * 修改日戳: 2010-07-11 15:28
 * 变更说明: 创建文件
 *
 *----------------------------------------------------------------------------
 */





#include <sys/socket.h>
#include <stdio.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <iostream>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <pthread.h>

#include <netinet/if_ether.h>
#include <linux/if_packet.h>
#include <linux/if.h>

#include <time.h>



#include <signal.h>
#include <time.h>
#include <errno.h>
#include <sched.h>
#include <stdio.h>
#include <libio.h>
#include <unistd.h>
#include <sys/param.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <stdarg.h>
#include <errno.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/time.h>
#include <fcntl.h>

using namespace std;
char g_devices[8];



#define FILE_HEAD_OFFSET 256
#define PACKET_HEAD_OFFSET 13

char g_fileHead[24] = {0xd4,0xc3,0xb2,0xa1,0x02,0x00,0x04,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
				     0xff,0xff,0x00,0x00,0x01,0x00,0x00,0x00};

typedef struct
{
	unsigned int time_sec;
	unsigned int time_nsec;
	unsigned int pkt_length;
	unsigned int cap_length;
	char msg[2048];
}IP_PACKET_MSG;


FILE * g_pFile;
char g_filename[64];
time_t timep;
struct tm *tm_val;

//char time_buff[8];
char g_date[50];
unsigned long g_diNum = 1;
int g_fd_log = 0;

int DIR_PER_TIME = 600;
#define TIME_ONE_DAY 86400
int g_mkdir_flag = 1;

//luchq add 2012-05-28
char version[50] = "$Name: V1_0_1_120703 $";

static char* levels[] =
{
	"EMERG",
	"ALERT",
	"CRITICAL",
	"ERROR",
	"WARNING",
	"NOTICE",
	"INFO",
	"DEBUG"
};



#define	EMERG  0
#define	ALERT 1
#define	CRITICAL 2
#define	ERR 3
#define	WARNING 4
#define	NOTICE 5
#define	INFO 6
#define	DEBUG 7



unsigned long old_serial_num = 1;
unsigned long serial_num = 1;


char* get_ctime(char *p_time)
{
	  char time_buff[64];
	  time_t timep;
	  struct tm *tm_val;
	  struct timeval      time_val;
	  char *delim = " ";
	  timep = time(NULL);
         tm_val = localtime(&timep);
	  strcpy(time_buff,ctime(&timep));
	  strtok(time_buff,delim);	
	  strtok(NULL,delim);
         strtok(NULL,delim);
	  p_time = strtok(NULL,delim);
         return p_time;
}



int open_log()
{
	static unsigned long log_serial_num=0;
	memset(g_filename,0,sizeof(g_filename));
	sprintf(g_filename,"/Recv_%d_%d_%d_%d.txt",tm_val->tm_year + 1900,tm_val->tm_mon + 1,tm_val->tm_mday,log_serial_num);
	g_fd_log = open(g_filename, O_CREAT|O_RDWR|O_TRUNC, S_IRWXU|S_IRWXG|S_IRWXO); 
	if (g_fd_log == -1)
	{
		printf("Open log file  %s faild...");
	}

	return 0;
}



void syslog_r(int priority, const char* format, ...)
{

	va_list ap;
	va_start(ap, format);
	static char buf[1024] = {'\0'};

	vsnprintf(buf, 1024, format, ap);
	va_end(ap);
	unsigned i = 0;
	char* level_str = levels[priority];
	unsigned buf_len = strlen(buf), level_len = strlen(level_str);
	buf[buf_len+level_len+2] = '\0';
	for ( i = buf_len+level_len+1; i > level_len+1; i-- )
	{
	    buf[i] = buf[i - level_len - 2];
	}
	buf[level_len+1] = ']';
	for ( i = 1; i <= level_len; i++ )
	{
	    buf[i] = level_str[i-1];
	}
	buf[0] = '[';

	static time_t 		cur_time;
	static struct tm 	*cur_tm;
	static char time_buf[64];

	cur_time 	= time(NULL);
	memset(time_buf, '\0', 64);

	strncpy(time_buf, ctime(&cur_time), 24);

	time_buf[24] = '\0';

	static char log_buf[1024] = {'\0'};
	snprintf(log_buf, 1024,"%s %s\n", time_buf, buf);

	// if exceeds 1M, just truncate this file.
	if (lseek(g_fd_log, 0, SEEK_CUR) > 15*1024*1024)
	{
		close(g_fd_log);
		open_log();
	}
	
	if ( g_fd_log != -1)
	{
    		write(g_fd_log, log_buf, strlen(log_buf));
	}
    	
}





void make_new_directory(unsigned long num)
{
	char file_name[64];
	char buf[100];
	
	timep = time(NULL);
	tm_val = localtime(&timep);

      	struct timespec tp;	

	memset(file_name,0,sizeof(file_name));
	sprintf(file_name,"mkdir -p /root/MPM-IP/data/%s/%d_%d_%d_%d",\
		g_devices,tm_val->tm_year + 1900,tm_val->tm_mon + 1,tm_val->tm_mday,num);

	sprintf(g_date,"%d_%d_%d_%d",tm_val->tm_year + 1900,tm_val->tm_mon + 1,tm_val->tm_mday,num);

	sprintf(buf,"file_name == %s\n", file_name);
	syslog_r(INFO, buf);	
	
	system(file_name);
	
}






void sig_handler(int signo)
{

       
		char buf[100];		
       	struct timespec tp;


		syslog_r(INFO, "processing......\n");
		//old_serial_num = serial_num;
		
		sprintf(buf,"serial_num == %d, old_serial_num = %d\n", serial_num, old_serial_num);
		syslog_r(INFO, buf);
		
		//get_time_interval();
		//g_time_interval=(TIME_ONE_DAY/g_num_of_file_oneday);

		sprintf(buf,"DIR_PER_TIME == %d\n", DIR_PER_TIME);
		syslog_r(INFO, buf);

		sprintf(buf,"((tp.tv_sec + 28800)%TIME_ONE_DAY) == %d\n", ((tp.tv_sec + 28800)%TIME_ONE_DAY));
		syslog_r(INFO, buf);
		
		clock_gettime(CLOCK_REALTIME,&tp);
		serial_num = ((tp.tv_sec + 28800)%TIME_ONE_DAY)/DIR_PER_TIME;
		
		sprintf(buf,"serial_num == %d, old_serial_num = %d\n", serial_num, old_serial_num);
		syslog_r(INFO, buf);
		
		if(serial_num != old_serial_num) //当序列号增加了。并且序列号大于零的时候开始move
		{
			syslog_r(INFO, "Recv process is still running normally\n");
			old_serial_num = serial_num;
			make_new_directory(serial_num);
		}
}



// 建立信号处理机制
void init_sigaction(void)
{

	struct sigaction tact;
	//信号到了要执行的任务处理函数为prompt_info

	tact.sa_handler = sig_handler;
	tact.sa_flags = 0;
	//初始化信号集
	sigemptyset(&tact.sa_mask);
	//建立信号处理机制
	sigaction(SIGALRM, &tact, NULL);

}


void init_time()
{

	struct itimerval value;

	//设定执行任务的时间间隔为2秒0微秒
	value.it_value.tv_sec = 30;

	value.it_value.tv_usec = 0;
	//设定初始时间计数也为2秒0微秒

	value.it_interval = value.it_value;
	//设置计时器ITIMER_REAL

	setitimer(ITIMER_REAL, &value, NULL);

}










int getRawSocketHandle(char *pdevice)
{
	int sockid;
	if((sockid = socket(PF_PACKET,SOCK_RAW,htons(ETH_P_ALL))) < 0)
	{
		printf("[Error]getRawSocketHandle : Creat socket : %s\n", strerror(errno));
		exit(0);
	}

	int s;

	printf("Get raw socket: %s\n",pdevice);

	struct sockaddr_ll addr;
	memset(&addr,0,sizeof(sockaddr_ll));

	addr.sll_family =PF_PACKET;

	struct ifreq ifr;
	memset(&ifr, 0, sizeof(ifreq));
	strncpy(ifr.ifr_name,pdevice,sizeof(ifr.ifr_name)-1);
	if(ioctl(sockid,SIOCGIFINDEX,&ifr) !=0)
	{
		printf("[Error]getRawSocketHandle : ioctl socket : %s\n", strerror(errno));
		exit(-1);
	}

	addr.sll_ifindex = ifr.ifr_ifindex;
	addr.sll_protocol = htons(ETH_P_ALL);

	if(bind(sockid,(struct sockaddr *)&addr,sizeof(addr))!=0)
	{
		printf("[Error]getRawSocketHandle : bind socket : %s\n", strerror(errno));
		exit(-1);
	}

//	ioctlsocket(sockid, SIO_RCVALL, &ifr);

	if ((s = ioctl(sockid, SIOCGIFFLAGS, &ifr)) < 0)
	{
		perror("ioctl SIOCGIFFLAGS ");
		exit(0);
	}
  
	ifr.ifr_flags |= IFF_PROMISC;
	if ((s = ioctl(sockid, SIOCSIFFLAGS, &ifr)) < 0)
	{
		perror("ioctl SIOCSIFFLAGS ");
		exit(0);	
	}

	return sockid;
}






int open_new_file()
{

	timep = time(NULL);
	tm_val = localtime(&timep);

	static unsigned long file_serial_num=0;

	memset(g_filename,0,sizeof(g_filename));
	sprintf(g_filename,"/root/MPM-IP/data/%s/%s/%d_%d_%d_%d.cap",\
		g_devices,g_date,tm_val->tm_year + 1900,tm_val->tm_mon + 1,tm_val->tm_mday,file_serial_num++);

	printf("file name = %s\n",g_filename);
	if((g_pFile = fopen(g_filename, "wb")) == NULL)
	{
		cout<<"open file error"<<endl;
		return 0;
	}
	fwrite((const void *)(g_fileHead), sizeof(g_fileHead), 1,g_pFile);
	return 0;
}


int write_file(char *msg_,int msgLen_)
{
	struct timespec tp;
    	int fd = fileno(g_pFile);
	char buf[100];
		
	if ((lseek(fd, 0, SEEK_CUR)) > (10*1024*1024))
	{
		fclose(g_pFile);
		
		clock_gettime(CLOCK_REALTIME,&tp);
		serial_num = ((tp.tv_sec + 28800)%TIME_ONE_DAY)/DIR_PER_TIME;
		sprintf(buf,"serial_num == %d, old_serial_num = %d\n", serial_num, old_serial_num);
		syslog_r(INFO, buf);
		open_new_file();
	}
	
	fwrite((const void *)(msg_), msgLen_, 1,g_pFile);
}


void init_daemon(void)
{
	int pid;
	int i;
	time_t t;
	FILE *fp;
	char message[25];

	
	if(pid=fork())          //是父进程，结束父进程
	{
		printf("init_daemon: 11111111  exit(0); \n");
		exit(0);                 
	}

	
	else if(pid< 0)
	{
		printf("init_daemon:  11111111  exit(1); \n");
		exit(1);               //fork失败，退出
	}

	else
	{
		/*
		是第一子进程，后台继续执行。
		第一子进程成为新的会话组长和进程组长
		并与控制终端分离
		*/
		printf("init_daemon: here we come; \n");
		
		setsid();

		if(pid=fork())
		{
			printf("init_daemon:  22222222222  exit(0); \n");
			exit(0);            //是第一子进程，结束第一子进程
		}
		else if(pid< 0)
		{
			printf("init_daemon:  22222222222  exit(1); \n");
			exit(1);          //fork失败，退出
		}
	       else
	      	{
		       /*
		       是第二子进程，不再是进程组长。继续
			*/

			printf("Here we are!!!");
			   
			for(i=0;i< NOFILE;++i)close(i);              //关闭打开的文件描述符
		   
			chdir("/tmp");                //改变工作目录到/tmp
			umask(0);                //重设文件创建掩模
			printf("init_daemon: fine; \n");
	       }

		   
	}
	printf("init_daemon: return; \n");
	return;
}



int main(int argc, char ** argv)
{


	IP_PACKET_MSG IppacketBuf;
	char *linebuf = (char *)(&(IppacketBuf.msg[0]));
	FILE * pFile;
	int a,b,c,d;
	char ip_buf[256];
	char buf[100];
	int  fd;
    	struct stat   f_stat;
    	bool res = true;
	unsigned long static_num = 0;
	unsigned long one_million = 0;

	struct timespec tp;
	/* luchq add 2012-05-28 for show version infomation */
	if ( (2==argc) && (!strcmp("ver",argv[1])) )
	{
		a = strlen(version);
		version[a-1] = '\0';
		printf("Version:Recv %s\n",&version[6]);
		return 1;
	}

	init_daemon();                //初始化为Daemon

	//open_log();

	strcpy(g_devices,argv[1]);
	DIR_PER_TIME = atoi(argv[2]);
	//printf("devices : %s , argv[1]: %s\n",g_devices,argv[1]);


	int nbyte = 0;
	int socketraw = getRawSocketHandle(g_devices);

	serial_num = ((tp.tv_sec + 28800)%TIME_ONE_DAY)/DIR_PER_TIME;
	sprintf(buf,"serial_num == %d, old_serial_num = %d\n", serial_num, old_serial_num);
	syslog_r(INFO, buf);
	make_new_directory(serial_num);
	clock_gettime(CLOCK_REALTIME,&tp);
	



	open_new_file();

	int noffset =0;
	//cout<<"+++++++++++++get message++++++++++++"<<endl;
	//(1) get filehead



	clock_gettime(CLOCK_REALTIME,&tp); //取得的是纳秒

	//printf("Sec = %d, nSec = %d\n",tp.tv_sec,tp.tv_nsec);
	//printf("time of day : %d\n",((tp.tv_sec + 28800)%86400));

	timep = time(NULL);
	tm_val = localtime(&timep);	

	open_log();

	init_sigaction();//初始信号机制

	init_time();//设置定时器时间


	while(1)
	{
		//cout<<"+++++++++++++while(1)++++++++++++"<<endl;

		nbyte = recvfrom(socketraw,linebuf,2048,0,0,NULL);
		if(nbyte >0)
		{
			//cout<<"+++++++++++++(nbyte >0)++++++++++++"<<endl;

			clock_gettime(CLOCK_REALTIME,&tp); //取得的是纳秒


			if(static_num<100*10000)
			{
				static_num++;
			}
			else
			{
				static_num=0;
				one_million++;				
				sprintf(buf,"one_million == %lu, static_num == %lu\n", one_million, static_num);
				syslog_r(INFO, buf);
			}
			


			IppacketBuf.pkt_length = nbyte;
			IppacketBuf.cap_length = nbyte;
			IppacketBuf.time_sec = tp.tv_sec;
			IppacketBuf.time_nsec = tp.tv_nsec;

       		write_file((char *)(&IppacketBuf),(nbyte + 16));
			
		}

	}

	 return 1;

}


