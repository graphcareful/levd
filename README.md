# levd

Daemon with configurable properties that will control NZXT Kraken x61 
liquid cooler.

### Dependencies

This project depends on glog, yaml-cpp and libusb. On fedora:

```
$ sudo dnf install yaml-devel glog-devel libusb-devel
```

### Building

To compile this project ensure you have an up-to-date cpp compiler toolchain 
that supports c++ 14. Also ensure you have the cmake build tool. To build:

```
$ mkdir build && cd build
$ cmake ..
$ make
$ sudo make install # Optionally
```

### Installing

Using `make install` will install the program and its necessary config files
into the filesystem. You'll need to run this command as sudo, as the installer
will copy files into directories outside of your home directory. Here's what 
the output of `make install` should look like:

```
[100%] Built target kraken
Install the project...
-- Install configuration: ""
-- Installing: /usr/bin/kraken
-- Up-to-date: /usr/bin
-- Installing: /usr/bin/kraken-start.sh
-- Installing: /etc/leviathan
-- Installing: /etc/leviathan/levd.cfg
-- Up-to-date: /etc/systemd/system
-- Installing: /etc/systemd/system/levd.service
```

### Setting up for use as daemon

Now that all of the necessary files have been copied into their respecive locations
(also with the correct permissions), you'll have to let systemctl be aware of the
modifications we're introducing. Follow the examples below:

```
$ sudo systemctl daemon-reload # Use this whenever levd.service changes
$ sudo systemctl enable levd 
$ sudo systemctl start levd # stop sends SIGKILL to process, gracefully terminating
```

### Configuration

The program must see a valid `levd.cfg` file located in `/etc/leviathan`. A sample can
be found in the `config/` folder. Your configuration file must be in yaml format and 
contain at least the `fan_profile`, `enable_color`, and `main_color` properties. Later
properties are not supported at this moment.

To set a fan curve, add to the `fan_profile` list, other lists of size two.
These are data points which build your fan profile curve - x value being CPU temp 
(in C) and y value being fan percentage (in factors of 5, 30 being lowest, 100 highest).

Eg:
```
fan_profile: 
  - 
    - 30
    - 30
  -
    - 35
    - 40
  -
    - 40
    - 60
  -
    - 42
    - 70
  -
    - 43
    - 80
  -
    - 45
    - 100
```

At a CPU temperature of 30C the fan will operate at its lowest value, 30%. At a reading
of 37C the program will perform the necessary calculations to find the fan value 48% - 
then rounding up to the nearest multiple of 5, being 50%. All of this happens every 0.5s.

Also real-time updates to the `levd.cfg` file are supported. No need to relaunch the daemon
every time you modify a property.

The future project `levd-settings` will allow you to modify the `levd.cfg` file using a 
GUI using the Qt framework.


### Can I get some logs?

Sure, using journalctl you can see the programs stderr/stdout logs.

```
$ sudo journalctl -f -u levd.service
[sudo] password for [user]: 
-- Logs begin at Wed 2017-08-16 15:57:01 EDT. --
Aug 16 20:18:13 localhost.localdomain systemd[1]: Started Leviathan - Daemon for Kraken x61 Watercooler.
Aug 16 20:31:02 localhost.localdomain kraken-start.sh[22349]: I0816 20:31:02.675717 22349 main.cpp:20] There are 15 usb devices hooked up
Aug 16 20:31:02 localhost.localdomain kraken-start.sh[22349]: I0816 20:31:02.675858 22349 main.cpp:24] Kraken X61 is detected
Aug 16 20:31:02 localhost.localdomain kraken-start.sh[22349]: I0816 20:31:02.675865 22349 main.cpp:25] Starting levd service...
Aug 16 20:31:02 localhost.localdomain kraken-start.sh[22349]: I0816 20:31:02.676039 22349 leviathan_service.cpp:68] Kraken Driver Initialized
Aug 16 20:31:02 localhost.localdomain kraken-start.sh[22349]: I0816 20:31:02.676582 22349 leviathan_service.cpp:69] Kraken Serial No: CCVI_1.0
Aug 16 20:31:02 localhost.localdomain kraken-start.sh[22349]: I0816 20:31:02.677431 22349 leviathan_service.cpp:100] Detected modifications to config file, updating
 preferences...
Aug 16 20:31:02 localhost.localdomain kraken-start.sh[22349]: I0816 20:31:02.925264 22349 leviathan_service.cpp:118] Changed fan speed to: 1020
...

...
Aug 16 20:36:56 localhost.localdomain systemd[1]: Stopping Leviathan - Daemon for Kraken x61 Watercooler...
Aug 16 20:36:57 localhost.localdomain kraken-start.sh[22349]: I0816 20:36:57.116379 22349 main.cpp:31] ... driver gracefully shutting down
Aug 16 20:36:57 localhost.localdomain systemd[1]: Stopped Leviathan - Daemon for Kraken x61 Watercooler.
```

And running `systemctl status levd` can give you quick insight into the status of the daemon
```
$ sudo systemctl status levd.service
● levd.service - Leviathan - Daemon for Kraken x61 Watercooler
   Loaded: loaded (/etc/systemd/system/levd.service; enabled; vendor preset: disabled)
   Active: active (running) since Wed 2017-08-16 20:37:19 EDT; 46s ago
 Main PID: 22870 (kraken)
    Tasks: 2 (limit: 4915)
   CGroup: /system.slice/levd.service
```

### Thank you to...

https://github.com/jaksi/leviathan 

As I was reverse engineering the windows driver, I later found out that someone else did a way 
better job then me :) 

This project would not have been possible without contributions to the open source community from
jaksi.

