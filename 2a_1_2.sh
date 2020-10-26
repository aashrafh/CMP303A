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

function dec2Bin {
    dec=$1;
    bin=0;
    i=0;

    while [ $dec -ne 0 ];
    do
        rem=`expr $dec % 2`;
        power=$((10 ** i));
        res=`expr $rem \* $power`;
        bin=$((bin+res));
        dec=`expr $dec \/ 2`;

        let i+=1;
    done

    echo $((bin));
}