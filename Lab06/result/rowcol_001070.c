/**************************************************************************
	��/����ͺ������������Ҫ��༭���ļ���
	1. �����ѧ�š���������ע�͵ķ�ʽд�����棻
	2. ʵ�ֲ�ͬ�汾��������ͺ�����
	3. �༭rc_fun_rec rc_fun_tab���飬�������õĴ�
		����õ��к�����͡���õ�����ͣ���Ϊ�����ǰ����
***************************************************************************/
   
/*
	ѧ�ţ�201209054233
	������ҹ��Ӱ��
*/


#include  <stdio.h>
#include  <stdlib.h>
#include  "rowcol.h"
#include  <math.h>

/* �ο�������ͺ���ʵ�� */
/* ��������е�ÿһ�еĺ͡���ע������к��������˵�����ò�����
	һ���ģ�ֻ�ǵ�2�����������õ�����
*/

void c_sum(matrix_t M, vector_t rowsum, vector_t colsum)
{
    int i,j;
    for (j = 0; j < N; j++) {
	colsum[j] = 0;
	for (i = 0; i < N; i++)
	    colsum[j] += M[i][j];
    }
}

void c_sum_10(matrix_t M, vector_t rowsum, vector_t colsum)
{
	int i, j, m;
	int sum1, sum2, sum3, sum4, sum5, sum6, sum7, sum8, sum9, sum10;
	for (j = 0; j < N ; j ++) {
		sum1 = sum2 = sum3 = sum4 = sum5 = sum6 = sum7 = sum8 = sum9 = sum10 = 0;
		for (i = 0; i < N-9; i+=10) {
			sum1 += M[i][j];
			sum2 += M[i + 1][j];
			sum3 += M[i + 2][j];
			sum4 += M[i + 3][j];
			sum5 += M[i + 4][j];
			sum6 += M[i + 5][j];
			sum7 += M[i + 6][j];
			sum8 += M[i + 7][j];
			sum9 += M[i + 8][j];
			sum10 += M[i + 9][j];

		}
		for (; i < N; i++) {
			sum1 += M[i][j];

		}
		colsum[j] = sum1 + sum2 + sum3 + sum4 + sum5 + sum6 + sum7 + sum8 + sum9 + sum10;
	}

}

/* �ο����к�����ͺ���ʵ�� */
/* ��������е�ÿһ�С�ÿһ�еĺ͡� */

void rc_sum(matrix_t M, vector_t rowsum, vector_t colsum)
{
    int i,j;
    for (i = 0; i < N; i++) {
	rowsum[i] = colsum[i] = 0;
	for (j = 0; j < N; j++) {
	    rowsum[i] += M[i][j];
	    colsum[i] += M[j][i];
	}
    }
}

void rc_sum_10(matrix_t M, vector_t rowsum, vector_t colsum)
{

	int i, j, m;
	int sum1, sum2, sum3, sum4, sum5, sum6, sum7, sum8, sum9, sum10;
	for (j = 0; j < N; j++) {
		sum1 = sum2 = sum3 = sum4 = sum5 = sum6 = sum7 = sum8 = sum9 = sum10 = 0;
		for (i = 0; i < N - 9; i+=10) {
			sum1 += M[i][j];
			sum2 += M[i + 1][j];
			sum3 += M[i + 2][j];
			sum4 += M[i + 3][j];
			sum5 += M[i + 4][j];
			sum6 += M[i + 5][j];
			sum7 += M[i + 6][j];
			sum8 += M[i + 7][j];
			sum9 += M[i + 8][j];
			sum10 += M[i + 9][j];

		}
		for (; i < N; i++) {
			sum1 += M[i][j];

		}
		colsum[j] = sum1 + sum2 + sum3 + sum4 + sum5 + sum6 + sum7 + sum8 + sum9 + sum10;
	}
	for (i = 0; i < N; i++) {
		sum1 = sum2 = sum3 = sum4 = sum5 = sum6 = sum7 = sum8 = sum9 = sum10 = 0;
		for (j = 0; j < N - 9; j+=10) {
			sum1 += M[i][j];
			sum2 += M[i ][j+ 1];
			sum3 += M[i ][j+ 2];
			sum4 += M[i ][j+ 3];
			sum5 += M[i ][j+ 4];
			sum6 += M[i ][j+ 5];
			sum7 += M[i ][j+ 6];
			sum8 += M[i ][j+ 7];
			sum9 += M[i ][j+ 8];
			sum10 += M[i ][j+ 9];

		}
		for (; j < N; j++) {
			sum1 += M[i][j];

		}
		rowsum[i] = sum1 + sum2 + sum3 + sum4 + sum5 + sum6 + sum7 + sum8 + sum9 + sum10;
	}
}

/* 
	����������������Ԫ�أ�ÿһ��Ԫ�أ���������, COL/ROWCOL, "�����ַ���"��
	COL��ʾ�ú�����������ÿһ�еĺ�
	ROWCOL��ʾ�ú�������ÿһ�С�ÿһ�еĺ�
	������Ϊ��õ�����ʵ�֣�������ǰ�档
	���磺
	{my_c_sum1, "�������������ʵ��"},
	{my_rc_sum2, "��һ����������ʵ��"},
*/

rc_fun_rec rc_fun_tab[] = 
{

  /* ��һ�Ӧ������д���������͵ĺ���ʵ�� */
    {c_sum_10, COL, "Best column sum"},
  /* �ڶ��Ӧ������д�����������͵ĺ���ʵ�� */
    {rc_sum_10, ROWCOL, "Best row and column sum"},

    {c_sum, COL, "Column sum, reference implementation"},

    {rc_sum, ROWCOL, "Row and column sum, reference implementation"},

 /* ����Ĵ��벻���޸Ļ���ɾ���������������б���� */
    {NULL,ROWCOL,NULL}
};
