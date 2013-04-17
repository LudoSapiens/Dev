#!/bin/sh
##
# Copies the directories contained inside the user's Data directory
# into the application bundle.
##

srcDir=${SRCROOT}/../../../Data
dstDir=${BUILT_PRODUCTS_DIR}/${UNLOCALIZED_RESOURCES_FOLDER_PATH}

# Parse argument.
case "${1}" in
   "link" )
      CP="ln -sf";;
   "copy" )
      CP="cp -rf";;
   "echo" )
      CP="echo";;
   * )
      # Default to use if no argument is sent.
      CP="ln -sf";;
esac

#echo file=${file}
#echo src=${src}
#echo dst=${dst}
#echo srcroot=${SRCROOT}
for src in ${srcDir}/*
do
   case ${src} in
      *_museum )
         ;;
      *model )
         ;;
      *skeleton )
         ;;
      * )
         ${CP} ${src} ${dstDir}
         ;;
   esac
done
