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
#include <string.h>
#include <unistd.h>
#include <limits.h>
#include <termios.h>
#include <plist/plist.h>
#include <commands.hpp>

typedef bool (*Command)(diagnostics_relay_client_t, lockdownd_client_t);

using namespace std;
bool exit(diagnostics_relay_client_t diagnostics_client, lockdownd_client_t client)
{
    return true;
}
static const char *mode_to_str(int mode)
{
    switch (mode)
    {
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

void count(int seconds)
{
    int i = seconds;
    while (i > 0)
    {
        cout << i << "   \r";
        fflush(stdout);
        sleep(1);
        i = i - 1;
    }
    printf("done\n");
}

int main()
{

    if (geteuid())
    {
        printf("You need to run as root!\n");
        return -1;
    }
    printf("Connecting...\n");
    std::map<string, Command> funcs;
    funcs["reboot"] = reboot;
    funcs["exit"] = exit;
    funcs["recovery"] = recovery;

    int mode;
    char hostname[HOST_NAME_MAX];
    int hname;
    unsigned long long ecid = 0;
    idevice_info_t *dev_list = NULL;
    int i;
    char **id;
    idevice_t device = NULL;
    diagnostics_relay_client_t diagnostics_client = NULL;
    lockdownd_service_descriptor_t service = NULL;
    lockdownd_error_t ret = LOCKDOWN_E_UNKNOWN_ERROR;
    lockdownd_client_t lockdown_client = NULL;
    int returnCode = 0;
    const char *udid = NULL;
    irecv_client_t client = NULL;

    if (IDEVICE_E_SUCCESS != idevice_new(&device, udid))
    {
        printf("No device found, is it plugged in?\nAttempting to connect in recovery...");

        irecv_error_t err = irecv_open_with_ecid(&client, ecid);
        if (err != IRECV_E_SUCCESS)
        {
            printf("Error: %s\n", irecv_strerror(err));
            returnCode = -1;
            goto cleanup;
        }
        printf("Success!\n");

        irecv_get_mode(client, &mode);

        printf("Device is in %s mode.\n", mode_to_str(mode));
        switch (mode)
        {
        case IRECV_K_RECOVERY_MODE_1:
        case IRECV_K_RECOVERY_MODE_2:
        case IRECV_K_RECOVERY_MODE_3:
        case IRECV_K_RECOVERY_MODE_4:
            break;
        case IRECV_K_DFU_MODE:
            printf("Press ctrl-c to quit or any key to run checkra1n jailbreak (iPhone 4 - iPhone X)\n");
            getch();
            if (system("which checkra1n > /dev/null 2>&1"))
            {
                printf("Checkra1n is not installed or not in path\n");
                return -1;
            }
            else
            {
                printf("Starting checkra1n! Use CTRL-C to exit checkra1n\n");
                irecv_close(client);
                system("sudo checkra1n -c");
                return 0;
            }
            break;
        default:
            printf("This mode is unsupported at the moment, you can only use devices in normal, DFU or recovery mode\n");
            returnCode = -1;
            goto cleanup;
        }

        printf("There are 2 commands you can run; boot and dfuinstruct. CTRL-C to quit\n");

        while (true)
        {
            printf("> ");
            string input;
            cin >> input;

            if (input == string("boot"))
            {

                printf("Booting into normal iOS...");

                err = irecv_setenv(client, "auto-boot", "true");
                if (err != IRECV_E_SUCCESS)
                {
                    printf("Error: %s\n", irecv_strerror(err));
                    printf("Is your device connected?");
                    returnCode = -1;
                    goto cleanup;
                }

                err = irecv_saveenv(client);
                if (err != IRECV_E_SUCCESS)
                {
                    printf("Error: %s\n", irecv_strerror(err));
                    returnCode = -1;
                    goto cleanup;
                }

                err = irecv_reboot(client);
                if (err != IRECV_E_SUCCESS)
                {
                    printf("Error: %s\n", irecv_strerror(err));
                    returnCode = -1;
                    goto cleanup;
                }
                else
                {
                    printf("Success!\n");
                    goto cleanup;
                }
                returnCode = -1;
                goto cleanup;
            }
            else if (input == string("dfuinstruct"))
            {

                printf("This is a manual process and can be exited by holding the soft reboot buttons on your iDevice\nFollow these instructions:\n");
                printf("Instructions:\n");
                printf(R"(Iphone 7/X:
            ┏━━┓Power (1)
Vol. down(2)┃  ┃
            ┗━━┛

Iphone 6 and below:
            ┏━━┓Power (1)
            ┃  ┃
            ┗━━┛
            Home Button(2)
)");
                sleep(1);
                while (true)
                {
                    printf("Get ready...Place (not press) your fingers on the buttons for your device above...press any key to begin\n");
                    getch();
                    fflush(stdout);
                    printf("Hold button 1 (power button) and button 2 (home/vol down)...\n");
                    count(8);
                    printf("Release button 1 (power button) but keep holding button 2 (home/vol down)...\n");
                    count(8);
                    printf("Giving time for device to boot into DFU...\n");
                    count(5);
                    irecv_close(client);
                    client = NULL;
                    irecv_error_t err = irecv_open_with_ecid(&client, ecid);
                    if (err != IRECV_E_SUCCESS)
                    {
                        printf("Could not connect to check if device is in DFU! Maybe it got powered off or it is taking too long to boot.\n");
                        returnCode = -1;
                        goto cleanup;
                    }
                    err = irecv_get_mode(client, &mode);
                    if (err != IRECV_E_SUCCESS)
                    {
                        printf("Could not check if device is in DFU\n");
                        returnCode = -1;
                        goto cleanup;
                    }
                    if (mode == IRECV_K_DFU_MODE)
                    {
                        printf("Entered DFU mode successfully!\n");
                        break;
                    }
                    else
                    {
                        printf("Failed to enter DFU, try again!\n\n");
                    }
                }
                goto cleanup;
            }
            else
            {
                printf("Invalid command\n");
            }
        }
    }

    if (LOCKDOWN_E_SUCCESS != (ret = lockdownd_client_new_with_handshake(device, &lockdown_client, "liniostools")))
    {
        printf("ERROR: Could not connect to lockdownd, error code %d\nMake sure you press \"trust this pc\" on the prompt.\n", ret);
        returnCode = -1;
        goto cleanup;
    }
    ret = lockdownd_start_service(lockdown_client, "com.apple.mobile.diagnostics_relay", &service);
    if (ret != LOCKDOWN_E_SUCCESS)
    {
        printf("Error: could not start service, maybe your device is old?\nAttempting to use old lockdownd service");
        ret = lockdownd_start_service(lockdown_client, "com.apple.iosdiagnostics.relay", &service);
        if (ret != LOCKDOWN_E_SUCCESS)
        {
            printf("Failed to start new and old lockdownd service");
            returnCode = -1;
            goto cleanup;
        }
    }
    diagnostics_relay_client_new(device, service, &diagnostics_client);

    printf("Connected! Available commands:\nreboot, exit, recovery\n");
    while (true)
    {
        printf("> ");
        string input;
        cin >> input;
        if (idevice_get_device_list_extended(&dev_list, &i) < 0)
        {
            fprintf(stderr, "ERROR: Device not found, was it disconnected?\n");
            returnCode = -1;
            goto cleanup;
        }
        bool conn = false;
        for (i = 0; dev_list[i] != NULL; i++)
        {
            char *id;
            lockdownd_get_device_udid(lockdown_client, &id);
            if (string(id).compare(string(id)) == 0)
            {
                conn = true;
            }
        }
        idevice_device_list_extended_free(dev_list);
        if (!conn)
        {
            printf("ERROR: Original device seems to have been removed\n");
            returnCode = -1;
            goto cleanup;
        }
        if (funcs.find(input) != funcs.end())
        {
            if ((*funcs[input])(diagnostics_client, lockdown_client))
            {
                goto cleanup;
                break;
            }
        }
        else
        {
            printf("Invalid command\n");
        }
    }
cleanup:
    if (device != NULL)
        idevice_free(device);
    if (lockdown_client != NULL)
    {
        lockdownd_client_free(lockdown_client);
    }
    if (diagnostics_client != NULL)
    {
        diagnostics_relay_goodbye(diagnostics_client);
        diagnostics_relay_client_free(diagnostics_client);
    }
    if (service != NULL)
        lockdownd_service_descriptor_free(service);
    if (client != NULL)
        irecv_close(client);
    return returnCode;
}
