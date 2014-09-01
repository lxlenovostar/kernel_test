#!/bin/sh
# pktgen.conf -- Sample configuration for send on two devices on a UP system

#modprobe pktgen

function pgset() {
	local result
	echo $1 > $PGDEV

	result=`cat $PGDEV | fgrep "Result: OK:"`	
	if [ "$result" = "" ]; then
		cat $PGDEV | fgrep Result:
	fi
}

function pg() {
	echo inject > $PGDEV
	cat $PGDEV
}

# On UP systems only one thread exists -- so just add devices 
# We use eth1, eth2

echo "Adding devices to run" 

PGDEV=/proc/net/pktgen/kpktgend_0
pgset "rem_device_all"
pgset "add_device eth0"
pgset "max_before_softirq 10000"

# Configure the individual devices
echo "Configuring devices"
PGDEV=/proc/net/pktgen/eth0
pgset "clone_skb 10"
pgset "pkt_size 60"
pgset "src_mac 00:50:56:AF:7F:C2"
#pgset "src_min 192.168.27.190"
pgset "src_min 10.0.0.1"
#pgset "src_max 192.168.27.190"
pgset "src_max 10.255.255.255"
#pgset "dst 10.10.10.3" 
pgset "dst 172.16.154.151"
pgset "dst_mac 00:50:56:AF:1A:AE"
pgset "udp_dst_min 53"
pgset "udp_dst_max 53"
pgset "count 5"

# Time to run

PGDEV=/proc/net/pktgen/pgctrl
echo "Running... ctrl^C to stop"
pgset "start"
echo "Done"

