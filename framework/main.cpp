#include <iostream>
#include <vector>
#include <string>
#include <map>

#include <stdarg.h>
#include <signal.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "Log.h"
#include "util_glue.h"
#include "handler.h"

static const char version[] = "1.0.0-20170328";
bool gDaemonFlag = false;
std::string g_prog_name;

static char *arg_start;
static char *arg_end;
static char *env_start;

volatile bool g_stopEvent = false;

struct TestConfig
{
	int work_num;
	int type;
	int rate;
};

bool InitServer()
{
	srand(time(NULL));

	if (!HandlerInit(MAIN_PROC))
	{
		IME_ERROR("man thread handler init fail");
		return false;
	}

	return true;
}

void EndServer()
{
	HandlerFinally(MAIN_PROC);
}

// Handle termination signals
// Put the World::m_stopEvent to 'true' if a termination signal is caught 
void OnSignal(int s)
{
	switch (s)  
	{ 
		case SIGINT: 
		case SIGQUIT: 
		case SIGTERM: 
//		case SIGABRT:
		case SIGUSR1:
			//sWorld->AllOffline();
			g_stopEvent = true;
			break; 
	} 
}
// Define hook '_OnSignal' for all termination signalsi
void HookSignals()
{    
	Signal(SIGINT, OnSignal);
	Signal(SIGQUIT, OnSignal);
	Signal(SIGTERM, OnSignal);
	Signal(SIGABRT, OnSignal);
	Signal(SIGPIPE, SIG_IGN);
	Signal(SIGUSR1, OnSignal);
}
/// Unhook the signals before leaving
void UnHookSignals()
{    
	Signal(SIGINT, SIG_DFL);
	Signal(SIGQUIT, SIG_DFL); 
	Signal(SIGTERM, SIG_DFL);
	Signal(SIGABRT, SIG_DFL);
	Signal(SIGUSR1, SIG_DFL);
}
void ShowBanner()
{
    IME_SIMPLE_LOG("TestEngine %s, report bugs to <631800266@qq.com>", version);
    IME_SIMPLE_LOG("Compiled at %s %s\n", __DATE__, __TIME__);
}

void ShowUsage(const char *msg)
{
	IME_SIMPLE_LOG("TestEngine %s", version);
	IME_SIMPLE_LOG("Usage: TestEngine [options]");
	IME_SIMPLE_LOG("options:\n"
"  -d		start as a daemon, default is false\n"
"  -c <number>	client number default is one\n"
"  -t <flag>	client model, flag = con means concurrence or means client number increases the maximum from zero in progress(default) \n"
"  -r X		cost X milliseconds every client is only useful for default client model\n"
"  -h		help\n"
);
	if (msg[0] != 0)
		IME_SIMPLE_ERROR("%s", msg);
    exit (0);
}

void daemon_set_title(const char* fmt, ...)
{   
    char title [64];
    int i, tlen;
    va_list ap;

    va_start(ap, fmt);
    vsnprintf (title, sizeof (title) - 1, fmt, ap);
    va_end (ap);
        
    tlen = strlen (title) + 1;
    if (arg_end - arg_start < tlen && env_start == arg_end) {
        char *env_end = env_start;
        for (i=0; environ[i]; i++) {
            if(env_end == environ[i]) {
                env_end = environ[i] + strlen (environ[i]) + 1;
                environ[i] = strdup(environ[i]);
            } else {
                break;
            }
        }
        arg_end = env_end;
        env_start = NULL;
    }

    i = arg_end - arg_start;
    if (tlen == i) {
        strcpy (arg_start, title);
    } else if (tlen < i) {
        strcpy (arg_start, title);
        memset (arg_start + tlen, 0, i - tlen);
    } else {
        stpncpy(arg_start, title, i - 1)[0] = '\0';
    }
}

void ParseArgs(int argc, char **argv, TestConfig &conf)
{
    g_prog_name = argv[0];

	int c;
	opterr = 0;		/* don't want getopt() writing to stderr */
	while ( (c = getopt(argc, argv, "c:t:r:hd")) != -1) {
		switch (c) {
		case 'c':
			conf.work_num = atoi(optarg);
			break;

		case 't':
			if (strcmp(optarg, "con") == 0) {
				conf.type = 1;
				break;
			}
			else
			{
				conf.type = 0;
			}
			break;

		case 'r':			/* address family */
			conf.rate = atoi(optarg);
			break;

		case 'h':			/* host */
			ShowUsage("");
			break;

		case 'd':			/* host */
			gDaemonFlag = true;
			break;

		case '?':
			ShowUsage("unrecognized option");
			break;
		}
	}
    ShowBanner();
}

pid_t CreateWork(int argc, char *argv[], TestConfig conf)
{
    pid_t p = fork();
    if (p < 0)
    {
        IME_ERROR("work fork failed");
        return -1;
    }
    else if (p != 0)
    {
        return p;
    }
    else
    {
		if (conf.type)
			WAIT_PARENT();

        daemon_set_title("%s-WORKER", g_prog_name.c_str());

		if (!HandlerInit(WORLD_PROC))
		{
			IME_ERROR("worker process pid %u handler init", p);
			exit(1);
		}

        HandlerWork();

        if (!HandlerFinally(WORLD_PROC))
            IME_ERROR("handler finally fail %u", p);
            
        exit(0);
    }
}

int main(int argc, char * argv[])
{
    arg_start = argv[0];
    arg_end = argv[argc-1] + strlen (argv[argc - 1]) + 1;
    env_start = environ[0];

	TestConfig conf;
	conf.work_num = 10;
	conf.type = 0;
	conf.rate = 10;

    std::map<pid_t, int> childs;

	ParseArgs(argc, argv, conf);

	if (gDaemonFlag)
	{
		DaemonInit();
	}
	
	sLog->Initialize();

	if (!InitServer())
	{
		IME_ERROR("InitServer failed");
		return 0;
	}

	IME_SIMPLE_LOG("TestEngine Start!");
	
	//hooksignal
	HookSignals();

	if (conf.type == 1)
		TELL_WAIT();	
	///- Launch
	time_t pre = 0, now = 0, interval = 0;
    for (int i = 0; i < conf.work_num; ++i) {
        pid_t p = CreateWork(argc, argv, conf);
        if (-1 == p)
            continue;
        childs[p] = WORLD_PROC;

		// adjust child process launch speed
		if (conf.type ==  0)
		{
			now = GetMSTime();
			interval = now - pre;
			if (interval < conf.rate)
			{
				usleep((conf.rate - interval) * 1000);
				pre += conf.rate;
			}
			else
			{
				pre = now;
			}
			//IME_SIMPLE_LOG("pre %ld now %ld interval %ld rate %d", pre, now, interval, conf.rate);
			if (i % 100 == 0)
			{
				IME_SIMPLE_LOG("launch %d process", i);
			}
		}

		if (g_stopEvent)
			break;
    }

    daemon_set_title("%s-MAIN", g_prog_name.c_str());

	// let child process exec in the same time
	if (conf.type)
	{
    	std::map<pid_t, int>::iterator it;
		for (it = childs.begin(); it != childs.end(); it++)
		{
			TELL_CHILD(it->first);
		}
	}

    while (!g_stopEvent && childs.size() != 0) {
        int status;
        pid_t p; 
        p = waitpid(-1, &status, 0);
        if(-1 == p)
            continue;
        childs.erase(p);
    }

	std::map<pid_t, int>::iterator it;                    
	for (it = childs.begin(); it != childs.end(); ++it) {
		kill((*it).first, SIGTERM);
	}

    /*
     * exit
     */
    while (childs.size() != 0) {
        int status;
        pid_t p; 
        p = waitpid(-1, &status, 0);
        if(-1 == p)
            continue;
        
        childs.erase(p);
    }

	//unhooksignal	
	UnHookSignals();
	EndServer();
	IME_SIMPLE_LOG("TestEngine End!");

	return 0;
}
