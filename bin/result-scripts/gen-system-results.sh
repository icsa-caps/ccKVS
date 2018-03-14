#!/usr/bin/env bash

INPUT_DIR="$(pwd)/../results/scattered-results"

OUTPUT_DIR="$(pwd)/../results/system-results"
#PATTERN="overTime_*_EREW_*_N_1250000000_o_40_w5_n_10000000*"
#PATTERN="BS_s_*_c_*_w_*_r_*_m_*"
PATTERN="EC_SKEW_CREW_s_0_a_99_v_32_m_5_c_16_w_10_r_0-*"
#PATTERN="EC_s_5_c_8_w_2_r_0_*"
# Each letter is an option argument, if it's followed by a collum 
# it requires an argument. The first colum indicates the '\?'
# help/error command when no arguments are given
while getopts ":p:o:" opt; do
  case $opt in
    p)
      PATTERN=$OPTARG
      ;;
    o)
      OUTPUT_DIR=$OPTARG
      ;;
   \?)
      echo "Invalid option: -$OPTARG" >&2
      exit 1
      ;;
    :)
      echo "Option -$OPTARG requires an argument." >&2
      exit 1
      ;;
  esac
done
shift $((OPTIND -1))


OUTPUT_FILENAME=$(echo "$PATTERN" | tr \* x)
FILES=""

if [[ $(ls -A ${INPUT_DIR}/${PATTERN}* 2> /dev/null) ]]; then
    rm -rf ${OUTPUT_DIR}/${OUTPUT_FILENAME}.csv
    echo Aggregating files for $PATTERN
    for filename in ${INPUT_DIR}/${PATTERN}*.csv; 
    do
        FILE_NAME=$(basename $filename)
        echo "file: $FILE_NAME" >> ${OUTPUT_DIR}/${OUTPUT_FILENAME}.csv
        cat $filename >> ${OUTPUT_DIR}/${OUTPUT_FILENAME}.csv
        FILES="$FILES $filename"
    done
    ./results-parser.py < ${OUTPUT_DIR}/${OUTPUT_FILENAME}.csv >> ${OUTPUT_DIR}/${OUTPUT_FILENAME}.csv
    #print files cat for debug
    #echo $FILES
fi
