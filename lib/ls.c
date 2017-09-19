#include "type.h"
#include "stdio.h"
#include "const.h"
#include "protect.h"
#include "string.h"
#include "fs.h"
#include "proc.h"
#include "tty.h"
#include "console.h"
#include "global.h"
#include "proto.h"

PUBLIC int ls(char *pathName)
{
	MESSAGE msg;
	msg.type = LS;
	msg.PATHNAME=(void*)pathName;
	msg.FLAGS=0;
	msg.NAME_LEN=strlen(pathName);
	send_recv(BOTH,TASK_FS,&msg);
	return msg.RETVAL;
}
