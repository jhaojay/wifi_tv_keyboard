# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

file(MAKE_DIRECTORY
  "/home/phi/esp/esp-idf/components/bootloader/subproject"
  "/home/phi/tv_keyboard/build/bootloader"
  "/home/phi/tv_keyboard/build/bootloader-prefix"
  "/home/phi/tv_keyboard/build/bootloader-prefix/tmp"
  "/home/phi/tv_keyboard/build/bootloader-prefix/src/bootloader-stamp"
  "/home/phi/tv_keyboard/build/bootloader-prefix/src"
  "/home/phi/tv_keyboard/build/bootloader-prefix/src/bootloader-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "/home/phi/tv_keyboard/build/bootloader-prefix/src/bootloader-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "/home/phi/tv_keyboard/build/bootloader-prefix/src/bootloader-stamp${cfgdir}") # cfgdir has leading slash
endif()
