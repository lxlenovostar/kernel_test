#! /bin/sh
chkconfig kdump on 
service kdump start
echo "1" > /proc/sys/kernel/softlockup_panic 
