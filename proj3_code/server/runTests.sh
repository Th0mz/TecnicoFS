#!/bin/bash

numArgs=3

# Check if the number of arguments is valid
if [ $# -ne $numArgs ]
then
	echo Error : Invalid number of arguments
	exit 1
fi

# Check if the input folder exists
if [ ! -d $1 ]
then
	echo Error : The file ${1}/ do not exist
	exit 1 
fi

# Check if the output folder exists
if [ ! -d $2 ]
then
	echo Error : The file ${2}/ do not exist
	exit 1
fi

# Check if the max number of threads if a valid number
#	Check if it is and integer
re='^[0-9]+$'
if ! [[ $3 =~ $re ]]
then
	echo Error : maxThreads is invalid, $3 must be an integer
	exit 1
fi

#	Check if it is grather than or equals 1
if [ $3 -lt 1 ]
then
	echo Error : maxThreads is invalid, $3 must be \>=  1
	exit 1
fi

# Check if tecnicofs exists and is executable
if [ ! -x tecnicofs ]
then
	echo Error : tecnicofs do not exist
	exit 1
fi

inputfile=$1
outputfile=$2
maxthreads=$3

echo
echo \ \##### Tests \#####
for file in $(ls $inputfile)
do
	for numthreads in $(seq 1 $maxthreads)
	do
		# Check if the file is not a folder
		if [ ! -d $inputfile/$file ]
		then
			filename=${file//.txt}
			echo InputFile=$file NumThreads=$numthreads
			./tecnicofs ${inputfile}/${file} ${outputfile}/${filename}-${numthreads}.txt ${numthreads} | grep "TecnicoFS completed in"
			echo
		fi
	done
done

