This is readme for Access control system server (ACS server/master).

Its main job is to respond to authentication requests from RFID readers connected on CAN bus.

The server is designed to be not accessible from ethernet network and be administred through ssh from outside
or from administrative application over secured channel.
Redis server should be on the same machine or network as it is designed to be accesed only from trusted clients
inside trusted environments.

Interactive usage:
    1. Read prerequisites
    2. Install python modules
        python3 -m pip install -r requirements.txt --user
    3. Use script start.sh (see parameters inside) or run src/acs-server.py --help for more info

Install permanently as service (for systemd):
    1. Read prerequisites
    2. Create system user for the service:
        sudo useradd --system acs-srvc
        sudo mkdir /var/log/acs-server
        sudo chown acs-srvc:acs-srvc /var/log/acs-server
    3. Install python modules system wide
        sudo python3 -m pip install -r requirements.txt
    2. Run build.sh (see parameters inside)
    3. Use bundled acs-server.service (see parameters inside) and register as service:
        sudo cp acs-server.service /lib/systemd/system/acs-server.service
        sudo systemctl daemon-reload
        sudo systemctl enable acs-server
    4. log files will be in /var/log/acs-server

Prerequisites:
    Linux (with SocketCAN)
    CAN 2.0 controller supported by SocketCAN
    Python 3.5 or higher (see below for module install)
    Redis database (see below)
        - It is the only implemented but others can be added.

Example steps for meeting prerequisites:

    CAN interface
        * Connect CAN controller (USB-CAN, SPI-CAN, USB-UART-CAN, ...)
        * You may need driver if not included in linux kernel
        * You may need to add CAN controller to your device tree (only if using ARM platform with real CAN hardware)
        * Enable auto setup of the CAN interface at startup -- add to /etc/network/interfaces:
            auto can0
            iface can0 can static
                            bitrate 125000
                            restart-ms 200

    Redis database server setup example on port 6379 (steps for Debian distribution)
        * Add "vm.overcommit_memory = 1" to /etc/sysctl.conf
        * (optional) Disable transparent huge pages:
            echo never > /sys/kernel/mm/transparent_hugepage/enabled
        * Build and install Redis (assumes /usr/local/bin in PATH)
            sudo apt-get install build-essential tcl
            wget http://download.redis.io/redis-stable.tar.gz
            tar xvzf redis-stable.tar.gz
            cd redis-stable
            make
            sudo make install
        * Use configuration and init script bundled with this server
            sudo mkdir /etc/redis
            sudo mkdir -r /var/redis/6379
            sudo cp ./redis_files/acs_redis_config.conf /etc/redis/redis_6379.conf
            sudo cp ./redis_files/asc_redis_init_script /etc/init.d/redis_6379
        * (Optional) Modify maxmemory option in configuration file to suit your system
        * Enable Redis to run at startup
            sudo chmod +x /etc/init.d/redis_6379
            sudo update-rc.d redis_6379 defaults
        * Reboot
        * log files will be in /var/redis/6379
        * allow connection to redis in firewall settings if on separate machine
