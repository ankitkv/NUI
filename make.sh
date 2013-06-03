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

# If no argument is given, everything will be built
# Use sudo ./make.sh install to install the plugin

if [[ $1 == "clean" ]]; then
  (cd anaglyph && ./make.sh clean)
elif [[ $1 != "install" ]]; then
  if [[ $EUID -eq 0 ]]; then
    echo -e "\033[1;31mDo not run this as root.\033[0m"
  else
    (cd nui/bin && ./make.sh)
    (cd anaglyph && ./make.sh)
  fi
else
  if [[ $EUID -ne 0 ]]; then
    echo -e "\033[1;31mPlease run this as root.\033[0m"
  else
    (cd nui/bin && ./make.sh install)
    (cd anaglyph && ./make.sh install)
  fi
fi

