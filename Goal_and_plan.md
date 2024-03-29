# 目标及规划

本项目旨在学习Linux 文件系统，参考ext2文件系统的实现方式去从0自己手动实现一个文件系统。本计划于2023-10-5开始，于2023-11-4结束(已经延期)

## 1. 目标

实现基本的文件系统应有的功能。  
必须实现：  
    1. 挂载与解挂  
    2. ls和cd  
    3. 创建和删除文件  
    4. 创建和删除文件夹  
    5. 编辑文件  
    6. 多线程问题  
可选实现  
    1. 硬链接和软连接  
    2. 对项目进行重构，使其结构更加清晰

## 2. 规划

实现此文件系统需要1个月时间进行完成，若未完成，则顺延至任务清单下一个，并将此时状态记录  

1-10：完成文件系统的挂载和解挂功能，能初始化磁盘，能说出技术选型的原因，能熟练运用ftrace进行内核追踪，能说出为什么这样设计可以完成这种功能。
完成挂载和解挂功能 OK
初始化磁盘 OK
技术选型原因 OK but few
熟练使用ftrace完成内核追踪 Not

11-20：完成文件系统的ls,cd创建和删除文件，文件夹的功能，将各种结构的功能，为什么这样设计的问题汇总成文档  
ls	Not
cd	Not
创建文件 Not
删除文件 Not
创建文件夹 Not
删除文件夹 Not

21-30：接续上一个10天的任务，完成文件系统的ls,cd创建和删除文件，文件夹的功能，将各种结构的功能，为什么这样设计的问题汇总成文档  
出差一趟；耗时4天
休息；耗时2天
任务中止：先准备找工作的部分内容，预计于11月5日继续
从11月5日至12月1日：完成了嵌入式工程设计中期，读了一些书，写了后端转发包和用户登录的功能，养了养病
12月1日继续


31-40：完成文件系统的编辑文件的问题，并尝试公开，将汇总的文档进行复盘，并开出系列文章，对此文件系统如何安装和使用进行说明。  

