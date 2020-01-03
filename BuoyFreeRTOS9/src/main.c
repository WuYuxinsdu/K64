/*

* 青岛海洋一所大洋浮标控制系统
* 实验平台 ： FRDM-K64F
* 板载芯片 ： MK64FN1MOVLL12
* 实验效果：单片机向传感器发命令，传感器收到命令将返回值传回单片机，
            单片机发送的命令和传感器传回的返回值均存入sd卡
* 程序制作：武玉欣 山东大学

*/   
#include "gpio.h"
#include "common.h"
#include "systick.h"
#include "uart.h"
#include "sd.h"
#include "pit.h"
#include "string.h"
#include "stdlib.h"
#include "stdio.h"
#include "ff.h"
#include "cjson.h"
#include "wdog.h"

static uint32_t uart_instance;   //串口号
//Uart Buffer
#define UART_MAXLENGTH 150
static char uartBuffer[UART_MAXLENGTH]="";  //接收缓冲区
static char str[UART_MAXLENGTH]="";
static int uartChCount=0;
static int Time_Count = 1;
static int cmpflag;
static int error;

static char cmp[] = "IMM>";  //判断正确返回

//FatFs
static		FRESULT rc;
static		FATFS fs_sd;
static		FIL fil;
static		FATFS *fs;
static 		FILINFO	fil_info;

struct task_cfg{
	char type[10];		//任务类型
  char command[20];		//任务命令
    int merge;         //任务是否合并
  //int data_start;		//返回报文中数据的开始位置
	//int data_end;			//返回报文中数据的结束位置
	//char value[10];		//被检测的量的值
} ;
static char *task_ID[]={"T1","T2","T3","T4","T5","T6","T7","T8","T9","T10","T11","T12","T13","T14","T15","T16","T17","T18","T19","T20","T21","T22","T23","T24"};
static struct task_cfg task[100];
static int task_num=0,task_current=0;
static int num;

char tasktotal[][5] = {"io","uart","file"};
struct typeone
{
    char port;
    int pin;
    char level;
};
static struct typeone io[10];
struct typetwo
{
    char port;
    int pin;
    char cmd[20];
};
static struct typetwo uart[10];
struct typethree
{
    char* path;
};
static struct typethree file[10];

#define LED1  PBout(22)  //red
#define LED2  PBout(21)  //blue
#define LED3  PEout(26) //green

/*串口中断接收中断回调函数*/
static void UART_RX_ISR(uint16_t byteReceived)
{

		*(uartBuffer+uartChCount)=byteReceived;//收到的内容接收，uartChount移位
		uartChCount++;
		
}
/*串口将传感器一位位传回的数据整合到str字符串*/
int UART_read(char *str)
{
  int i;

	for(i=0;i<uartChCount;i++)
		*(str+i)=*(uartBuffer+i);
	*(str+i)='\0';

	uartChCount=0;
  return i;//该函数返回值为接收到的数据长度
}
/*文件操作错误码输出*/
#define ERROR_TRACE(rc)     do {if(rc != 0){printf("fatfs error:%d\r\n", rc);}} while(0)
char buf[1024]="";


static void FatfsDisplayFileInfo(FILINFO *fileInfo)
{
    char *fileName;
#if _USE_LFN
    fileName = (fileInfo->lfname[0] ? fileInfo->lfname : fileInfo->fname;
#else
    fileName = fileInfo->fname;
#endif /* _USE_LFN */
    /* note: if this file/directory don't have one attribute, '_' replace the attribute letter ('R' - readonly, 'H' - hide, 'S' - system) */
    printf("    %s - %c%c%c - %s - %dBytes - %d-%d-%d %d:%d:%d\r\n", (fileInfo->fattrib & AM_DIR) ? "dir" : "fil",
             (fileInfo->fattrib & AM_RDO) ? 'R' : '_',
             (fileInfo->fattrib & AM_HID) ? 'H' : '_',
             (fileInfo->fattrib & AM_SYS) ? 'S' : '_',
             fileName,
             (fileInfo->fsize),
             (uint32_t)((fileInfo->fdate >> 9) + 1980) /* year */,
             (uint32_t)((fileInfo->fdate >> 5) & 0x000Fu) /* month */,
             (uint32_t)(fileInfo->fdate & 0x001Fu) /* day */,
             (uint32_t)((fileInfo->ftime >> 11) & 0x0000001Fu) /* hour */,
             (uint32_t)((fileInfo->ftime >> 5) & 0x0000003Fu) /* minute */,
             (uint32_t)(fileInfo->ftime & 0x0000001Fu) /* second */
             );
}

DWORD AccSize;				/* Work register for fs command */
WORD AccFiles, AccDirs;
FILINFO Finfo;

static FRESULT scan_files (
	char* path		/* Pointer to the path name working buffer */
)
{
	DIR dir;
	FRESULT res;
	BYTE i;
	char *fn;


	if ((res = f_opendir(&dir, path)) == FR_OK) {
		i = strlen(path);
		while (((res = f_readdir(&dir, &Finfo)) == FR_OK) && Finfo.fname[0]) {
			if (FF_FS_RPATH && Finfo.fname[0] == '.') continue;
#if _USE_LFN
			fn = *Finfo.lfname ? Finfo.lfname : Finfo.fname;
#else
			fn = Finfo.fname;
#endif
            FatfsDisplayFileInfo(&Finfo);
			if (Finfo.fattrib & AM_DIR) {
				AccDirs++;
				*(path+i) = '/'; strcpy(path+i+1, fn);
                printf("enter dir:%s\r\n", path);
				res = scan_files(path);
				*(path+i) = '\0';
				if (res != FR_OK) break;
			} else {
			//	xprintf("%s/%s\n", path, fn);
				AccFiles++;
				AccSize += Finfo.fsize;
			}
		}
	}

	return res;
}

void readConfigFile(void)
{

    fs = &fs_sd;
    UINT bw,br; /* bw = byte writted br = byte readed */
    DWORD fre_clust, fre_sect, tot_sect;

	cJSON * pJsonRoot = NULL,*pNode,*pSubNode;

	printf("FATFS test\r\n");
    printf("please insert SD card...\r\n");
    /*初始化sd卡*/	
    if(SD_QuickInit(20*1000*1000))
    {
        printf("SD card init failed!\r\n");
        return;
    }
    printf("SD size:%dMB\r\n", SD_GetSizeInMB());
    /*加载文件系统*/
    rc = f_mount(fs, "0:", 0);
    ERROR_TRACE(rc);
 
    printf("open or create file\r\n");
    rc = f_open(&fil, "0:/uart1.cfg", FA_READ);
    ERROR_TRACE(rc);
    
    while(1)
    {
        rc = f_read(&fil, buf, 1024, &br);
        if(rc || !br ) 
        {
            ERROR_TRACE(rc);
            break;
        }
        
    }
    printf("config file:%s",buf);

    rc = f_close(&fil);
    ERROR_TRACE(rc);
		
		rc = f_mount(NULL, "0:", 0);
		ERROR_TRACE(rc);
		
		pJsonRoot = cJSON_CreateObject();
		if(NULL == pJsonRoot)
		  printf("JSON object create failed.\n");	
		
		pJsonRoot=cJSON_Parse(buf);
		if(NULL==pJsonRoot)
			printf("JSON parse failed.\n");

        task_num=cJSON_GetArraySize(pJsonRoot);
		printf("task number=%d\r\n", task_num);	

		for(num=0;num<task_num;num++)
		{
			pNode=cJSON_GetObjectItem(pJsonRoot,task_ID[num]);
			sscanf(pNode->valuestring,"%s %s %d",task[num].type,task[num].command,&task[num].merge);
			//sscanf(pNode->valuestring,"%s %s",task[num].type,task[num].command);
			printf("num=%d:task is %s\r\n",num,task[num].command);

		}
		task_current=0;
		cJSON_Delete(pJsonRoot);
        
}

static void doFile(void* buff,char* path)
{
    fs = &fs_sd;
    UINT bw,br; /* bw = byte writted br = byte readed */
    DWORD fre_clust, fre_sect, tot_sect;
	
   	/*初始化sd卡*/
    if(SD_QuickInit(20*1000*1000))
    {
        //printf("SD card init failed!\r\n");
        return;
    }

    //printf("SD size:%dMB\r\n", SD_GetSizeInMB());
    /*加载文件系统*/
    rc = f_mount(fs, "0:", 0);
    ERROR_TRACE(rc);
    //rc = f_getfree("0:", &fre_clust, &fs);
    //ERROR_TRACE(rc);

    //memcpy(buf, "0:", 3);
    //scan_files((char*)buf);
    /*获取剩余空间大小*/
    //tot_sect = (fs->n_fatent - 2) * fs->csize;
    //fre_sect = fre_clust * fs->csize;
    //printf("%d KB total drive space.\r\n%d KB available.\r\n", tot_sect / 2, fre_sect / 2);
    /*追加写文件，如果不存在就新建*/
    //printf("open or create file\r\n");
    rc = f_open(&fil,path, FA_OPEN_ALWAYS  | FA_WRITE | FA_READ);
    printf("filename is %s\r\n",path);
    ERROR_TRACE(rc);

		rc =f_lseek(&fil,f_size(&fil));
		ERROR_TRACE(rc);
    //printf("write file\r\n");
    rc = f_write(&fil,buff, strlen(buff), &bw);
    ERROR_TRACE(rc);
    //printf("%d bytes writen\r\n", bw);
    rc = f_close(&fil);		
		rc = f_mount(NULL, "0:", 0);
		ERROR_TRACE(rc);
		
}
void GPIO_Task(struct typeone *t)
{
    if((task[num].command[2] > '0')&&(task[num].command[2] < '9'))
    {
        t->port = task[num].command[0];
        t->pin = (task[num].command[1]-'0')*10+(task[num].command[2]-'0');
        t->level = task[num].command[3];
    }
    else
    {
        t->port = task[num].command[0];
        t->pin = task[num].command[1]-'0';
        t->level = task[num].command[2];
    }
    switch (t->port)
    {
        
        case 'A' : GPIO_QuickInit(HW_GPIOA, t->pin, kGPIO_Mode_OPP);
                    if (t->level == 'H')
                    {
                        printf("PORT%c  %d is on.\r\n",t->port,t->pin);
                        GPIO_WriteBit(HW_GPIOB,t->pin,1);
                    }                  
                    else
                    {
                        printf("PORT%c  %d is off.\r\n",t->port,t->pin);
                        GPIO_WriteBit(HW_GPIOB,t->pin,0);
                    }     
                    break;
        case 'B' :
                   GPIO_QuickInit(HW_GPIOB,t->pin, kGPIO_Mode_OPP);
                   if (t->level == 'H')
                   {
                       printf("PORT%c  %d is on.\r\n",t->port,t->pin);
                       GPIO_WriteBit(HW_GPIOB,t->pin,1);
                   }               
                   else
                   {
                       printf("PORT%c  %d is off.\r\n",t->port,t->pin);
                       GPIO_WriteBit(HW_GPIOB,t->pin,0);
                   }         
                   break;
        case 'C' : GPIO_QuickInit(HW_GPIOC, t->pin, kGPIO_Mode_OPP);
                   if (t->level == 'H')
                   {
                       printf("PORT%c  %d is on.\r\n",t->port,t->pin);
                       GPIO_WriteBit(HW_GPIOB,t->pin,1);
                   }
                   else
                   {
                       printf("PORT%c  %d is off.\r\n",t->port,t->pin);
                       GPIO_WriteBit(HW_GPIOB,t->pin,0);
                   } 
                   break;
        case 'D' : GPIO_QuickInit(HW_GPIOD, t->pin, kGPIO_Mode_OPP);
                   if (t->level == 'H')
                   {
                       printf("PORT%c  %d is on.\r\n",t->port,t->pin);
                       GPIO_WriteBit(HW_GPIOB,t->pin,1);
                   }
                   else
                   {
                       printf("PORT%c  %d is off.\r\n",t->port,t->pin);
                       GPIO_WriteBit(HW_GPIOB,t->pin,0);
                   } 
                   break;
        case 'E' : GPIO_QuickInit(HW_GPIOE, t->pin, kGPIO_Mode_OPP);
                   if (t->level == 'H')
                   {
                       printf("PORT%c  %d is on.\r\n",t->port,t->pin);
                       GPIO_WriteBit(HW_GPIOB,t->pin,1);
                   }  
                   else
                   {
                       printf("PORT%c  %d is off.\r\n",t->port,t->pin);
                       GPIO_WriteBit(HW_GPIOB,t->pin,0);
                   }    
                   break;
                   
    }
        
}

static void Uart_Send(struct typetwo *s)
{
    //UART_QuickInit(UART4_RX_PC14_TX_PC15, 9600);
    //DelayMs(1000);
    int i,len;
    len = strlen(task[num].command);
    for(i=0;i<len;i++)
        s->cmd[i] = task[num].command[i];
    s->cmd[len] = '\r';
    s->cmd[len+1] = '\n';
    s->cmd[len+2] = '\0';
    len = strlen(s->cmd);
    //printf("cmd is %s",s->cmd);
    for (i=0;i<len;i++)
    {
        UART_WriteByte(uart_instance, s->cmd[i]);
    } 
    //DelayMs(1000);
}
/*判断返回值是否正确*/
/*static void Return_Cmp(char str[])
{
    int t=0;
    int len=0;
    len = strlen(str);
    cmpflag = 0;
    char uncmp[]="";
   for(t=0;t<4;t++)
		uncmp[t]=str[len-4+t];
	uncmp[4]='\0';
    //printf("uncmp is %s.\r\n",uncmp);
    if(strcmp(uncmp,cmp)==0)
    //if((str[len-4]==cmp[0])&&(str[len-3]==cmp[1])&&(str[len-2]==cmp[2])&&(str[len-1]==cmp[3]))
    {
        //printf("return is right.\r\n");
        cmpflag = 1;
    }
}*/
/*错误返回*/
/*static void Error_Return(char cmd1[],char cmd2[])
{
    int n;
    error = 0;
    for(n=1;n<4;n++)
    {
        if(cmpflag!=1)
        {
            error ++;
            UART_WriteCmd(cmd1);
            doFile(cmd1);
            if(UART_read(str)>0)
            doFile(str);
            Return_Cmp(str);
        }
            
        else
            break;
    }
    while(error == 3);
    UART_WriteCmd(cmd2);
    doFile(cmd2);
        
}*/

static void Uart_ReceiveFile(struct typethree *f)
{
    f->path = task[num].command;
    printf("filename is %s\r\n",f->path);
    if(UART_read(str)>0)
    {
        //DelayMs(1000);
        doFile(str,f->path);
    }
}

void PIT2_ISR(void)
{
    printf("%d:a new pit.\r\n",num);
    printf("num=%d:task is %s\r\n",num,task[num].command);
    if(task[num].merge == 1)
    {
        Uart_ReceiveFile(&file[num]);
        num++;
        GPIO_Task(&io[num]);
    }
    else
        if(task[num].type[0] == 'I')
        {
           GPIO_Task(&io[num]);  
        }
        if ((task[num].type[0] == 'S')&&(task[num].type[1] == 'S'))
        {
            Uart_Send(&uart[num]);
        }
        if((task[num].type[0] == 'S')&&(task[num].type[1] == 'F'))
        {
            Uart_ReceiveFile(&file[num]);
        }
    
    num ++;
    if(num >= task_num)
    {
        PIT_ITDMAConfig(HW_PIT_CH1, kPIT_IT_TOF, false);
        num = 0;
        EnterSTOPMode(false);
        printf("enter STOP MODE.\r\n");
    }
        
    
    
}
    
void PIT1_ISR(void)
{
    printf("a new pit2.\r\n");
    PIT_QuickInit(HW_PIT_CH1, 1000*1000);//定时1s
    /*注册PIT1 中断回调函数*/
    PIT_CallbackInstall(HW_PIT_CH1, PIT2_ISR);
    /*开启PIT1定时器中断*/
    PIT_ITDMAConfig(HW_PIT_CH1, kPIT_IT_TOF, true);
    
}

int main(void)
{

    DelayInit();

    uart_instance=UART_QuickInit(UART4_RX_PC14_TX_PC15, 9600);
    /*配置UART中断，打开接收中断，安装中断回调函数*/
    UART_CallbackRxInstall(HW_UART4, UART_RX_ISR);
    /*打开串口接收中断*/
    UART_ITDMAConfig(HW_UART4, kUART_IT_Rx, true);
    printf("UART%d OK! Hello Kinetis\r\n",uart_instance);
    readConfigFile();
    printf("Read config file.\r\n");
    num = 0;
 
    PIT_QuickInit(HW_PIT_CH0, 15*1000*1000);//定时10min
    /*注册PIT 中断回调函数*/
    PIT_CallbackInstall(HW_PIT_CH0, PIT1_ISR);
    /*开启PIT0定时器中断*/
    PIT_ITDMAConfig(HW_PIT_CH0, kPIT_IT_TOF, true);
    /*初始化看门狗*/
    //WDOG_InitTypeDef WDOG_InitStruct1;
    //WDOG_InitStruct1.windowInMs = 0;
    //WDOG_InitStruct1.mode = kWDOG_Mode_Normal;  //设置看门狗处于正常工作模式
    //WDOG_InitStruct1.timeOutInMs = 10*60*1000*1000; /* 时限 10min : 10min 内没有喂狗则复位 */
    //WDOG_Init(&WDOG_InitStruct1);
    //printf("WDOG test start!\r\n");
   
    

 
    while(1)
    {
        
        //if(UART_read(str)>0)
        //{
            /* 喂狗 防止复位 */
            //printf("wdog feed!\r\n");
            //WDOG_Refresh(); //喂狗
            //DelayMs(100);
        //}
            
        
    }
}
    
;