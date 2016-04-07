#include <common.h>
#include <command.h>

#if (CONFIG_COMMANDS & CFG_CMD_SF)

extern flash_info_t flash_info[];	/* info for FLASH chips */

int str2off(const char *p, loff_t *num)
{
	char *endptr;

	*num = (long long) simple_strtoul(p, &endptr, 16);
	return *p != '\0' && *endptr == '\0';
}

/*
 * This function computes the length argument for the erase command.
 * The length on which the command is to operate can be given in two forms:
 * 1. <cmd> offset len  - operate on <'offset',  'len')
 * 2. <cmd> offset +len - operate on <'offset',  'round_up(len)')
 * If the second form is used and the length doesn't fall on the
 * sector boundary, than it will be adjusted to the next sector boundary.
 * If it isn't in the flash, the function will fail (return -1).
 * Input:
 *    arg: length specification (i.e. both command arguments)
 * Output:
 *    len: computed length for operation
 * Return:
 *    1: success
 *   -1: failure (bad format, bad address).
 */
static int sf_parse_len_arg(char *arg, ulong *len)
{
	char *ep;
	char round_up_len; /* indicates if the "+length" form used */
	ulong len_arg;

	round_up_len = 0;
	if (*arg == '+') {
		round_up_len = 1;
		++arg;
	}

	len_arg = simple_strtoul(arg, &ep, 16);
	if (ep == arg || *ep != '\0')
		return -1;

	if (round_up_len && flash_info->sector_size > 0)
		*len = ROUND(len_arg, flash_info->sector_size);
	else
		*len = len_arg;

	return 1;
}

static int do_spi_flash_read_write(int argc, char * const argv[])
{
	unsigned long addr;
	void *buf;
	char *endp;
	int ret = 1;
	int dev = 0;
	loff_t offset, len, maxsize;

	if (argc < 3)
		return -1;

	addr = simple_strtoul(argv[1], &endp, 16);
	if (*argv[1] == 0 || *endp != 0)
		return -1;

	if (!str2off(argv[2], &offset)){
		puts("Offset is invalid\n");
		return -1;
	}

	if (!str2off(argv[3], &len)) {
		printf("'%s' is not a number\n", argv[1]);
		return -1;
	}

	/* Consistency checking */
	if (offset + len > flash_info->size) {
		printf("ERROR: attempting %s past flash size (%#x)\n",
		       argv[0], flash_info->size);
		return 1;
	}

	buf = (void*) addr;

	if (strncmp(argv[0], "read", 4) == 0 ||
			strncmp(argv[0], "write", 5) == 0) {
		int read;

		read = strncmp(argv[0], "read", 4) == 0;
		if (read)
			ret = qca_sf_read(flash_info, 0, (u32)offset, (u32)len, (u8*) buf);
		else
			ret = qca_sf_write_buf(flash_info, 0, (u32)offset, (u32)len, (u8*) buf);

		printf("SF: %u bytes @ %#x %s: ", (size_t)len, (u32)offset,
		       read ? "Read" : "Written");
		if (ret)
			printf("ERROR %d\n", ret);
		else
			printf("OK\n");
	}

	return ret == 0 ? 0 : 1;
}

static int do_spi_flash_erase(int argc, char * const argv[])
{
	int ret;
	int dev = 0;
	loff_t offset, len, maxsize;
	ulong size;

	if (argc < 3)
		return -1;

	if (!str2off(argv[1], &offset)){
		puts("Offset is invalid\n");
		return -1;
	}

	ret = sf_parse_len_arg(argv[2], &size);
	if (ret != 1)
		return -1;

	/* Consistency checking */
	if (offset + size > flash_info->size) {
		printf("ERROR: attempting %s past flash size (%#x)\n",
		       argv[0], flash_info->size);
		return 1;
	}

	ret = qca_sf_flash_erase(flash_info, (u32)offset, (u32)size);

	printf("SF: %u bytes @ %#x Erased: %s\n", (size_t)size, (u32)offset,
	       ret ? "ERROR" : "OK");

	return ret == 0 ? 0 : 1;
}


static int do_spi_flash(cmd_tbl_t *cmdtp, int flag, int argc,
			char * const argv[])
{
	const char *cmd;
	int ret;

	/* need at least two arguments */
	if (argc < 2)
		goto usage;

	cmd = argv[1];
	--argc;
	++argv;

	if (strcmp(cmd, "read") == 0 || strcmp(cmd, "write") == 0)
		ret = do_spi_flash_read_write(argc, argv);
	else if (strcmp(cmd, "erase") == 0)
		ret = do_spi_flash_erase(argc, argv);
	else
		ret = -1;

done:
	if (ret != -1)
		return ret;

usage:
	return CMD_RET_USAGE;
}


U_BOOT_CMD(
	sf,	5,	1,	do_spi_flash,
	"sf      - SPI flash sub-system\n",
	"\n"
	"sf read addr offset len	- read `len' bytes starting at\n"
	"			          `offset' to memory at `addr'\n"
	"sf write addr offset len	- write `len' bytes from memory\n"
	"			          at `addr' to flash at `offset'\n"
	"sf erase offset [+]len		- erase `len' bytes from `offset'\n"
	"				 `+len' round up `len' to block size\n"
);

#endif	/* CFG_CMD_SF */