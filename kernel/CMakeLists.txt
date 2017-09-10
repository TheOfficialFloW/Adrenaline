cmake_minimum_required(VERSION 2.8)

project(adrenaline_kernel)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wl,-q -Wall -O3 -nostdlib")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-rtti -fno-exceptions")

add_executable(${PROJECT_NAME}
  main.c
  utils.c
)

target_link_libraries(${PROJECT_NAME}
  taihenForKernel_stub
  SceCpuForDriver_stub
  SceCtrlForDriver_stub
  SceIofilemgrForDriver_stub
  SceModulemgrForDriver_stub
  SceModulemgrForKernel_stub
  SceProcessmgrForDriver_stub
  SceSysclibForDriver_stub
  SceSysmemForDriver_stub
  SceSysrootForKernel_stub
  SceThreadmgrForDriver_stub
)

vita_create_self(${PROJECT_NAME}.skprx ${PROJECT_NAME} CONFIG exports.yml UNSAFE)

vita_create_stubs(${PROJECT_NAME}_stubs ${PROJECT_NAME} exports.yml KERNEL)

install(DIRECTORY ${CMAKE_BINARY_DIR}/stubs/
  DESTINATION lib
  FILES_MATCHING PATTERN "*.a"
)

if(NOT PKG_PATH)
  set(PKG_PATH ../../bubble/pkg)
endif(NOT PKG_PATH)

add_custom_target(pkg_kernel
  COMMAND cp ${PROJECT_NAME}.skprx ${PKG_PATH}/sce_module/${PROJECT_NAME}.skprx
  DEPENDS ${PROJECT_NAME}.skprx
)
