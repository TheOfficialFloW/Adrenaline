/*
  Adrenaline
  Copyright (C) 2016-2017, TheFloW

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

#include <psp2/ctrl.h>
#include <psp2/io/dirent.h>
#include <psp2/io/fcntl.h>
#include <psp2/io/stat.h>
#include <psp2/kernel/sysmem.h>

#include <stdio.h>
#include <string.h>

#include "main.h"

#include "files.h"

#include "files/galaxy.h"
#include "files/inferno.h"
#include "files/kermit_idstorage.h"
#include "files/libpsardumper.h"
#include "files/march33.h"
#include "files/pspbtjnf.h"
#include "files/pspbtknf.h"
#include "files/pspbtlnf.h"
#include "files/pspbtmnf.h"
#include "files/pspbtrnf.h"
#include "files/systemctrl.h"
#include "files/popcorn.h"
#include "files/vshctrl.h"
#include "files/recovery.h"
#include "files/satelite.h"

typedef struct {
  char *name;
  void *buffer;
  uint32_t size;
} ScePspemuFlash0Package;

static ScePspemuFlash0Package custom_package[] = {
  { "/kd/galaxy.prx", galaxy, sizeof(galaxy) },
  { "/kd/inferno.prx", inferno, sizeof(inferno) },
  { "/kd/kermit_idstorage.prx", kermit_idstorage, sizeof(kermit_idstorage) },
  { "/kd/libpsardumper.prx", libpsardumper, sizeof(libpsardumper) },
  { "/kd/march33.prx", march33, sizeof(march33) },
  { "/kd/pspbtjnf.bin", pspbtjnf, sizeof(pspbtjnf) },
  { "/kd/pspbtknf.bin", pspbtknf, sizeof(pspbtknf) },
  { "/kd/pspbtlnf.bin", pspbtlnf, sizeof(pspbtlnf) },
  { "/kd/pspbtmnf.bin", pspbtmnf, sizeof(pspbtmnf) },
  { "/kd/pspbtrnf.bin", pspbtrnf, sizeof(pspbtrnf) },
  { "/kd/systemctrl.prx", systemctrl, sizeof(systemctrl) },
  { "/kd/popcorn.prx", popcorn, sizeof(popcorn) },
  { "/kd/vshctrl.prx", vshctrl, sizeof(vshctrl) },
  { "/vsh/module/recovery.prx", recovery, sizeof(recovery) },
  { "/vsh/module/satelite.prx", satelite, sizeof(satelite) },
};

#define SCE_PSPEMU_TEMP_SIZE 1 * 1024 * 1024

static void addFile(char *name, void *buffer, int size, void *data, uint32_t *pkg_ptr, uint32_t *name_ptr, uint32_t *buffer_ptr) {
  // Buffer
  *buffer_ptr = ALIGN(*buffer_ptr, 0x40);
  memcpy(data + *buffer_ptr, buffer, size);

  // Name
  strcpy(data + *name_ptr, name);

  // Header
  ScePspemuFlash0Package *header = (ScePspemuFlash0Package *)(data + *pkg_ptr);
  header->name = (char *)(SCE_PSPEMU_EXTRA_MEMORY + *name_ptr);
  header->buffer = (void *)(SCE_PSPEMU_EXTRA_MEMORY + *buffer_ptr);
  header->size = size;

  // Increase
  (*pkg_ptr) += sizeof(ScePspemuFlash0Package);
  (*name_ptr) += strlen(name) + 1;
  (*buffer_ptr) += size;
}

int ScePspemuBuildFlash0() {
  void *flash0_data = NULL, *temp_data = NULL;

  // Allocate flash0 memory
  SceUID flash0_blockid = sceKernelAllocMemBlock("ScePspemuFlash0", SCE_KERNEL_MEMBLOCK_TYPE_USER_RW_UNCACHE, SCE_PSPEMU_FLASH0_PACKAGE_SIZE, NULL);
  if (flash0_blockid < 0)
    return flash0_blockid;

  // Allocate temp memory
  SceUID temp_blockid = sceKernelAllocMemBlock("ScePspemuTemp", SCE_KERNEL_MEMBLOCK_TYPE_USER_RW_UNCACHE, SCE_PSPEMU_TEMP_SIZE, NULL);
  if (temp_blockid < 0)
    return temp_blockid;

  // Get base address
  sceKernelGetMemBlockBase(flash0_blockid, (void **)&flash0_data);
  sceKernelGetMemBlockBase(temp_blockid, (void **)&temp_data);

  // Clear flash0 data
  memset(flash0_data, 0, SCE_PSPEMU_FLASH0_PACKAGE_SIZE);

  ScePspemuFlash0Package *flash0_package = (ScePspemuFlash0Package *)(data_addr + 0xB9080);

  uint32_t pkg_ptr = 0, name_ptr = 0x1000, buffer_ptr = 0x2000;
  int i = 0, j = 0;

  // Add custom package
  for (i = 0; i < (sizeof(custom_package) / sizeof(ScePspemuFlash0Package)); i++) {
    addFile(custom_package[i].name, custom_package[i].buffer, custom_package[i].size, flash0_data, &pkg_ptr, &name_ptr, &buffer_ptr);
  }

  // Add package
  for (i = 0; i < N_FILES; i++) {
    while (flash0_package[j].buffer != NULL) {
      char *name = (char *)((uint32_t)flash0_package[j].name - SCE_PSPEMU_EXTRA_MEMORY + (uint32_t)flash0_package);
      void *buffer = (void *)((uint32_t)flash0_package[j].buffer - SCE_PSPEMU_EXTRA_MEMORY + (uint32_t)flash0_package);
      int size = flash0_package[j].size;

      if (strcmp(files[i], name) == 0) {
        addFile(name, buffer, size, flash0_data, &pkg_ptr, &name_ptr, &buffer_ptr);
        break;
      }

      j++;
    }
  }

  // Copy to .data
  memcpy(flash0_package, flash0_data, SCE_PSPEMU_FLASH0_PACKAGE_SIZE);

  // Copy to 0x8B000000
  void *flash0_package_psp = (void *)ScePspemuConvertAddress(SCE_PSPEMU_EXTRA_MEMORY, 0, SCE_PSPEMU_FLASH0_PACKAGE_SIZE);
  memcpy(flash0_package_psp, flash0_data, SCE_PSPEMU_FLASH0_PACKAGE_SIZE);
  ScePspemuWritebackCache(flash0_package_psp, SCE_PSPEMU_FLASH0_PACKAGE_SIZE);

  // Free blocks
  sceKernelFreeMemBlock(temp_blockid);
  sceKernelFreeMemBlock(flash0_blockid);

  // Make flash0 list
  ScePspemuFlash0Package *flash0_list = (ScePspemuFlash0Package *)(data_addr + 0xB7E88);
  memset(flash0_list, 0, 187 * sizeof(ScePspemuFlash0Package));

  i = 0;
  while (flash0_package[i].buffer != NULL) {
    char *name = (char *)((uint32_t)flash0_package[i].name - SCE_PSPEMU_EXTRA_MEMORY + (uint32_t)flash0_package);
    void *buffer = (void *)((uint32_t)flash0_package[i].buffer - SCE_PSPEMU_EXTRA_MEMORY + (uint32_t)flash0_package);
    int size = flash0_package[i].size;

    flash0_list[i].name = name;
    flash0_list[i].buffer = buffer;
    flash0_list[i].size = size;

    i++;
  }

  // Fix this weird thing
  int n_files = i;

  for (i = 0; i < 141; i++) {
    if (i < n_files)
      *(uint8_t *)(data_addr + 0x40D4 + i) = i;
    else
      *(uint8_t *)(data_addr + 0x40D4 + i) = 0;
  }

  return 0;
}
