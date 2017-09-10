cmake_minimum_required(VERSION 2.8)

project(adrenaline_bubble)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wl,-q -Wall -O3")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-rtti -fno-exceptions")
set(VITA_MAKE_FSELF_FLAGS "${VITA_MAKE_FSELF_FLAGS} -a 0x2800000000000001")

add_executable(${PROJECT_NAME}
  main.c
  utils.c
  graphics.c
  font.c
)

target_link_libraries(${PROJECT_NAME}
  taihen_stub
  SceAppMgr_stub
  SceCtrl_stub
  SceDisplay_stub
  SceHttp_stub
  SceNet_stub
  SceNetCtl_stub
  SceSysmodule_stub
  ScePgf_stub_weak
)

if(NOT PKG_PATH)
  set(PKG_PATH ${CMAKE_SOURCE_DIR}/pkg)
endif(NOT PKG_PATH)

vita_create_self(eboot.bin ${PROJECT_NAME} UNSAFE)

add_custom_target(pkg_bubble ALL
  COMMAND cp eboot.bin ${PKG_PATH}/eboot.bin
  DEPENDS eboot.bin
)
