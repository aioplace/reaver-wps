/*
 * Reaver - Main and usage functions
 * Copyright (c) 2011, Tactical Network Solutions, Craig Heffner <cheffner@tacnetsol.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * See README and LICENSE for more details.
 */

#include "wpscrack.h"

int main(int argc, char **argv)
{
	int ret_val = EXIT_FAILURE;
	time_t start_time = 0, end_time = 0;
	struct wps_data *wps = NULL;

	globule_init();
	init_default_settings();

	if(argc < 2)
	{
		ret_val = usage(argv[0]);
		goto end;
	}

	/* Process the command line arguments */
	if(process_arguments(argc, argv) == EXIT_FAILURE)
	{
		ret_val = usage(argv[0]);
		goto end;
	}

	/* Double check usage */
	if(!get_iface() || (memcmp(get_bssid(), NULL_MAC, MAC_ADDR_LEN) == 0))
	{
		usage(argv[0]);
		goto end;
	}

	/* If no MAC address was provided, get it ourselves */
	if(memcmp(get_mac(), NULL_MAC, MAC_ADDR_LEN) == 0)
	{
		if(!read_iface_mac())
		{
			fprintf(stderr, "Failed to retrieve a MAC address for interface '%s'!\n", get_iface());
			goto end;
		}
	}

	/* Sanity checking on the message timeout value */	
	if(get_m57_timeout() > M57_MAX_TIMEOUT) 
	{
		set_m57_timeout(M57_MAX_TIMEOUT);
	}
	else if(get_m57_timeout() <= 0)
	{
		set_m57_timeout(M57_DEFAULT_TIMEOUT);
	}

	/* Sanity checking on the receive timeout value */
	if(get_rx_timeout() <= 0)
	{
		set_rx_timeout(DEFAULT_TIMEOUT);
	}

	/* Initialize signal handlers */
	sigint_init();
	sigalrm_init();

	/* Mark the start time */
	start_time = time(NULL);

	/* Do it. */
	crack();

	/* Mark the end time */
	end_time = time(NULL);

	/* Check our key status */
	if(get_key_status() == KEY_DONE)
	{
		wps = get_wps();

		cprintf(VERBOSE,  		    "[+] Key cracked in %d seconds\n", (int) (end_time - start_time));
		cprintf(CRITICAL, 		    "[+] WPS PIN: '%s'\n", get_pin());
		if(wps->key)      cprintf(CRITICAL, "[+] WPA PSK: '%s'\n", wps->key);
		if(wps->essid)    cprintf(CRITICAL, "[+] AP SSID: '%s'\n", wps->essid);

		ret_val = EXIT_SUCCESS;
	}
	else 
	{
		cprintf(CRITICAL, "[-] Failed to recover WPA key\n");
	}
	
end:
	globule_deinit();
	return ret_val;
}

int usage(char *prog_name)
{
        float fail_timeout = 0;

        fail_timeout = ((float) M57_DEFAULT_TIMEOUT / (float) SEC_TO_US);

        fprintf(stderr, "\nReaver v%s\n", PACKAGE_VERSION);

        fprintf(stderr, "\nRequired Arguments:\n");
        fprintf(stderr, "\t-i, --interface=<wlan>          Name of the monitor-mode interface to use\n");
        fprintf(stderr, "\t-b, --bssid=<mac>               BSSID of the target AP\n");

        fprintf(stderr, "\nOptional Arguments:\n");
        fprintf(stderr, "\t-m, --mac=<mac>                 MAC of the host system\n");
        fprintf(stderr, "\t-e, --essid=<ssid>              ESSID of the target AP\n");
        fprintf(stderr, "\t-c, --channel=<channel>         Set the 802.11 channel for the interface (implies -f)\n");
	fprintf(stderr, "\t-o, --out-file=<file>           Send output to a log file [stdout]\n");
	fprintf(stderr, "\t-a, --auto                      Auto detect the best advanced options for the target AP\n");
        fprintf(stderr, "\t-f, --fixed                     Disable channel hopping\n");
        fprintf(stderr, "\t-5, --5ghz                      Use 5GHz 802.11 channels\n");
        fprintf(stderr, "\t-v, --verbose                   Display non-critical warnings (-vv for more)\n");
        fprintf(stderr, "\t-q, --quiet                     Only display critical messages\n");
        fprintf(stderr, "\t-h, --help                      Show help\n");
        
        fprintf(stderr, "\nAdvanced Options:\n");
	fprintf(stderr, "\t-d, --delay=<seconds>           Set the delay between pin attempts [%d]\n", DEFAULT_DELAY);
        fprintf(stderr, "\t-l, --lock-delay=<seconds>      Set the time to wait if the AP locks WPS pin attempts [%d]\n", DEFAULT_LOCK_DELAY);
        fprintf(stderr, "\t-g, --max-attempts=<num>        Quit after num pin attempts\n");
        fprintf(stderr, "\t-x, --fail-wait=<seconds>       Set the time to sleep after %d unexpected failures [0]\n", WARN_FAILURE_COUNT);
        fprintf(stderr, "\t-r, --recurring-delay=<x:y>     Sleep for y seconds every x pin attempts\n");
        fprintf(stderr, "\t-t, --timeout=<seconds>         Set the receive timeout period [%d]\n", DEFAULT_TIMEOUT);
        fprintf(stderr, "\t-T, --m57-timeout=<seconds>     Set the M5/M7 timeout period [%.2f]\n", fail_timeout);
        fprintf(stderr, "\t-L, --ignore-locks              Ignore locked state reported by the target AP\n");
        fprintf(stderr, "\t-E, --eap-terminate             Terminate each WPS session with an EAP FAIL packet\n");
        fprintf(stderr, "\t-n, --nack                      Target AP always sends a NACK [Auto]\n");

        fprintf(stderr, "\nExample:\n\t%s -i mon0 -b 00:90:4C:C1:AC:21 -vv\n\n", prog_name);

        return EXIT_FAILURE;
}
