/*****************************************************************************
 * 版权所有(C)  2008-2018  LIANGJIANG TECH CO.,LTD.
 *----------------------------------------------------------------------------
 * 模 块 名 : TEST 
 * 文件名称 : timer_zip_mv_rm.cpp
 * 文件标识 : {[N/A]}
 * 内容摘要 : 定时g_time_interval 从orig_path 压缩文件夹, 然后然后移动压缩后的文件夹到dest_path
 * 注意事项 : {[N/A]}
 * 作    者 : 王欣鑫
 * 创建日期 : 2010-07-10 14:51
 * 当前版本 : Ver1.0
 *----------------------------------------------------------------------------
 * 变更记录 :
 *
 * $记录1
 * 变 更 单: $0000000(N/A)
 * 责 任 人: 王欣鑫
 * 修改日戳: 2010-07-10 14:51
 * 变更说明: 创建文件
 *
 *----------------------------------------------------------------------------
 */





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
#include <iostream>
#include <stdarg.h>
#include <errno.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/time.h>
#include <fcntl.h>





char	zipfile_name[30];



char* in_param_1;
int     in_param_2;
char* in_param_3;


//----------------------------------------------------------------------------------------//



char g_filename[64];
int g_fd_log = 0;
//char time_buff[8];
time_t timep;
struct tm *tm_val;


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


#define TIME_ONE_DAY 86400
//int g_num_of_file_oneday=144;//10 mins per folder
int g_time_interval=0;

unsigned long old_serial_num = 1;
unsigned long serial_num = 1;
bool g_onedaypassed = 0;

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
		printf("Open log file  %s faild...", "/Recv.log");
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
	if (lseek(g_fd_log, 0, SEEK_CUR) > 1*1024*1024)
	{
		close(g_fd_log);
		open_log();
	}
	
	if ( g_fd_log != -1)
	{
    		write(g_fd_log, log_buf, strlen(log_buf));
	}
    	
}



//----------------------------------------------------------------------------------------//




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



void compress_move(char* origi_path, int num, char* dest_path)
{
	char cmd[100];
	char zipfile_temp[30];
	char folder[30];
	char buf[100];

	timep = time(NULL);
	tm_val = localtime(&timep);


	memset(zipfile_temp,0,sizeof(zipfile_temp));
	if(g_onedaypassed==1)
	{
		sprintf(buf,"g_onedaypassed==11111111111\n");
		syslog_r(INFO, buf);

		sprintf(zipfile_temp,"%d_%d_%d_%d.tar.gz",tm_val->tm_year + 1900,tm_val->tm_mon + 1,(tm_val->tm_mday-1),num);
		sprintf(folder,"%d_%d_%d_%d",tm_val->tm_year + 1900,tm_val->tm_mon + 1,(tm_val->tm_mday-1),num);		
		g_onedaypassed = 0;
	}
	else
	{
		sprintf(buf,"g_onedaypassed==0000000000\n");
		syslog_r(INFO, buf);
		
		sprintf(zipfile_temp,"%d_%d_%d_%d.tar.gz",tm_val->tm_year + 1900,tm_val->tm_mon + 1,tm_val->tm_mday,num);
		sprintf(folder,"%d_%d_%d_%d",tm_val->tm_year + 1900,tm_val->tm_mon + 1,tm_val->tm_mday,num);
	}
	syslog_r(INFO, "zipfile_name == %s  folder == %s\n", zipfile_temp, folder);

	

	sprintf(cmd, "sh /root/recvCap/tar_mv_rm.sh -n %s -f %s -d %s -o %s", zipfile_temp, folder, dest_path, origi_path);
	syslog_r(INFO, "cmd == %s\n", cmd);
	system(cmd);

}




void sig_handler(int signo)
{

       
		char buf[100];		
       	struct timespec tp;


		clock_gettime(CLOCK_REALTIME,&tp);
		
		syslog_r(INFO, "processing......\n");
		//old_serial_num = serial_num;
		
		sprintf(buf,"serial_num == %d, old_serial_num = %d\n", serial_num, old_serial_num);
		syslog_r(INFO, buf);
		
		sprintf(buf,"g_time_interval == %d\n", g_time_interval);
		syslog_r(INFO, buf);

		sprintf(buf,"((tp.tv_sec + 28800)%TIME_ONE_DAY) == %d\n", ((tp.tv_sec + 28800)%TIME_ONE_DAY));
		syslog_r(INFO, buf);
		
		


		if(((tp.tv_sec + 28800)%TIME_ONE_DAY)/g_time_interval == 0)
		{
			sprintf(buf, "11111111111111\n" );
			syslog_r(INFO, buf);
			serial_num = (TIME_ONE_DAY/g_time_interval)-1;
			g_onedaypassed = 1;
		}
		else if(((tp.tv_sec + 28800)%TIME_ONE_DAY)/g_time_interval == 1)
		{
			sprintf(buf, "22222222222222\n" );
			syslog_r(INFO, buf);
			serial_num = 0;
			g_onedaypassed = 1;
		}
		else
		{
			sprintf(buf, "33333333333333\n" );
			syslog_r(INFO, buf);
			serial_num = (((tp.tv_sec + 28800)%TIME_ONE_DAY)/g_time_interval)-1;
			g_onedaypassed = 0;
		}
		
		
		sprintf(buf,"serial_num == %d, old_serial_num = %d\n", serial_num, old_serial_num);
		syslog_r(INFO, buf);
		
		if(serial_num != old_serial_num) //当序列号增加了。并且序列号大于零的时候开始move
		{
			syslog_r(INFO, "Timer_zip_mv_rm process is still running normally\n");
			compress_move(in_param_1, old_serial_num, in_param_3);
			old_serial_num = serial_num;
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



int main(int argc, char ** argv)
{

	//usage ./a.out origi_path g_time_interval dest_path

	timer_t timer;
       struct itimerspec timeout;
       sigset_t block;
       struct sched_param param;
	struct timespec tp;
	char buf[100];


	
	if(argc!=4)
	{
		printf("usage: ./executable origi_path g_time_interval dest_path\n");
		return 0;
	}



	init_daemon();                //初始化为Daemon



//----------------------------------------------------------------------------------//



	g_time_interval=atoi(argv[2]);


	
	//memset(time_buff,0,8);
	//strcpy(time_buff,get_ctime(time_buff)); 	
	timep = time(NULL);
	tm_val = localtime(&timep);	

	open_log();



	clock_gettime(CLOCK_REALTIME,&tp);



	if(((tp.tv_sec + 28800)%TIME_ONE_DAY)/g_time_interval == 0)
	{
		serial_num = (TIME_ONE_DAY/g_time_interval)-1;
	}
	else
	{
		serial_num = (((tp.tv_sec + 28800)%TIME_ONE_DAY)/g_time_interval)-1;
	}
	
	old_serial_num = serial_num;
	
	sprintf(buf,"serial_num == %d, old_serial_num = %d\n", serial_num, old_serial_num);
	syslog_r(INFO, buf);
	

	in_param_1=argv[1];
	in_param_3=argv[3];
	

//----------------------------------------------------------------------------------//

	init_sigaction();//初始信号机制

	init_time();//设置定时器时间



	

	while ( 1 )
	{
	
		
		
	};

	exit(0);

	   
}	   
