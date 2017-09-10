6.61 Adrenaline
================================================================================

A software that transforms your PS Vita into a two-in-one device

Donation
--------
If you wish to donate me some money, I'd really appreciate. But just don't write thanks for PSP piracy xD
[Donation link](http://goo.gl/uAIPIY)

What is Adrenaline?
-------------------
Adrenaline is a software that modifies the official PSP Emulator using [taiHEN CFW framework](https://github.com/yifanlu/taiHEN)
to make it run a PSP 6.61 custom firmware. Thanks to the power of taiHEN, Adrenaline can
inject custom code into the IPL which allows unsigned code to run at boottime.

How to install
--------------
1. Download [Adrenaline.vpk](https://github.com/TheOfficialFloW/Adrenaline/releases) and install it using [VitaShell](https://github.com/TheOfficialFloW/VitaShell/releases).
2. Launch Adrenaline and press X to download the 6.61 firmware. After finishing it will automatically terminate.
3. Relaunch Adrenaline, this time it will go into pspemu mode. Follow the instructions on screen.

**Note #1:** Adrenaline has been redesigned in `6.61 Adrenaline-6`, so you'd need to launch Adrenaline twice everytime you reboot your device. To get rid of that, simply write this line to `*KERNEL`

```text
*KERNEL
ux0:app/PSPEMUCFW/sce_module/adrenaline_kernel.skprx
```

**Note #2:** If you have been using a previous Adrenaline, please uninstall it before using `6.61 Adrenaline-6` or higher. Just remove the directory `ux0:adrenaline` to avoid interference.

Network update
--------------
For 6.61 Adrenaline-4 or higher, you can use the 'System Update' in the XMB to update Adrenaline to the latest version.
1. Choose 'System Update' in XMB, then click 'Update via Internet'.
2. If a connection name does already exist, skip to 5).
3. Otherwise select '[New Connection]', then 'Enter Manually', press RIGHT three times until you need to enter a connection name.
4. Type any connection name and press RIGHT two times. You'll now see the message 'Press the X/O button to save settings'.
5. Press X/O on any connection name.
6. Follow the instructions to update Adrenaline.

Changelog
---------

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


How to build
------------
```
git clone https://github.com/frangarcj/vita2dlib.git
cd vita2dlib/libvita2d/
git checkout -b 0001 origin/fbo
make && make install
cd ../sample/
make
cp libvitashaders.a /usr/local/vitasdk/arm-vita-eabi/lib/
```
```
git clone https://github.com/TheOfficialFloW/Adrenaline.git
mkdir Adrenaline/build && cd Adrenaline/build
cmake .. && make
```