/*
  Adrenaline
  Copyright (C) 2016-2018, TheFloW

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <psp2/appmgr.h>
#include <psp2/mtpif.h>
#include <psp2/udcd.h>
#include <psp2/usbstorvstor.h>
#include <psp2/io/dirent.h>

#include <taihen.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "main.h"

int _vshIoMount(int id, const char *path, int permission, void *buf);
int vshIoUmount(int id, int a2, int a3, int a4);

int vshIoMount(int id, const char *path, int permission, int a4, int a5, int a6) {
  uint32_t buf[3];

  buf[0] = a4;
  buf[1] = a5;
  buf[2] = a6;

  return _vshIoMount(id, path, permission, buf);
}

void remount(int id) {
  vshIoUmount(id, 0, 0, 0);
  vshIoUmount(id, 1, 0, 0);
  vshIoMount(id, NULL, 0, 0, 0, 0);
}

int checkFolderExist(const char *folder) {
  SceUID dfd = sceIoDopen(folder);
  if (dfd < 0)
    return 0;

  sceIoDclose(dfd);
  return 1;
}

SceUID startUsb(const char *usbDevicePath, const char *imgFilePath, int type) {
  SceUID modid = -1;
  int res;

  // Destroy other apps
  sceAppMgrDestroyOtherApp();

  // Load and start usbdevice module
  res = taiLoadStartKernelModule(usbDevicePath, 0, NULL, 0);
  if (res < 0)
    goto ERROR_LOAD_MODULE;

  modid = res;

  // Stop MTP driver
  res = sceMtpIfStopDriver(1);
  if (res < 0 && res != 0x8054360C)
    goto ERROR_STOP_DRIVER;

  // Set device information
  res = sceUsbstorVStorSetDeviceInfo("\"PS Vita\" MC", "1.00");
  if (res < 0)
    goto ERROR_USBSTOR_VSTOR;

  // Set image file path
  res = sceUsbstorVStorSetImgFilePath(imgFilePath);
  if (res < 0)
    goto ERROR_USBSTOR_VSTOR;

  // Start USB storage
  res = sceUsbstorVStorStart(type);
  if (res < 0)
    goto ERROR_USBSTOR_VSTOR;

  // Lock power
  lockPower();

  return modid;

ERROR_USBSTOR_VSTOR:
  sceMtpIfStartDriver(1);

ERROR_STOP_DRIVER:
  taiStopUnloadKernelModule(modid, 0, NULL, 0, NULL, NULL);

ERROR_LOAD_MODULE:
  return res;
}

int stopUsb(SceUID modid) {
  int res;

  // Invalid module id
  if (modid < 0)
    return 0;

  // Stop USB storage
  res = sceUsbstorVStorStop();
  if (res < 0)
    return res;

  // Start MTP driver
  res = sceMtpIfStartDriver(1);
  if (res < 0)
    return res;

  // Stop and unload usbdevice module
  res = taiStopUnloadKernelModule(modid, 0, NULL, 0, NULL, NULL);
  if (res < 0)
    return res;

  // Unlock power
  unlockPower();

  // Remount Memory Card
  remount(0x800);

  // Remount imc0:
  if (checkFolderExist("imc0:"))
    remount(0xD00);

  // Remount xmc0:
  if (checkFolderExist("xmc0:"))
    remount(0xE00);

  // Remount uma0:
  if (checkFolderExist("uma0:"))
    remount(0xF00);

  return 0;
}
