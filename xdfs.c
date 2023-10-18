#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/time64.h>			/* timespec*/
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
/* VA_ARGS */
#include<linux/stdarg.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("hengG");

#define XDFS_DEBUG
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
const int BLKSIZE = 4096;	/* 一个块4KB */ 
const unsigned BLKSIZE_BITS = 21;
const int NUM_BLK = 32768;	/* 一共32768个块，128MB */ 

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
    struct timespec64 ctime;      		/* The last time the file attributes */
    struct timespec64 mtime;      		/* The last time the file data was changed */
    struct timespec64 atime;      		/* The last time the file data was accessed */
    unsigned int block_size_in_bit;				/* The size of the block in bits */
    blkcnt_t using_block_num;					/* The num of blks file using */
    unsigned long state;				/* State flag  */
    atomic_t inode_count;					/* Inode reference count  */

    UINT32 addr[XDFS_DIRECT_BLOCKS];	/* Store the address */
	struct inode vfs_inode;

}XDFS_INODE;

struct xdfs_superblock {
    UINT32        s_magic;
    UINT32        s_state;	/* The superblock state : XDFS_DIRTY or XDFS_CLEAN */
    UINT32        s_nifree;	/* The num of free inode */
    UINT32        s_inode;	/* The number of FIB0 block */
    UINT32        s_nbfree;	/* The num of free blk */
    UINT32        s_block;	/* The num of blocks */
	struct buffer_head* bh;
};
#define XDFS_DIR_ENTRY_BONDRY (4)
#define XDFS_DIR_ENTRY_MASK (XDFS_DIR_ENTRY_BONDRY-1)
#define XDFS_DIR_ENTRY_TOTAL_LEN(name_len)	(((name_len) + 8 + XDFS_DIR_ENTRY_MASK) & \
					 ~XDFS_DIR_ENTRY_MASK)
#define XDFS_MAX_ENTRY_TOTAL_LEN		((1<<16)-1)
/* size of dir_entry is 3 bytes(32 bits)*/
struct xdfs_dir_entry{
	UINT32 inode_no;
	UINT16 dir_entry_len; 
	UINT8 name_len; 
	UINT8 file_type; 
	char name[]; 
};

static struct super_operations 		   xdfs_sops;
static struct address_space_operations xdfs_aops;
static struct file_operations          xdfs_file_dir_operations;
static struct inode_operations         xdfs_inode_dir_operations;
static struct file_operations          xdfs_file_operations;
static struct inode_operations         xdfs_inode_operations;
static struct file_system_type         xdfs_fs_type;
/*all func declaration*/
static int xdfs_fill_super(struct super_block *sb, void *data, int silent);
static struct inode *xdfs_iget(struct super_block *sb, unsigned long ino);
static int xdfs_sync_fs(struct super_block *sb, int wait);
static struct dentry *xdfs_get_super(struct file_system_type *fs_type, int flags, const char *dev_name, void *data);

static struct inode *xdfs_alloc_inode(struct super_block *sb);

static int xdfs_write_inode(struct inode *inode, struct writeback_control *wbc);
static void xdfs_evict_inode(struct inode * inode);
static void xdfs_put_super(struct super_block *s);
static int xdfs_write_super(struct super_block *sb,int wait);
static int xdfs_statfs(struct dentry *dentry, struct kstatfs * buf);

static int xdfs_open(struct inode *inode, struct file *filp);
static ssize_t xdfs_read_iter(struct kiocb *iocb, struct iov_iter *to);
static ssize_t xdfs_write_iter(struct kiocb *iocb, struct iov_iter *from);
static ssize_t xdfs_splice_read(struct file *in, loff_t *ppos,
				 struct pipe_inode_info *pipe, size_t len,
				 unsigned int flags);
static ssize_t xdfs_splice_write(struct pipe_inode_info *pipe, struct file *out,
			  loff_t *ppos, size_t len, unsigned int flags);
static loff_t xdfs_file_llseek(struct file *file, loff_t offset, int whence);
static int xdfs_file_mmap(struct file *file, struct vm_area_struct *vma);
static int xdfs_file_fsync(struct file *file, loff_t start, loff_t end, int datasync);

static loff_t xdfs_generic_file_llseek(struct file *file, loff_t offset, int whence);
static ssize_t xdfs_generic_read_dir(struct file *filp, char __user *buf, size_t siz, loff_t *ppos);
static int xdfs_readdir(struct file *file, struct dir_context *ctx);
static struct page * xdfs_get_page(struct inode *dir, unsigned long n, int quiet, void **page_addr);
static inline void xdfs_put_page(struct page *page, void *page_addr);
static struct xdfs_dir_entry * xdfs_next_dir_entry(struct xdfs_dir_entry* de);




static void xdfs_kill_block_super(struct super_block *sb);
/* 通用函数 */
void xdfs_printk(const char * fmt, ...)
{
#ifdef XDFS_DEBUG
	va_list args;
	va_start(args,fmt);
	/* 忽略了 int 返回值 */
	printk("XDFS: ");
	vprintk(fmt,args);
	va_end(args);
#endif
}

/* 对接函数 */
static int 
xdfs_fill_super(struct super_block *sb, void *data, int silent)
{
	
	struct buffer_head *bh;
	struct xdfs_superblock *xdfs_sb;
	struct inode *inode_rt;
	dev_t  dev; 
	struct block_device * bdev;
	/* struct dax_device *dax_dev = blkdev_get_by_dev(sb->s_bdev); */
#ifdef XDFS_DEBUG
	printk("XDFS: xdfs_fill_super start(%p, %p, %d)\n ", sb, data, silent);
#endif
	/* alloc space */
	xdfs_sb = kzalloc(sizeof(struct xdfs_superblock), GFP_KERNEL);

	/* bdev problem doesn't be solved */
	bdev = sb->s_bdev;
	if(IS_ERR(bdev))
		return -ENOMEM;
	else
	{
		
#ifdef XDFS_DEBUG
		printk("XDFS: RIGHT in xdfs_fill_super(s->s_bdev = %p)\n", bdev);
#endif
	}
	
	/* set superblock parameter and read xdfs_superblock from the device */
#ifdef XDFS_DEBUG
	printk("XDFS: sb_bread starting\n");
#endif
	/* read superblock from disk*/
	bh = sb_bread(sb, (32768-1));
	xdfs_sb = (struct xdfs_superblock *)bh->b_data;
#ifdef XDFS_DEBUG
	printk("XDFS: xdfs_superblock-> =s_magic  %d\n",xdfs_sb->s_magic);
	printk("XDFS: xdfs_superblock-> =s_block  %d\n",xdfs_sb->s_block);
	printk("XDFS: xdfs_superblock-> =s_inode  %d\n",xdfs_sb->s_inode);
	printk("XDFS: xdfs_superblock-> =s_nbfree %d\n",xdfs_sb->s_nbfree);
	printk("XDFS: xdfs_superblock-> =s_nifree %d\n",xdfs_sb->s_nifree);
	printk("XDFS: xdfs_superblock-> =s_state  %d\n",xdfs_sb->s_state);
#endif
	xdfs_sb->bh = bh;
	msleep(1000);

	/* exam the data read from the device and written by mkfs.c */
	sb->s_blocksize = BLKSIZE;
	sb->s_blocksize_bits = BLKSIZE_BITS;
    sb->s_fs_info = xdfs_sb;
    sb->s_magic = XDFS_MAGIC;

#ifdef XDFS_DEBUG
	printk("XDFS: op starting\n");
	msleep(1000);
#endif
	sb->s_op = &xdfs_sops;
	sb->s_dev = dev;
	
	/* get a new vnode */
#ifdef XDFS_DEBUG
	printk("XDFS: iget fronting\n");
	msleep(1000);	
#endif
	/* alloc root inode and mount*/
    inode_rt = xdfs_iget(sb, XDFS_ROOT_INO);	/* need new function written by GuoHeng, need a error examine */
	
	/* make root directory */
#ifdef XDFS_DEBUG
	printk("XDFS: d_make_root starting\n");
	msleep(1000);
#endif
	sb->s_root = d_make_root(inode_rt);
    if (!sb->s_root)
	{
            iput(inode_rt);	/* The inode is malloced before, now it should be free */
#ifdef XDFS_DEBUG
            printk("XDFS: xdfs_fill_super out return -ENOMEM\n");
#endif
        	return -ENOMEM;
	}
	
#ifdef XDFS_DEBUG
	printk("XDFS: sync_fs begin\n");
#endif
	/* sync to disk */
	xdfs_sync_fs(sb, 1);	/* 1: sync, 0: async */
#ifdef XDFS_DEBUG
	printk("XDFS: xdfs_fill_super exit successfully\n ");
#endif
	return 0;
}
static struct inode *
xdfs_iget(struct super_block *sb, unsigned long ino)
{
	
	struct inode *inod;
	struct buffer_head *bh;
    struct xdfs_inode *raw_inode;                
	int block;
	long ret = -EIO;
#ifdef XDFS_DEBUG
	printk("XDFS: xdfs_iget(%p,%ld)\n",sb,ino);
	msleep(1000);
#endif
	
	inod = iget_locked(sb, ino);
	
#ifdef XDFS_DEBUG
	printk("XDFS: iget_locked ending\n");
	msleep(1000);
#endif

	if (!inod)
		return ERR_PTR(-ENOMEM);
	if (!(inod->i_state & I_NEW))      
		return inod;   

#ifdef XDFS_DEBUG
	printk("XDFS: xdfs_iget(%p,%ld) new inode ino=%ld\n",sb,ino,ino);
#endif
	
	block = XDFS_INODE_BLOCK + ino;
	bh = sb_bread(inod->i_sb,block);
	raw_inode = (struct xdfs_inode *)(bh->b_data);   //play the role of ext2_get_inode其实，这样读，还有风险，风险就是：可能存在字节序的情况，所以，像ext3等都要考虑字节序

#ifdef XDFS_DEBUG
	printk("XDFS: xdfs_inode->inode_no            %ld",raw_inode->inode_no);
	printk("XDFS: xdfs_inode->num_link            %d",raw_inode->num_link);
	printk("XDFS: xdfs_inode->state               %ld",raw_inode->state);
	printk("XDFS: xdfs_inode->block_size_in_bit   %d",raw_inode->block_size_in_bit);
	printk("XDFS: xdfs_inode->addr                %d",raw_inode->addr);
	printk("XDFS: xdfs_inode->inode_count.counter %d",raw_inode->inode_count.counter);
	printk("XDFS: inode->i_nlink =   %d",inod->i_nlink);
#endif
	
	inod->i_mode = raw_inode->mode;
	if(S_ISREG(inod->i_mode))
	{
#ifdef XDFS_DEBUG
		printk("XDFS: it is a common file\n");
#endif
	    inod->i_mode |= S_IFREG;
	    inod->i_op = &xdfs_inode_operations;
	    inod->i_fop = &xdfs_file_operations;
	    inod->i_mapping->a_ops = &xdfs_aops;  
	}
	else
	{
#ifdef XDFS_DEBUG
		printk("XDFS: it is a directory\n");
#endif
		inod->i_mode |= S_IFDIR;
		inod->i_op = &xdfs_inode_dir_operations;
		inod->i_fop = &xdfs_file_dir_operations;
	}
    inod->i_uid.val = raw_inode->uid;
    inod->i_gid.val = raw_inode->gid;
    // set_nlink(inod,raw_inode->inode_count.counter);
	if(inod->i_nlink==0)
	{
#ifdef XDFS_DEBUG
		printk("XDFS ERROR: xdfs_iget bad inode\n");
#endif
		goto bad_inode;
	}
    inod->i_size = raw_inode->file_size;
    // inod->i_blocks = raw_inode->blocks;
	inod->i_blkbits = 0; 
	
	inod->i_private=(void*)kvmalloc(sizeof(struct xdfs_inode),GFP_KERNEL);   //内核分配
	memcpy(inod->i_private, raw_inode, sizeof(struct xdfs_inode));    //raw_inode:struct xdfs_incore_inode类型
	brelse(bh);
	inod->i_flags &= ~(S_SYNC | S_APPEND | S_IMMUTABLE | S_NOATIME |
                      S_DIRSYNC | S_DAX);
  	inod->i_flags |= S_SYNC;
#ifdef XDFS_DEBUG
	printk("XDFS: xdfs_iget ending\n");
#endif
  	return inod;
	
bad_inode:
	brelse(bh);
	iget_failed(inod);
	return ERR_PTR(ret);
} 
static int 
xdfs_sync_fs(struct super_block *sb, int wait)
{
	struct buffer_head *bh;
#ifdef XDFS_DEBUG
	printk("XDFS: xdfs_sync_fs(%p) start \n", sb);
#endif
	bh = ((struct xdfs_superblock*)(sb->s_fs_info))->bh;
	
	/* Is superblock read only? */
	if(!sb_rdonly(sb))
	{
		/* mark it dirty */	
		mark_buffer_dirty(bh);
		if(wait)
			sync_dirty_buffer(bh);

#ifdef XDFS_DEBUG
		printk("XDFS: xdfs_sync_fs(%p) exit \n",sb);
#endif
		return 0;
	}
	return 0;
}
static struct file_system_type xdfs_fs_type = {
  .owner = THIS_MODULE, 
  .name = "xdfs",
  .mount = xdfs_get_super,     
  .kill_sb = xdfs_kill_block_super, //it is default, not provided by us.
  .fs_flags = FS_REQUIRES_DEV, //need to required a physical device. we use loop device
};


static void 
xdfs_kill_block_super(struct super_block *sb)
{
#ifdef XDFS_DEBUG
	printk("XDFS: kill_block_super is called\n");
#endif

	kill_block_super(sb);

#ifdef XDFS_DEBUG
	printk("XDFS: kill_block_super return\n");
#endif
}

static struct dentry *
xdfs_get_super(struct file_system_type *fs_type, int flags, const char *dev_name, void *data)
{
	struct dentry *ret;
#ifdef XDFS_DEBUG
	printk("XDFS: mount begin");
#endif

	ret = mount_bdev(fs_type, flags, dev_name, data, xdfs_fill_super);

#ifdef XDFS_DEBUG
	printk("XDFS: mount success");
#endif
	return ret;
}

static int __init init_xdfs_fs(void)
{
	int ret;
#ifdef XDFS_DEBUG
	printk("XDFS: init_xdfs_fs begin\n");
#endif

	ret = register_filesystem(&xdfs_fs_type);

#ifdef XDFS_DEBUG
	printk("XDFS: init_xdfs_fs success\n");
#endif
  return ret;
}
MODULE_ALIAS_FS("xdfs");

static void __exit exit_xdfs_fs(void)
{
#ifdef XDFS_DEBUG
  	printk("XDFS: exit_xdfs_fs begin\n");
#endif

	unregister_filesystem(&xdfs_fs_type);

#ifdef XDFS_DEBUG
	printk("XDFS: exit_xdfs_fs success\n");
#endif
}

/* operations set  and achievement*/
// all is commented to test mount ? all achieve printk to test mount(used)
static struct super_operations xdfs_sops = {
    //read_inode : xdfs_read_inode,//2.6没有这一项 已经和iget合并为my_iget
	alloc_inode : xdfs_alloc_inode,
	write_inode : xdfs_write_inode,
	evict_inode : xdfs_evict_inode,
	put_super : xdfs_put_super,
	sync_fs : xdfs_write_super,
	statfs : xdfs_statfs,
    //drop_inode: xdfs_delete_inode,//generic_drop_inode,//如果这里不定义的话 系统会自动调用generic_drop_inode
};

static struct inode *
xdfs_alloc_inode(struct super_block *sb)
{
	/* in ext2 its global static variable*/
	struct kmem_cache * xdfs_inode_cachep;
	struct xdfs_inode * xd_inode;
	xd_inode = alloc_inode_sb(sb, xdfs_inode_cachep, GFP_KERNEL);
	if (!xd_inode)
		return NULL;
	// xd_inode->addr = NULL;
	inode_set_iversion(&xd_inode->vfs_inode, 1);

	return &xd_inode->vfs_inode;
}
static int 
xdfs_write_inode(struct inode *inode, struct writeback_control *wbc)
{
#ifdef XDFS_DEBUG
	printk("XDFS: write_inode is called inode is (%p) wbc is (%p)\n",inode,wbc);
#endif
	return 0;
}
static void 
xdfs_evict_inode(struct inode * inode)
{
#ifdef XDFS_DEBUG
	printk("XDFS: evict_inode is called inode is (%p) and its n_link = 0\n",inode);
#endif
	return;
}
static void 
xdfs_put_super(struct super_block *s)
{
	struct buffer_head *bh;
#ifdef XDFS_DEBUG
	printk("XDFS: xdfs_put_super(%p) is called\n",s);
#endif

	bh = ((struct xdfs_superblock*)(s->s_fs_info))->bh;
	brelse(bh);
	
#ifdef XDFS_DEBUG
	printk("XDFS: xdfs_put_super(%p) return\n",s);
#endif
}
static int 
xdfs_write_super(struct super_block *sb,int wait)
{
	struct buffer_head *bh;
#ifdef XDFS_DEBUG
	printk("XDFS: xdfs_write_super(%p) is called\n",sb);
#endif

	bh = ((struct xdfs_superblock*)(sb->s_fs_info))->bh;
	mark_buffer_dirty(bh);

#ifdef XDFS_DEBUG
	printk("XDFS: xdfs_write_super(%p) return\n",sb);
#endif
	return 0;
}
static int 
xdfs_statfs(struct dentry *dentry, struct kstatfs * buf)
{
#ifdef XDFS_DEBUG
	printk("XDFS: statfs is called dentry is (%p),kstatfs is (%p)\n",dentry,buf);
#endif
	return 0;
}
/* operations about file,inode,address*/

static struct address_space_operations xdfs_aops = {
    //是否需要
    // readpage : my_readpage,
    // writepage : my_writepage,
    // sync_page : block_sync_page,
    // prepare_write : my_prepare_write,
    // commit_write : generic_commit_write,
    // bmap : my_bmap,
};
static struct inode_operations xdfs_inode_operations = {
    //这个地方我觉得编写的有问题//是否需要
    //link : my_link,
    //unlink : my_unlink,
};
static struct file_operations xdfs_file_operations = {
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
	int error;
#ifdef XDFS_DEBUG
	printk("XDFS: open is called\n");
#endif

	error = generic_file_open(inode, filp);
	
#ifdef XDFS_DEBUG
	printk("XDFS: open return\n");
#endif
	return error;
}

static ssize_t 
xdfs_read_iter(struct kiocb *iocb, struct iov_iter *to)
{
	ssize_t error;
#ifdef XDFS_DEBUG
	printk("XDFS: read_iter is called\n");
#endif

	error = generic_file_read_iter(iocb, to);

#ifdef XDFS_DEBUG
	printk("XDFS: read_iter return\n");
#endif
	return error;
}
static ssize_t 
xdfs_write_iter(struct kiocb *iocb, struct iov_iter *from)
{
	ssize_t error;
#ifdef XDFS_DEBUG
	printk("XDFS: write_iter is called\n");
#endif

	error = generic_file_write_iter(iocb, from);
	
#ifdef XDFS_DEBUG
	printk("XDFS: write_iter return\n");
#endif
	return error;
}

static ssize_t 
xdfs_splice_read(struct file *in, loff_t *ppos,
				 struct pipe_inode_info *pipe, size_t len,
				 unsigned int flags)
{
	ssize_t error;
#ifdef XDFS_DEBUG
	printk("XDFS: splice_read is called\n");
#endif

	error = generic_file_splice_read(in, ppos,pipe,len,flags);

#ifdef XDFS_DEBUG
	printk("XDFS: splice_read return\n");
#endif
	return error;
}

static ssize_t
xdfs_splice_write(struct pipe_inode_info *pipe, struct file *out,
			  loff_t *ppos, size_t len, unsigned int flags)
{
	ssize_t error;
#ifdef XDFS_DEBUG
	printk("XDFS: write is called\n");
#endif

	error = iter_file_splice_write(pipe,out,ppos,len,flags);

#ifdef XDFS_DEBUG
	printk("XDFS: splice_writsplice_e return\n");
#endif
	return error;
}
static loff_t 
xdfs_file_llseek(struct file *file, loff_t offset, int whence)
{
	loff_t error;
#ifdef XDFS_DEBUG
	printk("XDFS: file_llseek is called\n");
#endif

	error = generic_file_llseek(file,offset,whence);

#ifdef XDFS_DEBUG
	printk("XDFS: file_llseek return\n");
#endif
	return error;
}

static int 
xdfs_file_mmap(struct file *file, struct vm_area_struct *vma)
{
	int error;
#ifdef XDFS_DEBUG
	printk("XDFS: file_mmap is called\n");
#endif

	error = generic_file_mmap(file,vma);

#ifdef XDFS_DEBUG
	printk("XDFS: file_mmap return\n");
#endif
	return error;
}

static int 
xdfs_file_fsync(struct file *file, loff_t start, loff_t end, int datasync)
{
	int error;
#ifdef XDFS_DEBUG
	printk("XDFS: file_fsync is called\n");
#endif
	error = generic_file_fsync(file, start, end, datasync);
#ifdef XDFS_DEBUG
	printk("XDFS: file_fsync return\n");
#endif
	return error;
}

static struct file_operations xdfs_file_dir_operations = {
	.llseek		= xdfs_generic_file_llseek,
	.read		= xdfs_generic_read_dir,
	.iterate_shared	= xdfs_readdir,
	// .unlocked_ioctl = xdfs_ioctl,
	// .fsync		= xdfs_fsync,
};

static loff_t 
xdfs_generic_file_llseek(struct file *file, loff_t offset, int whence)
{
	struct inode *inode = file->f_mapping->host;
	loff_t ret;

#ifdef XDFS_DEBUG
	printk("XDFS: xdfs_generic_file_llseek is called\n");
#endif

	ret = generic_file_llseek_size(file, offset, whence,
					inode->i_sb->s_maxbytes,
					i_size_read(inode));
#ifdef XDFS_DEBUG
	printk("XDFS: xdfs_generic_file_llseek return\n");
#endif
	return ret;
}
static ssize_t 
xdfs_generic_read_dir(struct file *filp, char __user *buf, size_t siz, loff_t *ppos)
{
#ifdef XDFS_DEBUG
	printk("XDFS: xdfs_generic_read_dir is called\n");
	printk("XDFS: xdfs_generic_read_dir return\n");
#endif
	return -EISDIR;
}

static int
xdfs_readdir(struct file *file, struct dir_context *ctx)
{
	loff_t pos = ctx->pos;
	struct inode *inode = file_inode(file);
	/* sb may be used to show error but i dont need */
	// struct super_block *sb = inode->i_sb;
	unsigned int offset = pos & ~PAGE_MASK;
	unsigned long n = pos >> PAGE_SHIFT;
	unsigned long npages = dir_pages(inode);
	unsigned int last_byte;

#ifdef XDFS_DEBUG
	printk("XDFS: readdir is called\n");
#endif

	for ( ; n < npages; n++, offset = 0) {
		char* kaddr,*limit;
		struct xdfs_dir_entry* de;
		struct page *page = xdfs_get_page(inode, n, 0, (void **)&kaddr);
		/* may do some check aboout pages*/
		if (IS_ERR(page)) {
			xdfs_printk("page error\n");
			ctx->pos += PAGE_SIZE - offset;
			return PTR_ERR(page);
		}
		/* find dir_entry in data space */
		de = (struct xdfs_dir_entry *)(kaddr+offset);

		
		last_byte = inode->i_size -( n << PAGE_SHIFT);
		if(last_byte > PAGE_SIZE){ last_byte = PAGE_SIZE;}


		limit = kaddr + last_byte - XDFS_DIR_ENTRY_TOTAL_LEN(1);
		for ( ;(char*)de <= limit; de = xdfs_next_dir_entry(de)) {
			if (de->dir_entry_len == 0) {
				xdfs_printk("dir entry zero-length\n");
				xdfs_put_page(page, kaddr);
				return -EIO;
			}
			if (de->inode_no) {
				unsigned char d_type = DT_UNKNOWN;

				/* 用于确定此文件的文件类型，内核里面有张表*/
				d_type = fs_ftype_to_dtype(de->file_type);
				/* 用于填充内核里面的目录项 */
				if (!dir_emit(ctx, de->name, de->name_len,
						le32_to_cpu(de->inode_no),
						d_type)) {
					xdfs_put_page(page, kaddr);
					return 0;
				}
			}
			ctx->pos += le16_to_cpu(de->dir_entry_len);
		}
		xdfs_put_page(page, kaddr);
		xdfs_printk("one dir_entry read\n");
	}
#ifdef XDFS_DEBUG
	printk("XDFS: readdir return\n");
#endif
	return 0;
}
static struct xdfs_dir_entry*
xdfs_next_dir_entry(struct xdfs_dir_entry* de)
{
	return (struct xdfs_dir_entry*)((char*)de + le16_to_cpu(de->dir_entry_len));
}
static struct page * 
xdfs_get_page(struct inode *dir, unsigned long n, int quiet, void **page_addr)
{
	struct address_space *mapping = dir->i_mapping;
	struct folio *folio = read_mapping_folio(mapping, n, NULL);

	if (IS_ERR(folio))
		return &folio->page;
	*page_addr = kmap_local_folio(folio, n & (folio_nr_pages(folio) - 1));
	/* i don't know function of this part, so I comment this part */
	// if (unlikely(!folio_test_checked(folio))) {
	// 	if (!ext2_check_page(&folio->page, quiet, *page_addr))
	// 		goto fail;
	// }
	return &folio->page;

// fail:
// 	xdfs_put_page(&folio->page, *page_addr);
// 	return ERR_PTR(-EIO);
}

static inline void 
xdfs_put_page(struct page *page, void *page_addr)
{
	kunmap_local(page_addr);
	/* 用于将线性地址修改的数据落实到物理地址上，就是将内存中的页更新到磁盘中 */
	put_page(page);
}

static struct inode_operations xdfs_inode_dir_operations = {
	// .create		    = xdfs_dir_create,
	// .lookup		    = xdfs_dir_lookup,
	// .link		    = xdfs_dir_link,
	// .unlink		    = xdfs_dir_unlink,
	// .symlink	    = xdfs_dir_symlink,
	// .mkdir		    = xdfs_dir_mkdir,
	// .rmdir		    = xdfs_dir_rmdir,
	// .mknod		    = xdfs_dir_mknod,
	// .rename		    = xdfs_dir_rename,
	// .listxattr	    = xdfs_dir_listxattr,
	// .getattr	    = xdfs_dir_getattr,
	// .setattr	    = xdfs_dir_setattr,
	// .get_inode_acl	= xdfs_dir_get_acl,
	// .set_acl	    = xdfs_dir_set_acl,
	// .tmpfile	    = xdfs_dir_tmpfile,
	// .fileattr_get	= xdfs_dir_fileattr_get,
	// .fileattr_set	= xdfs_dir_fileattr_set,
};

static int 
xdfs_create (struct user_namespace * mnt_userns,
			struct inode * dir, struct dentry * dentry,
			umode_t mode, bool excl)
{
	/* 1. new inode */
	/* 2. set file opeartions */
	/* 3. sync to disk */
	/* 4. add nondir */
	return 0;
}
module_init(init_xdfs_fs)
module_exit(exit_xdfs_fs)