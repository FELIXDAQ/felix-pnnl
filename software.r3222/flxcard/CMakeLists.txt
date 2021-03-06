cmake_minimum_required (VERSION 2.8.8)
project(flxcard)

set(VERSION "0.0.0dev")

set(CPACK_RPM_PACKAGE_AUTOREQPROV "no")
set(CPACK_PACKAGE_VERSION ${VERSION})
set(CPACK_GENERATOR "RPM" "TGZ")
set(CPACK_PACKAGE_NAME "flxcard")
set(CPACK_PACKAGE_RELEASE 1)
set(CPACK_PACKAGE_CONTACT "Markus Joos")
set(CPACK_PACKAGE_VENDOR "CERN")
set(CPACK_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION}.${CMAKE_SYSTEM_PROCESSOR}")
include(CPack)

add_subdirectory(../regmap ${CMAKE_CURRENT_BINARY_DIR}/regmap)

###include_directories(/afs/cern.ch/user/j/joos/DAQ/DataFlow/ROSRCDdrivers/FELIX_bundle/FELIX_bundle/include)
###link_directories(/afs/cern.ch/user/j/joos/DAQ/DataFlow/ROSRCDdrivers/FELIX_bundle/FELIX_bundle/lib)

include_directories(flxcard "../drivers_rcc/flx/include/" "../drivers_rcc/flx" "../regmap" "/afs/cern.ch/atlas/project/tdaq/inst/tdaq/tdaq-06-01-01/installed/include/" "../drivers/cmem/include/" "../drivers_rcc/" "../drivers_rcc/cmem_rcc/") # 2nd to last one needed for DFDebug.h, last one for CMEM_* declarations.
link_directories(/afs/cern.ch/atlas/project/tdaq/inst/tdaq/tdaq-06-01-01/installed/x86_64-slc6-gcc49-opt/lib ../drivers_rcc/lib64) ## last one for DFDebug.so

set (CMAKE_CXX_FLAGS "-DDEBUG_LEVEL=1 --pedantic -Wall")

set (CMAKE_BUILD_TYPE Debug)

add_library(FlxCard SHARED src/FlxCard.cpp src/FlxException.cpp)

add_executable(FlxCard_scope src/FlxCard_scope.cpp)
target_link_libraries (FlxCard_scope FlxCard regmap DFDebug getinput)

add_executable(flx-eeprom src/flx-eeprom.cpp)
target_link_libraries (flx-eeprom FlxCard regmap DFDebug getinput)

add_executable(flx-reset src/flx-reset.cpp)
target_link_libraries (flx-reset FlxCard regmap DFDebug getinput)

add_executable(flx-config src/flx-config.cpp)
target_link_libraries (flx-config FlxCard regmap DFDebug getinput)

add_executable(flx-irq-test src/flx-irq-test.cpp)
target_link_libraries (flx-irq-test FlxCard regmap DFDebug getinput)

add_executable(flx-i2c src/flx-i2c.cpp)
target_link_libraries (flx-i2c FlxCard regmap DFDebug getinput)

add_executable(flx-throughput src/flx-throughput.cpp)
target_link_libraries (flx-throughput FlxCard DFDebug regmap getinput rcc_error cmem_rcc rt)

add_executable(flx-info src/flx-info.cpp)
target_link_libraries (flx-info FlxCard regmap DFDebug getinput)

add_executable(flx-spi src/flx-spi.cpp)
target_link_libraries (flx-spi FlxCard regmap DFDebug getinput)

add_executable(flx-dma-test src/flx-dma-test.cpp)
target_link_libraries (flx-dma-test FlxCard regmap DFDebug getinput rcc_error cmem_rcc)

add_executable(flx-dma-stat src/flx-dma-stat.cpp)
target_link_libraries (flx-dma-stat FlxCard regmap DFDebug getinput rcc_error cmem_rcc)

add_executable(flx-dump-blocks src/flx-dump-blocks.cpp)
target_link_libraries (flx-dump-blocks FlxCard regmap DFDebug getinput rcc_error cmem_rcc)

add_executable(flx-init src/flx-init.cpp)
target_link_libraries (flx-init FlxCard regmap DFDebug getinput rcc_error cmem_rcc)

#SET(CMAKE_INSTALL_PREFIX "/afs/cern.ch/user/j/joos/DAQ/FELIX/hostSoftware/FlxCard")
SET(CMAKE_INSTALL_PREFIX "/data/daq/FELIX/hostSoftware/FlxCard")

install(TARGETS FlxCard FlxCard_scope flx-config flx-eeprom flx-reset flx-i2c flx-irq-test flx-throughput flx-info flx-spi flx-dma-test flx-dump-blocks flx-init
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib)
install(FILES flxcard/FlxCard.h DESTINATION include)
install(FILES flxcard/FlxException.h DESTINATION include)
