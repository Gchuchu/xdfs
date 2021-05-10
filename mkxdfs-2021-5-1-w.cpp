
#include <stdio.h>
#include <stdint.h>  
#include <fcntl.h>
#include <iostream>
#include <time.h>
#include <string.h>

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


#define XDFS_BYTE_ORDER LITTLE_BYTE_ORDER	/* Ĭ��С���ֽ��� */ 

const int blksize = 4096;	/* һ����4KB */ 
const int nblks = 32768;	/* һ��32768���飬128MB */ 


/*************************PBR_DEFINE*************************/

#define PBR_JMP             0x00	/* x86 jump instruction (3 bytes) */
#define PBR_SYS_ID          0x03	/* system ID string     (8 bytes) */
#define PBR_BYTES_PER_BLK   0x0b	/* bytes per block      (2 bytes) */
#define PBR_NUM_BLKS        0x20	/* # of blocks on vol   (4 bytes) */
#define PBR_NBLKS 1	/*��PBR��С������͹��ˡ�*/ 


/*************************FSB_DEFINE*************************/


#define FSB_NBLKS 1	/* 4096 * 8 = 37268,������һ�����ÿ������ļ�ϵͳ��FSB */ 


/*************************FIB_DEFINE*************************/

#define FIB_NBLKS 1	/*��һ��4096��inode��1��͹��ˡ�*/


/*************************INODE_SET_DEFINE*************************/


#define INODE_SET_NBLKS 120	/* һ��inode���ݽṹ120�ֽڣ�������Ҫ120���� */
#define INODE_NUMS  4096	/* һ��4096��inode */

typedef struct _Vx_dlnode
    {
    struct _Vx_dlnode *next;
    struct _Vx_dlnode *previous;
    } _Vx_DL_NODE;
    
typedef _Vx_DL_NODE DL_NODE;
typedef INT64   xdfsTime_t;
typedef INT64	fsize_t;
typedef UINT32  fgen_t;
typedef UINT32  fid_t;
typedef UINT16  linkCount_t;
typedef UINT16  fmode_t;

typedef struct xdfs_incore_inode
    {
    xdfsTime_t  ctime;       /* The last time the file attributes */
                             /* (inode) were changed. */
    xdfsTime_t  mtime;       /* The last time the file data was changed */
    xdfsTime_t  atime;       /* The last time the file data was accessed */
    fsize_t     size;        /* The size of the file */
    fgen_t      generation;  /* # of times inode has been allocated */
    fid_t       uid;         /* UID of value of the owner of the file */
    fid_t       gid;         /* GID of value of the owner of the file */
    UINT32      nBlocks;     /* The number of blocks associated with inode */
    UINT32      indirect[4]; /* Direct & Indirect block references */
    linkCount_t linkCount;   /* Number of hard links to file */
    fmode_t     mode;        /* Mode bits for file */
    unsigned char      state;       /* State:  free, allocated, to be deleted */
    UINT32	version;     /* version format of on-disk inode */
    } XDFS_INCORE_INODE, * XDFS_INCORE_INODE_PTR;
    
typedef struct xdfs_alloc_pos
    {
    UINT32     lastAllocPhyBlkNum;    
    } XDFS_ALLOC_POS, * XDFS_ALLOC_POS_PTR;
    
typedef struct xdfs_inode_struct
    {
    DL_NODE             delNode;       
    XDFS_INCORE_INODE   iNodeData;
    UINT32              iNodeNumber;
    UINT32              dirSlot;        
    UINT32              dirInode;       
    UINT32              dirDelSlot;     
    UINT32              dirRenameSlot;  
    XDFS_ALLOC_POS      iAllocPos;      
    } XDFS_INODE, * XDFS_INODE_PTR;


/*************************INODE_LOG_DEFINE*************************/

#define INODE_LOG_NBLKS 1 /* INODE_LOG�Ĵ�Сǡ��Ϊ512�ֽڣ����ռ��1���� */

/*
typedef struct xdfs_inode_log_struct
    {
    UINT64          transNumber;                        
    UINT32          iNodeNumber[INODE_JOURNAL_ENTRIES];  
    char            reserved[28];                        
    HRFS_DISK_INODE iNodeData[INODE_JOURNAL_ENTRIES];    
    }; //_WRS_PACK_ALIGN(1) XDFS_INODE_LOG, * XDFS_INODE_LOG_PTR;
*/

/*************************METABITMAP_DEFINE*************************/

#define METABITMAP_NBLKS 1 /* FSB_METABITMAP��FIB_METABITMAPÿһ����ռ��1�� */


/*************************GLOBAL_INFO_DEFINE*************************/

#define GLOBAL_INFO_NBLKS 1	/* GLOBAL_INFO��������ÿһ����ռ��1���� */

/*
typedef struct xdfs_global_info        
    {
    UINT64    transNumber;      
    UINT64    timeStamp;       
    UINT32    numInodesToDelete;
    UINT32    numFreeInodes;
    UINT32    numFreeDataBlocks;
    UINT32    spare;
    } //_WRS_PACK_ALIGN(1) XDFS_TMR;
*/    
    
/*************************SUPERBLOCK_DEFINE*************************/

#define SUPERBLOCK_NBLKS 1	/* SUPERBLOCK��СΪ96�ֽڣ�ռ��1���� */
#define XDFS_ID_STRING "xdfs"	/*���ļ�ϵͳ��ID��*/

/*
typedef struct xdfs_disk_super_block_struct
    {
    char      idString[8];  
    INT64     ctime;        
    UINT8     majorVers;    
    UINT8     minorVers;    
    UINT16    blkSize2;     
    UINT32    totalBlks;   

    UINT32    reservedBlks;
                           

    UINT32    iNodeCount;   
                            

    UINT32    bgSize;      
    UINT32    dsSize;       
    UINT32    nBlkGroups;  

    UINT32    fsbOff[2];    
    UINT32    fibOff[2];    

    UINT32    itOff;       
    UINT32    ijOff;        

    UINT32    tmOff[2];    
    UINT32    tmrOff[2];    

    UINT32    dsOff;     
    UINT32    pad;         
    UINT32    crc;          
    };*/

/* �ں�̬�µ�SUPERBLOCK */
typedef struct xdfs_incore_super_block_struct
    {
    char      idString[8];  /* Identification or eyecatcher string */
    INT64     ctime;        /* time at which superblock was created. */
    UINT8     majorVers;    /* Major version number */
    UINT8     minorVers;    /* Minor version number */
    UINT16    blkSize2;     /* Block size as a power of 2. */
    UINT32    totalBlks;    /* Total # of blocks in fs (including offset). */

    UINT32    reservedBlks; /* Size of the reserved space at */
                            /* the start of the media. */

    UINT32    iNodeCount;   /* The number of iNodes this file */
                            /* system instantiation has. */

    UINT32    bgSize;       /* Block group size  (=dsSize + 1 for 1st iter) */
    UINT32    dsSize;       /* Data space size   (=bgSize - 1 for 1st iter) */
    UINT32    nBlkGroups;   /* Number of block groups. (1 for 1st iteration) */

    UINT32    fsbOff[2];    /* 1st & 2nd free space bitmap offsets */
    UINT32    fibOff[2];    /* 1st & 2nd free inode bitmap offsets */

    UINT32    itOff;        /* inode table offset */
    UINT32    ijOff;        /* inode journal offset */

    UINT32    tmOff[2];     /* 1st & 2nd transaction map offsets */
    UINT32    tmrOff[2];    /* 1st & 2nd transaction  master record offsets */

    UINT32    dsOff;        /* data space offset */
    UINT32    pad;          /* pad out structure */
    UINT32    crc;          /* superblock CRC */
    } XDFS_INCORE_SUPER_BLOCK, * XDFS_INCORE_SUPER_BLOCK_PTR;


/*************************DEFINE_END*************************/ 


int main(int argc, char *argv[])
{
	int error;
	 	 
	/* ������Ч�Լ�� */
/*	if(argc != 2)
	{
		fprintf(stderr, "xdfs: need to specify device\n");
		return (ERROR);
	} */
	
	/* ���豸 */
	int devfd;
	//devfd = open("\\dev\\loop0", O_WRONLY); 
	devfd = open("D:\\��������\\����ϵͳ��Ŀ\\����ϵͳ����\\2021.4.2\\test.txt", O_WRONLY);
	if(devfd < 0)
	{
		fprintf(stderr, "xdfs: failed to open device\n");
		return (ERROR);
	}
	
	/* �ļ�ϵͳ��С��� */
	error = lseek(devfd, (off_t)(nblks * blksize), SEEK_SET);
	if(error == -1)
	{
		fprintf(stderr, "xdfs: cannot create filesystem of specified size\n");
		return (ERROR);
	}
	lseek(devfd, 0, SEEK_SET); 
	
/*************************PBR_START*************************/

printf("PBR START\n");

	lseek(devfd, 0, SEEK_SET);
	char *p_pbr;
	p_pbr = (char *)malloc(blksize * PBR_NBLKS);
	if (p_pbr == NULL)
    	return (ERROR);
	memset(p_pbr, 0xff, blksize * PBR_NBLKS);	/* ��������0xff */ 
    
	/* PBR������� */
	p_pbr[PBR_JMP] = 0xe9;
	strcpy(p_pbr + PBR_SYS_ID, "xdfs"); 
	
	/* �ļ�ϵͳPBR����С���ֽ���д�� */
#if (LTFS_BYTE_ORDER == LITTLE_BYTE_ORDER)
    *((UINT32 *)(&p_pbr[PBR_NUM_BLKS])) = nblks;
#else
    p_pbr[PBR_NUM_BLKS]   = (nblks) & 0xFF;
    p_pbr[PBR_NUM_BLKS+1] = (nblks >> 8) & 0xFF;
    p_pbr[PBR_NUM_BLKS+2] = (nblks >> 16) & 0xFF;
    p_pbr[PBR_NUM_BLKS+3] = (nblks >> 24) & 0xFF;
#endif

	/* PBR_BYTES_PER_BLK��һ����������������ֽ�˳����Σ����Ƕ�����һ��д��һ���ֽ� */
	p_pbr[PBR_BYTES_PER_BLK]     = blksize & 0xFF;
    p_pbr[PBR_BYTES_PER_BLK + 1] = (blksize >> 8) & 0xFF;
	
	/* PBRд���豸 */
	write(devfd, p_pbr, blksize * PBR_NBLKS);
	 
	free(p_pbr); 
	
printf("PBR END\n");

/*************************PBR_END*************************/	


/*************************FSB0_START*************************/
	
printf("FSB0 START\n");
	
	lseek(devfd, blksize * PBR_NBLKS, SEEK_SET);
	char *p_fsb0;
	p_fsb0 = (char *)malloc(blksize * FSB_NBLKS);
	if (p_fsb0 == NULL)
    {
    	printf("p_fsb0 malloc failed!");
    	return (ERROR);
	}
	memset(p_fsb0, 0xff, blksize * FSB_NBLKS);	/*��������0xff����ʾ��δʹ��*/ 
	
	/* ��1λ������ʾ����PBR */
	p_fsb0[0] = p_fsb0[0] & 0b11111110 ;
	/* ��2λ������ʾ����FSB0 */
	p_fsb0[0] = p_fsb0[0] & 0b11111101;
	/* ��3λ������ʾ����FIB0 */ 
	p_fsb0[0] = p_fsb0[0] & 0b11111011;
	/* ��4λ������ʾ����FSB1 */ 
	p_fsb0[0] = p_fsb0[0] & 0b11110111;
	/* ��5λ������ʾ����FIB1 */ 
	p_fsb0[0] = p_fsb0[0] & 0b11101111;

printf("part 1 is OK\n");
	
	/* ��������120λ��ʾ����INODE_SET */
	p_fsb0[0] = p_fsb0[0] & 0b00011111;
	for(int i = 0; i < int((120 - 3) / 8); i++)
		p_fsb0[i + 1] = p_fsb0[i + 1] & 0b00000000;
	p_fsb0[15] = p_fsb0[15] & 0b11111000;

printf("part 2 is OK\n");
	
	/* ��126λ������ʾ����INODE_LOG */ 
	p_fsb0[15] = p_fsb0[15] & 0b11110111;	
	
	/* ��127��128��129��130λ������ʾ����FSB_METABITMAP��FIB_METABITMAP */ 
	p_fsb0[15] = p_fsb0[15] & 0b00001111;
	
	/* ��131λ������ʾ����GLOBAL_INFO */
	p_fsb0[16] = p_fsb0[16] & 0b11111110;
		
	/* FSB0�����1λ������ʾ����SUPERBLOCK */
	p_fsb0[blksize - 1] = p_fsb0[blksize - 1] & 0b01111111;	

printf("part 3 is OK\n");
		 
	/* FSB0д���豸 */
	write(devfd, p_fsb0, blksize * FSB_NBLKS);
	
	free(p_fsb0); 

printf("FSB0 END\n");	

/*************************FSB0_END*************************/


/*************************FIB0_START*************************/

printf("FIB0 START\n");	

	lseek(devfd, blksize * PBR_NBLKS + blksize * FSB_NBLKS, SEEK_SET);
	char *p_fib0;
	p_fib0 = (char *)malloc(blksize * FIB_NBLKS);
	if (p_fib0 == NULL)
    	return (ERROR);
	memset(p_fib0, 0xff, blksize * FIB_NBLKS);	/*��������0xff����ʾ��δʹ��*/ 
	
	/* FIB0д���豸 */
	write(devfd, p_fib0, blksize * FIB_NBLKS);
	
	free(p_fib0);

printf("FIB0 END\n");

/*************************FIB0_END*************************/


/*************************FSB1_START*************************/

printf("FSB1 START\n");
	
	lseek(devfd, blksize * PBR_NBLKS + blksize * FSB_NBLKS + blksize * FIB_NBLKS, SEEK_SET);
	char *p_fsb1;
	p_fsb1 = (char *)malloc(blksize * FSB_NBLKS);
	if (p_fsb1 == NULL)
    	return (ERROR);
	memset(p_fsb1, 0xff, blksize * FSB_NBLKS);	/*��������0xff����ʾ��δʹ��*/ 
	
	/* ��1λ������ʾ����PBR */
	p_fsb1[0] = p_fsb1[0] & 0b11111110 ;
	/* ��2λ������ʾ����FSB0 */
	p_fsb1[0] = p_fsb1[0] & 0b11111101;
	/* ��3λ������ʾ����FIB0 */ 
	p_fsb1[0] = p_fsb1[0] & 0b11111011;
	/* ��4λ������ʾ����FSB1 */ 
	p_fsb1[0] = p_fsb1[0] & 0b11110111;
	/* ��5λ������ʾ����FIB1 */ 
	p_fsb1[0] = p_fsb1[0] & 0b11101111;

printf("part 1 is OK\n");
	
	/* ��������120λ��ʾ����INODE_SET */
	p_fsb1[0] = p_fsb1[0] & 0b00011111;
	for(int i = 0; i < ((120 - 3) / 8); i++)
		p_fsb1[i + 1] = p_fsb1[i + 1] & 0b00000000;
	p_fsb1[15] = p_fsb1[15] & 0b11111000;

printf("part 2 is OK\n");
	
	/* ��126λ������ʾ����INODE_LOG */ 
	p_fsb1[15] = p_fsb1[15] & 0b11110111;	
	
	/* ��127��128��129��130λ������ʾ����FSB_METABITMAP��FIB_METABITMAP */ 
	p_fsb1[15] = p_fsb1[15] & 0b00001111;
	
	/* ��131λ������ʾ����GLOBAL_INFO */
	p_fsb1[16] = p_fsb1[16] & 0b11111110;
		
	/* FSB1�����1λ������ʾ����SUPERBLOCK */
	p_fsb1[blksize - 1] = p_fsb1[blksize - 1] & 0b01111111;		 

printf("part 3 is OK\n");
	
	/* FSB1д���豸 */
	write(devfd, p_fsb1, blksize * FSB_NBLKS);
	
	free(p_fsb1);

printf("FSB1 END\n");
	 
/*************************FSB1_END*************************/


/*************************FIB1_START*************************/

printf("FIB1 START\n");
	
	lseek(devfd, blksize * PBR_NBLKS + blksize * FSB_NBLKS * 2 + blksize * FIB_NBLKS, SEEK_SET);
	char *p_fib1;
	p_fib1 = (char *)malloc(blksize * FIB_NBLKS);
	if (p_fib1 == NULL)
    	return (ERROR);
	memset(p_fib1, 0xff, blksize * FIB_NBLKS);	/*��������0xff����ʾ��δʹ��*/ 
	
	/* FIB1д���豸 */
	write(devfd, p_fib1, blksize * FIB_NBLKS);
	
	free(p_fib1);

printf("FIB1 END\n");
	
/*************************FIB1_END*************************/
	

/*************************INODE_SET_START*************************/
	
printf("INODE_SET START\n");
	
	lseek(devfd, blksize * PBR_NBLKS + blksize * FSB_NBLKS * 2 + blksize * FIB_NBLKS * 2, SEEK_SET);
	char *p_inode_set;
	p_inode_set = (char *)malloc(1 * sizeof(XDFS_INODE));
	if (p_inode_set == NULL)
    	return (ERROR);
	memset(p_inode_set, 0xff, 1 * sizeof(XDFS_INODE));	/*��������0xff����ʾ��δʹ��*/ 

printf("part 1 is OK\n");

	for(int i = 0; i < INODE_NUMS; i++)
	{
		write(devfd, p_inode_set, 1 * sizeof(XDFS_INODE));
	}
	
printf("part 2 is OK\n");
	
	free(p_inode_set);

printf("INODE_SET END\n");
	
/*************************INODE_SET_END*************************/


/*************************INODE_LOG_START*************************/

printf("INODE_LOG START\n");

	lseek(devfd, blksize * PBR_NBLKS + blksize * FSB_NBLKS * 2 + blksize * FIB_NBLKS * 2 \
+ blksize * INODE_SET_NBLKS, SEEK_SET);
	char *p_inode_log;
	p_inode_log = (char *)malloc(blksize * INODE_LOG_NBLKS);
	if (p_inode_log == NULL)
    	return (ERROR);
	memset(p_inode_log, 0xff, blksize * INODE_LOG_NBLKS);	/*��������0xff*/ 
	write(devfd, p_inode_log, blksize * INODE_LOG_NBLKS);

printf("part 1 is OK\n");

	free(p_inode_log);

printf("INODE_LOG END\n");
	
/*************************INODE_LOG_END*************************/


/*************************FSB_METABITMAP0_START*************************///Metabitmap 

printf("FSB_METABITMAP0 START\n");

	lseek(devfd, blksize * PBR_NBLKS + blksize * FSB_NBLKS * 2 + blksize * FIB_NBLKS * 2 \
+ blksize * INODE_SET_NBLKS + blksize * INODE_LOG_NBLKS, SEEK_SET);
	char *p_fsb_metabitmap0;
	p_fsb_metabitmap0 = (char *)malloc(blksize * METABITMAP_NBLKS);
	if (p_fsb_metabitmap0 == NULL)
    	return (ERROR);
	memset(p_fsb_metabitmap0, 0x00, blksize * METABITMAP_NBLKS);	/*��������0x00*/ 
	write(devfd, p_fsb_metabitmap0, blksize * METABITMAP_NBLKS);
	free(p_fsb_metabitmap0);

printf("FSB_METABITMAP0 END\n");
	
/*************************FSB_METABITMAP0_END*************************/


/*************************FSB_METABITMAP1_START*************************/

printf("FSB_METABITMAP1 START\n");

	lseek(devfd, blksize * PBR_NBLKS + blksize * FSB_NBLKS * 2 + blksize * FIB_NBLKS * 2 \
+ blksize * INODE_SET_NBLKS + blksize * INODE_LOG_NBLKS + blksize * METABITMAP_NBLKS, SEEK_SET);
	char *p_fsb_metabitmap1;
	p_fsb_metabitmap1 = (char *)malloc(blksize * METABITMAP_NBLKS);
	if (p_fsb_metabitmap1 == NULL)
    	return (ERROR);
	memset(p_fsb_metabitmap1, 0x00, blksize * METABITMAP_NBLKS);	/*��������0x00*/ 
	write(devfd, p_fsb_metabitmap1, blksize * METABITMAP_NBLKS);
	free(p_fsb_metabitmap1);

printf("FSB_METABITMAP1 END\n");

/*************************FSB_METABITMAP1_END*************************/


/*************************FIB_METABITMAP0_START*************************/

printf("FIB_METABITMAP0 START\n");

	lseek(devfd, blksize * PBR_NBLKS + blksize * FSB_NBLKS * 2 + blksize * FIB_NBLKS * 2 \
+ blksize * INODE_SET_NBLKS + blksize * INODE_LOG_NBLKS + blksize * METABITMAP_NBLKS * 2, SEEK_SET);
	char *p_fib_metabitmap0;
	p_fib_metabitmap0 = (char *)malloc(blksize * METABITMAP_NBLKS);
	if (p_fib_metabitmap0 == NULL)
    	return (ERROR);
	memset(p_fib_metabitmap0, 0x00, blksize * METABITMAP_NBLKS);	/*��������0x00*/ 
	write(devfd, p_fib_metabitmap0, blksize * METABITMAP_NBLKS);
	free(p_fib_metabitmap0);

printf("FIB_METABITMAP0 END\n");

/*************************FIB_METABITMAP0_END*************************/


/*************************FIB_METABITMAP1_START*************************/

printf("FIB_METABITMAP1 START\n");

	lseek(devfd, blksize * PBR_NBLKS + blksize * FSB_NBLKS * 2 + blksize * FIB_NBLKS * 2 \
+ blksize * INODE_SET_NBLKS + blksize * INODE_LOG_NBLKS + blksize * METABITMAP_NBLKS * 3, SEEK_SET);
	char *p_fib_metabitmap1;
	p_fib_metabitmap1 = (char *)malloc(blksize * METABITMAP_NBLKS);
	if (p_fib_metabitmap1 == NULL)
    	return (ERROR);
	memset(p_fib_metabitmap1, 0x00, blksize * METABITMAP_NBLKS);	/*��������0x00*/ 
	write(devfd, p_fib_metabitmap1, blksize * METABITMAP_NBLKS);
	free(p_fib_metabitmap1);

printf("FIB_METABITMAP0 END\n");

/*************************FIB_METABITMAP1_END*************************/


/*************************GLOBAL_INFO0_START*************************///globalinf 

printf("GLOBAL_INFO0 START\n");

	lseek(devfd, blksize * PBR_NBLKS + blksize * FSB_NBLKS * 2 + blksize * FIB_NBLKS * 2 \
+ blksize * INODE_SET_NBLKS + blksize * INODE_LOG_NBLKS + blksize * METABITMAP_NBLKS * 4, SEEK_SET);
	char *p_global_info0;
	p_global_info0 = (char *)malloc(blksize * GLOBAL_INFO_NBLKS);
	if (p_global_info0 == NULL)
    	return (ERROR);
	memset(p_global_info0, 0x00, blksize * GLOBAL_INFO_NBLKS);	/*��������0x00*/
	write(devfd, p_global_info0, blksize * GLOBAL_INFO_NBLKS);
	free(p_global_info0);

printf("GLOBAL_INFO0 END\n");

/*************************GLOBAL_INFO0_SEND*************************/


/*************************GLOBAL_INFO1_START*************************/

printf("GLOBAL_INFO1 START\n");

	lseek(devfd, blksize * PBR_NBLKS + blksize * FSB_NBLKS * 2 + blksize * FIB_NBLKS * 2 \
+ blksize * INODE_SET_NBLKS + blksize * INODE_LOG_NBLKS + blksize * METABITMAP_NBLKS * 4 + blksize * GLOBAL_INFO_NBLKS, SEEK_SET);
	char *p_global_info1;
	p_global_info1 = (char *)malloc(blksize * GLOBAL_INFO_NBLKS);
	if (p_global_info1 == NULL)
    	return (ERROR);
	memset(p_global_info1, 0x00, blksize * GLOBAL_INFO_NBLKS);	/*��������0x00*/ 
	write(devfd, p_global_info1, blksize * GLOBAL_INFO_NBLKS);
	free(p_global_info1);

printf("GLOBAL_INFO1 END\n");

/*************************GLOBAL_INFO1_END*************************/


/*************************SUPERBLOCK_START*************************/

printf("SUPERBLOCK START\n");

	char * p_data;
	UINT32 subTotal;
	UINT64 xdfsTime;
	xdfsTime = time(NULL);
	int byteShift;
	byteShift = 12; /*4096��2��12�η�*/
	UINT32 freeSpaceBlks;
	freeSpaceBlks = 32636;	/* һ����132�鱻ռ�ã��ļ�ϵͳһ����32768�飬ʣ��32636����� */ 
	XDFS_INCORE_SUPER_BLOCK *p_superblock;   /* in-core copy of super block */
    //HRFS_DISK_SUPER_BLOCK   *pdisksuperBlk; /* on-disk copy of super block */
	

	p_data = (char *)malloc(blksize * SUPERBLOCK_NBLKS);
	if (p_data == NULL)
    {
    	return (ERROR);
	}
//	p_disksuperblk = (HRFS_DISK_SUPER_BLOCK *)p_data;

	memset(p_data, 0xff, blksize * SUPERBLOCK_NBLKS);
	lseek(devfd, blksize * (nblks - 1), SEEK_SET);
	write(devfd, p_data, blksize * SUPERBLOCK_NBLKS);
	free(p_data);
	
	if ((p_superblock = (XDFS_INCORE_SUPER_BLOCK *)malloc(sizeof(XDFS_INCORE_SUPER_BLOCK))) == NULL)
        {
        return (ERROR);
        }
	memset(p_superblock, 0xff, sizeof(XDFS_INCORE_SUPER_BLOCK));
/*
    bzero((char *)psuperblock, sizeof(HRFS_INCORE_SUPER_BLOCK));
*/

printf("part 1 is OK\n");
 
    memcpy(p_superblock->idString , XDFS_ID_STRING, sizeof(p_superblock->idString));
    p_superblock->ctime = xdfsTime;
    p_superblock->majorVers = 0;	/*ȱʡ��Ϊ0*/
	p_superblock->minorVers = 0;	/*ȱʡ��Ϊ0*/
    p_superblock->blkSize2 = byteShift;
    p_superblock->totalBlks = nblks;
    p_superblock->reservedBlks = 1;
    p_superblock->iNodeCount = INODE_NUMS;
    p_superblock->bgSize = freeSpaceBlks;
    p_superblock->dsSize = freeSpaceBlks;
    p_superblock->nBlkGroups = 1;

printf("part 2 is OK\n");

    subTotal = 1;
    p_superblock->fsbOff[0] = subTotal;
    subTotal += FSB_NBLKS;
    p_superblock->fsbOff[1] = subTotal;

    subTotal += FSB_NBLKS;
    p_superblock->fibOff[0] = subTotal;
    subTotal += FIB_NBLKS;
    p_superblock->fibOff[1] = subTotal;

    subTotal += FIB_NBLKS;
    p_superblock->itOff = subTotal;
    subTotal += INODE_SET_NBLKS;
    p_superblock->ijOff = subTotal;

    subTotal ++;
    p_superblock->tmOff[0] = subTotal;
    subTotal += 2;
    p_superblock->tmOff[1] = subTotal;

    subTotal += 2;
    p_superblock->tmrOff[0] = subTotal;
    subTotal ++;
    p_superblock->tmrOff[1] = subTotal;

    subTotal ++;
    p_superblock->dsOff = subTotal;
    
printf("part 3 is OK\n");
    
  /*  if (pSuperBlkOut != NULL)
    {
    	hrfsSuperblockDiskCoreConvert(pSuperBlkOut, pDiskSuperBlk);
    }*/
	
	/* SuperBlockд���豸 */
	lseek(devfd, blksize * (nblks - 1), SEEK_SET);
	
printf("part 4 is OK\n");	
	
	write(devfd, p_superblock , sizeof(XDFS_INCORE_SUPER_BLOCK));
	
printf("part 5 is OK\n");	
	
	free(p_superblock);
	
printf("SUPERBLOCK END\n");
	
/*************************SUPERBLOCK_END*************************/

	return 0;	
} 
