imem = "" #指令存储器
dmem = ""
reg = [0]*15 #15个常用存储器
#instruction
iReg = 0 
#PC
pReg = 0 
#STAT
Stat=1
#CC
ZF=0 
SF=0 
OF=0 


#加载
"""
def loadProgram(file):
    global mem, reg, iReg, pReg, ZF, SF, OF, Stat
    fil = open(file,'r')
    first = True
    lineno = 0
    line1=fil.readline()
    line2=fil.readline() #跳过前两行
    lines = fil.readlines()
    for line in lines:
        mem+=line.strip('\n') 
    mem=(mem.replace(",",""))[:-2]
    print(mem)
    fil.close()
    """
def loadProgram(file):
    global pReg, iReg, reg, imem, dmem, Stat, ZF, SF, OF
    fil = open(file, 'r')
    instruc = ""
    first = True
    line_count = 0
    line1=fil.readline()
    line2=fil.readline() #跳过前两行
    for line in fil:
        line = line.strip()

        for char in line:
            if char == ',' or ';':
                instruc += " "
            else:
                instruc += char

            if len(instruc.split()) == 2:
                imem.append(instruc)
                instruc = ""

                if first:
                    pReg = line_count
                    first = False

                line_count += 1

    fil.close()
    

file="Toy计算机\model_test.COE"
loadProgram(file)