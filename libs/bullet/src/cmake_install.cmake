# Install script for directory: /home/i7626944/Documents/masterclass/libs/bullet/src

# Set the install prefix
IF(NOT DEFINED CMAKE_INSTALL_PREFIX)
  SET(CMAKE_INSTALL_PREFIX "/usr/local")
ENDIF(NOT DEFINED CMAKE_INSTALL_PREFIX)
STRING(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
IF(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  IF(BUILD_TYPE)
    STRING(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  ELSE(BUILD_TYPE)
    SET(CMAKE_INSTALL_CONFIG_NAME "Release")
  ENDIF(BUILD_TYPE)
  MESSAGE(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
ENDIF(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)

# Set the component getting installed.
IF(NOT CMAKE_INSTALL_COMPONENT)
  IF(COMPONENT)
    MESSAGE(STATUS "Install component: \"${COMPONENT}\"")
    SET(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  ELSE(COMPONENT)
    SET(CMAKE_INSTALL_COMPONENT)
  ENDIF(COMPONENT)
ENDIF(NOT CMAKE_INSTALL_COMPONENT)

# Install shared libraries without execute permission?
IF(NOT DEFINED CMAKE_INSTALL_SO_NO_EXE)
  SET(CMAKE_INSTALL_SO_NO_EXE "0")
ENDIF(NOT DEFINED CMAKE_INSTALL_SO_NO_EXE)

IF(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")
  FILE(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/bullet" TYPE FILE FILES
    "/home/i7626944/Documents/masterclass/libs/bullet/src/btBulletCollisionCommon.h"
    "/home/i7626944/Documents/masterclass/libs/bullet/src/btBulletDynamicsCommon.h"
    )
ENDIF(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")

IF(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for each subdirectory.
  INCLUDE("/home/i7626944/Documents/masterclass/libs/bullet/src/Bullet3OpenCL/cmake_install.cmake")
  INCLUDE("/home/i7626944/Documents/masterclass/libs/bullet/src/Bullet3Serialize/Bullet2FileLoader/cmake_install.cmake")
  INCLUDE("/home/i7626944/Documents/masterclass/libs/bullet/src/Bullet3Dynamics/cmake_install.cmake")
  INCLUDE("/home/i7626944/Documents/masterclass/libs/bullet/src/Bullet3Collision/cmake_install.cmake")
  INCLUDE("/home/i7626944/Documents/masterclass/libs/bullet/src/Bullet3Geometry/cmake_install.cmake")
  INCLUDE("/home/i7626944/Documents/masterclass/libs/bullet/src/Bullet3Common/cmake_install.cmake")
  INCLUDE("/home/i7626944/Documents/masterclass/libs/bullet/src/BulletInverseDynamics/cmake_install.cmake")
  INCLUDE("/home/i7626944/Documents/masterclass/libs/bullet/src/BulletSoftBody/cmake_install.cmake")
  INCLUDE("/home/i7626944/Documents/masterclass/libs/bullet/src/BulletCollision/cmake_install.cmake")
  INCLUDE("/home/i7626944/Documents/masterclass/libs/bullet/src/BulletDynamics/cmake_install.cmake")
  INCLUDE("/home/i7626944/Documents/masterclass/libs/bullet/src/LinearMath/cmake_install.cmake")

ENDIF(NOT CMAKE_INSTALL_LOCAL_ONLY)

