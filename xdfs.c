#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/string.h>
#include <linux/fs.h>
#include <asm/atomic.h>
#include <linux/cred.h>
#include <linux/statfs.h>
#include <asm/uaccess.h> 		/* copy_to_user */
#include <linux/buffer_head.h>
#include <linux/sched.h>
#include <linux/device.h>
#include <linux/fcntl.h>
#include <linux/unistd.h>
#include <linux/termios.h>
/* inode */
#include <linux/delay.h>
#include <linux/export.h>
#include <linux/mm.h>
#include <linux/backing-dev.h>
#include <linux/hash.h>
#include <linux/swap.h>
#include <linux/security.h>
#include <linux/cdev.h>
#include <linux/fsnotify.h>
#include <linux/mount.h>
#include <linux/posix_acl.h>
#include <linux/prefetch.h>
#include <linux/iversion.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("hengG");


typedef int8_t   INT8;
typedef int16_t  INT16;
typedef int32_t  INT32;
typedef int64_t  INT64;

typedef uint8_t  UINT8;
typedef uint16_t UINT16;
typedef uint32_t UINT32;
typedef uint64_t UINT64;

#define A_BYTE 1
#define BITS_OF_BYTE 8	/* A byte has 8 bits */

/* A inode has a state, it may be one of the TO_BE_DELETED, ALLOCATED or FREE */
#define TO_BE_DELETED 2
#define ALLOCATED 1
#define FREE 0

#define SB_OFFSET 32767	/* The offset of xdfs_superblock in blocks */ 

#define XDFS_NAMELEN 28	/* The max length of file name */	

#define XDFS_DIRECT_BLOCKS 16	/* The max num of blks a inode can have */
#define XDFS_BSIZE 4096			/* A blk has 4096 bytes */
#define XDFS_BSIZE_BITS 32768	/* 4096 bytes equal 32768 bits */
#define XDFS_MAGIC 0x19911008	/* This value is not suitable!!! */

/* The filesystem has a state, it may be one of the XDFS_DIRTY or XDFS_CLEAN */
#define XDFS_DIRTY 1
#define XDFS_CLEAN 0

#define XDFS_MAXBLOCKS 32768	/* The filesystem has 32768 blks */
#define XDFS_INODE_BLOCK 5		/* The INODE_SET starts from 5th blk(count from 0) */
#define XDFS_ROOT_INO 2			/* This value should equal the num of non-data zones */
#define XDFS_MAXFILES 4096		/* The filesystem has 4096 inodes */

/* The next two item should be replaced by XDFS_BSIZE and XDFS_MAXBLOCKS */
const int blksize = 4096;	/* Bits of one block */
const int nblks = 32768;	/* All occupy 32768 blocks，128MB */

#define PBR_JMP 0x00           	/* x86 jump instruction (3 bytes) */
#define PBR_SYS_ID 0x03        	/* System ID string     (8 bytes) */
#define PBR_BYTES_PER_BLK 0x0b 	/* Bytes per block      (2 bytes) */
#define PBR_NUM_BLKS 0x20      	/* Bytes per block(error value)    (4 bytes) */
#define PBR_NBLKS 1            	/* PBR so small 　*/

#define FSB_NBLKS 1	/* 4096 * 8bits = 37268bits <---> 32768blks */

#define FIB_EFFECTIVE_BYTES 512	/* Effective num of Inode Bitmap : 512 Bytes = 4096bits / 8bits */
#define FIB_NBLKS 1 			/* The inodeBitmap occupies 1 block */

#define INODE_SET_NBLKS 120 /* The area of pratical inode : 120 blocks(error value) */
#define INODE_NUMS 4096     /* The num of inode is 4096 */

#define INODE_LOG_NBLKS 1	/* The FSB_METABITMAP or FIB_METABITMAP occupies 1 blk */

#define GLOBAL_INFO_NBLKS 1 /* The GLOBAL_INFO occupies 1 blk */

#define SUPERBLOCK_NBLKS 1    	/* The SUPERBLOCK occupies 1 blk */
#define XDFS_ID_STRING "xdfs" 	/* The name of xdfs*/

struct xdfs_inode
{
    umode_t mode;						/* IS directory?  */
    uid_t uid; 						/* User id */
    gid_t gid;						/* User group id */
    unsigned long inode_no;				/* Stat data, not accessed from path walking, the unique label of the inode */
    unsigned int num_link;				/* The num of the hard link  */
    loff_t file_size;					/* The file size in bytes */
    struct timespec ctime;      		/* The last time the file attributes */
    struct timespec mtime;      		/* The last time the file data was changed */
    struct timespec atime;      		/* The last time the file data was accessed */
    unsigned int block_size_in_bit;				/* The size of the block in bits */
    blkcnt_t using_block_num;					/* The num of blks file using */
    unsigned long state;				/* State flag  */
    atomic_t inode_count;					/* Inode reference count  */

    UINT32 addr[XDFS_DIRECT_BLOCKS]	/* Store the address */
};

struct xdfs_superblock {
    UINT32        s_magic;
    UINT32        s_state;	/* The superblock state : XDFS_DIRTY or XDFS_CLEAN */
    UINT32        s_nifree;	/* The num of free inode */
    UINT32        s_inode;	/* The number of FIB0 block */
    UINT32        s_nbfree;	/* The num of free blk */
    UINT32        s_block;	/* The num of blocks */
};

static int xdfs_fill_super(struct super_block *sb, void *data, int silent)
{
	printk("XDFS: xdfs_fill_super start(%x, %x, %d)\n ", sb, data, silent);
	
	struct buffer_head *bh;
	struct xdfs_superblock *xdfs_sb;
	struct xdfs_info * xdfs;
	struct inode *inode_rt;
	dev_t  dev; 
	struct block_device * bdev;
	/* struct dax_device *dax_dev = blkdev_get_by_dev(sb->s_bdev); */

	/* bdev problem doesn't be solved */
	bdev = sb->s_bdev;
	if(IS_ERR(bdev))
		return -ENOMEM;
	else
		printk(" XDFS: RIGHT in xdfs_fill_super(s->s_bdev = %x, block_size = %x)\n", bdev, bdev->bd_block_size);
	
	/* set superblock parameter and read xdfs_superblock from the device */
	sb_set_blocksize(sb, XDFS_BSIZE);	
	sb->s_blocksize = XDFS_BSIZE;
	sb->s_blocksize_bits = XDFS_BSIZE_BITS;
	printk("XDFS: sb_bread starting\n");
	msleep(5000);
	bh = sb_bread(sb, SB_OFFSET);
	printk("XDFS: sb_bread ending\n");
	msleep(5000);

	/* exam the data read from the device and written by mkfs.c */
	xdfs_sb = (struct xdfs_superblock *)bh->b_data;

	/* fill superblock and xdfs_superblock */
	xdfs = (struct xdfs_info *)kvmalloc(sizeof(struct xdfs_info), GFP_KERNEL);
	xdfs->xdfs_ifsb = xdfs_sb;
	xdfs->xdfs_ifbh = bh;
	
    sb->s_fs_info = xdfs;
    sb->s_magic = XDFS_MAGIC;

	printk("XDFS: op starting\n");
	msleep(5000);
	sb->s_op = &xdfs_sops;
	printk("XDFS: op ending\n");
	msleep(5000);
	sb->s_dev = dev;
	
	/* get a new vnode */
	printk("XDFS: iget fronting\n");
	msleep(5000);	
    inode_rt = xdfs_iget(sb, XDFS_ROOT_INO);	/* need new function written by GuoHeng, need a error examine */
	printk("XDFS: iget fronting\n");
	
	/* make root directory */
	printk("XDFS: d_make_root starting\n");
	msleep(5000);
	sb->s_root = d_make_root(inode_rt);
    if (!sb->s_root)
	{
            iput(inode_rt);	/* The inode is malloced before, now it should be free */
            printk("XDFS: xdfs_fill_super out return -ENOMEM\n");
        	return -ENOMEM;
	}
	
	printk("XDFS: syncfsfronting\n");
	xdfs_sync_fs(sb, 1);	/* 1: sync, 0: async */
	
	printk("XDFS: xdfs_fill_super exit successfully\n ");
	return 0;
}

static struct file_system_type xdfs_fs_type = {
  .owner = THIS_MODULE, 
  .name = "xdfs",
  .mount = xdfs_get_super,     
  .kill_sb = kill_block_super, //it is default, not provided by us.
  .fs_flags = FS_REQUIRES_DEV, //need to required a physical device. we use loop device
};

static struct dentry *xdfs_get_super(struct file_system_type *fs_type, int flags, const char *dev_name, void *data)
{
  printk("xdfs: mount begin");
  struct dentry *ret = mount_bdev(fs_type, flags, dev_name, data, xdfs_fill_super);
  printk("xdfs: mount success");
  return ret;
}

static int __init init_xdfs_fs(void)
{
  return register_filesystem(&xdfs_fs_type);
}

static void __exit exit_xdfs_fs(void)
{
  unregister_filesystem(&xdfs_fs_type);
}

module_init(init_xdfs_fs)
module_exit(exit_xdfs_fs)