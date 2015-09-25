# ClientClient

## Installation

First you need to compile *ClientClient*. *ClientClient* should always run in the graphical session of the experiment user on the clients. So it should be added to the autostart programs of this user.

Afterwards the configuration file has to be placed in the appropriate directory for your system. On *Linux* this directory is */etc/xdg/Economic Laboratory/*. On Windows the file has to be placed at *C:\\EcoLabLib\\ClientClient.conf*. Then the file should be edited according to the laboratory's setup.

*ClientClient* will try to connect to the server running *Labcontrol*. If there exists a firewall on your system, then the port specified via the *server_port* should not be blocked. Also the ports which shall be used by the z-Leaves should not be blocked.

## Building and Running on Windows

Install a recent *MinGW* version of *Qt* on your computer. Afterwards open the project file of *ClientClient* and build it in *Debug* mode. *ClientClient* should run just fine from *Qt Creator*. If you want to run it normally you will have to put the following files from the *Qt* installation directory into the same directory as the created executable:

Qt5Cored.dll
libgcc\_s\_dw2-1.dll
libwinpthread-1.dll
libstdc++-6.dll
icuin53.dll
icuuc53.dll
icudt53.dll
Qt5Networkd.dll
Qt5WebSocketsd.dll
