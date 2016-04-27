#!/bin/bash

set -e -u -x

echo "Installing 'ClientClient' and all its dependencies"
sudo dpkg --add-architecture i386
sudo apt update
sudo apt install build-essential libqt5websockets5-dev lightdm git psmisc openbox qt5-default util-linux wine-development xorg

echo "Cloning the relevant Github repositories 'ClientClient' and 'HelpMessageSender'"
git clone https://github.com/markuspg/ClientClient
git clone https://github.com/markuspg/HelpMessageSender

echo 'Building and installing the programs'
mkdir -p build-ClientClient
cd build-ClientClient
qmake  ../ClientClient/
make
sudo cp ClientClient /usr/local/bin
cd ..
mkdir -pv build-HelpMessageSender
cd build-HelpMessageSender
qmake  ../HelpMessageSender/
make
sudo cp HelpMessageSender /usr/local/bin
cd ..

echo 'Creating a dedicated user for the experiments'
read -p '  Please enter the name the experimenter user shall have: ' userName
read -p '  Please enter the UID the experimenter user shall have (>1000): ' userUID
sudo adduser --uid "$userUID" "$userName"

echo "Patching 'LightDM'"

echo "After finish of this script please copy all 'zleaf.exe' versions to separate"
echo "subdirectories of '/opt/z-Leaves/' following the naming scheme"
echo "'/opt/z-Leaves/zTree_X.Y.Z/zleaf.exe' or '/opt/z-Leaves/zTree_X.Y.ZZ/zleaf.exe'"
echo "This hierarchy should match the one on the server exactly."
sudo mkdir -p /opt/z-Leaves

echo 'Cleaning up'
rm -rf build-ClientClient
rm -rf ClientClient
rm -rf build-HelpMessageSender
rm -rf HelpMessageSender
