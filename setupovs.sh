#!/bin/sh

cd ~/openvswitch/openvswitch-2.0.0
sudo make install
make modules_install
sudo rmmod openvswitch
sudo insmod /datapath/linux/openvswitch.ko
/sbin/modprobe openvswitch
sudo rm -rf /usr/local/etc/openvswitch
mkdir -p /usr/local/etc/openvswitch
ovsdb-tool create /usr/local/etc/openvswitch/conf.db vswitchd/vswitch.ovsschema
sudo rm -rf /usr/etc/openvswitch
mkdir -p /usr/etc/openvswitch
ovsdb-tool create /usr/etc/openvswitch/conf.db vswitchd/vswitch.ovsschema

kill `cd /usr/local/var/run/openvswitch && cat ovsdb-server.pid ovs-vswitchd.pid`
ovsdb-server --remote=punix:/usr/local/var/run/openvswitch/db.sock \
                     --remote=db:Open_vSwitch,manager_options \
                     --private-key=db:SSL,private_key \
                     --certificate=db:SSL,certificate \
                     --bootstrap-ca-cert=db:SSL,ca_cert \
                     --pidfile --detach

ovs-vsctl --no-wait init
ovs-vswitchd --pidfile --detach
