#include <termios.h>
#include <iostream>
#include <string.h>
#include <unistd.h>
#include <libimobiledevice/libimobiledevice.h>
#include <libimobiledevice/lockdown.h>
#include <libimobiledevice/diagnostics_relay.h>
#include <plist/plist.h>
#include <libirecovery.h>

#ifndef _linuxiDeviceTools_commands
#define _linuxiDeviceTools_commands

bool recovery(diagnostics_relay_client_t diagnostics_client, lockdownd_client_t client);
char getch(void);
bool reboot(diagnostics_relay_client_t diagnostics_client, lockdownd_client_t client);

#endif