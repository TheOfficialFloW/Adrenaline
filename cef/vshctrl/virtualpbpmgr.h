#ifndef __VIRTUALPBPMGR_H__

#define __VIRTUALPBPMGR_H__

#include "isofs_driver/isofs_driver.h"

typedef struct
{
	u32  header[10];
	char isofile[256];	
	char discid[10];
	u8   pad[6];
	char sfotitle[64];
	int psfo_lba;
	int psfo_size;
	int i0png_lba; 
	int i0png_size;
	int i1pmf_lba ;
	int i1pmf_size;
	int p0png_lba;
	int p0png_size;
	int p1png_lba;
	int p1png_size;
	int s0at3_lba;
	int s0at3_size;
	int filesize;
	int filepointer;
	ScePspDateTime mtime;
} VirtualPbp;

typedef struct
{
	int dread;
	int deleted;
	int psdirdeleted;
} InternalState;

typedef struct {
	char FileName[13];
	char LongName[256];
}SceFatMsDirentPrivate;

int virtualpbp_init();
int virtualpbp_exit();
int virtualpbp_reset();
int virtualpbp_add(char *isofile, ScePspDateTime *mtime, VirtualPbp *res);
int virtualpbp_fastadd(VirtualPbp *pbp);
int virtualpbp_open(int i);
int virtualpbp_close(SceUID fd);
int virtualpbp_read(SceUID fd, void *data, SceSize size);
int virtualpbp_lseek(SceUID fd, SceOff offset, int whence);
int virtualpbp_getstat(int i, SceIoStat *stat);
int virtualpbp_chstat(int i, SceIoStat *stat, int bits);
int virtualpbp_remove(int i);
int virtualpbp_rmdir(int i);
int virtualpbp_dread(SceUID fd, SceIoDirent *dir);
char *virtualpbp_getfilename(int i);


#endif

