#!/bin/bash

checkIP ( ) {
    if [ `echo $1 | grep "[0-9]*" ` ] && [ $1 -lt 255 ] && [ $1 -gt 0 ];then 
	    echo "Your parameter is OK"
    else 
	    echo "Error :Your parameter must be a interage less than 255,greater than 0"
	    exit 1
    fi
}

checkArgs ( ) {
    echo "Check your first parameter: "
    checkIP $1 

    echo "Check your second parameter: "
    checkIP $2 

    if [ $1 -gt $2 ];then 
        echo "Error : Your first parameter is greater than the second"
        exit 1
    fi

    if [ -d $3 ];then
        echo "Your third parameter is OK"
    else
        echo "Error : Your third parameter is not a directory!"
	    exit 1
    fi
}

createFile ( ) {
	read FileName
	while [ `echo $FileName | grep -c " "` -ne 0 ] || [ -f $FileName ];do
		echo "Your file name contains space or file already exist! Please input again"
		read FileName
	done
	touch $FileName
}

pingIP ( ) {
    StartIP=$1
    EndIP=$2
	echo "input the file name to save your result of ping command :"
	createFile
    while [ $StartIP -le $EndIP ]; do
		echo "now,ping $IpAddr.$StartIP"
        ping -c 4 $IpAddr.$StartIP > /dev/null
		if [ $? -ne 0 ];then
			echo "not connect"
		else
			echo "write ip address into file"
			echo "$IpAddr.$StartIP">>$FileName
		fi
        StartIP=`expr $StartIP + 1`
    done
}

compressFile ( ) {
	echo "input the compress file name :"
	read CompressFile
	while [ `echo $ComPressFile | grep -c " "` -ne 0 ] || [ -f $CompressFile ];do
		echo "Your file name contains space or file already exist! Please input again"
		read CompressFile
	done
	tar -zcf $CompressFile $1
	if [ $? -eq 0 ];then 
		echo "compress ok"
	else
		echo "the process of compression has some problem"
		exit 1
	fi
}

loginAsUser ( ) { 
	#send your compress file to this directory
	SendDir=/tmp
	
	if [ ! -s $FileName ];then 
		echo "No IP address is connect!"
		exit 0
	else 
		for IP in `cat $FileName`;do
			echo "Please input the user name to login($IP):"
			read LoginName
			scp $CompressFile $LoginName@$IP:$SendDir
		done
	fi

}

###### Main part #####

# IP address,it can be changed in your computer
IpAddr=49.140.62

# file to save the result of ping command
FileName=""

# file to send to every computer
CompressFile=""

if [ $# != 3 ];then
        echo "Error : You parameter has some problems,3 parameter required(int,int,directory)"
	    exit 1
fi
checkArgs $1 $2 $3
echo ""
pingIP $1 $2
echo ""
compressFile $3
echo ""
loginAsUser
