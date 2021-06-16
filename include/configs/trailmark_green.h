/*
 * trailmark_green.h
 *
 *  Created on: Jun 16, 2021
 *      Author: jdoe
 */

#ifndef HE2939EC9_A20D_4C71_81D1_B6659EF24F76
#define HE2939EC9_A20D_4C71_81D1_B6659EF24F76

#include <configs/ti_am335x_common.h>

#define CONFIG_MACH_TYPE (MACH_TYPE_BEAGLEBONE)

#define CONFIG_EXTRA_ENV_SETTINGS \
	"load mmc 0:2 ${loadaddr} /boot/zImage\n"





#endif /* HE2939EC9_A20D_4C71_81D1_B6659EF24F76 */
