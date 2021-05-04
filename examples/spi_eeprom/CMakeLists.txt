#  Copyright (c) Alan Ludwig. All rights reserved.
#  Licensed under the MIT License.

cmake_minimum_required(VERSION 3.11)
project (spi_eeprom CXX)

azsphere_configure_tools(TOOLS_REVISION "21.04")
azsphere_configure_api(TARGET_API_SET "9")

# Create executable
add_executable (${PROJECT_NAME} 
					main.cpp
					../../spi25xx.cpp
					../../spi25xx.h
					)
TARGET_LINK_LIBRARIES(${PROJECT_NAME} applibs pthread gcc_s c)

azsphere_target_add_image_package(${PROJECT_NAME})

set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-rtti")
