#ifndef __DEMO_MEM_H__
#define __DEMO_MEM_H__

#include <linux/ioctl.h>

#define DEMO_MEM_TYPE           'k'
#define DEMO_MEM_CLEAN          _IO(DEMO_MEM_TYPE,0x10)
#define DEMO_MEM_SETVAL         _IOW(DEMO_MEM_TYPE,0x11,int)
#define DEMO_MEM_GETVAL         _IOR(DEMO_MEM_TYPE,0x12,int)






#endif
