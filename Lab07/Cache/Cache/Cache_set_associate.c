///////////////////////////////////////////////////////////////////////
////  Copyright 2022 by mars.                                        //
///////////////////////////////////////////////////////////////////////

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
	ֱ��ӳ��Data Cache��16KB��С
	ÿ�д��64���ֽڣ���256��
*/
#define DCACHE_SIZE						16384   //C
#define DCACHE_DATA_PER_LINE			16		    //B							
#define DCACHE_LINE_PER_SET             2   //E,������˸�����һ��
#define DCACHE_LINE 					(DCACHE_SIZE/DCACHE_DATA_PER_LINE)
#define DCACHE_DATA_PER_LINE_ADDR_BITS	GET_POWER_OF_2(DCACHE_DATA_PER_LINE)	
#define DCACHE_SET						(DCACHE_SIZE/DCACHE_DATA_PER_LINE/DCACHE_LINE_PER_SET)//S
#define DCACHE_SET_ADDR_BITS			GET_POWER_OF_2(DCACHE_SET)		

// Cache�еĽṹ������Valid��Tag��Data�������е�״̬��Ϣ��ֻ�ܼ�¼��Cache���У�
struct DCACHE_LineStruct
{
	UINT8	Valid;//4λ��1�ֽ�
	UINT64	Tag;//8�ֽ�
	UINT8	Data[DCACHE_DATA_PER_LINE];
}DCache[DCACHE_LINE];
//DCache��Ľṹ

/*
	DCache��ʼ�����룬һ����Ҫ��DCache����ЧλValid����Ϊ0
	ģ��������ʱ������ô�InitDataCache����
*/
void InitDataCache()
{
	UINT32 i;
	printf("[%s] +-----------------------------------+\n", __func__);
	printf("[%s] |   zly��Data Cache��ʼ��ing.... |\n", __func__);
	printf("[%s] +-----------------------------------+\n", __func__);

    for (i = 0; i < DCACHE_SET*DCACHE_LINE_PER_SET; i++)
		DCache[i].Valid = 0;
    srand(time(NULL));
}

/*
	��Memory�ж���һ�����ݵ�Data Cache��
*/
void LoadDataCacheLineFromMemory(UINT64 Address, UINT32 CacheLineAddress)
{
	// һ���Դ�Memory�н�DCACHE_DATA_PER_LINE���ݶ���ĳ��Data Cache��
	// �ṩ��һ��������һ�ο��Զ���8���ֽ�
	UINT32 i;
	UINT64 ReadData;
	UINT64 AlignAddress;
	UINT64* pp;

	AlignAddress = Address & ~(DCACHE_DATA_PER_LINE - 1);	// ��ַ������뵽DCACHE_DATA_PER_LINE (64)�ֽڱ߽�
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
	��Data Cache�е�һ�����ݣ�д��洢��
*/
void StoreDataCacheLineToMemory(UINT64 Address, UINT32 CacheLineAddress)
{
	// һ���Խ�DCACHE_DATA_PER_LINE���ݴ�ĳ��Data Cache��д��Memory��
	// �ṩ��һ��������һ�ο���д��8���ֽ�
	UINT32 i;
	UINT64 WriteData;
	UINT64 AlignAddress;
	UINT64* pp;

	AlignAddress = Address & ~(DCACHE_DATA_PER_LINE - 1);	// ��ַ������뵽DCACHE_DATA_PER_LINE (64)�ֽڱ߽�
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
	Data Cache���ʽӿڣ�ϵͳģ��������ô˽ӿڣ���ʵ�ֶ����Data Cache����
	Address:	�ô��ֽڵ�ַ
	Operation:	��������������'L'����д������'S'������-�޸�-д������'M'��
	DataSize:	���ݴ�С��1�ֽڡ�2�ֽڡ�4�ֽڡ�8�ֽ�
	StoreValue:	��ִ��д������ʱ����Ҫд�������
	LoadResult:	��ִ�ж�������ʱ�򣬴�Cache����������
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
	*	ֱ��ӳ���У�Address���з�Ϊ  AddressTag��CacheLineAddress��BlockOffset
	*/

	
	CacheSetIndex = (Address >> DCACHE_DATA_PER_LINE_ADDR_BITS) % DCACHE_SET;
	BlockOffset = Address % DCACHE_DATA_PER_LINE;
	AddressTag = (Address >> DCACHE_DATA_PER_LINE_ADDR_BITS) >> DCACHE_SET_ADDR_BITS;	// ��ַȥ��DCACHE_SET��DCACHE_DATA_PER_LINE��ʣ�µ���ΪTag�����棡���ܽ�������ַ��ΪTag����
    for(LineOffset=0;LineOffset<DCACHE_LINE_PER_SET;LineOffset++){
        CacheLineAddress=CacheSetIndex* DCACHE_LINE_PER_SET+LineOffset;
        if (DCache[CacheLineAddress].Valid == 1 && DCache[CacheLineAddress].Tag == AddressTag)
        {
            MissFlag = 'H';		// ���У�
            Found=1;
           if (Operation == 'L')	// ������
		{
			ReadValue = 0;
			switch (DataSize)
			{
			case 1:	// 1���ֽ�
				ReadValue = DCache[CacheLineAddress].Data[BlockOffset + 0];
				break;
			case 2:	// 2���ֽ�
				BlockOffset = BlockOffset & 0xFE;	// ����뵽2�ֽڱ߽�
				ReadValue = DCache[CacheLineAddress].Data[BlockOffset + 1]; ReadValue = ReadValue << 8;
				ReadValue |= DCache[CacheLineAddress].Data[BlockOffset + 0];
				break;
			case 4:	// 4���ֽ�
				BlockOffset = BlockOffset & 0xFC;	// ����뵽4�ֽڱ߽�
				ReadValue = DCache[CacheLineAddress].Data[BlockOffset + 3]; ReadValue = ReadValue << 8;
				ReadValue |= DCache[CacheLineAddress].Data[BlockOffset + 2]; ReadValue = ReadValue << 8;
				ReadValue |= DCache[CacheLineAddress].Data[BlockOffset + 1]; ReadValue = ReadValue << 8;
				ReadValue |= DCache[CacheLineAddress].Data[BlockOffset + 0];
				break;
			case 8:	// 8���ֽ�
				BlockOffset = BlockOffset & 0xF8;	// ����뵽8�ֽڱ߽�
				ReadValue = DCache[CacheLineAddress].Data[BlockOffset + 7]; ReadValue = ReadValue << 8;
				ReadValue |= DCache[CacheLineAddress].Data[BlockOffset + 6]; ReadValue = ReadValue << 8;
				ReadValue |= DCache[CacheLineAddress].Data[BlockOffset + 5]; ReadValue = ReadValue << 8;
				ReadValue |= DCache[CacheLineAddress].Data[BlockOffset + 4]; ReadValue = ReadValue << 8;
				ReadValue |= DCache[CacheLineAddress].Data[BlockOffset + 3]; ReadValue = ReadValue << 8;
				ReadValue |= DCache[CacheLineAddress].Data[BlockOffset + 2]; ReadValue = ReadValue << 8;
				ReadValue |= DCache[CacheLineAddress].Data[BlockOffset + 1]; ReadValue = ReadValue << 8;
				ReadValue |= DCache[CacheLineAddress].Data[BlockOffset + 0];
				break;
			}
			*LoadResult = ReadValue;
			if (DEBUG)
				printf("[%s] Address=%016llX Operation=%c DataSize=%u StoreValue=%016llX ReadValue=%016llX\n", __func__, Address, Operation, DataSize, StoreValue, ReadValue);
		}
		else if (Operation == 'S' || Operation == 'M')	// д�������޸Ĳ����ڴ˵ȼ���д������
		{
			if (DEBUG)
				printf("[%s] Address=%016llX Operation=%c DataSize=%u StoreValue=%016llX\n", __func__, Address, Operation, DataSize, StoreValue);
			switch (DataSize)
			{
			case 1:	// 1���ֽ�
				DCache[CacheLineAddress].Data[BlockOffset + 0] = StoreValue & 0xFF;
				break;
			case 2:	// 2���ֽ�
				BlockOffset = BlockOffset & 0xFE;	// ����뵽2�ֽڱ߽�
				DCache[CacheLineAddress].Data[BlockOffset + 0] = StoreValue & 0xFF; StoreValue = StoreValue >> 8;
				DCache[CacheLineAddress].Data[BlockOffset + 1] = StoreValue & 0xFF;
				break;
			case 4:	// 4���ֽ�
				BlockOffset = BlockOffset & 0xFC;	// ����뵽4�ֽڱ߽�
				DCache[CacheLineAddress].Data[BlockOffset + 0] = StoreValue & 0xFF; StoreValue = StoreValue >> 8;
				DCache[CacheLineAddress].Data[BlockOffset + 1] = StoreValue & 0xFF; StoreValue = StoreValue >> 8;
				DCache[CacheLineAddress].Data[BlockOffset + 2] = StoreValue & 0xFF; StoreValue = StoreValue >> 8;
				DCache[CacheLineAddress].Data[BlockOffset + 3] = StoreValue & 0xFF;
				break;
			case 8:	// 8���ֽ�
				BlockOffset = BlockOffset & 0xF8;	// ����뵽8�ֽڱ߽�
				DCache[CacheLineAddress].Data[BlockOffset + 0] = StoreValue & 0xFF; StoreValue = StoreValue >> 8;
				DCache[CacheLineAddress].Data[BlockOffset + 1] = StoreValue & 0xFF; StoreValue = StoreValue >> 8;
				DCache[CacheLineAddress].Data[BlockOffset + 2] = StoreValue & 0xFF; StoreValue = StoreValue >> 8;
				DCache[CacheLineAddress].Data[BlockOffset + 3] = StoreValue & 0xFF; StoreValue = StoreValue >> 8;
				DCache[CacheLineAddress].Data[BlockOffset + 4] = StoreValue & 0xFF; StoreValue = StoreValue >> 8;
				DCache[CacheLineAddress].Data[BlockOffset + 5] = StoreValue & 0xFF; StoreValue = StoreValue >> 8;
				DCache[CacheLineAddress].Data[BlockOffset + 6] = StoreValue & 0xFF; StoreValue = StoreValue >> 8;
				DCache[CacheLineAddress].Data[BlockOffset + 7] = StoreValue & 0xFF;
				break;
			}
		}
	}
    }
	if(Found==0)
	{
		if (DEBUG)
			printf("[%s] Address=%016llX Operation=%c DataSize=%u StoreValue=%016llX\n", __func__, Address, Operation, DataSize, StoreValue);
		MissFlag = 'M';		// ������
		for(LineOffset=0;LineOffset<DCACHE_LINE_PER_SET;LineOffset++){
            CacheLineAddress = CacheSetIndex * DCACHE_LINE_PER_SET + LineOffset;
			if(DCache[CacheLineAddress].Valid == 0){
				// �ҵ�����
				break;
			}
        }
        if (LineOffset == DCACHE_LINE_PER_SET)
		{
            LineOffset=rand()%DCACHE_LINE_PER_SET;
            CacheLineAddress=CacheSetIndex*DCACHE_LINE_PER_SET+LineOffset;
			// ��̭��Ӧ��Cache�У������Ӧ��Cache�������ݣ���Ҫд�ص�Memory��
			UINT64 OldAddress;
			// OldAddress = > (Tag,Set,0000)
			OldAddress = ((DCache[CacheLineAddress].Tag << DCACHE_SET_ADDR_BITS) << DCACHE_DATA_PER_LINE_ADDR_BITS) | ((UINT64)CacheSetIndex << DCACHE_DATA_PER_LINE_ADDR_BITS);	// ��Tag�лָ��ɵĵ�ַ
			StoreDataCacheLineToMemory(OldAddress, CacheLineAddress);
		}
		// ��Ҫ��Memory�ж����µ��У���ʵ����£����LoadCacheLineFromMemory��Ҫ�ܳ�ʱ��ģ�
		LoadDataCacheLineFromMemory(Address, CacheLineAddress);
		DCache[CacheLineAddress].Valid = 1;
		DCache[CacheLineAddress].Tag = AddressTag;
		if (Operation == 'L')	// ������
		{
			// ����������Ҫ�����飬��Ϊ�Ѿ�MISS��
		}
		else if (Operation == 'S' || Operation == 'M')	// д�������޸Ĳ����ڴ˵ȼ���д������
		{
			// д��������Ҫ���µ�StoreValue���µ�CacheLine��
			switch (DataSize)
			{
			case 1:	// 1���ֽ�
				DCache[CacheLineAddress].Data[BlockOffset + 0] = StoreValue & 0xFF;
				break;
			case 2:	// 2���ֽ�
				BlockOffset = BlockOffset & 0xFE;	// ����뵽2�ֽڱ߽�
				DCache[CacheLineAddress].Data[BlockOffset + 0] = StoreValue & 0xFF; StoreValue = StoreValue >> 8;
				DCache[CacheLineAddress].Data[BlockOffset + 1] = StoreValue & 0xFF;
				break;
			case 4:	// 4���ֽ�
				BlockOffset = BlockOffset & 0xFC;	// ����뵽4�ֽڱ߽�
				DCache[CacheLineAddress].Data[BlockOffset + 0] = StoreValue & 0xFF; StoreValue = StoreValue >> 8;
				DCache[CacheLineAddress].Data[BlockOffset + 1] = StoreValue & 0xFF; StoreValue = StoreValue >> 8;
				DCache[CacheLineAddress].Data[BlockOffset + 2] = StoreValue & 0xFF; StoreValue = StoreValue >> 8;
				DCache[CacheLineAddress].Data[BlockOffset + 3] = StoreValue & 0xFF;
				break;
			case 8:	// 8���ֽ�
				BlockOffset = BlockOffset & 0xF8;	// ����뵽8�ֽڱ߽�
				DCache[CacheLineAddress].Data[BlockOffset + 0] = StoreValue & 0xFF; StoreValue = StoreValue >> 8;
				DCache[CacheLineAddress].Data[BlockOffset + 1] = StoreValue & 0xFF; StoreValue = StoreValue >> 8;
				DCache[CacheLineAddress].Data[BlockOffset + 2] = StoreValue & 0xFF; StoreValue = StoreValue >> 8;
				DCache[CacheLineAddress].Data[BlockOffset + 3] = StoreValue & 0xFF; StoreValue = StoreValue >> 8;
				DCache[CacheLineAddress].Data[BlockOffset + 4] = StoreValue & 0xFF; StoreValue = StoreValue >> 8;
				DCache[CacheLineAddress].Data[BlockOffset + 5] = StoreValue & 0xFF; StoreValue = StoreValue >> 8;
				DCache[CacheLineAddress].Data[BlockOffset + 6] = StoreValue & 0xFF; StoreValue = StoreValue >> 8;
				DCache[CacheLineAddress].Data[BlockOffset + 7] = StoreValue & 0xFF;
				break;
			}
		}
	}
    
	return MissFlag;

}

/* ָ��Cacheʵ�ֲ��֣���ѡʵ�� */
void InitInstCache(void)
{
	return;
}

void LoadInstCacheLineFromMemory(UINT64 Address, UINT32 CacheLineAddress)
{
	return;
}

UINT8 AccessInstCache(UINT64 Address, UINT8 Operation, UINT8 InstSize, UINT64* InstResult)
{
	// ����ֵ'M' = Miss��'H'=Hit
	return 'M';
}