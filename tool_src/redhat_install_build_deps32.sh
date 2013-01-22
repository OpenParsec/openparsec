#!/bin/bash
echo; echo;
echo "This program should, in theory, install all the deps needed to"
echo "compile Open Parsec on a Red Hat based system (RHEL 6.x and higher"
echo; echo;
echo "*** WARNING *** *** WARNING *** *** WARNING ***  "
echo ; echo
echo "This program WILL remove some programs which depend on 64bit SDL"
echo "since 32bit SDL cannot co-exist at the moment.  You will be prompted"
echo "by yum to remove them."   
echo; echo
echo "Press Enter to continue."
read
echo "Attempting to remove previous verisons of SDL..."
yum remove "SDL*"
if [ "$?" -ne "0" ]
then
	echo "*** Exiting on user cancel. Open Parsec Build Dependencies not installed!!!"
	exit 1
fi
 
echo; 
echo;
echo "Installing build Dependencies."
yum -y install glibc-devel.i686 libjpeg-devel.i686 libstdc++-devel.i686 libXext-devel.i686 http://www.libsdl.org/release/SDL-devel-1.2.15-1.i386.rpm http://www.libsdl.org/release/SDL-1.2.15-1.i386.rpm http://www.libsdl.org/projects/SDL_mixer/release/SDL_mixer-1.2.12-1.i386.rpm http://www.libsdl.org/projects/SDL_mixer/release/SDL_mixer-devel-1.2.12-1.i386.rpm
