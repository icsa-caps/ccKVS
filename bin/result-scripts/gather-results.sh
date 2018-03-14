#!/usr/bin/env bash
#HOSTS=( "austin" "houston" "sanantonio" "indianapolis" "philly" )
#HOSTS=( "austin" "houston" "sanantonio" "indianapolis" "philly" "detroit" "baltimore" "chicago" "atlanta")
HOSTS=( "austin" "houston" "sanantonio" "indianapolis" "philly" "baltimore" "chicago" "atlanta")

#HOSTS=( "austin" "houston" "sanantonio" "indianapolis" )
LOCAL_HOST=`hostname`
HOME_FOLDER="/home/user/armonia/results/scattered-results"
DEST_FOLDER="/home/user/armonia-exec/results/scattered-results"
PATTERN="*.csv"
PATTERN="EC_SKEW_CREW_s_0_a_99_v_32_m_5_c_16_w_10_r*-*.csv"
#PATTERN="EC_s_5_c_8_w_2_r_0*"

while getopts ":p" opt; do
    case $opt in
        p)
            PATTERN=$OPTARG
            ;;
        \?)
            echo "Invalid option: -$OPTARG" >&2
            exit 1
            ;;
        :)
            echo "Option -$OPTARG requires an argument" >&2
            exit 1
            ;;
    esac
done
shift $((OPTIND -1))


cd $HOME_FOLDER
cd -
for HOST in "${HOSTS[@]}"
do
    if [ "$HOST" != "$LOCAL_HOST" ]
    then
    	echo "Copying from $HOST:"
	scp  ${HOST}:${DEST_FOLDER}/${PATTERN} ${HOME_FOLDER} #/${PATTERN}
	echo "scp  ${HOST}:${DEST_FOLDER}/${PATTERN} ${HOME_FOLDER}" #/${PATTERN}
    fi
done
