#!/usr/bin/env bash
#HOSTS=( "austin" "houston" "sanantonio" "indianapolis" "philly" )
HOSTS=( "austin" "houston" "sanantonio")
HOSTS=( "austin" "houston" "sanantonio" "indianapolis" "philly" "detroit" "baltimore" "chicago" "atlanta")
LOCAL_HOST=`hostname`
EXECUTABLES=( "armonia-sc" ) #"run-symmetric-sc.sh")
HOME_FOLDER="/home/user/armonia/src/Armonia"
DEST_FOLDER="/home/user/armonia-exec/src/Armonia"

cd $HOME_FOLDER
make
cd -
#for HOST in "${HOSTS[@]}"
#do
#    if [ "$HOST" != "$LOCAL_HOST" ]
#    then
#    	echo "Copying to $HOST:"
#	for EXEC in "${EXECUTABLES[@]}"
#	do
#	    scp ${HOME_FOLDER}/${EXEC} ${HOST}:${DEST_FOLDER}/${EXEC}
#	done
#    fi
#done

for EXEC in "${EXECUTABLES[@]}"
do
	#echo "${EXEC} copied to {${HOSTS[@]/$LOCAL_HOST}}"
	parallel scp ${HOME_FOLDER}/${EXEC} {}:${DEST_FOLDER}/${EXEC} ::: $(echo ${HOSTS[@]/$LOCAL_HOST})
	echo "${EXEC} copied to {${HOSTS[@]/$LOCAL_HOST}}"
done
