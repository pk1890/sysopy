#include <stdio.h>
#include <zconf.h>

int main(){
	return sysconf(_SC_CLK_TCK);
}
