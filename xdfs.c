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
#include <linux/mpage.h> 	/* mpage */
#include <linux/exportfs.h>	/* export_fs */
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
#define XDFS_FIRST_INO 3
#define XDFS_MAX_INO 4096
#define XDFS_INODE_SIZE (sizeof(struct xdfs_inode))
#define XDFS_SB_SIZE 32

#define XDFS_MAXFILES 4096		/* The filesystem has 4096 inodes */

#define PBR_BLKNO 0
#define FSB0_BLKNO 1
#define FSB1_BLKNO 2
#define FIB0_BLKNO 3
#define FIB1_BLKNO 4
#define INODE_SET_BLKNO 5
#define INODE_LOG_BLKNO 30
#define FSB_MB0_BLKNO 31
#define FSB_MB1_BLKNO 32
#define FIB_MB0_BLKNO 33
#define FIB_MB1_BLKNO 34
#define GTI0_BLKNO 35
#define GTI1_BLKNO 36
#define DS_BLKNO 37
#define SB_BLKNO 32767

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

// struct xdfs_inode
// {
//     umode_t mode;						/* IS directory?  */
//     uid_t uid; 						/* User id */
//     gid_t gid;						/* User group id */
// 	unsigned long flags;			/* file flags */
//     unsigned long inode_no;				/* Stat data, not accessed from path walking, the unique label of the inode */
//     unsigned int num_link;				/* The num of the hard link  */
//     loff_t file_size;					/* The file size in bytes */
//     unsigned long ctime;      		/* The last time the file attributes */
//     unsigned long mtime;      		/* The last time the file data was changed */
//     unsigned long atime;      		/* The last time the file data was accessed */
//     unsigned int block_size_in_bit;				/* The size of the block in bits */
//     blkcnt_t using_block_num;					/* The num of blks file using */
//     unsigned long state;				/* State flag  */
// 	unsigned long blockno;
//     atomic_t inode_count;					/* Inode reference count  */

//     UINT32 addr[XDFS_DIRECT_BLOCKS];	/* Store the address */

// };

struct xdfs_inode
{
    umode_t mode;						/* IS directory?  */
    uid_t uid; 						/* User id */
    gid_t gid;						/* User group id */
	unsigned long flags;			/* file flags */
    unsigned long inode_no;				/* Stat data, not accessed from path walking, the unique label of the inode */
    unsigned int num_link;				/* The num of the hard link  */
    loff_t file_size;					/* The file size in bytes */
    unsigned long ctime;      		/* The last time the file attributes */
    unsigned long mtime;      		/* The last time the file data was changed */
    unsigned long atime;      		/* The last time the file data was accessed */
    unsigned int block_size_in_bit;				/* The size of the block in bits */
    unsigned long state;				/* State flag  */
	unsigned long blockno;				/* inode blkno */
	unsigned long file_blockno;				/* file blkno */
    blkcnt_t using_block_num;				/* The num of blks file using */
    atomic_t inode_count;					/* Inode reference count  */

};


/* xdfs_inode in memory and extend something */
struct xdfs_inode_info{
	UINT32	i_data[15];
	UINT32  i_flags;
	UINT32 i_dir_start_lookup;
	struct inode vfs_inode;
};

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

struct xdfs_dir_entry{
	UINT32 inode_no;
	UINT16 dir_entry_len; 
	UINT8 name_len; 
	UINT8 file_type; 
	char name[255]; 
};

static struct super_operations 		   xdfs_sops;
static struct address_space_operations xdfs_aops;
static struct file_operations          xdfs_file_dir_operations;
static struct inode_operations         xdfs_inode_dir_operations;
static struct file_operations          xdfs_file_operations;
static struct inode_operations         xdfs_inode_operations;
static struct file_system_type         xdfs_fs_type;
static struct export_operations xdfs_export_ops;
/*all func declaration*/
static int xdfs_fill_super(struct super_block *sb, void *data, int silent);
static struct inode *xdfs_iget(struct super_block *sb, unsigned long ino);
static struct xdfs_inode *xdfs_get_inode(struct super_block *sb, ino_t ino,
					struct buffer_head **p);
void xdfs_set_inode_flags(struct inode *inode);
static int xdfs_sync_fs(struct super_block *sb, int wait);
static struct dentry *xdfs_get_super(struct file_system_type *fs_type, int flags, const char *dev_name, void *data);

static struct inode *xdfs_alloc_inode(struct super_block *sb);
static int __init init_inodecache(void);
static void init_once(void *foo);
static void destroy_inodecache(void);

static int xdfs_write_inode(struct inode *inode, struct writeback_control *wbc);
static void xdfs_evict_inode(struct inode * inode);
static void xdfs_put_super(struct super_block *s);
static int xdfs_write_super(struct super_block *sb,int wait);
static int xdfs_statfs(struct dentry *dentry, struct kstatfs * buf);

bool xdfs_block_dirty_folio(struct address_space *mapping, struct folio *folio);
void xdfs_block_invalidate_folio(struct folio *folio, size_t offset, size_t length);
int xdfs_buffer_migrate_folio(struct address_space *mapping,
		struct folio *dst, struct folio *src, enum migrate_mode mode);
bool xdfs_block_is_partially_uptodate(struct folio *folio, size_t from, size_t count);
int xdfs_generic_error_remove_page(struct address_space *mapping, struct page *page);
static int xdfs_writepages(struct address_space *mapping, struct writeback_control *wbc);
static ssize_t xdfs_direct_IO(struct kiocb *iocb, struct iov_iter *iter);
static int xdfs_write_begin(struct file *file, struct address_space *mapping,
		loff_t pos, unsigned len, struct page **pagep, void **fsdata);
static int xdfs_write_end(struct file *file, struct address_space *mapping,
			loff_t pos, unsigned len, unsigned copied,
			struct page *page, void *fsdata);
static void xdfs_readahead(struct readahead_control *rac);
static int xdfs_read_folio(struct file *file, struct folio *folio);
static sector_t xdfs_bmap(struct address_space *mapping, sector_t block);

int xdfs_get_block(struct inode *inode, sector_t iblock,
		struct buffer_head *bh_result, int create);

static int xdfs_get_blocks(struct inode *inode,
			   sector_t iblock, unsigned long maxblocks,
			   struct buffer_head *bh_result,
			   int create);


static ssize_t xdfs_listxattr(struct dentry *dentry, char *buffer, size_t buffer_size);
int xdfs_getattr(struct user_namespace *mnt_userns, const struct path *path,
		 struct kstat *stat, u32 request_mask, unsigned int query_flags);

int xdfs_setattr(struct user_namespace *mnt_userns, struct dentry *dentry,
		 struct iattr *iattr);



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
static int xdfs_release_file (struct inode * inode, struct file * filp);
static int xdfs_file_fsync(struct file *file, loff_t start, loff_t end, int datasync);

static loff_t xdfs_generic_file_llseek(struct file *file, loff_t offset, int whence);
static ssize_t xdfs_generic_read_dir(struct file *filp, char __user *buf, size_t siz, loff_t *ppos);
static int xdfs_readdir(struct file *file, struct dir_context *ctx);
static struct page * xdfs_get_page(struct inode *dir, unsigned long n, int quiet, void **page_addr);
static inline void xdfs_put_page(struct page *page, void *page_addr);
static int xdfs_fsync(struct file *file, loff_t start, loff_t end, int datasync);

static struct dentry *
xdfs_fh_to_dentry(struct super_block *sb, struct fid *fid,
		int fh_len, int fh_type);
static struct dentry *
xdfs_fh_to_parent(struct super_block *sb, struct fid *fid,
		int fh_len, int fh_type);
struct dentry *xdfs_get_parent(struct dentry *child);

int xdfs_inode_by_name(struct inode *dir, const struct qstr *child, ino_t *ino);
struct xdfs_dir_entry *xdfs_find_entry (struct inode *dir,
			const struct qstr *child, struct page **res_page,
			void **res_page_addr);
static struct xdfs_dir_entry * xdfs_next_dir_entry(struct xdfs_dir_entry* de);

static int xdfs_dir_create (struct user_namespace * mnt_userns,
			struct inode * dir, struct dentry * dentry,
			umode_t mode, bool excl);
			
static ssize_t xdfs_dir_listxattr(struct dentry *dentry, char *buffer, size_t buffer_size);
int xdfs_dir_getattr(struct user_namespace *mnt_userns, const struct path *path,
		 struct kstat *stat, u32 request_mask, unsigned int query_flags);

int xdfs_dir_setattr(struct user_namespace *mnt_userns, struct dentry *dentry,
		 struct iattr *iattr);


static int xdfs_setsize(struct inode *inode, loff_t newsize);

static inline struct xdfs_inode_info *XDFS_I(struct inode *inode);

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
    sb->s_magic = XDFS_MAGIC;
	sb->s_flags = SB_NOSUID|SB_NOATIME;
	sb->s_blocksize = BLKSIZE;
	sb->s_blocksize_bits = BLKSIZE_BITS;
	sb->s_maxbytes = sb->s_blocksize_bits;
	sb->s_max_links = INODE_NUMS;
	sb->s_time_min = S32_MIN;
	sb->s_time_max = S32_MAX;
    sb->s_fs_info = xdfs_sb;
	
	sb->s_flags |= SB_POSIXACL;
	// sb->s_iflags |= SB_I_CGROUPWB;

#ifdef XDFS_DEBUG
	printk("XDFS: op starting\n");
	msleep(1000);
#endif
	sb->s_op = &xdfs_sops;
	sb->s_export_op = &xdfs_export_ops;
	sb->s_dev = dev;
	
	/* get a new vnode */
#ifdef XDFS_DEBUG
	printk("XDFS: iget fronting\n");
	msleep(1000);	
#endif
	/* alloc root inode and mount*/
    inode_rt = xdfs_iget(sb, XDFS_ROOT_INO);	/* need new function written by GuoHeng, need a error examine */
	
	if (!S_ISDIR(inode_rt->i_mode)) {
		iput(inode_rt);
		xdfs_printk("corrupt root inode not directory\n");
		return -1;
	}

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
	//xdfs_write_super(sb);两者一样
	xdfs_sync_fs(sb, 1);	/* 1: sync, 0: async */
#ifdef XDFS_DEBUG
	printk("XDFS: xdfs_fill_super exit successfully\n ");
#endif
	return 0;
}
static struct inode *
xdfs_iget(struct super_block *sb, unsigned long ino)
{
	
	struct xdfs_inode_info* raw_inode_info;
	struct inode *inod;
	struct buffer_head *bh;
    struct xdfs_inode *raw_inode;    
	uid_t i_uid;
	gid_t i_gid;            
	// int block;
	long ret = -EIO;
#ifdef XDFS_DEBUG
	printk("XDFS: xdfs_iget(%p,%ld)\n",sb,ino);
	msleep(1000);
#endif

	inod = iget_locked(sb, ino);
	raw_inode_info = XDFS_I(inod);
	
#ifdef XDFS_DEBUG
	printk("XDFS: iget_locked ending\n");
	msleep(1000);
#endif
	
	if (!inod)
		return ERR_PTR(-ENOMEM);

#ifdef XDFS_DEBUG
	printk("XDFS: inode->i_state            %x and I_NEW = %x",inod->i_state,I_NEW);
#endif

	if (!(inod->i_state & I_NEW))      
		return inod;   

#ifdef XDFS_DEBUG
	printk("XDFS: xdfs_iget(%p,%ld) new inode ino=%ld\n",sb,ino,ino);
#endif
	
	
	raw_inode = xdfs_get_inode(inod->i_sb, ino, &bh);
	
#ifdef XDFS_DEBUG
	printk("XDFS: xdfs_inode->mode                %ld",raw_inode->mode);
	printk("XDFS: xdfs_inode->inode_no            %ld",raw_inode->inode_no);
	printk("XDFS: xdfs_inode->num_link            %d",raw_inode->num_link);
	printk("XDFS: xdfs_inode->state               %ld",raw_inode->state);
	printk("XDFS: xdfs_inode->block_size_in_bit   %d",raw_inode->block_size_in_bit);
	// printk("XDFS: xdfs_inode->addr                %p",raw_inode->addr);
	printk("XDFS: xdfs_inode->inode_count.counter %d",raw_inode->inode_count.counter);
	printk("XDFS: inode->i_nlink =   			  %d",inod->i_nlink);
	
	printk("XDFS: inode->i_sb->s_dev         %ld",inod->i_sb->s_dev);
	printk("XDFS: inode->i_mode              %ld",inod->i_mode);
	printk("XDFS: inode->i_ino               %ld",inod->i_ino);
	printk("XDFS: inode->i_nlink             %d", inod->i_nlink);
	printk("XDFS: inode->i_state             %ld",inod->i_state);
	printk("XDFS: inode->i_atime             %d", inod->i_atime.tv_sec);
	printk("XDFS: inode->i_ctime             %p", inod->i_ctime.tv_sec);
	printk("XDFS: inode->i_mtime             %d", inod->i_mtime.tv_sec);
	printk("XDFS: inode->i_nlink =   		 %d", inod->i_nlink);
#endif
	
	inod->i_mode = raw_inode->mode;
	if(ino==2)
	{
		inod->i_mode |= S_IFDIR;
	}
	raw_inode_info->i_flags = le32_to_cpu(raw_inode->flags);
	raw_inode_info->i_dir_start_lookup = 0;
	xdfs_set_inode_flags(inod);
	if(S_ISREG(inod->i_mode))
	{
#ifdef XDFS_DEBUG
		printk("XDFS: it is a common file\n");
#endif
	    inod->i_op = &xdfs_inode_operations;
	    inod->i_fop = &xdfs_file_operations;
	    inod->i_mapping->a_ops = &xdfs_aops;  
	}
	else if (S_ISDIR(inod->i_mode)) {
#ifdef XDFS_DEBUG
		printk("XDFS: it is a directory\n");
#endif
		inod->i_op = &xdfs_inode_dir_operations;
		inod->i_fop = &xdfs_file_dir_operations;
	    inod->i_mapping->a_ops = &xdfs_aops;  
	} else 
	{
#ifdef XDFS_DEBUG
		printk("XDFS: what type is this file ? inod->mod is %x\n",inod->i_mode);
#endif
	}
	i_uid = (uid_t)le16_to_cpu(raw_inode->uid);
	i_gid = (gid_t)le16_to_cpu(raw_inode->gid);
	i_uid_write(inod, i_uid);
	i_gid_write(inod, i_gid);
    // inod->i_uid.val = raw_inode->uid;
    // inod->i_gid.val = raw_inode->gid;
	raw_inode->atime = current_time(inod).tv_sec;
	raw_inode->ctime = current_time(inod).tv_sec;
	raw_inode->mtime = current_time(inod).tv_sec;
	inod->i_atime.tv_sec = (signed)le32_to_cpu(raw_inode->atime);
	inod->i_ctime.tv_sec = (signed)le32_to_cpu(raw_inode->ctime);
	inod->i_mtime.tv_sec = (signed)le32_to_cpu(raw_inode->mtime);
	inod->i_generation = 1;
	inod->i_blocks = raw_inode->blockno;
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
	// inod->i_blkbits = 0; 
	
	inod->i_private=(void*)kvmalloc(sizeof(struct xdfs_inode),GFP_KERNEL);   //内核分配
	memcpy(inod->i_private, raw_inode, sizeof(struct xdfs_inode));    //raw_inode:struct xdfs_incore_inode类型
	brelse(bh);
	inod->i_flags &= ~(S_SYNC | S_APPEND | S_IMMUTABLE | S_NOATIME |
                      S_DIRSYNC | S_DAX);
  	inod->i_flags |= S_SYNC | S_DIRSYNC | S_NOATIME | S_APPEND;
#ifdef XDFS_DEBUG
	printk("XDFS: xdfs_iget ending\n");
#endif
  	return inod;
	
bad_inode:
	brelse(bh);
	iget_failed(inod);
	return ERR_PTR(ret);
} 

static struct xdfs_inode *xdfs_get_inode(struct super_block *sb, ino_t ino,
					struct buffer_head **p)
{
	struct buffer_head * bh;
	// unsigned long block_group;
	unsigned long block;
	unsigned long offset;
	/* we dont have group */
	// struct ext2_group_desc * gdp;

	*p = NULL;
	/* ino范围检查 */
	if ((ino != XDFS_ROOT_INO && (ino < XDFS_FIRST_INO ||
	    ino > XDFS_MAX_INO)))
		goto Einval;

	// block_group = (ino - 1) / EXT2_INODES_PER_GROUP(sb);
	// gdp = ext2_get_group_desc(sb, block_group, NULL);
	// if (!gdp)
	// 	goto Egdp;

	/*
	 * Figure out the offset within the block group inode table
	 */
	offset = (ino*XDFS_INODE_SIZE);
	block = XDFS_INODE_BLOCK;
	xdfs_printk("get_inode read from (block,offset) is (%ld,%ld)",block,offset);
	if (!(bh = sb_bread(sb, block)))
		goto Eio;

	*p = bh;
	offset &= XDFS_BSIZE - 1;
	return (struct xdfs_inode *) (bh->b_data + offset);

Einval:
	xdfs_printk("get_inode ino not in bound\n");
	return ERR_PTR(-EINVAL);
Eio:
	xdfs_printk("get_inode sb_bread read wrong\n");
	return ERR_PTR(-EIO);
// Egdp:
// 	return ERR_PTR(-EIO);
}

void xdfs_set_inode_flags(struct inode *inod)
{
	// unsigned int flags = XDFS_I(inod)->i_flags;

	inod->i_flags &= ~(S_SYNC | S_APPEND | S_IMMUTABLE | S_NOATIME |
				S_DIRSYNC | S_DAX);
	// if (flags & EXT2_SYNC_FL)
	// 	inode->i_flags |= S_SYNC;
	// if (flags & EXT2_APPEND_FL)
	// 	inode->i_flags |= S_APPEND;
	// if (flags & EXT2_IMMUTABLE_FL)
	// 	inode->i_flags |= S_IMMUTABLE;
	// if (flags & EXT2_NOATIME_FL)
	// 	inode->i_flags |= S_NOATIME;
	// if (flags & EXT2_DIRSYNC_FL)
	// 	inode->i_flags |= S_DIRSYNC;
	// if (test_opt(inode->i_sb, DAX) && S_ISREG(inode->i_mode))
	// 	inode->i_flags |= S_DAX;
	if (S_ISREG(inod->i_mode))
		inod->i_flags |= S_DAX;
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


static struct export_operations xdfs_export_ops = {
	.fh_to_dentry = xdfs_fh_to_dentry,
	.fh_to_parent = xdfs_fh_to_parent,
	.get_parent = xdfs_get_parent,
};


static struct dentry *
xdfs_fh_to_dentry(struct super_block *sb, struct fid *fid,
		int fh_len, int fh_type)
{
	xdfs_printk("xdfs_fh_to_dentry called \n");
	return generic_fh_to_dentry(sb, fid, fh_len, fh_type,
				    NULL);
}
static struct dentry *
xdfs_fh_to_parent(struct super_block *sb, struct fid *fid,
		int fh_len, int fh_type)
{
	xdfs_printk("xdfs_fn_to_parent called \n");
	return generic_fh_to_parent(sb, fid, fh_len, fh_type,
				    NULL);
}
struct dentry *
xdfs_get_parent(struct dentry *child)
{
	ino_t ino;
	int res;
	xdfs_printk("xdfs_get_parent is called\n");
	res = xdfs_inode_by_name(d_inode(child), &dotdot_name, &ino);
	if (res)
		return ERR_PTR(res);

	return d_obtain_alias(xdfs_iget(child->d_sb, ino));
} 
int 
xdfs_inode_by_name(struct inode *dir, const struct qstr *child, ino_t *ino)
{
	struct xdfs_dir_entry *de;
	struct page *page;
	void *page_addr;
	
	xdfs_printk("xdfs_inode_by_name is called\n");
	de = xdfs_find_entry(dir, child, &page, &page_addr);
	if (IS_ERR(de))
		return PTR_ERR(de);

	*ino = le32_to_cpu(de->inode_no);
	xdfs_put_page(page, page_addr);

	xdfs_printk("xdfs_inode_by_name return\n");
	return 0;
}

struct xdfs_dir_entry *
xdfs_find_entry (struct inode *dir,
			const struct qstr *child, struct page **res_page,
			void **res_page_addr)
{
	const char *name = child->name;
	int namelen = child->len;
	unsigned reclen = namelen+8;
	unsigned long start, n;
	unsigned long npages = dir_pages(dir);
	struct page *page = NULL;
	struct xdfs_inode_info *ei = XDFS_I(dir);
	struct xdfs_dir_entry * de;
	void *page_addr;
	unsigned last_byte;
	xdfs_printk("xdfs_find_entry is called\n");

	if (npages == 0)
		goto out;

	/* OFFSET_CACHE */
	*res_page = NULL;
	*res_page_addr = NULL;

	start = ei->i_dir_start_lookup;
	if (start >= npages)
		start = 0;
	n = start;
	do {
		char *kaddr;
		page = xdfs_get_page(dir, n, 0, &page_addr);
		if (IS_ERR(page))
			return ERR_CAST(page);

		kaddr = page_addr;
		de = (struct xdfs_dir_entry *) kaddr;

		
		//xdfs_last_byte(dir,n)
		last_byte = dir->i_size -( n << PAGE_SHIFT);
		if(last_byte > PAGE_SIZE){ last_byte = PAGE_SIZE;}

		kaddr += last_byte - reclen;
		while ((char *) de <= kaddr) {
			if (de->dir_entry_len == 0) {
				xdfs_printk("zero-length directory entry\n");
				xdfs_put_page(page, page_addr);
				goto out;
			}
			if (/* xdfs_match (namelen, name, de)*/(namelen == de->name_len)&&(de->inode_no)&&(memcmp(name, de->name, namelen)))
					goto found;
			de = xdfs_next_dir_entry(de);
		}
		xdfs_put_page(page, page_addr);

		if (++n >= npages)
			n = 0;
		/* next page is past the blocks we've got */
		if (unlikely(n > (dir->i_blocks >> (PAGE_SHIFT - 9)))) {
			xdfs_printk("dir %lu size %lld exceeds block count %llu",
				dir->i_ino, dir->i_size,
				(unsigned long long)dir->i_blocks);
			goto out;
		}
	} while (n != start);
out:
	xdfs_printk("xdfs_find_entry return null !!!\n");
	return ERR_PTR(-ENOENT);

found:
	*res_page = page;
	*res_page_addr = page_addr;
	ei->i_dir_start_lookup = n;
	xdfs_printk("xdfs_find_entry found\n");
	return de;
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

	ret = init_inodecache();
	if(ret!=0)
	{
		return ret;
	}
	ret = register_filesystem(&xdfs_fs_type);
	if(ret!=0)
	{
		goto err;
	}

#ifdef XDFS_DEBUG
	printk("XDFS: init_xdfs_fs success\n");
#endif
	return ret;
err:
	destroy_inodecache();
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
    //.read_inode = xdfs_read_inode,//2.6没有这一项 已经和iget合并为my_iget
	.alloc_inode = xdfs_alloc_inode,
	.write_inode = xdfs_write_inode,
	.evict_inode = xdfs_evict_inode,
	.put_super = xdfs_put_super,
	.sync_fs = xdfs_sync_fs,
	.statfs = xdfs_statfs,
    //.drop_inode= xdfs_delete_inode,//generic_drop_inode,//如果这里不定义的话 系统会自动调用generic_drop_inode
};

struct kmem_cache * xdfs_inode_cachep;

static struct inode *
xdfs_alloc_inode(struct super_block *sb)
{
	struct xdfs_inode_info * xd_inode_info;

	xdfs_printk("alloc_inode is called\n");

	xd_inode_info = alloc_inode_sb(sb, xdfs_inode_cachep, GFP_KERNEL);
	if (!xd_inode_info)
		return NULL;
	// xd_inode_info->addr = NULL;
	inode_set_iversion(&xd_inode_info->vfs_inode, 1);

	xdfs_printk("alloc_inode return\n");

	return &xd_inode_info->vfs_inode;
}

static int __init init_inodecache(void)
{
	xdfs_printk("init_inodecache is called\n");
	xdfs_inode_cachep = kmem_cache_create_usercopy("xdfs_inode_cache",
				sizeof(struct xdfs_inode_info), 0,
				(SLAB_RECLAIM_ACCOUNT|SLAB_MEM_SPREAD|
					SLAB_ACCOUNT),
				offsetof(struct xdfs_inode_info, i_data),
				sizeof_field(struct xdfs_inode_info, i_data),
				init_once);
	if (xdfs_inode_cachep == NULL)
	{
		xdfs_printk("init_inodecache exit no mem");
		return -ENOMEM;
	}
	xdfs_printk("init_inodecache exit successfully\n");
	return 0;
}

static void init_once(void *foo)
{
	struct xdfs_inode_info *ei = (struct xdfs_inode_info *) foo;
	xdfs_printk("init_once is called\n");

	/* mutex 暂不考虑 */
	// rwlock_init(&ei->i_meta_lock);
	// mutex_init(&ei->truncate_mutex);
	inode_init_once(&ei->vfs_inode);
	xdfs_printk("init_once return\n");
}

static void destroy_inodecache(void)
{
	xdfs_printk("destory_inode_cache is called\n");
	/*
	 * Make sure all delayed rcu free inodes are flushed before we
	 * destroy cache.
	 */
	rcu_barrier();
	kmem_cache_destroy(xdfs_inode_cachep);
	xdfs_printk("destory_inode_cache return\n");
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
    .dirty_folio		= xdfs_block_dirty_folio,
	.invalidate_folio	= xdfs_block_invalidate_folio,
	.read_folio		= xdfs_read_folio,
	.readahead		= xdfs_readahead,
	.write_begin		= xdfs_write_begin,
	.write_end		= xdfs_write_end,
	.bmap			= xdfs_bmap,
	.direct_IO		= xdfs_direct_IO,
	.writepages		= xdfs_writepages,
	.migrate_folio		= xdfs_buffer_migrate_folio,
	.is_partially_uptodate	= xdfs_block_is_partially_uptodate,
	.error_remove_page	= xdfs_generic_error_remove_page,
    // .sync_page = block_sync_page,
    // .prepare_write = my_prepare_write,
    // .commit_write = generic_commit_write,
    // .bmap = my_bmap,
};

bool xdfs_block_dirty_folio(struct address_space *mapping, struct folio *folio)
{
	bool ret;
	xdfs_printk("block_dirty_folio is called\n");
	ret = block_dirty_folio(mapping,folio);
	xdfs_printk("block_dirty_folio return\n");
	return ret;
}

void xdfs_block_invalidate_folio(struct folio *folio, size_t offset, size_t length)
{
	xdfs_printk("block_invalidate_folio is called \n");
	block_invalidate_folio(folio,offset,length);
	xdfs_printk("block_invalidate_folio return \n");
}


int xdfs_buffer_migrate_folio(struct address_space *mapping,
		struct folio *dst, struct folio *src, enum migrate_mode mode)
{
	int ret;
	xdfs_printk("buffer_migrate_folio is called\n");
	ret = buffer_migrate_folio(mapping, dst, src, mode);
	xdfs_printk("buffer_migrate_folio return\n");
	return ret;
}


bool xdfs_block_is_partially_uptodate(struct folio *folio, size_t from, size_t count)
{
	bool ret;
	xdfs_printk("block_is_partially_uptodate is called\n");
	ret = block_is_partially_uptodate(folio,from,count);
	xdfs_printk("block_is_partially_uptodate return\n");
	return ret;
}


int xdfs_generic_error_remove_page(struct address_space *mapping, struct page *page)
{
	int ret;
	xdfs_printk("generic_error_remove_page is called\n");
	ret = generic_error_remove_page(mapping, page);
	xdfs_printk("generic_error_remove_page return\n");
	return ret;
}


static int
xdfs_writepages(struct address_space *mapping, struct writeback_control *wbc)
{
	int ret;
	xdfs_printk("writepages is called \n");
	ret = mpage_writepages(mapping, wbc, xdfs_get_block);
	xdfs_printk("writepages return \n");
	return ret;
}
static ssize_t
xdfs_direct_IO(struct kiocb *iocb, struct iov_iter *iter)
{
	struct file *file = iocb->ki_filp;
	struct address_space *mapping = file->f_mapping;
	struct inode *inode = mapping->host;
	size_t count = iov_iter_count(iter);
	ssize_t ret;
	xdfs_printk("xdfs_direct_IO is called \n");	

	ret = blockdev_direct_IO(iocb, inode, iter, xdfs_get_block);
	xdfs_printk("xdfs_direct_IO return \n");
	return ret;
}
static int
xdfs_write_begin(struct file *file, struct address_space *mapping,
		loff_t pos, unsigned len, struct page **pagep, void **fsdata)
{
	int ret;
	xdfs_printk("xdfs_write_begin is called\n");
	ret = block_write_begin(mapping, pos, len, pagep, xdfs_get_block);
	xdfs_printk("xdfs_write_begin return\n");
	return ret;
}
static int 
xdfs_write_end(struct file *file, struct address_space *mapping,
			loff_t pos, unsigned len, unsigned copied,
			struct page *page, void *fsdata)
{
	int ret;
	xdfs_printk("xdfs_write_end is called\n");
	ret = generic_write_end(file, mapping, pos, len, copied, page, fsdata);
	xdfs_printk("xdfs_write_end return\n");
	return ret;
}

static void xdfs_readahead(struct readahead_control *rac)
{
	xdfs_printk("readahead is called \n");
	mpage_readahead(rac, xdfs_get_block);
	xdfs_printk("readahead return \n");
}
static int xdfs_read_folio(struct file *file, struct folio *folio)
{
	int ret;
	xdfs_printk("read_folio is called\n");
	ret = mpage_read_folio(folio, xdfs_get_block);
	xdfs_printk("read_folio return\n");
	return ret;
}

static sector_t xdfs_bmap(struct address_space *mapping, sector_t block)
{
	xdfs_printk("xdfs_bmap is called and return \n");
	return generic_block_bmap(mapping,block,xdfs_get_block);
}

int 
xdfs_get_block(struct inode *inode, sector_t iblock,
		struct buffer_head *bh_result, int create)
{
	unsigned max_blocks = bh_result->b_size >> inode->i_blkbits;
	
	int ret;

	xdfs_printk("xdfs_get_block is called \n");

	ret = xdfs_get_blocks(inode, iblock, max_blocks,
			      bh_result, create);

	if (ret <= 0)
		return ret;

	bh_result->b_size = (ret << inode->i_blkbits);
	
	xdfs_printk("xdfs_get_block return \n");
	return 0;

}

static int 
xdfs_get_blocks(struct inode *inode,
			   sector_t iblock, unsigned long maxblocks,
			   struct buffer_head *bh_result,
			   int create)
{
	xdfs_printk("xdfs_get_block is called \n");
	struct xdfs_inode* xd_inode = (struct xdfs_inode*)inode->i_private;
	struct super_block *sb = inode->i_sb;

	iblock = xd_inode->file_blockno;
	maxblocks = xd_inode->using_block_num;
	bh_result = sb_bread(sb,xd_inode->file_blockno);

	map_bh(bh_result, inode->i_sb, iblock);

	xdfs_printk("xdfs_get_block return \n");
	return 0;
}
static struct inode_operations xdfs_inode_operations = {
    //这个地方我觉得编写的有问题//是否需要
	.listxattr	    = xdfs_listxattr,
	.getattr	= xdfs_getattr,
	.setattr	= xdfs_setattr,
    //link : my_link,
    //unlink : my_unlink,
};


static ssize_t
xdfs_listxattr(struct dentry *dentry, char *buffer, size_t buffer_size)
{
	struct inode *inode = dentry->d_inode;
	struct buffer_head *bh = NULL;
	char *end;
	size_t rest = buffer_size;
	int error;

	xdfs_printk("xdfs_listxattr is called\n");

// 	down_read(&EXT2_I(inode)->xattr_sem);
// 	error = 0;
// 	if (!EXT2_I(inode)->i_file_acl)
// 		goto cleanup;
// 	ea_idebug(inode, "reading block %d", EXT2_I(inode)->i_file_acl);
// 	bh = sb_bread(inode->i_sb, EXT2_I(inode)->i_file_acl);
// 	error = -EIO;
// 	if (!bh)
// 		goto cleanup;
// 	ea_bdebug(bh, "b_count=%d, refcount=%d",
// 		atomic_read(&(bh->b_count)), le32_to_cpu(HDR(bh)->h_refcount));
// 	end = bh->b_data + bh->b_size;
// 	if (HDR(bh)->h_magic != cpu_to_le32(EXT2_XATTR_MAGIC) ||
// 	    HDR(bh)->h_blocks != cpu_to_le32(1)) {
// bad_block:	ext2_error(inode->i_sb, "ext2_xattr_list",
// 			"inode %ld: bad block %d", inode->i_ino,
// 			EXT2_I(inode)->i_file_acl);
// 		error = -EIO;
// 		goto cleanup;
// 	}

// 	/* check the on-disk data structure */
// 	entry = FIRST_ENTRY(bh);
// 	while (!IS_LAST_ENTRY(entry)) {
// 		struct ext2_xattr_entry *next = EXT2_XATTR_NEXT(entry);

// 		if ((char *)next >= end)
// 			goto bad_block;
// 		entry = next;
// 	}
// 	if (ext2_xattr_cache_insert(bh))
// 		ea_idebug(inode, "cache insert failed");

// 	/* list the attribute names */
// 	for (entry = FIRST_ENTRY(bh); !IS_LAST_ENTRY(entry);
// 	     entry = EXT2_XATTR_NEXT(entry)) {
// 		const struct xattr_handler *handler =
// 			ext2_xattr_handler(entry->e_name_index);

// 		if (handler) {
// 			size_t size = handler->list(dentry, buffer, rest,
// 						    entry->e_name,
// 						    entry->e_name_len,
// 						    handler->flags);
// 			if (buffer) {
// 				if (size > rest) {
// 					error = -ERANGE;
// 					goto cleanup;
// 				}
// 				buffer += size;
// 			}
// 			rest -= size;
// 		}
// 	}
// 	error = buffer_size - rest;  /* total size */

	xdfs_printk("xdfs_listxattr return\n");

cleanup:
	brelse(bh);
	// up_read(&XDFS_I(inode)->xattr_sem);

	xdfs_printk("xdfs_listxattr return error !!!\n");
	
	return error;
}

int 
xdfs_getattr(struct user_namespace *mnt_userns, const struct path *path,
		 struct kstat *stat, u32 request_mask, unsigned int query_flags)
{

	struct inode *inode = d_inode(path->dentry);
	xdfs_printk("xdfs_getattr is called\n");


	stat->attributes_mask |= (STATX_ATTR_APPEND |
			STATX_ATTR_COMPRESSED |
			STATX_ATTR_ENCRYPTED |
			STATX_ATTR_IMMUTABLE |
			STATX_ATTR_NODUMP);
	generic_fillattr(&init_user_ns, inode, stat);
	printk("%x stat->dev = inode->i_sb->s_dev",stat->dev);
	printk("%x stat->ino = inode->i_ino",stat->ino);
	printk("%x stat->mode = inode->i_mode",stat->mode);
	printk("%x stat->nlink = inode->i_nlink",stat->nlink);
	printk("%x stat->uid = vfsuid_into_kuid(vfsuid)",stat->uid);
	printk("%x stat->gid = vfsgid_into_kgid(vfsgid)",stat->gid);
	printk("%x stat->rdev = inode->i_rdev",stat->rdev);
	printk("%x stat->size = i_size_read(inode)",stat->size);
	printk("%x stat->atime = inode->i_atime",stat->atime);
	printk("%x stat->mtime = inode->i_mtime",stat->mtime);
	printk("%x stat->ctime = inode->i_ctime",stat->ctime);
	printk("%x stat->blksize = i_blocksize(inode)",stat->blksize);
	printk("%x stat->blocks = inode->i_blocks",stat->blocks);
	xdfs_printk("xdfs_getattr return\n");
	return 0;
}
int 
xdfs_setattr(struct user_namespace *mnt_userns, struct dentry *dentry,
		 struct iattr *iattr)
{
	struct inode *inode = d_inode(dentry);
	int error;

	xdfs_printk("xdfs_setattr is called\n");

	error = setattr_prepare(&init_user_ns, dentry, iattr);
	if (error)
	{
		return error;
	}
	
	if (iattr->ia_valid & ATTR_SIZE && iattr->ia_size != inode->i_size) {
		error = xdfs_setsize(inode, iattr->ia_size);
		if (error)
			return error;
	}
	setattr_copy(&init_user_ns, inode, iattr);
	if (iattr->ia_valid & ATTR_MODE)
		error = posix_acl_chmod(&init_user_ns, dentry, inode->i_mode);
	mark_inode_dirty(inode);
	
	xdfs_printk("xdfs_setattr return\n");
	return error;
}



static struct file_operations xdfs_file_operations = {
	open : xdfs_open,
	read_iter : xdfs_read_iter,
	write_iter : xdfs_write_iter,
	splice_read : xdfs_splice_read,
	splice_write : xdfs_splice_write,
	llseek : xdfs_file_llseek,
	mmap : xdfs_file_mmap,
	release : xdfs_release_file,
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
xdfs_release_file (struct inode * inode, struct file * filp)
{
	int error;
#ifdef XDFS_DEBUG
	printk("XDFS: release_file is called\n");
#endif


#ifdef XDFS_DEBUG
	printk("XDFS: release_file return\n");
#endif
	return 0;
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
	.fsync		= xdfs_fsync,
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
	// ssize_t generic_read_dir(struct file *filp, char __user *buf, size_t siz, loff_t *ppos)
	// {
	// 	return -EISDIR;
	// }
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

static int xdfs_fsync(struct file *file, loff_t start, loff_t end, int datasync)
{
	int ret;
	// struct super_block *sb = file->f_mapping->host->i_sb;
	xdfs_printk("xdfs_fsync is called\n");

	ret = generic_file_fsync(file, start, end, datasync);
	if (ret == -EIO)
	{
		xdfs_printk("xdfs_fsync return -EIO\n");
	}
	
	xdfs_printk("xdfs_fsync return \n");
	return ret;
}
static struct inode_operations xdfs_inode_dir_operations = {
	.create		    = xdfs_dir_create,
	// .lookup		    = xdfs_dir_lookup,
	// .link		    = xdfs_dir_link,
	// .unlink		    = xdfs_dir_unlink,
	// .symlink	    = xdfs_dir_symlink,
	// .mkdir		    = xdfs_dir_mkdir,
	// .rmdir		    = xdfs_dir_rmdir,
	// .mknod		    = xdfs_dir_mknod,
	// .rename		    = xdfs_dir_rename,
	.listxattr	    = xdfs_dir_listxattr,
	.getattr	    = xdfs_dir_getattr,
	.setattr	    = xdfs_dir_setattr,
	// .get_inode_acl	= xdfs_dir_get_acl,
	// .set_acl	    = xdfs_dir_set_acl,
	// .tmpfile	    = xdfs_dir_tmpfile,
	// .fileattr_get	= xdfs_dir_fileattr_get,
	// .fileattr_set	= xdfs_dir_fileattr_set,
};

static int 
xdfs_dir_create (struct user_namespace * mnt_userns,
			struct inode * dir, struct dentry * dentry,
			umode_t mode, bool excl)
{
	xdfs_printk("dir_create is called\n");
	/* 1. new inode */
	/* 2. set file opeartions */
	/* 3. sync to disk */
	/* 4. add nondir */
	xdfs_printk("dir_create return\n");
	return 0;
}

static ssize_t
xdfs_dir_listxattr(struct dentry *dentry, char *buffer, size_t buffer_size)
{
	struct inode *inode = dentry->d_inode;
	struct buffer_head *bh = NULL;
	char *end;
	size_t rest = buffer_size;
	int error;

	xdfs_printk("xdfs_dir_listxattr is called\n");

// 	down_read(&EXT2_I(inode)->xattr_sem);
// 	error = 0;
// 	if (!EXT2_I(inode)->i_file_acl)
// 		goto cleanup;
// 	ea_idebug(inode, "reading block %d", EXT2_I(inode)->i_file_acl);
// 	bh = sb_bread(inode->i_sb, EXT2_I(inode)->i_file_acl);
// 	error = -EIO;
// 	if (!bh)
// 		goto cleanup;
// 	ea_bdebug(bh, "b_count=%d, refcount=%d",
// 		atomic_read(&(bh->b_count)), le32_to_cpu(HDR(bh)->h_refcount));
// 	end = bh->b_data + bh->b_size;
// 	if (HDR(bh)->h_magic != cpu_to_le32(EXT2_XATTR_MAGIC) ||
// 	    HDR(bh)->h_blocks != cpu_to_le32(1)) {
// bad_block:	ext2_error(inode->i_sb, "ext2_xattr_list",
// 			"inode %ld: bad block %d", inode->i_ino,
// 			EXT2_I(inode)->i_file_acl);
// 		error = -EIO;
// 		goto cleanup;
// 	}

// 	/* check the on-disk data structure */
// 	entry = FIRST_ENTRY(bh);
// 	while (!IS_LAST_ENTRY(entry)) {
// 		struct ext2_xattr_entry *next = EXT2_XATTR_NEXT(entry);

// 		if ((char *)next >= end)
// 			goto bad_block;
// 		entry = next;
// 	}
// 	if (ext2_xattr_cache_insert(bh))
// 		ea_idebug(inode, "cache insert failed");

// 	/* list the attribute names */
// 	for (entry = FIRST_ENTRY(bh); !IS_LAST_ENTRY(entry);
// 	     entry = EXT2_XATTR_NEXT(entry)) {
// 		const struct xattr_handler *handler =
// 			ext2_xattr_handler(entry->e_name_index);

// 		if (handler) {
// 			size_t size = handler->list(dentry, buffer, rest,
// 						    entry->e_name,
// 						    entry->e_name_len,
// 						    handler->flags);
// 			if (buffer) {
// 				if (size > rest) {
// 					error = -ERANGE;
// 					goto cleanup;
// 				}
// 				buffer += size;
// 			}
// 			rest -= size;
// 		}
// 	}
// 	error = buffer_size - rest;  /* total size */

	xdfs_printk("xdfs_dir_listxattr return\n");

cleanup:
	brelse(bh);
	// up_read(&XDFS_I(inode)->xattr_sem);

	xdfs_printk("xdfs_dir_listxattr return error !!!\n");
	
	return error;
}



int 
xdfs_dir_getattr(struct user_namespace *mnt_userns, const struct path *path,
		 struct kstat *stat, u32 request_mask, unsigned int query_flags)
{

	struct inode *inode = d_inode(path->dentry);
	xdfs_printk("xdfs_dir_getattr is called\n");

	stat->attributes_mask |= (STATX_ATTR_APPEND |
			STATX_ATTR_COMPRESSED |
			STATX_ATTR_ENCRYPTED |
			STATX_ATTR_IMMUTABLE |
			STATX_ATTR_NODUMP);
	generic_fillattr(&init_user_ns, inode, stat);
	printk("%x stat->dev = inode->i_sb->s_dev",stat->dev);
	printk("%x stat->ino = inode->i_ino",stat->ino);
	printk("%x stat->mode = inode->i_mode",stat->mode);
	printk("%x stat->nlink = inode->i_nlink",stat->nlink);
	printk("%x stat->uid = vfsuid_into_kuid(vfsuid)",stat->uid);
	printk("%x stat->gid = vfsgid_into_kgid(vfsgid)",stat->gid);
	printk("%x stat->rdev = inode->i_rdev",stat->rdev);
	printk("%x stat->size = i_size_read(inode)",stat->size);
	printk("%x stat->atime = inode->i_atime",stat->atime);
	printk("%x stat->mtime = inode->i_mtime",stat->mtime);
	printk("%x stat->ctime = inode->i_ctime",stat->ctime);
	printk("%x stat->blksize = i_blocksize(inode)",stat->blksize);
	printk("%x stat->blocks = inode->i_blocks",stat->blocks);
	xdfs_printk("xdfs_dir_getattr return\n");
	return 0;
}
int 
xdfs_dir_setattr(struct user_namespace *mnt_userns, struct dentry *dentry,
		 struct iattr *iattr)
{
	struct inode *inode = d_inode(dentry);
	int error;

	xdfs_printk("xdfs_dir_setattr is called\n");

	error = setattr_prepare(&init_user_ns, dentry, iattr);
	if (error)
	{
		return error;
	}
	
	if (iattr->ia_valid & ATTR_SIZE && iattr->ia_size != inode->i_size) {
		error = xdfs_setsize(inode, iattr->ia_size);
		if (error)
			return error;
	}
	setattr_copy(&init_user_ns, inode, iattr);
	if (iattr->ia_valid & ATTR_MODE)
		error = posix_acl_chmod(&init_user_ns, dentry, inode->i_mode);
	mark_inode_dirty(inode);
	
	xdfs_printk("xdfs_dir_setattr return\n");
	return error;
}

static int
xdfs_setsize(struct inode *inode, loff_t newsize)
{
	int error;
	xdfs_printk("xdfs_setize is called\n");

	if (!(S_ISREG(inode->i_mode) || S_ISDIR(inode->i_mode) ||
	    S_ISLNK(inode->i_mode)))
		return -EINVAL;
	// if (ext2_inode_is_fast_symlink(inode))
	// 	return -EINVAL;
	if (IS_APPEND(inode) || IS_IMMUTABLE(inode))
		return -EPERM;

	inode_dio_wait(inode);

	error = block_truncate_page(inode->i_mapping,
			newsize, xdfs_get_block);
	if (error)
		return error;

	filemap_invalidate_lock(inode->i_mapping);
	truncate_setsize(inode, newsize);
	// __ext2_truncate_blocks(inode, newsize);
	filemap_invalidate_unlock(inode->i_mapping);

	inode->i_mtime = inode->i_ctime = current_time(inode);
	if (inode_needs_sync(inode)) {
		sync_mapping_buffers(inode->i_mapping);
		sync_inode_metadata(inode, 1);
	} else {
		mark_inode_dirty(inode);
	}
	xdfs_printk("xdfs_setize return\n");
	return 0;
}

static inline struct xdfs_inode_info *XDFS_I(struct inode *inode)
{
	return container_of(inode, struct xdfs_inode_info, vfs_inode);
}



module_init(init_xdfs_fs)
module_exit(exit_xdfs_fs)