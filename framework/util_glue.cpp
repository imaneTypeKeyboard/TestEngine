/*
 * =====================================================================================
 *
 *       Filename:  util_glue.cpp
 *
 *    Description:  杂类函数
 *
 *        Version:  1.0
 *        Created:  03/23/2017 11:48:57 AM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  imane (), imanecr@gmail.com
 *   Organization:  
 *
 * =====================================================================================
 */

#include "util_glue.h"

#include <stdarg.h>

#include <algorithm>

#include <syslog.h>
#include <time.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/resource.h>
#include <sys/time.h>

#include "Log.h"

#define	MAXFD	64

int Fcntl(int fd, int cmd, int arg)
{
	int	n;

	if ( (n = fcntl(fd, cmd, arg)) == -1)
	{
		IME_ERROR("fcntl error");
		return -1;
	}
	return(n);
}

int DaemonInit()
{
	int		i;
	pid_t	pid;

	if ( (pid = fork()) < 0)
		return (-1);
	else if (pid)
		_exit(0);			/* parent terminates */

	/* child 1 continues... */

	if (setsid() < 0)			/* become session leader */
		return (-1);

	Signal(SIGHUP, SIG_IGN);
	if ( (pid = fork()) < 0)
		return (-1);
	else if (pid)
		_exit(0);			/* child 1 terminates */

	//chdir("/");				/* change working directory */

	/* close off file descriptors */
	for (i = 0; i < MAXFD; i++)
		close(i);

	/* redirect stdin, stdout, and stderr to /dev/null */
	open("/dev/null", O_RDONLY);
	open("/dev/null", O_RDWR);
	open("/dev/null", O_RDWR);

	return (0);				/* success */
}

Sigfunc *SignalInner(int signo, Sigfunc *func)
{
	struct sigaction	act, oact;

	act.sa_handler = func;
	sigemptyset(&act.sa_mask);
	act.sa_flags = 0;
	if (signo == SIGALRM || signo == SIGTERM) {
#ifdef	SA_INTERRUPT
		act.sa_flags |= SA_INTERRUPT;	/* SunOS 4.x */
#endif
	} else {
#ifdef	SA_RESTART
		act.sa_flags |= SA_RESTART;		/* SVR4, 44BSD */
#endif
	}
	if (sigaction(signo, &act, &oact) < 0)
		return(SIG_ERR);
	return(oact.sa_handler);
}
/* end signal */

Sigfunc *Signal(int signo, Sigfunc *func)	/* for our signal() function */
{
	Sigfunc	*sigfunc;

	if ( (sigfunc = SignalInner(signo, func)) == SIG_ERR)
	{
		IME_ERROR("signal error");
		exit(1);
	}
	return(sigfunc);
}

bool SetRLimit()
{
	struct rlimit limit;
	limit.rlim_cur = RLIM_INFINITY;
	limit.rlim_max = RLIM_INFINITY;

	if(-1 == setrlimit(RLIMIT_CORE, &limit))
	{
		IME_ERROR("set limit core failed");
		return false;
	}

	struct rlimit limitF;
	limitF.rlim_cur = 1024*6;
	limitF.rlim_max = 1024*6;

	if(-1 == setrlimit(RLIMIT_NOFILE, &limitF))
	{
		IME_ERROR("set limit nofile failed");
		return false;
	}
	
	return true;
}

time_t GetMSTime()
{    
	struct timespec time1 = {0, 0};
	clock_gettime(CLOCK_MONOTONIC, &time1);
	return (time1.tv_sec * 1000) + (time1.tv_nsec / 1000000);
}

static volatile sig_atomic_t sigflag; /* set nonzero by sig handler */
static sigset_t newmask, oldmask, zeromask;

static void sig_usr(int signo)	/* one signal handler for SIGUSR1 and SIGUSR2 */
{
	sigflag = 1;
}

void TELL_WAIT(void)
{
	if (signal(SIGUSR1, sig_usr) == SIG_ERR)
	{
		IME_ERROR("signal(SIGUSR1) error");
		exit(1);
	}
	if (signal(SIGUSR2, sig_usr) == SIG_ERR)
	{
		IME_ERROR("signal(SIGUSR2) error");
		exit(1);
	}
	sigemptyset(&zeromask);
	sigemptyset(&newmask);
	sigaddset(&newmask, SIGUSR1);
	sigaddset(&newmask, SIGUSR2);

	/* Block SIGUSR1 and SIGUSR2, and save current signal mask */
	if (sigprocmask(SIG_BLOCK, &newmask, &oldmask) < 0)
	{
		IME_ERROR("SIG_BLOCK error");
		exit(1);
	}
}

void TELL_PARENT(pid_t pid)
{
	kill(pid, SIGUSR2);		/* tell parent we're done */
}

void WAIT_PARENT(void)
{
	while (sigflag == 0)
		sigsuspend(&zeromask);	/* and wait for parent */
	sigflag = 0;

	/* Reset signal mask to original value */
	if (sigprocmask(SIG_SETMASK, &oldmask, NULL) < 0)
	{
		IME_ERROR("SIG_SETMASK error");
		exit(1);
	}
}

void TELL_CHILD(pid_t pid)
{
	kill(pid, SIGUSR1);			/* tell child we're done */
}

void WAIT_CHILD(void)
{
	while (sigflag == 0)
		sigsuspend(&zeromask);	/* and wait for child */
	sigflag = 0;

	/* Reset signal mask to original value */
	if (sigprocmask(SIG_SETMASK, &oldmask, NULL) < 0)
	{
		IME_ERROR("SIG_SETMASK error");
		exit(1);
	}
}

std::string Uint32ToString(uint32_t dwNum)
{
	std::string strRet;
	if (dwNum == 0)
		return "0";
	while(dwNum)
	{
		int nNow = dwNum % 10;
		strRet.append(1,nNow - 0 + '0');
		dwNum /= 10;
	}

	reverse(strRet.begin(),strRet.end());
	return strRet;
}

std::string Uint64ToString(uint64_t dwNum)
{
	std::string strRet;
	if (dwNum == 0)
		return "0";
	while(dwNum)
	{
		int nNow = dwNum % 10;
		strRet.append(1,nNow - 0 + '0');
		dwNum /= 10;
	}

	reverse(strRet.begin(),strRet.end());
	return strRet;
}
