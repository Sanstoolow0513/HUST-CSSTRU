/*
 * trans.c - 矩阵转置B=A^T
 *每个转置函数都必须具有以下形式的原型：
 *void trans（int M，int N，int a[N][M]，int B[M][N]）；
 *通过计算，块大小为32字节的1KB直接映射缓存上的未命中数来计算转置函数。
 */
#include <stdio.h>
#include "cachelab.h"
int is_transpose(int M, int N, int A[N][M], int B[M][N]);
char transpose_submit_desc[] = "Transpose submission"; // 请不要修改“Transpose_submission”

void transpose_submit(int M, int N, int A[N][M], int B[M][N])
{
    int i, j, k, l;
    int t0, t1, t2, t3, t4, t5, t6, t7;

    if (M == 64 && N == 64)
    {
        for (l = 0; l < N / 8; l++)
        {
            for (k = 0; k < M / 8; k++)
            {
                // 第一阶段：处理前4行
                for (i = 0; i < 4; i++)
                {
                    t0 = A[l * 8 + i][k * 8];
                    t1 = A[l * 8 + i][k * 8 + 1];
                    t2 = A[l * 8 + i][k * 8 + 2];
                    t3 = A[l * 8 + i][k * 8 + 3];
                    t4 = A[l * 8 + i][k * 8 + 4];
                    t5 = A[l * 8 + i][k * 8 + 5];
                    t6 = A[l * 8 + i][k * 8 + 6];
                    t7 = A[l * 8 + i][k * 8 + 7];

                    B[k * 8][l * 8 + i] = t0;
                    B[k * 8 + 1][l * 8 + i] = t1;
                    B[k * 8 + 2][l * 8 + i] = t2;
                    B[k * 8 + 3][l * 8 + i] = t3;
                    B[k * 8][l * 8 + i + 4] = t4;
                    B[k * 8 + 1][l * 8 + i + 4] = t5;
                    B[k * 8 + 2][l * 8 + i + 4] = t6;
                    B[k * 8 + 3][l * 8 + i + 4] = t7;
                }

                // 第二阶段：处理后4行 + 数据交换
                for (i = 0; i < 4; i++)
                {
                    t0 = B[k * 8 + i][l * 8 + 4];
                    t1 = B[k * 8 + i][l * 8 + 5];
                    t2 = B[k * 8 + i][l * 8 + 6];
                    t3 = B[k * 8 + i][l * 8 + 7];
                    t4 = A[l * 8 + 4][k * 8 + i];
                    t5 = A[l * 8 + 5][k * 8 + i];
                    t6 = A[l * 8 + 6][k * 8 + i];
                    t7 = A[l * 8 + 7][k * 8 + i];

                    B[k * 8 + i][l * 8 + 4] = t4;
                    B[k * 8 + i][l * 8 + 5] = t5;
                    B[k * 8 + i][l * 8 + 6] = t6;
                    B[k * 8 + i][l * 8 + 7] = t7;

                    t4 = A[l * 8 + 4][k * 8 + i + 4];
                    t5 = A[l * 8 + 5][k * 8 + i + 4];
                    t6 = A[l * 8 + 6][k * 8 + i + 4];
                    t7 = A[l * 8 + 7][k * 8 + i + 4];

                    B[k * 8 + i + 4][l * 8] = t0;
                    B[k * 8 + i + 4][l * 8 + 1] = t1;
                    B[k * 8 + i + 4][l * 8 + 2] = t2;
                    B[k * 8 + i + 4][l * 8 + 3] = t3;
                    B[k * 8 + i + 4][l * 8 + 4] = t4;
                    B[k * 8 + i + 4][l * 8 + 5] = t5;
                    B[k * 8 + i + 4][l * 8 + 6] = t6;
                    B[k * 8 + i + 4][l * 8 + 7] = t7;
                }
            }

            // 处理未对齐的列
            for (j = k * 8; j < M; j++)
            {
                t0 = A[l * 8][j];
                t1 = A[l * 8 + 1][j];
                t2 = A[l * 8 + 2][j];
                t3 = A[l * 8 + 3][j];
                t4 = A[l * 8 + 4][j];
                t5 = A[l * 8 + 5][j];
                t6 = A[l * 8 + 6][j];
                t7 = A[l * 8 + 7][j];

                B[j][l * 8] = t0;
                B[j][l * 8 + 1] = t1;
                B[j][l * 8 + 2] = t2;
                B[j][l * 8 + 3] = t3;
                B[j][l * 8 + 4] = t4;
                B[j][l * 8 + 5] = t5;
                B[j][l * 8 + 6] = t6;
                B[j][l * 8 + 7] = t7;
            }
        }

        // 处理未对齐的行
        if (l * 8 < N)
        {
            for (j = 0; j < M; j++)
            {
                for (i = l * 8; i < N; i++)
                {
                    B[j][i] = A[i][j];
                }
            }
        }
    }
    else if (M == 32 && N == 32)
    {
        // 原有 32x32 优化逻辑
        for (int i = 0; i < 32; i += 8)
        {
            for (int j = 0; j < 32; j += 8)
            {
                for (int k = 0; k < 8; k++)
                {
                    int r = i + k;
                    t0 = A[r][j];
                    t1 = A[r][j + 1];
                    t2 = A[r][j + 2];
                    t3 = A[r][j + 3];
                    t4 = A[r][j + 4];
                    t5 = A[r][j + 5];
                    t6 = A[r][j + 6];
                    t7 = A[r][j + 7];
                    B[j][r] = t0;
                    B[j + 1][r] = t1;
                    B[j + 2][r] = t2;
                    B[j + 3][r] = t3;
                    B[j + 4][r] = t4;
                    B[j + 5][r] = t5;
                    B[j + 6][r] = t6;
                    B[j + 7][r] = t7;
                }
            }
        }
    }
    else if (M == 61 && N == 67)
    {
        // 原有 61x67 优化逻辑
        int block_size = 16;
        for (i = 0; i < N; i += block_size)
        {
            for (j = 0; j < M; j += block_size)
            {
                int end_i = (i + block_size < N) ? i + block_size : N;
                int end_j = (j + block_size < M) ? j + block_size : M;
                for (k = i; k < end_i; k++)
                {
                    for (int l = j; l < end_j; l++)
                    {
                        B[l][k] = A[k][l];
                    }
                }
            }
        }
    }
}

/*
 * 我们在下面定义了一个简单的方法来帮助您开始，您可以根据下面的例子把上面值置补充完整。
 */

/*
 * 简单的基线转置功能，未针对缓存进行优化。
 */
char trans_desc[] = "Simple row-wise scan transpose";
void trans(int M, int N, int A[N][M], int B[M][N])
{
    int i, j, tmp;

    for (i = 0; i < N; i++)
    {
        for (j = 0; j < M; j++)
        {
            tmp = A[i][j];
            B[j][i] = tmp;
        }
    }
}

/*
 * registerFunctions-此函数向驱动程序注册转置函数。
 *在运行时，驱动程序将评估每个注册的函数并总结它们的性能。这是一种试验不同转置策略的简便方法。
 */
void registerFunctions()
{
    /* 注册解决方案函数  */
    registerTransFunction(transpose_submit, transpose_submit_desc);

    /* 注册任何附加转置函数 */
    registerTransFunction(trans, trans_desc);
}

/*
 * is_transpose - 函数检查B是否是A的转置。在从转置函数返回之前，可以通过调用它来检查转置的正确性。
 */
int is_transpose(int M, int N, int A[N][M], int B[M][N])
{
    int i, j;

    for (i = 0; i < N; i++)
    {
        for (j = 0; j < M; ++j)
        {
            if (A[i][j] != B[j][i])
            {
                return 0;
            }
        }
    }
    return 1;
}
