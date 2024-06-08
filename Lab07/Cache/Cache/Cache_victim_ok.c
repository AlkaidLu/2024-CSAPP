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
	??????Data Cache??16KB??§³
	??§Õ??64????????256??
*/
#define DCACHE_SIZE						16384   //C
#define DCACHE_DATA_PER_LINE			16		    //B							
#define DCACHE_LINE_PER_SET             32   //E,???????????????
#define DCACHE_DATA_PER_LINE_ADDR_BITS	GET_POWER_OF_2(DCACHE_DATA_PER_LINE)	
#define DCACHE_SET						(DCACHE_SIZE/DCACHE_DATA_PER_LINE/DCACHE_LINE_PER_SET)//S
#define DCACHE_SET_ADDR_BITS			GET_POWER_OF_2(DCACHE_SET)		

#define VCACHE_SIZE                     4096
#define VCACHE_DATA_PER_LINE			16
#define VCACHE_LINE                     GET_POWER_OF_2(VCACHE_SIZE/VCACHE_DATA_PER_LINE)

#define ICACHE_SIZE 16384		// 16 KB
#define ICACHE_DATA_PER_LINE 16 // ??§Ö??????
#define ICACHE_DATA_PER_LINE_ADDR_BITS GET_POWER_OF_2(ICACHE_DATA_PER_LINE)
#define ICACHE_LINE (ICACHE_SIZE / ICACHE_DATA_PER_LINE) // Cache??????
#define ICACHE_LINE_ADDR_BITS GET_POWER_OF_2(ICACHE_LINE)

// Cache?§Ö????????Valid??Tag??Data???????§Ö???????????????Cache???§µ?

struct DCACHE_LineStruct
{
	UINT8	Valid;//4¦Ë??1???
	UINT64	Tag;//8???
	UINT8	Data[DCACHE_DATA_PER_LINE];
}DCache[DCACHE_SET*DCACHE_LINE_PER_SET],VCache[VCACHE_LINE],ICache[ICACHE_LINE];
//DCache????

/*
	DCache?????????????????DCache????§¹¦ËValid?????0
	??????????????????InitDataCache????
*/

void InitDataCache()
{
	UINT32 i,j,k,l;
	printf("[%s] +-----------------------------------+\n", __func__);
	printf("[%s] |   zly??Data Cache?????ing.... |\n", __func__);
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
	??Memory?§Ø???????????Data Cache??
*/
void LoadDataCacheLineFromMemory(UINT64 Address, UINT32 CacheLineAddress)
{
	// ??????Memory?§ß?DCACHE_DATA_PER_LINE??????????Data Cache??
	// ???????????????¦Ï??????8?????
	UINT32 i;
	UINT64 ReadData;
	UINT64 AlignAddress;
	UINT64* pp;

	AlignAddress = Address & ~(DCACHE_DATA_PER_LINE - 1);	// ??????????DCACHE_DATA_PER_LINE (64)?????
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
	??Data Cache?§Ö?????????§Õ??›¥??
*/
void StoreDataCacheLineToMemory(UINT64 Address, UINT32 CacheLineAddress)
{
	// ??????DCACHE_DATA_PER_LINE????????Data Cache??§Õ??Memory??
	// ???????????????¦Ï???§Õ??8?????
	UINT32 i;
	UINT64 WriteData;
	UINT64 AlignAddress;
	UINT64* pp;

	AlignAddress = Address & ~(DCACHE_DATA_PER_LINE - 1);	// ??????????DCACHE_DATA_PER_LINE (64)?????
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
    ??????????¨²??????????????
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
		BlockOffset = BlockOffset & 0xF8; // ?????8?????
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
    ??????????¨²??????????????
*/
void WriteDataCache(UINT32 LineAddress, UINT8 DataSize, UINT8 BlockOffset, UINT64 StoreValue)
{
	switch (DataSize)
	{
	case 1: // 1?????
		DCache[LineAddress].Data[BlockOffset + 0] = StoreValue & 0xFF;
		break;
	case 2:								  // 2?????
		BlockOffset = BlockOffset & 0xFE; // ?????2?????
		DCache[LineAddress].Data[BlockOffset + 0] = StoreValue & 0xFF;
		StoreValue = StoreValue >> 8;
		DCache[LineAddress].Data[BlockOffset + 1] = StoreValue & 0xFF;
		break;
	case 4:								  // 4?????
		BlockOffset = BlockOffset & 0xFC; // ?????4?????
		DCache[LineAddress].Data[BlockOffset + 0] = StoreValue & 0xFF;
		StoreValue = StoreValue >> 8;
		DCache[LineAddress].Data[BlockOffset + 1] = StoreValue & 0xFF;
		StoreValue = StoreValue >> 8;
		DCache[LineAddress].Data[BlockOffset + 2] = StoreValue & 0xFF;
		StoreValue = StoreValue >> 8;
		DCache[LineAddress].Data[BlockOffset + 3] = StoreValue & 0xFF;
		break;
	case 8:								  // 8?????
		BlockOffset = BlockOffset & 0xF8; // ?????8?????
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
    ??DCache????????????????VictimCache
*/
void StoreDataCacheLineToVictimCache(UINT32 DCacheLineAddress, UINT32 VCacheLineAddress)
{
    // ???DCacheLineAddress?????DCache???¦¶??
    if (DCacheLineAddress < DCACHE_SET * DCACHE_LINE_PER_SET)
    {
        // ???VCacheLineAddress?????VCache???¦¶??
        if (VCacheLineAddress < VCACHE_LINE)
        {
            // ????DCache?§Ö?????§Ö?VCache??
            VCache[VCacheLineAddress].Tag=DCache[DCacheLineAddress].Tag ;
            memcpy(VCache[VCacheLineAddress].Data, DCache[DCacheLineAddress].Data, DCACHE_DATA_PER_LINE);
        }
    }
}
/*
    ??VictimCache?????????Dcache
*/
void LoadDataCacheLineFromVictimCache(UINT32 VCacheLineAddress, UINT32 DCacheLineAddress)
{
     if (DCacheLineAddress < DCACHE_SET * DCACHE_LINE_PER_SET)
    {
        // ???VCacheLineAddress?????VCache???¦¶??
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
	// ??Victim Cache???§á??§µ??????§Ö??§Ü?
	for(i = 0; i< VCACHE_SIZE; i++){
		if(VCache[i].Valid == 0){
			return i;
		}
	}
	// ????§á??§µ???????????victim??????(???dcache?§Ö???????§Õ??victimcache???????????)
	UINT32 line=rand()%VCACHE_SIZE;
    return line;
}

/*
	Data Cache????????????????????????????????Data Cache????
	Address:	????????
	Operation:	??????????????'L'????§Õ??????'S'??????-???-§Õ??????'M'??
	DataSize:	?????§³??1????2????4????8???
	StoreValue:	?????§Õ????????????§Õ???????
	LoadResult:	????§Ø???????????Cache??????????
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
	*	???????§µ?Address???§Ù??  AddressTag??CacheLineAddress??BlockOffset
	*/

	
	CacheSetIndex = (Address >> DCACHE_DATA_PER_LINE_ADDR_BITS) % DCACHE_SET;
	BlockOffset = Address % DCACHE_DATA_PER_LINE;
	AddressTag = (Address >> DCACHE_DATA_PER_LINE_ADDR_BITS) >> DCACHE_SET_ADDR_BITS;	// ??????DCACHE_SET??DCACHE_DATA_PER_LINE?????????Tag?????—ž???????????????Tag????
    for(LineOffset=0;LineOffset<DCACHE_LINE_PER_SET;LineOffset++){
        CacheLineAddress=CacheSetIndex* DCACHE_LINE_PER_SET+LineOffset;
        if (DCache[CacheLineAddress].Valid == 1 && DCache[CacheLineAddress].Tag == AddressTag)
        {
            MissFlag = 'H';		// ???§µ?
            Found=1;
            if (Operation == 'L')	// ??????
            {
                ReadValue=ReadDataCache(CacheLineAddress, DataSize, BlockOffset);
                *LoadResult = ReadValue;
                if (DEBUG)
                    printf("[%s] Address=%016llX Operation=%c DataSize=%u StoreValue=%016llX ReadValue=%016llX\n", __func__, Address, Operation, DataSize, StoreValue, ReadValue);
            }
            else if (Operation == 'S' || Operation == 'M')	// §Õ???????????????????§Õ??????
            {
                if (DEBUG)
                printf("[%s] Hit S/M: Address=%016llX Operation=%c DataSize=%u StoreValue=%016llX\n", __func__, Address, Operation, DataSize, StoreValue);
                WriteDataCache(CacheLineAddress, DataSize, BlockOffset, StoreValue);
            }
        }
	}
    //L1??§µ?????victim???????
    if(Found==0){
        UINT32 i=0;
        for(i=0;i<VCACHE_LINE;i++){
            if(VCache[i].Valid==1&& VCache[i].Tag==AddressTag)
            {
                if (DEBUG)
                    printf("[%s] Address=%016llX Operation=%c DataSize=%u StoreValue=%016llX\n", __func__, Address, Operation, DataSize, StoreValue);
                MissFlag = 'H';//??victimcache???????,????Hit
                Found=1;
                for(LineOffset=0;LineOffset<DCACHE_LINE_PER_SET;LineOffset++){
                    CacheLineAddress = CacheSetIndex * DCACHE_LINE_PER_SET + LineOffset;
                    if(DCache[CacheLineAddress].Valid == 0){
                        // ???????
                        break;
                    }
                }
                //????????
                if (LineOffset == DCACHE_LINE_PER_SET)
                {
                    LineOffset=rand()%DCACHE_LINE_PER_SET;
                    CacheLineAddress=CacheSetIndex*DCACHE_LINE_PER_SET+LineOffset;
                    // ????????Cache?§µ?????????Cache????????????§Õ???Memory??
                    // OldAddress = > (Tag,Set,0000)
                    Address = ((DCache[CacheLineAddress].Tag << DCACHE_SET_ADDR_BITS) << DCACHE_DATA_PER_LINE_ADDR_BITS) | ((UINT64)CacheSetIndex << DCACHE_DATA_PER_LINE_ADDR_BITS);	// ??Tag?§Ý???????
                    UINT64 NextVictimAddress=GetNextVictim();                 
                    StoreDataCacheLineToVictimCache(CacheLineAddress,NextVictimAddress);
                    StoreDataCacheLineToMemory(Address,CacheLineAddress);
                }
                
                // ?????VictimCache?§Ø????????
                LoadDataCacheLineFromVictimCache(i,CacheLineAddress);

                DCache[CacheLineAddress].Valid = 1;
                DCache[CacheLineAddress].Tag = AddressTag;
                if (Operation == 'L')	// ??????
                {
                    ReadValue=ReadDataCache(CacheLineAddress, DataSize, BlockOffset);
                    *LoadResult = ReadValue;
                    if (DEBUG)
                        printf("[%s] Address=%016llX Operation=%c DataSize=%u StoreValue=%016llX ReadValue=%016llX\n", __func__, Address, Operation, DataSize, StoreValue, ReadValue);
                }
                else if (Operation == 'S' || Operation == 'M')	// §Õ???????????????????§Õ??????
                {
                    if (DEBUG)
                    printf("[%s] Hit S/M: Address=%016llX Operation=%c DataSize=%u StoreValue=%016llX\n", __func__, Address, Operation, DataSize, StoreValue);
                    WriteDataCache(CacheLineAddress, DataSize, BlockOffset, StoreValue);
                    }
                }
            }
        }
    //????§µ???memory????
    if(Found==0){
        		if (DEBUG)
			printf("[%s] Address=%016llX Operation=%c DataSize=%u StoreValue=%016llX\n", __func__, Address, Operation, DataSize, StoreValue);
		MissFlag = 'M';		// ??????
		for(LineOffset=0;LineOffset<DCACHE_LINE_PER_SET;LineOffset++){
            CacheLineAddress = CacheSetIndex * DCACHE_LINE_PER_SET + LineOffset;
			if(DCache[CacheLineAddress].Valid == 0){
				// ???????
				break;
			}
        }
        if (LineOffset == DCACHE_LINE_PER_SET)
		{
            LineOffset=rand()%DCACHE_LINE_PER_SET;
            CacheLineAddress=CacheSetIndex*DCACHE_LINE_PER_SET+LineOffset;
			// ????????Cache?§µ?????????Cache????????????§Õ???Memory??
			UINT64 OldAddress;
            UINT64 NextVictimAddress=GetNextVictim();                 
            StoreDataCacheLineToVictimCache(CacheLineAddress,NextVictimAddress);
			// OldAddress = > (Tag,Set,0000)
			OldAddress = ((DCache[CacheLineAddress].Tag << DCACHE_SET_ADDR_BITS) << DCACHE_DATA_PER_LINE_ADDR_BITS) | ((UINT64)CacheSetIndex << DCACHE_DATA_PER_LINE_ADDR_BITS);	// ??Tag?§Ý???????
			StoreDataCacheLineToMemory(OldAddress, CacheLineAddress);
		}
		// ?????Memory?§Ø???????§µ?????????????LoadCacheLineFromMemory???????????
		LoadDataCacheLineFromMemory(Address, CacheLineAddress);
		DCache[CacheLineAddress].Valid = 1;
		DCache[CacheLineAddress].Tag = AddressTag;
		if (Operation == 'L')	// ??????
		{
			// ????????????????ï…??????MISS??
		}
		else if (Operation == 'S' || Operation == 'M')	// §Õ???????????????????§Õ??????
        {
            if (DEBUG)
                printf("[%s] Hit S/M: Address=%016llX Operation=%c DataSize=%u StoreValue=%016llX\n", __func__, Address, Operation, DataSize, StoreValue);
                WriteDataCache(CacheLineAddress, DataSize, BlockOffset, StoreValue);
        }
    }
    
    
	return MissFlag;

}

/* ???Cache????????????? */
void InitInstCache(void)
{
	UINT32 i;
	printf("[%s] +-----------------------------------+\n", __func__);
	printf("[%s] | zly ?? Instrction Cache ?????ing..|\n", __func__);
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

	AlignAddress = Address & ~(ICACHE_DATA_PER_LINE - 1); // ??????????DCACHE_DATA_PER_LINE (64)?????
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
		BlockOffset = BlockOffset & 0xF8; // ?????8?????
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
	// ?????'M' = Miss??'H'=Hit
	UINT32 CacheLineAddress;
	UINT8 BlockOffset;
	UINT64 AddressTag;
	UINT8 MissFlag = 'M';
	UINT64 ReadValue;

	*InstResult = 0;

	/*
	 *	???????§µ?Address???§Ù??  AddressTag??CacheLineAddress??BlockOffset
	 */
	CacheLineAddress = (Address >> ICACHE_DATA_PER_LINE_ADDR_BITS) % ICACHE_LINE;
	BlockOffset = Address % ICACHE_DATA_PER_LINE;
	AddressTag = (Address >> ICACHE_DATA_PER_LINE_ADDR_BITS) >> ICACHE_LINE_ADDR_BITS;

	if (ICache[CacheLineAddress].Valid == 1 && ICache[CacheLineAddress].Tag == AddressTag)
	{
		MissFlag = 'H'; // ???§µ?

		if (Operation == 'I')
		{ // ?????
			ReadValue = ReadInstCache(CacheLineAddress, InstSize, BlockOffset);
			*InstResult = ReadValue;
		}
	}
	else
	{
		MissFlag = 'M'; // ??????
		LoadInstCacheLineFromMemory(Address, CacheLineAddress);
		ICache[CacheLineAddress].Valid = 1;
		ICache[CacheLineAddress].Tag = AddressTag;
	}
	return MissFlag;
}
