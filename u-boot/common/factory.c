/*
 * Factory image loading
 *
 * Copyright (C) 2015 Mantas Pucka <mantas@8devices.com>
 *
 * SPDX-License-Identifier:GPL-2.0
*/

#include <common.h>
#include <config.h>

#define XMK_STR(x)        #x
#define MK_STR(x) XMK_STR(x)

static void carambola_factory_load_image(void)
{
	char *filename = CFG_FACTORY_IMG_FILENAME;
	int tftp_ret;

	int  argc_tftp = 3;
	char* argv_tftp[] = {"tftpboot", CFG_FACTORY_IMG_LOAD_ADDR, filename};

	int argc_bootm = 2;
	char* argv_bootm[] = {"bootm", CFG_FACTORY_IMG_LOAD_ADDR};

	if (!getenv ("ipaddr"))
		setenv("ipaddr", MK_STR(CONFIG_IPADDR));

	if (!getenv ("serverip"))
		setenv("serverip", MK_STR(CONFIG_SERVERIP));

	//Workaround for slow TFTP:
	//wake-up network stack with ping to server
	char *serverip = getenv ("serverip");
	char ping_command[32]="ping ";
	strncpy(ping_command+5, serverip, 16);
	run_command(ping_command, NULL);
	//end of workaround

	setenv("netretry", "once"); // Try once, reboot after
	tftp_ret=do_tftpb (NULL, 0, argc_tftp, argv_tftp);
	if (0 == tftp_ret) {
		  printf("Booting TFTP image...\n");
		  do_bootm(NULL, 0, argc_bootm, argv_bootm);
		  do_reset(NULL, 0, 0, NULL);
	}
	else{
		  printf("Error getting TFTP image. Rebooting...\n");
		  do_reset(NULL, 0, 0, NULL);
	}
	return;
}

void carambola_factory_mode(void)
{
	char* production_env = getenv("production");

	if (production_env){
		if (strncmp(production_env, "yes", 3) == 0){
			carambola_factory_load_image();
		}
	}
	return;
}
