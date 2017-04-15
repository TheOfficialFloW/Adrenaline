#ifndef __CISOREAD_H__
#define __CISOREAD_H__

typedef struct ciso_header
{
	unsigned char magic[4];			// +00 : 'C','I','S','O'                          
	unsigned long header_size;		// +04 : header size (==0x18)                     
	unsigned long long total_bytes;	// +08 : number of original data size             
	unsigned long block_size;		// +10 : number of compressed block size          
	unsigned char ver;				// +14 : version 01                               
	unsigned char align;			// +15 : align of index (offset = index[n]<<align)
	unsigned char rsv_06[2];		// +16 : reserved                                 
} CISO_H;

int CisoOpen(int umdfd);
int CisofileReadSectors(int lba, int nsectors, void *buf);

#endif