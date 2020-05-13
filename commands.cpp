#include "commands.hpp"

char getch(void)
{
    char buf = 0;
    struct termios old = {0};
    fflush(stdout);
    if (tcgetattr(0, &old) < 0)
        perror("tcsetattr()");
    old.c_lflag &= ~ICANON;
    old.c_lflag &= ~ECHO;
    old.c_cc[VMIN] = 1;
    old.c_cc[VTIME] = 0;
    if (tcsetattr(0, TCSANOW, &old) < 0)
        perror("tcsetattr ICANON");
    if (read(0, &buf, 1) < 0)
        perror("read()");
    old.c_lflag |= ICANON;
    old.c_lflag |= ECHO;
    if (tcsetattr(0, TCSADRAIN, &old) < 0)
        perror("tcsetattr ~ICANON");
    printf("\n");
    return buf;
}

bool reboot(diagnostics_relay_client_t diagnostics_client, lockdownd_client_t client)
{
    printf("Attempting to reboot...");
    if (diagnostics_relay_restart(diagnostics_client, DIAGNOSTICS_RELAY_ACTION_FLAG_WAIT_FOR_DISCONNECT) == DIAGNOSTICS_RELAY_E_SUCCESS)
    {
        printf("Restarting device!\n");
        return true;
    }
    else
    {
        printf("Failed to restart device.\n");
    }
    return false;
}

bool recovery(diagnostics_relay_client_t diagnostics_client, lockdownd_client_t client)
{
    irecv_client_t clients = NULL;
    plist_t value = NULL;

    if (LOCKDOWN_E_SUCCESS != lockdownd_get_value(client, NULL, "UniqueChipID", &value))
    {
        printf("Failed to get ecid!\n");
        return false;
    }
    uint64_t s;
    plist_get_uint_val(value, &s);

    printf("Attempting to enter recovery...");
    if (lockdownd_enter_recovery(client) == LOCKDOWN_E_SUCCESS)
    {
        printf("Sent device to recovery!\n");
        printf("Waiting for it to boot...");
        fflush(stdout);
        int time = 0;
        while (true)
        {
            if (time > 25)
            {
                printf("Took too long to boot, so when it turns on it will stay in recovery on reboot. You can exit recovery by plugging the device in and running this tool once its in recovery\n");
                return true;
            }
            unsigned long long ecid = s;

            irecv_error_t err = irecv_open_with_ecid(&clients, ecid);
            if (err == IRECV_E_SUCCESS)
            {
                printf("Entered!\n");

                err = irecv_setenv(clients, "auto-boot", "true");
                if (err != IRECV_E_SUCCESS)
                {
                    printf("Error: %s\nFailed to set autoboot, so when it turns on it will stay in recovery on reboot. You can exit recovery by plugging the device in and running this tool once its in recovery\n", irecv_strerror(err));
                }

                err = irecv_saveenv(clients);
                if (err != IRECV_E_SUCCESS)
                {
                    printf("Error: %s\nFailed to save env, so when it turns on it will stay in recovery on reboot. You can exit recovery by plugging the device in and running this tool once its in recovery\n", irecv_strerror(err));
                }

                return true;
            }
            else
            {
                time++;
                sleep(1);
            }
        }
    }
    else
    {
        printf("Failed to enter recovery.\n");
    }

    return false;
}