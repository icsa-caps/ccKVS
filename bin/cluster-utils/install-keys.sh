#!/usr/bin/env bash
HOSTS=("baltimore")
#HOSTS=("baltimore" "detroit" "atlanta" "chicago")
#HOSTS=("philly" )
#HOSTS=( "austin" "houston" "sanantonio" "indianapolis" )
LOCAL_HOST=`hostname`
IDENTITY_FILE=/home/user/.ssh/id_rsa_$LOCAL_HOST
CONFIG=/home/user/.ssh/config
echo $IDENTITY_FILE
#ssh-keygen -t rsa -f $IDENTITY_FILE -q -N ""

for HOST in "${HOSTS[@]}"
do
    if [ "$HOST" != "$LOCAL_HOST" ]
    then
    	echo "Host $HOST" >> $CONFIG
    	echo "  User $USER" >> $CONFIG 
    	echo "  HostName $HOST" >> $CONFIG
    	echo "  IdentityFile $IDENTITY_FILE" >> $CONFIG
    	echo "" >> $CONFIG
	cat $IDENTITY_FILE.pub | ssh ${USER}@${HOST} "mkdir -p home/user/.ssh && cat >>  /home/user/.ssh/authorized_keys"
    fi
done
