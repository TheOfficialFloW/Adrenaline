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
set(VITA_APP_NAME "Adrenaline")
set(VITA_TITLEID  "PSPEMUCFW")

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wl,-q -Wall -O3")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-rtti -fno-exceptions")
set(VITA_MAKE_FSELF_FLAGS "${VITA_MAKE_FSELF_FLAGS} -a 0x2800000000000001")

add_executable(adrenaline
  main.c
  utils.c
  graphics.c
  font.c
)

target_link_libraries(adrenaline
  taiHEN_stub
  SceAppMgr_stub
  SceCtrl_stub
  SceDisplay_stub
  SceHttp_stub
  SceNet_stub
  SceNetCtl_stub
  SceSysmodule_stub
  ScePgf_stub_weak
)

vita_create_self(eboot.bin adrenaline UNSAFE)
vita_create_vpk(Adrenaline.vpk ${VITA_TITLEID} eboot.bin
  VERSION ${VITA_VERSION}
  NAME ${VITA_APP_NAME}
  FILE pkg/sce_sys/icon0.png sce_sys/icon0.png
       pkg/sce_sys/livearea/contents/bg.png sce_sys/livearea/contents/bg.png
       pkg/sce_sys/livearea/contents/startup.png sce_sys/livearea/contents/startup.png
       pkg/sce_sys/livearea/contents/template.xml sce_sys/livearea/contents/template.xml
       pkg/eboot.pbp eboot.pbp
	   pkg/sce_module/usbdevice.skprx sce_module/usbdevice.skprx
	   pkg/sce_module/adrenaline_kernel.skprx sce_module/adrenaline_kernel.skprx
	   pkg/sce_module/adrenaline_user.suprx sce_module/adrenaline_user.suprx
	   pkg/sce_module/adrenaline_vsh.suprx sce_module/adrenaline_vsh.suprx
)

add_custom_target(copy
  COMMAND cp eboot.bin F:/app/${VITA_TITLEID}/eboot.bin
  DEPENDS eboot.bin
)