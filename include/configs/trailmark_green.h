/*
 * trailmark_green.h
 *
 *  Created on: Jun 16, 2021
 *      Author: jdoe
 */

#ifndef HE2939EC9_A20D_4C71_81D1_B6659EF24F76
#define HE2939EC9_A20D_4C71_81D1_B6659EF24F76

#include <configs/ti_am335x_common.h>
#include <linux/sizes.h>

#ifndef CONFIG_SPL_BUILD
# define CONFIG_TIMESTAMP
#endif

#define CONFIG_SYS_BOOTM_LEN		SZ_16M

#define CONFIG_MACH_TYPE		MACH_TYPE_AM335XEVM

/* Clock Defines */
#define V_OSCK				24000000  /* Clock output from T2 */
#define V_SCLK				(V_OSCK)

#include <config_distro_bootcmd.h>

#define BOOTENV_DEV_LEGACY_MMC(devtypeu, devtypel, instance) \
	"bootcmd_" #devtypel #instance "=" \
	"setenv mmcdev " #instance"; "\
	"setenv bootpart " #instance":2 ; "\
	"run mmcboot\0"

#define BOOTENV_DEV_NAME_LEGACY_MMC(devtypeu, devtypel, instance) \
	#devtypel #instance " "

#define BOOT_TARGET_DEVICES(func) \
	func(LEGACY_MMC, legacy_mmc, 0) \
	func(LEGACY_MMC, legacy_mmc, 1)

#ifndef CONFIG_SPL_BUILD
#define CONFIG_EXTRA_ENV_SETTINGS \
	"load_addr=0x82000000\0" \
	"fdt_addr=0x88000000\0" \
	"rd_addr=0x88080000\0" \
	\
	"bootpart=0:2\0" \
	\
	"zimage_file=/boot/zImage\0" \
	"fdt_file=/boot/device-tree.dtb\0" \
    "rd_file=/boot/initrd\0" \
	\
    "load_image=load mmc ${bootpart} ${load_addr} ${zimage_file}\0" \
    "load_fdt=load mmc ${bootpart} ${fdt_addr} ${fdt_file}\0" \
    "load_rd=load mmc ${bootpart} ${rd_addr} ${rd_file}\0" \
	\
	"distro_bootcmd="\
	"setenv bootargs 'console=ttyS0,115200n8;' " \
	"run load_image; run load_fdt; run load_rd; " \
	"bootz ${load_addr} - ${fdt_addr}\0"
#endif /* CONFIG_SPL_BUILD */

/* NS16550 Configuration */
#define CONFIG_SYS_NS16550_COM1		0x44e09000	/* Base EVM has UART0 */
#define CONFIG_SYS_NS16550_COM2		0x48022000	/* UART1 */
#define CONFIG_SYS_NS16550_COM3		0x48024000	/* UART2 */
#define CONFIG_SYS_NS16550_COM4		0x481a6000	/* UART3 */
#define CONFIG_SYS_NS16550_COM5		0x481a8000	/* UART4 */
#define CONFIG_SYS_NS16550_COM6		0x481aa000	/* UART5 */

/* PMIC support */
#define CONFIG_POWER_TPS65217

/*
 * Disable MMC DM for SPL build and can be re-enabled after adding
 * DM support in SPL
 */
#ifdef CONFIG_SPL_BUILD
#undef CONFIG_DM_MMC
#undef CONFIG_TIMER
#endif

#endif /* HE2939EC9_A20D_4C71_81D1_B6659EF24F76 */
