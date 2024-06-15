#include <stdio.h>
#include <stdlib.h>

#include "common.h"

#define DEBUG	0

#define GET_POWER_OF_2(X)	(X == 0x00		? 0 : \
							X == 0x01		? 0 : \
							X == 0x02		? 1 : \
							X == 0x04		? 2 : \
							X == 0x08		? 3 : \
							X == 0x10		? 4 : \
							X == 0x20		? 5 : \
							X == 0x40		? 6 : \
							X == 0x80		? 7 : \
							X == 0x100		? 8 : \
							X == 0x200		? 9 : \
							X == 0x400		? 10 : \
							X == 0x800		? 11 : \
							X == 0x1000		? 12 : \
							X == 0x2000		? 13 : \
							X == 0x4000		? 14 : \
							X == 0x8000		? 15 : \
							X == 0x10000	? 16 : \
							X == 0x20000	? 17 : \
							X == 0x40000	? 18 : \
							X == 0x80000	? 19 : \
							X == 0x100000	? 20 : \
							X == 0x200000	? 21 : \
							X == 0x400000	? 22 : \
							X == 0x800000	? 23 : \
							X == 0x1000000	? 24 : \
							X == 0x2000000	? 25 : \
							X == 0x4000000	? 26 : \
							X == 0x8000000	? 27 : \
							X == 0x10000000	? 28 : \
							X == 0x20000000	? 29 : \
							X == 0x40000000	? 30 : \
							X == 0x80000000	? 31 : \
							X == 0x100000000	? 32 : 0)

/*
	直接映射Data Cache，16KB大小
	每行存放64个字节，共256行
*/
#define DCACHE_SIZE						16384   //C
#define DCACHE_DATA_PER_LINE			16		    //B							
#define DCACHE_LINE_PER_SET             32   //E,随便设了个两行一组
#define DCACHE_DATA_PER_LINE_ADDR_BITS	GET_POWER_OF_2(DCACHE_DATA_PER_LINE)	
#define DCACHE_SET						(DCACHE_SIZE/DCACHE_DATA_PER_LINE/DCACHE_LINE_PER_SET)//S
#define DCACHE_SET_ADDR_BITS			GET_POWER_OF_2(DCACHE_SET)		

#define VCACHE_SIZE                     4096
#define VCACHE_DATA_PER_LINE			16
#define VCACHE_LINE                     GET_POWER_OF_2(VCACHE_SIZE/VCACHE_DATA_PER_LINE)

#define ICACHE_SIZE 16384		// 16 KB
#define ICACHE_DATA_PER_LINE 16 // 每行的字节数
#define ICACHE_DATA_PER_LINE_ADDR_BITS GET_POWER_OF_2(ICACHE_DATA_PER_LINE)
#define ICACHE_LINE (ICACHE_SIZE / ICACHE_DATA_PER_LINE) // Cache的行数
#define ICACHE_LINE_ADDR_BITS GET_POWER_OF_2(ICACHE_LINE)

// Cache行的结构，包括Valid、Tag和Data。你所有的状态信息，只能记录在Cache行中！

struct DCACHE_LineStruct
{
	UINT8	Valid;//4位，1字节
	UINT64	Tag;//8字节
	UINT8	Data[DCACHE_DATA_PER_LINE];
}DCache[DCACHE_SET*DCACHE_LINE_PER_SET],VCache[VCACHE_LINE],ICache[ICACHE_LINE];
//DCache组的结构

/*
	DCache初始化代码，一般需要把DCache的有效位Valid设置为0
	模拟器启动时，会调用此InitDataCache函数
*/

void InitDataCache()
{
	UINT32 i,j,k,l;
	printf("[%s] +-----------------------------------+\n", __func__);
	printf("[%s] |   zly的Data Cache初始化ing.... |\n", __func__);
	printf("[%s] +-----------------------------------+\n", __func__);
    for (i = 0; i < DCACHE_SET*DCACHE_LINE_PER_SET; i++)
		DCache[i].Valid = 0;
    for(j=0;j<VCACHE_LINE;j++){
        VCache[j].Valid=0;
    }
    for(k=0;k<ICACHE_LINE;k++){
        ICache[k].Valid=0;
    }
    srand(time(NULL));
}

/*
	从Memory中读入一行数据到Data Cache中
*/
void LoadDataCacheLineFromMemory(UINT64 Address, UINT32 CacheLineAddress)
{
	// 一次性从Memory中将DCACHE_DATA_PER_LINE数据读入某个Data Cache行
	// 提供了一个函数，一次可以读入8个字节
	UINT32 i;
	UINT64 ReadData;
	UINT64 AlignAddress;
	UINT64* pp;

	AlignAddress = Address & ~(DCACHE_DATA_PER_LINE - 1);	// 地址必须对齐到DCACHE_DATA_PER_LINE (64)字节边界
	pp = (UINT64*)DCache[CacheLineAddress].Data;
	for (i = 0; i < DCACHE_DATA_PER_LINE / 8; i++)
	{
		ReadData = ReadMemory(AlignAddress + 8LL * i);
		if (DEBUG)
			printf("[%s] Address=%016llX ReadData=%016llX\n", __func__, AlignAddress + 8LL * i, ReadData);
		pp[i] = ReadData;
	}

}

/*
	将Data Cache中的一行数据，写入存储器
*/
void StoreDataCacheLineToMemory(UINT64 Address, UINT32 CacheLineAddress)
{
	// 一次性将DCACHE_DATA_PER_LINE数据从某个Data Cache行写入Memory中
	// 提供了一个函数，一次可以写入8个字节
	UINT32 i;
	UINT64 WriteData;
	UINT64 AlignAddress;
	UINT64* pp;

	AlignAddress = Address & ~(DCACHE_DATA_PER_LINE - 1);	// 地址必须对齐到DCACHE_DATA_PER_LINE (64)字节边界
	pp = (UINT64*)DCache[CacheLineAddress].Data;
	WriteData = 0;
	for (i = 0; i < DCACHE_DATA_PER_LINE / 8; i++)
	{
		WriteData = pp[i];
		WriteMemory(AlignAddress + 8LL * i, WriteData);
		if (DEBUG)
			printf("[%s] Address=%016llX ReadData=%016llX\n", __func__, AlignAddress + 8LL * i, WriteData);
	}
}
/*
    封装了一下，让后面代码看起来简洁点
*/
UINT64 ReadDataCache(UINT32 LineAddress, UINT8 DataSize, UINT8 BlockOffset)
{
	UINT64 ReadValue = 0;
	switch (DataSize)
	{
	case 1:
		ReadValue = DCache[LineAddress].Data[BlockOffset];
		break;
	case 2:
		BlockOffset = BlockOffset & 0xFE;
		ReadValue = DCache[LineAddress].Data[BlockOffset + 1];
		ReadValue = ReadValue << 8;
		ReadValue |= DCache[LineAddress].Data[BlockOffset + 0];
		break;
	case 4:
		BlockOffset = BlockOffset & 0xFC;
		ReadValue = DCache[LineAddress].Data[BlockOffset + 3];
		ReadValue = ReadValue << 8;
		ReadValue |= DCache[LineAddress].Data[BlockOffset + 2];
		ReadValue = ReadValue << 8;
		ReadValue |= DCache[LineAddress].Data[BlockOffset + 1];
		ReadValue = ReadValue << 8;
		ReadValue |= DCache[LineAddress].Data[BlockOffset + 0];
		break;
	case 8:
		BlockOffset = BlockOffset & 0xF8; // 需对齐到8字节边界
		ReadValue = DCache[LineAddress].Data[BlockOffset + 7];
		ReadValue = ReadValue << 8;
		ReadValue |= DCache[LineAddress].Data[BlockOffset + 6];
		ReadValue = ReadValue << 8;
		ReadValue |= DCache[LineAddress].Data[BlockOffset + 5];
		ReadValue = ReadValue << 8;
		ReadValue |= DCache[LineAddress].Data[BlockOffset + 4];
		ReadValue = ReadValue << 8;
		ReadValue |= DCache[LineAddress].Data[BlockOffset + 3];
		ReadValue = ReadValue << 8;
		ReadValue |= DCache[LineAddress].Data[BlockOffset + 2];
		ReadValue = ReadValue << 8;
		ReadValue |= DCache[LineAddress].Data[BlockOffset + 1];
		ReadValue = ReadValue << 8;
		ReadValue |= DCache[LineAddress].Data[BlockOffset + 0];
		break;
	}
	return ReadValue;
}
/*
    封装了一下，让后面代码看起来简洁点
*/
void WriteDataCache(UINT32 LineAddress, UINT8 DataSize, UINT8 BlockOffset, UINT64 StoreValue)
{
	switch (DataSize)
	{
	case 1: // 1个字节
		DCache[LineAddress].Data[BlockOffset + 0] = StoreValue & 0xFF;
		break;
	case 2:								  // 2个字节
		BlockOffset = BlockOffset & 0xFE; // 需对齐到2字节边界
		DCache[LineAddress].Data[BlockOffset + 0] = StoreValue & 0xFF;
		StoreValue = StoreValue >> 8;
		DCache[LineAddress].Data[BlockOffset + 1] = StoreValue & 0xFF;
		break;
	case 4:								  // 4个字节
		BlockOffset = BlockOffset & 0xFC; // 需对齐到4字节边界
		DCache[LineAddress].Data[BlockOffset + 0] = StoreValue & 0xFF;
		StoreValue = StoreValue >> 8;
		DCache[LineAddress].Data[BlockOffset + 1] = StoreValue & 0xFF;
		StoreValue = StoreValue >> 8;
		DCache[LineAddress].Data[BlockOffset + 2] = StoreValue & 0xFF;
		StoreValue = StoreValue >> 8;
		DCache[LineAddress].Data[BlockOffset + 3] = StoreValue & 0xFF;
		break;
	case 8:								  // 8个字节
		BlockOffset = BlockOffset & 0xF8; // 需对齐到8字节边界
		DCache[LineAddress].Data[BlockOffset + 0] = StoreValue & 0xFF;
		StoreValue = StoreValue >> 8;
		DCache[LineAddress].Data[BlockOffset + 1] = StoreValue & 0xFF;
		StoreValue = StoreValue >> 8;
		DCache[LineAddress].Data[BlockOffset + 2] = StoreValue & 0xFF;
		StoreValue = StoreValue >> 8;
		DCache[LineAddress].Data[BlockOffset + 3] = StoreValue & 0xFF;
		StoreValue = StoreValue >> 8;
		DCache[LineAddress].Data[BlockOffset + 4] = StoreValue & 0xFF;
		StoreValue = StoreValue >> 8;
		DCache[LineAddress].Data[BlockOffset + 5] = StoreValue & 0xFF;
		StoreValue = StoreValue >> 8;
		DCache[LineAddress].Data[BlockOffset + 6] = StoreValue & 0xFF;
		StoreValue = StoreValue >> 8;
		DCache[LineAddress].Data[BlockOffset + 7] = StoreValue & 0xFF;
		break;
	}
}


/*
    把DCache中淘汰的数据，放入VictimCache
*/
void StoreDataCacheLineToVictimCache(UINT32 DCacheLineAddress, UINT32 VCacheLineAddress)
{
    // 检查DCacheLineAddress是否在DCache的范围内
    if (DCacheLineAddress < DCACHE_SET * DCACHE_LINE_PER_SET)
    {
        // 检查VCacheLineAddress是否在VCache的范围内
        if (VCacheLineAddress < VCACHE_LINE)
        {
            // 复制DCache中的缓存行到VCache中
            VCache[VCacheLineAddress].Tag=DCache[DCacheLineAddress].Tag ;
            memcpy(VCache[VCacheLineAddress].Data, DCache[DCacheLineAddress].Data, DCACHE_DATA_PER_LINE);
        }
    }
}
/*
    把VictimCache里的数放入Dcache
*/
void LoadDataCacheLineFromVictimCache(UINT32 VCacheLineAddress, UINT32 DCacheLineAddress)
{
     if (DCacheLineAddress < DCACHE_SET * DCACHE_LINE_PER_SET)
    {
        // 检查VCacheLineAddress是否在VCache的范围内
        if (VCacheLineAddress < VCACHE_LINE)
        {

            DCache[DCacheLineAddress].Tag = VCache[VCacheLineAddress].Tag;
            memcpy( DCache[DCacheLineAddress].Data,VCache[VCacheLineAddress].Data, VCACHE_DATA_PER_LINE);
        }
    }

}

UINT32 GetNextVictim()
{
    UINT32 i;
	// 若Victim Cache中有空行，则返回空行的行号
	for(i = 0; i< VCACHE_SIZE; i++){
		if(VCache[i].Valid == 0){
			return i;
		}
	}
	// 若没有空行，则随机淘汰个victim行里的值(原来dcache中的值已经在写入victimcache时就进入内存了)
	UINT32 line=rand()%VCACHE_SIZE;
    return line;
}

/*
	Data Cache访问接口，系统模拟器会调用此接口，来实现对你的Data Cache访问
	Address:	访存字节地址
	Operation:	操作：读操作（'L'）、写操作（'S'）、读-修改-写操作（'M'）
	DataSize:	数据大小：1字节、2字节、4字节、8字节
	StoreValue:	当执行写操作的时候，需要写入的数据
	LoadResult:	当执行读操作的时候，从Cache读出的数据
*/
UINT8 AccessDataCache(UINT64 Address, UINT8 Operation, UINT8 DataSize, UINT64 StoreValue, UINT64* LoadResult)
{
	UINT32 CacheSetIndex;
    UINT32 CacheLineAddress;
	UINT8 BlockOffset;
	UINT64 AddressTag;
	UINT8 MissFlag = 'M';
	UINT64 ReadValue;
    UINT8 Found = 0;
    UINT32 LineOffset;
	*LoadResult = 0;

	/*
	*	直接映射中，Address被切分为  AddressTag，CacheLineAddress，BlockOffset
	*/

	
	CacheSetIndex = (Address >> DCACHE_DATA_PER_LINE_ADDR_BITS) % DCACHE_SET;
	BlockOffset = Address % DCACHE_DATA_PER_LINE;
	AddressTag = (Address >> DCACHE_DATA_PER_LINE_ADDR_BITS) >> DCACHE_SET_ADDR_BITS;	// 地址去掉DCACHE_SET、DCACHE_DATA_PER_LINE，剩下的作为Tag。警告！不能将整个地址作为Tag！！
    for(LineOffset=0;LineOffset<DCACHE_LINE_PER_SET;LineOffset++){
        CacheLineAddress=CacheSetIndex* DCACHE_LINE_PER_SET+LineOffset;
        if (DCache[CacheLineAddress].Valid == 1 && DCache[CacheLineAddress].Tag == AddressTag)
        {
            MissFlag = 'H';		// 命中！
            Found=1;
            if (Operation == 'L')	// 读操作
            {
                ReadValue=ReadDataCache(CacheLineAddress, DataSize, BlockOffset);
                *LoadResult = ReadValue;
                if (DEBUG)
                    printf("[%s] Address=%016llX Operation=%c DataSize=%u StoreValue=%016llX ReadValue=%016llX\n", __func__, Address, Operation, DataSize, StoreValue, ReadValue);
            }
            else if (Operation == 'S' || Operation == 'M')	// 写操作（修改操作在此等价于写操作）
            {
                if (DEBUG)
                printf("[%s] Hit S/M: Address=%016llX Operation=%c DataSize=%u StoreValue=%016llX\n", __func__, Address, Operation, DataSize, StoreValue);
                WriteDataCache(CacheLineAddress, DataSize, BlockOffset, StoreValue);
            }
        }
	}
    //L1没有，看看victim里有没有
    if(Found==0){
        UINT32 i=0;
        for(i=0;i<VCACHE_LINE;i++){
            if(VCache[i].Valid==1&& VCache[i].Tag==AddressTag)
            {
                if (DEBUG)
                    printf("[%s] Address=%016llX Operation=%c DataSize=%u StoreValue=%016llX\n", __func__, Address, Operation, DataSize, StoreValue);
                MissFlag = 'H';//在victimcache里找到了,也算进Hit
                Found=1;
                for(LineOffset=0;LineOffset<DCACHE_LINE_PER_SET;LineOffset++){
                    CacheLineAddress = CacheSetIndex * DCACHE_LINE_PER_SET + LineOffset;
                    if(DCache[CacheLineAddress].Valid == 0){
                        // 找到空行
                        break;
                    }
                }
                //没找到空行
                if (LineOffset == DCACHE_LINE_PER_SET)
                {
                    LineOffset=rand()%DCACHE_LINE_PER_SET;
                    CacheLineAddress=CacheSetIndex*DCACHE_LINE_PER_SET+LineOffset;
                    // 淘汰对应的Cache行，如果对应的Cache行有数据，需要写回到Memory中
                    // OldAddress = > (Tag,Set,0000)
                    Address = ((DCache[CacheLineAddress].Tag << DCACHE_SET_ADDR_BITS) << DCACHE_DATA_PER_LINE_ADDR_BITS) | ((UINT64)CacheSetIndex << DCACHE_DATA_PER_LINE_ADDR_BITS);	// 从Tag中恢复旧的地址
                    UINT64 NextVictimAddress=GetNextVictim();                 
                    StoreDataCacheLineToVictimCache(CacheLineAddress,NextVictimAddress);
                    StoreDataCacheLineToMemory(Address,CacheLineAddress);
                }
                
                // 需要从VictimCache中读入新的行
                LoadDataCacheLineFromVictimCache(i,CacheLineAddress);

                DCache[CacheLineAddress].Valid = 1;
                DCache[CacheLineAddress].Tag = AddressTag;
                if (Operation == 'L')	// 读操作
                {
                    ReadValue=ReadDataCache(CacheLineAddress, DataSize, BlockOffset);
                    *LoadResult = ReadValue;
                    if (DEBUG)
                        printf("[%s] Address=%016llX Operation=%c DataSize=%u StoreValue=%016llX ReadValue=%016llX\n", __func__, Address, Operation, DataSize, StoreValue, ReadValue);
                }
                else if (Operation == 'S' || Operation == 'M')	// 写操作（修改操作在此等价于写操作）
                {
                    if (DEBUG)
                    printf("[%s] Hit S/M: Address=%016llX Operation=%c DataSize=%u StoreValue=%016llX\n", __func__, Address, Operation, DataSize, StoreValue);
                    WriteDataCache(CacheLineAddress, DataSize, BlockOffset, StoreValue);
                    }
                }
            }
        }
    //都没有，从memory里找
    if(Found==0){
        		if (DEBUG)
			printf("[%s] Address=%016llX Operation=%c DataSize=%u StoreValue=%016llX\n", __func__, Address, Operation, DataSize, StoreValue);
		MissFlag = 'M';		// 不命中
		for(LineOffset=0;LineOffset<DCACHE_LINE_PER_SET;LineOffset++){
            CacheLineAddress = CacheSetIndex * DCACHE_LINE_PER_SET + LineOffset;
			if(DCache[CacheLineAddress].Valid == 0){
				// 找到空行
				break;
			}
        }
        if (LineOffset == DCACHE_LINE_PER_SET)
		{
            LineOffset=rand()%DCACHE_LINE_PER_SET;
            CacheLineAddress=CacheSetIndex*DCACHE_LINE_PER_SET+LineOffset;
			// 淘汰对应的Cache行，如果对应的Cache行有数据，需要写回到Memory中
			UINT64 OldAddress;
            UINT64 NextVictimAddress=GetNextVictim();                 
            StoreDataCacheLineToVictimCache(CacheLineAddress,NextVictimAddress);
			// OldAddress = > (Tag,Set,0000)
			OldAddress = ((DCache[CacheLineAddress].Tag << DCACHE_SET_ADDR_BITS) << DCACHE_DATA_PER_LINE_ADDR_BITS) | ((UINT64)CacheLineAddress << DCACHE_DATA_PER_LINE_ADDR_BITS);	// 从Tag中恢复旧的地址
			StoreDataCacheLineToMemory(OldAddress, CacheLineAddress);
		}
		// 需要从Memory中读入新的行（真实情况下，这个LoadCacheLineFromMemory需要很长时间的）
		LoadDataCacheLineFromMemory(Address, CacheLineAddress);
		DCache[CacheLineAddress].Valid = 1;
		DCache[CacheLineAddress].Tag = AddressTag;
		if (Operation == 'L')	// 读操作
		{
			// 读操作不需要做事情，因为已经MISS了
		}
		else if (Operation == 'S' || Operation == 'M')	// 写操作（修改操作在此等价于写操作）
        {
            if (DEBUG)
                printf("[%s] Hit S/M: Address=%016llX Operation=%c DataSize=%u StoreValue=%016llX\n", __func__, Address, Operation, DataSize, StoreValue);
                WriteDataCache(CacheLineAddress, DataSize, BlockOffset, StoreValue);
        }
    }
    
    
	return MissFlag;

}

/* 指令Cache实现部分，可选实现 */
void InitInstCache(void)
{
	UINT32 i;
	printf("[%s] +-----------------------------------+\n", __func__);
	printf("[%s] | zly 的 Instrction Cache 初始化ing..|\n", __func__);
	printf("[%s] +-----------------------------------+\n", __func__);
	for (i = 0; i < ICACHE_LINE; i++)
		ICache[i].Valid = 0;
}

void LoadInstCacheLineFromMemory(UINT64 Address, UINT32 CacheLineAddress)
{
	UINT32 i;
	UINT64 ReadData;
	UINT64 AlignAddress;
	UINT64* pp;

	AlignAddress = Address & ~(ICACHE_DATA_PER_LINE - 1); // 地址必须对齐到DCACHE_DATA_PER_LINE (64)字节边界
	pp = (UINT64*)ICache[CacheLineAddress].Data;
	for (i = 0; i < ICACHE_DATA_PER_LINE / 8; i++)
	{
		ReadData = ReadMemory(AlignAddress + 8LL * i);
		if (DEBUG)
			printf("[%s] Address=%016llX ReadData=%016llX\n", __func__, AlignAddress + 8LL * i, ReadData);
		pp[i] = ReadData;
	}

	return;
}

UINT64 ReadInstCache(UINT32 LineAddress, UINT8 DataSize, UINT8 BlockOffset)
{
	UINT64 ReadValue = 0;
	switch (DataSize)
	{
	case 1:
		ReadValue = ICache[LineAddress].Data[BlockOffset];
		break;
	case 2:
		BlockOffset = BlockOffset & 0xFE;
		ReadValue = ICache[LineAddress].Data[BlockOffset + 1];
		ReadValue = ReadValue << 8;
		ReadValue |= ICache[LineAddress].Data[BlockOffset + 0];
		break;
	case 4:
		BlockOffset = BlockOffset & 0xFC;
		ReadValue = ICache[LineAddress].Data[BlockOffset + 3];
		ReadValue = ReadValue << 8;
		ReadValue |= ICache[LineAddress].Data[BlockOffset + 2];
		ReadValue = ReadValue << 8;
		ReadValue |= ICache[LineAddress].Data[BlockOffset + 1];
		ReadValue = ReadValue << 8;
		ReadValue |= ICache[LineAddress].Data[BlockOffset + 0];
		break;
	case 8:
		BlockOffset = BlockOffset & 0xF8; // 需对齐到8字节边界
		ReadValue = ICache[LineAddress].Data[BlockOffset + 7];
		ReadValue = ReadValue << 8;
		ReadValue |= ICache[LineAddress].Data[BlockOffset + 6];
		ReadValue = ReadValue << 8;
		ReadValue |= ICache[LineAddress].Data[BlockOffset + 5];
		ReadValue = ReadValue << 8;
		ReadValue |= ICache[LineAddress].Data[BlockOffset + 4];
		ReadValue = ReadValue << 8;
		ReadValue |= ICache[LineAddress].Data[BlockOffset + 3];
		ReadValue = ReadValue << 8;
		ReadValue |= ICache[LineAddress].Data[BlockOffset + 2];
		ReadValue = ReadValue << 8;
		ReadValue |= ICache[LineAddress].Data[BlockOffset + 1];
		ReadValue = ReadValue << 8;
		ReadValue |= ICache[LineAddress].Data[BlockOffset + 0];
		break;
	}
	return ReadValue;
}

UINT8 AccessInstCache(UINT64 Address, UINT8 Operation, UINT8 InstSize, UINT64* InstResult)
{
	// 返回值'M' = Miss，'H'=Hit
	UINT32 CacheLineAddress;
	UINT8 BlockOffset;
	UINT64 AddressTag;
	UINT8 MissFlag = 'M';
	UINT64 ReadValue;

	*InstResult = 0;

	/*
	 *	直接映射中，Address被切分为  AddressTag，CacheLineAddress，BlockOffset
	 */
	CacheLineAddress = (Address >> ICACHE_DATA_PER_LINE_ADDR_BITS) % ICACHE_LINE;
	BlockOffset = Address % ICACHE_DATA_PER_LINE;
	AddressTag = (Address >> ICACHE_DATA_PER_LINE_ADDR_BITS) >> ICACHE_LINE_ADDR_BITS;

	if (ICache[CacheLineAddress].Valid == 1 && ICache[CacheLineAddress].Tag == AddressTag)
	{
		MissFlag = 'H'; // 命中！

		if (Operation == 'I')
		{ // 读指令
			ReadValue = ReadInstCache(CacheLineAddress, InstSize, BlockOffset);
			*InstResult = ReadValue;
		}
	}
	else
	{
		MissFlag = 'M'; // 不命中
		LoadInstCacheLineFromMemory(Address, CacheLineAddress);
		ICache[CacheLineAddress].Valid = 1;
		ICache[CacheLineAddress].Tag = AddressTag;
	}
	return MissFlag;
}
