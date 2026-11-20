#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>

int main(void){
	uid_t real_uid = getuid();
	uid_t effective_uid = geteuid();

	printf("Real UID: %u\n", (unsigned int)real_uid);
	printf("Effective UID: %u\n", (unsigned int)effective_uid);
	return 0;
}