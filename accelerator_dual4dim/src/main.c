#include "myLib.h"

void myFunc (unsigned int size, unsigned int dim, dataType_t threshold, dataType_t * data0, dataType_t * data1, dataType_t * data2)
{
	unsigned int i, k, l;

	for ( i = 0 ; i < size ; i ++ )
	{
		for ( k = 0 ; k < dim ; k ++ )
		{
			data2 [ i*dim + k ] = 0.0 ;
		}

		for ( k = 0 ; k < dim ; k ++ )
		{
			for ( l = 0 ; l < dim ; l ++ )
			{
				data2 [ i*dim + k ] += data0 [ k * dim + l ] * data1 [ i*dim+ l ];
			}			
		}
		
		int r = 1 ;

		for ( l = 0 ; r && ( l < dim ) ; l ++ )
		{
			r = ( data2 [ i*dim + l ] > threshold ) ;
		}

		if ( r )
		{
			for ( l = 0 ; l < dim ; l ++ )
			{
				data2 [ i*dim + l ] = 0.0;
			}
		}
	}
}

int main(int argc, char ** argv)
{

    /*
     * Timing variables
     */
    double totalTime_sw=0.0;
	struct timespec timerStart_sw;
	struct timespec timerStop_sw;
    
    double totalTime_hw=0.0;
	struct timespec timerStart_hw;
	struct timespec timerStop_hw;
    
	unsigned int i;

	assert(argv[1]!=NULL);
	unsigned int seed = (unsigned int)atoi(argv[1]);
	assert(seed);
        
    /*
     * rand() seed
     */
	srand(seed);

	assert(argv[2] != NULL);
	unsigned int size = (unsigned int)atoi(argv[2]);
	assert(size>=1);

	assert(argv[3] != NULL);
	unsigned int dim = (unsigned int)atoi(argv[3]);
	assert(dim>=2);

	assert(argv[4] != NULL);
	dataType_t threshold = (dataType_t)atof(argv[4]);
	assert(threshold>=0.0);

	printf("Seed %u\nSize %u\nDimension %u\nThreshold %.2f\n", seed, size, dim, threshold);
	fflush(stdout);	

	dataType_t * data0 = (dataType_t *)sds_alloc(sizeof(dataType_t)*dim*dim);
	assert(data0!=NULL);

	for(i=0;i<dim*dim;i++) {
		dataType_t t = (float)(rand()%10);
		dataType_t d = ((float)(rand()%10))/10;
		data0[i] = t+d;
        assert(data0[i] == t+d);
	}
	
	dataType_t * data1 = (dataType_t *)sds_alloc(sizeof(dataType_t)*dim*size);
	assert(data1!=NULL);

	for(i=0;i<dim*size;i++) {
		dataType_t t = (float)(rand()%10);
		dataType_t d = ((float)(rand()%10))/10;
		data1[i] = t+d;
        assert(data1[i] == t+d);
	}
	
	/********************************/
    /* Reference Software Execution */
    /********************************/

	dataType_t * data2_sw = (dataType_t *)malloc(sizeof(dataType_t)*dim*size);
	assert(data2_sw!=NULL);
    
	printf("Calling myFunc... ");
	fflush(stdout);
    
    /*
     * Start timing
     */
    clock_gettime(CLOCK_REALTIME, &timerStart_sw);
    
	myFunc (size, dim, threshold, data0, data1, data2_sw);
    /*
     * Stop timing and get function running time
     */
    clock_gettime(CLOCK_REALTIME, &timerStop_sw);
    totalTime_sw = (timerStop_sw.tv_sec-timerStart_sw.tv_sec)+(timerStop_sw.tv_nsec-timerStart_sw.tv_nsec)/BILLION;
	printf("DONE\n");
	fflush(stdout);
    
    printf("Software execution time: %f seconds\n", totalTime_sw);
    
    /**********************/
    /* Hardware Execution */
    /**********************/

    /*
     * Determine the indeces of the split array for multi instance accelerator
     */
    int size0 = size/2;
    int size1 = size - size0;

	dataType_t * data2_hw = (dataType_t *)sds_alloc(sizeof(dataType_t)*dim*size);
	assert(data2_hw!=NULL);
    printf("Calling myFuncAccel... ");
    fflush(stdout);
    /*
     * Start timing
     */
    clock_gettime(CLOCK_REALTIME, &timerStart_hw);

#pragma SDS async(1)
    myFuncAccel (size0, dim, threshold, data0, data1, data2_hw, 0);
#pragma SDS async(2)
    myFuncAccel (size, dim, threshold, data0, data1, data2_hw, size1);
#pragma SDS wait(1)
#pragma SDS wait(2)
    /*
     * Stop timing and get function running time
     */
    clock_gettime(CLOCK_REALTIME, &timerStop_hw);
    totalTime_hw = (timerStop_hw.tv_sec-timerStart_hw.tv_sec)+(timerStop_hw.tv_nsec-timerStart_hw.tv_nsec)/BILLION;
	printf("DONE\n");
	fflush(stdout);
    
    printf("Hardware execution time: %f seconds\n", totalTime_hw);

    /*****************************************************/
    /* Correctness check/Compare with software reference */
    /*****************************************************/
    
    for (i=0; i<size; i++) {
        for (int k=0; k<dim; k++) {
        	assert(data2_sw[i*dim + k] == data2_hw[i*dim + k]);
        }
    }

	sds_free(data0);
	sds_free(data1);
	free(data2_sw);
    sds_free(data2_hw);

	return 0;
}
