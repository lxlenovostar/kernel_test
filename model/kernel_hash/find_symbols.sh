#!/bin/bash

kernel_dir="$1"
sysmap_path="$1/System.map"
headfile="./ws_st_symbols.h"

func_name=(
	"tcp_v4_rcv"
)
func_num=${#func_name[@]}
find_func_addr() {
	#The symbols \< and \> respectively match the empty string at the beginning and end of a word. 
	func_addr=`cat $sysmap_path | grep "\<$1\>" | awk -F " " '{print $1}'`
	if [ -z "$func_addr" ] ; then
		echo "can't find func address : $1"
		exit 2
	else
		echo "find func address : $1 [0x$func_addr]"
		echo "#define $1_addr 0x$func_addr" >> $headfile
	fi
}

write_headfile_head() {
	echo "" >> $headfile
}
write_headfile_tail() {
	echo "" >> $headfile
}

if [ ! -d "$kernel_dir" ] || [ ! -f "$sysmap_path" ] ; then
	echo "can't find Symtep.map"
	exit 1
fi


true > $headfile
echo "#include <linux/skbuff.h>" >> $headfile
write_headfile_head
for ((i=0; i<$func_num; i++)) ; do
	find_func_addr ${func_name[$i]}
done
echo "static int (*tcp_v4_rcv_ptr)(struct sk_buff *skb) = (int (*)(struct sk_buff *skb))tcp_v4_rcv_addr;" >> $headfile
write_headfile_tail
echo "symbols address write to $headfile done!"
