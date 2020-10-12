cmake_minimum_required(VERSION 2.8)

if(NOT DEFINED CMAKE_TOOLCHAIN_FILE)
  if(DEFINED ENV{VITASDK})
    set(CMAKE_TOOLCHAIN_FILE "$ENV{VITASDK}/share/vita.toolchain.cmake" CACHE PATH "toolchain file")
  else()
    message(FATAL_ERROR "Please define VITASDK to point to your SDK path!")
  endif()
endif()

project(adrenaline_user)
include("${VITASDK}/share/vita.cmake" REQUIRED)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wl,-q -Wall -O3 -nostartfiles -fno-lto")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-rtti -fno-exceptions")

# Builds
FUNCTION(ADD_RESOURCES out_var)
  SET(result)
  FOREACH(in_f ${ARGN})
  SET(out_f "${CMAKE_CURRENT_BINARY_DIR}/${in_f}.o")
  GET_FILENAME_COMPONENT(out_dir ${out_f} DIRECTORY)
  ADD_CUSTOM_COMMAND(OUTPUT ${out_f}
    COMMAND ${CMAKE_COMMAND} -E make_directory ${out_dir}
    COMMAND ${CMAKE_LINKER} -r -b binary -o ${out_f} ${in_f}
    DEPENDS ${in_f}
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    COMMENT "Building resource ${out_f}"
    VERBATIM
    )
  LIST(APPEND result ${out_f})
  ENDFOREACH()
  SET(${out_var} "${result}" PARENT_SCOPE)
ENDFUNCTION()

file(GLOB res_files RELATIVE
  ${CMAKE_SOURCE_DIR}
  flash0/kd/*.prx
  flash0/kd/*.bin
  flash0/vsh/module/*.prx
  flash0/*.bin
)
add_resources(cef_res ${res_files})

add_executable(adrenaline_user
  ${cef_res}
  main.c
  pops.c
  titleinfo.c
  flashfs.c
  msfs.c
  menu.c
  states.c
  usb.c
  utils.c
  math_utils.c
  lz4/lz4.c
)

target_link_libraries(adrenaline_user
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

vita_create_self(adrenaline_user.suprx adrenaline_user CONFIG exports.yml UNSAFE)

vita_create_stubs(stubs adrenaline_user ${CMAKE_SOURCE_DIR}/exports.yml)

add_custom_target(copy
  COMMAND cp adrenaline_user.suprx F:/app/PSPEMUCFW/sce_module/adrenaline_user.suprx
  DEPENDS adrenaline_user.suprx
)

add_custom_target(pkg
  COMMAND mkdir -p ../../bubble/pkg
  COMMAND cp adrenaline_user.suprx ../../bubble/pkg/sce_module/adrenaline_user.suprx
  DEPENDS adrenaline_user.suprx
)

add_custom_target(updater
  COMMAND bin2c adrenaline_user.suprx ../../cef/updater/adrenaline_user.h adrenaline_user
  DEPENDS adrenaline_user.suprx
)
