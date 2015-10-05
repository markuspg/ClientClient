# ClientClient

## Installation

First you need to compile *ClientClient*. *ClientClient* should always run in the graphical session of the experiment user on the clients. So it should be added to the autostart programs of this user.

Afterwards the configuration file has to be placed in the appropriate directory for the system. On *Linux* this directory is */etc/xdg/Economic Laboratory/*. On Windows the file has to be placed at *C:\\EcoLabLib\\ClientClient.conf*. Then the file should be edited according to the laboratory's setup. The configuration file is crucial for *ClientClient*'s functionality. If it is missing, *ClientClient* will not start connection attempts to *Labcontrol* and will therefore be useless.

*ClientClient* will try to connect to the server running *Labcontrol*. If there exists a firewall on your system, then the port specified via the *server_port* should not be blocked. Also the ports which shall be used by the z-Leaves should not be blocked.

## Building and Running on Windows

Install a recent *MinGW* version of *Qt* on your computer. Afterwards open the project file of *ClientClient* and build it in *Debug* mode (which is the default). *ClientClient* should run just fine from *Qt Creator*, only complaining on every start, since the necessary webcam URL as argument is not passed. If you want to run it normally you will have to put the following files from the *Qt* installation directory (e.g. C:\\Qt\\Qt5.4.2\\5.4\\mingw491\_32\\bin) into the same directory as the created executable:

icuin53.dll
icudt53.dll
icuuc53.dll
libgcc\_s\_dw2-1.dll
libstdc++-6.dll
libwinpthread-1.dll
Qt5Cored.dll
Qt5Networkd.dll
Qt5WebSocketsd.dll

The *d* at the end of the *Qt* dlls marks them as debug versions. If disk space and resources shall be saved, *ClientClient* can be built in *Release* mode. In this case, use the *Qt* files without the attached *d* (e.g. *Qt5Core.dll* instead of *Qt5Cored.dll*).