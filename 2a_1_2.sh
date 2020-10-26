#!/bin/bash

function bin2Dec {
    bin=$1;
    dec=0;
    i=0;
    power=${#bin}-1;

    while [ $i -lt ${#bin} ];
    do
        curBit=${bin:$i:1};
        curPow=$((2 ** power));
        res=`expr $curBit \* $curPow`;
        dec=$((dec+res));

        let i+=1;
        let power-=1;
    done

    echo $((dec));
    # A shrtened method I found on the internet
    # echo "$((2#bin))"
}