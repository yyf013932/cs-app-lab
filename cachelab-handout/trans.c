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

void blockTrans(int M, int N, int A[N][M], int B[M][N],int cSize,int rSize);

/* 
 * transpose_submit - This is the solution transpose function that you
 *     will be graded on for Part B of the assignment. Do not change
 *     the description string "Transpose submission", as the driver
 *     searches for that string to identify the transpose function to
 *     be graded. 
 */
char transpose_submit_desc[] = "Transpose submission";
void transpose_submit(int M, int N, int A[N][M], int B[M][N]){
	if(M!=N)
		blockTrans(M,N,A,B,16,16);
	else if(M==32){
		blockTrans(M,N,A,B,8,8);	
	}else{
		blockTrans(M,N,A,B,8,4);
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
 * trans - A simple baseline transpose function, not optimized for the cache.
 */
char trans_desc_block_84[] = "using blocking with block size 84";
void trans_block_84(int M, int N, int A[N][M], int B[M][N])
{   
	blockTrans(M,N,A,B,8,4);
}

/* 
 * trans - A simple baseline transpose function, not optimized for the cache.
 */
char trans_desc_block_88[] = "using blocking with block size 88";
void trans_block_88(int M, int N, int A[N][M], int B[M][N])
{   
	blockTrans(M,N,A,B,8,8);
}

/* 
 * trans - A simple baseline transpose function, not optimized for the cache.
 */
char trans_desc_block_44[] = "using blocking with block size 44";
void trans_block_44(int M, int N, int A[N][M], int B[M][N])
{   
	blockTrans(M,N,A,B,4,4);
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

	registerTransFunction(trans_block_84, trans_desc_block_84); 

	registerTransFunction(trans_block_88, trans_desc_block_88); 

	registerTransFunction(trans_block_44, trans_desc_block_44); 
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

void blockTrans(int M, int N, int A[N][M], int B[M][N],int cSize,int rSize){
	int rowBlocks = M/rSize;
	int colBlocks = N/cSize;
	for(int i=0;i<colBlocks;i++){
		for(int j=0;j<rowBlocks;j++){
			for(int r=cSize*i;r<cSize*(i+1);r++){
				int tem=-1;
				for(int c=rSize*j;c<rSize*(j+1);c++){
					if(c==r){
						tem = A[r][c];					
						continue;
					}
					B[c][r] = A[r][c]; 
				}
				if(tem!=-1)
					B[r][r] = tem;
			}
		}
	}
	for(int i=0;i<colBlocks;i++){
		for(int r = cSize*i;r<cSize*(i+1);r++){
			for(int c = rSize*rowBlocks;c<M;c++){
				B[c][r] = A[r][c];
			}
		}
	}
	for(int j=0;j<rowBlocks;j++){
		for(int r = cSize*colBlocks;r<N;r++){
			for(int c = rSize*j;c<rSize*(j+1);c++){
				B[c][r] = A[r][c];
			}
		}
	}
	
	for(int r=cSize*colBlocks;r<N;r++){
		for(int c=rSize*rowBlocks;c<M;c++){
			B[c][r]=A[r][c];
		}
	}
}

