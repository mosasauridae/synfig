#!/bin/bash

if [[ -e Synfig-1.5.1-win64.exe ]]; then
  mv Synfig-1.5.1-win64.exe Synfig-1.5.1-win64.bak.exe
fi

cd cmake-build
cpack -G NSIS

cp Synfig-1.5.1-win64.exe ..
