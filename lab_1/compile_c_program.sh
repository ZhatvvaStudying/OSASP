#!/bin/bash
argsCount=$#
sourceFile=$1
outputFile=$2
if [ $argsCount -ne 2 ]
then
    echo -e "Only 2 args must be passed\n1 - source file name\n2 - output file name"
else
    if ! [ -f "$sourceFile" ]
    then
        echo "Source file doesen't exist"
    else
        gcc "$sourceFile" -o "$outputFile" && ./"$outputFile" 
    fi
fi