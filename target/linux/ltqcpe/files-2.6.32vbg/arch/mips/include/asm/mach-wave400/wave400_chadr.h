#ifndef __MT_CHADR_H__
#define __MT_CHADR_H__

#ifdef  MT_GLOBAL
#define MT_EXTERN
#define MT_I(x) x
#else
#define MT_EXTERN extern
#define MT_I(x)
#endif

/* unit base addresses */
#define MT_LOCAL_MIPS_BASE_ADDRESS           0xa7000000
#define WAVE400_SYS_SHARED_REGS_OFFSET       0x001c0000
#define WAVE400_SYS_RESET_REG_OFFSET         0x00000008
#define WAVE400_SPI_MODE_ADDR                (MT_LOCAL_MIPS_BASE_ADDRESS+0x130)
#define WAVE400_SPI_MODE_SW_BIT              (0x400) //When this bit is set, FLASH is accessed in SW mode

#define WAVE400_RESET_OFFSET                 8
#define WAVE400_RESET_ADDR                   (MT_LOCAL_MIPS_BASE_ADDRESS+WAVE400_SYS_SHARED_REGS_OFFSET+WAVE400_RESET_OFFSET)
#define WAVE400_REBOOT_DATA                  0
#define WAVE400_REBOOT_BIT_MASK_NOT          0xfffeffff

#define WAVE400_SHARED_GMAC_BASE_ADDR        (MT_LOCAL_MIPS_BASE_ADDRESS+WAVE400_SYS_SHARED_REGS_OFFSET)
#define WAVE400_GMAC_MODE_REG_OFFSET         0x68
#define WAVE400_GMAC_MODE_REG_ADDR           (MT_LOCAL_MIPS_BASE_ADDRESS+WAVE400_SYS_SHARED_REGS_OFFSET+WAVE400_GMAC_MODE_REG_OFFSET)
#define WAVE400_GMAC_MODE_2_REG_OFFSET       0x80
#define WAVE400_GMAC_MODE_2_REG_ADDR         (MT_LOCAL_MIPS_BASE_ADDRESS+WAVE400_SYS_SHARED_REGS_OFFSET+WAVE400_GMAC_MODE_2_REG_OFFSET)
#define WAVE400_DLY_PGM_REG_OFFSET           0x64
#define WAVE400_DLY_PGM_REG_ADDR             (MT_LOCAL_MIPS_BASE_ADDRESS+WAVE400_SYS_SHARED_REGS_OFFSET+WAVE400_DLY_PGM_REG_OFFSET)
#define WAVE400_HCYCLE_CALIB_IND_REG_OFFSET  0x90
#define WAVE400_HCYCLE_CALIB_IND_REG_ADDR    (MT_LOCAL_MIPS_BASE_ADDRESS+WAVE400_SYS_SHARED_REGS_OFFSET+WAVE400_HCYCLE_CALIB_IND_REG_OFFSET)


#undef MT_EXTERN
#undef MT_I

#endif /* __MT_CHADR_H__ */ 
