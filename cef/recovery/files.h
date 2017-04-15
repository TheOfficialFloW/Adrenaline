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

#define N_FILES (sizeof(files) / sizeof(char **))

char *files[] = {
	"/codepage/cptbl.dat",
	"/data/cert/CA_LIST.cer",
	"/dic/apotp.dic",
	"/dic/atokp.dic",
	"/dic/aux0.dic",
	"/dic/aux1.dic",
	"/dic/aux2.dic",
	"/dic/aux3.dic",
	"/font/arib.pgf",
	"/font/gb3s1518.bwfon",
	"/font/imagefont.bin",
	"/font/jpn0.pgf",
	"/font/kr0.pgf",
	"/font/ltn0.pgf",
	"/font/ltn1.pgf",
	"/font/ltn2.pgf",
	"/font/ltn3.pgf",
	"/font/ltn4.pgf",
	"/font/ltn5.pgf",
	"/font/ltn6.pgf",
	"/font/ltn7.pgf",
	"/font/ltn8.pgf",
	"/font/ltn9.pgf",
	"/font/ltn10.pgf",
	"/font/ltn11.pgf",
	"/font/ltn12.pgf",
	"/font/ltn13.pgf",
	"/font/ltn14.pgf",
	"/font/ltn15.pgf",
	"/kd/amctrl.prx",
	// "/kd/ata.prx",				// Only for PSP
	// "/kd/audio.prx",				// kermit_audio.prx
	"/kd/audiocodec_260.prx",
	// "/kd/avcodec.prx",			// Not compatible on PS Vita
	"/kd/cert_loader.prx",
	"/kd/chkreg.prx",
	"/kd/chnnlsv.prx",
	// "/kd/clockgen.prx",			// Only for PSP
	// "/kd/codec_01g.prx",			// Only for PSP
	"/kd/codepage.prx",
	// "/kd/ctrl.prx",				// kermit_ctrl.prx
	// "/kd/display_01g.prx",		// kermit_display.prx
	// "/kd/dmacman.prx",			// pre-loaded
	// "/kd/exceptionman.prx",		// pre-loaded
	// "/kd/fatms.prx",				// Only for PSP
	"/kd/g729.prx",
	// "/kd/ge.prx",				// pre-loaded
	// "/kd/hpremote_01g.prx",		// kermit_hpremote.prx
	"/kd/http_storage.prx",
	// "/kd/idstorage.prx",			// kermit_idstorage.prx
	"/kd/ifhandle.prx",
	"/kd/impose_01g.prx",
	// "/kd/init.prx",				// add this when adrenaline has got flashfs
	// "/kd/interruptman.prx",		// pre-loaded
	// "/kd/iofilemgr.prx",			// pre-loaded
	"/kd/iofilemgr_dnas.prx",
	// "/kd/irda.prx",				// Only for PSP
	"/kd/isofs.prx",
	// "/kd/led.prx",				// Only for PSP
	// "/kd/lfatfs.prx",			// Only for PSP
	// "/kd/lflash_fatfmt.prx",		// Only for PSP
	"/kd/libaac.prx",
	"/kd/libasfparser.prx",
	"/kd/libatrac3plus.prx",
	"/kd/libaudiocodec2.prx",
	"/kd/libdnas.prx",
	"/kd/libdnas_core.prx",
	"/kd/libgameupdate.prx",
	"/kd/libhttp.prx",
	"/kd/libmp3.prx",
	"/kd/libmp4.prx",
	"/kd/libparse_http.prx",
	"/kd/libparse_uri.prx",
	"/kd/libssl.prx",
	"/kd/libupdown.prx",
	// "/kd/loadcore.prx",			// pre-loaded
	// "/kd/loadexec_01g.prx",		// add this when adrenaline has got flashfs
	// "/kd/lowio.prx",				// kermit_lowio.prx
	"/kd/mcctrl.prx",
	"/kd/mediaman.prx",
	"/kd/mediasync.prx",
	"/kd/memab.prx",
	// "/kd/memlmd_01g.prx",		// use PS Vita's
	// "/kd/mesg_led_01g.prx",		// use PS Vita's
	// "/kd/me_wrapper.prx",		// kermit_me_wrapper.prx
	// "/kd/mgr.prx",				// Only for PSP
	"/kd/mgvideo.prx",
	"/kd/mlnbridge.prx",
	"/kd/mlnbridge_msapp.prx",
	// "/kd/modulemgr.prx",			// pre-loaded
	"/kd/mp4msv.prx",
	"/kd/mpeg.prx",
	// "/kd/mpegbase_260.prx",		// Not compatible on PS Vita. Returns error in GTA
	// "/kd/mpeg_vsh.prx",			// Not compatible on PS Vita
	// "/kd/msaudio.prx",			// Only for PSP
	// "/kd/msstor.prx",			// Only for PSP
	"/kd/np.prx",
	"/kd/np9660.prx",
	"/kd/npdrm.prx",
	"/kd/np_auth.prx",
	"/kd/np_campaign.prx",
	"/kd/np_commerce2.prx",
	"/kd/np_commerce2_regcam.prx",
	"/kd/np_commerce2_store.prx",
	"/kd/np_core.prx",
	"/kd/np_inst.prx",
	"/kd/np_matching2.prx",
	"/kd/np_service.prx",
	"/kd/openpsid.prx",
	// "/kd/popsman.prx",			// use PS Vita's
	// "/kd/pops_01g.prx",			// use PS Vita's
	// "/kd/power_01g.prx",			// kermit_power.prx
	"/kd/psheet.prx",
	// "/kd/pspbtcnf.bin",			// pre-loaded
	// "/kd/pspnet.prx",
	// "/kd/pspnet_adhoc.prx",
	// "/kd/pspnet_adhocctl.prx",
	// "/kd/pspnet_adhoc_auth.prx",
	// "/kd/pspnet_adhoc_discover.prx",
	// "/kd/pspnet_adhoc_download.prx",
	// "/kd/pspnet_adhoc_matching.prx",
	// "/kd/pspnet_adhoc_transfer_int.prx",
	// "/kd/pspnet_apctl.prx",		// Not compatible on PS Vita
	// "/kd/pspnet_inet.prx",		// Not compatible on PS Vita
	// "/kd/pspnet_resolver.prx",
	// "/kd/pspnet_upnp.prx",
	// "/kd/pspnet_wispr.prx",
	"/kd/registry.prx",
	// "/kd/rtc.prx",				// kermit_rtc.prx
	"/kd/sc_sascore.prx",
	"/kd/semawm.prx",
	"/kd/sircs.prx",
	// "/kd/syscon.prx",			// Only for PSP
	// "/kd/sysmem.prx",			// pre-loaded
	// "/kd/systimer.prx",			// pre-loaded
	// "/kd/threadman.prx",			// pre-loaded
	// "/kd/umd9660.prx",			// Only for PSP
	// "/kd/umdcache.prx",			// Only for PSP
	// "/kd/umdman.prx",			// Only for PSP
	// "/kd/usb.prx",				// kermit_usb.prx
	// "/kd/usb1seg.prx",			// Only for PSP
	// "/kd/usbacc.prx",			// use PS Vita's
	// "/kd/usbcam.prx",			// use PS Vita's
	// "/kd/usbdmb.prx",			// Only for PSP
	// "/kd/usbgps.prx",			// use PS Vita's
	// "/kd/usbmic.prx",			// use PS Vita's
	// "/kd/usbpspcm.prx",			// use PS Vita's
	// "/kd/usbstor.prx",			// Only for PSP
	// "/kd/usbstorboot.prx",		// Only for PSP
	// "/kd/usbstormgr.prx",		// Only for PSP
	// "/kd/usbstorms.prx",			// Only for PSP
	"/kd/usersystemlib.prx",
	"/kd/utility.prx",
	"/kd/vaudio.prx",
	"/kd/videocodec_260.prx",
	"/kd/vshbridge.prx",
	"/kd/vshbridge_msapp.prx",
	// "/kd/wlan.prx",				// kermit_wlan.prx
	// "/kd/wlanfirm_01g.prx",		// Only for PSP
	"/kd/resource/impose.rsc",
	// "/kd/resource/meimg.img",	// Only for PSP
	// "/kd/resource/me_blimg.img",	// Only for PSP
	// "/kd/resource/me_sdimg.img",	// Only for PSP
	// "/kd/resource/me_t2img.img",	// Only for PSP
	// "/vsh/etc/index_01g.dat",	// Gives RSOD
	"/vsh/etc/version.txt",
	"/vsh/module/adhoc_transfer.prx",
	"/vsh/module/auth_plugin.prx",
	"/vsh/module/auto_connect.prx",
	"/vsh/module/camera_plugin.prx",
	"/vsh/module/common_gui.prx",
	"/vsh/module/common_util.prx",
	"/vsh/module/content_browser.prx",
	"/vsh/module/dd_helper.prx",
	"/vsh/module/dd_helper_utility.prx",
	"/vsh/module/dialogmain.prx",
	"/vsh/module/dnas_plugin.prx",
	"/vsh/module/file_parser_base.prx",
	"/vsh/module/game_install_plugin.prx",
	"/vsh/module/game_plugin.prx",
	"/vsh/module/htmlviewer_plugin.prx",
	"/vsh/module/htmlviewer_ui.prx",
	"/vsh/module/htmlviewer_utility.prx",
	"/vsh/module/hvauth_r.prx",
	"/vsh/module/impose_plugin.prx",
	"/vsh/module/launcher_plugin.prx",
	"/vsh/module/lftv_main_plugin.prx",
	"/vsh/module/lftv_middleware.prx",
	"/vsh/module/lftv_plugin.prx",
	"/vsh/module/libfont_arib.prx",
	"/vsh/module/libfont_hv.prx",
	"/vsh/module/libpspvmc.prx",
	"/vsh/module/libslim.prx",
	"/vsh/module/libwww.prx",
	"/vsh/module/marlindownloader.prx",
	"/vsh/module/mcore.prx",
	"/vsh/module/mlnapp_proxy.prx",
	"/vsh/module/mlnbb.prx",
	"/vsh/module/mlncmn.prx",
	"/vsh/module/mlnusb.prx",
	"/vsh/module/mm_flash.prx",
	"/vsh/module/msgdialog_plugin.prx",
	"/vsh/module/msvideo_main_plugin.prx",
	"/vsh/module/msvideo_plugin.prx",
	"/vsh/module/music_browser.prx",
	"/vsh/module/music_main_plugin.prx",
	"/vsh/module/music_parser.prx",
	"/vsh/module/music_player.prx",
	// "/vsh/module/netconf_plugin.prx",				// Use PS Vita's
	"/vsh/module/netconf_plugin_auto_bfl.prx",
	"/vsh/module/netconf_plugin_auto_nec.prx",
	"/vsh/module/netfront.prx",
	"/vsh/module/netplay_client_plugin.prx",
	"/vsh/module/netplay_server2_utility.prx",
	"/vsh/module/netplay_server_plus_utility.prx",
	"/vsh/module/netplay_server_utility.prx",
	"/vsh/module/npadmin_plugin.prx",
	"/vsh/module/npinstaller_plugin.prx",
	"/vsh/module/npsignin_plugin.prx",
	"/vsh/module/npsignup_plugin.prx",
	"/vsh/module/oneseg_core.prx",
	"/vsh/module/oneseg_hal_toolbox.prx",
	"/vsh/module/oneseg_launcher_plugin.prx",
	"/vsh/module/oneseg_plugin.prx",
	"/vsh/module/oneseg_sal.prx",
	"/vsh/module/oneseg_sdk.prx",
	"/vsh/module/oneseg_sdkcore.prx",
	"/vsh/module/opening_plugin.prx",
	"/vsh/module/osk_plugin.prx",
	"/vsh/module/paf.prx",
	"/vsh/module/pafmini.prx",
	"/vsh/module/photo_browser.prx",
	"/vsh/module/photo_main_plugin.prx",
	"/vsh/module/photo_player.prx",
	"/vsh/module/premo_plugin.prx",
	"/vsh/module/ps3scan_plugin.prx",
	"/vsh/module/psn_plugin.prx",
	"/vsh/module/psn_utility.prx",
	"/vsh/module/radioshack_plugin.prx",
	"/vsh/module/recommend_browser.prx",
	"/vsh/module/recommend_launcher_plugin.prx",
	"/vsh/module/recommend_main.prx",
	"/vsh/module/rss_browser.prx",
	"/vsh/module/rss_common.prx",
	"/vsh/module/rss_downloader.prx",
	"/vsh/module/rss_main_plugin.prx",
	"/vsh/module/rss_reader.prx",
	"/vsh/module/rss_subscriber.prx",
	"/vsh/module/savedata_auto_dialog.prx",
	"/vsh/module/savedata_plugin.prx",
	"/vsh/module/savedata_utility.prx",
	"/vsh/module/screenshot_plugin.prx",
	"/vsh/module/skype_main_plugin.prx",
	"/vsh/module/skype_plugin.prx",
	"/vsh/module/skype_skyhost.prx",
	"/vsh/module/skype_ve.prx",
	"/vsh/module/store_browser_plugin.prx",
	"/vsh/module/store_checkout_plugin.prx",
	"/vsh/module/store_checkout_utility.prx",
	"/vsh/module/subs_plugin.prx",
	"/vsh/module/sysconf_plugin.prx",
	"/vsh/module/update_plugin.prx",
	"/vsh/module/video_main_plugin.prx",
	"/vsh/module/video_plugin.prx",
	"/vsh/module/visualizer_plugin.prx",
	"/vsh/module/vshmain.prx",
	"/vsh/resource/01-12.bmp",
	"/vsh/resource/01-12_03g.bmp",
	"/vsh/resource/13-27.bmp",
	"/vsh/resource/adhoc_transfer.rco",
	"/vsh/resource/auth_plugin.rco",
	"/vsh/resource/camera_plugin.rco",
	"/vsh/resource/common_page.rco",
	"/vsh/resource/content_browser_plugin.rco",
	"/vsh/resource/custom_theme.dat",
	"/vsh/resource/dd_helper.rco",
	"/vsh/resource/dnas_plugin.rco",
	"/vsh/resource/gameboot.pmf",
	"/vsh/resource/game_install_plugin.rco",
	"/vsh/resource/game_plugin.rco",
	"/vsh/resource/htmlviewer.res",
	"/vsh/resource/htmlviewer_plugin.rco",
	"/vsh/resource/impose_plugin.rco",
	"/vsh/resource/lftv_main_plugin.rco",
	"/vsh/resource/lftv_rmc_univer3in1.rco",
	"/vsh/resource/lftv_rmc_univer3in1_jp.rco",
	"/vsh/resource/lftv_rmc_univerpanel.rco",
	"/vsh/resource/lftv_rmc_univerpanel_jp.rco",
	"/vsh/resource/lftv_rmc_univertuner.rco",
	"/vsh/resource/lftv_rmc_univertuner_jp.rco",
	"/vsh/resource/lftv_tuner_jp_jp.rco",
	"/vsh/resource/lftv_tuner_us_en.rco",
	"/vsh/resource/msgdialog_plugin.rco",
	"/vsh/resource/msvideo_main_plugin.rco",
	"/vsh/resource/music_browser_plugin.rco",
	"/vsh/resource/music_player_plugin.rco",
	"/vsh/resource/netconf_dialog.rco",
	"/vsh/resource/netplay_plugin.rco",
	"/vsh/resource/npadmin_plugin.rco",
	"/vsh/resource/npinstaller_plugin.rco",
	"/vsh/resource/npsignin_plugin.rco",
	"/vsh/resource/npsignup_plugin.rco",
	"/vsh/resource/oneseg_plugin.rco",
	"/vsh/resource/opening_plugin.rco",
	"/vsh/resource/osk_plugin.rco",
	"/vsh/resource/photo_browser_plugin.rco",
	"/vsh/resource/photo_player_plugin.rco",
	"/vsh/resource/premo_plugin.rco",
	"/vsh/resource/ps3scan_plugin.rco",
	"/vsh/resource/psn_plugin.rco",
	"/vsh/resource/radioshack_plugin.rco",
	"/vsh/resource/recommend_browser.rco",
	"/vsh/resource/rss_browser_plugin.rco",
	"/vsh/resource/rss_downloader_plugin.rco",
	"/vsh/resource/rss_subscriber.rco",
	"/vsh/resource/savedata_plugin.rco",
	"/vsh/resource/savedata_utility.rco",
	"/vsh/resource/screenshot_plugin.rco",
	"/vsh/resource/skype_main_plugin.rco",
	"/vsh/resource/store_browser_plugin.rco",
	"/vsh/resource/store_checkout_plugin.rco",
	"/vsh/resource/subs_plugin.rco",
	"/vsh/resource/sysconf_plugin.rco",
	"/vsh/resource/sysconf_plugin_about.rco",
	"/vsh/resource/system_plugin.rco",
	"/vsh/resource/system_plugin_bg.rco",
	"/vsh/resource/system_plugin_fg.rco",
	"/vsh/resource/topmenu_icon.rco",
	"/vsh/resource/topmenu_plugin.rco",
	"/vsh/resource/update_plugin.rco",
	"/vsh/resource/video_main_plugin.rco",
	"/vsh/resource/video_plugin_videotoolbar.rco",
	"/vsh/resource/visualizer_plugin.rco",
};