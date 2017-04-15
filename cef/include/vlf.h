#ifndef __VLF_H__
#define __VLF_H__

typedef struct _VlfText *VlfText;
typedef struct _VlfPicture *VlfPicture;
typedef struct _VlfShadowedPicture *VlfShadowedPicture;
typedef struct _VlfBatteryIcon *VlfBatteryIcon;
typedef struct _VlfSpin *VlfSpin;
typedef struct _VlfCheckBox *VlfCheckBox;
typedef struct _VlfProgressBar *VlfProgressBar;
typedef struct _VlfScrollBar *VlfScrollBar;
typedef struct _VlfInputBox *VlfInputBox;


#define VLF_DEFAULT	-1

#define VLF_TITLEBAR_HEIGHT	22

#define VLF_ERROR_INVALID_INPUT		(-1)
#define VLF_ERROR_INVALID_INPUT_DATA	(-2)
#define VLF_ERROR_UNSUPPORTED_FORMAT	(-3)
#define VLF_ERROR_OBJECT_OVERFLOW	(-4)
#define VLF_ERROR_OBJECT_NOT_FOUND	(-5)
#define VLF_ERROR_NO_MEMORY		(-6)
#define VLF_ERROR_SYSTEM		(-7)
#define VLF_ERROR_DUPLICATED		(-8)


#define VLF_EV_RET_NOTHING			0
#define VLF_EV_RET_REMOVE_EVENT			1 
#define VLF_EV_RET_REMOVE_OBJECTS		2
#define VLF_EV_RET_REMOVE_HANDLERS		4
#define VLF_EV_RET_REFRESH_ON_DELAY		8
#define VLF_EV_RET_DELAY			0x80000000 /* Delay VLF_EV_RET_DELAY | (X << 16), 0 <= X <= 32767 milisecs */
#define VLF_EV_RET_DELAY_FRAME			(VLF_EV_RET_DELAY | 0x40000000) /* Delay VLF_EV_RET_DELAY_FRAME| (X << 16), 0 <= X <= 4095 */

/* Alignment */

enum VlfObjects
{
	VLF_TEXT = 0,
	VLF_PIC = 1,
	VLF_SHADOWED_PIC = 2,
	VLF_PROGRESS_BAR = 3
};

enum VlfTextAlignment
{
	VLF_ALIGNMENT_LEFT = 0,
 	VLF_ALIGNMENT_CENTER = 0x200,
  	VLF_ALIGNMENT_RIGHT = 0x400
};

enum VlfButtonIcon
{
	VLF_ENTER = 0,
	VLF_CANCEL = 1,
	VLF_CROSS = 2,
	VLF_CIRCLE = 3,
	VLF_TRIANGLE = 4,
	VLF_SQUARE = 5
};
/** Fade modes */

enum VlfFadeModesFlags
{
	VLF_FADE_MODE_IN = 1,
	VLF_FADE_MODE_OUT =	2,
	VLF_FADE_MODE_REPEAT = 4,
};

enum VlfFadeSpeed
{
	VLF_FADE_SPEED_STANDARD,
	VLF_FADE_SPEED_FAST,
	VLF_FADE_SPEED_VERY_FAST,
	VLF_FADE_SPEED_SLOW,
	VLF_FADE_SPEED_SUPER_FAST
};

enum VlfBatteryIconStatus
{
	VLF_BATTERY_ICON_HIGH,
 	VLF_BATTERY_ICON_MEDIUM,
  	VLF_BATTERY_ICON_LOW,
   	VLF_BATTERY_ICON_LOWEST
};

enum RCOType
{
	RCO_GRAPHIC,
	RCO_OBJECT,
	RCO_SOUND,
	RCO_LABEL,
	RCO_FILEPARAM,
	RCO_ANIMPARAM
};

enum VLF_MDType
{
	VLF_MD_TYPE_ERROR,
	VLF_MD_TYPE_NORMAL,
};

enum VLF_MD_Buttons
{
	VLF_MD_BUTTONS_NONE = 0,
	VLF_MD_BUTTONS_YESNO = 0x10,
};

enum VLF_MD_InitalCursor
{
	VLF_MD_INITIAL_CURSOR_YES = 0,
	VLF_MD_INITIAL_CURSOR_NO = 0x100,
};

enum VLF_MD_ButtonRes
{
	VLF_MD_NONE,
	VLF_MD_YES,
	VLF_MD_NO,
	VLF_MD_BACK
};

enum VLF_DialogItem
{
	VLF_DI_ENTER,
	VLF_DI_CANCEL,
	VLF_DI_BACK,
	VLF_DI_YES,
	VLF_DI_NO,
	VLF_DI_EDIT,
};

enum VLF_SpinState
{
	VLF_SPIN_STATE_NOT_FOCUS, // Spin control has not focus, buttons are not listened
	VLF_SPIN_STATE_FOCUS, // Spin control text has focus but arrow is not shown, buttons are not listened
	VLF_SPIN_STATE_ACTIVE, // Spin control has focus, and it is active  (arrows are shown, up and down buttons are listened)
};

enum VLF_InputBoxType
{
	VLF_INPUTBOX_TYPE_NORMAL,
 	VLF_INPUTBOX_TYPE_PASSWORD
};

enum PspCtrlExtension
{
	PSP_CTRL_ENTER = 0x40000000,
	PSP_CTRL_CANCEL = 0x80000000
};

/**
 * Inits VLF library
 *
 * @param heap_size - The heap size to be allocated. It can be negative.
 * @param app_main - The program main application.
*/
void vlfGuiInit(int heap_size, int (* app_main)(int argc, char *argv[]));

/**
 * Performs typical application initialization tasks (adding background, system model, and optionally battery icon and clock).
 *
 * @param battery - Inidicates if a battery icon should be added.
 * @param clock - Indicates if a clock should be added.
 * @param notuserwp - If user configuration is set to use a custom background and this param is 1, then the custom wallpaper won't be used.
 *
 * @returns 0 on success
 *
 * @Notes: If an user wallpaper is used, the background model ("waves") won't be added.
 */
int vlfGuiSystemSetup(int battery, int clock, int notuserwp);

/**
 * Gets language used by vlf application
 *
 * @returns - The language set to be used by current vlf application
 * By default is initialized to user language.
*/
int vlfGuiGetLanguage();

/**
 * Sets language to be used by blf application
 *
 * @param lang - The language to be set. 
 * This only sets the language to be used by current vlf application,
 * it doesn't overwrite user preferences in flash.
 */
void vlfGuiSetLanguage(int lang);

/**
 * Gets the button configuration used by vlf application (0 -> circle is enter, 1 -> cross is enter)
 *
 * @returns - The button configuration set to be used by current vlf application.
 * By default is initialized with user preferences.
*/
int vlfGuiGetButtonConfig();

/**
 * Sets the button configuration to be used by current vlf application.
 *
 * @param config - The button configuration to be set (0 -> circle is enter, 1 -> cross is enter)
 * This only sets the button configuration to be used by current vlf application, 
 * it doesn't overwrite user preferences in flash.
*/
void vlfGuiSetButtonConfig(int config);

/**
 * Sets the directories where resources are located.
 *
 * @param dir - The directory that will be used to locate resources (max 256 chars including '\0')
 * By default is initialized to flash0:/vsh/resource
*/
void vlfGuiSetResourceDir(char *dir);

/**
 * Performs the draw of next frame
*/
void vlfGuiDrawFrame();

/**
 * Loads resources from a rco file
 *
 * @param rco - It can be one of following things:
 * - path relative to the <resource_dir> directory without extension (e.g. "system_plugin_bg")
 * - path relative to the <resource_dir> directory with extension (e.g. "system_plugin_bg.rco")
 * - path to a file (e.g. "flash0:/vsh/resource/system_plugin_bg.rco", "ms0:/myresfile.rco")
 *
 * RCO param is evaluated in the order given above, so if a rco file exists in current directory with name 
 * "system_plugin_bg.rco", it would load the one of <resource_dir> and not the one of current directory. (in such a case, use "./system_plugin_bg.rco")
 * 
 * @param n - The number of resources to loads
 * @param names (IN) - An array with the names of resources
 * @param types (IN) - An array with the types of the resources (one of RCOType)
 * @param datas (OUT) - A pointer to a variable that will receive an array of pointers to the content of each resource,
 * or NULL if a specific resource has not been found.
 * Pointers returned are allocated with malloc, and should be deallocated by the application.
 *
 * @param sizes (OUT) - It will receive the sizes of the resources
 * @param pntable (OUT) - A pointer that will receive the string table. Pass NULL if no required.
 * Returned pointer is allocated with malloc and should be deallocated by the application.
 *
 * @returns - the number of resources loaded, or < 0 if there is an error.
 *
 * @Example: Load battery icon pic and shadow
 *
 * char *names[2];
 * void *datas[2];
 * int types[2], sizes[2];
 *
 * names[0] = "tex_battery";
 * names[1] = "tex_battery_shadow";
 * types[0] = types[1] = RCO_GRAPHIC;
 *
 * int res = vlfGuiLoadResources("system_plugin_fg", 2, names, types, datas, sizes, NULL);
 * if (res != 2) // error or not all resources loaded
 * {
 *    if (res > 0)
 *    {
 *       if (datas[0])
 *          free(datas[0]);
 *       if (datas[1])
 *          free(datas[1]);
 *    }
 * }
 * else
 * {
 *    void *bat;    
 *    vlfGuiAddShadowedPicture(&bat, datas[0], sizes[0], datas[1], sizes[1], 441, 4, 1, 1, 1); 
 *    free(datas[0]);
 *    free(datas[1]);
 * }
 *
*/
int vlfGuiLoadResources(char *rco, int n, char **names, int *types, void **datas, int *sizes, char **pntable);

/**
 * Caches a resource in RAM, so it doesn't have to be loaded from storage device anymore.
 *
 * @param rco - The resource. Same rules apply to this param
 * 
 * @returns - < 0 on error.
*/
int vlfGuiCacheResource(char *rco);

/**
 * Uncaches a resource previously cached.
 *
 * @param rco - The resource to be uncached.
 * 
 * @returns - < 0 on error.
*/
int vlfGuiUncacheResource(char *rco);


//int vlfGuiGetResourceSubParam(void *entry, int insize, char *ntable, char *name, void **data, int *size);

/**
 * Loads an unicode string from a resource.
 *
 * @param str - Buffer that receives the string
 * @param rco - The resource file to load the label from.
 * @param name - The name of the resource
 *
 * @returns - < 0 on error.
*/
int vlfGuiLoadLabel(u16 *str, char *rco, char *name);

/**
 * Sets the background from 8888 texture data
 *
 * @param texture - The texture data in 8888 format
 * @param width - The width of texture. Must be a power of 2.
 * @param height - The height of texture. Must be multiple of 8.
 * @param swizzled - Indicates if the texture is already in the psp GE fast texture format
 * @param scale_x - The x scale to apply
 * @param scale_y - The y scale to apply
 *
 * @returns 0 on success, or < 0 on error (params invalid)
*/
int vlfGuiSetBackground(u32 *texture, int width, int height, int swizzled, float scale_x, float scale_y);

/**
 * Sets the background from a file buffer.
 * Supported formats are currently: BMP, TIM, GIM and PNG, with a depth of 24 or 32 bits.
 *
 * @param data - The buffer with the file data
 * @param size - The size of the data
 * @param scale - Wether to scale the image. If it is 0, the image will be centered and filled by black.
 *
 * @returns - 0 on success, < 0 on error.
*/
int vlfGuiSetBackgroundFileBuffer(void *data, int size, int scale);

/**
 * Sets the background from a file
 * Supported formats are currently: BMP, TIM, GIM and PNG, with a depth of 24 or 32 bits.
 *
 * @param file - Path to the file.
 * @param scale - Wether to scale the image. If it is 0, the image will be centered and filled by black.
 *
 * @returns - 0 on success, < 0 on error.
*/
int vlfGuiSetBackgroundFile(char *file, int scale);

/**
 * Sets one of system backgrounds based on the index.
 *
 * @param index - The index of the background, valid values are 1-27 
 * (note that 13-27 is only available on slim and will return an error on old psp)
 *
 * @returns 0 on success, < 0 on error
*/
int vlfGuiSetBackgroundIndex(int index);

/**
 * Sets one of system backgrounds based on the current date
 *
 * @returns 0 on success, < 0 on error
*/
int vlfGuiSetBackgroundDate();

/** 
 * Sets a background of a single color
 *
 * @param color - The color in XXBBGGRR format (XX is ignored).
 *
 * @returns - this functions always succeeds returning 0
*/
int vlfGuiSetBackgroundPlane(u32 color);

/**
 * Sets the background according to the system configuration
 *
 * @returns - 0 on success, < 0 on error.
*/
int vlfGuiSetBackgroundSystem(int notuserwp);

/**
 * Sets the system color, used in titlebars or menus
 *
 * @param index - the index of the color, 1-27
*/
void vlfGuiSetSystemColor(int index); 

/**
 * Sets the background model from a buffer.
 *
 * @param data - The buffer with the model in GMO format
 * @param size - The size of the model
 *
 * @returns - 0 on success, < 0 on error.
*/
int vlfGuiSetModel(void *data, int size);

/**
 * Sets the background model from a file.
 *
 * @param file - The file with the model in GMO format
 *
 * @returns - 0 on success, < 0 on error.
*/
int vlfGuiSetModelFile(char *file);

/**
 * Sets the background model from a resource.
 *
 * @param rco - The path to the RCO file
 * @param name - The name of the resource
 *
 * @returns - 0 on success, < 0 on error.
*/
int vlfGuiSetModelResource(char *rco, char *name);

/**
 * Sets the background model of the system, and applies the proper world matrix to it.
 *
 * @returns 0 on success, < 0 on error.
*/
int vlfGuiSetModelSystem();

/**
 * Sets the world matrix for the model. (by default, the world matrix is the identity 
 * after a model has been loaded, except when calling vlfGuiSetModelSystem).
 *
 * @param matrix - The matrix to set.
 *
 * @Example: Load waves (this sample assumes the default scale of 8.5)
 *
 * int res = vlfGuiSetModelResource("system_plugin_bg", "mdl_bg");
 * if (res < 0) process_error;
 *
 * ScePspFMatrix4 matrix;
 * ScePspFVector3 scale;
 *
 * scale.x = scale.y = scale.z = 8.5f;
 * gumLoadIdentity(&matrix);
 * gumScale(&matrix, &scale);
 * vlfGuiSetModelWorldMatrix(&matrix);
*/
void vlfGuiSetModelWorldMatrix(ScePspFMatrix4 *matrix);

/**
 * Gets the world matrix of the model
 *
 * @returns a pointer to the model world matrix
*/
ScePspFMatrix4 *vlfGuiGetModelWorldMatrix();

/**
 * Sets the model speed 
 *
 * @param speed - The speed, default model speed is 1.0f/60.0f
*/
void vlfGuiSetModelSpeed(float speed);

/**
 * Sets a title bar with the current system color.
 *
 * @param text - Text of the title bar. Pass NULL if no required.
 * @param pic - Picture of the title bar. Pass NULL if no required.
 * @param visible - If the tile bar will be visible
 * @param hideobj - If 1, it will hide objects that were current added within the area of the title bar.
 */
void vlfGuiSetTitleBar(VlfText text, VlfPicture pic, int visible, int hideobj);

/**
 * Sets a title bar with the desired color.
 *
 * @param text - Text of the title bar. Pass NULL if no required.
 * @param pic - Picture of the title bar. Pass NULL if no required.
 * @param visible - If the tile bar will be visible
 * @param hideobj - If 1, it will hide objects that were current added within the area of the title bar.
 * @param color - The color of the title bar.
*/
void vlfGuiSetTitleBarEx(VlfText text, VlfPicture pic, int visible, int hideobj, u32 color);

/**
 * Sets the tile bar visibility.
*/
void vlfGuiSetTitleBarVisibility(int visible);

/**
 * Adds a new text item from an ascii string.
 *
 * @param x - x position
 * @param y - y position
 * @param string - ascii string with the desired text
 *
 * @returns a VlfText item on success, NULL on error.
*/
VlfText vlfGuiAddText(int x, int y, char *string);

/**
 * Adds a new text item from an unicode string.
 *
 * @param x - x position
 * @param y - y position
 * @param string - unicode string with the desired text
 *
 * @returns a VlfText item on success, NULL on error.
*/
VlfText vlfGuiAddTextW(int x, int y, u16 *string);

/**
 * Adds a new text item from a string with format
 *
 * @param x - x position
 * @param y - y position
 * @param fmt - string with format
 *
 * @returns a VlfText item on success, NULL on error.
*/
VlfText vlfGuiAddTextF(int x, int y, char *fmt, ...);

/**
 * Adds a new text item from a resource label
 *
 * @param rco - The resource file to load the label from.
 * @param name - The name of the resource.
 * @param x - x position
 * @param y - y position 
 *
 * @returns a VlfText item on success, NULL on error.
*/
VlfText vlfGuiAddTextResource(char *rco, char *name, int x, int y);

/**
 * Removes a text item.
 *
 * @param text - The text item to remove
 *
 * @returns - < 0 on error.
*/
int vlfGuiRemoveText(VlfText text);

/**
 * Sets the text of a VlfText item from an ascii string.
 *
 * @param text - The text item 
 * @param string - The ascii string to set.
 *
 * @returns - < 0 on error.
*/
int vlfGuiSetText(VlfText text, char *string);

/**
 * Sets the text of a VlfText item from an unicode string.
 *
 * @param text - The text item 
 * @param string - The unicode string to set.
 *
 * @returns - < 0 on error.
*/
int vlfGuiSetTextW(VlfText text, u16 *string);

/**
 * Sets the text of a VlfText item from a string with format.
 *
 * @param text - The text item 
 * @param fmt - The  string with format.
 *
 * @returns - < 0 on error.
*/
int vlfGuiSetTextF(VlfText text, char *fmt, ...);

/**
 * Sets the text of a VlfText item from a resource label.
 *
 * @param text - The text item 
 * @param rco - The resource file to load the label from.
 * @param name - The name of the resource.
 *
 * @returns - < 0 on error.
*/
int vlfGuiSetTextResource(VlfText text, char *rco, char *name);

/**
 * Sets focus on a VlfText item.
 *
 * @param text - The text item to set the focus.
 *
 * @returns - < 0 on error.
 *
 * @Note: this function should only be used with a text with a single line, and
 * with default font size.
*/
int vlfGuiSetTextFocus(VlfText text);

/** 
 * Removes focus on a VlfText item previously focused.
 *
 * @param text - The text item to remove focus.
 *
 * @returns - < 0 on error.
*/
int vlfGuiRemoveTextFocus(VlfText text, int keepres);

/**
 * Sets the visibility of a VlfText item.
 *
 * @param text - The text item.
 * @param visible - boolean for the visibility.
 *
 * @returns - < 0 on error.
*/
int vlfGuiSetTextVisibility(VlfText text, int visible);

/**
 * Makes a VlfText item to blink.
 *
 * @param text - The text item to set blinking.
 * @param nshow - The number of frames the item will be shown.
 * @param nhide - The number of frames the item wil be hidden.
 *
 * @returns - < 0 on error
 *
 * @Notes: To remove blinking, pass both params to 0.
*/
int vlfGuiSetTextBlinking(VlfText text, u32 nshow, u32 nhide);

/**
 * Makes a VlfText item to fade.
 *
 * @param text - The text item to fade.
 * @param mode - Whatever OR combination of VlfFadeModesFlags
 * @param speed - The fade speed, one of VlfFadeSpeed
 * @param direction_out - If both, VLF_FADE_MODE_IN and VLF_FADE_MODE_OUT, were specified, this param indicates
 * wether to start from direction out or in. Otherwise, it is ignored.
 *
 * @returns - < 0 on error.
*/
int vlfGuiSetTextFade(VlfText text, int mode, int speed, int direction_out);

/**
 * Cancels a VlfText item fade.
 *
 * @param text - The text item to remove fade.
 *
 * @returns - < 0 on error
*/
int vlfGuiCancelTextFade(VlfText text);

/**
 * Sets a callback to report the end of a fade.
 *
 * @param text - The text item fading.
 * @param callback - The callback function that will be called at end of fade.
 * @param param - param that will be passed to the callback function
 * @param delay - The delay between the end of fade and the call to the callback
 * 
 * @returns - < 0 on error
*/
int vlfGuiSetTextFadeFinishCallback(VlfText text, void (* callback)(void *), void *param, int delay);

/**
 * Sets text item alignment
 *
 * @param text - The text item to set alignment
 * @param alignment - One of VlfTextAlignment values
 *
 * @returns - < 0 on error
*/
int vlfGuiSetTextAlignment(VlfText text, int alignment);

/**
 * Sets text position
 *
 * @param text - The text item to set position
 * @param x - The x position
 * @param y - The y position
 *
 * @returns - < 0 on error
*/
int vlfGuiSetTextXY(VlfText text, int x, int y);

/**
 * Sets the font size of a text item
 *
 * @param text - The text to set font size
 * @param size - The size of the font
 *
 * @returns - < 0 on error
*/
int vlfGuiSetTextFontSize(VlfText text, float size);

/**
 * Returns the size of a text item.
 *
 * @param text - The text item yo get size from
 * @param width - pointer to a variable that receives the width
 * @param height - pointer to a variable that receivs the height
 *
 * @returns - < 0 on error
*/
int vlfGuiGetTextSize(VlfText text, int *width, int *height);

/**
 * Sets a scrollbar in the specified text item.
 *
 * @param text - The text item
 * @param height - The height of the scrollbar
 *
 * @returns - < 0 on error
*/
int vlfGuiSetTextScrollBar(VlfText text, int height);

/**
 * Sets a scrollbar in the specified text item (with more options)
 *
 * @param text - The text item
 * @param height - The height of the scrollbar
 * @param dispx - displacement between text and scrollbar in X axis
 * @param dispy - displacement between text and scrollbar in Y axis
 *
 * @returns - < 0 on error
 */
int vlfGuiSetTextScrollBarEx(VlfText text, int height, int dispx, int dispy);

/**
 * Removes a scrollbar of a text item
 *
 * @param text - The text item
 *
 * @returns - < 0 on error
*/
int vlfGuiRemoveTextScrollBar(VlfText text);

/**
 * Sets a character that will be replaced by a button icon
 *
 * @param ch - The character that will be replaced.
 * @param button - The button used for the replacement, one of VlfButtonIcon
 *
 * @returns - < 0 on error
*/
int vlfGuiChangeCharacterByButton(u16 ch, int button);

/**
 * Adds a new picture item from a buffer.
 * Supported formats are GIM, TIM, BMP and PNG.
 *
 * @param data - The buffer with the picture
 * @param size - The size of data buffer
 * @param x - x position
 * @param y - y position
 *
 * @returns - a new VlfPivture on success, NULL on error
*/
VlfPicture vlfGuiAddPicture(void *data, int size, int x, int y);

/**
 * Adds a new picture item from a file.
 * Supported formats are GIM, TIM, BMP and PNG.
 *
 * @param file - The file with the picture 
 * @param x - x position
 * @param y - y position
 *
 * @returns - a new VlfPivture on success, NULL on error
*/
VlfPicture vlfGuiAddPictureFile(char *file, int x, int y);

/**
 * Adds a new picture item from a resource.
 * Supported formats are GIM, TIM, BMP and PNG.
 *
 * @param rco - The rco
 * @param name - The name of the resource
 * @param x - x position
 * @param y - y position
 *
 * @returns - a new VlfPivture on success, NULL on error
*/
VlfPicture vlfGuiAddPictureResource(char *rco, char *name, int x, int y);

/**
 * Removes a picture
 * 
 * @param pic - The picture to remove
 *
 * @returns - < 0 on error.
*/
int vlfGuiRemovePicture(VlfPicture pic);

/**
 * Sets a picture position.
 *
 * @param pic - The picture 
 * @param x - x position
 * @param y - y position
 *
 * @returns - < 0 on error
*/
int vlfGuiSetPictureXY(VlfPicture pic, int x, int y);

/**
 * Gets a picture size.
 *
 * @param pic - The picture 
 * @param width - pointer to a variable that receives the width
 * @param height - pointer to a variable that receives the height
 *
 * @returns - < 0 on error
*/
int vlfGuiGetPictureSize(VlfPicture pic, int *width, int *height);

/**
 * Sets the picture display area
 *
 * @param pic - The picture
 * @param x - x position of the display area, relative to the top left corner of the picture.
 * @param y - y position of the display area, relative to the top left corner of the picture.
 * @param width - the width of the rectangle display area
 * @param height - The height of the rectangle display area
 *
 * @returns - < 0 on error
*/
int vlfGuiSetPictureDisplayArea(VlfPicture pic, int x, int y, int width, int height);

/**
 * Sets picture alpha blend operation.
 *
 * @param pic - the picture
 * @param op - Blending operation (see pspgu.h)
 * @param src - Blending function for source operand (see pspgu.h)
 * @param dst - Blending function for dest operand (see pspgu.h)
 * @param srcfix - Fix value for GU_FIX (source operand)
 * @param destfix - Fix value for GU_FIX (dest operand)
 *
 * @returns - < 0 on error
*/
int vlfGuiSetPictureAlphaBlend(VlfPicture pic, int op, int src, int dst, u32 srcfix, u32 dstfix);

/**
 * Clones (duplicate) a picture
 *
 * @param pic - The picture to clone
 * @param real - If 0, then the picture is not totally copied, but only a reference. Otherwise, a total duplication is performed.
 * @param x - The x position for the cloned picture
 * @param y - The y position for the cloned picture
 *
 * @returns - The cloned picture
*/
VlfPicture vlfGuiClonePicture(VlfPicture pic, int real, int x, int y);

/**
 * Sets picture visibility
 *
 * @param pic - The picture
 * @param visible - boolean indicating visibility
 *
 * @returns - < 0 on error
*/
int vlfGuiSetPictureVisibility(VlfPicture pic, int visible);

/**
 * Makes a picture blink
 *
 * @param pic - The picture to blink
 * @param nshow - The number of frames the picture will be shown
 * @param nhide - The number of frames the picture will be hidden
 *
 * @returns - < 0 on error
*/
int vlfGuiSetPictureBlinking(VlfPicture pic, u32 nshow, u32 nhide);

/**
 * Animates a picture.
 * Frames are created from rectangle areas of the picture.
 *
 * @param pic - The picture to animate
 * @param w - The width of each frame, must be a divisor of picture width
 * @param h - The height of each frame, must be a divisor of picture height
 * @param frames - The number of frames each frame is drawn.
 * @param vertical - If 0, animaction is created from rectangles in horizontal direction. Otherwise, from vertical direction.
 *
 * @returns - < 0 on error
*/
int vlfGuiAnimatePicture(VlfPicture pic, int w, int h, int frames, int vertical);

/**
 * Makes a VlfPicture item to fade.
 *
 * @param pic - The picture item to fade.
 * @param mode - Whatever OR combination of VlfFadeModesFlags
 * @param speed - The fade speed, one of VlfFadeSpeed
 * @param direction_out - If both, VLF_FADE_MODE_IN and VLF_FADE_MODE_OUT, were specified, this param indicates
 * wether to start from direction out or in. Otherwise, it is ignored.
 *
 * @returns - < 0 on error.
 */
int vlfGuiSetPictureFade(VlfPicture pic, int mode, int effect, int direction_out);

/**
 * Cancels a VlfPicture item fade.
 *
 * @param pic - The picture item to remove fade.
 *
 * @returns - < 0 on error
*/
int vlfGuiCancelPictureFade(VlfPicture pic);

/**
 * Sets a callback to report the end of a fade.
 *
 * @param pic - The picture item fading.
 * @param callback - The callback function that will be called at end of fade.
 * @param param - param that will be passed to the callback function
 * @param delay - The delay between the end of fade and the call to the callback
 * 
 * @returns - < 0 on error
*/
int vlfGuiSetPictureFadeFinishCallback(VlfPicture pic, void (* callback)(void *), void *param, int delay);

/**
 * Adds a new shadowed picture from two buffers.
 *
 * @param pic - The buffer with the main picture
 * @param pic_size - The size of the pic buffer
 * @param shpic - The buffer with the shadow picture
 * @param shpic_size - The size of the shpic buffer
 * @param x - x position
 * @param y - y position
 * @param sh_offsx - distance between the main picture and the shadow (x)
 * @param sh_offsy - distance between the main picture and the shadow (y)
 * @param shadow_before - indicates if shadow should be painted before
 *
 * @returns - a new shadowed picture on success, NULL on error
*/
VlfShadowedPicture vlfGuiAddShadowedPicture(void *pic, int pic_size, void *shpic, int shpic_size, int x, int y, int sh_offsx, int sh_offsy, int shadow_before);

/**
 * Adds a new shadowed picture from two files
 *
 * @param pic - The file with the main picture
 * @param shpic - The file with the shadow picture
 * @param x - x position
 * @param y - y position
 * @param sh_offsx - distance between the main picture and the shadow (x)
 * @param sh_offsy - distance between the main picture and the shadow (y)
 * @param shadow_before - indicates if shadow should be painted before
 *
 * @returns - a new shadowed picture on success, NULL on error
*/
VlfShadowedPicture vlfGuiAddShadowedPictureFile(char *pic, char *shpic, int x, int y, int sh_offsx, int sh_offsy, int shadow_before);

/**
 * Adds a new shadowed picture from two resources.
 *
 * @param rco .- The resource file
 * @param pic - The name of the resource with the main picture
 * @param shpic - The name of the resource with the shadow picture
 * @param x - x position
 * @param y - y position
 * @param sh_offsx - distance between the main picture and the shadow (x)
 * @param sh_offsy - distance between the main picture and the shadow (y)
 * @param shadow_before - indicates if shadow should be painted before
 *
 * @returns - a new shadowed picture on success, NULL on error
*/
VlfShadowedPicture vlfGuiAddShadowedPictureResource(char *rco, char *pic, char *shpic, int x, int y, int sh_offsx, int sh_offsy, int shadow_before);

/**
 * Removes a shadowed picture
 * 
 * @param sp - The shadowed picture to remove
 *
 * @returns - < 0 on error.
*/
int vlfGuiRemoveShadowedPicture(VlfShadowedPicture sp);

/**
 * Sets shadowed picture visibility
 *
 * @param sp - The shadowed picture
 * @param visible - boolean indicating visibility
 *
 * @returns - < 0 on error
*/
int vlfGuiSetShadowedPictureVisibility(VlfShadowedPicture sp, int visible);

/**
 * Makes a shadowed picture blink
 *
 * @param sp - The shadowed picture to blink
 * @param nshow - The number of frames the picture will be shown
 * @param nhide - The number of frames the picture will be hidden
 *
 * @returns - < 0 on error
*/
int vlfGuiSetShadowedPictureBlinking(VlfShadowedPicture sp, u32 nshow, u32 nhide);

/**
 * Animates a shadowed picture.
 * Frames are created from rectangle areas of the picture.
 *
 * @param sp - The shadowed picture to animate
 * @param w - The width of each frame of main picture, must be a divisor of main picture width
 * @param h - The height of each frame of main picure, must be a divisor of main picture height
 * @param ws - The width of each frame of shadow picture, must be a divisor of shadow picture width
 * @param hs - The height of each frame of shadow picure, must be a divisor of shadow picture height
 * @param frames - The number of frames each frame is drawn
 * @param vertical - If 0, animaction is created from rectangles in horizontal direction. Otherwise, from vertical direction.
 *
 * @returns - < 0 on error
 *
 * @Example: vlfGuiAddWaitIconEx source code. Image is divided in 17x17 rectangles, each frame is drawn for 3 frames.
 *
 * VlfShadowedPicture vlfGuiAddWaitIconEx(int x, int y)
 * {
 * 	VlfShadowedPicture res = vlfGuiAddShadowedPictureResource("system_plugin_fg", "tex_busy", "tex_busy_shadow", x, y, 1, 1, 0);
 *
 *	if (!res)
 *		return NULL;
 *
 *	if (vlfGuiAnimateShadowedPicture(res, 17, 17, 17, 17, 3, 1) < 0)
 *	{
 *		vlfGuiRemoveShadowedPicture(res);
 *		return NULL;
 *	}
 *
 *	return res;
 * }
*/
int vlfGuiAnimateShadowedPicture(VlfShadowedPicture sp, int w, int h, int ws, int hs, int frames, int vertical);

/**
 * Makes a VlfShadowedPicture item to fade.
 *
 * @param sp - The shadowed picture item to fade.
 * @param mode - Whatever OR combination of VlfFadeModesFlags
 * @param speed - The fade speed, one of VlfFadeSpeed
 * @param direction_out - If both, VLF_FADE_MODE_IN and VLF_FADE_MODE_OUT, were specified, this param indicates
 * wether to start from direction out or in. Otherwise, it is ignored.
 *
 * @returns - < 0 on error.
 */
int vlfGuiSetShadowedPictureFade(VlfShadowedPicture sp, int mode, int effect, int direction_out);

/**
 * Cancels a VlfShadowedPicture item fade.
 *
 * @param sp - The shadowed picture item to remove fade.
 *
 * @returns - < 0 on error
 */
int vlfGuiCancelShadowedPictureFade(VlfShadowedPicture sp);

/**
 * Sets a callback to report the end of a fade.
 *
 * @param sp - The shadowed picture item fading.
 * @param callback - The callback function that will be called at end of fade.
 * @param param - param that will be passed to the callback function
 * @param delay - The delay between the end of fade and the call to the callback
 * 
 * @returns - < 0 on error
*/
int vlfGuiSetShadowedPictureFadeFinishCallback(VlfShadowedPicture sp, void (* callback)(void *), void *param, int delay);

/**
 * Adds a new battery icon at its default position. Icon created by this function must be manually operated by programmer.
 *
 * @param status - The status, one of VlfBatteryIconStatus
 * @param blink - blinking flag
 *
 * @returns - a new VlfBatteryIcon item
*/
VlfBatteryIcon vlfGuiAddBatteryIcon(u32 status, int blink);

/**
 * Adds a new battery icon (with more options). Icon created by this function must be manually operated by programmer.
 *
 * @param x - The x position
 * @param y - The y position
 * @param status - The status, one of VlfBatteryIconStatus
 * @param blink - blinking flag
 *
 * @returns - a new VlfBatteryIcon item
 */
VlfBatteryIcon vlfGuiAddBatteryIconEx(int x, int y, u32 status, int blink);

/**
 * Adds a new battery icon at its default position.
 * Icon created by this function is handled automatically by the library.
 *
 * @param timer_ms - The miliseconds the timer that checks the battery is executed.
 *
 * @returns - a new VlfBatteryIcon item
*/
VlfBatteryIcon vlfGuiAddBatteryIconSystem(int timer_ms);

/**
 * Sets battery icon status.
 *
 * @param baticon - The battery icon
 * @param status - The status, one of VlfBatteryIconStatus
 * @param blink - blinking flag
 *
 * @returns - < 0 on error
 *
 * @Note: this function shouldn't be used with a battery icon created by vlfGuiAddBatteryIconSystem.
*/
int vlfGuiSetBatteryIconStatus(VlfBatteryIcon baticon, int status, int blink);

/**
 * Removes a battery icon.
 * 
 * @param baticon - The battery icon to remove
 *
 * @returns - < 0 on error
*/
int vlfGuiRemoveBatteryIcon(VlfBatteryIcon baticon);

/**
 * Adds a system clock. Only one clock is allowed currently
 *
 * @returns - < 0 on error.
 *
 * @Note: this function will change in future versions.
*/
int vlfGuiAddClock();

/**
 * Adds an animated wait icon at its default position in lower-right corner.
 *
 * @returns - a new wait icon.
*/
VlfShadowedPicture vlfGuiAddWaitIcon();

/**
 * Adds an animated wait icon at position choosed by programmer.
 *
 * @param x - x position
 * @param y - y position
 *
 * @returns - a new wait icon.
 */
VlfShadowedPicture vlfGuiAddWaitIconEx(int x, int y);

/**
 * Adds a cross picture.
 *
 * @param x - x position
 * @param y - y position
 *
 * @returns - a new cross picture
*/
VlfShadowedPicture vlfGuiAddCross(int x, int y);

/**
 * Adds a circle picture.
 *
 * @param x - x position
 * @param y - y position
 *
 * @returns - a new circle picture
*/
VlfShadowedPicture vlfGuiAddCircle(int x, int y);

/**
 * Adds a triangle picture.
 *
 * @param x - x position
 * @param y - y position
 *
 * @returns - a new triangle picture
*/
VlfShadowedPicture vlfGuiAddTriangle(int x, int y);

/**
 * Adds a square picture.
 *
 * @param x - x position
 * @param y - y position
 *
 * @returns - a new square picture
*/
VlfShadowedPicture vlfGuiAddSquare(int x, int y);

/**
 * Adds an enter picture (cross or circle)
 *
 * @param x - x position
 * @param y - y position
 *
 * @returns - a new enter picture
*/
VlfShadowedPicture vlfGuiAddEnter(int x, int y);

/**
 * Adds a cancel picture (cross or circle)
 *
 * @param x - x position
 * @param y - y position
 *
 * @returns - a new cancel picture
*/
VlfShadowedPicture vlfGuiAddCancel(int x, int y);

/**
 * Adds a spinup picture.
 *
 * @param x - x position
 * @param y - y position
 *
 * @returns - a new spinup picture
*/
VlfShadowedPicture vlfGuiAddSpinUp(int x, int y);

/**
 * Adds a spindown picture.
 *
 * @param x - x position
 * @param y - y position
 *
 * @returns - a new spindown picture
*/
VlfShadowedPicture vlfGuiAddSpinDown(int x, int y);

/**
 * Adds an arrow left picture.
 *
 * @param x - x position
 * @param y - y position
 *
 * @returns - a new arrow left picture
*/
VlfShadowedPicture vlfGuiAddArrowLeft(int x, int y);

/**
 * Adds an arrow right picture.
 *
 * @param x - x position
 * @param y - y position
 *
 * @returns - a new arrow right picture
*/
VlfShadowedPicture vlfGuiAddArrowRight(int x, int y);

/**
 * Adds a new integer spin
 *
 * @param x - x position
 * @param y - y position
 * @param min - minim value of the spin
 * @param max - max value of the spin
 * @param cur - The initial integer value of the spin
 * @param step - The step in which the value is increased/decreased
 * @param loop - boolean indicating if the spin loops
 * @param speed - The speed of the spin in milisecs
 * @param initstate - the initial state of the spin, one of VLF_SpinState
 * @param prefix - Prefix string, NULL if not needed
 * @param suffix - Suffix string, NULL if not needed
 *
 * @returns - a new VlfSpin object on success, NULL on error
*/
VlfSpin vlfGuiAddIntegerSpinControl(int x, int y, int min, int max, int cur, int step, int loop, int speed, int initstate, char *prefix, char *suffix);

/**
 * Removes a spin item
 *
 * @param spin - the spin to remove
 *
 * @returns - < 0 on error
*/
int  vlfGuiRemoveSpinControl(VlfSpin spin);

/**
 * Sets spin state
 *
 * @param spin - the spin
 * @param state - the state, one of VLF_SpinState
 *
 * @returns - < 0 on error
*/
int vlfGuiSetSpinState(VlfSpin spin, int state);

/**
 * Sets integer minimum and maximum values for an integer spin
 *
 * @param spin - The integer spin
 * @param min - the minimum value
 * @param max - the maximum value
 * 
 * @returns - < 0 on error
*/
int vlfGuiSetIntegerSpinMinMax(VlfSpin spin, int min, int max);

/**
 * Gets the integer value of an integer spin
 *
 * @param spin - the integer spin
 * @param value - pointer to a variable that receives the value
 * 
 * @returns - < 0 on error
*/
int vlfGuiGetIntegerSpinValue(VlfSpin spin, int *value);

/**
 * Sets the integer value of an integer spin
 *
 * @param spin - the integer spin
 * @param value - the integer value to set
 *
 * @returns - < 0 on error
*/
int vlfGuiSetIntegerSpinValue(VlfSpin spin, int value);

/**
 * Adds a new checkbox item
 * By default the checkbox is created without focus and without check.
 * When a checbox has focus, library handles automatically the press of enter button to change check state
 *
 * @param x - x position
 * @param y - y position
 *
 * @returns - a new VlfCheckBox item on sucess, NULL on error
*/
VlfCheckBox vlfGuiAddCheckBox(int x, int y);

/**
 * Removes a checkbox item
 * 
 * @param cb - The checkbox to remove
 *
 * @returns - < 0 on error
*/
int vlfGuiRemoveCheckBox(VlfCheckBox cb);

/**
 * Sets checkbox check state
 *
 * @param cb - The checkbox to check/uncheck
 * @param check - boolean indicating check
 *
 * @returns - < 0 on error
*/
int vlfGuiSetCheckBoxCheck(VlfCheckBox cb, int check);

/**
 * Sets checkbox focus state
 *
 * @param cb - The checkbox 
 * @param focus - boolean indicating focus
 *
 * @returns - < 0 on error
 */
int vlfGuiSetCheckBoxFocus(VlfCheckBox cb, int focus);

/**
 * Boolean function that checks check state of a checkbox
 *
 * @param cb - the checkbox
 *
 * @returns 1 if the checkbox is checked, 0 otherwise
*/
int vlfGuiIsCheckBoxChecked(VlfCheckBox cb);

/**
 * Boolean function that checks focus state of a checkbox
 *
 * @param cb - the checkbox
 *
 * @returns 1 if the checkbox has focus, 0 otherwise
*/
int vlfGuiIsCheckBoxFocused(VlfCheckBox cb);

/**
 * Adds a new inputbox item with default width.
 * By default the newly created inputbox is focused and of normal type.
 * When a inputbox item is focused, library automatically handles press of enter button to show OSK.
 *
 * @param desc - description that will be shown in the OSK dialog.
 * @param x - the x position
 * @param y - the y position
 *
 * @returns - a new VlfInputBox on sucess, NULL on error
*/
VlfInputBox vlfGuiAddInputBox(char *desc, int x, int y);

/**
 * Adds a new inputbox item.
 * By default the newly created inputbox is focused and of normal type.
 * When a inputbox item is focused, library automatically handles press of enter button to show OSK.
 *
 * @param desc - description that will be shown in the OSK dialog.
 * @param x - the x position
 * @param y - the y position
 * @param width - width of the box
 *
 * @returns - a new VlfInputBox on sucess, NULL on error
*/
VlfInputBox vlfGuiAddInputBoxEx(u16 *desc, int x, int y, int width);

/**
 * Removes an inputbox
 *
 * @param ib - the inputbox to remove
 *
 * @returns - < 0 on error
*/
int vlfGuiRemoveInputBox(VlfInputBox ib);

/**
 * Sets inputbox text from an ascii string
 *
 * @param ib - the inputbox
 * @param text - the ascii string
 *
 * @returns - < 0 on error 
*/
int vlfGuiSetInputBoxText(VlfInputBox ib, char *text);

/**
 * Sets inputbox text from an unicode string
 *
 * @param ib - the inputbox
 * @param text - the unicode string
 *
 * @returns - < 0 on error 
 */
int vlfGuiSetInputBoxTextW(VlfInputBox ib, u16 *text);

/**
 * Sets inputbox text from a string with format
 *
 * @param ib - the inputbox
 * @param fmt - the string with format
 *
 * @returns - < 0 on error 
 */
int vlfGuiSetInputBoxTextF(VlfInputBox ib, char *fmt, ...);

/**
 * Gets inputbox text.
 *
 * @param ib - thei inputbox 
 * @param text - output buffer that receives the strig as unicode
 *
 * @returns - < 0 on error
*/
int vlfGuiGetInputBoxText(VlfInputBox ib, u16 *text);

/**
 * Sets inputbox focus state
 *
 * @param ib - the inputbox
 * @param focus - the focus state boolean
 *
 * @returns - < 0 on error
*/
int vlfGuiSetInputBoxFocus(VlfInputBox ib, int focus);

/**
 * Boolean function that checks inputbox focus state
 *
 * @param ib - The inputbox
 *
 * @returns 1 if the inputbox has focus, 0 otherwise
*/
int vlfGuiIsInputBoxFocused(VlfInputBox ib);

/**
 * Sets inputbox type (normal or password).
 *
 * @param ib - the inputbox
 * @param type - the type to set, one of VLF_InputBoxType
 *
 * @returns - < 0 on error
*/
int vlfGuiSetInputBoxType(VlfInputBox ib, int type);

/**
 * Makes a VlfInputBox item to fade.
 *
 * @param ib - The inputbox item to fade.
 * @param mode - Whatever OR combination of VlfFadeModesFlags
 * @param speed - The fade speed, one of VlfFadeSpeed
 * @param direction_out - If both, VLF_FADE_MODE_IN and VLF_FADE_MODE_OUT, were specified, this param indicates
 * wether to start from direction out or in. Otherwise, it is ignored.
 *
 * @returns - < 0 on error.
*/
int vlfGuiSetInputBoxFade(VlfInputBox ib, int mode, int effect, int direction_out);

/**
 * Cancels a VlfInputBox item fade.
 *
 * @param ib - The inputbox item to remove fade.
 *
 * @returns - < 0 on error
*/
int vlfGuiCancelInputBoxFade(VlfInputBox ib);

/**
 * Sets a callback to report the end of a fade.
 *
 * @param ib- The inputbox item fading.
 * @param callback - The callback function that will be called at end of fade.
 * @param param - param that will be passed to the callback function
 * @param delay - The delay between the end of fade and the call to the callback
 * 
 * @returns - < 0 on error
*/
int vlfGuiSetInputBoxFadeFinishCallback(VlfInputBox ib, void (* callback)(void *), void *param, int delay);

/**
 * Adds a new progressbar at default x position.
 *
 * @param y - y position
 *
 * @return - a new VlfProgressBar item on success, NULL on error
*/
VlfProgressBar vlfGuiAddProgressBar(int y);

/**
 * Adds a new progressbar
 *
 * @param x - x position
 * @param y - y position
 *
 * @return - a new VlfProgressBar item on success, NULL on error
*/
VlfProgressBar vlfGuiAddProgressBarEx(int x, int y);

/**
 * Removes a progress bar
 *
 * @param pb - the progress bar to remove.
 *
 * @returns - < 0 on error
*/
int vlfGuiRemoveProgressBar(VlfProgressBar pb);

/**
 *  Sets the progress of a progressbar
 *
 * @param pb - the progress bar
 * @param perc - the percentage progress
 * 
 * @returns - < 0 on error
*/
int vlfGuiProgressBarSetProgress(VlfProgressBar pb, u32 perc);

/**
 * Makes a VlfProgressBar item to fade.
 *
 * @param pb - The progressbar item to fade.
 * @param mode - Whatever OR combination of VlfFadeModesFlags
 * @param speed - The fade speed, one of VlfFadeSpeed
 * @param direction_out - If both, VLF_FADE_MODE_IN and VLF_FADE_MODE_OUT, were specified, this param indicates
 * wether to start from direction out or in. Otherwise, it is ignored.
 *
 * @returns - < 0 on error.
*/
int vlfGuiSetProgressBarFade(VlfProgressBar pb, int mode, int effect, int direction_out);

/**
 * Cancels a VlfProgressBar item fade.
 *
 * @param pb - The progressbar item to remove fade.
 *
 * @returns - < 0 on error
*/
int vlfGuiCancelProgressBarFade(VlfProgressBar pb);

/**
 * Sets a callback to report the end of a fade.
 *
 * @param pb - The progressbar item fading.
 * @param callback - The callback function that will be called at end of fade.
 * @param param - param that will be passed to the callback function
 * @param delay - The delay between the end of fade and the call to the callback
 * 
 * @returns - < 0 on error
*/
int vlfGuiSetProgressBarFadeFinishCallback(VlfProgressBar pb, void (* callback)(void *), void *param, int delay);

/**
 * Adds a new scrollbar. (must be controlled by programmer)
 *
 * @param x - x position
 * @param y - y position
 * @param height - height of scrollbar
 * @param sliderheight - height of slider
 *
 * @returns a new VlfScrollBar item on success, NULL on error
*/
VlfScrollBar vlfGuiAddScrollBar(int x, int y, int height, int sliderheight);

/**
 * Removes a scrollbar itemm.
 *
 * @param sb - the scrollbar to remove
 *
 * @returns - < 0 on error
*/
int vlfGuiRemoveScrollBar(VlfScrollBar sb);

/**
 * Moves the slider of a scrollbar
 *
 * @param y - the new position of the slider
 *
 * @returns - < 0 on error
*/
int vlfGuiMoveScrollBarSlider(VlfScrollBar sb, int y);

/**
 * Sets the height of the slider of a scrollbar
 *
 * @param sb - the scrollbar
 * @param height - the new height for the slider
 *
 * @returns - < 0 on error.
*/
int vlfGuiSetScrollBarSliderHeight(VlfScrollBar sb, int height);

/**
 * Sets the visibility of the items in a rectangle
 *
 * @param x - x position of rectangle
 * @param y - y position of rectangle
 * @param w - width of rectangle
 * @param h - height of rectangle
 * @param visible . boolean indicating visibility
 *
 * @returns - < 0 on error 
*/
int vlfGuiSetRectangleVisibility(int x, int y, int w, int h, int visible);

/**
 * Saves the visibility context in a rectangle area
 *
 * @param x - x position of rectangle
 * @param y - y position of rectangle
 * @param w - width of rectangle
 * @param h - height of rectangle
 *
 * @returns - the visibility context. Use vlfGuiRestoreVisibilityContext to restore it, and vlfGuiFreeVisibilityContext to deallocate it.
*/
void *vlfGuiSaveRectangleVisibilityContext(int x, int y, int w, int h);

/**
 * Restores a visibility context.
 *
 * @param ctx - the visibility context
*/
void vlfGuiRestoreVisibilityContext(void *ctx);

/**
 * Deallocates memory used by a visibility context
 *
 * @param ctx - the visibility context
*/
void vlfGuiFreeVisibilityContext(void *ctx);

/**
 * Makes all items within a rectangle to fade
 *
 * @param x - x position of rectangle
 * @param y - y position of rectangle
 * @param w - width of rectangle
 * @param h - height of rectangle
 * @param mode - Whatever OR combination of VlfFadeModesFlags
 * @param speed - The fade speed, one of VlfFadeSpeed
 * @param direction_out - If both, VLF_FADE_MODE_IN and VLF_FADE_MODE_OUT, were specified, this param indicates
 * wether to start from direction out or in. Otherwise, it is ignored.
 *
 * @returns - < 0 on error.
*/
int vlfGuiSetRectangleFade(int x, int y, int w, int h, int mode, int effect, int direction_out, void (* callback)(void *), void *param, int delay);

/**
 * Makes all items within a rectangle to blink
 *
 * @param x - x position of rectangle
 * @param y - y position of rectangle
 * @param w - width of rectangle
 * @param h - height of rectangle
 * @param nshow - the numbers of frames the items will be shown
 * @param nhide - the numbers of frames the items will be hidden
 *
 * @returns - < 0 on error.
 *
 * @Note: currently not all kind of items can blink. This will be fixed in future.
*/
int vlfGuiSetRectangleBlinking(int x, int y, int w, int h, u32 nshow, u32 nhide);

/**
 * Synchronizes the blinking of one item with the one of a second one
 *
 * @param dst - the destiny item
 * @param dst_type - the type of the destiny object, one of VlfObjects
 * @param src - the source item
 * @param src_type - the type of source object, one of VlfObjects
 *
 * @returns - < 0 on error
*/
int vlfGuiSetSynchronizedBlinking(void *dst, int dst_type, void *src, int src_type);

/**
 * Shows a message dialog.
 *
 * @param msg - the message dialog
 * @param flags - a combination of VLF_MDType, VLF_MD_Buttons and VLF_MD_InitalCursor
 *
 * @returns - one of VLF_MD_ButtonRes
*/
int vlfGuiMessageDialog(char *msg, u32 flags);

/**
 * Shows an error dialog.
 *
 * @param error - the system error code
 *
 * @returns - one of VLF_MD_ButtonRes
*/
int vlfGuiErrorDialog(int error);

/**
 * Shows a net config dialog.
 * Network libraries must be loaded before.
 *
 * @returns 0 if the connection was done.
*/
int vlfGuiNetConfDialog();

/**
 * Shows an OSK dialog.
 *
 * @param intext - the initial text of the OSK, NULL if no required
 * @param desc - the description that will be shown in the OSK, NULL if no required
 * @param outtext - the output text
 *
 * @returns 0 if the text was entered.
*/
int vlfGuiOSKDialog(u16 *intext, u16 *desc, u16 *outtext);

/**
 * Shows one or two buttons with enter/cancel icons at bottom of screen.
 * Button presses are handled by the library.
 *
 * @param button1 - a button code, one of VLF_DialogItem. < 0 if no required
 * @param button2 - a button code, one of VLF_DialogItem- < 0 if no required
 * @param automatic - if automatic is 0, button1 will be the left button and button2 will be right button.
 * Otherwise, the buttons are rearranged automatically acording to current button configuration.
 * @param enter_is_left - if automatic is 0, this button indicates which button is assigned enter.
 * @param distance - the distance between the buttons, use VLF_DEFAULT for a default distance
 * @param handler - the function that will be called when enter/cancel are pressed. The function shall return VLF_REMOVE_HANDLERS if it
 * wants to cancel further events, VLF_REMOVE_OBJECTS to remove bottom dialog, or both to fully remove the bottom dialog.
 *
 * @returns - < 0 on error.
*/
int vlfGuiBottomDialog(int button1, int button2, int automatic, int enter_is_left, int distance, int (* handler)(int enter));

/**
 * Shows one or two buttons with enter/cancel icons at bottom of screen.
 * Button presses are handled by the library.
 *
 * @param button1 - button string. NULL if no required
 * @param button2 - button string. NULL if no required
 * @param automatic - if automatic is 0, button1 will be the left button and button2 will be right button.
 * Otherwise, the buttons are rearranged automatically acording to current button configuration.
 * @param enter_is_left - if automatic is 0, this button indicates which button is assigned enter.
 * @param distance - the distance between the buttons, use VLF_DEFAULT for a default distance
 * @param handler - the function that will be called when enter/cancel are pressed, enter param indicates if enter was pressed, otherwise cancel was pressed.
 * The function shall return VLF_REMOVE_HANDLERS if it wants to cancel further events, VLF_REMOVE_OBJECTS to remove buttons,
 * or both to fully remove the bottom dialog.
 *
 * @returns - < 0 on error.
 */
int vlfGuiCustomBottomDialog(char *button1, char *button2, int automatic, int enter_is_left, int distance, int (* handler)(int enter));

/**
 * Cancels (remove) the bottom dialog
*/
void vlfGuiCancelBottomDialog();

/**
 * Creates a central menu.
 * Button presses of menu are automatically handled by the library
 *
 * @param noptions - number of options of the menu
 * @param items - an array of noptions strings
 * @param defaultsel- the index of the item with the initial selection
 * @param handler - the function that will be called on enter button is pressed. sel param is the index of the selected item.
 * The function shall return VLF_REMOVE_HANDLERS if it wants to cancel further events, VLF_REMOVE_OBJECTS to remove items,
 * or both to fully remove the central menu.
 * @param dispx - displacement relative to the default centered position (x)
 * @param dispy - displacement relative to the default centered position (y)
 *
 * @returns - < 0 on error
*/
int vlfGuiCentralMenu(int noptions, char **items, int defaultsel, int (* handler)(int sel), int dispx, int dispy);

/**
 * Cancels (remove) the central menu
*/
void vlfGuiCancelCentralMenu();

/**
 * Returns the current selected item of the central menu
 *
 * @returns - the selected item
*/
int vlfGuiCentralMenuSelection();

/**
 * Creates a lateral menu with current system color.
 * Button presses of menu are automatically handled by the library
 *
 * @param noptions - number of options of the menu
 * @param items - an array of noptions strings
 * @param defaultsel- the index of the item with the initial selection
 * @param handler - the function that will be called on enter button is pressed. sel param is the index of the selected item.
 * The function shall return VLF_REMOVE_HANDLERS if it wants to cancel further events, VLF_REMOVE_OBJECTS to remove menu,
 * or both to fully remove the lateral menu.
 * @param y - y position of first item
 *
 * @returns - < 0 on error
*/
int vlfGuiLateralMenu(int noptions, char **items, int defaultsel, int (* handler)(int sel), int y);

/**
 * Creates a lateral menu with desired color.
 * Button presses of menu are automatically handled by the library
 *
 * @param noptions - number of options of the menu
 * @param items - an array of noptions strings
 * @param defaultsel- the index of the item with the initial selection
 * @param handler - the function that will be called on enter button is pressed. sel param is the index of the selected item.
 * The function shall return VLF_REMOVE_HANDLERS if it wants to cancel further events, VLF_REMOVE_OBJECTS to remove menu,
 * or both to fully remove the lateral menu.
 * @param y - y position of first item
 * @param color - the color of the lateral menu
 *
 * @returns - < 0 on error
*/
int vlfGuiLateralMenuEx(int noptions, char **items, int defaultsel, int (* handler)(int sel), int y, u32 color);

/**
 * Cancels (remove) the lateral menu
 */
void vlfGuiCancelLateralMenu();

/**
 * Returns the current selected item of the lateral menu
 *
 * @returns - the selected item
 */
int vlfGuiLateralMenuSelection();

/**
 * Creates a control for previous page handling.
 *
 * @param handler - the function that will be called when the previous button (left) is pressed. page parameter holds the page number.
 * The function shall return VLF_REMOVE_HANDLERS if it wants to cancel further events, VLF_REMOVE_OBJECTS to remove control pics and text,
 * or both to fully remove the control.
 *
 * @returns - < 0 on error
*/
int vlfGuiPreviousPageControl(int (* handler)(int page));

/**
 * Creates a control for next page handling.
 *
 * @param handler - the function that will be called when the next button (right) is pressed. page parameter holds the page number.
 * The function shall return VLF_REMOVE_HANDLERS if it wants to cancel further events, VLF_REMOVE_OBJECTS to remove control pics and text,
 * or both to fully remove the control.
 *
 * @returns - < 0 on error
 */
int vlfGuiNextPageControl(int (* handler)(int page));

/**
 * Cancels (remove) previous page control
*/
void vlfGuiCancelPreviousPageControl();

/**
 * Cancels (remove) next page control
 */
void vlfGuiCancelNextPageControl();

/**
 * Enable or disable page control.(it does not add/remove them).
 *
 * @param enable - enable or disable
*/
void vlfGuiSetPageControlEnable(int enable);

/**
 * Adds a handler for an event.
 *
 * @param buttons - the button event, 0 if no event (will be called every frame)
 * @param wait - wait for the handler to be added. Use 0 for no wait, and -1 to make this handler impossible to delay.
 * @param func - the function that will be called when the event happens.
 * @param param - custom param for the handler function. The function must return one or a combination of following values:
 *
 * VLF_EV_RET_NOTHING: normal return, it does nothing.
 * VLF_EV_RET_REMOVE_EVENT: it removes the event (button press), so other handler doesn't receive it.
 * VLF_EV_RET_REMOVE_HANDLERS: it removes this handler so that it won't receive more event notifications.
 * VLF_EV_RET_DELAY: the  event is delayed for specified number of miliseconds. VLF_EV_RET_DELAY | (X << 16), where 0 <= X <= 32727
 * VLF_EV_RET_DELAY_FRAME: the  event is delayed for specified number of frames. VLF_EV_RET_DELAY_FRAME | (X << 16), where 0 <= X <= 4095
 * VLF_EV_RET_REFRESH_ON_DELAY: the event is refreshed after a delay, so that if button/s are kept pressed until after the event, it will receive it as a new event 
 *
 * @returns - < 0 on error
*/
int vlfGuiAddEventHandler(int buttons, int wait, int (* func)(void *), void *param);

/**
 * Adds a handler for an event.
 * Unlike the previous function, this one adds the handler to the head instead of the tail.
 *
 * @param buttons - the button event, 0 if no event (will be called every frame)
 * @param wait - wait for the handler to be added. Use 0 for no wait, and -1 to make this handler impossible to delay.
 * @param func - the function that will be called when the event happens.
 * @param param - custom param for the handler function. The function must return one or a combination of following values:
 *
 * VLF_EV_RET_NOTHING: normal return, it does nothing.
 * VLF_EV_RET_REMOVE_EVENT: it removes the event (button press), so other handler doesn't receive it.
 * VLF_EV_RET_REMOVE_HANDLERS: it removes this handler so that it won't receive more event notifications.
 * VLF_EV_RET_DELAY: the  event is delayed for specified number of miliseconds. VLF_EV_RET_DELAY | (X << 16), where 0 <= X <= 32727
 * VLF_EV_RET_DELAY_FRAME: the  event is delayed for specified number of frames. VLF_EV_RET_DELAY_FRAME | (X << 16), where 0 <= X <= 4095
 * VLF_EV_RET_REFRESH_ON_DELAY: the event is refreshed after a delay, so that if button/s are kept pressed until after the event, it will receive it as a new event 
 *
 * @returns - < 0 on error
*/
int vlfGuiAddEventHandlerHead(int buttons, int wait, int (* func)(void *), void *param);

/**
 * Adds a handler for a negative event.
 *
 * @param buttons - the negative button event. For Example if you pass PSP_CTRL_CROSS, it will mean NOT PSP_CTRL_CROSS pressed.
 * @param wait - wait for the handler to be added. Use 0 for no wait, and -1 to make this handler impossible to delay.
 * @param func - the function that will be called when the event happens.
 * @param param - custom param for the handler function. The function must return one or a combination of following values:
 *
 * VLF_EV_RET_NOTHING: normal return, it does nothing.
 * VLF_EV_RET_REMOVE_EVENT: it removes the event (button press), so other handler doesn't receive it.
 * VLF_EV_RET_REMOVE_HANDLERS: it removes this handler so that it won't receive more event notifications.
 * VLF_EV_RET_DELAY: the  event is delayed for specified number of miliseconds. VLF_EV_RET_DELAY | (X << 16), where 0 <= X <= 32727
 * VLF_EV_RET_DELAY_FRAME: the  event is delayed for specified number of frames. VLF_EV_RET_DELAY_FRAME | (X << 16), where 0 <= X <= 4095
 * VLF_EV_RET_REFRESH_ON_DELAY: the event is refreshed after a delay, so that if button/s are kept pressed until after the event, it will receive it as a new event 
 *
 * @returns - < 0 on error
 */
int vlfGuiAddNegativeEventHandler(int buttons, int wait, int (* func)(void *), void *param);

/**
 * Adds a handler for a negative event.
 * Unlike the previous function, this one adds the handler to the head instead of the tail.
 *
 * @param buttons - the negative button event. For Example if you pass PSP_CTRL_CROSS, it will mean NOT PSP_CTRL_CROSS pressed.
 * @param wait - wait for the handler to be added. Use 0 for no wait, and -1 to make this handler impossible to delay.
 * @param func - the function that will be called when the event happens.
 * @param param - custom param for the handler function. The function must return one or a combination of following values:
 *
 * VLF_EV_RET_NOTHING: normal return, it does nothing.
 * VLF_EV_RET_REMOVE_EVENT: it removes the event (button press), so other handler doesn't receive it.
 * VLF_EV_RET_REMOVE_HANDLERS: it removes this handler so that it won't receive more event notifications.
 * VLF_EV_RET_DELAY: the  event is delayed for specified number of miliseconds. VLF_EV_RET_DELAY | (X << 16), where 0 <= X <= 32727
 * VLF_EV_RET_DELAY_FRAME: the  event is delayed for specified number of frames. VLF_EV_RET_DELAY_FRAME | (X << 16), where 0 <= X <= 4095
 * VLF_EV_RET_REFRESH_ON_DELAY: the event is refreshed after a delay, so that if button/s are kept pressed until after the event, it will receive it as a new event 
 *
 * @returns - < 0 on error
 */
int vlfGuiAddNegativeEventHandlerHead(int buttons, int wait, int (* func)(void *), void *param);

/**
 * Removes an event handler. Use vlfGuiRemoveHandlerEx if you have used the same handler with different params.
 *
 * @param func - the handler function
 *
 * @returns - < 0 on error
*/
int vlfGuiRemoveEventHandler(int (* func)(void *));

/**
 * Removes an event hnadler
 *
 * @param func - the handler function
 * @param param - the parameter that was passed to one of the vlfGuiAddEventHandler functions.
 *
 * @returns - < 0 on error
*/
int vlfGuiRemoveEventHandlerEx(int (* func)(void *), void *param);

/**
 * Boolean function that checks if a handler is registered
 *
 * @param func - the handler to be checked.
 *
 * @return - < 0 on error
*/
int vlfGuiIsEventRegistered(int (* func)(void *));

/**
 * Delays an event handler. Use vlfGuiSetEventDelayEx if you have used the same handler with different params.
 *
 * @param func - the handler function
 * @param delay - the delay in milisecs
 *
 * @return - < 0 on error
*/
int vlfGuiSetEventDelay(int (* func)(void *), u32 delay);

/**
 * Delays an event handler. 
 *
 * @param func - the handler function
 * @param param - the parameter that was passed to one of the vlfGuiAddEventHandler functions.
 * @param delay - the delay in milisecs
 *
 * @return - < 0 on error
 */
int vlfGuiSetEventDelayEx(int (* func)(void *), void * param, u32 delay);

/**
 * Delays all events that can be delayed
 *
 * @param delay - the delay in milisecs
 *
 * @returns - < 0 on error
*/
int vlfGuiDelayAllEvents(u32 delay);



#endif

