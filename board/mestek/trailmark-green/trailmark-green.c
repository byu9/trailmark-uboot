// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2021 Mestek, Inc
 */

#include <common.h>
#include <asm/arch/ddr_defs.h>
#include <asm/arch/mux.h>
#include <asm/arch/sys_proto.h>


int spl_start_uboot(void)
{
}

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


/*
 *
 */
int board_init(void)
{
	return 0;
}



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
int ft_board_setup(void *device_tree, struct bd_info *bd)
{
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
	configure_module_pin_mux(mii1_pin_mux);
	configure_module_pin_mux(mmc0_pin_mux);
	configure_module_pin_mux(mmc1_pin_mux);

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
 * In the second case, the sdram_init() is actually skipped in dram_init()
 * because in /u-boot/include/configs/ti_am335x_common.h,
 *    CONFIG_SKIP_LOWLEVEL_INIT is defined when (!SPL_BUILD && !NOR_BOOT)
 *
 * So some code of u-boot is in need of desperate refactoring.
 * If Mestek is to maintain this code, I strongly suggest looking at CoreBoot
 * instead. However, if needing the Ethernet PHYADDR DTB fixup, you're stuck
 * with u-boot.
 */
void sdram_init(void)
{
	const unsigned ddr_pll_m = 400;

	const struct cmd_control init_ddr_control =
	{
		.cmd0csratio = MT41K256M16HA125E_RATIO,
		.cmd0iclkout = MT41K256M16HA125E_INVERT_CLKOUT,

		.cmd1csratio = MT41K256M16HA125E_RATIO,
		.cmd1iclkout = MT41K256M16HA125E_INVERT_CLKOUT,

		.cmd2csratio = MT41K256M16HA125E_RATIO,
		.cmd2iclkout = MT41K256M16HA125E_INVERT_CLKOUT,
	};

	const struct ctrl_ioregs init_ctrl_ioregs =
	{
		.cm0ioctl = MT41K256M16HA125E_IOCTRL_VALUE,
		.cm1ioctl = MT41K256M16HA125E_IOCTRL_VALUE,
		.cm2ioctl = MT41K256M16HA125E_IOCTRL_VALUE,
		.dt0ioctl = MT41K256M16HA125E_IOCTRL_VALUE,
		.dt1ioctl = MT41K256M16HA125E_IOCTRL_VALUE,
	};

	const struct ddr_data init_ddr_data =
	{
		.datardsratio0 = MT41K256M16HA125E_RD_DQS,
		.datawdsratio0 = MT41K256M16HA125E_WR_DQS,
		.datafwsratio0 = MT41K256M16HA125E_PHY_FIFO_WE,
		.datawrsratio0 = MT41K256M16HA125E_PHY_WR_DATA,
	};



	const struct emif_regs init_ddr_emif_regs =
	{
		.sdram_config = MT41K256M16HA125E_EMIF_SDCFG,
		.ref_ctrl = MT41K256M16HA125E_EMIF_SDREF,
		.sdram_tim1 = MT41K256M16HA125E_EMIF_TIM1,
		.sdram_tim2 = MT41K256M16HA125E_EMIF_TIM2,
		.sdram_tim3 = MT41K256M16HA125E_EMIF_TIM3,
		.ocp_config = EMIF_OCP_CONFIG_BEAGLEBONE_BLACK,
		.zq_config = MT41K256M16HA125E_ZQ_CFG,
		.emif_ddr_phy_ctlr_1 = MT41K256M16HA125E_EMIF_READ_LATENCY,
	};

}





