#include <common.h>
#include <command.h>
#include <part.h>
#include <usb.h>
#include <fs.h>
#include <fat.h>

#define XMK_STR(x) #x
#define MK_STR(x) XMK_STR(x)


void usbboot_indicate_recovery(int type)
{
	// type=0 - before recovery
	// type=1 - after recovery
	int cnt = 4;  // how many times to blink

	if (type) cnt=2;
	while(cnt--){
		ar7240_gpio_led_switch(3, 1);
		mdelay(500);
		ar7240_gpio_led_switch(3, 0);
		mdelay(500);
	}
	if (!type) mdelay(500); //wait 0.5s after last blink before recovery
}

int usbboot_boot(char * boot_dev_part, char * boot_file_name)
{
	int fat_read_ret;
	char* bootm_argv[] = {"bootm", MK_STR(CFG_USB_BOOT_LOAD_ADDR)};

	fs_set_blk_dev ("usb", boot_dev_part, FS_TYPE_FAT);
	fat_read_ret = do_fat_read_at(boot_file_name,
								0,
								CFG_USB_BOOT_LOAD_ADDR,
								CFG_MAX_USB_BOOT_FILE_SIZE,
								0);
	if (fat_read_ret > 0){
		printf ("Booting image %s (%d bytes) from usb device %s \n",
			boot_file_name , fat_read_ret, boot_dev_part);
		do_bootm(NULL, 0 , 2, bootm_argv);
	}
	return -1; //Boot failed
}

int usbboot_recovery(char * boot_dev_part, char * boot_file_name)
{
	int fat_read_ret;
	char flash_cmd [128];
	char image_size[16];
	
	fs_set_blk_dev ("usb", boot_dev_part, FS_TYPE_FAT);
	fat_read_ret = do_fat_read_at(boot_file_name,
					0,
					CFG_USB_BOOT_LOAD_ADDR,
					CFG_MAX_USB_RECOVERY_FILE_SIZE,
					0);

	if (fat_read_ret > 0){
		usbboot_indicate_recovery(0);
		//TODO: check image??
		printf ("Flashing image %s (%d bytes) from usb device %s \n",
			boot_file_name , fat_read_ret, boot_dev_part);
		sprintf(image_size,"%x",fat_read_ret);

		// Example command
		//erase 0x9f050000 +3FA000; cp.b 0x80060000 0x9f050000 3FA000
		char* load_addr = MK_STR(CFG_USB_BOOT_LOAD_ADDR);
		sprintf(flash_cmd,"erase %s +%s; cp.b %s %s %s",
			  CFG_USB_RECOVERY_FW_START_IN_FLASH,
			  image_size,
			  load_addr,
			  CFG_USB_RECOVERY_FW_START_IN_FLASH,
			  image_size);
		printf ("\nFlashing image\n%s\n", flash_cmd);
		setenv("recovery_flash_cmd", flash_cmd);
		int  argc_flash=2;
		char* argv_flash[]={"run", "recovery_flash_cmd"};
		cmd_tbl_t *cmdtp=NULL;
		
		int flash_ret = do_run ( cmdtp,  0,  argc_flash, argv_flash);		
		printf ("\nFlashing sucsessful. Remove USB storage with recovery image.");
		usbboot_indicate_recovery(1);
		do_reset(NULL, 0, 0, NULL);
	}
	return -1; //Boot failed
}

int usbboot_scan(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	int i;
	char part_id[8];
	block_dev_desc_t *stor_dev = NULL;
	disk_partition_t *info = NULL;
	
	char* boot_file_name; //Can include directories eg. "boot/vmlinux.uimage"
	char* recovery_file_name;
	char* boot_dev_part; 
	/* boot_dev_part format "<usb device>:<partition>", 
	 * eg. "0:1" - frst usb storage device's first partition
	 * */
	  
	boot_file_name = getenv ("bootfile");
	if (boot_file_name == NULL)
		boot_file_name = CFG_USB_BOOT_FILENAME;
	
	recovery_file_name = getenv ("recoveryfile");
	if (recovery_file_name == NULL)
		recovery_file_name = CFG_USB_RECOVERY_FILENAME;
	
	boot_dev_part = getenv ("bootdev");
	
	usb_stop();

	if (usb_init() < 0)
		return -1;
	/* try to recognize storage devices immediately */
	usb_stor_scan(0);

	int devno = 0;

	//if given device name just boot it
	if (boot_dev_part != NULL){
		usbboot_recovery(boot_dev_part, recovery_file_name);
		usbboot_boot(boot_dev_part, boot_file_name);
		return -2; //if returns, means boot failed
	}

	// scan all devices and partitions for boot image
	for (devno = 0; ; ++devno) {
		stor_dev = usb_stor_get_dev(devno);
		if (stor_dev == NULL)
			break;

		//try boot from full device (no partitions)
		sprintf(part_id,"%d:%d", devno, 0);
		usbboot_recovery(part_id, recovery_file_name);
		usbboot_boot(part_id, boot_file_name);

		//scan partitions
		if (stor_dev->part_type != PART_TYPE_DOS)
			continue;
		for (i=1; i<=CFG_USB_BOOT_MAX_PARTITIONS_SCAN; i++){
			printf("start %d size %d name %s\n", info->start, info->size, info->name);
			printf("SCANNING device %d\n",i);
			if (get_partition_info_dos(stor_dev, i, info) == 0){
				sprintf(part_id,"%d:%d", devno, i);
				usbboot_recovery(part_id, recovery_file_name);
				usbboot_boot(part_id, boot_file_name);
			}
		}
	}
	printf ("Boot or recovery image (%s or %s) not found in usb storage\n", boot_file_name, recovery_file_name);
	return -1;
}

void do_usb_boot (void){
	char* boot_mode;
	int s_force=0, s_gpio=0, s_never = 0;

	boot_mode = getenv ("usbboot");
	if (boot_mode != NULL){
		s_force = !(strcmp(boot_mode, "force"));
		s_gpio = !(strcmp(boot_mode, "gpio"));
		s_never = !(strcmp(boot_mode, "never"));
	}
	
	if ( (s_force && s_never == 0) || (boot_mode == NULL))
		s_gpio = 1;
	
	if (s_never){
		printf("USB boot is disabled in environment\n");
		return;
	}
	if (s_force){
		printf("USB boot is forced in environment\n");
		usbboot_scan(0,0,0,0);
	}
	if ( s_gpio ){
	debug("USB GPIO: %d\n", button_read(CFG_USB_BOOT_BUTTON_ID));
	if (button_read(CFG_USB_BOOT_BUTTON_ID)==1)
		usbboot_scan(0,0,0,0);
	}
}
	
U_BOOT_CMD(
	usb_boot_file,	5,	1,	usbboot_scan,
	"usb_boot_file - Automatic boot/recovery from file in USB drive\n",
	" "
);
