#ifndef OS_INFO_H

#define OS_INFO_H

int get_Partition_Information(PartitionInfo** partition_list_ptr, short* partition_cnt);
void get_Partition_Usage(PartitionInfo* partitionBuf);

#endif
