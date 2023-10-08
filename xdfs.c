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
#include <linux/time.h>			/* struct timespec */
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
const int BLKSIZE = 4096;	/* Bits of one block */
const int NUM_BLK = 32768;	/* All occupy 32768 blocks，128MB */

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

static struct xdfs_inode
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

    UINT32 addr[XDFS_DIRECT_BLOCKS];	/* Store the address */
};

static struct xdfs_superblock {
    UINT32        s_magic;
    UINT32        s_state;	/* The superblock state : XDFS_DIRTY or XDFS_CLEAN */
    UINT32        s_nifree;	/* The num of free inode */
    UINT32        s_inode;	/* The number of FIB0 block */
    UINT32        s_nbfree;	/* The num of free blk */
    UINT32        s_block;	/* The num of blocks */
	struct buffer_head* bh;
};

static struct address_space_operations	xdfs_aops;
static struct file_operations 			xdfs_dir_operations;
static struct inode_operations 		xdfs_dir_inops;
static static struct file_operations 			xdfs_file_operations;
static struct inode_operations 		xdfs_file_inops;
static struct file_system_type xdfs_fs_type;

static int 
xdfs_fill_super(struct super_block *sb, void *data, int silent)
{
	printk("XDFS: xdfs_fill_super start(%p, %p, %d)\n ", sb, data, silent);
	
	struct buffer_head *bh;
	struct xdfs_superblock *xdfs_sb;
	struct inode *inode_rt;
	dev_t  dev; 
	struct block_device * bdev;
	/* struct dax_device *dax_dev = blkdev_get_by_dev(sb->s_bdev); */

	/* bdev problem doesn't be solved */
	bdev = sb->s_bdev;
	if(IS_ERR(bdev))
		return -ENOMEM;
	else
		printk(" XDFS: RIGHT in xdfs_fill_super(s->s_bdev = %p, block_size = %x)\n", bdev, bdev->bd_block_size);
	
	/* set superblock parameter and read xdfs_superblock from the device */
	printk("XDFS: sb_bread starting\n");
	bh = sb_bread(sb, BLKSIZE*(32768-1));
	sb->s_blocksize = BLKSIZE;
	sb->s_blocksize_bits = NUM_BLK;
	msleep(5000);

	/* exam the data read from the device and written by mkfs.c */
	xdfs_sb = (struct xdfs_superblock *)bh->b_data;
	xdfs_sb->bh = bh;
	
    sb->s_fs_info = xdfs_sb;
    sb->s_magic = XDFS_MAGIC;

	printk("XDFS: op starting\n");
	msleep(5000);
	sb->s_op = &xdfs_sops;
	sb->s_dev = dev;
	
	/* get a new vnode */
	printk("XDFS: iget fronting\n");
	msleep(5000);	
    inode_rt = xdfs_iget(sb, XDFS_ROOT_INO);	/* need new function written by GuoHeng, need a error examine */
	
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
	
	printk("XDFS: sync_fs begin\n");
	xdfs_sync_fs(sb, 1);	/* 1: sync, 0: async */
	printk("XDFS: xdfs_fill_super exit successfully\n ");
	return 0;
}
static struct inode *
xdfs_iget(struct super_block *sb, unsigned long ino)
{
	
	struct inode *inod;
	struct buffer_head *bh;
    struct xdfs_inode *raw_inode;                
	int block;
	uid_t i_uid;
	gid_t i_gid;
	printk("XDFS: xdfs_iget(%p,%d)\n",sb,ino);
	msleep(2000);
	
	inod = iget_locked(sb, ino);
	
	printk("XDFS:iget_locked ending\n",sb,ino);
	msleep(5000);
	if (!inod)
		return ERR_PTR(-ENOMEM);
	if (!(inod->i_state & I_NEW))      
		return inod;   
	printk("XDFS:xdfs_iget(%p,%d) new inode ino=%d\n",sb,ino);
	block = XDFS_INODE_BLOCK + ino;
	bh = sb_bread(inod->i_sb,block);
	raw_inode = (struct xdfs_inode *)(bh->b_data);   //play the role of ext2_get_inode其实，这样读，还有风险，风险就是：可能存在字节序的情况，所以，像ext3等都要考虑字节序
	
	inod->i_mode = raw_inode->mode;
	if(S_ISREG(inod->i_mode))
	{
		printk("XDFS: it is a common file\n");
	    inod->i_mode |= S_IFREG;
	    inod->i_op = &xdfs_file_inops;
	    inod->i_fop = &xdfs_file_operations;
	    inod->i_mapping->a_ops = &xdfs_aops;  
	}
	else
	{
		printk("XDFS: it is a directory\n");
		inod->i_mode |= S_IFDIR;
		inod->i_op = &xdfs_dir_inops;
		inod->i_fop = &xdfs_dir_operations;
	}
    inod->i_uid.val = raw_inode->uid;
    inod->i_gid.val = raw_inode->gid;
    set_nlink(inod,raw_inode->i_count.counter);
    inod->i_size = raw_inode->i_size;
    inod->i_blocks = raw_inode->blocks;
	inod->i_blkbits = NUM_BLK; 
	
	inod->i_private=(void*)kvmalloc(sizeof(struct xdfs_inode),GFP_KERNEL);   //内核分配
	memcpy(inod->i_private, raw_inode, sizeof(struct xdfs_inode));    //raw_inode:struct xdfs_incore_inode类型
	brelse(bh);
	inod->i_flags &= ~(S_SYNC | S_APPEND | S_IMMUTABLE | S_NOATIME |
                      S_DIRSYNC | S_DAX);
  	inod->i_flags |= S_SYNC;
	printk("XDFS: xdfs_iget ending\n");
  	return inod;
} 
static int 
xdfs_sync_fs(struct super_block *sb, int wait)
{
	printk("XDFS: xdfs_sync_fs(%x) start \n", sb);
	struct buffer_head *bh = (strcut xdfs_superblock*)(sb->s_fs_info)->bh;
	
	/* Is superblock read only? */
	if(!sb_rdonly(sb))
	{
		/* mark it dirty */	
		mark_buffer_dirty(bh);
		if(wait)
			sync_dirty_buffer(bh);

		printk("XDFS: xdfs_sync_fs(%x) exit \n",sb);
		return 0;
	}
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
  printk("XDFS: mount begin");
  struct dentry *ret = mount_bdev(fs_type, flags, dev_name, data, xdfs_fill_super);
  printk("XDFS: mount success");
  return ret;
}

static int __init init_xdfs_fs(void)
{
  return register_filesystem(&xdfs_fs_type);
}
MODULE_ALIAS_FS("xdfs");

static void __exit exit_xdfs_fs(void)
{
  unregister_filesystem(&xdfs_fs_type);
}

/* operations set  and achievement*/
// all is commented to test mount ? all achieve printk to test mount(used)
struct super_operations xdfs_sops = {
    //read_inode : xdfs_read_inode,//2.6没有这一项 已经和iget合并为my_iget
	//alloc_inode : xdfs_alloc_inode,
	write_inode : xdfs_write_inode,
	evict_inode : xdfs_evict_inode,
	put_super : xdfs_put_super,
	sync_fs : xdfs_write_super,
	statfs : xdfs_statfs,
    //drop_inode: xdfs_delete_inode,//generic_drop_inode,//如果这里不定义的话 系统会自动调用generic_drop_inode
};
int 
xdfs_write_inode(struct inode *inode, struct writeback_control *wbc)
{
	printk("XDFS: write_inode is called inode is (%p) wbc is (%p)\n",inode,wbc);
	return 0;
}
void 
xdfs_evict_inode(struct inode * inode)
{
	printk("XDFS: evict_inode is called inode is (%p)\n",inode);
	return;
}
void 
xdfs_put_super(struct super_block *s)
{
	printk("XDFS: xdfs_put_super(%p) is called\n",s);
	struct buffer_head *bh = (strcut xdfs_superblock*)(sb->s_fs_info)->bh;
	brelse(bh);
	printk("XDFS: xdfs_put_super(%p) return\n",s);
}
int 
xdfs_write_super(struct super_block *sb,int wait)
{
	printk("XDFS: xdfs_write_super(%p) is called\n",sb);
	struct buffer_head *bh = (strcut xdfs_superblock*)(sb->s_fs_info)->bh;

	mark_buffer_dirty(bh);
	printk("XDFS: xdfs_write_super(%p) return\n",sb);
	return 0;
}
int 
xdfs_statfs(struct dentry *dentry, struct kstatfs * buf)
{
	printk("XDFS: statfs is called dentry is (%p),kstatfs is (%p)\n",dentry,buf);
	return 0;
}
/* operations about file,inode,address*/

struct address_space_operations xdfs_aops = {
    //是否需要
    // readpage : my_readpage,
    // writepage : my_writepage,
    // sync_page : block_sync_page,
    // prepare_write : my_prepare_write,
    // commit_write : generic_commit_write,
    // bmap : my_bmap,
};
struct inode_operations xdfs_file_inops = {
    //这个地方我觉得编写的有问题//是否需要
    //link : my_link,
    //unlink : my_unlink,
};
struct file_operations xdfs_file_operations = {
  open : xdfs_open,
  read_iter : xdfs_read_iter,
  write_iter : xdfs_write_iter,
  splice_read : xdfs_splice_read,
  splice_write : xdfs_splice_write,
  llseek : xdfs_file_llseek,
  mmap : xdfs_file_mmap,
  //  aio_read : generic_file_aio_read,
  //  aio_write : generic_file_aio_write,
  fsync : xdfs_file_fsync, //给文件里面也加一个fsync就可以用vi 修改文件实现同步。
};

static int 
xdfs_open(struct inode *inode, struct file *filp)
{
  printk("XDFS: open is called\n");
  int error;
  error = generic_file_open(inode, filp);
  printk("XDFS: open return\n");
  return error;
}

static ssize_t 
xdfs_read_iter(struct kiocb *iocb, struct iov_iter *to)
{
  printk("XDFS: read_iter is called\n");
  ssize_t error;
  error = generic_file_read_iter(iocb, to);
  printk("XDFS: read_iter return\n");
  return error;
}
static ssize_t 
xdfs_write_iter(struct kiocb *iocb, struct iov_iter *from)
{
  printk("XDFS: write_iter is called\n");
  ssize_t error;
  error = generic_file_write_iter(iocb, from);
  printk("XDFS: write_iter return\n");
  return error;
}

static ssize_t 
xdfs_splice_read(struct file *in, loff_t *ppos,
				 struct pipe_inode_info *pipe, size_t len,
				 unsigned int flags)
{
  printk("XDFS: splice_read is called\n");
  ssize_t error;
  error = generic_file_read_iter(in, ppos,pipe,len,flags);
  printk("XDFS: splice_read return\n");
  return error;
}

static ssize_t
xdfs_splice_write(struct pipe_inode_info *pipe, struct file *out,
			  loff_t *ppos, size_t len, unsigned int flags)
{
  printk("XDFS: write is called\n");
  ssize_t error;
  error = iter_file_splice_write(ppos,out,pipe,len,flags);
  printk("XDFS: splice_writsplice_e return\n");
  return error;
}
static loff_t 
xdfs_file_llseek(struct file *file, loff_t offset, int whence)
{
  printk("XDFS: file_llseek is called\n");
  loff_t error;
  error = generic_file_llseek(file,offset,whence);
  printk("XDFS: file_llseek return\n");
  return error;
}

static int 
xdfs_file_mmap(struct file *file, struct vm_area_struct *vma)
{
  printk("XDFS: file_mmap is called\n");
  int error;
  error = generic_file_mmap(file,vma);
  printk("XDFS: file_mmap return\n");
  return error;
}

static int 
xdfs_file_fsync(struct file *file, loff_t start, loff_t end, int datasync)
{
  printk("XDFS: file_fsync is called\n");
  int error;
  error = generic_file_sync(file,vma);
  printk("XDFS: file_fsync return\n");
  return error;
}
module_init(init_xdfs_fs)
module_exit(exit_xdfs_fs)