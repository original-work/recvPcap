#! /bin/sh
# run.sh
# Copyright(c) 2000, by WangXinXin.ShangHaiLiangJiang.China.
# All rights reserved.


cd /root/recvCap/
make -f Recv.make clean
make -f Recv.make
g++ -lrt -o timer_zip_mv_rm timer_zip_mv_rm.cpp
mkdir -p /root/MPM-IP/data_bak/eth0
mkdir -p /root/MPM-IP/data_bak/eth1
mkdir -p /root/MPM-IP/data_bak/eth2
mkdir -p /root/MPM-IP/data_bak/eth3
mkdir -p /root/MPM-IP/data_bak/eth4
chmod -R 777 *
./Recv eth0 3600
./Recv eth1 3600
./Recv eth2 3600
./Recv eth3 3600
./Recv eth4 3600
./timer_zip_mv_rm /root/MPM-IP/data/eth0/ 3600 /root/MPM-IP/data_bak/eth0/
./timer_zip_mv_rm /root/MPM-IP/data/eth1/ 3600 /root/MPM-IP/data_bak/eth1/
./timer_zip_mv_rm /root/MPM-IP/data/eth2/ 3600 /root/MPM-IP/data_bak/eth2/
./timer_zip_mv_rm /root/MPM-IP/data/eth3/ 3600 /root/MPM-IP/data_bak/eth3/
./timer_zip_mv_rm /root/MPM-IP/data/eth4/ 3600 /root/MPM-IP/data_bak/eth4/