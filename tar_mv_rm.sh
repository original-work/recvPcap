#! /bin/sh
# tar_mv_rm.sh
# Copyright(c) 2000, by WangXinXin.ShangHaiLiangJiang.China.
# All rights reserved.


quit=1
zipfile_name="2010.tar.gz"
folder_name=abc
dest_path="/root/MPM-IP/data_bak"
origi_path="/root/MPM-IP/data/eth0"



while getopts n:f:d:o:h options
do
        case $options in
        n) echo "zipfile_name == "
           echo $OPTARG
           zipfile_name=$OPTARG
           quit=0;;
        f) echo "folder_name == "
           echo $OPTARG
           folder_name=$OPTARG
           quit=0;;
        d) echo "dest_path == "
           echo $OPTARG
           dest_path=$OPTARG
	   quit=0;;
        o) echo "origi_path == "
           echo $OPTARG
           origi_path=$OPTARG
           quit=0;;
        h) echo
           echo "Usage: tar_mv_rm.sh -n zipfile_name -f folder_name -d dest_path -o origi_path"
           echo
           exit 0;;
        \?) echo
            echo "Usage: tar_mv_rm.sh -n zipfile_name -f folder_name -d dest_path -o origi_path"
            echo
            exit 1;;
        esac
done



if [ $quit = 1 ]
then
        echo
        echo "Usage: tar_mv_rm.sh -n zipfile_name -f folder_name -d dest_path -o origi_path"
        echo
        exit 1
fi


cd $origi_path
tar -czvf $zipfile_name $folder_name
mv $zipfile_name $dest_path
rm -rf $folder_name
