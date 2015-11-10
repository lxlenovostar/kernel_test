#! /bin/sh

#build read/write file.
NUM=`cat /proc/cpuinfo  | grep "processor" | wc | awk '{print $1}'`

for ((i=0; i<$NUM; ++i))
do
     echo $i
	 dd if=/dev/zero of=/opt/wspackfile$i count=2097152 
done
