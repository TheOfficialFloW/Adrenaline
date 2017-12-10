cmake_minimum_required(VERSION 2.8)

if(NOT DEFINED CMAKE_TOOLCHAIN_FILE)
  if(DEFINED ENV{VITASDK})
    set(CMAKE_TOOLCHAIN_FILE "$ENV{VITASDK}/share/vita.toolchain.cmake" CACHE PATH "toolchain file")
  else()
    message(FATAL_ERROR "Please define VITASDK to point to your SDK path!")
  endif()
endif()

include("$ENV{VITASDK}/share/vita.cmake" REQUIRED)

project(Adrenaline.vpk)

set(VITA_APP_NAME "Adrenaline")

if(NOT VITA_TITLEID)
  set(VITA_TITLEID  "PSPEMUCFW")
  add_definitions(-DADRENALINE_TITLEID="${VITA_TITLEID}")
endif(NOT VITA_TITLEID)

set(PKG_PATH ${CMAKE_BINARY_DIR}/pkg)
set(BUBBLE_PKG_PATH bubble/pkg)

file(MAKE_DIRECTORY ${PKG_PATH})
file(MAKE_DIRECTORY ${PKG_PATH}/sce_module)

add_subdirectory(kernel)
add_subdirectory(user)
add_subdirectory(vsh)
add_subdirectory(bubble)

vita_create_vpk(${PROJECT_NAME} ${VITA_TITLEID} ${PKG_PATH}/eboot.bin
  VERSION ${VITA_VERSION}
  NAME ${VITA_APP_NAME}
  FILE ${BUBBLE_PKG_PATH}/sce_sys/icon0.png sce_sys/icon0.png
       ${BUBBLE_PKG_PATH}/sce_sys/livearea/contents/bg.png sce_sys/livearea/contents/bg.png
       ${BUBBLE_PKG_PATH}/sce_sys/livearea/contents/startup.png sce_sys/livearea/contents/startup.png
       ${BUBBLE_PKG_PATH}/sce_sys/livearea/contents/template.xml sce_sys/livearea/contents/template.xml
      ${BUBBLE_PKG_PATH}/eboot.pbp eboot.pbp
      ${BUBBLE_PKG_PATH}/sce_module/usbdevice.skprx sce_module/usbdevice.skprx
      ${PKG_PATH}/sce_module/adrenaline_kernel.skprx sce_module/adrenaline_kernel.skprx
      ${PKG_PATH}/sce_module/adrenaline_user.suprx sce_module/adrenaline_user.suprx
      ${PKG_PATH}/sce_module/adrenaline_vsh.suprx sce_module/adrenaline_vsh.suprx
)

add_dependencies(${PROJECT_NAME} pkg_kernel)
add_dependencies(${PROJECT_NAME} pkg_user)
add_dependencies(${PROJECT_NAME} pkg_vsh)
add_dependencies(${PROJECT_NAME} pkg_bubble)
