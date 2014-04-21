#!/bin/sh

cd ~/openvswitch/openvswitch-2.0.0
sudo sh ./boot.sh
./configure --prefix=/usr --localstatedir=/var --with-linux=/lib/modules/`uname -r`/build
make -j4
sudo make install
make modules_install
/sbin/modprobe openvswitch
sudo rm -rf /usr/local/etc/openvswitch
mkdir -p /usr/local/etc/openvswitch
ovsdb-tool create /usr/local/etc/openvswitch/conf.db vswitchd/vswitch.ovsschema
sudo rm -rf /usr/etc/openvswitch
mkdir -p /usr/etc/openvswitch
ovsdb-tool create /usr/etc/openvswitch/conf.db vswitchd/vswitch.ovsschema
