#!/bin/bash
function checkPal {
    string=$1;
    string=$(expr $string | tr '[:upper:]' '[:lower:]'); # to lowercase

    # Reverse the string
    len=${#string};
    i=0;
    rev="";
    while (( i<len )); do rev="${string:i++:1}$rev"; done
    
    if [ "$string" = "$rev" ];
    then
        echo 1;
    else
        echo 0;
    fi
}