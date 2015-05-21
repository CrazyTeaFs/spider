#include <sys/socket.h>
#include <execinfo.h>
#include <signal.h>
#include <unistd.h>
#include <iostream>
#include <strings.h>

#include "server.h"
#include "event_driver.h"
#include "timer.h"
#include "fsm_container.h"
#include "log.h"
#include "ini.h"
#include "message.pb.h"

static const char *version = "Spider0.1.0";
extern LogLevel Levelname[LOG_DEBUG + 1];

using namespace std;

int bar(void* data) {
	INFO("Now %s", now_str().c_str());
	return 0;
}

int flush_log(void* data) {
	Log::Instance()->Flush();
	return 0;
}

void flush_log(void) {
	Log::Instance()->Flush();
}

void ctrl_c_handler(int signal) {
	EMERG("Process Interupted By Signal(%d) SIGINT(CTRL+C)", signal);
	printf("Process Interupted By SIGINT(CTRL+C)\n");
	Log::Instance()->Flush();
	exit(EXIT_FAILURE);
}

void dump_stacktrace(int signal) {
	printf("Signal(%d) Segmentation Fault!\n", signal);
	CRIT("Signal(%d) Segmentation Fault!", signal);
	// Flush Log
	Log::Instance()->Flush();

	void *stack[64];
	size_t size;
	char **symbols;
	size = backtrace(stack, 64);
	symbols = backtrace_symbols(stack, size);

	if (symbols == NULL) {
		CRIT("No Backtrace Symbols");
		perror("No Backtrace Symbols");
		exit(EXIT_FAILURE);
	}

	printf("Obtained %lu Stack Frames\n", size);
	CRIT("Obtained %lu Stack Frames", size);
	for (size_t i = 0; i < size; i++) {
		CRIT("%s", symbols[i]);
		printf("%s\n", symbols[i]);
	}

	free(symbols);
	CRIT("Process Abort!");
	Log::Instance()->Flush();
	printf("Process Abort!\n");
	exit(EXIT_FAILURE);
}

// Config Log And Listenning Port 
int init_config(int &port) {
	Ini config;
	if (config.LoadFile("../conf/server.ini") != 0) {
		printf("Failed To Initialize Configuration File, Process Abort\n");
		exit(EXIT_FAILURE);
	}

	int ret = 0;
	Loglevel_t level = LOG_DEBUG;
	string logprefix, path, suffix, loglevel;
	ret = config.GetStringKey("log", "prefix", logprefix);
	if (ret != 0) {
		printf("Failed To Initialize Log Prefix, Process Abort\n");
		exit(EXIT_FAILURE);
	}

	ret = config.GetStringKey("log", "suffix", suffix);
	if (ret != 0) {
		printf("Failed To Initialize Log Suffix, Process Abort\n");
		exit(EXIT_FAILURE);
	}

	ret = config.GetStringKey("log", "path", path);
	if (ret != 0) {
		printf("Failed To Initialize Log Path, Process Abort\n");
		exit(EXIT_FAILURE);
	}

	ret = config.GetStringKey("log", "level", loglevel);
	if (ret != 0) {
		printf("Failed To Get Config Log Level, Process Abort\n");
		exit(EXIT_FAILURE);
	}

	ret = config.GetIntKey("server", "port", port);
	if (ret != 0) {
		printf("Failed To Get Config Port, Process Abort\n");
		exit(EXIT_FAILURE);
	}
	
	for (int i = 0; i <= LOG_DEBUG; i++) {
		if (strcasecmp(Levelname[i].name, loglevel.c_str()) == 0) {
			level = Levelname[i].level;
		}
	}

	Log::Instance(path, logprefix, suffix, level);
	return ret;
}

void init_timer_callbacks(EventDriver *driver) {
	// Every 100ms Flush Log Cache Buffer
	driver->AddTimer(0, 100, false, flush_log, NULL);
	// Every 5 Minutes Kick Out Idle Connection
	driver->AddTimer(300, 0, false, Socket::IdleCtrlCb, NULL);
	// Every 500ms Inspect FSM Timeout
	driver->AddTimer(0, 500, false, FsmTimeoutCb, NULL);
}

int main(int argc , char **argv) {
	int port = 0;
	init_config(port);
	// When Segmentation Fault Occurs, Print Diagnose Information
	signal(SIGSEGV, dump_stacktrace);
	signal(SIGABRT, dump_stacktrace);
	signal(SIGPIPE, SIG_IGN);
	signal(SIGINT, ctrl_c_handler);

	int fd = socket(PF_INET, SOCK_STREAM, 0);

	Socket *server = new Socket(fd);
	server->BindListen(port);

	printf("%s %s Server Start, pid %d, Listen Fd %d On TCP Port %d\n", now_str().c_str(), version, getpid(), server->GetFd(), port);
	// Detach From Terminal
	INFO("%s Server Start, pid %d, Listen Fd %d On TCP Port %d", version, getpid(), server->GetFd(), port);
	daemon(1, 1);

	EventDriver *driver = EventDriver::Instance();
	driver->CreateDriver();

	// Listen Fd Use Edge-Trigger
	driver->AddEvent(fd, server, EDGE_TRIGGER);

	init_timer_callbacks(driver);

	driver->StartLoop();	

	delete server;
	delete driver;

	atexit(flush_log);
	return 0;
}
