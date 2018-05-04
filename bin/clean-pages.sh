#!/usr/bin/env bash

# blue "Removing SHM keys used by the workers 24 -> 24 + Workers_per_machine (request regions hugepages)"
for i in `seq 0 40`; do
	key=`expr 24 + $i`
	sudo ipcrm -M $key 2>/dev/null
done

# free the  pages workers use

# blue "Removing SHM keys used by MICA"
for i in `seq 0 28`; do
	key=`expr 3185 + $i`
	sudo ipcrm -M $key 2>/dev/null
	key=`expr 4185 + $i`
	sudo ipcrm -M $key 2>/dev/null
done

: ${MEMCACHED_IP:?"Need to set MEMCACHED_IP non-empty"}


# blue "Removing hugepages"

shm-rm.sh 1>/dev/null 2>/dev/null

# blue "Reset server QP registry"
sudo killall memcached
memcached -l 0.0.0.0 1>/dev/null 2>/dev/null &
