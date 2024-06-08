imem = [''] * 1000      #指令存储
dmem = [0] * 1000       #数据存储
mem = ""
reg = [0]*15 #15个常用存储器
reg[4] = 750            #rsp栈位置 取dmem中的一段作为栈空间
#instructionlen
instructionlen = 0 
#PC
pReg = 0
iReg='' 
#STAT
Stat=1
#CC
ZF=0 
SF=0 
OF=0 

    
def get_V_D(instr):
 
    return str(instr[14]+instr[15]+instr[12]+instr[13]+instr[10]+instr[11]+instr[8]+instr[9]+instr[6]+instr[7]+instr[4]+instr[5])
 
def get_Dest(instr):
 
    return str(instr[12]+instr[13]+instr[10]+instr[11]+instr[8]+instr[9]+instr[6]+instr[7]+instr[4]+instr[5]+instr[2]+instr[3])
 
def loadProgram(file):
    
    global imem, dmem, mem, reg, iReg, pReg, ZF, SF, OF, Stat
    fil = open(file,'r')
    first = True
    lineno = 0
    line1=fil.readline()
    line2=fil.readline() #跳过前两行
    lines = fil.readlines()
    for line in lines:
        mem+=line.strip('\n') 
    mem=(mem.replace(",",""))[:-1]
    fil.close()
    p=0
    i=0
    while p < len(mem) and i<200:
        if mem[p] in ["0", "1", "9"]:
            imem[i]=mem[p:p+2]
            i += 1
            p += 2

        elif mem[p] in ["2", "6", "A", "B", "a", "b"]:
            imem[i]=mem[p:p+4]
            i += 1
            p += 4
        
        elif mem[p] in ["3", "4", "5"]:
            imem[i]=mem[p:p+20]
            i += 1
            p += 20
           
        elif mem[p] in ["7", "8"]:
            imem[i]=mem[p:p+18]
            i += 1
            p += 18
          
        elif mem[p] in ["F"]:
            imem[i]=mem[p:p+4]
            i += 1
            p += 4

        else:
            print("instruction error")
            break
          # 将指令存入imem
 
    if first:                           # PC寄存器初始化
 
        pReg = 0
 
        first = False
 
 
 
 
def cycle():
 
    global pReg, iReg, reg, imem, dmem, Stat, ZF, SF, OF
 
 
 
    # 取指令
 
    iReg = imem[pReg]     # 指令寄存器
 
    pReg = pReg + 1       # PC寄存器
 
   
 
 
 
    # 译码、执行和写结果
 
    instr = list(iReg)                      # 指令（列表）
 
    opcode = int(instr[0],16)
 
    opfunction = int(instr[1],16)
 
    if opcode == 0 :                        #halt
 
        Stat = 2
 
    elif opcode == 1 :                      #nop
 
        1==1
 
    elif opcode == 9 :                      #ret
 
        pReg = dmem[reg[4]]
 
        reg[4] = reg[4] + 8
 
    else:
 
        rA = int(instr[2],16)
 
        rB = int(instr[3],16)
 
        if   opcode == 2 :                  #rrmovq rA,rB
 
            reg[rB] = reg[rA]
 
        elif opcode == 3 :                  #irmovq V,rB
 
            V = int(get_V_D(instr),16)
 
            reg[rB] = V
 
        elif opcode == 4 :                  #rmmovq rA,D(rB)
 
            D = int(get_V_D(instr),16)
 
            dmem[D+rB] = reg[rA]
 
        elif opcode == 5 :                  #mrmovq D(rB),rA
 
            D = int(get_V_D(instr),16)
 
            reg[rA] = dmem[D+rB]
 
        elif opcode == 6 :                  #OPq rA,rB
 
            fn = opfunction
 
            if   (fn==0):                       #addq
 
                reg[rB] = reg[rB] + reg[rA]
 
            elif (fn==1):                       #subq
 
                reg[rB] = reg[rB] - reg[rA]
 
            elif (fn==2):                       #andq
 
                reg[rB] = reg[rB] & reg[rA]
 
            elif (fn==3):                       #xorq
 
                reg[rB] = reg[rB] ^ reg[rA]
 
            #set CC
 
            ZF = 1 if reg[rB] == 0 else 0
 
            SF = 1 if reg[rB] < 0 else 0
 
           
 
        elif opcode == 7:                   #jXX
 
            fn = opfunction
 
            Dest = int(get_Dest(instr),16)
 
            if   (fn==0):                       #jmp
 
                pReg = Dest
 
            elif (fn==1):                       #jle
 
                if (SF^OF)|ZF:
 
                    pReg = Dest
 
            elif (fn==2):                       #jl
 
                if SF^OF:
 
                    pReg = Dest
 
            elif (fn==3):                       #je
 
                if ZF==1:
 
                    pReg = Dest
 
            elif (fn==4):                       #jne
 
                if ZF==0:
 
                    pReg = Dest
 
            elif (fn==5):                       #jge
 
                if (SF^OF)==0:
 
                    pReg = Dest
 
            elif (fn==6):                       #jg
 
                if ((SF^OF)==0)&(ZF==0):
 
                    pReg = Dest
 
        elif opcode == 8:                   #call Dest
 
            Dest = int(get_Dest(instr),16)
 
            pReg = Dest
 
            reg[4] = reg[4] - 8
 
            dmem[reg[4]] = reg[rA]
 
        elif opcode == 10:                  #pushq rA
 
            reg[4] = reg[4] - 8
 
            dmem[reg[4]] = reg[rA]
 
        elif opcode == 11:                  #popq rA
 
            reg[rA] = dmem[reg[4]]
 
            reg[4] = reg[4] + 8
 
        elif opcode == 15:
 
            print("output:",reg[rA])
 
        else:
 
            Stat = 4
 
 
 
    return True
 
 
 #运行
def run(file):
 
    global pReg, iReg, reg, imem, dmem, Stat, ZF, SF, OF
 
    loadProgram(file)
 
 
 
    while True:
 
        cycle()
 
        if Stat == 2:
 
            print("HALT")
 
            break
 
        if Stat == 4:
 
            print("INS")
 
            break
 


run('Toy计算机\model_test.COE')