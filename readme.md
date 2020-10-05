6.61 Adrenaline
================================================================================

A software that transforms your PS Vita into a two-in-one device

What is Adrenaline?
-------------------
Adrenaline is a software that modifies the official PSP Emulator using [taiHEN CFW framework](https://github.com/yifanlu/taiHEN)
to make it run a PSP 6.61 custom firmware. Thanks to the power of taiHEN, Adrenaline can
inject custom code into the IPL which allows unsigned code to run at boottime.

How to update
-------------
If you have already been using Adrenaline, simply open Adrenaline.vpk as zip file and copy all modules from sce_module to ux0:app/PSPEMUCFW/sce_module.

How to install
--------------
Please only do this approach for a fresh installation, otherwise please refer to the guide above.

1. Remove the Adrenaline bubble and the `ux0:app/PSPEMUCFW/sce_module/adrenaline_kernel.skprx` path from the taiHEN config.txt and finally reboot your device.
2. Download [Adrenaline.vpk](https://github.com/TheOfficialFloW/Adrenaline/releases) and install it using [VitaShell](https://github.com/TheOfficialFloW/VitaShell/releases).
3. Launch Adrenaline and press ❌ to download the 6.61 firmware. After finishing it will automatically terminate.
4. Relaunch Adrenaline, this time it will go into pspemu mode. Follow the instructions on screen.

Getting rid of double launch bug
--------------------------------
Adrenaline has been redesigned in `6.61 Adrenaline-6`, so you'd need to launch Adrenaline twice everytime you reboot your device. To get rid of that, simply write this line to `*KERNEL`

```text
*KERNEL
ux0:app/PSPEMUCFW/sce_module/adrenaline_kernel.skprx
```

Changelog
---------

### Changelog v7
- Fixed issue where plugins were loaded in recovery mode.
- Moved native display buffer to a different location, so GePatch works for some more games. Please note that only GePatch v0.18 or above will only work, older versions will cause a black screen. If you're using GTANativeRes, please download the latest binary as well.

### Changelog v6.9
- Added support for native resolution patches.

### Changelog v6.8
- Added sharp bilinear without scanlines filter. Thanks to rsn8887.
- Fixed PS1 slowdowns. Thanks to rsn8887.
- Fixed compatiblity with h-encore 2.0.
- Fixed compatiblity with udcd_uvc.skprx plugin.

### Changelog v6.7
- Added support for PS1 multiplayer on PS Vita using an upcoming DS3/DS4 plugin.

### Changelog v6.6
- Fixed bug from previous update that caused black screen in other DJ max games.
- Inferno driver was not included correcty, now it is.
- Tekken 6 can now be played with any CPU speed.

### Changelog v6.5
- Added updated inferno driver by codestation which improves performance of CSO reading.
- Added option to choose USB device.
- Added xmc0: option.
- Fixed stupid mistake that made DJ max portable 1 crash. **Install it again, because this has been added post-release**
- Fixed little bug in msfs.
- Removed savestate version restriction, old savestates will not disappear anymore.

### Changelog v6.4
- Added ability to fast forward in PS1 games by pressing L+SELECT.
- Fixed double launch bug when using without enso. You need to add the kernel module to config to do so.

### Changelog v6.3
- Added support for 3.68.
- Added ability to freely adjust the screen size to your desire in both psp and ps1 modes.
- Removed 'Screen Mode' and 'Screen Size' from menu.
- Improved Adrenaline Menu.

### Changelog v6.2
- Added f.lux by Rinnegatamante.
- Fixed PS1 framerate when using custom screen.
- Fixed bug where exiting a game with 'High memory layout' would crash.
- Fixed compatiblity with 'Kingdom Hearts: Birth by Sleep' english patch, again.

### Changelog v6.1
- Added support for 3.65/3.67.

### Changelog v6 fix
- Fixed bug where CSO games freezed up the system.

### Changelog v6
- Redesigned boot process. Adrenaline does now not require any basegame or activation anymore.
- Added 'uma0:' to 'Memory Stick Location'.
- Added support for longer ISO filenames.
- Fixed Memory Stick free space integer overflow in games like Outrun.
- Fixed bug where ISO games did not show up when the folder 'PSP/GAME' was missing.
- Improved core and fixed some small bugs.

### Changelog v5.1
- Added ability to skip adrenaline boot logo.
- Added message for original filter.
- Fixed bug where payloadex was not updated and caused some bugs.
- Fixed '20000006' bug on PS TV. Network update will work on PS TV in the future.
- Changed CPU clock back to 333 MHz.

### Changelog v5
- Added 'Hide DLC's in game menu' functionality.
- Readded 'Original' graphics filtering, since PS1 games have got framedrops using custom filters.
- Fixed corrupted icons bug that was introduced in the previous update.
- Fixed bug where the framebuffer was corrupted after loading savestate.
- Adrenaline icon is now hidden in game menu.

### Changelog v4.2
- Added support for ISO sorting using 'Game Categories Lite' plugin.
- Fixed compatiblity with 'Kingdom Hearts: Birth by Sleep' english patch.

### Changelog v4.1
- Fixed bug where holding R trigger while launching Adrenaline didn't open the recovery menu.
- Fixed msfs truncation bug that caused savedata corruption for Little Big Planet and maybe other games.
- Fixed wrong scale of PS1 games on PS TV.

### Changelog v4
- Added custom graphics filtering support for PS1 games.
- Added screen mode adjustment for PS1 games. If you're using this feature on a PS Vita, select 'Original' screen mode in the
offical settings, then apply the custom screen mode. On the other hand, if you want to the screen mode of the official settings,
	select 'Original' screen mode in Adrenaline settings. On a PS TV this will finally allow you to play your games in fullscreen.
- Added screenshot support in PS1 games.
- Added network update feature for future updates.
- Fixed a bug in msfs driver that caused weird behaviour in XMB after resuming from standby.
- Removed 'Official' graphics filtering in order to support the features mentioned above.

### Changelog v3.1
- Added support for cwcheat in PS1 games.
- Fixed sound problems after exiting a PS1 game.
- Fixed 'Please wait...' bug in some games.
- Fixed problem where mounting ur0: as USB device would cause problems in livearea.

### Changelog v3 fix
- Fixed bug where 'Cannot find application' would show up instead of returning to livearea.
- Fixed bug where you couldn't access the Adrenaline Menu after enabling/disabling wifi.

### Changelog v3
- Added ability to launch PS1 games from XMB and play them with full sound.
- Added ability to save and load states using the 'States' tab in the Adrenaline menu.
- Added possiblity to connect USB in XMB and added 'Toggle USB' option to recovery menu.
- Added ability to return to livearea by double tapping the PS button.
- Added Adrenaline startup image designed by Freakler.
- Added option to force high memory layout. For 'The Elder Scrolls Travels: Oblivion' Demo.
- Added option to execute 'BOOT.BIN' in UMD/ISO. For 'Saints Row: Undercover' Demo.
- Added correct enter and cancel buttons assignment.
- Fixed volatile memory allocation bug that made 'Star Wars: The Force Unleashed',
'Tony Hawk's Project 8' and maybe more games crashing.
- Fixed bug that was introduced in v2 which caused some games to crashed at PMF sequences.
- Fixed NoDrm engine bug where fan translated games couldn't load PGD decrypted files.
- Fixed msfs directory filter bug that caused some games not to recognize savedatas.
- Fixed compatiblity of base games, any game should now be able to use Adrenaline to the fullest.

### Changelog v2
- Added 64MB RAM support for homebrews.
- Added ability to use the 'ur0:' partition as Memory Stick.
- Added Advanced AA filter (disable 'Smooth Graphics' for that filter).
- Added ability to change smooth graphics (GPU internal bilinear filter).
- Added ability to change screen size (2.0x, 1.75x, 1.5x, 1.25x, 1.0x).
- Fixed sound issue in 'MotorStorm' and some other games.
- Fixed bug where 'ms0:/MUSIC' and 'ms0:/PICTURE' were not found in XMB.
- Fixed bug where changing options in the official settings menu didn't have any effect.

# Dependencies
- [vitasdk](https://vitasdk.org/)
- [vita2dlib-fbo](https://github.com/frangarcj/vita2dlib/tree/fbo)
- [vita-shader-collection](https://github.com/frangarcj/vita-shader-collection)
- [pspdev](https://github.com/pspdev/pspdev)
- [psp-packer](https://bitbucket.org/DaveeFTW/psp-packer)

# Building
A set of build scripts in the `buildscripts` folder is provided for convenience.
On a *nix systems with `bash` installed, Adrenaline can be built by running:

```
./buildscripts/build.sh
```

Adrenaline can also be built using an isolated containerized environment using [Docker](https://www.docker.com/).
Install Docker for your operating system and run the following script:

```
./buildscripts/build_docker.sh
```

This will create the container image with all the required dependencies and build Adrenaline afterwards.
Docker caches images, so running this script again to build Adrenaline will _not_ require a full rebuild 
of the container image.

If a full rebuild is desired, however (e.g. to update the `vitasdk` version), you can run:

```
./buildscripts/build_docker.sh --no-cache
```

The final product from building is:
* Uncompressed modules at `bubble/pkg/sce_modules`.
* The `eboot.bin` file at `bubble/build/eboot.bin`.
* The packaged application (includes all above) at `bubble/build/Adrenaline.vpk`.
