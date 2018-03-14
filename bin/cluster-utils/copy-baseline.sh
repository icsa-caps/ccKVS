#!/usr/bin/env bash
HOSTS=( "austin" "houston" "sanantonio" "indianapolis" "philly" )
HOSTS=( "austin" "houston" "sanantonio" "indianapolis" "philly" "detroit" "baltimore" "chicago" "atlanta")
LOCAL_HOST=`hostname`
EXECUTABLES=( "main" "run-symmetric.sh")
HOME_FOLDER="/home/user/armonia/src/herd-UD"
DEST_FOLDER="/home/user/armonia-exec/src/herd-UD"

cd $HOME_FOLDER
make
cd -
for HOST in "${HOSTS[@]}"
do
    if [ "$HOST" != "$LOCAL_HOST" ]
    then
    	echo "Copying to $HOST:"
	for EXEC in "${EXECUTABLES[@]}"
	do
	    scp ${HOME_FOLDER}/${EXEC} ${HOST}:${DEST_FOLDER}/${EXEC}
	done
    fi
done
