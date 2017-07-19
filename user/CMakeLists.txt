cmake_minimum_required(VERSION 2.8)

if(NOT DEFINED CMAKE_TOOLCHAIN_FILE)
  if(DEFINED ENV{VITASDK})
    set(CMAKE_TOOLCHAIN_FILE "$ENV{VITASDK}/share/vita.toolchain.cmake" CACHE PATH "toolchain file")
  else()
    message(FATAL_ERROR "Please define VITASDK to point to your SDK path!")
  endif()
endif()

project(adrenaline)
include("${VITASDK}/share/vita.cmake" REQUIRED)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wl,-q -Wall -O3 -nostartfiles")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-rtti -fno-exceptions")

add_executable(adrenaline
  main.c
  flashfs.c
  msfs.c
  menu.c
  states.c
  usb.c
  utils.c
  lz4/lz4.c
)

target_link_libraries(adrenaline
  AdrenalineKernel_stub
  taiHEN_stub
  vita2d
  vitashaders
  c
  m
  SceAppMgr_stub
  SceAppUtil_stub
  SceAudio_stub
  SceCommonDialog_stub
  SceCompat_stub
  SceCtrl_stub
  SceDisplay_stub
  SceKernelDmacMgr_stub
  SceGxm_stub
  SceIofilemgr_stub
  SceKernelModulemgr_stub
  SceKernelThreadMgr_stub
  SceLibKernel_stub
  SceMtpIfDriver_stub
  ScePgf_stub_weak
  ScePower_stub
  SceProcessMgr_stub
  SceRtc_stub
  SceScreenShot_stub_weak
  SceShellSvc_stub
  SceSysmem_stub
  SceSysmodule_stub
  SceUdcd_stub
  SceUsbstorVStorDriver_stub
  SceVshBridge_stub
)

vita_create_self(adrenaline.suprx adrenaline CONFIG exports.yml UNSAFE)

vita_create_stubs(stubs adrenaline ${CMAKE_SOURCE_DIR}/exports.yml)

add_custom_target(copy
  COMMAND cp adrenaline.suprx F:/adrenaline/adrenaline.suprx
  DEPENDS adrenaline.suprx
)

add_custom_target(updater
  COMMAND bin2c adrenaline.suprx ../../cef/updater/adrenaline_suprx.h adrenaline_suprx
  DEPENDS adrenaline.suprx
)