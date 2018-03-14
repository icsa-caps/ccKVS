#!/usr/bin/env bash
HOSTS=( "austin" "houston" "sanantonio" )
LOCAL_HOST=`hostname`
EXECUTABLES=( "run-symmetric-sc.sh" )
HOME_FOLDER="/home/user/armonia/src/Armonia"
DEST_FOLDER="/home/user/armonia-exec/src/Armonia"
HOME_DBUG_FOLDER="/home/user/armonia/src/Armonia"
DEST_DBUG_FOLDER="/home/user/armonia-exec/debug"

TIMESTAMP=`date "+%Y_%m_%d-%H_%M_%S"`
echo "Timestamp: $TIMESTAMP"

cd $HOME_FOLDER
pwd
bash ${EXECUTABLES[0]} > ${HOME_DBUG_FOLDER}/${HOST}_${TIMESTAMP}.txt &
cd -
for HOST in "${HOSTS[@]}"
do
    
    if [ "$HOST" != "$LOCAL_HOST" ]
    then
	for EXEC in "${EXECUTABLES[@]}"
	do
	    echo "Running $EXEC to $HOST"
	    COMMAND="cd ${DEST_FOLDER}; bash ${EXEC}"
	    ssh -t $HOST "$COMMAND > ${DEST_DBUG_FOLDER}/${HOST}_${TIMESTAMP}.txt &"
	done
    fi
done
