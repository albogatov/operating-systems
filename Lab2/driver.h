#ifndef __IOCTL_H
#define __IOCTL_H
#define DEPENDENCIES_BUFFER 128
#include <linux/ioctl.h>
#include <linux/module.h>
struct user_fpu_state {
	uint16_t control_word;
	uint16_t state_word;
	uint16_t last_instr_opcode;
	uint64_t instr_pointer;
	uint64_t data_pointer;
	uint32_t ip_offset;
	uint32_t ip_selector;
	uint32_t operand_offset;
	uint32_t operand_selector;
};
struct _lsmod {
	char name[MODULE_NAME_LEN];
	int refcnt;
	unsigned int size;
	char dependencies[DEPENDENCIES_BUFFER];
};
#define IOCTL_BASE 'i'
#define WR_VALUE _IOW(IOCTL_BASE,1,int32_t*)
#define RD_FPU_STATE _IOR(IOCTL_BASE,2, struct user_fpu_state*)
#define IOCTL_GET_LSMOD _IOR(IOCTL_BASE, 3, char *)
#define IOCTL_LSMOD_COUNT _IOR(IOCTL_BASE, 4, int32_t *)
#endif