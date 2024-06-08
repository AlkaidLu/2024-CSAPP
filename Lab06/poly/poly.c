/**************************************************************************
	����ʽ���㺯�����������Ҫ��༭���ļ���
	1. �����ѧ�š���������ע�͵ķ�ʽд�����棻
	2. ʵ�ֲ�ͬ�汾�Ķ���ʽ���㺯����
	3. �༭peval_fun_rec peval_fun_tab���飬�������õĴ�
		����СCPE����СC10����Ϊ�����ǰ����
***************************************************************************/
   
/*
	ѧ�ţ�201209054233
	������ҹ��Ӱ��
*/



#include  <stdio.h>
#include  <stdlib.h>

typedef int (*peval_fun)(int*, int, int);

typedef struct {
  peval_fun f;
  char *descr;
} peval_fun_rec, *peval_fun_ptr;


/**************************************************************************
 Edit this comment to indicate your name and Andrew ID
#ifdef ASSIGN
   Submission by Harry Q. Bovik, bovik@andrew.cmu.edu
#else
   Instructor's version.
   Created by Randal E. Bryant, Randy.Bryant@cs.cmu.edu, 10/07/02
#endif
***************************************************************************/

/*
	ʵ��һ��ָ���ĳ�ϵ������ʽ���� result=48+28*x+27*x^2+41*x^3
	��һ�Σ���ֱ�����г����Ա��֪����Ҫʵ�ֵĳ�ϵ����ɶ
*/
int const_poly_eval(int *not_use, int not_use2, int x)
{
	
	register int x2 = x * x;
	register int p2 = (x2 << 5) - (x2 << 2)  - x2;
	register int x3 = x2 * x;
	register int p3 = (x3 << 5) + (x3 << 3) + x3;
	return 48 + (x << 5) - (x << 2) + p2 + p3;
	
	//return 48 + (28 + (27 + 41 * x) * x) * x;
}



/* ����ʽ���㺯����ע�⣺���ֻ��һ���ο�ʵ�֣�����Ҫʵ���Լ��İ汾 */

/*
	������ʾ��lcc֧��ATT��ʽ��Ƕ��ʽ��࣬����
	
	_asm("movl %eax,%ebx");
	_asm("pushl %edx");
	
	������lcc��project->configuration->Compiler->Code Generation->Generate .asm��
	����ѡ�к󣬿�����lccĿ¼�������ɶ�Ӧ����Ļ�����ʵ�֡�ͨ���鿴����ļ���
	������˽�����������ʵ����Ĵ���ġ���Щʵ�ֿ��ܷǳ���Ч��
	��������ʵ��ĵط�����Ƕ��ʽ��࣬���������߼������ܡ�
*/

int poly_eval(int *a, int degree, int x)
{
    int result = 0;
    int i;
    int xpwr = 1; /* x���ݴ� */
//    printf("��=%d\n",degree);
    for (i = 0; i <= degree; i++) {
	result += a[i]*xpwr;
	xpwr   *= x;
    }
    return result;
}

int poly_eval_1(int* a, int degree, int x)
{
	register int result1 = 0;
	register int result2 = 0;
	register int i;
	register int xpwr = 1; /* x���ݴ� */
//    printf("��=%d\n",degree);
	for (i = 0; i < degree-1; i+=2) {
		result1 += a[i] * xpwr;
		result2 += a[i + 1] * xpwr * x;
		xpwr *= x*x;
	}
	result1 += result2;
	for (; i <= degree; i ++) {
		result1 += a[i] * xpwr;
		xpwr *= x;
	}
	return result1;
}
int poly_eval_2(int* a, int degree, int x)
{
	int result1 = 0;
	int result2 = 0;
	int result3 = 0;
	int i;
	int xpwr = 1; /* x���ݴ� */
//    printf("��=%d\n",degree);
	if (degree == 9) {
		return ((((((((a[9] * x + a[8]) * x + a[7]) * x + a[6]) * x + a[5]) * x + a[4]) * x + a[3]) * x + a[2]) * x + a[1]) * x + a[0];
	}
	for (i = 0; i < degree - 2; i += 3) {
		result1 += a[i] * xpwr;
		result2 += a[i + 1] * xpwr * x;
		result3 += a[i + 2] * xpwr * x * x;
		xpwr *= x * x * x;
	}
	result1 += result2;
	result1 += result3;
	for (; i <= degree; i++) {
		result1 += a[i] * xpwr;
		xpwr *= x;
	}
	return result1;
}
int poly_eval_3(int* a, int degree, int x)
{
	register int result1 = 0;
	register int i;
	register int xpwr = 1; /* x���ݴ� */
//    printf("��=%d\n",degree);
	for (i = 0; i < degree - 1; i += 2) {
		result1 += (a[i] + a[i + 1] * x) * xpwr;
		xpwr *= x * x;
	}
	for (; i <= degree; i++) {
		result1 += a[i] * xpwr;
		xpwr *= x;
	}
	return result1;
}



/*
	����������������Ԫ�أ�ÿһ��Ԫ�أ���������, "�����ַ���"��
	������Ϊ��õ�����ʵ�֣�������ǰ�档
	���磺
	{my_poly_eval1, "��������ʵ��"},
	{my_poly_eval2, "��һ���ʵ��"},
*/
   
peval_fun_rec peval_fun_tab[] = 
{

  /* ��һ�Ӧ������д�����CPE�ĺ���ʵ�� */
 {poly_eval_3, "ҹ��Ӱ���CPE"},
  /* �ڶ��Ӧ������д����10��ʱ����������ܵ�ʵ�� */
 {poly_eval_2, "ҹ��Ӱ���10��ʵ��"},

 {poly_eval, "poly_eval: �ο�ʵ��"},

 /* ����Ĵ��벻���޸Ļ���ɾ���������������б���� */
 {NULL, ""}
};







