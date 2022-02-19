#!/bin/bash
argsCount=$#
outputFile=$1
inputDir=$2
fileExtension=$3
if [ $argsCount -ne 3 ]
then
    echo -e "Only 3 args must be passed\n1 - output file name\n2 - input directory\n3 - file extension to search"
else
    if ! [ -d "$inputDir" ] 
    then 
        echo "$inputDir isn't a directory" >&2
    else
        find "$inputDir" -name "*.$fileExtension" | sort -f > "$outputFile" 
    fi
fi