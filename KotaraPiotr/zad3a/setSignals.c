#include "setSignals.h"


void exiterror(char* err){
    printf(err);
    exit(1);
}


void setSigHandlers(void (*handle_func)(int, siginfo_t*, void*), int siginfo, int sigstop){
    sigset_t signals;

	if (sigfillset(&signals) == -1) exiterror("error in setting signal set\n");

	sigdelset(&signals, siginfo);
	sigdelset(&signals, sigstop);

	if (sigprocmask(SIG_BLOCK, &signals, NULL) != 0)exiterror("error in setting signal set\n");

	if (sigemptyset(&signals) == -1)exiterror("error in setting signal set\n");

	struct sigaction usr_action;
	usr_action.sa_sigaction= handle_func;
	usr_action.sa_mask = signals;
	usr_action.sa_flags = SA_SIGINFO;

	if (sigaction(siginfo, &usr_action, NULL) == -1
		|| sigaction(sigstop, &usr_action, NULL) == -1)exiterror("error in setting signal set\n");


}
