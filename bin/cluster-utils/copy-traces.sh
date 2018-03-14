#!/usr/bin/env bash
#HOSTS=( "austin" "houston" "sanantonio" "indianapolis" "philly" )
HOSTS=( "austin" "houston" "sanantonio" )
HOSTS=( "austin" "houston" "sanantonio" "indianapolis" "philly" "detroit" "baltimore" "chicago" "atlanta")
LOCAL_HOST=`hostname`
TRACE_PREFIX="s_"
HOME_FOLDER="/home/user/armonia/traces/"
DEST_FOLDER="/home/user/armonia-exec/traces"

for HOST in "${HOSTS[@]}"
do
    if [ "$HOST" != "$LOCAL_HOST" ]
    then
    	echo "Copying to $HOST:"
	scp ${HOME_FOLDER}/${TRACE_PREFIX}* ${HOST}:${DEST_FOLDER}/
    fi
done
