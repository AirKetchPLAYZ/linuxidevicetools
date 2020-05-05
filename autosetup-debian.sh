rm -rf libirecovery/
git clone https://github.com/libimobiledevice/libirecovery.git
sudo apt-get install -y libtool automake autoconf libreadline-dev libusb-1.0-0 libusb-1.0-0-dev curl wget
cd libirecovery
./autogen.sh
make
sudo make install
echo "Installed libirecovery!"
cd ..
echo "Installing libimobiledevice!"
sudo apt-get install -y libimobiledevice6
echo "Installed!"
