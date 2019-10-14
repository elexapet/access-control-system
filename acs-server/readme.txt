********************************************************************

This is readme for Access control system server (ACS server/master).

********************************************************************
Its main job is to respond to authentication requests from RFID readers connected on CAN bus.

The server is designed to be not accessible from ethernet network and be administred through ssh from outside
or from administrative application over secured channel.
Redis server should be on the same machine or network as it is designed to be accesed only from trusted clients
inside trusted environments.

------------------
Interactive usage:
------------------
    1. Read prerequisites
    2. Install python modules (pip module requred)
        python3 -m pip install -r requirements.txt --user
    3. Adjust parameters in start.sh
    4. Run script start.sh or run src/acs-server.py --help for more info

---------------------------------------------
Install permanently as service (for systemd init):
---------------------------------------------
    1. Read prerequisites
    2. Create system user for the acs service:
        sudo useradd --system acs-srvc
        sudo mkdir /var/log/acs-server
        sudo chown acs-srvc:acs-srvc /var/log/acs-server
    3. Install python modules system wide
        sudo python3 -m pip install -r requirements.txt
    4. Run build.sh (see parameters inside)
    5. Use bundled acs-server.service (you may need to adjust settings inside)
        sudo cp acs-server.service /lib/systemd/system/acs-server.service
    6. Enabble the service:
        sudo systemctl daemon-reload
        sudo systemctl enable acs-server
    7. Log file(s) will be in /var/log/acs-server

--------------
Prerequisites:
--------------
    * Linux with SocketCAN (tested on 3.13.0)
    * CAN 2.0 controller supported by SocketCAN
    * Python 3.5 or higher + pip
    * Redis database >= 2.0 (see below)
        - It is the the only implemented but support for others can be added.

----------------------------------------
Example steps for meeting prerequisites:
----------------------------------------

    CAN interface
        1. You may need to install driver if not included in linux kernel
        2. You may need to add CAN controller to your device tree (only if using ARM platform with real CAN hardware)
        3. Add setup of the CAN interface at startup -- add to /etc/network/interfaces:
            auto can0
            iface can0 can static
                            bitrate 125000
                            restart-ms 1000
                            txqueuelen 1000
        4. Reboot
        5. Connect CAN controller (USB-CAN, SPI-CAN, USB-UART-CAN, ...)

    Redis server setup example on port 6379 (steps for Debian distribution)
        * Add "vm.overcommit_memory = 1" to /etc/sysctl.conf
        * [optional] Disable transparent huge pages (improves latency). Add to /etc/rc.local following line:
            echo never > /sys/kernel/mm/transparent_hugepage/enabled
        * Compile and install Redis server (assumes /usr/local/bin in PATH)
            sudo apt-get install build-essential tcl
            wget http://download.redis.io/redis-stable.tar.gz
            tar xvzf redis-stable.tar.gz
            cd redis-stable
            make
            sudo make install
        * Use configuration and init script bundled with this server
            sudo mkdir /etc/redis
            sudo mkdir -p /var/redis/6379
            sudo cp ./redis_files/acs_redis_config.conf /etc/redis/redis_6379.conf
            sudo cp ./redis_files/asc_redis_init_script /etc/init.d/redis_6379
        * [recommended] Modify maxmemory option in redis_6379.conf to suit your system
        * Enable Redis to run at startup
            sudo chmod +x /etc/init.d/redis_6379
            sudo update-rc.d redis_6379 defaults
        * Allow connection to redis in firewall if on separate machine
        * [recommended] Secure redis by adding password to /etc/redis/redis_6379.conf if accessible remotely
        * Reboot
        * Redis database file will be in /var/redis/6379
        * Redis log file will be in /var/log/redis_6379.log
