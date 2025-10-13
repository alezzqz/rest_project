# rest_project
Haribda Cluster Manager

## Install dependencies

1. boost 1.74 minimum
2. yaml-cpp library

## Building and installing

1. go to the project root folder
2. `$mkdir build`
3. `$cd build`
4. `$cmake ..`
5. `$make`
6. `$make install`

this will build and install cmagent and cmserver services into `/usr/sbin`
service files into `/etc/systemd/system`
config files into `/etc/cmagent` and `/etc/cmserver`

use `systemctl` to manage services
you can also run binaries locally using command line option `[start|stop]` but user should have permissions to store pid file to /var/run