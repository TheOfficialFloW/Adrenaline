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

#ifndef __EXECUTABLE_PATCH_H__
#define __EXECUTABLE_PATCH_H__

typedef struct {
	u32 e_magic;
	u8 e_class;
	u8 e_data;
	u8 e_idver;
	u8 e_pad[9];
	u16 e_type;
	u16 e_machine;
	u32 e_version;
	u32 e_entry;
	u32 e_phoff;
	u32 e_shoff;
	u32 e_flags;
	u16 e_ehsize;
	u16 e_phentsize;
	u16 e_phnum;
	u16 e_shentsize;
	u16 e_shnum;
	u16 e_shstrndx;
} __attribute__((packed)) Elf32_Ehdr;

typedef struct {
	u32 sh_name;
	u32 sh_type;
	u32 sh_flags;
	u32 sh_addr;
	u32 sh_offset;
	u32 sh_size;
	u32 sh_link;
	u32 sh_info;
	u32 sh_addralign;
	u32 sh_entsize;
} __attribute__((packed)) Elf32_Shdr;

#define SCE_KERNEL_MAX_MODULE_SEGMENT           (4)

typedef struct {
    /** Unknown. */
    u32 unk0;
    /** The mode attribute of the executable file. One of ::SceExecFileModeAttr. */
    u32 modeAttribute; //4
    /** The API type. */
    u32 apiType; //8
    /** Unknown. */
    u32 unk12;
    /** The size of the executable, including the ~PSP header. */
    SceSize execSize; //16
    /** The maximum size needed for the decompression. */
    SceSize maxAllocSize; //20
    /** The memory ID of the decompression buffer. */
    SceUID decompressionMemId; //24
    /** Pointer to the compressed module data. */
    void *fileBase; //28
    /** Indicates the ELF type of the executable. One of ::SceExecFileElfType. */
    u32 elfType; //32 
    /** The start address of the TEXT segment of the executable in memory. */
    void *topAddr; //36
    /**
     * The entry address of the module. It is the offset from the start of the TEXT segment to the 
     * program's entry point. 
     */
    u32 entryAddr; //40
    /** Unknown. */
    u32 unk44;
    /** 
     * The size of the largest module segment. Should normally be "textSize", but technically can 
     * be any other segment. 
     */
    SceSize largestSegSize; //48
    /** The size of the TEXT segment. */
    SceSize textSize; //52
    /** The size of the DATA segment. */
    SceSize dataSize; //56
    /** The size of the BSS segment. */
    SceSize bssSize; //60
    /** The memory partition of the executable. */
    u32 partitionId; //64
    /** 
     * Indicates whether the executable is a kernel module or not. Set to 1 for kernel module, 
     * 0 for user module. 
     */
    u32 isKernelMod; //68
    /** 
     * Indicates whether the executable is decrypted or not. Set to 1 if it is successfully decrypted, 
     * 0 for encrypted. 
     */
    u32 isDecrypted; //72
    /** The offset from the start address of the TEXT segment to the SceModuleInfo section. */
    u32 moduleInfoOffset; //76
    /** The pointer to the module's SceModuleInfo section. */
    SceModuleInfo *moduleInfo; //80
    /** Indicates whether the module is compressed or not. Set to 1 if it is compressed, otherwise 0.*/
    u32 isCompressed; //84
    /** The module's attributes. One or more of ::SceModuleAttribute and ::SceModulePrivilegeLevel. */
    u16 modInfoAttribute; //88
    /** The attributes of the executable file. One of ::SceExecFileAttr. */
    u16 execAttribute; //90
    /** The size of the decompressed module, including its headers. */
    SceSize decSize; //92
    /** Indicates whether the module is decompressed or not. Set to 1 for decompressed, otherwise 0. */
    u32 isDecompressed; //96
    /** 
     * Indicates whether the module was signChecked or not. Set to 1 for signChecked, otherwise 0. 
     * A signed module has a "mangled" executable header, in other words, the "~PSP" signature can't 
     * be seen. 
     */
    u32 isSignChecked; //100
    /** Unknown. */
    u32 unk104;
    /** The size of the GZIP compression overlap. */
    SceSize overlapSize; //108
    /** Pointer to the first resident library entry table of the module. */
    void *exportsInfo; //112
    /** The size of all resident library entry tables of the module. */
    SceSize exportsSize; //116
    /** Pointer to the first stub library entry table of the module. */
    void *importsInfo; //120
    /** The size of all stub library entry tables of the module. */
    SceSize importsSize; //124
    /** Pointer to the string table section. */
    void *strtabOffset; //128
    /** The number of segments in the executable. */
    u8 numSegments; //132
    /** Reserved. */
    u8 padding[3]; //133
    /** An array containing the start address of each segment. */
    u32 segmentAddr[SCE_KERNEL_MAX_MODULE_SEGMENT]; //136
    /** An array containing the size of each segment. */
    u32 segmentSize[SCE_KERNEL_MAX_MODULE_SEGMENT]; //152
    /** The ID of the ELF memory block containing the TEXT, DATA and BSS segment. */
    SceUID memBlockId; //168
    /** An array containing the alignment information of each segment. */
    u32 segmentAlign[SCE_KERNEL_MAX_MODULE_SEGMENT]; //172
    /** The largest value of the segmentAlign array. */
    u32 maxSegAlign; //188
} SceLoadCoreExecFileInfo;

extern int (* _sceKernelCheckExecFile)(void *buf, SceLoadCoreExecFileInfo *execInfo);
extern int (* _sceKernelProbeExecutableObject)(void *buf, SceLoadCoreExecFileInfo *execInfo);
extern int (* PspUncompress)(void *buf, SceLoadCoreExecFileInfo *execInfo, u32 *newSize);
extern int (* PartitionCheck)(u32 *param, SceLoadCoreExecFileInfo *execInfo);

int sceKernelCheckExecFilePatched(void *buf, SceLoadCoreExecFileInfo *execInfo);
int sceKernelProbeExecutableObjectPatched(void *buf, SceLoadCoreExecFileInfo *execInfo);
int PspUncompressPatched(void *buf, SceLoadCoreExecFileInfo *execInfo, u32 *newSize);
int PartitionCheckPatched(u32 *param, SceLoadCoreExecFileInfo *execInfo);

#endif