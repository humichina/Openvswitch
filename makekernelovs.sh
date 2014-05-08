#!/bin/sh

kill `cd /usr/local/var/run/openvswitch && cat ovsdb-server.pid ovs-vswitchd.pid`
cd ~/openvswitch/openvswitch-2.0.0
sudo sh ./boot.sh
./configure --prefix=/usr --localstatedir=/var --with-linux=/lib/modules/`uname -r`/build
make -j4
