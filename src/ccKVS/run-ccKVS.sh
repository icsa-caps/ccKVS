#!/usr/bin/env bash

is_RoCE=0
executable="ccKVS-sc" # either "ccKVS-sc" or "ccKVS-lin"

# Setting up a unique machine id via a list of all ip addresses
machine_id=-1
allIPs=(129.215.165.8 129.215.165.7 129.215.165.9 129.215.165.6 129.215.165.5 129.215.165.3 129.215.165.4 129.215.165.2 129.215.165.1)
localIP=$(ip addr | grep 'state UP' -A2 | sed -n 3p | awk '{print $2}' | cut -f1  -d'/')
for i in "${!allIPs[@]}"; do
	if [  "${allIPs[i]}" ==  "$localIP" ]; then
		machine_id=$i
	else
    remoteIPs+=( "${allIPs[i]}" )
	fi
done

# machine_id = # uncomment this line to manually set the machine id
echo Machine-Id "$machine_id"

export MEMCACHED_IP="129.215.165.8" #Node having memcached for to initialize RDMA QPs connections/handlers
export MLX5_SINGLE_THREADED=1
export MLX5_SCATTER_TO_CQE=1


# A function to echo in blue color
function blue() {
	es=`tput setaf 4`
	ee=`tput sgr0`
	echo "${es}$1${ee}"
}

blue "Removing SHM keys used by the workers 24 -> 24 + Workers_per_machine (request regions hugepages)"
for i in `seq 0 32`; do
	key=`expr 24 + $i`
	sudo ipcrm -M $key 2>/dev/null
done

# free the  pages workers use
blue "Removing SHM keys used by MICA"
for i in `seq 0 28`; do
	key=`expr 3185 + $i`
	sudo ipcrm -M $key 2>/dev/null
	key=`expr 4185 + $i`
	sudo ipcrm -M $key 2>/dev/null
done

: ${MEMCACHED_IP:?"Need to set MEMCACHED_IP non-empty"}


blue "Removing hugepages"
shm-rm.sh 1>/dev/null 2>/dev/null


blue "Reset server QP registry"
sudo killall memcached
sudo killall ccKVS-sc
memcached -l 0.0.0.0 1>/dev/null 2>/dev/null &
sleep 1

blue "Running client and worker threads"
sudo LD_LIBRARY_PATH=/usr/local/lib/ -E \
	./${executable} \
	--machine-id $machine_id \
	--is-roce $is_RoCE \
	2>&1
