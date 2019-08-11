This is Access control system server (ACS server/master).

Its main job is to respond to authentication requests from access panels connected on CAN bus.

Usage:
    1. Install requirements
    2. Use start.sh to run

    or

    1. Install requirements
    2. Create system user for service:
        sudo useradd --system acs-srvc
        sudo mkdir /var/log/acs-server
        sudo chown acs-srvc:acs-srvc /var/log/acs-server
    3. Install python packages system wide
        sudo python3 -m pip install -r requirements.txt
    2. Run build.sh
    3. Use bundled acs-server.service and register as service:
        sudo cp acs-server.service /lib/systemd/system/acs-server.service
        sudo systemctl daemon-reload
        sudo systemctl enable acs-server
    4. log files will be in /var/log/acs-server

Requirements:
    Linux (with SocketCAN)
    a CAN interface (see below)
    Python 3.5 or higher (see below)
    Redis server (see below)

Python modules
    python3 -m pip install -r requirements.txt

CAN setup
    * Connect CAN controller
    * Add CAN controller to your device tree (if using real CAN interface and not virtual)
    * Setup the CAN interface at startup - add to /etc/network/interfaces:
        auto can0
        iface can0 can static
                        bitrate 125000
                        restart-ms 200

Redis server setup on port 6379 (for Debian):
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
    * Use configurlsation and init script bundled with this server
        sudo mkdir /etc/redis
        sudo mkdir -r /var/redis/6379
        sudo cp ./redis_files/acs_redis_config.conf /etc/redis/redis_6379.conf
        sudo cp ./redis_files/asc_redis_init_script /etc/init.d/redis_6379
    * (Optional) Modify maxmemory option in configuration file to your system
    * Enable Redis to run at startup
        sudo chmod +x /etc/init.d/redis_6379
        sudo update-rc.d redis_6379 defaults
    * Reboot
    * log files will be in /var/redis/6379
    * allow connection to redis in firewall settings
