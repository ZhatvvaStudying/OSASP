#!/bin/bash 
argsCount=$#
inpStr=$1
dirPath=$2
if [ $argsCount -ne 2 ]
then
    echo -e "Only 2 args must be passed\n1 - string to search\n2 - directory path"
else
    if ! [ -d "$dirPath" ]
    then
        echo "$dirPath isn't a directory"
    else
       find "$dirPath" -type f -exec grep -l "$inpStr" {} \; -exec wc -c {} \; | sort -n | grep ^[0-9]   
    fi
fi