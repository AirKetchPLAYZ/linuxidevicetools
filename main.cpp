/*
 * Name: linuxidevicetools
 * Author: airketchupplayz on github
 * Author's name: Josh Lausch
 * This project is licensed under the MIT license. If you do not recieve a
 * copy of the license with this software, it can be viewed at
 * it can be viewed at https://pastebin.com/bWC8qta4
 *


              @@@@@@@@@@@@@
          @@@@@           @@@@@
        @@@                   @@@
      @@@                       @@@    Copyright Joshua Lausch,
     @@@       @@@@@@@@@@@       @@@   2020
    @@@      @@@                  @@@
    @@@      @@                   @@@
    @@@      @@@                  @@@
     @@@       @@@@@@@@@@@       @@@
      @@@                       @@@
        @@@                   @@@
          @@@@@           @@@@@
              @@@@@@@@@@@@@

 */


#include <libimobiledevice/libimobiledevice.h>
#include <libimobiledevice/lockdown.h>
#include <libimobiledevice/diagnostics_relay.h>
#include <libirecovery.h>
#include <map>
#include <cassert>
#include <iostream>
#include <string.h>
#include <unistd.h>

typedef bool (*Command) (diagnostics_relay_client_t, lockdownd_client_t);

using namespace std;

bool reboot(diagnostics_relay_client_t diagnostics_client, lockdownd_client_t client) {
    printf("Attempting to reboot...");
    if (diagnostics_relay_restart(diagnostics_client, DIAGNOSTICS_RELAY_ACTION_FLAG_WAIT_FOR_DISCONNECT) == DIAGNOSTICS_RELAY_E_SUCCESS) {
        printf("Restarting device!\n");
        return true;
    } else {
        printf("Failed to restart device.\n");
    }
    return false;
}
bool exit(diagnostics_relay_client_t diagnostics_client, lockdownd_client_t client) {
    return true;
}
bool recovery(diagnostics_relay_client_t diagnostics_client, lockdownd_client_t client) {
    printf("Attempting to enter recovery...");
    if (lockdownd_enter_recovery(client) == LOCKDOWN_E_SUCCESS) {
        printf("Entered recovery!\n");
        return true;
    } else {
        printf("Failed to enter recovery.\n");
    }

    return false;
}
static const char* mode_to_str(int mode) {
    switch (mode) {
        case IRECV_K_RECOVERY_MODE_1:
        case IRECV_K_RECOVERY_MODE_2:
        case IRECV_K_RECOVERY_MODE_3:
        case IRECV_K_RECOVERY_MODE_4:
            return "Recovery";
            break;
        case IRECV_K_DFU_MODE:
            return "DFU";
            break;
        case IRECV_K_WTF_MODE:
            return "WTF";
            break;
        default:
            return "Unknown";
            break;
    }
}
int main() {

    if (geteuid()) {
        printf("You need to run as root!\n");
        return -1;
    }
    printf("Connecting...\n");
    std::map<string, Command> funcs;
    funcs["reboot"] = reboot;
    funcs["exit"] = exit;
    funcs["recovery"] = recovery;

    unsigned long long ecid = 0;
    idevice_info_t *dev_list = NULL;
    int i;
    irecv_error_t error;
    idevice_t device = NULL;
    diagnostics_relay_client_t diagnostics_client = NULL;
    lockdownd_service_descriptor_t service = NULL;
    lockdownd_error_t ret = LOCKDOWN_E_UNKNOWN_ERROR;
    lockdownd_client_t lockdown_client = NULL;
    int returnCode = 0;
    const char *udid = NULL;
    irecv_client_t client = NULL;


    if (IDEVICE_E_SUCCESS != idevice_new(&device, udid)) {
        printf("No device found, is it plugged in?\nAttempting to connect in recovery...");

        irecv_error_t err = irecv_open_with_ecid(&client, ecid);
        if (err != IRECV_E_SUCCESS) {
            printf("Error: %s\n", irecv_strerror(error));
            returnCode = -1;
            goto cleanup;
        }
        printf("Success!\n");
        int mode;
        irecv_get_mode(client, &mode);

        printf("Device is in %s mode.\n", mode_to_str(mode));
        switch (mode) {
            case IRECV_K_RECOVERY_MODE_1:
            case IRECV_K_RECOVERY_MODE_2:
            case IRECV_K_RECOVERY_MODE_3:
            case IRECV_K_RECOVERY_MODE_4:
                break;
            default:
                printf("This mode is unsupported at the moment, we can only use devices in normal or recovery mode\n");
                returnCode = -1;
                goto cleanup;
        }


        printf("Press enter to boot into normal mode or CTRL-C to quit\n");
        cin.get();
        printf("Booting into normal iOS...");

        error = irecv_setenv(client, "auto-boot", "true");
        if (error != IRECV_E_SUCCESS) {
            printf("Error: %s\n", irecv_strerror(error));
            printf("Is your device connected?");
            returnCode = -1;
            goto cleanup;
        }

        error = irecv_saveenv(client);
        if (error != IRECV_E_SUCCESS) {
            printf("Error: %s\n", irecv_strerror(error));
            returnCode = -1;
            goto cleanup;
        }

        error = irecv_reboot(client);
        if (error != IRECV_E_SUCCESS) {
            printf("Error: %s\n", irecv_strerror(error));
            returnCode = -1;
            goto cleanup;
        } else {
            printf("Success!\n");
            goto cleanup;
        }
        returnCode = -1;
        goto cleanup;

    }

    if (LOCKDOWN_E_SUCCESS != (ret = lockdownd_client_new_with_handshake(device, &lockdown_client, "liniostools"))) {
        printf("ERROR: Could not connect to lockdownd, error code %d\nMake sure you press \"trust this pc\" on the prompt.\n", ret);
        returnCode = -1;
        goto cleanup;
    }
    ret = lockdownd_start_service(lockdown_client, "com.apple.mobile.diagnostics_relay", &service);
    if (ret != LOCKDOWN_E_SUCCESS) {
        printf("Error: could not start service, maybe your device is old?\nAttempting to use old lockdownd service");
        ret = lockdownd_start_service(lockdown_client, "com.apple.iosdiagnostics.relay", &service);
        if (ret != LOCKDOWN_E_SUCCESS) {
            printf("Failed to start new and old lockdownd service");
            returnCode = -1;
            goto cleanup;
        }
    }
    diagnostics_relay_client_new(device, service, &diagnostics_client);

    printf("Connected! Available commands:\nreboot, exit, recovery\n");
    while (true) {
        printf("> ");
        string input;
        cin >> input;
        if (idevice_get_device_list_extended(&dev_list, &i) < 0) {
            fprintf(stderr, "ERROR: Device not found, was it disconnected?\n");
            returnCode = -1;
            goto cleanup;

        }
        bool conn = false;
        for (i = 0; dev_list[i] != NULL; i++) {
            char *id;
            lockdownd_get_device_udid(lockdown_client, &id);
            if (string(id).compare(string(id)) == 0) {
                conn = true;
            }
        }
        idevice_device_list_extended_free(dev_list);
        if (!conn) {
            printf("ERROR: Original device seems to have been removed\n");
            returnCode = -1;
            goto cleanup;
        }
        if (funcs.find(input) != funcs.end()) {
            if ((*funcs[input])(diagnostics_client, lockdown_client)) {
                goto cleanup;
                break;
            }
        } else {
            printf("Invalid command\n");
        }
    }
    cleanup:
    if (device != NULL)
        idevice_free(device);
    if (lockdown_client != NULL) {
        lockdownd_client_free(lockdown_client);
    }
    if (diagnostics_client != NULL) {
        diagnostics_relay_goodbye(diagnostics_client);
        diagnostics_relay_client_free(diagnostics_client);
    }
    if (service != NULL)
        lockdownd_service_descriptor_free(service);
    if (client != NULL)
        irecv_close(client);
    return returnCode;
}
