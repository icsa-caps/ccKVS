#!/usr/bin/env bash
#HOSTS=( "austin" "houston" "sanantonio" "indianapolis" "philly" )
#HOSTS=( "austin" "houston" "sanantonio")
#HOSTS=( "austin" "houston" "sanantonio" "chicago" "baltimore")
HOSTS=( "austin" "houston" "sanantonio" "indianapolis" "philly" "baltimore" "chicago" "atlanta" "detroit")
LOCAL_HOST=`hostname`
#EXECUTABLES=( "armonia-ec") #"run-symmetric-ec.sh" )
EXECUTABLES=( "test.txt") #"run-symmetric-ec.sh" )
HOME_FOLDER="/home/user/armonia/bin"
DEST_FOLDER="/home/user/armonia-exec/src/Armonia"

#echo ${HOSTS[@]/$LOCAL_HOST}
parallel scp ${HOME_FOLDER}/test.txt {}:${DEST_FOLDER}/${EXEC} ::: $(echo ${HOSTS[@]/$LOCAL_HOST})
