cmake_minimum_required (VERSION 2.8.8)
project (libregmap)

set(CMAKE_DISABLE_IN_SOURCE_BUILD ON)
set(CMAKE_DISABLE_SOURCE_CHANGES  OFF)

if ("${CMAKE_SOURCE_DIR}" STREQUAL "${CMAKE_BINARY_DIR}")
  message(SEND_ERROR "In-source builds are not allowed.")
endif ()

set(CMAKE_VERBOSE_MAKEFILE OFF)
set(CMAKE_COLOR_MAKEFILE   ON)


set(VERSION "0.1.0")

set(WUPPER_CODE_GEN ${CMAKE_CURRENT_SOURCE_DIR}/../wuppercodegen/wuppercodegen/cli.py)
set(REGISTER_MAP ${CMAKE_CURRENT_SOURCE_DIR}/../../firmware/sources/templates/registers-3.2.yaml)

set(CPACK_RPM_PACKAGE_AUTOREQPROV " no")
set(CPACK_PACKAGE_VERSION ${VERSION})
set(CPACK_GENERATOR "RPM" "TGZ")
set(CPACK_PACKAGE_NAME "regmap")
set(CPACK_PACKAGE_RELEASE 1)
set(CPACK_PACKAGE_CONTACT "Mark Dönszelmann")
set(CPACK_PACKAGE_VENDOR "CERN")
#set(CPACK_PACKAGING_INSTALL_PREFIX ${CMAKE_INSTALL_PREFIX})
set(CPACK_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION}.${CMAKE_SYSTEM_PROCESSOR}")

if(!CPack_CMake_INCLUDED)
  include(CPack)
endif()

## Add fPIC to get static lib to be linkable by other .so's, as in the artdaq code. EC, 1-Nov-2017
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -std=c++11 -Wall -g -fPIC")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall")
if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Intel")
endif()


include_directories("." ${CMAKE_CURRENT_SOURCE_DIR})

SET_DIRECTORY_PROPERTIES(PROPERTIES CLEAN_NO_CUSTOM 1)

if (EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/regenerate")
  add_custom_command(
      OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/regmap/regmap-struct.h
      COMMAND ${WUPPER_CODE_GEN} ${REGISTER_MAP} src/regmap-struct.h.template regmap/regmap-struct.h
      DEPENDS src/regmap-struct.h.template ${REGISTER_MAP}
      WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
  )

  add_custom_command(
      OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/regmap/regmap-symbol.h
      COMMAND ${WUPPER_CODE_GEN} ${REGISTER_MAP} src/regmap-symbol.h.template regmap/regmap-symbol.h
      DEPENDS src/regmap-symbol.h.template ${REGISTER_MAP}
      WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
  )

  add_custom_command(
      OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/src/regmap-symbol.c
      COMMAND ${WUPPER_CODE_GEN} ${REGISTER_MAP} src/regmap-symbol.c.template src/regmap-symbol.c
      DEPENDS src/regmap-symbol.c.template ${REGISTER_MAP}
      WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
  )
endif()


add_library(regmap regmap/regmap-common.h regmap/regmap-struct.h regmap/regmap-symbol.h src/regmap-symbol.c)
target_link_libraries(regmap)
add_library(regmap2 SHARED src/regmap-symbol.c)

add_executable(test-regmap-symbol test/test-regmap-symbol.c)
target_link_libraries(test-regmap-symbol regmap)

add_executable(test-regmap-struct test/test-regmap-struct.c)
target_link_libraries(test-regmap-struct regmap)

install(TARGETS regmap
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib)
install(FILES regmap/regmap-common.h regmap/regmap-struct.h regmap/regmap-symbol.h regmap/regmap.h DESTINATION include/regmap)
