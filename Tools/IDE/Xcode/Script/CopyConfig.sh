#!/bin/sh
##
# Copies the config.lua file if it exists in the user's Data directory
# or simply creates an empty one otherwise.
##
file=config.lua
src=${SRCROOT}/../../../Data/${file}
dst=${BUILT_PRODUCTS_DIR}/${UNLOCALIZED_RESOURCES_FOLDER_PATH}/${file}
#echo file=${file}
#echo src=${src}
#echo dst=${dst}
#echo srcroot=${SRCROOT}
if [ -e ${src} ]
then
   #echo cp -f ${src} ${dst}
   cp -f ${src} ${dst}
#else
#   #echo touch ${dst}
#   touch ${dst}
fi
#exit 1
