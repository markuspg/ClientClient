# ClientClient

## Installation

First you need to compile *ClientClient*. *ClientClient* should always run in the graphical session of the experiment user on the clients. So it should be added to the autostart programs of this user.

Afterwards the configuration file has to be placed in the appropriate directory for the system. On *Linux* this directory is */etc/xdg/Economic Laboratory/*. On Windows the file has to be placed at *C:\\EcoLabLib\\ClientClient.conf*. Then the file should be edited according to the laboratory's setup. The configuration file is crucial for *ClientClient*'s functionality. If it is missing, *ClientClient* will not start connection attempts to *Labcontrol* and will therefore be useless.

*ClientClient* will try to connect to the server running *Labcontrol*. If there exists a firewall on your system, then the port specified via the *server_port* should not be blocked. Also the ports which shall be used by the z-Leaves should not be blocked.
