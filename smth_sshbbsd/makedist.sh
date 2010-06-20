#!/bin/sh
echo "hehe,you must have automake 1.6.x and autoconf 2.5.x to use this,kick kcn@smth"
aclocal-1.7
autoheader2.50
automake-1.7 --add-missing
autoconf2.50
