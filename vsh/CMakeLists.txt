cmake_minimum_required(VERSION 2.8)

project(adrenaline_vsh)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wl,-q -Wall -O3 -nostartfiles")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-rtti -fno-exceptions")

add_executable(${PROJECT_NAME}
  main.c
  utils.c
)

target_link_libraries(${PROJECT_NAME}
  taihen_stub
  SceAppMgr_stub
  SceLsdb_stub_weak
)

vita_create_self(${PROJECT_NAME}.suprx ${PROJECT_NAME} CONFIG exports.yml UNSAFE)

vita_create_stubs(${PROJECT_NAME}_stubs ${PROJECT_NAME} exports.yml)

if(NOT PKG_PATH)
  set(PKG_PATH ../../bubble/pkg)
endif(NOT PKG_PATH)

add_custom_target(pkg_vsh
  COMMAND cp ${PROJECT_NAME}.suprx ${PKG_PATH}/sce_module/${PROJECT_NAME}.suprx
  DEPENDS ${PROJECT_NAME}.suprx
)
