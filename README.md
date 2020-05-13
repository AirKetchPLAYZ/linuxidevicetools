# linuxidevicetools
A command line utility to interact with ios devices over usb. Requires libimobiledevice and libirecovery, which are under different licenses to this and is therefore NOT provided. You must agree to the license and install them yourself.


# Setup instructions:  
Debian:  
Go to the releases, download the linuxidevicetools file and autosetup.sh then run `./autosetup.sh` in that directory in your terminal emulator of choice and it will automatically setup all the dependencies.  
Other OS's (This doesnt work on windows, there are seperate tools for windows like iMazing. Windows support may be added at a later date):  
Follow the instructions to install libimobiledevice on your os. There is a chance it is already installed.  
Install https://github.com/libimobiledevice/libirecovery (if it is not in your package manager install it from source as per its readme.md) then download the linuxidevicetools file from the releases.  

Now (on both OSes) open your terminal emulator of choice and go into the directory of the downloaded `./linuxidevicetools` file. Run it with `sudo ./linuxidevicetools` and enter your password if prompted to allow the software to interact with your device.  
