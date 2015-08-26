#!/bin/bash
# module rename

if [ "${#@}" -lt 1 ] ; then
	echo "$0 module"
	exit 1
fi

module=$1
modif=`modinfo $module`
if [ "$?" != 0 ] ; then 
	echo "can't modinfo"
	exit 2
fi 

module_ver=`echo "$modif" | grep "^version:" | awk '{print $2}'`
module_kernel_ver=`echo "$modif" | grep "^vermagic:" | awk '{print $2}'`

ws_kernel=`echo "$module_kernel_ver" | grep "ws"`
if [ -n "$ws_kernel" ] ; then
	module_kernel_ver="ws"`echo $module_kernel_ver | awk -F "ws" '{print $2}'`
fi

module_name=`echo "$module" | awk -F '.' '{print $1}'`

echo "mv $module "${module_name}"_"${module_ver}"_"${module_kernel_ver}".ko"
mv $module "${module_name}"_"${module_ver}"_"${module_kernel_ver}".ko

#add for debug-info, to make ko smaller
if [ "$2"x == "cut"x ] ; then
	echo "dump-debuginfo "${module_name}"_"${module_ver}"_"${module_kernel_ver}".ko"
	dump-debuginfo "${module_name}"_"${module_ver}"_"${module_kernel_ver}".ko
fi







