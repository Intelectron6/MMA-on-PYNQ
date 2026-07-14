#include <ap_fixed.h>

#define TILE_SIZE 12

typedef ap_fixed<16,8> data_t;


//------------------------------------------------------------
// Load Matrix from DDR
//------------------------------------------------------------
void load_matrix(
    const data_t *src,
    data_t buf[TILE_SIZE][TILE_SIZE])
{
#pragma HLS INLINE off

    for(int i=0;i<TILE_SIZE;i++)
    {
        for(int j=0;j<TILE_SIZE;j++)
        {
#pragma HLS PIPELINE II=1
            buf[i][j] = src[i*TILE_SIZE + j];
        }
    }
}


//------------------------------------------------------------
// Compute Engine
//------------------------------------------------------------
void compute_matrix(
    data_t A[TILE_SIZE][TILE_SIZE],
    data_t B[TILE_SIZE][TILE_SIZE],
    data_t C[TILE_SIZE][TILE_SIZE])
{
#pragma HLS INLINE off

#pragma HLS ARRAY_PARTITION variable=A complete dim=2
#pragma HLS ARRAY_PARTITION variable=B complete dim=1
#pragma HLS ARRAY_PARTITION variable=C complete dim=0

    for(int k=0;k<TILE_SIZE;k++)
    {
#pragma HLS PIPELINE II=1

        for(int i=0;i<TILE_SIZE;i++)
        {
#pragma HLS UNROLL

            for(int j=0;j<TILE_SIZE;j++)
            {
#pragma HLS UNROLL

                if(k==0)
                    C[i][j]=0;

                C[i][j] += A[i][k] * B[k][j];
            }
        }
    }
}


//------------------------------------------------------------
// Store Matrix back to DDR
//------------------------------------------------------------
void store_matrix(
    data_t buf[TILE_SIZE][TILE_SIZE],
    data_t *dst)
{
#pragma HLS INLINE off

    for(int i=0;i<TILE_SIZE;i++)
    {
        for(int j=0;j<TILE_SIZE;j++)
        {
#pragma HLS PIPELINE II=1
            dst[i*TILE_SIZE + j] = buf[i][j];
        }
    }
}


//------------------------------------------------------------
// Top Level
//------------------------------------------------------------
void mma_top(
    const data_t *A,
    const data_t *B,
    data_t *C)
{

#pragma HLS INTERFACE m_axi port=A offset=slave bundle=gmem0 depth=144
#pragma HLS INTERFACE m_axi port=B offset=slave bundle=gmem1 depth=144
#pragma HLS INTERFACE m_axi port=C offset=slave bundle=gmem2 depth=144

#pragma HLS INTERFACE s_axilite port=A bundle=control
#pragma HLS INTERFACE s_axilite port=B bundle=control
#pragma HLS INTERFACE s_axilite port=C bundle=control
#pragma HLS INTERFACE s_axilite port=return bundle=control


    data_t buf_A[TILE_SIZE][TILE_SIZE];
    data_t buf_B[TILE_SIZE][TILE_SIZE];
    data_t buf_C[TILE_SIZE][TILE_SIZE];

#pragma HLS DATAFLOW

    load_matrix(A, buf_A);

    load_matrix(B, buf_B);

    compute_matrix(buf_A, buf_B, buf_C);

    store_matrix(buf_C, C);
}
