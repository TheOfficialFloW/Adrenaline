- 6.61 Adrenaline -
  A software that transforms your PS Vita into a two-in-one device
  
- Donation -
  If you wish to donate me some money, I'd really appreciate. But just don't write thanks for PSP piracy xD
  Link: goo.gl/uAIPIY
  
- What is Adrenaline? -
  Adrenaline is a software that modifies the official PSP Emulator using taiHEN CFW framework
  to make it run a PSP 6.61 custom firmware. Thanks to the power of taiHEN, Adrenaline can
  inject custom code into the IPL which allows unsigned code to run at boottime.
  
- IMPORTANT NOTE -
  For 6.61 Adrenaline-2 or higher, you must put Adrenaline files to 'ux0:adrenaline' instead of 'ux0:pspemu/adrenaline'.
  
- Changelog v3.1 -
  - Fixed 'Please wait...' bug in some games.
  - Fixed sound problems after exiting a PS1 game.
  - Fixed problem where mounting ur0: as USB device would cause problems in livearea.
  
- Changelog v3 fix -
  - Fixed bug where 'Cannot find application' would show up instead of returning to livearea.
  - Fixed bug where you couldn't access the Adrenaline Menu after enabling/disabling wifi.
  
- Changelog v3 -
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
  
- Changelog v2 -
  - Added 64MB RAM support for homebrews.
  - Added ability to use the 'ur0:' partition as Memory Stick.
  - Added Advanced AA filter (disable 'Smooth Graphics' for that filter).
  - Added ability to change smooth graphics (GPU internal bilinear filter).
  - Added ability to change screen size (2.0x, 1.75x, 1.5x, 1.25x, 1.0x).
  - Fixed sound issue in 'MotorStorm' and some other games.
  - Fixed bug where 'ms0:/MUSIC' and 'ms0:/PICTURE' were not found in XMB.
  - Fixed bug where changing options in the official settings menu didn't have any effect.
  
- How to install Adrenaline -
  * Install any PSP game/demo onto your PS Vita (compatiblity for PS1 games as bubble will be added in the future).
  * Shutdown your PS Vita and visit http://beta.henkaku.xyz/.
  * Launch molecularShell, press START and set 'Enable unsafe homebrews' to 'On'.
  * Reboot your PS Vita and then install VitaShell from molecularShell (https://github.com/TheOfficialFloW/VitaShell/releases).
  * In VitaShell open FTP by pressing SELECT and copy the 'adrenaline' folder from the archive and paste it at 'ux0:'.
  * Download the 6.61 EBOOT.PBP (for 1000/2000/3000) from the Sony server (google it)
    and paste it at 'ux0:adrenaline/661.PBP'.
  * Copy 'ux0:tai/config.txt' to your PC and add the following lines:
        *KERNEL
        ux0:adrenaline/adrenaline.skprx

        *TITLEID
        ux0:adrenaline/adrenaline.suprx

    Example of config.txt:
        # You must reboot for changes to take place.
        *KERNEL
        ux0:adrenaline/adrenaline.skprx
        *main
        # main is a special titleid for SceShell
        ux0:app/MLCL00001/henkaku.suprx
        *NPXS10015
        # this is for modifying the version string
        ux0:app/MLCL00001/henkaku.suprx
        *TITLEID
        ux0:adrenaline/adrenaline.suprx

    Note that you replace TITLEID by the titleid of your PSP game (the name of the folder where EBOOT.PBP is).
  * Reboot your device and revisit the page I have mentioned above.
  * Launch the game for which you have added the plugin's line to config.txt.
  * Follow the instructions on screen.
  * Enjoy
  
- About right analog stick in games -
  The right analog patch for GTA LCS/GTA VCS will be downloaded from here: https://github.com/TheOfficialFloW/GTA_Remastered
  Don't think that any other games will now have right analog stick patch too ;)