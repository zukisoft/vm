//-----------------------------------------------------------------------------
// Copyright (c) 2015 Michael G. Brehm
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

#ifndef __LINUX_MAGIC_H_
#define LINUX___LINUX_MAGIC_H_
#pragma once

//-----------------------------------------------------------------------------
// include/uapi/linux/magic.h
//-----------------------------------------------------------------------------

#define LINUX_ADFS_SUPER_MAGIC					0xadf5
#define LINUX_AFFS_SUPER_MAGIC					0xadff
#define LINUX_AFS_SUPER_MAGIC					0x5346414F
#define LINUX_AUTOFS_SUPER_MAGIC				0x0187
#define LINUX_CODA_SUPER_MAGIC					0x73757245
#define LINUX_CRAMFS_MAGIC						0x28cd3d45	/* some random number */
#define LINUX_CRAMFS_MAGIC_WEND					0x453dcd28	/* magic number with the wrong endianess */
#define LINUX_DEBUGFS_MAGIC						0x64626720
#define LINUX_SECURITYFS_MAGIC					0x73636673
#define LINUX_SELINUX_MAGIC						0xf97cff8c
#define LINUX_SMACK_MAGIC						0x43415d53	/* "SMAC" */
#define LINUX_RAMFS_MAGIC						0x858458f6	/* some random number */
#define LINUX_TMPFS_MAGIC						0x01021994
#define LINUX_HUGETLBFS_MAGIC					0x958458f6	/* some random number */
#define LINUX_SQUASHFS_MAGIC					0x73717368
#define LINUX_ECRYPTFS_SUPER_MAGIC				0xf15f
#define LINUX_EFS_SUPER_MAGIC					0x414A53
#define LINUX_EXT2_SUPER_MAGIC					0xEF53
#define LINUX_EXT3_SUPER_MAGIC					0xEF53
#define LINUX_XENFS_SUPER_MAGIC					0xabba1974
#define LINUX_EXT4_SUPER_MAGIC					0xEF53
#define LINUX_BTRFS_SUPER_MAGIC					0x9123683E
#define LINUX_NILFS_SUPER_MAGIC					0x3434
#define LINUX_F2FS_SUPER_MAGIC					0xF2F52010
#define LINUX_HPFS_SUPER_MAGIC					0xf995e849
#define LINUX_ISOFS_SUPER_MAGIC					0x9660
#define LINUX_JFFS2_SUPER_MAGIC					0x72b6
#define LINUX_PSTOREFS_MAGIC					0x6165676C
#define LINUX_EFIVARFS_MAGIC					0xde5e81e4
#define LINUX_HOSTFS_SUPER_MAGIC				0x00c0ffee
#define LINUX_MINIX_SUPER_MAGIC					0x137F		/* minix v1 fs, 14 char names */
#define LINUX_MINIX_SUPER_MAGIC2				0x138F		/* minix v1 fs, 30 char names */
#define LINUX_MINIX2_SUPER_MAGIC				0x2468		/* minix v2 fs, 14 char names */
#define LINUX_MINIX2_SUPER_MAGIC2				0x2478		/* minix v2 fs, 30 char names */
#define LINUX_MINIX3_SUPER_MAGIC				0x4d5a		/* minix v3 fs, 60 char names */
#define LINUX_MSDOS_SUPER_MAGIC					0x4d44		/* MD */
#define LINUX_NCP_SUPER_MAGIC					0x564c		/* Guess, what 0x564c is :-) */
#define LINUX_NFS_SUPER_MAGIC					0x6969
#define LINUX_OPENPROM_SUPER_MAGIC				0x9fa1
#define LINUX_QNX4_SUPER_MAGIC					0x002f		/* qnx4 fs detection */
#define LINUX_QNX6_SUPER_MAGIC					0x68191122	/* qnx6 fs detection */
#define LINUX_REISERFS_SUPER_MAGIC				0x52654973	/* used by gcc */
#define LINUX_REISERFS_SUPER_MAGIC_STRING		"ReIsErFs"
#define LINUX_REISER2FS_SUPER_MAGIC_STRING		"ReIsEr2Fs"
#define LINUX_REISER2FS_JR_SUPER_MAGIC_STRING	"ReIsEr3Fs"
#define LINUX_SMB_SUPER_MAGIC					0x517B
#define LINUX_CGROUP_SUPER_MAGIC				0x27e0eb
#define LINUX_STACK_END_MAGIC					0x57AC6E9D
#define LINUX_TRACEFS_MAGIC						0x74726163
#define LINUX_V9FS_MAGIC						0x01021997
#define LINUX_BDEVFS_MAGIC						0x62646576
#define LINUX_BINFMTFS_MAGIC					0x42494e4d
#define LINUX_DEVPTS_SUPER_MAGIC				0x1cd1
#define LINUX_FUTEXFS_SUPER_MAGIC				0xBAD1DEA
#define LINUX_PIPEFS_MAGIC						0x50495045
#define LINUX_PROC_SUPER_MAGIC					0x9fa0
#define LINUX_SOCKFS_MAGIC						0x534F434B
#define LINUX_SYSFS_MAGIC						0x62656572
#define LINUX_USBDEVICE_SUPER_MAGIC				0x9fa2
#define LINUX_MTD_INODE_FS_MAGIC				0x11307854
#define LINUX_ANON_INODE_FS_MAGIC				0x09041934
#define LINUX_BTRFS_TEST_MAGIC					0x73727279
#define LINUX_NSFS_MAGIC						0x6e736673

//-----------------------------------------------------------------------------

#endif		// __LINUX_MAGIC_H_