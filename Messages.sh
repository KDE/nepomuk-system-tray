#! /usr/bin/env bash
$EXTRACTRC `find . -name \*.rc -o  -name "*.ui" -o -name "*.kcfg"` >> rc.cpp
$XGETTEXT `find . -name "*.cpp"` -o $podir/nepomuk-system-tray.pot
#rm -rf rc.cpp
