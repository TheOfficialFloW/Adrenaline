#ifndef __LIBPSARDUMPER_H__
#define __LIBPSARDUMPER_H__

int pspDecryptTable(u8 *buf1, u8 *buf2, int size, int mode);
int pspPSARInit(u8 *dataPSAR, u8 *dataOut, u8 *dataOut2);
int pspPSARGetNextFile(u8 *dataPSAR, int cbFile, u8 *dataOut, u8 *dataOut2, char *name, int *retSize, int *retPos);
int pspPSARSetBufferPosition(int position);

#endif