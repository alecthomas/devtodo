#!/bin/sh

echo libtoolize
libtoolize --force --automake
#echo gettextize
#gettextize --force --copy --intl
#echo intltoolize
#intltoolize --force --copy
echo aclocal
aclocal
echo automake
automake --add-missing
echo autoconf
autoconf
echo autoheader
autoheader

echo "Now you are ready to run ./configure"
