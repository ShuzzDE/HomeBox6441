/*
 *  drivers/mtd/ifxmips_mtd_nand_ecc.h
 *
 *  Copyright (C) 2000 Steven J. Hill (sjhill@realitydiluted.com)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This file is the header for the ECC algorithm.
 */

#ifndef __IFX_MTD_NAND_ECC_H__
#define __IFX_MTD_NAND_ECC_H__
#include <linux/version.h>

#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,32))
struct mtd_info;

/*
 * Calculate 3 byte ECC code for 256 byte block
 */
extern int ifx_nand_calculate_ecc(struct mtd_info *mtd, const u_char *dat, u_char *ecc_code);

/*
 * Detect and correct a 1 bit error for eccsize byte block
 */
int __ifx_nand_correct_data(u_char *dat, u_char *read_ecc, u_char *calc_ecc,
			unsigned int eccsize);

/*
 * Detect and correct a 1 bit error for 256 byte block
 */
int ifx_nand_correct_data(struct mtd_info *mtd, u_char *dat, u_char *read_ecc, u_char *calc_ecc);

#endif // (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,32))
#endif /* __IFX_MTD_NAND_ECC_H__ */
