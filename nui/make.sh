#!/bin/bash
##############################################################################
#                                                                            #
#  3D Natural User Interface for Operating Systems                           #
#  Copyright (C) 2013 Ankit Vani,                                            #
#                     Humayun Mulla,                                         #
#                     Ronit Kulkarni,                                        #
#                     Siddharth Kulkarni                                     #
#                                                                            #
#  Licensed under the Apache License, Version 2.0 (the "License");           #
#  you may not use this file except in compliance with the License.          #
#  You may obtain a copy of the License at                                   #
#                                                                            #
#      http://www.apache.org/licenses/LICENSE-2.0                            #
#                                                                            #
#  Unless required by applicable law or agreed to in writing, software       #
#  distributed under the License is distributed on an "AS IS" BASIS,         #
#  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.  #
#  See the License for the specific language governing permissions and       #
#  limitations under the License.                                            #
#                                                                            #
##############################################################################

# SYNTAX: ./make.sh [what]
# where [what] can be one of:
# 
# l OR lib:  Build only the library
# v OR view: Build only the library and the calib
# t OR test: Build only the library and test app
# clean:     Clean all binaries
# 
# If no argument is given, everything will be built
#
# Use sudo ./make.sh install to install the library


if [[ $1 == "l"* ]]; then
  if [[ $EUID -eq 0 ]]; then
    echo -e "\033[1;31mDo not run this as root.\033[0m"
  else
    echo -e "\033[1;34mCompiling...\033[0m"
    (( echo -e "\033[1;33m-\033[0m libnui" && (cd libnui && make CFG=Debug) > /dev/null \
    && echo -e "\033[1;32mSuccessfully compiled!\033[0m") || echo -e "\033[1;31mCompilation failed.\033[0m")
  fi
elif [[ $1 == "v"* ]]; then
  if [[ $EUID -eq 0 ]]; then
    echo -e "\033[1;31mDo not run this as root.\033[0m"
  else
    echo -e "\033[1;34mCompiling...\033[0m"
    (( echo -e "\033[1;33m-\033[0m libnui" && (cd libnui && make CFG=Debug) > /dev/null \
    && echo -e "\033[1;33m-\033[0m calib" && (cd calib && make CFG=Debug) > /dev/null \
    && echo -e "\033[1;32mSuccessfully compiled!\033[0m") || echo -e "\033[1;31mCompilation failed.\033[0m")
  fi
elif [[ $1 == "t"* ]]; then
  if [[ $EUID -eq 0 ]]; then
    echo -e "\033[1;31mDo not run this as root.\033[0m"
  else
    echo -e "\033[1;34mCompiling...\033[0m"
    (( echo -e "\033[1;33m-\033[0m libnui" && (cd libnui && make CFG=Debug) > /dev/null \
    && echo -e "\033[1;33m-\033[0m test"   && (cd test   && make CFG=Debug) > /dev/null \
    && echo -e "\033[1;32mSuccessfully compiled!\033[0m") || echo -e "\033[1;31mCompilation failed.\033[0m")
  fi
elif [[ $1 == "clean" ]]; then
  if [[ $EUID -eq 0 ]]; then
    echo -e "\033[1;31mDo not run this as root.\033[0m"
  else
    echo -e "\033[1;34mCleaning...\033[0m"
    (( echo -e "\033[1;33m-\033[0m libnui" && (cd libnui && make clean) > /dev/null \
    && echo -e "\033[1;33m-\033[0m calib" && (cd calib && make clean) > /dev/null \
    && echo -e "\033[1;33m-\033[0m test"   && (cd test   && make clean) > /dev/null \
    && rm -rf bin/Intermediate/ > /dev/null \
    && echo -e "\033[1;32mCleaned.\033[0m") || echo -e "\033[1;31mCleaning failed.\033[0m")
  fi
elif [[ $1 != "install" ]]; then
  echo -e "\033[1;34mCompiling...\033[0m"
  (( echo -e "\033[1;33m-\033[0m libnui" && (cd libnui && make CFG=Debug) > /dev/null \
  && echo -e "\033[1;33m-\033[0m calib" && (cd calib && make CFG=Debug) > /dev/null \
  && echo -e "\033[1;33m-\033[0m test"   && (cd test   && make CFG=Debug) > /dev/null \
  && echo -e "\033[1;32mSuccessfully compiled!\033[0m") || echo -e "\033[1;31mCompilation failed.\033[0m")
else
  if [[ $EUID -ne 0 ]]; then
    echo -e "\033[1;31mPlease run this as root.\033[0m"
  else
    (install bin/libnui.so /usr/local/lib/ && echo -e "\033[1;32mSuccessfully installed!\033[0m") || echo -e "\033[1;31mInstallation failed. Please make sure you have the correct permissions.\033[0m"
  fi
fi

