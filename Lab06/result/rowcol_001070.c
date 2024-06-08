/**************************************************************************
	行/列求和函数。按下面的要求编辑此文件：
	1. 将你的学号、姓名，以注释的方式写到下面；
	2. 实现不同版本的行列求和函数；
	3. 编辑rc_fun_rec rc_fun_tab数组，将你的最好的答案
		（最好的行和列求和、最好的列求和）作为数组的前两项
***************************************************************************/
   
/*
	学号：201209054233
	姓名：夜半加班狂
*/


#include  <stdio.h>
#include  <stdlib.h>
#include  "rowcol.h"
#include  <math.h>

/* 参考的列求和函数实现 */
/* 计算矩阵中的每一列的和。请注意对于行和列求和来说，调用参数是
	一样的，只是第2个参数不会用到而已
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

/* 参考的列和行求和函数实现 */
/* 计算矩阵中的每一行、每一列的和。 */

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
	这个表格包含多个数组元素，每一组元素（函数名字, COL/ROWCOL, "描述字符串"）
	COL表示该函数仅仅计算每一列的和
	ROWCOL表示该函数计算每一行、每一列的和
	将你认为最好的两个实现，放在最前面。
	比如：
	{my_c_sum1, "超级垃圾列求和实现"},
	{my_rc_sum2, "好一点的行列求和实现"},
*/

rc_fun_rec rc_fun_tab[] = 
{

  /* 第一项，应当是你写的最好列求和的函数实现 */
    {c_sum_10, COL, "Best column sum"},
  /* 第二项，应当是你写的最好行列求和的函数实现 */
    {rc_sum_10, ROWCOL, "Best row and column sum"},

    {c_sum, COL, "Column sum, reference implementation"},

    {rc_sum, ROWCOL, "Row and column sum, reference implementation"},

 /* 下面的代码不能修改或者删除！！表明数组列表结束 */
    {NULL,ROWCOL,NULL}
};
