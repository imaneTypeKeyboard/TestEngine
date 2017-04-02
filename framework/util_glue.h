/*
 * =====================================================================================
 *
 *       Filename:  util_glue.h
 *
 *    Description:  杂类函数 
 *
 *        Version:  1.0
 *        Created:  03/23/2017 11:49:01 AM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  imane (), imanecr@gmail.com
 *   Organization:  
 *
 * =====================================================================================
 */
#ifndef _UTIL_GLUE_H_
#define _UTIL_GLUE_H_

#include <string>
#include <stdint.h>

typedef	void	Sigfunc(int);	/* for signal handlers */

enum ProcessType 
{
	MAIN_PROC = 0,                                /* main */
	WORLD_PROC                                /* logical */
};

enum TimerType
{
	TIMER_1S = 1,
	TIMER_3S = 3,
	TIMER_5S = 5,
	TIMER_15S = 15,
	TIMER_30S = 30,
	TIMER_1MIN = 60,
	TIMER_3MIN = 180,
	TIMER_5MIN = 300,
	TIMER_10MIN = 600,
	TIMER_30MIN = 1800,
};

int Fcntl(int fd, int cmd, int arg);
Sigfunc *SignalInner(int signo, Sigfunc *func);
Sigfunc *Signal(int signo, Sigfunc *func); /* for our signal() function */
int DaemonInit();
bool SetRLimit();
time_t GetMSTime();

void TELL_WAIT();
void TELL_PARENT(pid_t pid);
void WAIT_PARENT();
void TELL_CHILD(pid_t pid);
void WAIT_CHILD();

std::string Uint32ToString(uint32_t dwNum);
std::string Uint64ToString(uint64_t dwNum);
#endif
