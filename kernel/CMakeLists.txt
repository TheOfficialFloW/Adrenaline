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

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wl,-q -Wall -O3 -nostdlib")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-rtti -fno-exceptions")

add_executable(adrenaline
  main.c
  utils.c
)

target_link_libraries(adrenaline
  taihenForKernel_stub
  SceCpuForDriver_stub
  SceCtrlForDriver_stub
  SceIofilemgrForDriver_stub
  SceModulemgrForKernel_stub
  SceSysclibForDriver_stub
  SceSysmemForDriver_stub
  SceThreadmgrForDriver_stub
)

vita_create_self(adrenaline.skprx adrenaline CONFIG exports.yml UNSAFE)

vita_create_stubs(stubs adrenaline ${CMAKE_SOURCE_DIR}/exports.yml KERNEL)

install(DIRECTORY ${CMAKE_BINARY_DIR}/stubs/
  DESTINATION lib
  FILES_MATCHING PATTERN "*.a"
)

add_custom_target(copy
  COMMAND cp adrenaline.skprx G:/adrenaline/adrenaline.skprx
  DEPENDS adrenaline.skprx
)