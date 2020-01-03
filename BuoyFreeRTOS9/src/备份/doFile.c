#include <string.h>
#include <stdio.h>

#include "sd.h"

#include "ff.h"

/* ��ӡ������� */
#define ERROR_TRACE(rc)     do {if(rc != 0){printf("fatfs error:%d\r\n", rc);}} while(0)
uint8_t buf[1024];


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

static void doFile(void)
{

		FRESULT rc;
		FATFS fs_sd;
		FIL fil;
		FATFS *fs;
    fs = &fs_sd;
    UINT bw,br; /* bw = byte writted br = byte readed */
    DWORD fre_clust, fre_sect, tot_sect;
    /* �����ļ�ϵͳ */
    rc = f_mount(fs, "0:", 0);
    ERROR_TRACE(rc);
    rc = f_getfree("0:", &fre_clust, &fs);
    ERROR_TRACE(rc);

    //memcpy(buf, "0:", 3);
    //scan_files((char*)buf);
    
    /* ������̿ռ估ʣ��ռ� */
    tot_sect = (fs->n_fatent - 2) * fs->csize;
    fre_sect = fre_clust * fs->csize;
    printf("%d KB total drive space.\r\n%d KB available.\r\n", tot_sect / 2, fre_sect / 2);
    /* д���ļ� */
    printf("open or create file\r\n");
    rc = f_open(&fil, "0:/fatfs.txt", FA_WRITE | FA_CREATE_ALWAYS);
    ERROR_TRACE(rc);
    printf("write file\r\n");
    rc = f_write(&fil, "HelloWorld\r\n", 12, &bw);
    ERROR_TRACE(rc);
    printf("%d bytes writen\r\n", bw);
    rc = f_close(&fil);
    /* ��ȡ�ļ�*/ 
    rc = f_open(&fil, "0:/fatfs.txt", FA_READ);
    ERROR_TRACE(rc);
    printf("file size:%d\r\n", f_size(&fil));
    printf("file contents:\r\n");
    while(1)
    {
        rc = f_read(&fil, buf, sizeof(buf), &br);
        if(rc || !br ) 
        {
            ERROR_TRACE(rc);
            break;
        }
        printf("%s", buf);
    }
    rc = f_close(&fil);
    ERROR_TRACE(rc);
		
		rc = f_mount(NULL, "0:", 0);
		ERROR_TRACE(rc);
		
}


