/*
 * CS:APP Data Lab 
 * 
 * btest.c - A test harness that checks a student's solution 
 *           in bits.c for correctness. 
 *
 * Copyright (c) 2001, R. Bryant and D. O'Hallaron, All rights reserved.
 * May not be used, modified, or copied without permission.
 *
 * Usage:
 *    -e <N>       ���ƶ�ĳ�������ı�����ĿΪN��ȱʡ�ò��������������ƣ�
 *    -f <Name>    ����ض����ֵĺ���
 *    -g           ��ӡ����Ҫ��Ϣ(��ͬ�� -v 0 �� -e 0)
 *    -h           ��ʾ����Ϣ
 *    -a           ��������
 *    -r <N>       ����������������ͬ��Ȩ��N
 *    -v <N>       ������ʾ����Ϊ N
 *                 N=0:  �����������յ÷�
 *                 N=1:  N=1 ���⻹��������÷֣�ȱʡֵ��
 * 
 * Each problem has a weight 1 to 4, which is defined in legallist.c.

 */
#include <stdio.h>
//#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include "getopt.h"
#include "btest.h"

/* Globals defined in other modules */
extern team_struct team;    /* defined in bits.c */
extern test_rec test_set[]; /* defined in decl.c */
                            /* and generated from templates in ./puzzles */

/* Generate test values near "corner cases" */
#define TEST_RANGE 5
#define TEST_COUNT 33

/* Print only compact grading summary if set (-g) */
static int grade = 0;

/* Max errors reported per function (-e) */
static int error_limit = 1000;

/* If non-NULL, test only one function (-f) */
static char* test_fname = NULL;  

/* Should I used fixed weight for rating, and if so, what should it be? (-r)*/
static int global_rating = 0;

/* Return random value between min and max */
static int random_val(int min, int max)
{
    double weight = rand()/(double) RAND_MAX;
    int result = min * (1-weight) + max * weight;
    return result;
}

/* Generate the integer values we'll use to test a function */
static int gen_vals(int test_vals[], int min, int max)
{
    int i;
    int test_count = 0;

    /* If range small enough, then do exhaustively */
    if (max-32 <= min) {
	for (i = min; i <= max; i++)
	    test_vals[test_count++] = i;
	return test_count;
    }
    /* Otherwise, need to sample.
       Do so near the boundaries and for a few random cases */
    for (i = 0; i < TEST_RANGE; i++) {
	test_vals[test_count++] = min+i;
	test_vals[test_count++] = max-i;
	test_vals[test_count++] = (max+min-TEST_RANGE)/2+i;
	test_vals[test_count++] = random_val(min, max);
    }
    return test_count;
}

/* Test a function with zero arguments */
static int test_0_arg(funct_t f, funct_t ft, char *name, int report)
{
    int r = f();
    int rt = ft();
    int error =  (r != rt);

    if (error && report)
	printf("���� %s() ʧ�ܣ�\n  ����õ� %d[0x%x].  Ӧ���� %d[0x%x]\n",
	       name, r, r, rt, rt);
    return error;
}

/* Test a function with one argument */
static int test_1_arg(funct_t f, funct_t ft, int arg1, char *name, int report)
{
    funct1_t f1 = (funct1_t) f;
    funct1_t f1t = (funct1_t) ft;
    int r, rt, error;

    r = f1(arg1);
    rt = f1t(arg1);
    error = (r != rt);
    if (error && report)
	printf("���� %s(%d[0x%x]) ʧ�ܣ�\n  ����õ� %d[0x%x].  Ӧ���� %d[0x%x]\n",
	       name, arg1, arg1, r, r, rt, rt);
    return error;
}

/* Test a function with two arguments */
static int test_2_arg(funct_t f, funct_t ft, 
		      int arg1, int arg2, 
		      char *name, int report)
{
    funct2_t f2 = (funct2_t) f;
    funct2_t f2t = (funct2_t) ft;
    int r = f2(arg1, arg2);
    int rt = f2t(arg1, arg2);
    int error = (r != rt);

    if (error && report)
	printf(
	       "���� %s(%d[0x%x],%d[0x%x]) ʧ�ܣ�\n  ����õ� %d[0x%x].  Ӧ���� %d[0x%x]\n",
	       name, arg1, arg1, arg2, arg2, r, r, rt, rt);
    return error;
}

/* Test a function with three arguments */
static int test_3_arg(funct_t f, funct_t ft,
	   int arg1, int arg2, int arg3,
	   char *name, int report)
{
    funct3_t f3 = (funct3_t) f;
    funct3_t f3t = (funct3_t) ft;
    int r = f3(arg1, arg2, arg3);
    int rt = f3t(arg1, arg2, arg3);
    int error = (r != rt);

    if (error && report)
	printf(
	       "���� %s(%d[0x%x],%d[0x%x],%d[0x%x]) ʧ�ܣ�\n  ����õ� %d[0x%x].  Ӧ���� %d[0x%x]\n",
	       name, arg1, arg1, arg2, arg2, arg3, arg3, r, r, rt, rt);
    return error;
}

/* Test a function.  Return number of errors */
static int test_function(test_ptr t, int report) {
    int test_vals[3][TEST_COUNT];
    int test_counts[3];
    int errors = 0;
    int i;
    int a1, a2, a3;
    int args = t->args;

    /* Create test set */
    for (i = 0; i < 3; i++)
	test_counts[i] =
	    gen_vals(test_vals[i], t->arg_ranges[i][0], t->arg_ranges[i][1]);
    if (args == 0) {
	errors += test_0_arg(t->solution_funct, t->test_funct,
			     t->name, report && errors < error_limit);
    } else for (a1 = 0; a1 < test_counts[0]; a1++) {
	if (args == 1) {
	    errors += test_1_arg(t->solution_funct, t->test_funct,
				 test_vals[0][a1],
				 t->name, report && errors < error_limit);
	} else for (a2 = 0; a2 < test_counts[1]; a2++) {
	    if (args == 2) {
		errors += test_2_arg(t->solution_funct, t->test_funct,
				     test_vals[0][a1], test_vals[1][a2],
				     t->name, report && errors < error_limit);
	    } else for (a3 = 0; a3 < test_counts[2]; a3++) {
		errors += test_3_arg(t->solution_funct, t->test_funct,
				     test_vals[0][a1], test_vals[1][a2],
				     test_vals[2][a3],
				     t->name, report && errors < error_limit);
	    }
	}
    }

    if (!grade) {
	if (report && errors > error_limit)
	    printf("���� %s �ܹ���%d������\n",
		   t->name,errors);
    }

    return errors;
}

/* Run series of tests.  Return number of errors */ 
static int run_tests(int report) {
    int i;
    int errors = 0;
    double points = 0.0;
    double max_points = 0.0;

    if (grade)
	printf("�÷�\t������\t����\n");

    for (i = 0; test_set[i].solution_funct; i++) {
	int terrors;
	double tscore;
	double tpoints;
	if (!test_fname || strcmp(test_set[i].name,test_fname) == 0) {
	    int rating = global_rating ? global_rating : test_set[i].rating;
	    terrors = test_function(&test_set[i], report);
	    errors += terrors;
	    if (test_set[i].args == 0)
		tscore = terrors == 0 ? 1.0 : 0.0;
	    else
		tscore = terrors == 0 ? 1.0 : terrors == 1 ? 0.5 : 0.0;
	    tpoints = rating * tscore;
	    points += tpoints;
	    max_points += rating;
	    if (grade)
		printf(" %.1f\t%d\t%s\n", tpoints, terrors, test_set[i].name);
	    if (report)
		printf("���� %s �÷�: %.2f/%.2f\n",
		       test_set[i].name, tpoints, (double) rating);
	}
    }

    if (grade) 
	printf("�ܷ֣� %.2f/%.2f\n", points, max_points);
    else
	printf("�ܹ���ȷ�÷֣� %.2f/%.2f\n", points, max_points);

    return errors;
}

static void usage(char *cmd) {
    printf("Usage: %s [-v 0|1] [-hag] [-f <func name>] [-e <max errors>]\n", cmd);
    printf("  -e <n>    ���ƶ�ĳ�������ı�����ĿΪn��ȱʡ�ò��������������ƣ�\n");
    printf("  -f <name> ����ض����ֵĺ���\n");
    printf("  -g        ��ӡ����Ҫ��Ϣ(��ͬ�� -v 0 �� -e 0)\n");
    printf("  -h        ��ʾ����Ϣ\n");
    printf("  -a        ��������\n");
    printf("  -r <n>    ����������������ͬ��Ȩ��n\n");
    printf("  -v <n>    ������ʾ����Ϊ n\n");
    printf("               n=0: �����������յ÷�\n");
    printf("               n=1: ���⻹��������÷֣�ȱʡֵ��\n");
    exit(1);
}


/* 
 * main routine 
 */

int main(int argc, char *argv[])
{
    int verbose_level = 1;
    int errors;
    int team_check = 1;
    char c;

    /* parse command line args */
    while ((c = getopt(argc, argv, "hagv:f:e:r:")) != -1)
        switch (c) {
        case 'h': /* help */
	    usage(argv[0]);
	    break;
        case 'a': /* Don't check team structure */
	    team_check = 0;
	    break;
        case 'g': /* grading summary */
	    grade = 1;
	    break;
        case 'v': /* set verbosity level */
	    verbose_level = atoi(optarg);
	    if (verbose_level < 0 || verbose_level > 1)
		usage(argv[0]);
	    break;
	case 'f': /* test only one function */
	    test_fname = _strdup(optarg);
	    break;
	case 'e': /* set error limit */
	    error_limit = atoi(optarg);
	    if (error_limit < 0)
		usage(argv[0]);
	    break;
	case 'r': /* set global rating for each problem */
	    global_rating = atoi(optarg);
	    if (global_rating < 0)
		usage(argv[0]);
	    break;
	default:
	    usage(argv[0]);
    }

    if (grade) {
	error_limit = 0;
	verbose_level = 0;
    }

    if (team_check) {
	if ((*team.name1 == '\0') || (*team.id1 == '\0')) {
	    printf("%s: �����������д����С���Ա��\n", argv[0]);
	    exit(1);
	} 
	else
	    printf("ѧ����\t%s\t%s\n", team.name1, team.id1);

	printf("\n");
    }

    /* test each function */
    errors = run_tests(verbose_level > 0);

    if (!grade) {
	if (errors > 0)
	    printf("����%d������\n", errors);
	else {
	    printf("�ɹ�ͨ�����в��ԣ�\n");
	}
    }

    return 0;
}