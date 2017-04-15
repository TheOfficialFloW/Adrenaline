#ifndef __SCTRLLIBRARY_SE_H__
#define __SCTRLLIBRARY_SE_H__

/**
 * These functions are only available in SE-C and later, 
 * and they are not in HEN 
*/

enum 
{
	FAKE_REGION_DISABLED = 0,
	FAKE_REGION_JAPAN = 1,
	FAKE_REGION_AMERICA = 2,
	FAKE_REGION_EUROPE = 3,
	FAKE_REGION_KOREA = 4, 
	FAKE_REGION_UNK = 5,
	FAKE_REGION_UNK2 = 6,
	FAKE_REGION_AUSTRALIA = 7,
	FAKE_REGION_HONGKONG = 8,
	FAKE_REGION_TAIWAN = 9,
	FAKE_REGION_RUSSIA = 10,
	FAKE_REGION_CHINA = 11,
	FAKE_REGION_DEBUG_TYPE_I = 12,
	FAKE_REGION_DEBUG_TYPE_II = 13,
};

enum SEUmdModes
{
	MODE_INFERNO,
	MODE_MARCH33,
	MODE_NP9660,
};

#define ADRENALINE_CFG_MAGIC_1 0x31483943
#define ADRENALINE_CFG_MAGIC_2 0x334F4E33

typedef struct {
	int magic[2];
	int hidecorrupt;
	int	skiplogo;
	int startupprog;
	int umdmode;
	int	vshcpuspeed;
	int	umdisocpuspeed;
	int fakeregion;
	int skipgameboot;
	int hidemacaddr;
	int hidedlcs;
	int hidepic0pic1;
	int useextendedcolors;
	int usesonypsposk;
	int notusenodrmengine;
	int notusexmbplugins;
	int notusegameplugins;
	int notusepopsplugins;
	int forcehighmemory;
	int executebootbin;
} AdrenalineConfig;


/**
 * Gets the SE/OE version
 *
 * @returns the SE version
 *
 * 3.03 OE-A: 0x00000500
*/
int sctrlSEGetVersion();

/**
 * Gets the SE configuration.
 * Avoid using this function, it may corrupt your program.
 * Use sctrlSEGetCongiEx function instead.
 *
 * @param config - pointer to a AdrenalineConfig structure that receives the SE configuration
 * @returns 0 on success
*/
int sctrlSEGetConfig(AdrenalineConfig *config);

/**
 * Gets the SE configuration
 *
 * @param config - pointer to a AdrenalineConfig structure that receives the SE configuration
 * @param size - The size of the structure
 * @returns 0 on success
*/
int sctrlSEGetConfigEx(AdrenalineConfig *config, int size);

/**
 * Sets the SE configuration
 * This function can corrupt the configuration in flash, use
 * sctrlSESetConfigEx instead.
 *
 * @param config - pointer to a AdrenalineConfig structure that has the SE configuration to set
 * @returns 0 on success
*/
int sctrlSESetConfig(AdrenalineConfig *config);

/**
 * Sets the SE configuration
 *
 * @param config - pointer to a AdrenalineConfig structure that has the SE configuration to set
 * @param size - the size of the structure
 * @returns 0 on success
*/
int sctrlSESetConfigEx(AdrenalineConfig *config, int size);

/**
 * Initiates the emulation of a disc from an ISO9660/CSO file.
 *
 * @param file - The path of the 
 * @param noumd - Wether use noumd or not
 * @param isofs - Wether use the custom SE isofs driver or not
 * 
 * @returns 0 on success
 *
 * @Note - When setting noumd to 1, isofs should also be set to 1,
 * otherwise the umd would be still required.
 *
 * @Note 2 - The function doesn't check if the file is valid or even if it exists
 * and it may return success on those cases
 *
 * @Note 3 - This function is not available in SE for devhook
 * @Example:
 *
 * AdrenalineConfig config;
 *
 * sctrlSEGetConfig(&config);
 *
 * if (config.usenoumd)
 * {
 *		sctrlSEMountUmdFromFile("ms0:/ISO/mydisc.iso", 1, 1);
 * }
 * else
 * {
 *		sctrlSEMountUmdFromFile("ms0:/ISO/mydisc.iso", 0, config.useisofsonumdinserted);
 * }
*/
int sctrlSEMountUmdFromFile(char *file, int noumd, int isofs);

/**
 * Umounts an iso.
 *
 * @returns 0 on success
*/
int sctrlSEUmountUmd(void);

/**
 * Forces the umd disc out state
 *
 * @param out - non-zero for disc out, 0 otherwise
 *
*/
void sctrlSESetDiscOut(int out);

/**
 * Sets the disctype.
 *
 * @param type - the disctype (0x10=game, 0x20=video, 0x40=audio)
*/
void sctrlSESetDiscType(int type);

/**
 * Gets the current umd file (kernel only)
*/
char *sctrlSEGetUmdFile();
char *GetUmdFile();

/**
 * Sets the current umd file (kernel only)
 *
 * @param file - The umd file
*/
void sctrlSESetUmdFile(char *file);
void SetUmdFile(char *file);

/** 
 * Sets the boot config file for next reboot (kernel only)
 *
 * @param index - The index identifying the file (0 -> normal bootconf, 1 -> march33 driver bootconf, 2 -> np9660 bootcnf)
*/
void sctrlSESetBootConfFileIndex(int index);

void SetConfig(AdrenalineConfig *conf);
void SetSpeed(int cpu, int bus);

void oe_free(void *ptr);
void *oe_malloc(SceSize size);
int mallocinit();

#endif
