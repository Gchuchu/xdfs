#include <stdio.h>
#include <stdint.h>  
#include <fcntl.h>
#include <iostream>
#include <time.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <linux/fs.h>
typedef int8_t INT8;
typedef int16_t INT16;
typedef int32_t INT32;
typedef int64_t INT64;

typedef uint8_t UINT8;
typedef uint16_t UINT16;
typedef uint32_t UINT32;
typedef uint64_t UINT64;

/*************************DEFINE_START*************************/ 

/*************************SYSTEM_DEFINE*************************/

#define ERROR 1
#define OK 0

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

#define XDFS_INODE_SIZE (sizeof(struct xdfs_inode))

#define DT_REG  0
#define DT_DIR  1
#define DT_LNK  2
#define DT_CHR  3
#define DT_BLK  4
#define DT_FIFO	5
#define DT_SOCK	6
#define FT_REG_FILE	0
#define FT_DIR		1
#define FT_SYMLINK	2
#define FT_CHRDEV	3
#define FT_BLKDEV	4
#define FT_FIFO		5
#define FT_SOCK		6
#define XDFS_BYTE_ORDER LITTLE_BYTE_ORDER	/* 默认小端字节序 */ 

const int BLKSIZE = 4096;	/* 一个块4KB */ 
const unsigned BLKSIZE_BITS = 21;
const int NUM_BLK = 32768;	/* 一共32768个块，128MB */ 


/*************************PBR_DEFINE*************************/

#define PBR_JMP             0x00	/* x86 jump instruction (3 bytes) */
#define PBR_SYS_ID          0x03	/* system ID string     (8 bytes) */
#define PBR_BYTES_PER_BLK   0x0b	/* bytes per block      (2 bytes) */
#define PBR_NUM_BLKS        0x20	/* # of blocks on vol   (4 bytes) */
#define PBR_NBLKS 1					/*　PBR很小，１块就够了　*/ 


/*************************FSB_DEFINE*************************/


#define FSB_NBLKS 1	/* 4096 * 8 = 37268,所以用一块正好可以做文件系统的FSB */ 


/*************************FIB_DEFINE*************************/

#define FIB_NBLKS 1	/*　一共4096个inode，1块就够了　*/


/*************************INODE_SET_DEFINE*************************/


#define INODE_SET_NBLKS 120	/* 一个inode数据结构120字节，所以需要120个块 */
#define INODE_NUMS  4096	/* 一共4096个inode */
#define XDFS_DIRECT_BLOCKS 16	/* The max num of blks a inode can have */
#define XDFS_ROOT_INO 2			/* This value should equal the num of non-data zones */
    
typedef INT64   xdfsTime_t;
typedef INT64	fsize_t;
typedef UINT32  fgen_t;
typedef UINT32  fid_t;
typedef UINT16  linkCount_t;
typedef UINT16  fmode_t;
typedef unsigned short		umode_t;
typedef struct {
	int counter;
} atomic_t;


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

struct xdfs_dir_entry{
	UINT32 inode_no;
	UINT16 dir_entry_len; 
	UINT8 name_len; 
	UINT8 file_type; 
	char name[255]; 
};

/*************************INODE_LOG_DEFINE*****************/

#define INODE_LOG_NBLKS 1 /* INODE_LOG的大小恰好为512字节，因此占用1个块 */

/*************************METABITMAP_DEFINE*********************/

#define METABITMAP_NBLKS 1 /* FSB_METABITMAP与FIB_METABITMAP每一个都占用1块 */

/*************************GLOBAL_INFO_DEFINE*********************/

#define GLOBAL_INFO_NBLKS 1	/* GLOBAL_INFO有两个，每一个都占用1个块 */

/*************************SUPERBLOCK_DEFINE*************************/

#define SUPERBLOCK_NBLKS 1	/* SUPERBLOCK大小为96字节，占用1个块 */
#define XDFS_ID_STRING "xdfs"	/*　文件系统的ID　*/


struct xdfs_superblock {
    UINT32        s_magic;
    UINT32        s_state;	/* The superblock state : XDFS_DIRTY or XDFS_CLEAN */
    UINT32        s_nifree;	/* The num of free inode */
    UINT32        s_inode;	/* The number of FIB0 block */
    UINT32        s_nbfree;	/* The num of free blk */
    UINT32        s_block;	/* The num of blocks */
	struct buffer_head* bh;
};

/*************************DEFINE_END*************************/ 

/**
 * 使用方法
 * ./<name> /mnt/testfs
*/
int main(int argc, char *argv[])
{
	int error;
			
	/* 参数有效性检查 */
	/*	if(argc != 2)
	{
		fprintf(stderr, "xdfs: need to specify device\n");
		return (ERROR);
	} */

	/* 打开设备 */
	int devfd;
	devfd = open(argv[1], O_WRONLY); 
	if(devfd < 0)
	{
		fprintf(stderr, "xdfs: failed to open device\n");
		return (ERROR);
	}

	/* 文件系统大小检查 */
	error = lseek(devfd, (off_t)(NUM_BLK * BLKSIZE), SEEK_SET);
	if(error == -1)
	{
		fprintf(stderr, "xdfs: cannot create filesystem of specified size\n");
		return (ERROR);
	}
	lseek(devfd, 0, SEEK_SET); 
	
/*************************PBR_START*************************/

printf("PBR START\n");

	lseek(devfd, PBR_BLKNO*BLKSIZE, SEEK_SET);
	char *p_pbr;
	p_pbr = (char *)malloc(BLKSIZE * PBR_NBLKS);
	if (p_pbr == NULL)
    	return (ERROR);
	memset(p_pbr, 0xff, BLKSIZE * PBR_NBLKS);	/* 缓冲区置0xff */ 
    
	/* PBR内容填充 */
	p_pbr[PBR_JMP] = 0xe9;
	strcpy(p_pbr + PBR_SYS_ID, "xdfs"); 
	
	/* 文件系统PBR需以小端字节序写入 */
#if (LTFS_BYTE_ORDER == LITTLE_BYTE_ORDER)
    *((UINT32 *)(&p_pbr[PBR_NUM_BLKS])) = NUM_BLK;
#else
    p_pbr[PBR_NUM_BLKS]   = (NUM_BLK) & 0xFF;
    p_pbr[PBR_NUM_BLKS+1] = (NUM_BLK >> 8) & 0xFF;
    p_pbr[PBR_NUM_BLKS+2] = (NUM_BLK >> 16) & 0xFF;
    p_pbr[PBR_NUM_BLKS+3] = (NUM_BLK >> 24) & 0xFF;
#endif

	/* PBR_BYTES_PER_BLK是一个奇数，因此无论字节顺序如何，我们都必须一次写入一个字节 */
	p_pbr[PBR_BYTES_PER_BLK]     = BLKSIZE & 0xFF;
    p_pbr[PBR_BYTES_PER_BLK + 1] = (BLKSIZE >> 8) & 0xFF;
	
	/* PBR写入设备 */
	write(devfd, p_pbr, BLKSIZE * PBR_NBLKS);
	 
	free(p_pbr); 
	
printf("PBR END\n");

/*************************PBR_END*************************/	


/*************************FSB0_START*************************/
	
printf("FSB0 START\n");
	
	lseek(devfd, FSB0_BLKNO*BLKSIZE, SEEK_SET);
	char *p_fsb0;
	p_fsb0 = (char *)malloc(BLKSIZE * FSB_NBLKS);
	if (p_fsb0 == NULL)
    {
    	printf("p_fsb0 malloc failed!");
    	return (ERROR);
	}
	memset(p_fsb0, 0xff, BLKSIZE * FSB_NBLKS);	/*缓冲区置0xff，表示均未使用*/ 
	
	/* 第1位用来表示保存PBR */
	p_fsb0[0] = p_fsb0[0] & 0b11111110 ;
	/* 第2位用来表示保存FSB0 */
	p_fsb0[0] = p_fsb0[0] & 0b11111101;
	/* 第3位用来表示保存FIB0 */ 
	p_fsb0[0] = p_fsb0[0] & 0b11111011;
	/* 第4位用来表示保存FSB1 */ 
	p_fsb0[0] = p_fsb0[0] & 0b11110111;
	/* 第5位用来表示保存FIB1 */ 
	p_fsb0[0] = p_fsb0[0] & 0b11101111;

printf("part 1 is OK\n");
	
	/* 接下来的120位表示保存INODE_SET */
	p_fsb0[0] = p_fsb0[0] & 0b00011111;
	for(int i = 0; i < int((120 - 3) / 8); i++)
		p_fsb0[i + 1] = p_fsb0[i + 1] & 0b00000000;
	p_fsb0[15] = p_fsb0[15] & 0b11111000;

printf("part 2 is OK\n");
	
	/* 第126位用来表示保存INODE_LOG */ 
	p_fsb0[15] = p_fsb0[15] & 0b11110111;	
	
	/* 第127、128、129、130位用来表示保存FSB_METABITMAP与FIB_METABITMAP */ 
	p_fsb0[15] = p_fsb0[15] & 0b00001111;
	
	/* 第131位用来表示保存GLOBAL_INFO */
	p_fsb0[16] = p_fsb0[16] & 0b11111110;
		
	/* FSB0的最后1位用来表示保存SUPERBLOCK */
	p_fsb0[BLKSIZE - 1] = p_fsb0[BLKSIZE - 1] & 0b01111111;	

printf("part 3 is OK\n");
		 
	/* FSB0写入设备 */
	write(devfd, p_fsb0, BLKSIZE * FSB_NBLKS);
	
	free(p_fsb0); 

printf("FSB0 END\n");	

/*************************FSB0_END*************************/


/*************************FIB0_START*************************/

printf("FIB0 START\n");	

	lseek(devfd, FSB1_BLKNO*BLKSIZE, SEEK_SET);
	char *p_fib0;
	p_fib0 = (char *)malloc(BLKSIZE * FIB_NBLKS);
	if (p_fib0 == NULL)
    	return (ERROR);
	memset(p_fib0, 0xff, BLKSIZE * FIB_NBLKS);	/*缓冲区置0xff，表示均未使用*/ 
	
	/* FIB0写入设备 */
	write(devfd, p_fib0, BLKSIZE * FIB_NBLKS);
	
	free(p_fib0);

printf("FIB0 END\n");

/*************************FIB0_END*************************/


/*************************FSB1_START*************************/

printf("FSB1 START\n");
	
	lseek(devfd, FIB0_BLKNO*BLKSIZE, SEEK_SET);
	char *p_fsb1;
	p_fsb1 = (char *)malloc(BLKSIZE * FSB_NBLKS);
	if (p_fsb1 == NULL)
    	return (ERROR);
	memset(p_fsb1, 0xff, BLKSIZE * FSB_NBLKS);	/*缓冲区置0xff，表示均未使用*/ 
	
	/* 第1位用来表示保存PBR */
	p_fsb1[0] = p_fsb1[0] & 0b11111110 ;
	/* 第2位用来表示保存FSB0 */
	p_fsb1[0] = p_fsb1[0] & 0b11111101;
	/* 第3位用来表示保存FIB0 */ 
	p_fsb1[0] = p_fsb1[0] & 0b11111011;
	/* 第4位用来表示保存FSB1 */ 
	p_fsb1[0] = p_fsb1[0] & 0b11110111;
	/* 第5位用来表示保存FIB1 */ 
	p_fsb1[0] = p_fsb1[0] & 0b11101111;

printf("part 1 is OK\n");
	
	/* 接下来的120位表示保存INODE_SET */
	p_fsb1[0] = p_fsb1[0] & 0b00011111;
	for(int i = 0; i < ((120 - 3) / 8); i++)
		p_fsb1[i + 1] = p_fsb1[i + 1] & 0b00000000;
	p_fsb1[15] = p_fsb1[15] & 0b11111000;

printf("part 2 is OK\n");
	
	/* 第126位用来表示保存INODE_LOG */ 
	p_fsb1[15] = p_fsb1[15] & 0b11110111;	
	
	/* 第127、128、129、130位用来表示保存FSB_METABITMAP与FIB_METABITMAP */ 
	p_fsb1[15] = p_fsb1[15] & 0b00001111;
	
	/* 第131位用来表示保存GLOBAL_INFO */
	p_fsb1[16] = p_fsb1[16] & 0b11111110;
		
	/* FSB1的最后1位用来表示保存SUPERBLOCK */
	p_fsb1[BLKSIZE - 1] = p_fsb1[BLKSIZE - 1] & 0b01111111;		 

printf("part 3 is OK\n");
	
	/* FSB1写入设备 */
	write(devfd, p_fsb1, BLKSIZE * FSB_NBLKS);
	
	free(p_fsb1);

printf("FSB1 END\n");
	 
/*************************FSB1_END*************************/


/*************************FIB1_START*************************/

printf("FIB1 START\n");
	
	lseek(devfd, FIB1_BLKNO, SEEK_SET);
	char *p_fib1;
	p_fib1 = (char *)malloc(BLKSIZE * FIB_NBLKS);
	if (p_fib1 == NULL)
    	return (ERROR);
	memset(p_fib1, 0xff, BLKSIZE * FIB_NBLKS);	/*缓冲区置0xff，表示均未使用*/ 
	
	/* FIB1写入设备 */
	write(devfd, p_fib1, BLKSIZE * FIB_NBLKS);
	
	free(p_fib1);

printf("FIB1 END\n");
	
/*************************FIB1_END*************************/
	

/*************************INODE_SET_START*************************/
	
printf("INODE_SET START\n");
	
	lseek(devfd, INODE_SET_BLKNO*BLKSIZE, SEEK_SET);
	char *p_inode_set;
	p_inode_set = (char *)malloc(1 * sizeof(struct xdfs_inode));
	if (p_inode_set == NULL)
    	return (ERROR);
	memset(p_inode_set, 0x0, 1 * sizeof(struct xdfs_inode));	/*缓冲区置0xff，表示均未使用*/ 

printf("part 1 is OK\n");

	int inode_num = BLKSIZE/XDFS_INODE_SIZE;
	int blknum = INODE_NUMS/inode_num+1;//25 when size = 168
	int yu = (XDFS_INODE_SIZE*INODE_NUMS)%BLKSIZE;
	for(int i=0;i<blknum-1;i++)
	{
		lseek(devfd, (INODE_SET_BLKNO+i)*BLKSIZE, SEEK_SET);
		for(int j=0;j<inode_num;j++)
		{
			if(i==0&&j==2)
			{
				struct xdfs_inode* root_inode = (struct xdfs_inode*)p_inode_set;
				root_inode->num_link = 1;
				root_inode->uid = 0;
				root_inode->gid = 0;
				root_inode->using_block_num = 1;
				root_inode->addr[0] = DS_BLKNO;
				root_inode->file_size = sizeof(struct xdfs_dir_entry);
				root_inode->blockno = INODE_SET_BLKNO;
				write(devfd, p_inode_set, 1 * sizeof(struct xdfs_inode));
				memset(p_inode_set, 0x0, 1 * sizeof(struct xdfs_inode));
				continue;
			}
			write(devfd, p_inode_set, 1 * sizeof(struct xdfs_inode));
		}
	}
	lseek(devfd, (INODE_SET_BLKNO+blknum-1)*BLKSIZE, SEEK_SET);
	for(int i=0;i<yu;i++)
	{
		write(devfd, p_inode_set, 1 * sizeof(struct xdfs_inode));
	}
	
printf("part 2 is OK\n");
	
	free(p_inode_set);

printf("INODE_SET END\n");
	
/*************************INODE_SET_END*************************/


/*************************INODE_LOG_START*************************/

printf("INODE_LOG START\n");

	lseek(devfd, INODE_LOG_BLKNO, SEEK_SET);
	char *p_inode_log;
	p_inode_log = (char *)malloc(BLKSIZE * INODE_LOG_NBLKS);
	if (p_inode_log == NULL)
    	return (ERROR);
	memset(p_inode_log, 0xff, BLKSIZE * INODE_LOG_NBLKS);	/*缓冲区置0xff*/ 
	write(devfd, p_inode_log, BLKSIZE * INODE_LOG_NBLKS);

printf("part 1 is OK\n");

	free(p_inode_log);

printf("INODE_LOG END\n");
	
/*************************INODE_LOG_END*************************/


/*************************FSB_METABITMAP0_START*************************///Metabitmap 

printf("FSB_METABITMAP0 START\n");

	lseek(devfd, FSB_MB0_BLKNO*BLKSIZE, SEEK_SET);
	char *p_fsb_metabitmap0;
	p_fsb_metabitmap0 = (char *)malloc(BLKSIZE * METABITMAP_NBLKS);
	if (p_fsb_metabitmap0 == NULL)
    	return (ERROR);
	memset(p_fsb_metabitmap0, 0x00, BLKSIZE * METABITMAP_NBLKS);	/*缓冲区置0x00*/ 
	write(devfd, p_fsb_metabitmap0, BLKSIZE * METABITMAP_NBLKS);
	free(p_fsb_metabitmap0);

printf("FSB_METABITMAP0 END\n");
	
/*************************FSB_METABITMAP0_END*************************/


/*************************FSB_METABITMAP1_START*************************/

printf("FSB_METABITMAP1 START\n");

	lseek(devfd, FSB_MB1_BLKNO*BLKSIZE, SEEK_SET);
	char *p_fsb_metabitmap1;
	p_fsb_metabitmap1 = (char *)malloc(BLKSIZE * METABITMAP_NBLKS);
	if (p_fsb_metabitmap1 == NULL)
    	return (ERROR);
	memset(p_fsb_metabitmap1, 0x00, BLKSIZE * METABITMAP_NBLKS);	/*缓冲区置0x00*/ 
	write(devfd, p_fsb_metabitmap1, BLKSIZE * METABITMAP_NBLKS);
	free(p_fsb_metabitmap1);

printf("FSB_METABITMAP1 END\n");

/*************************FSB_METABITMAP1_END*************************/


/*************************FIB_METABITMAP0_START*************************/

printf("FIB_METABITMAP0 START\n");

	lseek(devfd, FIB_MB0_BLKNO*BLKSIZE, SEEK_SET);
	char *p_fib_metabitmap0;
	p_fib_metabitmap0 = (char *)malloc(BLKSIZE * METABITMAP_NBLKS);
	if (p_fib_metabitmap0 == NULL)
    	return (ERROR);
	memset(p_fib_metabitmap0, 0x00, BLKSIZE * METABITMAP_NBLKS);	/*缓冲区置0x00*/ 
	write(devfd, p_fib_metabitmap0, BLKSIZE * METABITMAP_NBLKS);
	free(p_fib_metabitmap0);

printf("FIB_METABITMAP0 END\n");

/*************************FIB_METABITMAP0_END*************************/


/*************************FIB_METABITMAP1_START*************************/

printf("FIB_METABITMAP1 START\n");

	lseek(devfd, FIB_MB1_BLKNO*BLKSIZE, SEEK_SET);
	char *p_fib_metabitmap1;
	p_fib_metabitmap1 = (char *)malloc(BLKSIZE * METABITMAP_NBLKS);
	if (p_fib_metabitmap1 == NULL)
    	return (ERROR);
	memset(p_fib_metabitmap1, 0x00, BLKSIZE * METABITMAP_NBLKS);	/*缓冲区置0x00*/ 
	write(devfd, p_fib_metabitmap1, BLKSIZE * METABITMAP_NBLKS);
	free(p_fib_metabitmap1);

printf("FIB_METABITMAP0 END\n");

/*************************FIB_METABITMAP1_END*************************/


/*************************GLOBAL_INFO0_START*************************///globalinf 

printf("GLOBAL_INFO0 START\n");

	lseek(devfd, GTI0_BLKNO*BLKSIZE, SEEK_SET);
	char *p_global_info0;
	p_global_info0 = (char *)malloc(BLKSIZE * GLOBAL_INFO_NBLKS);
	if (p_global_info0 == NULL)
    	return (ERROR);
	memset(p_global_info0, 0x00, BLKSIZE * GLOBAL_INFO_NBLKS);	/*缓冲区置0x00*/
	write(devfd, p_global_info0, BLKSIZE * GLOBAL_INFO_NBLKS);
	free(p_global_info0);

printf("GLOBAL_INFO0 END\n");

/*************************GLOBAL_INFO0_SEND*************************/


/*************************GLOBAL_INFO1_START*************************/

printf("GLOBAL_INFO1 START\n");

	lseek(devfd, GTI1_BLKNO*BLKSIZE, SEEK_SET);
	char *p_global_info1;
	p_global_info1 = (char *)malloc(BLKSIZE * GLOBAL_INFO_NBLKS);
	if (p_global_info1 == NULL)
    	return (ERROR);
	memset(p_global_info1, 0x00, BLKSIZE * GLOBAL_INFO_NBLKS);	/*缓冲区置0x00*/ 
	write(devfd, p_global_info1, BLKSIZE * GLOBAL_INFO_NBLKS);
	free(p_global_info1);

printf("GLOBAL_INFO1 END\n");

/*************************GLOBAL_INFO1_END*************************/


/*************************DATA_SPACE_START*************************/
printf("DATA_SPACE START\n");

	lseek(devfd, DS_BLKNO*BLKSIZE, SEEK_SET);
	char *a = (char*)malloc((SB_BLKNO-DS_BLKNO)*BLKSIZE);
	memset(a,0,(SB_BLKNO-DS_BLKNO)*BLKSIZE);
	write(devfd, a, (SB_BLKNO-DS_BLKNO)*BLKSIZE);
	free(a);

	struct xdfs_dir_entry* de;
	const char *str = "what";
	int name_len = strlen(str);
	de = (struct xdfs_dir_entry *)malloc(sizeof(struct xdfs_dir_entry));
	de->file_type = FT_DIR;
	strcpy(de->name,str);
	de->inode_no = XDFS_ROOT_INO;
	de->name_len = name_len;
	de->dir_entry_len = sizeof(de);
	if (de == NULL)
    	return (ERROR);
	memset(de, 0x00, sizeof(struct xdfs_dir_entry));	/*缓冲区置0x00*/
	write(devfd, de, sizeof(struct xdfs_dir_entry));
	free(de);

printf("DATA_SPACE END\n");
/*************************DATA_SPACE_END*************************/



/*************************SUPERBLOCK_START*************************/

printf("SUPERBLOCK START\n");

	char * p_data;
	UINT32 subTotal;
	UINT64 xdfsTime;
	xdfsTime = time(NULL);
	int byteShift;
	byteShift = 12; /*4096是2的12次方*/
	UINT32 freeSpaceBlks;
	freeSpaceBlks = 32731;	/* 一共有37块被占用，文件系统一共有32768块，剩下32731块空闲 */ 
	struct xdfs_superblock *p_superblock;   /* in-core copy of super block */
    //HRFS_DISK_SUPER_BLOCK   *pdisksuperBlk; /* on-disk copy of super block */
	

	p_data = (char *)malloc(BLKSIZE * SUPERBLOCK_NBLKS);
	if (p_data == NULL)
    {
    	return (ERROR);
	}
	

	memset(p_data, 0xff, BLKSIZE * SUPERBLOCK_NBLKS);
	lseek(devfd, BLKSIZE * (NUM_BLK - 1), SEEK_SET);
	write(devfd, p_data, BLKSIZE * SUPERBLOCK_NBLKS);
	free(p_data);
	
	if ((p_superblock = (struct xdfs_superblock *)malloc(sizeof(struct xdfs_superblock))) == NULL)
        {
        return (ERROR);
        }
	memset(p_superblock, 0xff, sizeof(struct xdfs_superblock));
/*
    bzero((char *)psuperblock, sizeof(HRFS_INCORE_SUPER_BLOCK));
*/
printf("part 1 is OK\n");
 
    // memcpy(p_superblock->idString , XDFS_ID_STRING, sizeof(p_superblock->idString));
    p_superblock->s_state = 0;
	p_superblock->s_block = 32768-4096-8;
    p_superblock->s_inode = 4096;
    p_superblock->s_magic = 12;
    p_superblock->s_nbfree = (SB_BLKNO-DS_BLKNO+1);
    p_superblock->s_nifree = 4095;
    
printf("part 2 is OK\n");
    
  /*  if (pSuperBlkOut != NULL)
    {
    	hrfsSuperblockDiskCoreConvert(pSuperBlkOut, pDiskSuperBlk);
    }*/
	
	/* SuperBlock写入设备 */
	lseek(devfd, BLKSIZE * (NUM_BLK - 1), SEEK_SET);
	
printf("part 3 is OK\n");	
	
	write(devfd, p_superblock , sizeof(struct xdfs_superblock));
	
printf("part 4 is OK\n");	
	
	free(p_superblock);
	
printf("SUPERBLOCK END\n");
	
/*************************SUPERBLOCK_END*************************/
	printf("struct size:\n");
	printf("\tinode:\t%ld\n",sizeof(struct xdfs_inode));
	printf("\tsuperblock:\t%ld\n",sizeof(struct xdfs_superblock));
	return 0;	
} 
