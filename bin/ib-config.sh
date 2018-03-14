#!/usr/bin/env bash
echo 8192 > /sys/devices/system/node/node0/hugepages/hugepages-2048kB/nr_hugepages
echo 10000000001 > /proc/sys/kernel/shmall
echo 10000000001 > /proc/sys/kernel/shmmax
ifconfig ib0 129.215.165.8  netmask 255.255.254.0
/etc/init.d/opensmd start 
cat /sys/devices/system/node/node0/hugepages/hugepages-2048kB/nr_hugepages
#PCIe counter settings
echo 0 > /proc/sys/kernel/nmi_watchdog
modprobe msr
