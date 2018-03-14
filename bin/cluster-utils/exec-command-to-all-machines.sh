#!/usr/bin/env bash
HOSTS=( "austin" "houston" "sanantonio" "indianapolis") # "philly" )
LOCAL_HOST=`hostname`
COMMAND="sudo useradd --groups root armonia && sudo passwd armonia"

for HOST in "${HOSTS[@]}"
do
    if [ "$HOST" != "$LOCAL_HOST" ]
    then
    	ssh $HOST '${COMMAND}'
    else
    	${COMMAND}
    fi
done
