

##  Lab07 Cache

### 资料查找

#### Victim Cache

[Computer Architecture —— Advanced Cache 高级缓存优化（三） - 知乎 (zhihu.com)](https://zhuanlan.zhihu.com/p/388751723)

虽然名字怪怪的（有的地方翻译成“受害者缓存”），但它的特性和原理其实和简单。Victim Cache 就是另一块小的 Cache，是缓存的缓存（Cache of Cache）。里面的数据都是从原本的 Cache 驱逐出来的（Evicted），所以它们被称为受害者，或者被牺牲者（Victim）。

在传统的缓存系统中，被驱逐的脏线（Evicted Cache Line）会失效，或者被新的缓存线（Cache Line）代替。而有了牺牲缓存，这些被驱逐出去的脏线就有了去除，他们会被存在这个小缓存中，等待着下一次被召唤（也可能没被召唤，下场就是被踢回 Memory 中了）。

这块牺牲缓存通常是全相联（Fully Associative），并且有着非常少的输入口（Entries）（通常 4 ~ 16 个）。在[多级缓存系统](https://zhuanlan.zhihu.com/p/386437697)中，牺牲缓存通常放置在 L1 缓存旁，两个缓存可以进行并行访问（Parallel Checking），或者按序访问（Series Checking）。

牺牲缓存虽然可以减少错失率和错失成本，但会给硬件带了一些新的挑战。如果我们把它和 L1 缓存设计成并行访问，那么硬件复杂度和功耗都会上升；如果设计出按序访问，那么缓存系统整体的延迟就会增加，毕竟多了一个访问步骤。

![img](https://pic1.zhimg.com/80/v2-f1f3d6955a7e96b96c362a95f175185c_1440w.webp)

1. 检查 L1 Cache；如果命中，那么返回数据
2. 如果没有命中，那么检查 Victim Cache（假设是按序检查）。如果 Victim Cache 里面有这个数据，那么该数据将会返回给 L1 Cache 然后再送还给处理器。在 L1 Cache 里被驱逐的数据，会存放进 Victim Cache 里面
3. 如果 L1 Cache 和 Victim Cache 都没有该数据，那么就会从 Memory/L2 Cache 取出该数据，并把它放在 L1 Cache 里面。如果在该次操作里面，L1 Cache 需要驱逐一个数据，那么这个被驱逐的数据会被存进 Victim Cache 里面。任何被 Victim Cache 驱逐的未修改过的数据，都会写进 Memory 里面或者被丢弃

### 初始化部分

```c
#define DCACHE_SIZE						16384   //C
#define DCACHE_DATA_PER_LINE			16		    //B							
#define DCACHE_LINE_PER_SET             2   //E,随便设了个两行一组
#define DCACHE_DATA_PER_LINE_ADDR_BITS	GET_POWER_OF_2(DCACHE_DATA_PER_LINE)	
#define DCACHE_SET						(DCACHE_SIZE/DCACHE_DATA_PER_LINE/DCACHE_LINE_PER_SET)//S
#define DCACHE_SET_ADDR_BITS			GET_POWER_OF_2(DCACHE_SET)		

// Cache行的结构，包括Valid、Tag和Data。你所有的状态信息，只能记录在Cache行中！
struct DCACHE_LineStruct
{
	UINT8	Valid;//4位，1字节
	UINT64	Tag;//8字节
	UINT8	Data[DCACHE_DATA_PER_LINE];
}DCache[DCACHE_SET*DCACHE_LINE_PER_SET];
//DCache组的结构
struct DCACHE_Set
{
    struct DCACHE_LineStruct * Lines;
}DCache_Sets[DCACHE_SET];

/*
	DCache初始化代码，一般需要把DCache的有效位Valid设置为0
	模拟器启动时，会调用此InitDataCache函数
*/
void InitDataCache()
{
	UINT32 i;
	printf("[%s] +-----------------------------------+\n", __func__);
	printf("[%s] |   zly的Data Cache初始化ing.... |\n", __func__);
	printf("[%s] +-----------------------------------+\n", __func__);
	for(i=0;i<DCACHE_SET;i++){
        DCache_Sets[i].Lines=&(DCache[DCACHE_LINE_PER_SET*i]);
    }
    for (i = 0; i < DCACHE_SET*DCACHE_LINE_PER_SET; i++)
		DCache[i].Valid = 0;
}

```

在不改变 `DCache` 数组的存储方式的前提下，将每组的首地址赋给 `DCache_Sets` 数组中的元素

### 结果

### 综合

|                   |         直接映射          |     简单组相联（E=2)      |   组相联+Istr+Vic(E=2)    |          组相联+Istr+Vic(E=4)           | 组相联+Istr+Vic(E=8) |
| :---------------: | :-----------------------: | :-----------------------: | :-----------------------: | :-------------------------------------: | :------------------: |
| **DCache命中率**  |          85.01%           |          88.57%           |          88.57%           |                 89.59%                  |        91.27%        |
| **ICache命中率**  |           0.00%           |           0.00%           |          97.71%           |                 97.71%                  |        97.71%        |
| **时间耗费（ms)** |           3036            |           3380            |           5381            |                  4809                   |         5054         |
|                   | **组相联+Istr+Vic(E=16)** | **组相联+Istr+Vic(E=32)** | **组相联+Istr+Vic(E=64)** | **(不加victim的对比)组相联+Istr(E=64)** |                      |
| **DCache命中率**  |          92.77%           |          93.39%           |          93.58%           |                 93.39%                  |                      |
| **ICache命中率**  |          97.71%           |          97.71%           |          97.71%           |                 97.71%                  |                      |
| **时间耗费（ms)** |           5421            |           6612            |           7654            |                  7019                   |                      |



#### 直接映射

```
lianyi@lianyi-Ubuntu:~/CSAPP/Cache$ ./Cache traces/gedit.trace.zst
[main] Cache模拟器框架 v3.0 by mars, 2022
[main] 初始化存储器，读入Trace文件[traces/gedit.trace.zst]，请稍后...
[parse_TRACE_file] ====已处理74360000行====
[parse_TRACE_file] +-----------------------------------------------------+
[parse_TRACE_file] |  Memory Trace数量                 	 :   45326334    |
[parse_TRACE_file] |    Instruction操作数量            	 :   25126783    |
[parse_TRACE_file] |    Data Load操作数量              	 :   14776439    |
[parse_TRACE_file] |    Data Store操作数量             	 :    5299553    |
[parse_TRACE_file] |    Data Modify操作数量            	 :     123559    |
[parse_TRACE_file] | 时间耗费（ms）                    	 :      94778    |
[parse_TRACE_file] +-----------------------------------------------------+
[main] 处理Trace文件完毕
[InitDataCache] +-----------------------------------+
[InitDataCache] |   威震天的Data Cache初始化ing.... |
[InitDataCache] +-----------------------------------+
[main] 开始Cache模拟，请稍后...
[SimTrace] +-----------------------------------------------------+
[SimTrace] |  Memory Trace数量                	 :   45326334    |
[SimTrace] |    Instruction操作数量           	 :   25126783    |
[SimTrace] |    Data Load操作数量             	 :   14776439    |
[SimTrace] |    Data Store操作数量            	 :    5299553    |
[SimTrace] |    Data Modify操作数量           	 :     123559    |
[SimTrace] |    Instruction操作Cache命中数量  	 :          0    |
[SimTrace] |    Data Load操作Cache命中数量    	 :   11946639    |
[SimTrace] |    Data Store操作Cache命中数量   	 :    5108655    |
[SimTrace] |    Data Modify操作Cache命中数量  	 :     115529    |
[SimTrace] |  Cache访存数量                   	 :   12112864    |
[SimTrace] |    Cache读存储器数量             	 :    6057456    |
[SimTrace] |    Cache写存储器数量             	 :    6055408    |
[SimTrace] |  Data Cache命中率                	 :     85.01%    |
[SimTrace] |  Inst Cache命中率                	 :      0.00%    |
[SimTrace] |  时间耗费（ms）                  	 :       3036    |
[SimTrace] +-----------------------------------------------------+
[main] Cache模拟成功完成

```

#### 组相联（E=2）

```
lianyi@lianyi-Ubuntu:~/CSAPP/Cache$ ./Cache traces/gedit.trace.zst
[main] Cache模拟器框架 v3.0 by mars, 2022
[main] 初始化存储器，读入Trace文件[traces/gedit.trace.zst]，请稍后...
[parse_TRACE_file] ====已处理74360000行====
[parse_TRACE_file] +-----------------------------------------------------+
[parse_TRACE_file] |  Memory Trace数量                 	 :   45326334    |
[parse_TRACE_file] |    Instruction操作数量            	 :   25126783    |
[parse_TRACE_file] |    Data Load操作数量              	 :   14776439    |
[parse_TRACE_file] |    Data Store操作数量             	 :    5299553    |
[parse_TRACE_file] |    Data Modify操作数量            	 :     123559    |
[parse_TRACE_file] | 时间耗费（ms）                    	 :      72949    |
[parse_TRACE_file] +-----------------------------------------------------+
[main] 处理Trace文件完毕
[InitDataCache] +-----------------------------------+
[InitDataCache] |   zly��Data Cache��ʼ��ing.... |
[InitDataCache] +-----------------------------------+
[main] 开始Cache模拟，请稍后...
[SimTrace] +-----------------------------------------------------+
[SimTrace] |  Memory Trace数量                	 :   45326334    |
[SimTrace] |    Instruction操作数量           	 :   25126783    |
[SimTrace] |    Data Load操作数量             	 :   14776439    |
[SimTrace] |    Data Store操作数量            	 :    5299553    |
[SimTrace] |    Data Modify操作数量           	 :     123559    |
[SimTrace] |    Instruction操作Cache命中数量  	 :          0    |
[SimTrace] |    Data Load操作Cache命中数量    	 :   12620542    |
[SimTrace] |    Data Store操作Cache命中数量   	 :    5151325    |
[SimTrace] |    Data Modify操作Cache命中数量  	 :     119572    |
[SimTrace] |  Cache访存数量                   	 :    9230400    |
[SimTrace] |    Cache读存储器数量             	 :    4616224    |
[SimTrace] |    Cache写存储器数量             	 :    4614176    |
[SimTrace] |  Data Cache命中率                	 :     88.57%    |
[SimTrace] |  Inst Cache命中率                	 :      0.00%    |
[SimTrace] |  时间耗费（ms）                  	 :       3380    |
[SimTrace] +-----------------------------------------------------+
[main] Cache模拟成功完成

```

### victim+instruction+(E=2)

```
 ./Cache traces/gedit.trace.zst
[main] Cache模拟器框架 v3.0 by mars, 2022
[main] 初始化存储器，读入Trace文件[traces/gedit.trace.zst]，请稍后...
[parse_TRACE_file] ====已处理74360000行====
[parse_TRACE_file] +-----------------------------------------------------+
[parse_TRACE_file] |  Memory Trace数量                 	 :   45326334    |
[parse_TRACE_file] |    Instruction操作数量            	 :   25126783    |
[parse_TRACE_file] |    Data Load操作数量              	 :   14776439    |
[parse_TRACE_file] |    Data Store操作数量             	 :    5299553    |
[parse_TRACE_file] |    Data Modify操作数量            	 :     123559    |
[parse_TRACE_file] | 时间耗费（ms）                    	 :      74479    |
[parse_TRACE_file] +-----------------------------------------------------+
[main] 处理Trace文件完毕
[InitDataCache] +-----------------------------------+
[InitDataCache] |   zly��Data Cache��ʼ��ing.... |
[InitDataCache] +-----------------------------------+
[InitInstCache] +-----------------------------------+
[InitInstCache] | zly �� Instrction Cache ��ʼ��ing..|
[InitInstCache] +-----------------------------------+
[main] 开始Cache模拟，请稍后...
[SimTrace] +-----------------------------------------------------+
[SimTrace] |  Memory Trace数量                	 :   45326334    |
[SimTrace] |    Instruction操作数量           	 :   25126783    |
[SimTrace] |    Data Load操作数量             	 :   14776439    |
[SimTrace] |    Data Store操作数量            	 :    5299553    |
[SimTrace] |    Data Modify操作数量           	 :     123559    |
[SimTrace] |    Instruction操作Cache命中数量  	 :   24552212    |
[SimTrace] |    Data Load操作Cache命中数量    	 :   12620490    |
[SimTrace] |    Data Store操作Cache命中数量   	 :    5151469    |
[SimTrace] |    Data Modify操作Cache命中数量  	 :     119573    |
[SimTrace] |  Cache访存数量                   	 :   10379170    |
[SimTrace] |    Cache读存储器数量             	 :    5765180    |
[SimTrace] |    Cache写存储器数量             	 :    4613990    |
[SimTrace] |  Data Cache命中率                	 :     88.57%    |
[SimTrace] |  Inst Cache命中率                	 :     97.71%    |
[SimTrace] |  时间耗费（ms）                  	 :       5381    |
[SimTrace] +-----------------------------------------------------+
[main] Cache模拟成功完成

```

### victim+instruction+(E=4)

```
./Cache traces/gedit.trace.zst
[main] Cache模拟器框架 v3.0 by mars, 2022
[main] 初始化存储器，读入Trace文件[traces/gedit.trace.zst]，请稍后...
[parse_TRACE_file] ====已处理74360000行====
[parse_TRACE_file] +-----------------------------------------------------+
[parse_TRACE_file] |  Memory Trace数量                 	 :   45326334    |
[parse_TRACE_file] |    Instruction操作数量            	 :   25126783    |
[parse_TRACE_file] |    Data Load操作数量              	 :   14776439    |
[parse_TRACE_file] |    Data Store操作数量             	 :    5299553    |
[parse_TRACE_file] |    Data Modify操作数量            	 :     123559    |
[parse_TRACE_file] | 时间耗费（ms）                    	 :      73295    |
[parse_TRACE_file] +-----------------------------------------------------+
[main] 处理Trace文件完毕
[InitDataCache] +-----------------------------------+
[InitDataCache] |   zly��Data Cache��ʼ��ing.... |
[InitDataCache] +-----------------------------------+
[InitInstCache] +-----------------------------------+
[InitInstCache] | zly �� Instrction Cache ��ʼ��ing..|
[InitInstCache] +-----------------------------------+
[main] 开始Cache模拟，请稍后...
[SimTrace] +-----------------------------------------------------+
[SimTrace] |  Memory Trace数量                	 :   45326334    |
[SimTrace] |    Instruction操作数量           	 :   25126783    |
[SimTrace] |    Data Load操作数量             	 :   14776439    |
[SimTrace] |    Data Store操作数量            	 :    5299553    |
[SimTrace] |    Data Modify操作数量           	 :     123559    |
[SimTrace] |    Instruction操作Cache命中数量  	 :   24552212    |
[SimTrace] |    Data Load操作Cache命中数量    	 :   12815239    |
[SimTrace] |    Data Store操作Cache命中数量   	 :    5160759    |
[SimTrace] |    Data Modify操作Cache命中数量  	 :     119990    |
[SimTrace] |  Cache访存数量                   	 :    9561346    |
[SimTrace] |    Cache读存储器数量             	 :    5356268    |
[SimTrace] |    Cache写存储器数量             	 :    4205078    |
[SimTrace] |  Data Cache命中率                	 :     89.59%    |
[SimTrace] |  Inst Cache命中率                	 :     97.71%    |
[SimTrace] |  时间耗费（ms）                  	 :       4809    |
[SimTrace] +-----------------------------------------------------+
[main] Cache模拟成功完成

```

### victim+instruction+(E=8)

```
./Cache traces/gedit.trace.zst
[main] Cache模拟器框架 v3.0 by mars, 2022
[main] 初始化存储器，读入Trace文件[traces/gedit.trace.zst]，请稍后...
[parse_TRACE_file] ====已处理74360000行====
[parse_TRACE_file] +-----------------------------------------------------+
[parse_TRACE_file] |  Memory Trace数量                 	 :   45326334    |
[parse_TRACE_file] |    Instruction操作数量            	 :   25126783    |
[parse_TRACE_file] |    Data Load操作数量              	 :   14776439    |
[parse_TRACE_file] |    Data Store操作数量             	 :    5299553    |
[parse_TRACE_file] |    Data Modify操作数量            	 :     123559    |
[parse_TRACE_file] | 时间耗费（ms）                    	 :      72092    |
[parse_TRACE_file] +-----------------------------------------------------+
[main] 处理Trace文件完毕
[InitDataCache] +-----------------------------------+
[InitDataCache] |   zly��Data Cache��ʼ��ing.... |
[InitDataCache] +-----------------------------------+
[InitInstCache] +-----------------------------------+
[InitInstCache] | zly �� Instrction Cache ��ʼ��ing..|
[InitInstCache] +-----------------------------------+
[main] 开始Cache模拟，请稍后...
[SimTrace] +-----------------------------------------------------+
[SimTrace] |  Memory Trace数量                	 :   45326334    |
[SimTrace] |    Instruction操作数量           	 :   25126783    |
[SimTrace] |    Data Load操作数量             	 :   14776439    |
[SimTrace] |    Data Store操作数量            	 :    5299553    |
[SimTrace] |    Data Modify操作数量           	 :     123559    |
[SimTrace] |    Instruction操作Cache命中数量  	 :   24552212    |
[SimTrace] |    Data Load操作Cache命中数量    	 :   13143846    |
[SimTrace] |    Data Store操作Cache命中数量   	 :    5171453    |
[SimTrace] |    Data Modify操作Cache命中数量  	 :     120502    |
[SimTrace] |  Cache访存数量                   	 :    8202094    |
[SimTrace] |    Cache读存储器数量             	 :    4676642    |
[SimTrace] |    Cache写存储器数量             	 :    3525452    |
[SimTrace] |  Data Cache命中率                	 :     91.27%    |
[SimTrace] |  Inst Cache命中率                	 :     97.71%    |
[SimTrace] |  时间耗费（ms）                  	 :       5054    |
[SimTrace] +-----------------------------------------------------+
[main] Cache模拟成功完成
```

### victim+instruction+(E=16)

```
./Cache traces/gedit.trace.zst
[main] Cache模拟器框架 v3.0 by mars, 2022
[main] 初始化存储器，读入Trace文件[traces/gedit.trace.zst]，请稍后...
[parse_TRACE_file] ====已处理74360000行====
[parse_TRACE_file] +-----------------------------------------------------+
[parse_TRACE_file] |  Memory Trace数量                 	 :   45326334    |
[parse_TRACE_file] |    Instruction操作数量            	 :   25126783    |
[parse_TRACE_file] |    Data Load操作数量              	 :   14776439    |
[parse_TRACE_file] |    Data Store操作数量             	 :    5299553    |
[parse_TRACE_file] |    Data Modify操作数量            	 :     123559    |
[parse_TRACE_file] | 时间耗费（ms）                    	 :      71501    |
[parse_TRACE_file] +-----------------------------------------------------+
[main] 处理Trace文件完毕
[InitDataCache] +-----------------------------------+
[InitDataCache] |   zly��Data Cache��ʼ��ing.... |
[InitDataCache] +-----------------------------------+
[InitInstCache] +-----------------------------------+
[InitInstCache] | zly �� Instrction Cache ��ʼ��ing..|
[InitInstCache] +-----------------------------------+
[main] 开始Cache模拟，请稍后...
[SimTrace] +-----------------------------------------------------+
[SimTrace] |  Memory Trace数量                	 :   45326334    |
[SimTrace] |    Instruction操作数量           	 :   25126783    |
[SimTrace] |    Data Load操作数量             	 :   14776439    |
[SimTrace] |    Data Store操作数量            	 :    5299553    |
[SimTrace] |    Data Modify操作数量           	 :     123559    |
[SimTrace] |    Instruction操作Cache命中数量  	 :   24552212    |
[SimTrace] |    Data Load操作Cache命中数量    	 :   13444722    |
[SimTrace] |    Data Store操作Cache命中数量   	 :    5173454    |
[SimTrace] |    Data Modify操作Cache命中数量  	 :     120365    |
[SimTrace] |  Cache访存数量                   	 :    6991134    |
[SimTrace] |    Cache读存储器数量             	 :    4071162    |
[SimTrace] |    Cache写存储器数量             	 :    2919972    |
[SimTrace] |  Data Cache命中率                	 :     92.77%    |
[SimTrace] |  Inst Cache命中率                	 :     97.71%    |
[SimTrace] |  时间耗费（ms）                  	 :       5421    |
[SimTrace] +-----------------------------------------------------+
[main] Cache模拟成功完成

```

### victim+instruction+(E=32)

```
./Cache traces/gedit.trace.zst
[main] Cache模拟器框架 v3.0 by mars, 2022
[main] 初始化存储器，读入Trace文件[traces/gedit.trace.zst]，请稍后...
[parse_TRACE_file] ====已处理74360000行====
[parse_TRACE_file] +-----------------------------------------------------+
[parse_TRACE_file] |  Memory Trace数量                 	 :   45326334    |
[parse_TRACE_file] |    Instruction操作数量            	 :   25126783    |
[parse_TRACE_file] |    Data Load操作数量              	 :   14776439    |
[parse_TRACE_file] |    Data Store操作数量             	 :    5299553    |
[parse_TRACE_file] |    Data Modify操作数量            	 :     123559    |
[parse_TRACE_file] | 时间耗费（ms）                    	 :      71403    |
[parse_TRACE_file] +-----------------------------------------------------+
[main] 处理Trace文件完毕
[InitDataCache] +-----------------------------------+
[InitDataCache] |   zly��Data Cache��ʼ��ing.... |
[InitDataCache] +-----------------------------------+
[InitInstCache] +-----------------------------------+
[InitInstCache] | zly �� Instrction Cache ��ʼ��ing..|
[InitInstCache] +-----------------------------------+
[main] 开始Cache模拟，请稍后...
[SimTrace] +-----------------------------------------------------+
[SimTrace] |  Memory Trace数量                	 :   45326334    |
[SimTrace] |    Instruction操作数量           	 :   25126783    |
[SimTrace] |    Data Load操作数量             	 :   14776439    |
[SimTrace] |    Data Store操作数量            	 :    5299553    |
[SimTrace] |    Data Modify操作数量           	 :     123559    |
[SimTrace] |    Instruction操作Cache命中数量  	 :   24552212    |
[SimTrace] |    Data Load操作Cache命中数量    	 :   13570003    |
[SimTrace] |    Data Store操作Cache命中数量   	 :    5174310    |
[SimTrace] |    Data Modify操作Cache命中数量  	 :     120294    |
[SimTrace] |  Cache访存数量                   	 :    6486870    |
[SimTrace] |    Cache读存储器数量             	 :    3819030    |
[SimTrace] |    Cache写存储器数量             	 :    2667840    |
[SimTrace] |  Data Cache命中率                	 :     93.39%    |
[SimTrace] |  Inst Cache命中率                	 :     97.71%    |
[SimTrace] |  时间耗费（ms）                  	 :       6612    |
[SimTrace] +-----------------------------------------------------+
[main] Cache模拟成功完成

```





### victim的效果

* 用victim前

```
 ./Cache traces/gedit.trace.zst
[main] Cache模拟器框架 v3.0 by mars, 2022
[main] 初始化存储器，读入Trace文件[traces/gedit.trace.zst]，请稍后...
[parse_TRACE_file] ====已处理74360000行====
[parse_TRACE_file] +-----------------------------------------------------+
[parse_TRACE_file] |  Memory Trace数量                   :   45326334    |
[parse_TRACE_file] |    Instruction操作数量              :   25126783    |
[parse_TRACE_file] |    Data Load操作数量                :   14776439    |
[parse_TRACE_file] |    Data Store操作数量               :    5299553    |
[parse_TRACE_file] |    Data Modify操作数量              :     123559    |
[parse_TRACE_file] | 时间耗费（ms）                      :      77066    |
[parse_TRACE_file] +-----------------------------------------------------+
[main] 处理Trace文件完毕
[InitDataCache] +-----------------------------------+
[InitDataCache] |   zly��Data Cache��ʼ��ing.... |
[InitDataCache] +-----------------------------------+
[InitInstCache] +-----------------------------------+
[InitInstCache] | zly �� Instrction Cache ��ʼ��ing..|
[InitInstCache] +-----------------------------------+
[main] 开始Cache模拟，请稍后...
[SimTrace] +-----------------------------------------------------+
[SimTrace] |  Memory Trace数量                   :   45326334    |
[SimTrace] |    Instruction操作数量              :   25126783    |
[SimTrace] |    Data Load操作数量                :   14776439    |
[SimTrace] |    Data Store操作数量               :    5299553    |
[SimTrace] |    Data Modify操作数量              :     123559    |
[SimTrace] |    Instruction操作Cache命中数量     :   24552212    |
[SimTrace] |    Data Load操作Cache命中数量       :   13569737    |
[SimTrace] |    Data Store操作Cache命中数量      :    5174231    |
[SimTrace] |    Data Modify操作Cache命中数量     :     120283    |
[SimTrace] |  Cache访存数量                      :    6488294    |
[SimTrace] |    Cache读存储器数量                :    3819742    |
[SimTrace] |    Cache写存储器数量                :    2668552    |
[SimTrace] |  Data Cache命中率                   :     93.39%    |
[SimTrace] |  Inst Cache命中率                   :     97.71%    |
[SimTrace] |  时间耗费（ms）                     :       7019    |
[SimTrace] +-----------------------------------------------------+
[main] Cache模拟成功完成
```

* 用victim后

```
lianyi@lianyi-Ubuntu:~/CSAPP/Cache$ ./Cache traces/gedit.trace.zst
[main] Cache模拟器框架 v3.0 by mars, 2022
[main] 初始化存储器，读入Trace文件[traces/gedit.trace.zst]，请稍后...
[parse_TRACE_file] ====已处理74360000行====
[parse_TRACE_file] +-----------------------------------------------------+
[parse_TRACE_file] |  Memory Trace数量                   :   45326334    |
[parse_TRACE_file] |    Instruction操作数量              :   25126783    |
[parse_TRACE_file] |    Data Load操作数量                :   14776439    |
[parse_TRACE_file] |    Data Store操作数量               :    5299553    |
[parse_TRACE_file] |    Data Modify操作数量              :     123559    |
[parse_TRACE_file] | 时间耗费（ms）                      :      56889    |
[parse_TRACE_file] +-----------------------------------------------------+
[main] 处理Trace文件完毕
[InitDataCache] +-----------------------------------+
[InitDataCache] |   zly��Data Cache��ʼ��ing.... |
[InitDataCache] +-----------------------------------+
[InitInstCache] +-----------------------------------+
[InitInstCache] | zly �� Instrction Cache ��ʼ��ing..|
[InitInstCache] +-----------------------------------+
[main] 开始Cache模拟，请稍后...
[SimTrace] +-----------------------------------------------------+
[SimTrace] |  Memory Trace数量                   :   45326334    |
[SimTrace] |    Instruction操作数量              :   25126783    |
[SimTrace] |    Data Load操作数量                :   14776439    |
[SimTrace] |    Data Store操作数量               :    5299553    |
[SimTrace] |    Data Modify操作数量              :     123559    |
[SimTrace] |    Instruction操作Cache命中数量     :   24552212    |
[SimTrace] |    Data Load操作Cache命中数量       :   13610017    |
[SimTrace] |    Data Store操作Cache命中数量      :    5172487    |
[SimTrace] |    Data Modify操作Cache命中数量     :     120333    |
[SimTrace] |  Cache访存数量                      :    6333950    |
[SimTrace] |    Cache读存储器数量                :    3742570    |
[SimTrace] |    Cache写存储器数量                :    2591380    |
[SimTrace] |  Data Cache命中率                   :     93.58%    |
[SimTrace] |  Inst Cache命中率                   :     97.71%    |
[SimTrace] |  时间耗费（ms）                     :       7654    |
[SimTrace] +-----------------------------------------------------+
[main] Cache模拟成功完成
```

