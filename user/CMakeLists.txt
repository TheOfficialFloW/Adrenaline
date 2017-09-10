cmake_minimum_required(VERSION 2.8)

project(adrenaline_user)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wl,-q -Wall -O3 -nostartfiles")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-rtti -fno-exceptions")

add_executable(${PROJECT_NAME}
  main.c
  pops.c
  titleinfo.c
  flashfs.c
  msfs.c
  menu.c
  states.c
  usb.c
  utils.c
  lz4/lz4.c
)

target_link_libraries(${PROJECT_NAME}
  AdrenalineKernel_stub
  taihen_stub
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
  SceProcessmgr_stub
  SceRtc_stub
  SceScreenShot_stub_weak
  SceShellSvc_stub
  SceSysmem_stub
  SceSysmodule_stub
  SceUdcd_stub
  SceUsbstorVStorDriver_stub
  SceVshBridge_stub
)

vita_create_self(${PROJECT_NAME}.suprx ${PROJECT_NAME} CONFIG exports.yml UNSAFE)

vita_create_stubs(${PROJECT_NAME}_stubs ${PROJECT_NAME} exports.yml)

if(NOT PKG_PATH)
  set(PKG_PATH ../../bubble/pkg)
endif(NOT PKG_PATH)

add_custom_target(pkg_user
  COMMAND cp ${PROJECT_NAME}.suprx ${PKG_PATH}/sce_module/${PROJECT_NAME}.suprx
  DEPENDS ${PROJECT_NAME}.suprx
)
