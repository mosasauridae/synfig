#!/bin/bash

if [[ -z "$SYNFIG_VERSION" ]]; then
  SYNFIG_VERSION=1.5.3
fi

if [[ -e Synfig-$SYNFIG_VERSION-win64.exe ]]; then
  mv Synfig-$SYNFIG_VERSION-win64.exe Synfig-$SYNFIG_VERSION-win64.bak.exe
fi

cd cmake-build
cpack -G NSIS

cp Synfig-$SYNFIG_VERSION-win64.exe ..
