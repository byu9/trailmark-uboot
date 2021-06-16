// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2021 Mestek, Inc
 */

#include <common.h>
#include <asm/arch/sys_proto.h>
#include <asm/arch/mux.h>



#ifdef CONFIG_SPL_BUILD

#	include <asm/arch/ddr_defs.h>
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



static const struct module_pin_mux _uart0_pin_mux[] =
{
	{OFFSET(uart0_rxd), (MODE(0) | PULLUP_EN | RXACTIVE)},
	{OFFSET(uart0_txd), (MODE(0))},
	{-1}
};

void set_uart_mux_conf(void)
{
	configure_module_pin_mux(_uart0_pin_mux);
}



static const struct module_pin_mix _mdio_pin_mux[] =
{
	{OFFSET(mdio_data), MODE(0) | RXACTIVE | PULLUP_EN},
	{OFFSET(mdio_clk), MODE(0) | PULLUP_EN},
	{-1}
};

static const struct module_pin_mix _mmc0_pin_mux[] =
{
	{OFFSET(mmc0_dat3), (MODE(0) | RXACTIVE | PULLUP_EN)},	/* MMC0_DAT3 */
	{OFFSET(mmc0_dat2), (MODE(0) | RXACTIVE | PULLUP_EN)},	/* MMC0_DAT2 */
	{OFFSET(mmc0_dat1), (MODE(0) | RXACTIVE | PULLUP_EN)},	/* MMC0_DAT1 */
	{OFFSET(mmc0_dat0), (MODE(0) | RXACTIVE | PULLUP_EN)},	/* MMC0_DAT0 */
	{OFFSET(mmc0_clk), (MODE(0) | RXACTIVE | PULLUP_EN)},	/* MMC0_CLK */
	{OFFSET(mmc0_cmd), (MODE(0) | RXACTIVE | PULLUP_EN)},	/* MMC0_CMD */
	{OFFSET(spi0_cs1), (MODE(5) | RXACTIVE | PULLUP_EN)},	/* MMC0_CD */
	{-1},
};

static const struct module_pin_mix _mmc1_pin_mux[] =
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

void set_mux_conf_regs(void)
{
	configure_module_pin_mux(_mdio_pin_mux);
	configure_module_pin_mux(_mmc0_pin_mux);
	configure_module_pin_mux(_mmc1_pin_mux);
}

#endif /* CONFIG_SPL_BUILD */



/*
 *
 */
int board_init(void)
{
	gd->bd->bi_boot_params = CONFIG_SYS_SDRAM_BASE + 0x100;

	return 0;
}



/*
 * OK, so this part is confusing. This function is called by
 *
 * First, for CONFIG_SPL_BUILD,
 *        by board_init_f() /u-boot/arch/arm/mach-omap2/am33xx/board.c
 *        by _main() - /u-boot/arch/arm/lib/crt0.S
 *        by <untraceable>
 *
 * Second, for !CONFIG_SPL_BUILD,
 *        by dram_init() - /u-boot/arch/arm/mach-omap2/am33xx/board.c
 *        by init_sequence_f[] - /u-boot/common/board_f.c
 *        by board_init_f() - /u-boot/common/board_f.c
 *        by _main() - /u-boot/arch/arm/lib/crt0.S
 *        by <untraceable>
 *
 * In the second case, the sdram_init() is skipped in dram_init()
 * because in /u-boot/include/configs/ti_am335x_common.h,
 *    CONFIG_SKIP_LOWLEVEL_INIT is defined when (!SPL_BUILD && !NOR_BOOT)
 *
 * So some code of u-boot is in need of desperate refactoring.
 * If Mestek is to maintain this code, I strongly suggest looking at CoreBoot
 * instead. However, if needing the Ethernet PHYADDR DTB fixup, you're stuck
 * with u-boot.
 */


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
#if CONFIG_IS_ENABLED(TRAILMARK_GREEN_FIXUP_ETH_PHYADDR)
int ft_board_setup(void *device_tree, struct bd_info *board_info)
{
	(void) board_info;

	const char *phy_node_path;
	phy_node_path = fdt_get_alias(device_tree, "fixup-ethphy");
	if (phy_node_path == NULL)
	{
		printf("Fixup ETH PHY: Skipped, missing \"fixup-ethphy\" node alias");
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

	printf("Fixup ETH PHY: probed slave PHY at <%d>, updating\n");
	int result;
	result = fdt_find_and_setprop(device_tree, phy_node_path, "reg",
			phy_addr_probe, sizeof(phy_addr_probe), 0);

	if (result)
		printf("Fixup ETH PHY: failed to update property");

	return 0;
}
#endif





#if CONFIG_IS_ENABLED(SPL_SERIAL_SUPPORT)
static void configure_uart_pinmux(void)
{
	const struct module_pin_mux uart0_pin_mux[] = {
		{OFFSET(uart0_rxd), (MODE(0) | PULLUP_EN | RXACTIVE)},
		{OFFSET(uart0_txd), (MODE(0))},
		{-1},
	};

	configure_module_pin_mux(uart0_pin_mux);
}
#endif







void early_system_init(void)
{
	watchdog_disable();

	spl_early_init();
}


/*
 * This requires CONFIG_BOARD_EARLY_INIT_F. This function is called by
 *
 * First, for CONIFG_SPL_BUILD,
 *   by board_early_init_f() - /u-boot/arch/arm/mach-omap2/am33xx/board.c
 *   by board_init_f() /u-boot/arch/arm/mach-omap2/am33xx/board.c
 *   by _main() - /u-boot/arch/arm/lib/crt0.S
 *   by <untraceable>
 *
 * Second, for !CONFIG_SPL_BUILD,
 *   by board_early_init_f() - /u-boot/arch/arm/mach-omap2/am33xx/board.c
 *   by board_init_f() - /u-boot/arch/arm/mach-omap2/am33xx/board.c
 *   by _main() - /u-boot/arch/arm/lib/crt0.S
 *   by <untraceable>
 *
 *
 */
void set_mux_conf_regs(void)
{
	struct module_pin_mux mii1_pin_mux[] =
	{
		{OFFSET(mii1_rxerr), MODE(0) | RXACTIVE},	/* MII1_RXERR */
		{OFFSET(mii1_txen), MODE(0)},			/* MII1_TXEN */
		{OFFSET(mii1_rxdv), MODE(0) | RXACTIVE},	/* MII1_RXDV */
		{OFFSET(mii1_txd3), MODE(0)},			/* MII1_TXD3 */
		{OFFSET(mii1_txd2), MODE(0)},			/* MII1_TXD2 */
		{OFFSET(mii1_txd1), MODE(0)},			/* MII1_TXD1 */
		{OFFSET(mii1_txd0), MODE(0)},			/* MII1_TXD0 */
		{OFFSET(mii1_txclk), MODE(0) | RXACTIVE},	/* MII1_TXCLK */
		{OFFSET(mii1_rxclk), MODE(0) | RXACTIVE},	/* MII1_RXCLK */
		{OFFSET(mii1_rxd3), MODE(0) | RXACTIVE},	/* MII1_RXD3 */
		{OFFSET(mii1_rxd2), MODE(0) | RXACTIVE},	/* MII1_RXD2 */
		{OFFSET(mii1_rxd1), MODE(0) | RXACTIVE},	/* MII1_RXD1 */
		{OFFSET(mii1_rxd0), MODE(0) | RXACTIVE},	/* MII1_RXD0 */
		{OFFSET(mdio_data), MODE(0) | RXACTIVE | PULLUP_EN}, /* MDIO_DATA */
		{OFFSET(mdio_clk), MODE(0) | PULLUP_EN},	/* MDIO_CLK */
		{-1},
	};

	struct module_pin_mux mmc0_pin_mux[] =
	{
		{OFFSET(mmc0_dat3), (MODE(0) | RXACTIVE | PULLUP_EN)},	/* MMC0_DAT3 */
		{OFFSET(mmc0_dat2), (MODE(0) | RXACTIVE | PULLUP_EN)},	/* MMC0_DAT2 */
		{OFFSET(mmc0_dat1), (MODE(0) | RXACTIVE | PULLUP_EN)},	/* MMC0_DAT1 */
		{OFFSET(mmc0_dat0), (MODE(0) | RXACTIVE | PULLUP_EN)},	/* MMC0_DAT0 */
		{OFFSET(mmc0_clk), (MODE(0) | RXACTIVE | PULLUP_EN)},	/* MMC0_CLK */
		{OFFSET(mmc0_cmd), (MODE(0) | RXACTIVE | PULLUP_EN)},	/* MMC0_CMD */
		{OFFSET(mcasp0_aclkr), (MODE(4) | RXACTIVE)},		/* MMC0_WP */
		{OFFSET(spi0_cs1), (MODE(7) | RXACTIVE | PULLUP_EN)},	/* GPIO0_6 */
		{-1},
	};


	configure_module_pin_mux(&mii1_pin_mux);
	configure_module_pin_mux(mmc0_pin_mux);
	configure_module_pin_mux(mmc1_pin_mux);

}







