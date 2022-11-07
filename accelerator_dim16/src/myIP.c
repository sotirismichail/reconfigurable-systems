#include "myLib.h"

#define dimf 16
#define bufdim 16
#define acache 256

#pragma SDS data access_pattern(data0:SEQUENTIAL)
#pragma SDS data access_pattern(data1:SEQUENTIAL)
#pragma SDS data access_pattern(data2:SEQUENTIAL)

#pragma SDS data copy(data0[0:dim*dimf])
#pragma SDS data copy(data1[0:size*dimf])
#pragma SDS data copy(data2[0:size*dimf])

#pragma SDS data data_mover(data0:AXIDMA_SIMPLE:1, data1:AXIDMA_SIMPLE:2, data2:AXIDMA_SIMPLE:3)

#pragma SDS data buffer_depth(data0buf:256, data1buf:16, data2buf:16, r:16)

void myFuncAccel (unsigned int size, unsigned int dim, dataType_t threshold, dataType_t * data0, dataType_t * data1, dataType_t * data2)
{
	unsigned int i, k, l;
    dataType_t data0buf[acache], data1buf[bufdim], data2buf[bufdim];
    int r[bufdim];

#pragma HLS ARRAY_PARTITION variable=data2buf block factor=4 dim=1
#pragma HLS ARRAY_PARTITION variable=data1buf block factor=4 dim=1
#pragma HLS ARRAY_PARTITION variable=data0buf block factor=4 dim=1

    data0_loop:for ( k = 0 ; k < dimf ; k ++ )
    {
#pragma HLS LOOP_TRIPCOUNT min=16 max=16 avg=16
#pragma HLS PIPELINE II=16
        data0_loop_inner:for ( l = 0 ; l < dimf; l ++ )
        {
#pragma HLS LOOP_TRIPCOUNT min=16 max=16 avg=16
#pragma HLS UNROLL
            data0buf [ k * dimf + l ] = data0 [ k * dimf + l ];
        }
    }

	i_loop:for ( i = 0 ; i < size ; i ++ )
	{
#pragma HLS LOOP_TRIPCOUNT min=16 max=16 avg=16
#pragma HLS PIPELINE II=16
		init_loop:for ( k = 0 ; k < dimf ; k ++ )
		{
#pragma HLS LOOP_TRIPCOUNT min=16 max=16 avg=16
#pragma HLS UNROLL
			data2 [ i * dimf + k ] = 0.0;
            data2buf [ k ] = 0.0;
		}

        l_loop:for ( l = 0 ; l < dimf ; l ++ )
        {
#pragma HLS LOOP_TRIPCOUNT min=16 max=16 avg=16
#pragma HLS UNROLL
                data1buf [ l ] = data1 [ i * dimf + l ];
        }

		k_loop_outer:for ( k = 0 ; k < dimf ; k ++ )
		{
#pragma HLS LOOP_TRIPCOUNT min=16 max=16 avg=16
			k_loop_inner:for ( l = 0 ; l < dimf ; l ++ )
			{
#pragma HLS LOOP_TRIPCOUNT min=16 max=16 avg=16
#pragma HLS UNROLL
                data2buf [ k ] += data0buf [ k * dimf + l ] * data1buf [ l ];
			}			
		}

		for ( l = 0 ; l < dimf ; l ++ )
		{
#pragma HLS UNROLL
			r[ l ] = 1;
			r[ l ] = ( data2buf [ l ] > threshold );
		}

int rf = ( r[0] == 1 && r[1] == 1 && r[2] == 1 && r[3] == 1 && r[4] == 1 && r[5] == 1 && r[6] == 1 && r[7] == 1 && r[8] == 1 && r[9] == 1 && r[10] == 1 && r[11] == 1 && r[12] == 1 && r[13] == 1 && r[14] == 1 &&  r[15] == 1);
//int rf = ( r[0] == 1 && r[1] == 1 && r[2] == 1 && r[3] == 1 );

		if ( rf )
		{
			thres_apply:for ( l = 0 ; l < dimf ; l ++ )
			{
#pragma HLS LOOP_TRIPCOUNT min=16 max=16 avg=16
#pragma HLS UNROLL
				data2buf [ l ] = 0.0;
			}
		}

		output:for( k = 0 ; k < dimf ; k ++ )
        {
#pragma HLS LOOP_TRIPCOUNT min=16 max=16 avg=16
#pragma HLS UNROLL
            data2 [ i * dimf + k ] = data2buf [ k ];
        }
    }
}
