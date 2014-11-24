//-----------------------------------------------------------------------------
// Copyright (c) 2014 Michael G. Brehm
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//-----------------------------------------------------------------------------

#ifndef __LINUX_MAJOR_H_
#define __LINUX_MAJOR_H_
#pragma once

//-----------------------------------------------------------------------------
// include/uapi/linux/major.h
//-----------------------------------------------------------------------------

#define LINUX_UNNAMED_MAJOR				0
#define LINUX_MEM_MAJOR					1
#define LINUX_RAMDISK_MAJOR				1
#define LINUX_FLOPPY_MAJOR				2
#define LINUX_PTY_MASTER_MAJOR			2
#define LINUX_IDE0_MAJOR				3
#define LINUX_HD_MAJOR					LINUX_IDE0_MAJOR
#define LINUX_PTY_SLAVE_MAJOR			3
#define LINUX_TTY_MAJOR					4
#define LINUX_TTYAUX_MAJOR				5
#define LINUX_LP_MAJOR					6
#define LINUX_VCS_MAJOR					7
#define LINUX_LOOP_MAJOR				7
#define LINUX_SCSI_DISK0_MAJOR			8
#define LINUX_SCSI_TAPE_MAJOR			9
#define LINUX_MD_MAJOR					9
#define LINUX_MISC_MAJOR				10
#define LINUX_SCSI_CDROM_MAJOR			11
#define LINUX_MUX_MAJOR					11		/* PA-RISC only */
#define LINUX_XT_DISK_MAJOR				13
#define LINUX_INPUT_MAJOR				13
#define LINUX_SOUND_MAJOR				14
#define LINUX_CDU31A_CDROM_MAJOR		15
#define LINUX_JOYSTICK_MAJOR			15
#define LINUX_GOLDSTAR_CDROM_MAJOR		16
#define LINUX_OPTICS_CDROM_MAJOR		17
#define LINUX_SANYO_CDROM_MAJOR			18
#define LINUX_CYCLADES_MAJOR			19
#define LINUX_CYCLADESAUX_MAJOR			20
#define LINUX_MITSUMI_X_CDROM_MAJOR		20
#define LINUX_MFM_ACORN_MAJOR			21		/* ARM Linux /dev/mfm */
#define LINUX_SCSI_GENERIC_MAJOR		21
#define LINUX_IDE1_MAJOR				22
#define LINUX_DIGICU_MAJOR				22
#define LINUX_DIGI_MAJOR				23
#define LINUX_MITSUMI_CDROM_MAJOR		23
#define LINUX_CDU535_CDROM_MAJOR		24
#define LINUX_STL_SERIALMAJOR			24
#define LINUX_MATSUSHITA_CDROM_MAJOR	25
#define LINUX_STL_CALLOUTMAJOR			25
#define LINUX_MATSUSHITA_CDROM2_MAJOR	26
#define LINUX_QIC117_TAPE_MAJOR			27
#define LINUX_MATSUSHITA_CDROM3_MAJOR	27
#define LINUX_MATSUSHITA_CDROM4_MAJOR	28
#define LINUX_STL_SIOMEMMAJOR			28
#define LINUX_ACSI_MAJOR				28
#define LINUX_AZTECH_CDROM_MAJOR		29
#define LINUX_FB_MAJOR					29		/* /dev/fb* framebuffers */
#define LINUX_MTD_BLOCK_MAJOR			31
#define LINUX_CM206_CDROM_MAJOR			32
#define LINUX_IDE2_MAJOR				33
#define LINUX_IDE3_MAJOR				34
#define LINUX_Z8530_MAJOR				34
#define LINUX_XPRAM_MAJOR				35		/* Expanded storage on S/390: "slow ram"*/
#define LINUX_NETLINK_MAJOR				36
#define LINUX_PS2ESDI_MAJOR				36
#define LINUX_IDETAPE_MAJOR				37
#define LINUX_Z2RAM_MAJOR				37
#define LINUX_APBLOCK_MAJOR				38		/* AP1000 Block device */
#define LINUX_DDV_MAJOR					39		/* AP1000 DDV block device */
#define LINUX_NBD_MAJOR					43		/* Network block device	*/
#define LINUX_RISCOM8_NORMAL_MAJOR		48
#define LINUX_DAC960_MAJOR				48		/* 48..55 */
#define LINUX_RISCOM8_CALLOUT_MAJOR		49
#define LINUX_MKISS_MAJOR				55
#define LINUX_DSP56K_MAJOR				55		/* DSP56001 processor device */
#define LINUX_IDE4_MAJOR				56
#define LINUX_IDE5_MAJOR				57
#define LINUX_SCSI_DISK1_MAJOR			65
#define LINUX_SCSI_DISK2_MAJOR			66
#define LINUX_SCSI_DISK3_MAJOR			67
#define LINUX_SCSI_DISK4_MAJOR			68
#define LINUX_SCSI_DISK5_MAJOR			69
#define LINUX_SCSI_DISK6_MAJOR			70
#define LINUX_SCSI_DISK7_MAJOR			71
#define LINUX_COMPAQ_SMART2_MAJOR		72
#define LINUX_COMPAQ_SMART2_MAJOR1		73
#define LINUX_COMPAQ_SMART2_MAJOR2		74
#define LINUX_COMPAQ_SMART2_MAJOR3		75
#define LINUX_COMPAQ_SMART2_MAJOR4		76
#define LINUX_COMPAQ_SMART2_MAJOR5		77
#define LINUX_COMPAQ_SMART2_MAJOR6		78
#define LINUX_COMPAQ_SMART2_MAJOR7		79
#define LINUX_SPECIALIX_NORMAL_MAJOR	75
#define LINUX_SPECIALIX_CALLOUT_MAJOR	76
#define LINUX_AURORA_MAJOR				79
#define LINUX_I2O_MAJOR					80		/* 80->87 */
#define LINUX_SHMIQ_MAJOR				85		/* Linux/mips, SGI /dev/shmiq */
#define LINUX_SCSI_CHANGER_MAJOR		86
#define LINUX_IDE6_MAJOR				88
#define LINUX_IDE7_MAJOR				89
#define LINUX_IDE8_MAJOR				90
#define LINUX_MTD_CHAR_MAJOR			90
#define LINUX_IDE9_MAJOR				91
#define LINUX_DASD_MAJOR				94
#define LINUX_MDISK_MAJOR				95
#define LINUX_UBD_MAJOR					98
#define LINUX_PP_MAJOR					99
#define LINUX_JSFD_MAJOR				99
#define LINUX_PHONE_MAJOR				100
#define LINUX_COMPAQ_CISS_MAJOR			104
#define LINUX_COMPAQ_CISS_MAJOR1		105
#define LINUX_COMPAQ_CISS_MAJOR2		106
#define LINUX_COMPAQ_CISS_MAJOR3		107
#define LINUX_COMPAQ_CISS_MAJOR4		108
#define LINUX_COMPAQ_CISS_MAJOR5		109
#define LINUX_COMPAQ_CISS_MAJOR6		110
#define LINUX_COMPAQ_CISS_MAJOR7		111
#define LINUX_VIODASD_MAJOR				112
#define LINUX_VIOCD_MAJOR				113
#define LINUX_ATARAID_MAJOR				114
#define LINUX_SCSI_DISK8_MAJOR			128
#define LINUX_SCSI_DISK9_MAJOR			129
#define LINUX_SCSI_DISK10_MAJOR			130
#define LINUX_SCSI_DISK11_MAJOR			131
#define LINUX_SCSI_DISK12_MAJOR			132
#define LINUX_SCSI_DISK13_MAJOR			133
#define LINUX_SCSI_DISK14_MAJOR			134
#define LINUX_SCSI_DISK15_MAJOR			135
#define LINUX_UNIX98_PTY_MASTER_MAJOR	128
#define LINUX_UNIX98_PTY_MAJOR_COUNT	8
#define LINUX_UNIX98_PTY_SLAVE_MAJOR	(LINUX_UNIX98_PTY_MASTER_MAJOR + LINUX_UNIX98_PTY_MAJOR_COUNT)
#define LINUX_DRBD_MAJOR				147
#define LINUX_RTF_MAJOR					150
#define LINUX_RAW_MAJOR					162
#define LINUX_USB_ACM_MAJOR				166
#define LINUX_USB_ACM_AUX_MAJOR			167
#define LINUX_USB_CHAR_MAJOR			180
#define LINUX_MMC_BLOCK_MAJOR			179
#define LINUX_VXVM_MAJOR				199		/* VERITAS volume i/o driver    */
#define LINUX_VXSPEC_MAJOR				200		/* VERITAS volume config driver */
#define LINUX_VXDMP_MAJOR				201		/* VERITAS volume multipath driver */
#define LINUX_XENVBD_MAJOR				202		/* Xen virtual block device */
#define LINUX_MSR_MAJOR					202
#define LINUX_CPUID_MAJOR				203
#define LINUX_OSST_MAJOR				206		/* OnStream-SCx0 SCSI tape */
#define LINUX_IBM_TTY3270_MAJOR			227
#define LINUX_IBM_FS3270_MAJOR			228
#define LINUX_VIOTAPE_MAJOR				230
#define LINUX_BLOCK_EXT_MAJOR			259
#define LINUX_SCSI_OSD_MAJOR			260		/* open-osd's OSD scsi device */

//-----------------------------------------------------------------------------

#endif		// __LINUX_MAJOR_H_