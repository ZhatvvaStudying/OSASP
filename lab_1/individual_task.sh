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
       grep -rl "$inpStr" "$dirPath" | xargs -d '\n' ls -lSr | awk '{print $5,$9}'
       #find "$dirPath" -exec grep -rl "$inpStr" {} \; | xargs -d '\n' ls -lSr | awk '{print $5,$9}'  
       #find "$dirPath" \( grep "$inpStr" {} \) | xargs -d '\n' ls -lSr | awk '{print $5,$9}'   
    fi
fi