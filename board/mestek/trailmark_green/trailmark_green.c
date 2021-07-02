// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2021 Mestek, Inc
 */

#include <common.h>
#include <asm/global_data.h>
#include <asm/arch/sys_proto.h>
#include <asm/arch/ddr_defs.h>
#include <asm/arch/clock.h>
#include <asm/arch/mux.h>
#include <cpsw.h>
#include <net.h>
#include <asm/omap_mmc.h>
#include <asm/arch/mmc_host_def.h>
#include <fdt_support.h>
#include <dm.h>


/*
 * NOTE TO MESTEK MAINTAINERS:
 *
 * The code base of U-Boot is very fragmented and hard to maintain. If you're
 * trying to maintain this board, I suggest looking at Coreboot instead.
 * We need a bootloader because the Linux kernel can't initialize DDR memory,
 * and expects itself and a device tree to be in memory when booting. The ROM
 * boot code of the SoC can't load the kernel directly because it's too big,
 * so the SoC loads the SPL bootloader. The SPL bootloader initializes the DDR,
 * loads u-boot, then u-boot initializes the MMC and access the filesystem to
 * retrieve the kernel image, loads it into memory, and jumps to it.
 *
 * Once started, the Linux kernel takes over all peripherals and overwrites the
 * memory used by the bootloader. There is no need to initialize peripherals in
 * the bootloader.
 *
 * Coreboot is a minimal bootloader. It only initializes the UART, DDR, and a
 * SPI flash memory containing the coreboot file system. A minimal Linux
 * kernel and device tree can be placed in the flash memory, acting as a
 * bootloader. Coreboot will load the minimal kernel, then the minimal kernel
 * can run the init script to display boot splash screens, load the full kernel
 * through Kexec from the network, USB, or an SDCard.
 * The minimal kernel can also be used as a Kdump kernel, logging data if the
 * main kernel crashes.
 *
 * Currently, with Trailmark Green, which is based on Beaglebone Green, we don't
 * have the freedom to do much besides using U-Boot. In addition, due to a
 * hardware design issue on BeagleBones, the Ethernet PHY address is sometimes
 * latched incorrectly. We need U-boot Ethernet drivers to scan the latched
 * address and modify the device tree passed to the Linux kernel, so that the
 * kernel can talk to the Ethernet PHY. We are stuck with U-Boot until we can
 * have our custom boards.
 */



/*
 * Only the SPL (first stage) initializes the DDR.
 */
#ifdef CONFIG_SPL_BUILD
/*
 * This value has no effect. The actual DDR PLL parameter is returned by
 * get_dpll_ddr_params(). .
 */
#	define DDR_CLK_MHZ   (400)
#	define DDR_EMIF_OCP  (0x00141414)

static const struct ctrl_ioregs _ddr_init_ctrl =
{
	.cm0ioctl = MT41K256M16HA125E_IOCTRL_VALUE,
	.cm1ioctl = MT41K256M16HA125E_IOCTRL_VALUE,
	.cm2ioctl = MT41K256M16HA125E_IOCTRL_VALUE,
	.dt0ioctl = MT41K256M16HA125E_IOCTRL_VALUE,
	.dt1ioctl = MT41K256M16HA125E_IOCTRL_VALUE
};

static const struct cmd_control _ddr_init_cmd =
{
	.cmd0csratio = MT41K256M16HA125E_RATIO,
	.cmd0iclkout = MT41K256M16HA125E_INVERT_CLKOUT,
	.cmd1csratio = MT41K256M16HA125E_RATIO,
	.cmd1iclkout = MT41K256M16HA125E_INVERT_CLKOUT,
	.cmd2csratio = MT41K256M16HA125E_RATIO,
	.cmd2iclkout = MT41K256M16HA125E_INVERT_CLKOUT
};

static const struct ddr_data _ddr_init_data =
{
	.datardsratio0 = MT41K256M16HA125E_RD_DQS,
	.datawdsratio0 = MT41K256M16HA125E_WR_DQS,
	.datafwsratio0 = MT41K256M16HA125E_PHY_FIFO_WE,
	.datawrsratio0 = MT41K256M16HA125E_PHY_WR_DATA
};

static const struct emif_regs _ddr_init_emif =
{
	.sdram_config = MT41K256M16HA125E_EMIF_SDCFG,
	.ref_ctrl = MT41K256M16HA125E_EMIF_SDREF,
	.sdram_tim1 = MT41K256M16HA125E_EMIF_TIM1,
	.sdram_tim2 = MT41K256M16HA125E_EMIF_TIM2,
	.sdram_tim3 = MT41K256M16HA125E_EMIF_TIM3,
	.ocp_config = DDR_EMIF_OCP,
	.zq_config = MT41K256M16HA125E_ZQ_CFG,
	.emif_ddr_phy_ctlr_1 = MT41K256M16HA125E_EMIF_READ_LATENCY
};

void sdram_init(void)
{
	config_ddr(DDR_CLK_MHZ, &_ddr_init_ctrl, &_ddr_init_data, &_ddr_init_cmd,
			&_ddr_init_emif, 0);
}

const struct dpll_params *get_dpll_ddr_params(void)
{
	int ind = get_sys_clk_index();

	/*
	 * Storage needs to be allocated statically instead of on the stack because
	 * it is referenced outside the function.
	 */
	static const struct dpll_params dpll_ddr3_400MHz[NUM_CRYSTAL_FREQ] =
	{
		{125, 5, 1, -1, -1, -1, -1}, /*19.2*/
		{50, 2, 1, -1, -1, -1, -1}, /* 24 MHz */
		{16, 0, 1, -1, -1, -1, -1}, /* 25 MHz */
		{200, 12, 1, -1, -1, -1, -1}  /* 26 MHz */
	};

	return &dpll_ddr3_400MHz[ind];
}
#endif /* CONFIG_SPL_BUILD */



/*
 * board_init() : int - board/mestek/trailmark_green/trailmark_green.c
 *	{init init_sequence_r}() : init_fnc_t [] - common/board_r.c
 */
int board_init(void)
{
	/*
	 * Due to an issue with the prototype of configure_module_pin_mux(),
	 * we can't use const here
	 */
	struct module_pin_mux _mdio_pin_mux[] =
	{
		{OFFSET(mdio_data), MODE(0) | RXACTIVE | PULLUP_EN},
		{OFFSET(mdio_clk), MODE(0) | PULLUP_EN},
		{-1}
	};

	struct module_pin_mux _mmc0_pin_mux[] =
	{
		{OFFSET(mmc0_dat3), (MODE(0) | RXACTIVE | PULLUP_EN)},	/* MMC0_DAT3 */
		{OFFSET(mmc0_dat2), (MODE(0) | RXACTIVE | PULLUP_EN)},	/* MMC0_DAT2 */
		{OFFSET(mmc0_dat1), (MODE(0) | RXACTIVE | PULLUP_EN)},	/* MMC0_DAT1 */
		{OFFSET(mmc0_dat0), (MODE(0) | RXACTIVE | PULLUP_EN)},	/* MMC0_DAT0 */
		{OFFSET(mmc0_clk), (MODE(0) | RXACTIVE | PULLUP_EN)},	/* MMC0_CLK */
		{OFFSET(mmc0_cmd), (MODE(0) | RXACTIVE | PULLUP_EN)},	/* MMC0_CMD */
		{OFFSET(spi0_cs1), (MODE(7) | RXACTIVE | PULLUP_EN)},	/* MMC0_CD */
		{-1},
	};

	struct module_pin_mux _mmc1_pin_mux[] =
	{
		{OFFSET(gpmc_ad7), (MODE(1) | RXACTIVE | PULLUP_EN)},	/* MMC1_DAT7 */
		{OFFSET(gpmc_ad6), (MODE(1) | RXACTIVE | PULLUP_EN)},	/* MMC1_DAT6 */
		{OFFSET(gpmc_ad5), (MODE(1) | RXACTIVE | PULLUP_EN)},	/* MMC1_DAT5 */
		{OFFSET(gpmc_ad4), (MODE(1) | RXACTIVE | PULLUP_EN)},	/* MMC1_DAT4 */
		{OFFSET(gpmc_ad3), (MODE(1) | RXACTIVE | PULLUP_EN)},	/* MMC1_DAT3 */
		{OFFSET(gpmc_ad2), (MODE(1) | RXACTIVE | PULLUP_EN)},	/* MMC1_DAT2 */
		{OFFSET(gpmc_ad1), (MODE(1) | RXACTIVE | PULLUP_EN)},	/* MMC1_DAT1 */
		{OFFSET(gpmc_ad0), (MODE(1) | RXACTIVE | PULLUP_EN)},	/* MMC1_DAT0 */
		{OFFSET(gpmc_csn1), (MODE(2) | RXACTIVE | PULLUP_EN)},	/* MMC1_CLK */
		{OFFSET(gpmc_csn2), (MODE(2) | RXACTIVE | PULLUP_EN)},	/* MMC1_CMD */
		{-1},
	};

	configure_module_pin_mux(_mdio_pin_mux);
	configure_module_pin_mux(_mmc0_pin_mux);
	configure_module_pin_mux(_mmc1_pin_mux);

	return 0;
}


/*
 * The call hierarchy seems to be
 * set_uart_mux_conf() : void - board/mestek/trailmark_green/trailmark_green.c
 *	early_system_init() : void - arch/arm/mach-omap2/am33xx/board.c
 *		arch_cpu_init_dm() : int - arch/arm/mach-omap2/am33xx/board.c
 *		arch_cpu_init_dm() : int - arch/arm/mach-omap2/hwinit-common.c
 *		board_init_f(ulong) : void - arch/arm/mach-omap2/am33xx/board.c
 *		s_init() : void - arch/arm/mach-omap2/omap3/board.c
 */
void set_uart_mux_conf(void)
{
	/*
	 * Sorry, we can't use const here because of a problem with the
	 * configure_module_pin_mux() function prototype.
	 */
	struct module_pin_mux _uart0_pin_mux[] =
	{
		{OFFSET(uart0_rxd), (MODE(0) | PULLUP_EN | RXACTIVE)},
		{OFFSET(uart0_txd), (MODE(0))},
		{-1}
	};

	configure_module_pin_mux(_uart0_pin_mux);
}


/*
 * Initializes the SDCards without using a u-boot device tree
 */
#if !CONFIG_IS_ENABLED(OF_CONTROL)
static const struct omap_hsmmc_plat am335x_mmc0_plat =
{
	.base_addr = (struct hsmmc *)OMAP_HSMMC1_BASE,
	.cfg.host_caps = MMC_MODE_HS_52MHz | MMC_MODE_HS | MMC_MODE_4BIT,
	.cfg.f_min = 400000,
	.cfg.f_max = 52000000,
	.cfg.voltages = MMC_VDD_32_33 | MMC_VDD_33_34 | MMC_VDD_165_195,
	.cfg.b_max = CONFIG_SYS_MMC_MAX_BLK_COUNT,
};

static const struct omap_hsmmc_plat am335x_mmc1_plat =
{
	.base_addr = (struct hsmmc *)OMAP_HSMMC2_BASE,
	.cfg.host_caps = MMC_MODE_HS_52MHz | MMC_MODE_HS | MMC_MODE_8BIT,
	.cfg.f_min = 400000,
	.cfg.f_max = 52000000,
	.cfg.voltages = MMC_VDD_32_33 | MMC_VDD_33_34 | MMC_VDD_165_195,
	.cfg.b_max = CONFIG_SYS_MMC_MAX_BLK_COUNT,
};

U_BOOT_DRVINFO(am335x_mmc0) =
{
	.name = "omap_hsmmc",
	.plat = &am335x_mmc0_plat,
};

U_BOOT_DRVINFO(am335x_mmc1) =
{
	.name = "omap_hsmmc",
	.plat = &am335x_mmc1_plat,
};
#endif


/*
 * Fixup the Linux device tree before passing it to the Linux kernel
 *
 * This is introduced as a workaround to resolve a PCB design issue inherent to
 * BeagleBones. Trailmark Green uses BeagleBone Green.
 *
 * The MDIO address of the Ethernet PHY is _SOMETIMES_ latched as 0x2 instead of
 * the designed 0x0. As a result, the Linux kernel is not always able to control
 * the PHY. The board-specific device tree setup probes the Ethernet PHY using
 * the U-Boot Ethernet drivers, and updates the PHY address in the device tree
 * before passing it to the Linux kernel.
 */
int ft_board_setup(void *device_tree, struct bd_info *board_info)
{
	(void) board_info;

	const char *phy_node_path;
	phy_node_path = fdt_get_alias(device_tree, "fixup-ethphy");
	if (phy_node_path == NULL)
	{
		printf("Fixup ETH PHY: Skipped, missing \"fixup-ethphy\" node alias\n");
		return 0;
	}

	int phy_node;
	phy_node = fdt_path_offset(device_tree, phy_node_path);

	int eth_node;
	const char *eth_name;
	struct udevice *eth_device;
	eth_node = fdt_parent_offset(device_tree, phy_node);
	eth_name = fdt_get_name(device_tree, eth_node, NULL);
	eth_device = eth_get_dev_by_name(eth_name);
	if (eth_device == NULL)
	{
		printf("Fixup ETH PHY: U-boot device not found.\n");
		return -1;
	}

	/* use the U-boot Ethernet driver to probe the actual PHY address */
	int phy_addr_probe;
	phy_addr_probe = cpsw_get_slave_phy_addr(eth_device, 0);

	printf("Fixup ETH PHY: probed at <0x%02X>, updating device tree\n",
			(unsigned) phy_addr_probe);
	int result;
	result = fdt_find_and_setprop(device_tree, phy_node_path, "reg",
			&phy_addr_probe, sizeof(phy_addr_probe), 0);

	if (result)
		printf("Fixup ETH PHY: failed to update property");
	return 0;
}

