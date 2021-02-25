#!/bin/bash


declare -A TIMES
for APPS in 800 1000 2000
do
ARRAY=()
    for VARIABLE  in {1..15};
    do
        
        start_time="$(date -u +%s.%N)"
        time ./run simple_platooning_scenario.ini  -c Highway_9Cells -u Cmdenv -r "\$realApp=$APPS";

        end_time="$(date -u +%s.%N)"

        elapsed="$(bc <<<"$end_time-$start_time")"
        ARRAY+=($elapsed)

    done
printf "num apps = $APPS: \n ${ARRAY[*]}\n\n" >> times.txt
done
