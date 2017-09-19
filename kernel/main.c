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
#define Mon   1
#define Tues  2
#define Wed   3
#define Thur  4
#define Fri   5
#define Sat   6
#define Sun   0

#define January_days   31
#define February_days   28
#define March_days    31
#define April_days    30
#define May_days     31
#define June_days     30
#define July_days     31
#define August_days    31
#define September_days  30
#define October_days   31
#define November_days   30
#define December_days   31

#define first1month January_days
#define first2month January_days+February_days
#define first3month January_days+February_days+March_days
#define first4month January_days+February_days+March_days+April_days
#define first5month January_days+February_days+March_days+April_days+May_days
#define first6month January_days+February_days+March_days+April_days+May_days+June_days
#define first7month January_days+February_days+March_days+April_days+May_days+June_days \
             +July_days
#define first8month January_days+February_days+March_days+April_days+May_days+June_days \
             +July_days+August_days
#define first9month January_days+February_days+March_days+April_days+May_days+June_days \
             +July_days+August_days+September_days
#define first10month January_days+February_days+March_days+April_days+May_days+June_days \
             +July_days+August_days+September_days+October_days
#define first11month January_days+February_days+March_days+April_days+May_days+June_days \
             +July_days+August_days+September_days+October_days+November_days

int known_weekday = Tues;
int known_year = 1901;
int konwn_month = 1;
int known_day = 1;

PUBLIC int kernel_main()
{

	struct task* p_task;
	struct proc* p_proc= proc_table;
	char* p_task_stack = task_stack + STACK_SIZE_TOTAL;
	u16   selector_ldt = SELECTOR_LDT_FIRST;
        u8    privilege;
        u8    rpl;
	int   eflags;
	int   i, j;
	int   prio;
	for (i = 0; i < NR_TASKS+NR_PROCS; i++) {
	        if (i < NR_TASKS) {    
                        p_task    = task_table + i;
                        privilege = PRIVILEGE_TASK;
                        rpl       = RPL_TASK;
                        eflags    = 0x1202; /* IF=1, IOPL=1, bit 2 is always 1 */
			prio      = 15;
                }
                else {                 
                        p_task    = user_proc_table + (i - NR_TASKS);
                        privilege = PRIVILEGE_USER;
                        rpl       = RPL_USER;
                        eflags    = 0x202; /* IF=1, bit 2 is always 1 */
			prio      = 5;
                }

		strcpy(p_proc->name, p_task->name);	/* name of the process */
		p_proc->pid = i;			/* pid */

		p_proc->ldt_sel = selector_ldt;

		memcpy(&p_proc->ldts[0], &gdt[SELECTOR_KERNEL_CS >> 3],
		       sizeof(struct descriptor));
		p_proc->ldts[0].attr1 = DA_C | privilege << 5;
		memcpy(&p_proc->ldts[1], &gdt[SELECTOR_KERNEL_DS >> 3],
		       sizeof(struct descriptor));
		p_proc->ldts[1].attr1 = DA_DRW | privilege << 5;
		p_proc->regs.cs	= (0 & SA_RPL_MASK & SA_TI_MASK) | SA_TIL | rpl;
		p_proc->regs.ds	= (8 & SA_RPL_MASK & SA_TI_MASK) | SA_TIL | rpl;
		p_proc->regs.es	= (8 & SA_RPL_MASK & SA_TI_MASK) | SA_TIL | rpl;
		p_proc->regs.fs	= (8 & SA_RPL_MASK & SA_TI_MASK) | SA_TIL | rpl;
		p_proc->regs.ss	= (8 & SA_RPL_MASK & SA_TI_MASK) | SA_TIL | rpl;
		p_proc->regs.gs	= (SELECTOR_KERNEL_GS & SA_RPL_MASK) | rpl;

		p_proc->regs.eip = (u32)p_task->initial_eip;
		p_proc->regs.esp = (u32)p_task_stack;
		p_proc->regs.eflags = eflags;

		p_proc->p_flags = 0;
		p_proc->p_msg = 0;
		p_proc->p_recvfrom = NO_TASK;
		p_proc->p_sendto = NO_TASK;
		p_proc->has_int_msg = 0;
		p_proc->q_sending = 0;
		p_proc->next_sending = 0;

		for (j = 0; j < NR_FILES; j++)
			p_proc->filp[j] = 0;

		p_proc->ticks = p_proc->priority = prio;

		p_task_stack -= p_task->stacksize;
		p_proc++;
		p_task++;
		selector_ldt += 1 << 3;
	}
	k_reenter = 0;
	ticks = 0;

	p_proc_ready	= proc_table;

	init_clock();
        init_keyboard();

	restart();

	while(1){}
}


PUBLIC int get_ticks()
{
	MESSAGE msg;
	reset_msg(&msg);
	msg.type = GET_TICKS;
	send_recv(BOTH, TASK_SYS, &msg);
	return msg.RETVAL;
}


void TestA()
{
	int fd;
	char tty_name[] = "/dev_tty0";
	char rdbuf[128];

	int fd_stdin  = open(tty_name, O_RDWR);
	assert(fd_stdin  == 0);
	int fd_stdout = open(tty_name, O_RDWR);
	assert(fd_stdout == 1);
	const char bufw[80] = {0};

	clear();
	welcome();
	animation();
	printl("\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n");

	while (1) {
		printl("root$");
		int r = read(fd_stdin, rdbuf, 70);
		rdbuf[r] = 0;
		if (strcmp(rdbuf, "processManager") == 0)
		{
			processManager();
		}
		else if (strcmp(rdbuf, "fileManager") == 0)
		{
			printf("File Manager is already running on tty1,press shift+F2 to enter\n");
			continue;
		}
		else if (strcmp(rdbuf, "help") == 0)
		{
			help();
		}
		else if (strcmp(rdbuf, "clear") == 0)
		{
			clear();
		}
		else if (strcmp(rdbuf, "joseph") == 0)
		{
			joseph();
		}
		else if(strcmp(rdbuf,"calendar")==0)
		{
			int year,i,r,s,t;
			char buf[128];
			while (1){
				printf("\nWelcome to system calendar!");
				printf("\nPress 1 to start or other key to end:");
				r=read(0,buf,128);
				i=getNum(buf);
				if (i == 1) {
					printf("Please enter a year:");
					s=read(0,buf,128);
				        year=getNum(buf);                     
					printf("\n");
					while (year < 1902)
					{
					printf("Please enter a year greater than 1901\n");
						printf("Please enter a year:");
						t=read(0,buf,128);
				                year=getNum(buf);
						printf("\n");
					}
					main_calendar(year);
				}
				else
					break; 
			}
		}
		else
			printf("No such command!\n");
	}
}


void TestB()
{
	char tty_name[] = "/dev_tty1";
	int fd_stdin  = open(tty_name, O_RDWR);
	assert(fd_stdin  == 0);
	int fd_stdout = open(tty_name, O_RDWR);
	assert(fd_stdout == 1);

	char rdbuf[128];
	char cmd[8];
	char filename[120];
	char buf[1024];
	int m,n;
	printf("    ===========================================================\n\n");
	printf("                          File Manager                         \n");
	printf("                           of LemonOS                        \n\n");
	printf("    ===========================================================\n");
	while (1) {
		printf("$ ");
		int r = read(fd_stdin, rdbuf, 70);
		rdbuf[r] = 0;
		
		if (strcmp(rdbuf, "help") == 0)
		{
			fileHelp();
		}
		else
		{
			int fd;
			int i = 0;
			int j = 0;
			char temp = -1;
			//get command and file name
			while(rdbuf[i]!=' '){
				cmd[i] = rdbuf[i];
				i++;
			}
			cmd[i++] = 0;
			while(rdbuf[i] != 0){
				filename[j] = rdbuf[i];
				i++;
				j++;
			}
			filename[j] = 0;

			//create file
			if (strcmp(cmd, "create") == 0)
			{
				fd = open(filename, O_CREAT | O_RDWR);
				if (fd == -1)
				{
					printf("Failed to create file!\n");
					continue ;
				}
				buf[0] = 0;
				write(fd, buf, 1);
				printf("File created: %s (fd %d)\n", filename, fd);
				close(fd);
			}
			//read file
			else if (strcmp(cmd, "read") == 0)
			{
				fd = open(filename, O_RDWR);
				if (fd == -1)
				{
					printf("Failed to open file!\n");
					continue ;
				}
				
				n = read(fd, buf, 1024);
				printf("%s\n", buf);
				close(fd);
			}

			//write file
			else if (strcmp(cmd, "write") == 0)
			{
				fd = open(filename, O_RDWR);
				if (fd == -1)
				{
					printf("Failed to open file!\n");
					continue ;
				}
				m = read(fd_stdin, rdbuf,80);
				rdbuf[m] = 0;
				
				n = write(fd, rdbuf, m+1);
				close(fd);
			}

			//delete file
			else if (strcmp(cmd, "delete") == 0)
			{
				m=unlink(filename);
				if (m == 0)
				{
					printf("File deleted!\n");
					continue;
				}
				else
				{
					printf("Failed to delete file!\n");
					continue;
				}
			}
			else 
			{
				printf("No such command!\n");
				continue;
			}	
		}	
	}
	assert(0); 
}


void TestC()
{
	for(;;);
}


PUBLIC void panic(const char *fmt, ...)
{
	int i;
	char buf[256];
	va_list arg = (va_list)((char*)&fmt + 4);

	i = vsprintf(buf, fmt, arg);

	printl("%c !!panic!! %s", MAG_CH_PANIC, buf);

	__asm__ __volatile__("ud2");
}

//clear screen
void clear()
{
	clear_screen(0,console_table[current_console].cursor);
	console_table[current_console].crtc_start = 0;
	console_table[current_console].cursor = 0;
}

//system help of tty0
void help()
{
	printf("\n*****************************************************************\n");
	printf("Command List     :\n");
	printf("1. processManager: Show all system process\n");
	printf("2. fileManager   : Run file manager\n");
	printf("3. clear         : Clear the screen\n");
	printf("4. help          : Show system help information\n");
	printf("5. calendar      : Look up the calendar of a specified year and month\n");
	printf("6. joseph        : Play the joseph game\n");
	printf("*****************************************************************\n");		
}

//system help of tty1
void fileHelp()
{
	printf("*****************************************************************\n");
	printf("Command List\n");
	printf("1. create [filename]              : Create a new file \n");
	printf("2. read [filename]                : Read the file\n");
	printf("3. write [filename]               : Write at the end of the file\n");
	printf("4. delete [filename]              : Delete the file\n");
	printf("5. help                           : Display the help message\n");
	printf("*****************************************************************\n");		
}

void processManager()
{
	int i;
	printf("*****************************************************************\n");
	printf("      ID      |    name       |  priority    | running\n");
	printf("--------------------------------------------------------------------\n");
	for ( i = 0 ; i < NR_TASKS + NR_PROCS ; ++i )
	{
//		if ( proc_table[i].priority == 0) continue;
		printf("        %d           %s            %d                yes\n", proc_table[i].pid, proc_table[i].name, proc_table[i].priority);
	}
	printf("****************************************************************\n");
}

int day_count(int month)
{
	switch (month)
	{
	case 1: return 0; break;
	case 2: return first1month; break;
	case 3: return first2month; break;
	case 4: return first3month; break;
	case 5: return first4month; break;
	case 6: return first5month; break;
	case 7: return first6month; break;
	case 8: return first7month; break;
	case 9: return first8month; break;
	case 10: return first9month; break;
	case 11: return first10month; break;
	case 12: return first11month; break;
	}
}

char * month_name(int month)
{
	switch (month)
	{
	case 1:
		return "January";
		break;
	case 2:
		return "February";
		break;
	case 3:
		return "March";
		break;
	case 4:
		return "April";
		break;
	case 5:
		return "May";
		break;
	case 6:
		return "June";
		break;
	case 7:
		return "July";
		break;
	case 8:
		return "August";
		break;
	case 9:
		return "September";
		break;
	case 10:
		return "October";
		break;
	case 11:
		return "November";
		break;
	case 12:
		return "December";
		break;
	default:
		break;
	}
}

void first_line_print(int month, int year)
{
	printf("%8dyear %s \n", year, month_name(month));
}

void week_print()
{
	printf("%3s%3s%3s%3s%3s%3s%3s\n", "7", "1", "2", "3", "4", "5", "6");
}


int date_distance_count(int month, int year)
{
	int leap_year_count = 0;
	int i;
	int distance;

	if (year > known_year)
	{
		for (i = known_year; i<year; i++)
		{
			if (((i % 4 == 0) && (i % 100 != 0)) || (i % 400 == 0))
			{
				leap_year_count++;
			}
		}

		if (month > 2)
		{
			if (((year % 4 == 0) && (year % 100 != 0)) || (year % 400 == 0))
			{
				leap_year_count++;
			}
		}
	}
	else
		if (year == known_year)
		{
			if (month > 2)
			{
				leap_year_count = 1;
			}
		}


	distance = (year - known_year) * 365 + leap_year_count + day_count(month);
	return distance;
}

int makesure_firstday_weekday(int month, int year)
{
	int date_distance = 0;
	int weekday;

	date_distance = date_distance_count(month, year);
	weekday = (known_weekday + date_distance) % 7;

	return weekday;
}

void print_in_turn(int month, int firstday, int year)
{
	int i = 1;
	int weekday;

	switch (firstday)
	{
	case Sun:
		break;
	case Mon:
		printf("%3s", "");
		break;
	case Tues:
		printf("%6s", "");
		break;
	case Wed:
		printf("%9s", "");
		break;
	case Thur:
		printf("%12s", "");
		break;
	case Fri:
		printf("%15s", "");
		break;
	case Sat:
		printf("%18s", "");
		break;
	}

	switch (month)
	{
	case 1:
	case 3:
	case 5:
	case 7:
	case 8:
	case 10:
	case 12:
	{
		for (i = 0; i<31; i++)
		{
			weekday = (firstday + i) % 7;
			printf("%3d", i + 1);

			if (weekday == Sat)
			{
				printf("\n");
			}
		}
		break;
	}
	case 2:
	{
		if (((year % 4 == 0) && (year % 100 != 0)) || (year % 400 == 0)) 
		{
			for (i = 0; i<29; i++)
			{
				weekday = (firstday + i) % 7;
				printf("%3d", i + 1);

				if (weekday == Sat)
				{
					printf("\n");
				}
			}
			break;
		}
		else
		{
			for (i = 0; i<28; i++)
			{
				weekday = (firstday + i) % 7;
				printf("%3d", i + 1);

				if (weekday == Sat)
				{
					printf("\n");
				}
			}
			break;
		}

	}
	case 4:
	case 6:
	case 9:
	case 11:
	{
		for (i = 0; i<30; i++)
		{
			weekday = (firstday + i) % 7;
			printf("%3d", i + 1);

			if (weekday == Sat)
			{
				printf("\n");
			}
		}
		break;
	}
	}
}

void date_print(int month, int year)
{
	int firstday;
	firstday = makesure_firstday_weekday(month, year);
	print_in_turn(month, firstday, year);
	printf("\n");
}

void main_month(int month, int year)
{
	first_line_print(month, year);
	week_print();
	date_print(month, year);
	printf("\n");
}

void main_calendar(int year)
{
	int i,n,month;
	char m[128];
        printf("please enter a month:");
	n=read(0,m,128);
        month=getNum(m);
	for (i = 1; i <= 12; i++)
	{
	   if(i==month)
           { 
             main_month(i, year);
	   }
        }
}  

int getNum(char*buf){
	int ten=1,i=0,res=0;
	for(i=0;i<strlen(buf)-1;i++){
		ten*=10;
	}
	for(i=0;i<strlen(buf);i++){
		res+=(buf[i]-'0')*ten;
		ten/=10;
	}
	return res;
}

void joseph()
{
	printf("\nWelcome to joseph game!");
	printf("\n\nGame rule:\nPeople are standing in a circle waiting to be executed. Counting begins at a specified point in the circle and proceeds around the circle in a specified direction. After a specified number of people are skipped, the next person is executed. The procedure is repeated with the remaining people, starting with the next person, going in the same direction and skipping the same number of people, until only one person remains, and is freed.\n\n");
	int flag[100] = { 0 };
	int n = 0, m = 0,r,t;
	char buf[128];
	printf("Please enter the sum of people:");
	r=read(0,buf,128);
	n=getNum(buf);
	printf("Please enter the death number:");
	t=read(0,buf,128);
	m=getNum(buf);
	int i = 0;
	int count = 0;  
	int num = 0;   
	for (i = 1; i <= n; i++) {
		flag[i] = 1;	
	}
	while (count < n - 1) {
		for (i = 1; i <= n; i++) {
			if (1 == flag[i]) {
				num++;
				if (num == m) {
					 printf("The death's id is %d\n",i);
					 count++;
					 flag[i] = 0;
					 num = 0;
				}
				if (count == n - 1) {
					break;			
				}	
			}
		}	
	}
	for (i = 1; i <= n; i++) {       
		if (1 == flag[i]) {
			printf("The survivor's id is : %d\n\n", i);		
		}	
	}
}

void welcome()
{
	disp_str("\n********************************************************\n");
	disp_str("*                                                      *\n");
	disp_str("*              Welcome to Lemon OS                     *\n");
	disp_str("*                1552681 Chen qige                     *\n");
	disp_str("*              1552616 Zhang xiaoyan                   *\n");
	disp_str("*                                                      *\n");
	disp_str("********************************************************\n");
}

void animation()
{
	disp_color_str("\n*********************************************************\n",0xb);
	disp_color_str("*  **    ******     *       *         ***     **     *   *\n",0xa);
	disp_color_str("*  **    *         ***     ***      **    *   ***    *   *\n",0xc);
	disp_color_str("*  **    ******    ***     ***      **    *   ** *   *   *\n",0xd);
	disp_color_str("*  **    ******   **  *   *  **     **    *   **  *  *   *\n",0x9);
	disp_color_str("*  **    *        **   * *    **    **    *   **   * *   *\n",0xd);
        disp_color_str("*  ***** ******  **     *      **     ***     **    **   *\n",0xa);
	disp_color_str("**********************************************************\n",0xb);
	
}

