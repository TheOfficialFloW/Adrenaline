#include <common.h>

#include "csoread.h"
#include "umd9660_driver.h"

// index buffer size
#define CISO_INDEX_SIZE (512/4)

// compresed data buffer cache size
#define CISO_BUF_SIZE 0x2000

static u8 *ciso_data_buf = NULL;

static u32 ciso_index_buf[CISO_INDEX_SIZE] __attribute__((aligned(64)));

static u32 ciso_buf_pos;    // file poisiotn of the top of ciso_data_buf //7e64
static u32 ciso_cur_index;  // index(LBA) number of the top of ciso_index_buf //7e60

// header buffer
static CISO_H ciso; //7e48

static int max_sectors;

int CisoOpen(int umdfd) {
	int result;

	ciso.magic[0] = 0;
	ciso_buf_pos = 0x7FFFFFFF;
	sceIoLseek(umdfd, 0, PSP_SEEK_SET);
	result = sceIoRead(umdfd, &ciso, sizeof(ciso));
	if (result < 0) return result;

	if (ciso.magic[0] == 'C' && ciso.magic[1] == 'I' && ciso.magic[2]=='S' && ciso.magic[3]=='O') {
		max_sectors = (int)(ciso.total_bytes) / ciso.block_size;
		ciso_cur_index = 0xFFFFFFFF;

		if (!ciso_data_buf) {
			ciso_data_buf = (unsigned char *)oe_malloc(CISO_BUF_SIZE+64);
			if (!ciso_data_buf) return -1;

			if ((((u32)ciso_data_buf) % 64) != 0) {
				ciso_data_buf += (64 - (((u32)ciso_data_buf) % 64));
			}
		}

		return 0;
	}

	// header check error
	return SCE_KERNEL_ERROR_NOFILE;
}

static int inline ciso_get_index(u32 sector, int *pindex) {
	// search index
	int index_off = sector - ciso_cur_index;
	if ((ciso_cur_index == 0xFFFFFFFF) || (index_off < 0) || (index_off >= CISO_INDEX_SIZE)) {
		int result = ReadUmdFileRetry(ciso_index_buf, sizeof(ciso_index_buf), sizeof(ciso)+sector*4);
		if (result < 0) return result;

		ciso_cur_index = sector;
		index_off = 0;
	}

	// get file posision and sector size
	*pindex = ciso_index_buf[index_off];
	return 0;
}

static int ciso_read_one(void *buf, int sector) {
	int result;
	int index,index2;

	// get current index
	result = ciso_get_index(sector,&index);
	if (result < 0) return result;

	// get file posision and sector size
	int dpos = (index & 0x7FFFFFFF) << ciso.align;

	if (index & 0x80000000) {
		result = ReadUmdFileRetry(buf, 0x800, dpos);
		return result;
	}

	// get sectoer size from next index
	result = ciso_get_index(sector + 1,&index2);
	if (result < 0) return result;

	int dsize = ((index2 & 0x7FFFFFFF) << ciso.align) - dpos;

	// adjust to maximum size for scramble(shared) sector index
	if ((dsize <= 0) || (dsize > 0x800)) dsize = 0x800;

	// read sector buffer
	if ((dpos < ciso_buf_pos) || ((dpos+dsize) > (ciso_buf_pos+CISO_BUF_SIZE))) {
		// seek & read
		result = ReadUmdFileRetry(ciso_data_buf, CISO_BUF_SIZE, dpos);
		if (result < 0) {
			ciso_buf_pos = 0xFFF00000; // set invalid position
			return result;
		}
		ciso_buf_pos = dpos;
	}

	result = sceKernelDeflateDecompress(buf, 0x800, ciso_data_buf + dpos - ciso_buf_pos, 0);
	if (result < 0) return result;

	return 0x800;
}

int CisofileReadSectors(int lba, int nsectors, void *buf) {
	int num_bytes = nsectors * SECTOR_SIZE;

	int i;
	for (i = 0; i < num_bytes; i += 0x800) {
		int result = ciso_read_one(buf, lba);
		if (result < 0) {
			nsectors = result;
			break;
		}
		buf += 0x800;
		lba++;
	}

	return nsectors;
}