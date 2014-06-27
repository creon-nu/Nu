#!/bin/bash
# create multiresolution windows icon
ICON_SRC=../../src/qt/res/icons/nu.png
ICON_DST=../../src/qt/res/icons/nu.ico
convert ${ICON_SRC} -resize 16x16 nu-16.png
convert ${ICON_SRC} -resize 32x32 nu-32.png
convert ${ICON_SRC} -resize 48x48 nu-48.png
convert nu-48.png nu-32.png nu-16.png ${ICON_DST}

