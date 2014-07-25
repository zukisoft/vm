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

#ifndef __FILESYSTEM_H_
#define __FILESYSTEM_H_
#pragma once

#include <memory>

// stuff to remove
typedef char char_t;

#pragma warning(push, 4)

//-----------------------------------------------------------------------------
// Class FileSystem

class FileSystem
{
public:

	// #define FS_REQUIRES_DEV         1 
	// #define FS_BINARY_MOUNTDATA     2
	// #define FS_HAS_SUBTYPE          4
	// #define FS_USERNS_MOUNT         8       /* Can be mounted by userns root */
	// #define FS_USERNS_DEV_MOUNT     16 /* A userns mount does not imply MNT_NODEV */
	// #define FS_RENAME_DOES_D_MOVE   32768   /* FS will handle d_move() during rename() internally. */
	enum class Flags
	{
	};

	// Destructor
	//
	virtual ~FileSystem()=default;

	class __declspec(novtable) Directory
	{
	public:

		// Destructor
		//
		virtual ~Directory()=default;

		// int (*d_revalidate)(struct dentry *, unsigned int);
		// int (*d_weak_revalidate)(struct dentry *, unsigned int);
		// int (*d_hash)(const struct dentry *, struct qstr *);
		// int (*d_compare)(const struct dentry *, const struct dentry *, unsigned int, const char *, const struct qstr *);
		// int (*d_delete)(const struct dentry *);
		// void (*d_release)(struct dentry *);
		// void (*d_prune)(struct dentry *);
		// void (*d_iput)(struct dentry *, struct inode *);
		// char *(*d_dname)(struct dentry *, char *, int);
		// struct vfsmount *(*d_automount)(struct path *);
		// int (*d_manage)(struct dentry *, bool);

	protected:

		Directory()=default;

	private:

		Directory(const Directory&)=delete;
		Directory& operator=(const Directory&)=delete;
	};

	class __declspec(novtable) Node
	{
	public:

		// Destructor
		//
		virtual ~Node()=default;

		// struct dentry * (*lookup) (struct inode *,struct dentry *, unsigned int);
		// void * (*follow_link) (struct dentry *, struct nameidata *);
		// int (*permission) (struct inode *, int);
		// struct posix_acl * (*get_acl)(struct inode *, int);
		// int (*readlink) (struct dentry *, char __user *,int);
		// void (*put_link) (struct dentry *, struct nameidata *, void *);
		// int (*create) (struct inode *,struct dentry *, umode_t, bool);
		// int (*link) (struct dentry *,struct inode *,struct dentry *);
		// int (*unlink) (struct inode *,struct dentry *);
		// int (*symlink) (struct inode *,struct dentry *,const char *);
		// int (*mkdir) (struct inode *,struct dentry *,umode_t);
		// int (*rmdir) (struct inode *,struct dentry *);
		// int (*mknod) (struct inode *,struct dentry *,umode_t,dev_t);
		// int (*rename) (struct inode *, struct dentry *, struct inode *, struct dentry *);
		// int (*rename2) (struct inode *, struct dentry *, struct inode *, struct dentry *, unsigned int);
		// int (*setattr) (struct dentry *, struct iattr *);
		// int (*getattr) (struct vfsmount *mnt, struct dentry *, struct kstat *);
		// int (*setxattr) (struct dentry *, const char *,const void *,size_t,int);
		// ssize_t (*getxattr) (struct dentry *, const char *, void *, size_t);
		// ssize_t (*listxattr) (struct dentry *, char *, size_t);
		// int (*removexattr) (struct dentry *, const char *);
		// int (*fiemap)(struct inode *, struct fiemap_extent_info *, u64 start, u64 len);
		// int (*update_time)(struct inode *, struct timespec *, int);
		// int (*atomic_open)(struct inode *, struct dentry *, struct file *, unsigned open_flag, umode_t create_mode, int *opened);
		// int (*tmpfile) (struct inode *, struct dentry *, umode_t);
		// int (*set_acl)(struct inode *, struct posix_acl *, int);

		// Index
		//
		// Gets the node index
		__declspec(property(get=getIndex)) uint32_t Index;
		uint32_t getIndex(void) const { return m_index; }

	protected:

		Node(uint32_t index) : m_index(index)
		{
		}

		uint32_t m_index;

	private:

		Node(const Node&)=delete;
		Node& operator=(const Node&)=delete;

	};

	class __declspec(novtable) File
	{
	public:

		// Destructor
		//
		virtual ~File()=default;

		// struct module *owner;
		// loff_t (*llseek) (struct file *, loff_t, int);
		// ssize_t (*read) (struct file *, char __user *, size_t, loff_t *);
		// ssize_t (*write) (struct file *, const char __user *, size_t, loff_t *);
		// ssize_t (*aio_read) (struct kiocb *, const struct iovec *, unsigned long, loff_t);
		// ssize_t (*aio_write) (struct kiocb *, const struct iovec *, unsigned long, loff_t);
		// int (*iterate) (struct file *, struct dir_context *);
		// unsigned int (*poll) (struct file *, struct poll_table_struct *);
		// long (*unlocked_ioctl) (struct file *, unsigned int, unsigned long);
		// long (*compat_ioctl) (struct file *, unsigned int, unsigned long);
		// int (*mmap) (struct file *, struct vm_area_struct *);
		// int (*open) (struct inode *, struct file *);
		// int (*flush) (struct file *, fl_owner_t id);
		// int (*release) (struct inode *, struct file *);
		// int (*fsync) (struct file *, loff_t, loff_t, int datasync);
		// int (*aio_fsync) (struct kiocb *, int datasync);
		// int (*fasync) (int, struct file *, int);
		// int (*lock) (struct file *, int, struct file_lock *);
		// ssize_t (*sendpage) (struct file *, struct page *, int, size_t, loff_t *, int);
		// unsigned long (*get_unmapped_area)(struct file *, unsigned long, unsigned long, unsigned long, unsigned long);
		// int (*check_flags)(int);
		// int (*flock) (struct file *, int, struct file_lock *);
		// ssize_t (*splice_write)(struct pipe_inode_info *, struct file *, loff_t *, size_t, unsigned int);
		// ssize_t (*splice_read)(struct file *, loff_t *, struct pipe_inode_info *, size_t, unsigned int);
		// int (*setlease)(struct file *, long, struct file_lock **);
		// long (*fallocate)(struct file *file, int mode, loff_t offset, loff_t len);
		// int (*show_fdinfo)(struct seq_file *m, struct file *f);

	protected:

		File()=default;

	private:

		File(const File&)=delete;
		File& operator=(const File&)=delete;
	};

	///------------------



	// instance methods
	//
	// struct inode *(*alloc_inode)(struct super_block *sb);
	// void (*destroy_inode)(struct inode *);
	// void (*dirty_inode) (struct inode *, int flags);
	// int (*write_inode) (struct inode *, struct writeback_control *wbc);
	// int (*drop_inode) (struct inode *);
	// void (*evict_inode) (struct inode *);
	// void (*put_super) (struct super_block *);
	// int (*sync_fs)(struct super_block *sb, int wait);
	// int (*freeze_fs) (struct super_block *);
	// int (*unfreeze_fs) (struct super_block *);
	// int (*statfs) (struct dentry *, struct kstatfs *);
	// int (*remount_fs) (struct super_block *, int *, char *);
	// void (*umount_begin) (struct super_block *);
	// int (*show_options)(struct seq_file *, struct dentry *);
	// int (*show_devname)(struct seq_file *, struct dentry *);
	// int (*show_path)(struct seq_file *, struct dentry *);
	// int (*show_stats)(struct seq_file *, struct dentry *);
	// ssize_t (*quota_read)(struct super_block *, int, char *, size_t, loff_t);
	// ssize_t (*quota_write)(struct super_block *, int, const char *, size_t, loff_t);
	// int (*bdev_try_to_free_page)(struct super_block*, struct page*, gfp_t);
	// long (*nr_cached_objects)(struct super_block *, int);
	// long (*free_cached_objects)(struct super_block *, long, int);

protected:

	FileSystem()=default;

private:

	FileSystem(const FileSystem&)=delete;
	FileSystem& operator=(const FileSystem&)=delete;
	
	// static methods/member variables
	//
	// --> const char *name;
	// --> int fs_flags;
	
	// becomes Mount() in derived class
	// --> struct dentry *(*mount) (struct file_system_type *, int flags, const char * devname, void * data);

	// becomes Destructor
	// --> void (*kill_sb) (struct super_block *);
	
	// stuff I'm not sure I will need
	//
	// struct module *owner;
	// struct file_system_type * next;
	// struct hlist_head fs_supers;
	// struct lock_class_key s_lock_key;
	// struct lock_class_key s_umount_key;
	// struct lock_class_key s_vfs_rename_key;
	// struct lock_class_key s_writers_key[SB_FREEZE_LEVELS];
	// struct lock_class_key i_lock_key;
	// struct lock_class_key i_mutex_key;
	// struct lock_class_key i_mutex_dir_key;
};

//-----------------------------------------------------------------------------

#pragma warning(pop)

#endif	// __FILESYSTEM_H_