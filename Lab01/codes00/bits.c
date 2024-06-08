/*
 * CS:APP Data Lab
 *
 * bits.c - ��ʵ������Ҫ�޸ı��ļ���
 *          ����Ҫ�Ѵ��ļ��ύ����ʦ/�̸���
 *
 */

#include "btest.h"
#include <limits.h>

/*
 * ѧ����ʵ��ָ����
 *
 * ����1����������֡�ѧ����д������Ľṹ�У�
 */
team_struct team =
{
   /* Replace this with your full name */
   "������",

    /* Replace this with your Andrew login ID */
   "202202001070"
};

#if 0
/*
 * ����2������ϸ�Ķ������˵����
 */

�������ʵ�飬��ͨ���༭���ļ���bits.c���еĸ�������������ύ�����

���Ҫ��

	����return������滻��һ�л��߶���ʵ�ֺ������ܵ�C���롣
	������/�޸ĵĴ�������������ķ��

  int Funct(arg1, arg2, ...) {
      /* ��Ҫ�����������ʵ�ָú������ܵ� */
      int var1 = Expr1;
      ...
      int varM = ExprM;

      varJ = ExprJ;
      ...
      varN = ExprN;
      return ExprR;
  }

	ÿһ����Expr����һ������ʹ���������ݵı���ʽ��
  1. 0~255��0xFF��������������������ʹ�ô�ĳ���������0xffffffff��
  2. �����Ĳ����;ֲ�������������ʹ��ȫ�ֱ�������
  3. ��Լ�����������" ! ~ "
  4. ����������������� & ^ | + << >> ��

	һЩ������ʵ�֣��������ܹ�ʹ�õ��������
	ÿһ����Expr������ʽ���ܰ�������������û��Ҫ�������ÿ��ֻ��һ���������

	��������ϸ��ֹ��
  1. ʹ���κο��������䣬����if, do, while, for, switch, �ȵȡ�
  2. �������ʹ���κκꡣ
  3. �ڴ��ļ������������κζ���ĺ�����
  4. �����κκ�����
  5. ʹ���κ������������������ &&, ||, -, ���� ?:
  6. ʹ���κ���ʽ��ǿ������ת����

  ����Ҫ������ĵ��ԣ�
  1. ʹ��2�Ĳ��룬32λ��ʾ��������
  2. ���������������������㡣
  3. �����λ�����������ֳ�����ô�����������Ԥ��Ľ����

���Խ��ܵı�̷���ʾ����
  /*
   * pow2plus1 - returns 2^x + 1, where 0 <= x <= 31
   */
  int pow2plus1(int x) {
     /* exploit ability of shifts to compute powers of 2 */
     return (1 << x) + 1;
  }

  /*
   * pow2plus4 - returns 2^x + 4, where 0 <= x <= 31
   */
  int pow2plus4(int x) {
     /* exploit ability of shifts to compute powers of 2 */
     int result = (1 << x);
     result += 4;
     return result;
  }


ע�⣺
  1. ����ʹ��dlc.exe������bits.c�Ƿ���ϱ��Ҫ��
  2. Each function has a maximum number of operators (! ~ & ^ | + << >>)
     that you are allowed to use for your implementation of the function.
     The max operator count is checked by dlc. Note that '=' is not
     counted; you may use as many of these as you want without penalty.
  3. ʹ��btest�����ĺ����Ƿ�����ȷ��
  4. The maximum number of ops for each function is given in the
     header comment for each function. If there are any inconsistencies
     between the maximum ops in the writeup and in this file, consider
     this file the authoritative source.
#endif

/*
 * ����3: ��������ı��Ҫ���޸�����ĺ�����
 *
 *   ��Ҫ��Ϊ�˱���ܲ�ĳɼ���
 *   1. ʹ��dlc.exe�����ı�̷���Ƿ����Ҫ��
 *   2. ʹ��btest�����ĺ����Ƿ�����ȷ����ע����Tmin��Tmax�����������Ƿ���ȷ��
 */



int bitAnd(int x, int y) {

  int r= ~(~x|~y);
  return r;

}






/*
 * bitXor - x^y using only ~ and &
 *   Example: bitXor(4, 5) = 1
 *   Legal ops: ~ &
 *   Max ops: 14
 *   Rating: 2
 */
int bitXor(int x, int y) {
  
  int r=~((~(x&~y))&(~(~x&y)));
  return r;

}






/*
 * evenBits - return word with all even-numbered bits set to 1
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 8
 *   Rating: 2
 */
int evenBits(void) {
  int a=0x55;
  int b=a|a<<8;
  int c=b|b<<16;
  return c;

}


/*
 * isLess - if x < y  then return 1, else return 0
 *   Example: isLess(4,5) = 1.
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 24
 *   Rating: 3
 */

int isLess(int x, int y) {
    int sign_x = (x >> 31) & 0x1; // Get the sign bit of x
    int sign_y = (y >> 31) & 0x1; // Get the sign bit of y
    int sign_diff = sign_x ^ sign_y; // Check if signs are different
    
    int diff = x + (~y + 1); // Calculate x-y
    
    // If signs are different, return 1 if x is negative, 0 otherwise
    // If signs are the same, return 1 if diff is negative(1), 0 otherwise
    return ((sign_diff & sign_x) | (~sign_diff & (diff >> 31) & 1));
}








/*
 * sm2tc - Convert from sign-magnitude to two's complement
 *   where the MSB is the sign bit
 *   Example: sm2tc(0x80000005) = -5.
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 15
 *   Rating: 4
 */
/*
int sm2tc(int x) {
  int sign=(x>>31)&0x1;
  int mask=sign<<31>>31;//����Ϊ0xFFFFFFFF,����Ϊ0x00000000
  int absx=(x^mask)+(~mask+1);//������λȡ��������Ϊԭ����ֵ;������1��������0;�õ�����;
  //����Ǹ����������λҲ��Ϊ0�ˣ���Ҫ�����������û��ϵ
  return absx;
}*/
/*return (sign & (~(x ^ (x & (1 << 31))) + 1)) | (~sign & x);  */

int sm2tc(int x) {
  int sign = x >> 31;  
  return (x ^ sign) + (((1 << 31) + 1) & sign);
}