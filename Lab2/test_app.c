#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/sched.h>
#include <stdbool.h>
#define MODULE_NAME_LEN (64 - sizeof(unsigned long))
#include "driver.h"
#define STRUCT(type, pInstance, ...)                                           \
  {                                                                            \
    printf("%s=%p: {\n", #type, &pInstance);                                   \
    type *pStr = &pInstance;                                                   \
    __VA_ARGS__                                                                \
    printf("}\n");                                                             \
  }
#define FIELD(pPat, pInstance)                                                 \
  { printf("  %s=%" #pPat "\n", #pInstance, pStr->pInstance); }
// for inttypes...
#define PFIELD(pPat, pInstance)                                                \
  { printf("  %s=%" pPat "\n", #pInstance, pStr->pInstance); }
int main()
{
        int fd;
        int32_t number;
        int32_t value;
        ssize_t count;
        /*
	char message[4096]; 
	int i;
	for (i = 0; i < 4096; i++)
		message[i] = "\0";
	*/
        printf("\nOpening Driver\n");
        fd = open("/dev/driver_device", O_RDWR);
        if(fd < 0) {
                printf("Cannot open device file...\n");
                return 0;
        }
        number = getpid();
        printf("Writing Value to Driver\n");
        ioctl(fd, WR_VALUE, (int32_t*) &number); 
        printf("Reading Value from Driver\n");
 	ioctl(fd, IOCTL_LSMOD_COUNT, &count);
// 		printf("%d", count);
 	struct _lsmod *lsmod = (struct _lsmod *)
		malloc(sizeof(struct _lsmod) * (count));
	ioctl(fd, IOCTL_GET_LSMOD, lsmod);
	for (size_t i = 0; i < count; i++) {
		printf("%-18s %-10d %-3d %s\n", lsmod[i].name,
            		       lsmod[i].size, lsmod[i].refcnt, lsmod[i].dependencies);
	}
        struct user_fpu_state user_fpstate;
        if (ioctl(fd, RD_FPU_STATE, &user_fpstate) < 0)
        	printf("oopsie");
       	else {
       		STRUCT (struct user_fpu_state, user_fpstate, FIELD(x, control_word) 
				FIELD(x, state_word) FIELD(x, last_instr_opcode) FIELD(x, instr_pointer) FIELD(x, data_pointer) 
					FIELD(x, ip_offset) FIELD(x, ip_selector) FIELD(x, operand_offset) FIELD(x, operand_selector));
       	}
        printf("Closing Driver\n");
        close(fd);
}