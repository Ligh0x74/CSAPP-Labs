/* 
 * trans.c - Matrix transpose B = A^T
 *
 * Each transpose function must have a prototype of the form:
 * void trans(int M, int N, int A[N][M], int B[M][N]);
 *
 * A transpose function is evaluated by counting the number of misses
 * on a 1KB direct mapped cache with a block size of 32 bytes.
 */ 
#include <stdio.h>
#include "cachelab.h"

int is_transpose(int M, int N, int A[N][M], int B[M][N]);

/* 
 * transpose_submit - This is the solution transpose function that you
 *     will be graded on for Part B of the assignment. Do not change
 *     the description string "Transpose submission", as the driver
 *     searches for that string to identify the transpose function to
 *     be graded. 
 */
char transpose_submit_desc[] = "Transpose submission";

// 简单优化的32×32矩阵转置
void transpose_32_32_demo(int A[32][32], int B[32][32]) {
    int x, y, z, flag = 0;
    for (int i = 0; i < 32; i += 8) {
        for (int j = 0; j < 32; j += 8) {
            // 转置8×8的矩阵
            for (int p = 0; p < 8; p++) {
                for (int q = 0; q < 8; q++) {
                    x = i + p;
                    y = j + q;
                    // 对角线上的元素最后进行转置
                    if (x == y && (flag = 1)) {
                        z = x;
                        continue;
                    }
                    B[y][x] = A[x][y];
                }
                // 转置对角线上的元素
                if (flag) {
                    flag = 0;
                    B[z][z] = A[z][z];
                }
            }
        }
    }
}

// 简单优化的64×64矩阵转置
void transpose_64_64_demo(int A[64][64], int B[64][64]) {
    int x, y, n, m, flag = 0;
    for (int i = 0; i < 64; i += 8) {
        for (int j = 0; j < 64; j += 8) {
            // 转置4个4×4的矩阵
            for (int k = 0; k < 4; k++) {
                for (int p = 0; p < 4; p++) {
                    for (int q = 0; q < 4; q++) {
                        x = i + p + ((k == 0 || k == 2) ? 0 : 4);
                        y = j + q + ((k == 0 || k == 1) ? 0 : 4);
                        // 对角线上的元素最后进行转置
                        if (i == j && p == q && (flag = 1)) {
                            n = x;
                            m = y;
                            continue;
                        }
                        B[y][x] = A[x][y];
                    }
                    // 转置对角线上的元素
                    if (flag) {
                        flag = 0;
                        B[m][n] = A[n][m];
                    }
                }
            }
        }
    }
}

// 最终版本的32×32矩阵转置
void transpose_32_32(int A[32][32], int B[32][32]) {
    int i, j, x, y;
    int v1, v2, v3, v4, v5, v6, v7, v8;
    for (i = 0; i < 32; i += 8) {
        for (j = 0; j < 32; j += 8) {
            if (i == j) {
                // 复制8×8的矩阵
                for (x = i + 7; x >= i; x--) {
                    v1 = A[x][i]; v2 = A[x][i + 1]; v3 = A[x][i + 2]; v4 = A[x][i + 3];
                    v5 = A[x][i + 4]; v6 = A[x][i + 5]; v7 = A[x][i + 6]; v8 = A[x][i + 7];
                    B[x][i] = v1; B[x][i + 1] = v2; B[x][i + 2] = v3; B[x][i + 3] = v4;
                    B[x][i + 4] = v5; B[x][i + 5] = v6; B[x][i + 6] = v7; B[x][i + 7] = v8;
                }
                // 转置8×8的矩阵
                for (x = i + 7; x >= i; x--) {
                    for (y = x - 1; y >= i; y--) {
                        v1 = B[x][y];
                        B[x][y] = B[y][x];
                        B[y][x] = v1;
                    }
                }
            } else {
                // 直接转置
                for (x = i + 7; x >= i; x--) {
                    for (y = j + 7; y >= j; y--) {
                        B[y][x] = A[x][y];
                    }
                }
            }
        }
    }
}

// 最终版本的64×64矩阵转置(不是最优解)
// 最优解的技巧:借用区块,延迟转置(参考:https://zhuanlan.zhihu.com/p/387662272)
void transpose_64_64(int A[64][64], int B[64][64]) {
    int i, j, x, y;
    int v1, v2, v3, v4, v5, v6, v7, v8;
    for (i = 0; i < 64; i += 8) {
        for (j = 0; j < 64; j += 8) {
            if (i == j) {
                // 复制4×8的矩阵
                for (x = i + 3; x >= i; x--) {
                    v1 = A[x][j]; v2 = A[x][j + 1]; v3 = A[x][j + 2]; v4 = A[x][j + 3];
                    v5 = A[x][j + 4]; v6 = A[x][j + 5]; v7 = A[x][j + 6]; v8 = A[x][j + 7];
                    B[x][j] = v1; B[x][j + 1] = v2; B[x][j + 2] = v3; B[x][j + 3] = v4;
                    B[x][j + 4] = v5; B[x][j + 5] = v6; B[x][j + 6] = v7; B[x][j + 7] = v8;
                }
                // 转置4×4的矩阵
                for (x = i + 3; x >= i; x--) {
                    for (y = x - 1; y >= i; y--) {
                        // 转置左上角的矩阵
                        v1 = B[x][y];
                        B[x][y] = B[y][x];
                        B[y][x] = v1;
                        // 转置右上角的矩阵
                        v1 = B[x][y + 4];
                        B[x][y + 4] = B[y][x + 4];
                        B[y][x + 4] = v1;
                    }
                }
                // 复制4×8的矩阵
                for (x = i + 7; x >= i + 4; x--) {
                    v1 = A[x][j]; v2 = A[x][j + 1]; v3 = A[x][j + 2]; v4 = A[x][j + 3];
                    v5 = A[x][j + 4]; v6 = A[x][j + 5]; v7 = A[x][j + 6]; v8 = A[x][j + 7];
                    B[x][j] = v1; B[x][j + 1] = v2; B[x][j + 2] = v3; B[x][j + 3] = v4;
                    B[x][j + 4] = v5; B[x][j + 5] = v6; B[x][j + 6] = v7; B[x][j + 7] = v8;
                }
                // 转置4×4的矩阵
                for (x = i + 7; x >= i + 4; x--) {
                    for (y = x - 5; y >= i; y--) {
                        // 转置左下角的矩阵
                        v1 = B[x][y];
                        B[x][y] = B[y + 4][x - 4];
                        B[y+ 4][x - 4] = v1;
                        // 转置右下角的矩阵
                        v1 = B[x][y + 4];
                        B[x][y + 4] = B[y + 4][x];
                        B[y+ 4][x] = v1;
                    }
                }
                // 交换左下角和右上角的矩阵(以B[j][i]为基准)
                for (x = j + 7; x >= j + 4; x--) {
                    // 读取左下角的一行
                    v1 = B[x][i]; v2 = B[x][i + 1]; v3 = B[x][i + 2]; v4 = B[x][i + 3];
                    // 读取右上角的一行
                    v5 = B[x - 4][i + 4]; v6 = B[x - 4][i + 5]; v7 = B[x - 4][i + 6]; v8 = B[x - 4][i + 7];
                    // 写入右上角的一行
                    B[x - 4][i + 4] = v1; B[x - 4][i + 5] = v2; B[x - 4][i + 6] = v3; B[x - 4][i + 7] = v4;
                    // 写入左下角的一行
                    B[x][i] = v5; B[x][i + 1] = v6; B[x][i + 2] = v7; B[x][i + 3] = v8;
                }
            } else {
                // 转置4×8的矩阵(左上角和右上角的矩阵)
                for (x = i + 3; x >= i; x--) {
                    v1 = A[x][j]; v2 = A[x][j + 1]; v3 = A[x][j + 2]; v4 = A[x][j + 3];
                    v5 = A[x][j + 4]; v6 = A[x][j + 5]; v7 = A[x][j + 6]; v8 = A[x][j + 7];
                    B[j][x] = v1; B[j + 1][x] = v2; B[j + 2][x] = v3; B[j + 3][x] = v4;
                    B[j][x + 4] = v5; B[j + 1][x + 4] = v6; B[j + 2][x + 4] = v7; B[j + 3][x + 4] = v8;
                }
                // 交换并转置4×4的矩阵(右上角和左下角的矩阵)
                for (y = j + 3; y >= j; y--) {
                    v1 = A[i + 4][y]; v2 = A[i + 5][y]; v3 = A[i + 6][y]; v4 = A[i + 7][y];
                    v5 = B[y][i + 4]; v6 = B[y][i + 5]; v7 = B[y][i + 6]; v8 = B[y][i + 7];
                    B[y][i + 4] = v1; B[y][i + 5] = v2; B[y][i + 6] = v3; B[y][i + 7] = v4;
                    B[y + 4][i] = v5; B[y + 4][i + 1] = v6; B[y + 4][i + 2] = v7; B[y + 4][i + 3] = v8;
                }
                // 转置4×4的矩阵(右下角的矩阵)
                for (x = i + 7; x >= i + 4; x--) {
                    v1 = A[x][j + 4]; v2 = A[x][j + 5]; v3 = A[x][j + 6]; v4 = A[x][j + 7];
                    B[j + 4][x] = v1; B[j + 5][x] = v2; B[j + 6][x] = v3; B[j + 7][x] = v4;
                }
            }
        }
    }
}

// 最终版本的61×67矩阵转置
void transpose_61_67(int A[67][61], int B[61][67]) {
    int i, j, x, y, block_size = 17;
    for (i = 0; i < 67; i += block_size) {
        for (j = 0; j < 61; j += block_size) {
            for (x = i; x < 67 && x < i + block_size; x++) {
                for (y = j; y < 61 && y < j + block_size; y++) {
                    B[y][x] = A[x][y];
                }
            }
        }
    }
}

void transpose_submit(int M, int N, int A[N][M], int B[M][N]) {
    if (M == 32 && N == 32) {
        transpose_32_32(A, B);
    } else if (M == 64 && N == 64) {
        transpose_64_64(A, B);
    } else if (M == 61 && N == 67) {
        transpose_61_67(A, B);
    }
}

/* 
 * You can define additional transpose functions below. We've defined
 * a simple one below to help you get started. 
 */ 

/* 
 * trans - A simple baseline transpose function, not optimized for the cache.
 */
char trans_desc[] = "Simple row-wise scan transpose";
void trans(int M, int N, int A[N][M], int B[M][N])
{
    int i, j, tmp;

    for (i = 0; i < N; i++) {
        for (j = 0; j < M; j++) {
            tmp = A[i][j];
            B[j][i] = tmp;
        }
    }
}

/*
 * registerFunctions - This function registers your transpose
 *     functions with the driver.  At runtime, the driver will
 *     evaluate each of the registered functions and summarize their
 *     performance. This is a handy way to experiment with different
 *     transpose strategies.
 */
void registerFunctions()
{
    /* Register your solution function */
    registerTransFunction(transpose_submit, transpose_submit_desc); 

    /* Register any additional transpose functions */
    registerTransFunction(trans, trans_desc); 

}

/* 
 * is_transpose - This helper function checks if B is the transpose of
 *     A. You can check the correctness of your transpose by calling
 *     it before returning from the transpose function.
 */
int is_transpose(int M, int N, int A[N][M], int B[M][N])
{
    int i, j;

    for (i = 0; i < N; i++) {
        for (j = 0; j < M; ++j) {
            if (A[i][j] != B[j][i]) {
                return 0;
            }
        }
    }
    return 1;
}

