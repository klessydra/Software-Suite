/*
Test
-Multithread
-Decoupled
-Uses the ZEROPADDING convolutional algorithm
-Single accelerator
-can work with three SPM banks instead of four
-SPM size must be > 4KB
*/
// ----------------------------------------------------------------------------------------------------
// ----------------------------------ZEROPADDED--------------------------------------------------------
// ----------------------------------------------------------------------------------------------------
#define REPLICATION 1	
#ifndef A_ORDER
#define A_ORDER 16 //Matrix size, don't do 2x2 case, for that i have another test
#endif
#ifndef B_ORDER
#define B_ORDER 3	//set to 3-5-7-9-11
#endif
#define EXPLICIT 1 // set to 0: uses the function with double inner loops; set to 1: use the rolled functions with explicit add&muls
// ----------------------------------------------------------------------------------------------------
#if B_ORDER == 3
#define Z_overhead 2
#endif
#if B_ORDER == 5
#define Z_overhead 4
#endif
#if B_ORDER == 7
#define Z_overhead 6
#endif
#if B_ORDER == 9
#define Z_overhead 8
#endif
#if B_ORDER == 11
#define Z_overhead 10
#endif
#define Z_ORDER (A_ORDER+Z_overhead)
// ----------------------------------------------------------------------------------------------------

#ifndef PRINT_NUM_CYCLES
//#define PRINT_NUM_CYCLES 1 // to print the cycle count
#endif

//#define PRINT_DEBUG 1 //to check if matrix is correct
// #define MATRIX_CHECK 0

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include "dsp_functions.h"
#include "functions.h"
#include "klessydra_defs.h"

int loop_index=1;

#define MULTI_TH 1

#define SPM_MAX 64
#define SIZE_OF_INT 4
#define K_COL (B_ORDER+1)

int matA0[A_ORDER*A_ORDER];
int matA1[A_ORDER*A_ORDER];
int matA2[A_ORDER*A_ORDER];
int dimension_A=A_ORDER*A_ORDER*sizeof(int);
int dimension_Zero=Z_ORDER*Z_ORDER*sizeof(int);

int matB[B_ORDER*B_ORDER] = {0};
int matB_5[5*5] = {0};
int dimension_B=B_ORDER*B_ORDER*sizeof(int);

int output_compare0[A_ORDER*A_ORDER]={0};
int output_compare1[A_ORDER*A_ORDER]={0};
int output_compare2[A_ORDER*A_ORDER]={0};


int output_compare_zero[A_ORDER*A_ORDER]={0};

int output_compare_s0[A_ORDER*A_ORDER]={0};
int output_compare_s1[A_ORDER*A_ORDER]={0};
int output_compare_s2[A_ORDER*A_ORDER]={0};
int mat_second_A[3][A_ORDER][A_ORDER];

// #define Z_ORDER A_ORDER
int conv2D_out_scal=5;
int shift_pre=8;
int shift_out=5;
int warn[3];

int print_global_k , print_global_id	,	print_global_dbg;
int azzero=0;
int sign;
int memory_offset[3]={0};

int final_perf[3][8] = {{777}};

int perf0ld = 0;
int final_perf0ld = 777;
int  *ptr_perf0ld = &perf0ld;

int final_perf_mem[2] = {0};
int 			perf_mem = 0;
int  *ptr_perf_mem = &perf_mem;
int kbcastld_cycles=0;

int preparing_data=0;


int preparing_data_kbcast_th0=0;

int perf0_prescaling0=0;
int *ptr_perf0_prescaling0=&perf0_prescaling0;
int prescaling_th0=0;

int perf0_prescaling1=0;
int *ptr_perf0_prescaling1=&perf0_prescaling1;
int prescaling_th1=0;

int perf0_prescaling2=0;
int *ptr_perf0_prescaling2=&perf0_prescaling2;
int prescaling_th2=0;

void matrix_check( int* mat1, int* mat2, int size );

void convolution2D_SPM_off(void* spm_dest, void* spm_fm, void* spm_krn, int size, int mem_off);
//------------------------------------------------------------------------------------------------------------
void convolution2D_SPM_off_NOB_zeropadded(void* spm_dest, 	 void* spm_fm,	 void* spm_krn,	 void* spm_temp,  int size);
void convolution2D_SPM_off_NOB_zeropadded_3(void* spm_dest, 	 void* spm_fm,	 void* spm_krn,	 void* spm_temp,  int size);
void convolution2D_SPM_off_NOB_zeropadded_5(void* spm_dest, 	 void* spm_fm,	 void* spm_krn,	 void* spm_temp,  int size);
void convolution2D_SPM_off_NOB_zeropadded_7(void* spm_dest, 	 void* spm_fm,	 void* spm_krn,	 void* spm_temp,  int size);
void convolution2D_SPM_off_NOB_zeropadded_9(void* spm_dest, 	 void* spm_fm,	 void* spm_krn,	 void* spm_temp,  int size);
void convolution2D_SPM_off_NOB_zeropadded_11(void* spm_dest, 	 void* spm_fm,	 void* spm_krn,	 void* spm_temp,  int size);
//------------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------------
void convolution2D_Zeropadding(int size, int (*matrix)[size], int *kernel_tmp, int *out);
void convolution2D_Zeropadding_3(int size, int (*matrix)[size], int *kernel_tmp, int *out);
void convolution2D_Zeropadding_5(int size, int (*matrix)[size], int *kernel_tmp, int *out);
void convolution2D_Zeropadding_7(int size, int (*matrix)[size], int *kernel_tmp, int *out);
void convolution2D_Zeropadding_9(int size, int (*matrix)[size], int *kernel_tmp, int *out);
void convolution2D_Zeropadding_11(int size, int (*matrix)[size], int *kernel_tmp, int *out);
//------------------------------------------------------------------------------------------------------------

#if B_ORDER == 3
	void conv2D_Zeropad_excplicit_3(int size, int (*matrix)[size], int *kernel_tmp, int *out);
#endif
#if B_ORDER == 5
	void conv2D_Zeropad_excplicit_5(int size, int (*matrix)[size], int *kernel_tmp, int *out);
#endif
#if B_ORDER == 7
	void conv2D_Zeropad_excplicit_7(int size, int (*matrix)[size], int *kernel_tmp, int *out);
#endif
#if B_ORDER == 9
	void conv2D_Zeropad_excplicit_9(int size, int (*matrix)[size], int *kernel_tmp, int *out);
#endif
#if B_ORDER == 11
	void conv2D_Zeropad_excplicit_11(int size, int (*matrix)[size], int *kernel_tmp, int *out);
#endif

//------------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------------
// 													MAIN
//------------------------------------------------------------------------------------------------------------
int main(){

	int off_idx=0;
	int array_boundaries[3]={0};
	int squares[A_ORDER*A_ORDER]={0};
	int padded[Z_ORDER][Z_ORDER]={0};
	int junk=777;
	print_global_k =0;
	print_global_id=0;
	print_global_dbg=0;;
	warn[0]=2;
	warn[1]=2;
	warn[2]=2;
	sign=1;
	// int increment=1000;
	int increment=1000;
	for(int i=0; i<B_ORDER*B_ORDER; i++){
		// matB[i]=(increment*1)<<8;
		matB[i]=(i+1)*increment<<8;
  }
	
	// for(int i=0; i<5*5; i++){
		// matB_5[i]=(1000*1)<<8;
  // }
	
	for(int i =0; i<A_ORDER*A_ORDER; i++){
    // matA0[i]=(1000*sign*(i+1))<<8;
    matA0[i]=(increment*1)<<8;
		matA1[i]=(20*sign*(i+1))<<8;
		matA2[i]=(3*sign*(i+1))<<8;
		sign=sign*(-1);
		output_compare_s0[i]=777;
		output_compare_s1[i]=777;
		output_compare_s2[i]=777;
		output_compare_zero[i]=777;
  }
	sign = 1;
  for(int i =0;i<A_ORDER; i++) {
    for(int j=0; j<A_ORDER; j++)
		{
      // mat_second_A[0][i][j]=(1000*sign*(i*A_ORDER+j+1))<<8;
			mat_second_A[0][i][j]=(1000*1)<<8;
			mat_second_A[1][i][j]=(20*sign*(i*A_ORDER+j+1))<<8;
			mat_second_A[2][i][j]=(3*sign*(i*A_ORDER+j+1))<<8;
			sign=sign*(-1);
    }
  }
	memory_offset[0]=0;
	memory_offset[1]=1*32*32;
	memory_offset[2]=2*32*32;
	__asm__("csrw 0x300, 0x8;" );// each thread enables it's own interrupt
  __asm__("csrrw zero, mcycle, zero");
	CSR_MVSIZE(SPM_MAX*SPM_MAX*sizeof(int));
	kbcast((void*)spmaddrA,(void*)azzero);
	kbcast((void*)spmaddrB,(void*)azzero);
	kbcast((void*)spmaddrC,(void*)azzero);
	kbcast((void*)spmaddrD,(void*)azzero);
	//so i just use a quick function that do the trick
	CSR_MVSIZE(2*SIZE_OF_INT);
	kdotpps_v3((void*)spmaddrA,	(void*)spmaddrA,	(void*)spmaddrB, (void*) conv2D_out_scal);
	array_boundaries[0] = A_ORDER/3;
	array_boundaries[1] =	array_boundaries[0]*2;
	array_boundaries[2] = A_ORDER;
	CSR_MVSIZE(dimension_Zero);
	
	int z_kmem_offset;
	if (B_ORDER==3) 	z_kmem_offset=1;
	if (B_ORDER==5) 	z_kmem_offset=2;
	if (B_ORDER==7) 	z_kmem_offset=3;
	if (B_ORDER==9) 	z_kmem_offset=4;
	if (B_ORDER==11) 	z_kmem_offset=5;
	
	#ifdef MULTI_TH
		//--------------------------------------LOADING & PRESCALING------------------------------------------
		#ifndef REPLICATION // if there is no replication, then just one kmemld is needed for the program to run
			int replication=0;
			sync_barrier_reset();		sync_barrier_thread_registration();
			if (Klessydra_get_coreID() == 0) {
				// ENABLE COUNTING -------------------------------------------------------------------------
				final_perf0ld=0;
				#ifdef PRINT_NUM_CYCLES
					Klessydra_perf_cnt_start();
				#endif
				//------------------------------------------------------------------------------------------// CSR_MVSIZE(dimension_A);
				kmemld((void*)((int*)spmaddrB+memory_offset[0]), (void*)matB, dimension_B);
				ksrav((void*)((int*)spmaddrB+memory_offset[0]),	 (void*)((int*)spmaddrB+memory_offset[0]),	(int*)shift_pre);
				// loop di kmemload discrete
				for (int i=0; i<A_ORDER; i++){
					kmemld(
						(void*)((int*)spmaddrA + (	z_kmem_offset*Z_ORDER+z_kmem_offset +i*Z_ORDER) ),
						(void*)((int*)matA0+ (i*A_ORDER) ),
						SIZE_OF_INT*(A_ORDER)	
					);							
				}
				ksrav((void*)((int*)spmaddrA+memory_offset[0]),	 (void*)((int*)spmaddrA+memory_offset[0]),	(int*)shift_pre);
				
				// DISABLE COUNTING AND SAVE MCYCLE -------------------------------------------------------
				__asm__("csrrw zero, 0x7A0, 0x00000000;"
								"csrrw %[perf0ld], mcycle, zero;"
								"sw %[perf0ld], 0(%[ptr_perf0ld]);"
								:
								:[perf0ld] "r" (perf0ld), [ptr_perf0ld] "r" (ptr_perf0ld)
				);
				preparing_data=*(ptr_perf0ld);
				// printf("Convolution with DSP 0 Speed is:\n\t%d Cycles\n", final_perf0);
				// //------------------------------------------------------------------------------------------
			}
			sync_barrier();
		#endif
		#ifdef REPLICATION	// if there is replication, then just a thread do a kbcast for all data
												// then all the threads are responsible for their prescaling of data
			int replication=0;
			// // One thread is responsible for kbcasting all data on the different SPM's of different thread
			sync_barrier_reset(); sync_barrier_thread_registration();
			if (Klessydra_get_coreID() == 0) {
				// ENABLE COUNTING -------------------------------------------------------------------------
				final_perf0ld=0;
				#ifdef PRINT_NUM_CYCLES
					Klessydra_perf_cnt_start();
				#endif
				//------------------------------------------------------------------------------------------// CSR_MVSIZE(dimension_A);
				
				kbcastld((void*)((int*)spmaddrB+memory_offset[0]), (void*)matB, dimension_B);
				// loop di kbcastld discrete
				for (int i=0; i<A_ORDER; i++){
					kbcastld(
						(void*)((int*)spmaddrA + (	z_kmem_offset*Z_ORDER+z_kmem_offset +i*Z_ORDER) ),
						(void*)((int*)matA0+ (i*A_ORDER) ),
						SIZE_OF_INT*(A_ORDER)	
					);							
				}
				
				// DISABLE COUNTING AND SAVE MCYCLE -------------------------------------------------------
				__asm__("csrrw zero, 0x7A0, 0x00000000;"
								"csrrw %[perf0ld], mcycle, zero;"
								"sw %[perf0ld], 0(%[ptr_perf0ld]);"
								:
								:[perf0ld] "r" (perf0ld), [ptr_perf0ld] "r" (ptr_perf0ld)
				);
				preparing_data_kbcast_th0=*(ptr_perf0ld);
				// //------------------------------------------------------------------------------------------
			}
			sync_barrier();

			// // The threads work in parallel to prescale the data, and the cycle_counter values are updated accordingly
			sync_barrier_reset(); sync_barrier_thread_registration();
			if (Klessydra_get_coreID() == 0) {
				// ENABLE COUNTING -------------------------------------------------------------------------
				perf0_prescaling0=0;
				__asm__("csrrw zero, mcycle, zero;"
								"csrrw zero, 0x7A0, 0x00000001"
				);
				//------------------------------------------------------------------------------------------// CSR_MVSIZE(dimension_A);
				
				ksrav((void*)((int*)spmaddrA+memory_offset[0]),	 (void*)((int*)spmaddrA+memory_offset[0]),	(int*)shift_pre);
				ksrav((void*)((int*)spmaddrB+memory_offset[0]),	 (void*)((int*)spmaddrB+memory_offset[0]),	(int*)shift_pre);
				
				// DISABLE COUNTING AND SAVE MCYCLE -------------------------------------------------------
				__asm__("csrrw zero, 0x7A0, 0x00000000;"
								"csrrw %[perf0_prescaling0], mcycle, zero;"
								"sw %[perf0_prescaling0], 0(%[ptr_perf0_prescaling0]);"
								:
								:[perf0_prescaling0] "r" (perf0_prescaling0), [ptr_perf0_prescaling0] "r" (ptr_perf0_prescaling0)
				);
				prescaling_th0=*(ptr_perf0_prescaling0);
				// //------------------------------------------------------------------------------------------
			}
			if (Klessydra_get_coreID() == 1) {
				// ENABLE COUNTING -------------------------------------------------------------------------
				perf0_prescaling1=0;
				__asm__("csrrw zero, mcycle, zero;"
								"csrrw zero, 0x7A0, 0x00000001"
				);
				//------------------------------------------------------------------------------------------// CSR_MVSIZE(dimension_A);
				
				ksrav((void*)((int*)spmaddrA+memory_offset[0]),	 (void*)((int*)spmaddrA+memory_offset[0]),	(int*)shift_pre);
				ksrav((void*)((int*)spmaddrB+memory_offset[0]),	 (void*)((int*)spmaddrB+memory_offset[0]),	(int*)shift_pre);
				
				// DISABLE COUNTING AND SAVE MCYCLE -------------------------------------------------------
				__asm__("csrrw zero, 0x7A0, 0x00000000;"
								"csrrw %[perf0_prescaling1], mcycle, zero;"
								"sw %[perf0_prescaling1], 0(%[ptr_perf0_prescaling1]);"
								:
								:[perf0_prescaling1] "r" (perf0_prescaling1), [ptr_perf0_prescaling1] "r" (ptr_perf0_prescaling1)
				);
				prescaling_th1=*(ptr_perf0_prescaling1);
				// //------------------------------------------------------------------------------------------
			}
			if (Klessydra_get_coreID() == 2) {
				// ENABLE COUNTING -------------------------------------------------------------------------
				perf0_prescaling2=0;
				__asm__("csrrw zero, mcycle, zero;"
								"csrrw zero, 0x7A0, 0x00000001"
				);
				//------------------------------------------------------------------------------------------// CSR_MVSIZE(dimension_A);
				
				ksrav((void*)((int*)spmaddrA+memory_offset[0]),	 (void*)((int*)spmaddrA+memory_offset[0]),	(int*)shift_pre);
				ksrav((void*)((int*)spmaddrB+memory_offset[0]),	 (void*)((int*)spmaddrB+memory_offset[0]),	(int*)shift_pre);
				
				// DISABLE COUNTING AND SAVE MCYCLE -------------------------------------------------------
				__asm__("csrrw zero, 0x7A0, 0x00000000;"
								"csrrw %[perf0_prescaling2], mcycle, zero;"
								"sw %[perf0_prescaling2], 0(%[ptr_perf0_prescaling2]);"
								:
								:[perf0_prescaling2] "r" (perf0_prescaling2), [ptr_perf0_prescaling2] "r" (ptr_perf0_prescaling2)
				);
				prescaling_th2=*(ptr_perf0_prescaling2);
				// //------------------------------------------------------------------------------------------
			}
			sync_barrier();
		#endif

		//--------------------------------------CONVOLUTIONS--------------------------------------------------
		sync_barrier_reset(); sync_barrier_thread_registration();
		int enable_perf_cnt = 0;
		int *ptr_perf;
		if (Klessydra_get_coreID() == 0) {
			// ENABLE COUNTING -------------------------------------------------------------------------
			#ifdef PRINT_NUM_CYCLES
				Klessydra_perf_cnt_start();
			#endif
			//------------------------------------------------------------------------------------------
			for (int l=0; l<loop_index; l++) {
				off_idx=0;
				if (B_ORDER==3)
				convolution2D_SPM_off_NOB_zeropadded_3(
						(void*)(	(int*)spmaddrC+	 memory_offset[off_idx]	),
						(void*)(	(int*)spmaddrA+	 memory_offset[off_idx]	), 
						(void*)(	(int*)spmaddrB+  memory_offset[off_idx]	), 
						(void*)(	(int*)spmaddrD+	 memory_offset[off_idx]	),	Z_ORDER
						);
				if (B_ORDER==5)
				convolution2D_SPM_off_NOB_zeropadded_5(
						(void*)(	(int*)spmaddrC+	 memory_offset[off_idx]	),
						(void*)(	(int*)spmaddrA+	 memory_offset[off_idx]	), 
						(void*)(	(int*)spmaddrB+  memory_offset[off_idx]	), 
						(void*)(	(int*)spmaddrD+	 memory_offset[off_idx]	),	Z_ORDER
						);
				if (B_ORDER==7)
				convolution2D_SPM_off_NOB_zeropadded_7(
						(void*)(	(int*)spmaddrC+	 memory_offset[off_idx]	),
						(void*)(	(int*)spmaddrA+	 memory_offset[off_idx]	), 
						(void*)(	(int*)spmaddrB+  memory_offset[off_idx]	), 
						(void*)(	(int*)spmaddrD+	 memory_offset[off_idx]	),	Z_ORDER
						);
				if (B_ORDER==9)
				convolution2D_SPM_off_NOB_zeropadded_9(
						(void*)(	(int*)spmaddrC+	 memory_offset[off_idx]	),
						(void*)(	(int*)spmaddrA+	 memory_offset[off_idx]	), 
						(void*)(	(int*)spmaddrB+  memory_offset[off_idx]	), 
						(void*)(	(int*)spmaddrD+	 memory_offset[off_idx]	),	Z_ORDER
						);
				if (B_ORDER==11)
				convolution2D_SPM_off_NOB_zeropadded_11(
						(void*)(	(int*)spmaddrC+	 memory_offset[off_idx]	),
						(void*)(	(int*)spmaddrA+	 memory_offset[off_idx]	), 
						(void*)(	(int*)spmaddrB+  memory_offset[off_idx]	), 
						(void*)(	(int*)spmaddrD+	 memory_offset[off_idx]	),	Z_ORDER
						);
			}
			for (int i=0; i<A_ORDER; i++){
				kmemstr((void*)((int*)output_compare_zero+ i*A_ORDER ),
								(void*)((int*)spmaddrC + ( z_kmem_offset*Z_ORDER + z_kmem_offset + i*Z_ORDER)) ,
								SIZE_OF_INT*(A_ORDER)	);							
			}
			// DISABLE COUNTING AND SAVE MCYCLE -------------------------------------------------------
			#ifdef PRINT_NUM_CYCLES
				ptr_perf = Klessydra_perf_cnt_finish();
			#endif
			// //------------------------------------------------------------------------------------------
		}
		if (Klessydra_get_coreID() == 1) {
			// ENABLE COUNTING -------------------------------------------------------------------------
			#ifdef PRINT_NUM_CYCLES
				Klessydra_perf_cnt_start();
			#endif
			//------------------------------------------------------------------------------------------
			for (int l=0; l<loop_index; l++) {
				off_idx=0;
				if (B_ORDER==3)
				convolution2D_SPM_off_NOB_zeropadded_3(
						(void*)(	(int*)spmaddrC+	 memory_offset[off_idx]	),
						(void*)(	(int*)spmaddrA+	 memory_offset[off_idx]	), 
						(void*)(	(int*)spmaddrB+  memory_offset[off_idx]	), 
						(void*)(	(int*)spmaddrD+	 memory_offset[off_idx]	),	Z_ORDER
						);
				if (B_ORDER==5)
				convolution2D_SPM_off_NOB_zeropadded_5(
						(void*)(	(int*)spmaddrC+	 memory_offset[off_idx]	),
						(void*)(	(int*)spmaddrA+	 memory_offset[off_idx]	), 
						(void*)(	(int*)spmaddrB+  memory_offset[off_idx]	), 
						(void*)(	(int*)spmaddrD+	 memory_offset[off_idx]	),	Z_ORDER
						);
				if (B_ORDER==7)
				convolution2D_SPM_off_NOB_zeropadded_7(
						(void*)(	(int*)spmaddrC+	 memory_offset[off_idx]	),
						(void*)(	(int*)spmaddrA+	 memory_offset[off_idx]	), 
						(void*)(	(int*)spmaddrB+  memory_offset[off_idx]	), 
						(void*)(	(int*)spmaddrD+	 memory_offset[off_idx]	),	Z_ORDER
						);
				if (B_ORDER==9)
				convolution2D_SPM_off_NOB_zeropadded_9(
						(void*)(	(int*)spmaddrC+	 memory_offset[off_idx]	),
						(void*)(	(int*)spmaddrA+	 memory_offset[off_idx]	), 
						(void*)(	(int*)spmaddrB+  memory_offset[off_idx]	), 
						(void*)(	(int*)spmaddrD+	 memory_offset[off_idx]	),	Z_ORDER
						);
				if (B_ORDER==11)
				convolution2D_SPM_off_NOB_zeropadded_11(
						(void*)(	(int*)spmaddrC+	 memory_offset[off_idx]	),
						(void*)(	(int*)spmaddrA+	 memory_offset[off_idx]	), 
						(void*)(	(int*)spmaddrB+  memory_offset[off_idx]	), 
						(void*)(	(int*)spmaddrD+	 memory_offset[off_idx]	),	Z_ORDER
						);
			}
			for (int i=0; i<A_ORDER; i++){
				kmemstr((void*)((int*)output_compare_s1+ i*A_ORDER ),
								(void*)((int*)spmaddrC + (Z_ORDER + 1 + i*Z_ORDER)) ,
								SIZE_OF_INT*(A_ORDER)	);							
			}
			// DISABLE COUNTING AND SAVE MCYCLE -------------------------------------------------------
			#ifdef PRINT_NUM_CYCLES
				ptr_perf = Klessydra_perf_cnt_finish();
			#endif
			// //------------------------------------------------------------------------------------------
		}
		if (Klessydra_get_coreID() == 2) {
			// ENABLE COUNTING -------------------------------------------------------------------------
			#ifdef PRINT_NUM_CYCLES
				Klessydra_perf_cnt_start();
			#endif
			//------------------------------------------------------------------------------------------
			for (int l=0; l<loop_index; l++) {
				off_idx=0;
				if (B_ORDER==3)
				convolution2D_SPM_off_NOB_zeropadded_3(
						(void*)(	(int*)spmaddrC+	 memory_offset[off_idx]	),
						(void*)(	(int*)spmaddrA+	 memory_offset[off_idx]	), 
						(void*)(	(int*)spmaddrB+  memory_offset[off_idx]	), 
						(void*)(	(int*)spmaddrD+	 memory_offset[off_idx]	),	Z_ORDER
						);
				if (B_ORDER==5)
				convolution2D_SPM_off_NOB_zeropadded_5(
						(void*)(	(int*)spmaddrC+	 memory_offset[off_idx]	),
						(void*)(	(int*)spmaddrA+	 memory_offset[off_idx]	), 
						(void*)(	(int*)spmaddrB+  memory_offset[off_idx]	), 
						(void*)(	(int*)spmaddrD+	 memory_offset[off_idx]	),	Z_ORDER
						);
				if (B_ORDER==7)
				convolution2D_SPM_off_NOB_zeropadded_7(
						(void*)(	(int*)spmaddrC+	 memory_offset[off_idx]	),
						(void*)(	(int*)spmaddrA+	 memory_offset[off_idx]	), 
						(void*)(	(int*)spmaddrB+  memory_offset[off_idx]	), 
						(void*)(	(int*)spmaddrD+	 memory_offset[off_idx]	),	Z_ORDER
						);
				if (B_ORDER==9)
				convolution2D_SPM_off_NOB_zeropadded_9(
						(void*)(	(int*)spmaddrC+	 memory_offset[off_idx]	),
						(void*)(	(int*)spmaddrA+	 memory_offset[off_idx]	), 
						(void*)(	(int*)spmaddrB+  memory_offset[off_idx]	), 
						(void*)(	(int*)spmaddrD+	 memory_offset[off_idx]	),	Z_ORDER
						);
				if (B_ORDER==11)
				convolution2D_SPM_off_NOB_zeropadded_11(
						(void*)(	(int*)spmaddrC+	 memory_offset[off_idx]	),
						(void*)(	(int*)spmaddrA+	 memory_offset[off_idx]	), 
						(void*)(	(int*)spmaddrB+  memory_offset[off_idx]	), 
						(void*)(	(int*)spmaddrD+	 memory_offset[off_idx]	),	Z_ORDER
						);
			}
			for (int i=0; i<A_ORDER; i++){
				kmemstr((void*)((int*)output_compare_s2+ i*A_ORDER ),
								(void*)((int*)spmaddrC + (Z_ORDER + 1 + i*Z_ORDER)) ,
								SIZE_OF_INT*(A_ORDER)	);							
			}
			// DISABLE COUNTING AND SAVE MCYCLE -------------------------------------------------------
			#ifdef PRINT_NUM_CYCLES
				ptr_perf = Klessydra_perf_cnt_finish();
			#endif
			// //------------------------------------------------------------------------------------------
		}
		sync_barrier();

		for (int i=0; i<3; i++) {
			if (Klessydra_get_coreID() == i) {
				for (int j=0; j<8; j++) {
					final_perf[i][j]=ptr_perf[j];
				}
			}
    }

		//--------------------------------------CHECK RESULTS--------------------------------------------------
		sync_barrier_reset(); sync_barrier_thread_registration();
		if (Klessydra_get_coreID() == 0) {
			
			// ENABLE COUNTING -------------------------------------------------------------------------
			final_perf_mem[0]=0;
			__asm__("csrrw zero, mcycle, zero;"
					"csrrw zero, 0x7A0, 0x00000001");
			
			//------------------------------------------------------------------------------------------
			
			// DISABLE COUNTING AND SAVE MCYCLE -------------------------------------------------------
			__asm__("csrrw zero, 0x7A0, 0x00000000;"
					"csrrw %[perf_mem], mcycle, zero;"
					"sw %[perf_mem], 0(%[ptr_perf_mem]);"
					:
					:[perf_mem] "r" (perf_mem), [ptr_perf_mem] "r" (ptr_perf_mem)
					);
			final_perf_mem[0]=*(ptr_perf_mem);
			// ENABLE COUNTING -------------------------------------------------------------------------
			final_perf_mem[1]=0;
			__asm__("csrrw zero, mcycle, zero;"
					"csrrw zero, 0x7A0, 0x00000001");
			//------------------------------------------------------------------------------------------
			/*
			#if EXPLICIT == 0
				#if B_ORDER == 3
				for (int l=0; l<loop_index; l++) convolution2D_Zeropadding_3(A_ORDER, mat_second_A[0],(int*)matB, (int*)output_compare1); 
				#endif
				#if B_ORDER == 5
				for (int l=0; l<loop_index; l++) convolution2D_Zeropadding_5(A_ORDER, mat_second_A[0],(int*)matB, (int*)output_compare1); 
				#endif
				#if B_ORDER == 7
				for (int l=0; l<loop_index; l++) convolution2D_Zeropadding_7(A_ORDER, mat_second_A[0],(int*)matB, (int*)output_compare1); 
				#endif
				#if B_ORDER == 9
				for (int l=0; l<loop_index; l++) convolution2D_Zeropadding_9(A_ORDER, mat_second_A[0],(int*)matB, (int*)output_compare1); 
				#endif
				#if B_ORDER == 11
				for (int l=0; l<loop_index; l++) convolution2D_Zeropadding_11(A_ORDER, mat_second_A[0],(int*)matB, (int*)output_compare1); 
				#endif
			#endif
			
			#if EXPLICIT == 1
				#if B_ORDER == 3
				for (int l=0; l<loop_index; l++) conv2D_Zeropad_excplicit_3(A_ORDER, mat_second_A[0],(int*)matB, (int*)output_compare1); 
				#endif
				#if B_ORDER == 5
				for (int l=0; l<loop_index; l++) conv2D_Zeropad_excplicit_5(A_ORDER, mat_second_A[0],(int*)matB, (int*)output_compare1); 
				#endif
				#if B_ORDER == 7
				for (int l=0; l<loop_index; l++) conv2D_Zeropad_excplicit_7(A_ORDER, mat_second_A[0],(int*)matB, (int*)output_compare1); 
				#endif
				#if B_ORDER == 9
				for (int l=0; l<loop_index; l++) conv2D_Zeropad_excplicit_9(A_ORDER, mat_second_A[0],(int*)matB, (int*)output_compare1); 
				#endif
				#if B_ORDER == 11
				for (int l=0; l<loop_index; l++) conv2D_Zeropad_excplicit_11(A_ORDER, mat_second_A[0],(int*)matB, (int*)output_compare1); 
				#endif
			#endif
			*/
			// DISABLE COUNTING AND SAVE MCYCLE -------------------------------------------------------

			__asm__("csrrw zero, 0x7A0, 0x00000000;"
					"csrrw %[perf_mem], mcycle, zero;"
					"sw %[perf_mem], 0(%[ptr_perf_mem]);"
					:
					:[perf_mem] "r" (perf_mem), [ptr_perf_mem] "r" (ptr_perf_mem)
					);
			final_perf_mem[1]=*(ptr_perf_mem);
			//------------------------------------------------------------------------------------------
			#ifdef PRINT_NUM_CYCLES
		
				#ifdef REPLICATION
					printf("--------Test: MULTIPL£ SPMU ZEROPADDED DECOUPLED[%dx%d]--------\n", A_ORDER,A_ORDER);
				#endif			
				
				printf("N of loops:%d\n\n",loop_index);
				
				#ifndef REPLICATION
					printf("Speed of Kmemld() and prescaling from th0 data:\t%d Cycles\n", preparing_data);
				#endif
				#ifdef REPLICATION
					printf("Speed of Kbcastld() from th0 data:\t%d Cycles\n", preparing_data_kbcast_th0);
					printf("Speed of prescaling from th0 data:\t%d Cycles\n", prescaling_th0);
					printf("Speed of prescaling from th1 data:\t%d Cycles\n", prescaling_th1);
					printf("Speed of prescaling from th2 data:\t%d Cycles\n", prescaling_th2);
				#endif
				printf("\n");
				for (int i=0; i<3; i++) {
					printf(" Cycle Count = %d \n Instruction Count = %d \n Instruction wait = %d \n Load Count = %d \n Store Count = %d \n Unconditional Jump Count = %d \n Branch Count = %d \n Taken Count = %d \n \n", 
									final_perf[i][0], final_perf[i][1], final_perf[i][2], final_perf[i][3], final_perf[i][4], final_perf[i][5], final_perf[i][6], final_perf[i][7]);
				}
				printf("FM_ORDER= %d \t KM_ORDER= %d\n",A_ORDER,B_ORDER);
				printf("Convolution with Multiplier (single thread) ZEROPADDED Speed is:\t%d Cycles\n", final_perf_mem[1]);
				// printf("Convolution with Multiplier (single thread) SUBKERNELS Speed is:\t%d Cycles\n", final_perf_mem[0]);
			#endif

			#ifdef PRINT_DEBUG
				matrix_check((int*)output_compare1,(int*)output_compare_zero, A_ORDER);
			#endif

		}
		sync_barrier();
	#endif
	return 0;
}
// // --------------------------------------------------------------------------------------------------------------------------------
// // --------------------------------------------------------------------------------------------------------------------------------
#if B_ORDER == 3
	void conv2D_Zeropad_excplicit_3(int size, int (*matrix)[size], int *kernel_tmp, int *out){
	#define KERN_EXPLICIT_3 3
	#define ZEROPAD_EXPLICIT_struct_3 2
	#define ZEROPAD_EXPLICIT_offset_3 1
	int print=0;
	int kernel[KERN_EXPLICIT_3*KERN_EXPLICIT_3];
	int conv2D_scaling_factor=8;
	int conv2D_out_scal=5;
	for(int i=0;i<KERN_EXPLICIT_3*KERN_EXPLICIT_3;i++){
    	kernel[i]=(kernel_tmp[i]>>conv2D_scaling_factor);
	}
	int i, j;
	int pt=0;
	int zeropad[Z_ORDER][Z_ORDER]={0};
	for (int i=ZEROPAD_EXPLICIT_offset_3; i< size+ZEROPAD_EXPLICIT_struct_3-ZEROPAD_EXPLICIT_offset_3; i++)
		for (int j=ZEROPAD_EXPLICIT_offset_3; j< size+ZEROPAD_EXPLICIT_struct_3-ZEROPAD_EXPLICIT_offset_3; j++)
			zeropad[i][j]=matrix[i-ZEROPAD_EXPLICIT_offset_3][j-ZEROPAD_EXPLICIT_offset_3];

	for (i = 1; i < (size+(2))-(1); i++)                                                          
	{                                                                                         
		for (j = (1); j < (size+(2))-(1); j++)                                                          
		{                                                                                         
			pt=(i+(-1))*size+(j+(-1));                                                                      
			out[pt] +=	                                                                              
			(	(	(zeropad	[i+(-1)][j+(-1)]		>>conv2D_scaling_factor)	* kernel[0])>>		conv2D_out_scal)    +   
			(	(	(zeropad	[i+(-1)][j+(0)]		>>conv2D_scaling_factor)	* kernel[1])>>		conv2D_out_scal)    +   
			(	(	(zeropad	[i+(-1)][j+(1)]		>>conv2D_scaling_factor)	* kernel[2])>>		conv2D_out_scal)    +   
			(	(	(zeropad	[i+(0)][j+(-1)]		>>conv2D_scaling_factor)	* kernel[3])>>		conv2D_out_scal)    +   
			(	(	(zeropad	[i+(0)][j+(0)]		>>conv2D_scaling_factor)	* kernel[4])>>		conv2D_out_scal)    +   
			(	(	(zeropad	[i+(0)][j+(1)]		>>conv2D_scaling_factor)	* kernel[5])>>		conv2D_out_scal)    +   
			(	(	(zeropad	[i+(1)][j+(-1)]		>>conv2D_scaling_factor)	* kernel[6])>>		conv2D_out_scal)    +   
			(	(	(zeropad	[i+(1)][j+(0)]		>>conv2D_scaling_factor)	* kernel[7])>>		conv2D_out_scal)    +   
			(	(	(zeropad	[i+(1)][j+(1)]		>>conv2D_scaling_factor)	* kernel[8])>>		conv2D_out_scal)   	;   
		}                                                                                         
	}                                                                                         
}
#endif
#if B_ORDER == 5
	void conv2D_Zeropad_excplicit_5(int size, int (*matrix)[size], int *kernel_tmp, int *out){
	#define KERN_EXPLICIT_5 5
	#define ZEROPAD_EXPLICIT_struct_5 4
	#define ZEROPAD_EXPLICIT_offset_5 2
	int print=0;
	int kernel[KERN_EXPLICIT_5*KERN_EXPLICIT_5];
	int conv2D_scaling_factor=8;
	int conv2D_out_scal=5;
	for(int i=0;i<KERN_EXPLICIT_5*KERN_EXPLICIT_5;i++){
    	kernel[i]=(kernel_tmp[i]>>conv2D_scaling_factor);
	}
	int i, j;
	int pt=0;
	int zeropad[Z_ORDER][Z_ORDER]={0};
	for (int i=ZEROPAD_EXPLICIT_offset_5; i< size+ZEROPAD_EXPLICIT_struct_5-ZEROPAD_EXPLICIT_offset_5; i++)
		for (int j=ZEROPAD_EXPLICIT_offset_5; j< size+ZEROPAD_EXPLICIT_struct_5-ZEROPAD_EXPLICIT_offset_5; j++)
			zeropad[i][j]=matrix[i-ZEROPAD_EXPLICIT_offset_5][j-ZEROPAD_EXPLICIT_offset_5];

	for (i = 2; i < (size+(4))-(2); i++)                                                          
	{                                                                                         
		for (j = (2); j < (size+(4))-(2); j++)                                                          
		{                                                                                         
			pt=(i+(-2))*size+(j+(-2));                                                                      
			out[pt] +=	                                                                              
			(	(	(zeropad	[i+(-2)][j+(-2)]		>>conv2D_scaling_factor)	* kernel[0])>>		conv2D_out_scal)    +   
			(	(	(zeropad	[i+(-2)][j+(-1)]		>>conv2D_scaling_factor)	* kernel[1])>>		conv2D_out_scal)    +   
			(	(	(zeropad	[i+(-2)][j+(0)]		>>conv2D_scaling_factor)	* kernel[2])>>		conv2D_out_scal)    +   
			(	(	(zeropad	[i+(-2)][j+(1)]		>>conv2D_scaling_factor)	* kernel[3])>>		conv2D_out_scal)    +   
			(	(	(zeropad	[i+(-2)][j+(2)]		>>conv2D_scaling_factor)	* kernel[4])>>		conv2D_out_scal)    +   
			(	(	(zeropad	[i+(-1)][j+(-2)]		>>conv2D_scaling_factor)	* kernel[5])>>		conv2D_out_scal)    +   
			(	(	(zeropad	[i+(-1)][j+(-1)]		>>conv2D_scaling_factor)	* kernel[6])>>		conv2D_out_scal)    +   
			(	(	(zeropad	[i+(-1)][j+(0)]		>>conv2D_scaling_factor)	* kernel[7])>>		conv2D_out_scal)    +   
			(	(	(zeropad	[i+(-1)][j+(1)]		>>conv2D_scaling_factor)	* kernel[8])>>		conv2D_out_scal)    +   
			(	(	(zeropad	[i+(-1)][j+(2)]		>>conv2D_scaling_factor)	* kernel[9])>>		conv2D_out_scal)    +   
			(	(	(zeropad	[i+(0)][j+(-2)]		>>conv2D_scaling_factor)	* kernel[10])>>		conv2D_out_scal)    +   
			(	(	(zeropad	[i+(0)][j+(-1)]		>>conv2D_scaling_factor)	* kernel[11])>>		conv2D_out_scal)    +   
			(	(	(zeropad	[i+(0)][j+(0)]		>>conv2D_scaling_factor)	* kernel[12])>>		conv2D_out_scal)    +   
			(	(	(zeropad	[i+(0)][j+(1)]		>>conv2D_scaling_factor)	* kernel[13])>>		conv2D_out_scal)    +   
			(	(	(zeropad	[i+(0)][j+(2)]		>>conv2D_scaling_factor)	* kernel[14])>>		conv2D_out_scal)    +   
			(	(	(zeropad	[i+(1)][j+(-2)]		>>conv2D_scaling_factor)	* kernel[15])>>		conv2D_out_scal)    +   
			(	(	(zeropad	[i+(1)][j+(-1)]		>>conv2D_scaling_factor)	* kernel[16])>>		conv2D_out_scal)    +   
			(	(	(zeropad	[i+(1)][j+(0)]		>>conv2D_scaling_factor)	* kernel[17])>>		conv2D_out_scal)    +   
			(	(	(zeropad	[i+(1)][j+(1)]		>>conv2D_scaling_factor)	* kernel[18])>>		conv2D_out_scal)    +   
			(	(	(zeropad	[i+(1)][j+(2)]		>>conv2D_scaling_factor)	* kernel[19])>>		conv2D_out_scal)    +   
			(	(	(zeropad	[i+(2)][j+(-2)]		>>conv2D_scaling_factor)	* kernel[20])>>		conv2D_out_scal)    +   
			(	(	(zeropad	[i+(2)][j+(-1)]		>>conv2D_scaling_factor)	* kernel[21])>>		conv2D_out_scal)    +   
			(	(	(zeropad	[i+(2)][j+(0)]		>>conv2D_scaling_factor)	* kernel[22])>>		conv2D_out_scal)    +   
			(	(	(zeropad	[i+(2)][j+(1)]		>>conv2D_scaling_factor)	* kernel[23])>>		conv2D_out_scal)    +   
			(	(	(zeropad	[i+(2)][j+(2)]		>>conv2D_scaling_factor)	* kernel[24])>>		conv2D_out_scal)    ;   
		}                                                                                         
	}                                                                                                                                                                                                                                                     
}
#endif
#if B_ORDER == 7
	void conv2D_Zeropad_excplicit_7(int size, int (*matrix)[size], int *kernel_tmp, int *out){									
	#define KERN_EXPLICIT_7 7                                                                                 
	#define ZEROPAD_EXPLICIT_struct_7 6                                                                       
	#define ZEROPAD_EXPLICIT_offset_7 3                                                                       
	int print=0;                                                                                              	
	int kernel[KERN_EXPLICIT_7*KERN_EXPLICIT_7];                                                              
	int conv2D_scaling_factor=8;                                                                              	
	int conv2D_out_scal=5;                                                                                    	
	for(int i=0;i<KERN_EXPLICIT_7*KERN_EXPLICIT_7;i++){                                                       
		kernel[i]=(kernel_tmp[i]>>conv2D_scaling_factor);                                                       	
	}                                                                                                         	
	int i, j;                                                                                                 	
	int pt=0;                                                                                                 	
	int zeropad[Z_ORDER][Z_ORDER]={0};                                                                        	
	for (int i=ZEROPAD_EXPLICIT_offset_7; i< size+ZEROPAD_EXPLICIT_struct_7-ZEROPAD_EXPLICIT_offset_7; i++)  
		for (int j=ZEROPAD_EXPLICIT_offset_7; j< size+ZEROPAD_EXPLICIT_struct_7-ZEROPAD_EXPLICIT_offset_7; j++)
			zeropad[i][j]=matrix[i-ZEROPAD_EXPLICIT_offset_7][j-ZEROPAD_EXPLICIT_offset_7];                       
	for (i = 3; i < (size+(6))-(3); i++)                                                          
	{                                                                                         
		for (j = (3); j < (size+(6))-(3); j++)                                                          
		{                                                                                         
			pt=(i+(-3))*size+(j+(-3));                                                                      
			out[pt] +=	                                                                              
		(	(	(zeropad	[i+(-3)][j+(-3)]		>>conv2D_scaling_factor)	* kernel[0])>>		conv2D_out_scal)    +   
		(	(	(zeropad	[i+(-3)][j+(-2)]		>>conv2D_scaling_factor)	* kernel[1])>>		conv2D_out_scal)    +   
		(	(	(zeropad	[i+(-3)][j+(-1)]		>>conv2D_scaling_factor)	* kernel[2])>>		conv2D_out_scal)    +   
		(	(	(zeropad	[i+(-3)][j+(0)]		>>conv2D_scaling_factor)	* kernel[3])>>		conv2D_out_scal)    +   
		(	(	(zeropad	[i+(-3)][j+(1)]		>>conv2D_scaling_factor)	* kernel[4])>>		conv2D_out_scal)    +   
		(	(	(zeropad	[i+(-3)][j+(2)]		>>conv2D_scaling_factor)	* kernel[5])>>		conv2D_out_scal)    +   
		(	(	(zeropad	[i+(-3)][j+(3)]		>>conv2D_scaling_factor)	* kernel[6])>>		conv2D_out_scal)    +   
		(	(	(zeropad	[i+(-2)][j+(-3)]		>>conv2D_scaling_factor)	* kernel[7])>>		conv2D_out_scal)    +   
		(	(	(zeropad	[i+(-2)][j+(-2)]		>>conv2D_scaling_factor)	* kernel[8])>>		conv2D_out_scal)    +   
		(	(	(zeropad	[i+(-2)][j+(-1)]		>>conv2D_scaling_factor)	* kernel[9])>>		conv2D_out_scal)    +   
		(	(	(zeropad	[i+(-2)][j+(0)]		>>conv2D_scaling_factor)	* kernel[10])>>		conv2D_out_scal)    +   
		(	(	(zeropad	[i+(-2)][j+(1)]		>>conv2D_scaling_factor)	* kernel[11])>>		conv2D_out_scal)    +   
		(	(	(zeropad	[i+(-2)][j+(2)]		>>conv2D_scaling_factor)	* kernel[12])>>		conv2D_out_scal)    +   
		(	(	(zeropad	[i+(-2)][j+(3)]		>>conv2D_scaling_factor)	* kernel[13])>>		conv2D_out_scal)    +   
		(	(	(zeropad	[i+(-1)][j+(-3)]		>>conv2D_scaling_factor)	* kernel[14])>>		conv2D_out_scal)    +   
		(	(	(zeropad	[i+(-1)][j+(-2)]		>>conv2D_scaling_factor)	* kernel[15])>>		conv2D_out_scal)    +   
		(	(	(zeropad	[i+(-1)][j+(-1)]		>>conv2D_scaling_factor)	* kernel[16])>>		conv2D_out_scal)    +   
		(	(	(zeropad	[i+(-1)][j+(0)]		>>conv2D_scaling_factor)	* kernel[17])>>		conv2D_out_scal)    +   
		(	(	(zeropad	[i+(-1)][j+(1)]		>>conv2D_scaling_factor)	* kernel[18])>>		conv2D_out_scal)    +   
		(	(	(zeropad	[i+(-1)][j+(2)]		>>conv2D_scaling_factor)	* kernel[19])>>		conv2D_out_scal)    +   
		(	(	(zeropad	[i+(-1)][j+(3)]		>>conv2D_scaling_factor)	* kernel[20])>>		conv2D_out_scal)    +   
		(	(	(zeropad	[i+(0)][j+(-3)]		>>conv2D_scaling_factor)	* kernel[21])>>		conv2D_out_scal)    +   
		(	(	(zeropad	[i+(0)][j+(-2)]		>>conv2D_scaling_factor)	* kernel[22])>>		conv2D_out_scal)    +   
		(	(	(zeropad	[i+(0)][j+(-1)]		>>conv2D_scaling_factor)	* kernel[23])>>		conv2D_out_scal)    +   
		(	(	(zeropad	[i+(0)][j+(0)]		>>conv2D_scaling_factor)	* kernel[24])>>		conv2D_out_scal)    +   
		(	(	(zeropad	[i+(0)][j+(1)]		>>conv2D_scaling_factor)	* kernel[25])>>		conv2D_out_scal)    +   
		(	(	(zeropad	[i+(0)][j+(2)]		>>conv2D_scaling_factor)	* kernel[26])>>		conv2D_out_scal)    +   
		(	(	(zeropad	[i+(0)][j+(3)]		>>conv2D_scaling_factor)	* kernel[27])>>		conv2D_out_scal)    +   
		(	(	(zeropad	[i+(1)][j+(-3)]		>>conv2D_scaling_factor)	* kernel[28])>>		conv2D_out_scal)    +   
		(	(	(zeropad	[i+(1)][j+(-2)]		>>conv2D_scaling_factor)	* kernel[29])>>		conv2D_out_scal)    +   
		(	(	(zeropad	[i+(1)][j+(-1)]		>>conv2D_scaling_factor)	* kernel[30])>>		conv2D_out_scal)    +   
		(	(	(zeropad	[i+(1)][j+(0)]		>>conv2D_scaling_factor)	* kernel[31])>>		conv2D_out_scal)    +   
		(	(	(zeropad	[i+(1)][j+(1)]		>>conv2D_scaling_factor)	* kernel[32])>>		conv2D_out_scal)    +   
		(	(	(zeropad	[i+(1)][j+(2)]		>>conv2D_scaling_factor)	* kernel[33])>>		conv2D_out_scal)    +   
		(	(	(zeropad	[i+(1)][j+(3)]		>>conv2D_scaling_factor)	* kernel[34])>>		conv2D_out_scal)    +   
		(	(	(zeropad	[i+(2)][j+(-3)]		>>conv2D_scaling_factor)	* kernel[35])>>		conv2D_out_scal)    +   
		(	(	(zeropad	[i+(2)][j+(-2)]		>>conv2D_scaling_factor)	* kernel[36])>>		conv2D_out_scal)    +   
		(	(	(zeropad	[i+(2)][j+(-1)]		>>conv2D_scaling_factor)	* kernel[37])>>		conv2D_out_scal)    +   
		(	(	(zeropad	[i+(2)][j+(0)]		>>conv2D_scaling_factor)	* kernel[38])>>		conv2D_out_scal)    +   
		(	(	(zeropad	[i+(2)][j+(1)]		>>conv2D_scaling_factor)	* kernel[39])>>		conv2D_out_scal)    +   
		(	(	(zeropad	[i+(2)][j+(2)]		>>conv2D_scaling_factor)	* kernel[40])>>		conv2D_out_scal)    +   
		(	(	(zeropad	[i+(2)][j+(3)]		>>conv2D_scaling_factor)	* kernel[41])>>		conv2D_out_scal)    +   
		(	(	(zeropad	[i+(3)][j+(-3)]		>>conv2D_scaling_factor)	* kernel[42])>>		conv2D_out_scal)    +   
		(	(	(zeropad	[i+(3)][j+(-2)]		>>conv2D_scaling_factor)	* kernel[43])>>		conv2D_out_scal)    +   
		(	(	(zeropad	[i+(3)][j+(-1)]		>>conv2D_scaling_factor)	* kernel[44])>>		conv2D_out_scal)    +   
		(	(	(zeropad	[i+(3)][j+(0)]		>>conv2D_scaling_factor)	* kernel[45])>>		conv2D_out_scal)    +   
		(	(	(zeropad	[i+(3)][j+(1)]		>>conv2D_scaling_factor)	* kernel[46])>>		conv2D_out_scal)    +   
		(	(	(zeropad	[i+(3)][j+(2)]		>>conv2D_scaling_factor)	* kernel[47])>>		conv2D_out_scal)    +   
		(	(	(zeropad	[i+(3)][j+(3)]		>>conv2D_scaling_factor)	* kernel[48])>>		conv2D_out_scal)    ;   
		}                                                                                         
	}                                                                                         
}// // --------------------------------------------------------------------------------------------------------------------------------
#endif
#if B_ORDER == 9
	void conv2D_Zeropad_excplicit_9(int size, int (*matrix)[size], int *kernel_tmp, int *out){									
	#define KERN_EXPLICIT_9 9                                                                                 
	#define ZEROPAD_EXPLICIT_struct_9 8                                                                       
	#define ZEROPAD_EXPLICIT_offset_9 4                                                                       
	int print=0;                                                                                              	
	int kernel[KERN_EXPLICIT_9*KERN_EXPLICIT_9];                                                              
	int conv2D_scaling_factor=8;                                                                              	
	int conv2D_out_scal=5;                                                                                    	
	for(int i=0;i<KERN_EXPLICIT_9*KERN_EXPLICIT_9;i++){                                                       
		kernel[i]=(kernel_tmp[i]>>conv2D_scaling_factor);                                                       	
	}                                                                                                         	
	int i, j;                                                                                                 	
	int pt=0;                                                                                                 	
	int zeropad[Z_ORDER][Z_ORDER]={0};                                                                        	
	for (int i=ZEROPAD_EXPLICIT_offset_9; i< size+ZEROPAD_EXPLICIT_struct_9-ZEROPAD_EXPLICIT_offset_9; i++)  
		for (int j=ZEROPAD_EXPLICIT_offset_9; j< size+ZEROPAD_EXPLICIT_struct_9-ZEROPAD_EXPLICIT_offset_9; j++)
			zeropad[i][j]=matrix[i-ZEROPAD_EXPLICIT_offset_9][j-ZEROPAD_EXPLICIT_offset_9];                       
	for (i = 4; i < (size+(8))-(4); i++)                                                          
	{                                                                                         
		for (j = (4); j < (size+(8))-(4); j++)                                                          
		{                                                                                         
			pt=(i+(-4))*size+(j+(-4));                                                                      
			out[pt] +=	                                                                              
		(	(	(zeropad	[i+(-4)][j+(-4)]		>>conv2D_scaling_factor)	* kernel[0])>>		conv2D_out_scal)    +   
		(	(	(zeropad	[i+(-4)][j+(-3)]		>>conv2D_scaling_factor)	* kernel[1])>>		conv2D_out_scal)    +   
		(	(	(zeropad	[i+(-4)][j+(-2)]		>>conv2D_scaling_factor)	* kernel[2])>>		conv2D_out_scal)    +   
		(	(	(zeropad	[i+(-4)][j+(-1)]		>>conv2D_scaling_factor)	* kernel[3])>>		conv2D_out_scal)    +   
		(	(	(zeropad	[i+(-4)][j+(0)]		>>conv2D_scaling_factor)	* kernel[4])>>		conv2D_out_scal)    +   
		(	(	(zeropad	[i+(-4)][j+(1)]		>>conv2D_scaling_factor)	* kernel[5])>>		conv2D_out_scal)    +   
		(	(	(zeropad	[i+(-4)][j+(2)]		>>conv2D_scaling_factor)	* kernel[6])>>		conv2D_out_scal)    +   
		(	(	(zeropad	[i+(-4)][j+(3)]		>>conv2D_scaling_factor)	* kernel[7])>>		conv2D_out_scal)    +   
		(	(	(zeropad	[i+(-4)][j+(4)]		>>conv2D_scaling_factor)	* kernel[8])>>		conv2D_out_scal)    +   
		(	(	(zeropad	[i+(-3)][j+(-4)]		>>conv2D_scaling_factor)	* kernel[9])>>		conv2D_out_scal)    +   
		(	(	(zeropad	[i+(-3)][j+(-3)]		>>conv2D_scaling_factor)	* kernel[10])>>		conv2D_out_scal)    +   
		(	(	(zeropad	[i+(-3)][j+(-2)]		>>conv2D_scaling_factor)	* kernel[11])>>		conv2D_out_scal)    +   
		(	(	(zeropad	[i+(-3)][j+(-1)]		>>conv2D_scaling_factor)	* kernel[12])>>		conv2D_out_scal)    +   
		(	(	(zeropad	[i+(-3)][j+(0)]		>>conv2D_scaling_factor)	* kernel[13])>>		conv2D_out_scal)    +   
		(	(	(zeropad	[i+(-3)][j+(1)]		>>conv2D_scaling_factor)	* kernel[14])>>		conv2D_out_scal)    +   
		(	(	(zeropad	[i+(-3)][j+(2)]		>>conv2D_scaling_factor)	* kernel[15])>>		conv2D_out_scal)    +   
		(	(	(zeropad	[i+(-3)][j+(3)]		>>conv2D_scaling_factor)	* kernel[16])>>		conv2D_out_scal)    +   
		(	(	(zeropad	[i+(-3)][j+(4)]		>>conv2D_scaling_factor)	* kernel[17])>>		conv2D_out_scal)    +   
		(	(	(zeropad	[i+(-2)][j+(-4)]		>>conv2D_scaling_factor)	* kernel[18])>>		conv2D_out_scal)    +   
		(	(	(zeropad	[i+(-2)][j+(-3)]		>>conv2D_scaling_factor)	* kernel[19])>>		conv2D_out_scal)    +   
		(	(	(zeropad	[i+(-2)][j+(-2)]		>>conv2D_scaling_factor)	* kernel[20])>>		conv2D_out_scal)    +   
		(	(	(zeropad	[i+(-2)][j+(-1)]		>>conv2D_scaling_factor)	* kernel[21])>>		conv2D_out_scal)    +   
		(	(	(zeropad	[i+(-2)][j+(0)]		>>conv2D_scaling_factor)	* kernel[22])>>		conv2D_out_scal)    +   
		(	(	(zeropad	[i+(-2)][j+(1)]		>>conv2D_scaling_factor)	* kernel[23])>>		conv2D_out_scal)    +   
		(	(	(zeropad	[i+(-2)][j+(2)]		>>conv2D_scaling_factor)	* kernel[24])>>		conv2D_out_scal)    +   
		(	(	(zeropad	[i+(-2)][j+(3)]		>>conv2D_scaling_factor)	* kernel[25])>>		conv2D_out_scal)    +   
		(	(	(zeropad	[i+(-2)][j+(4)]		>>conv2D_scaling_factor)	* kernel[26])>>		conv2D_out_scal)    +   
		(	(	(zeropad	[i+(-1)][j+(-4)]		>>conv2D_scaling_factor)	* kernel[27])>>		conv2D_out_scal)    +   
		(	(	(zeropad	[i+(-1)][j+(-3)]		>>conv2D_scaling_factor)	* kernel[28])>>		conv2D_out_scal)    +   
		(	(	(zeropad	[i+(-1)][j+(-2)]		>>conv2D_scaling_factor)	* kernel[29])>>		conv2D_out_scal)    +   
		(	(	(zeropad	[i+(-1)][j+(-1)]		>>conv2D_scaling_factor)	* kernel[30])>>		conv2D_out_scal)    +   
		(	(	(zeropad	[i+(-1)][j+(0)]		>>conv2D_scaling_factor)	* kernel[31])>>		conv2D_out_scal)    +   
		(	(	(zeropad	[i+(-1)][j+(1)]		>>conv2D_scaling_factor)	* kernel[32])>>		conv2D_out_scal)    +   
		(	(	(zeropad	[i+(-1)][j+(2)]		>>conv2D_scaling_factor)	* kernel[33])>>		conv2D_out_scal)    +   
		(	(	(zeropad	[i+(-1)][j+(3)]		>>conv2D_scaling_factor)	* kernel[34])>>		conv2D_out_scal)    +   
		(	(	(zeropad	[i+(-1)][j+(4)]		>>conv2D_scaling_factor)	* kernel[35])>>		conv2D_out_scal)    +   
		(	(	(zeropad	[i+(0)][j+(-4)]		>>conv2D_scaling_factor)	* kernel[36])>>		conv2D_out_scal)    +   
		(	(	(zeropad	[i+(0)][j+(-3)]		>>conv2D_scaling_factor)	* kernel[37])>>		conv2D_out_scal)    +   
		(	(	(zeropad	[i+(0)][j+(-2)]		>>conv2D_scaling_factor)	* kernel[38])>>		conv2D_out_scal)    +   
		(	(	(zeropad	[i+(0)][j+(-1)]		>>conv2D_scaling_factor)	* kernel[39])>>		conv2D_out_scal)    +   
		(	(	(zeropad	[i+(0)][j+(0)]		>>conv2D_scaling_factor)	* kernel[40])>>		conv2D_out_scal)    +   
		(	(	(zeropad	[i+(0)][j+(1)]		>>conv2D_scaling_factor)	* kernel[41])>>		conv2D_out_scal)    +   
		(	(	(zeropad	[i+(0)][j+(2)]		>>conv2D_scaling_factor)	* kernel[42])>>		conv2D_out_scal)    +   
		(	(	(zeropad	[i+(0)][j+(3)]		>>conv2D_scaling_factor)	* kernel[43])>>		conv2D_out_scal)    +   
		(	(	(zeropad	[i+(0)][j+(4)]		>>conv2D_scaling_factor)	* kernel[44])>>		conv2D_out_scal)    +   
		(	(	(zeropad	[i+(1)][j+(-4)]		>>conv2D_scaling_factor)	* kernel[45])>>		conv2D_out_scal)    +   
		(	(	(zeropad	[i+(1)][j+(-3)]		>>conv2D_scaling_factor)	* kernel[46])>>		conv2D_out_scal)    +   
		(	(	(zeropad	[i+(1)][j+(-2)]		>>conv2D_scaling_factor)	* kernel[47])>>		conv2D_out_scal)    +   
		(	(	(zeropad	[i+(1)][j+(-1)]		>>conv2D_scaling_factor)	* kernel[48])>>		conv2D_out_scal)    +   
		(	(	(zeropad	[i+(1)][j+(0)]		>>conv2D_scaling_factor)	* kernel[49])>>		conv2D_out_scal)    +   
		(	(	(zeropad	[i+(1)][j+(1)]		>>conv2D_scaling_factor)	* kernel[50])>>		conv2D_out_scal)    +   
		(	(	(zeropad	[i+(1)][j+(2)]		>>conv2D_scaling_factor)	* kernel[51])>>		conv2D_out_scal)    +   
		(	(	(zeropad	[i+(1)][j+(3)]		>>conv2D_scaling_factor)	* kernel[52])>>		conv2D_out_scal)    +   
		(	(	(zeropad	[i+(1)][j+(4)]		>>conv2D_scaling_factor)	* kernel[53])>>		conv2D_out_scal)    +   
		(	(	(zeropad	[i+(2)][j+(-4)]		>>conv2D_scaling_factor)	* kernel[54])>>		conv2D_out_scal)    +   
		(	(	(zeropad	[i+(2)][j+(-3)]		>>conv2D_scaling_factor)	* kernel[55])>>		conv2D_out_scal)    +   
		(	(	(zeropad	[i+(2)][j+(-2)]		>>conv2D_scaling_factor)	* kernel[56])>>		conv2D_out_scal)    +   
		(	(	(zeropad	[i+(2)][j+(-1)]		>>conv2D_scaling_factor)	* kernel[57])>>		conv2D_out_scal)    +   
		(	(	(zeropad	[i+(2)][j+(0)]		>>conv2D_scaling_factor)	* kernel[58])>>		conv2D_out_scal)    +   
		(	(	(zeropad	[i+(2)][j+(1)]		>>conv2D_scaling_factor)	* kernel[59])>>		conv2D_out_scal)    +   
		(	(	(zeropad	[i+(2)][j+(2)]		>>conv2D_scaling_factor)	* kernel[60])>>		conv2D_out_scal)    +   
		(	(	(zeropad	[i+(2)][j+(3)]		>>conv2D_scaling_factor)	* kernel[61])>>		conv2D_out_scal)    +   
		(	(	(zeropad	[i+(2)][j+(4)]		>>conv2D_scaling_factor)	* kernel[62])>>		conv2D_out_scal)    +   
		(	(	(zeropad	[i+(3)][j+(-4)]		>>conv2D_scaling_factor)	* kernel[63])>>		conv2D_out_scal)    +   
		(	(	(zeropad	[i+(3)][j+(-3)]		>>conv2D_scaling_factor)	* kernel[64])>>		conv2D_out_scal)    +   
		(	(	(zeropad	[i+(3)][j+(-2)]		>>conv2D_scaling_factor)	* kernel[65])>>		conv2D_out_scal)    +   
		(	(	(zeropad	[i+(3)][j+(-1)]		>>conv2D_scaling_factor)	* kernel[66])>>		conv2D_out_scal)    +   
		(	(	(zeropad	[i+(3)][j+(0)]		>>conv2D_scaling_factor)	* kernel[67])>>		conv2D_out_scal)    +   
		(	(	(zeropad	[i+(3)][j+(1)]		>>conv2D_scaling_factor)	* kernel[68])>>		conv2D_out_scal)    +   
		(	(	(zeropad	[i+(3)][j+(2)]		>>conv2D_scaling_factor)	* kernel[69])>>		conv2D_out_scal)    +   
		(	(	(zeropad	[i+(3)][j+(3)]		>>conv2D_scaling_factor)	* kernel[70])>>		conv2D_out_scal)    +   
		(	(	(zeropad	[i+(3)][j+(4)]		>>conv2D_scaling_factor)	* kernel[71])>>		conv2D_out_scal)    +   
		(	(	(zeropad	[i+(4)][j+(-4)]		>>conv2D_scaling_factor)	* kernel[72])>>		conv2D_out_scal)    +   
		(	(	(zeropad	[i+(4)][j+(-3)]		>>conv2D_scaling_factor)	* kernel[73])>>		conv2D_out_scal)    +   
		(	(	(zeropad	[i+(4)][j+(-2)]		>>conv2D_scaling_factor)	* kernel[74])>>		conv2D_out_scal)    +   
		(	(	(zeropad	[i+(4)][j+(-1)]		>>conv2D_scaling_factor)	* kernel[75])>>		conv2D_out_scal)    +   
		(	(	(zeropad	[i+(4)][j+(0)]		>>conv2D_scaling_factor)	* kernel[76])>>		conv2D_out_scal)    +   
		(	(	(zeropad	[i+(4)][j+(1)]		>>conv2D_scaling_factor)	* kernel[77])>>		conv2D_out_scal)    +   
		(	(	(zeropad	[i+(4)][j+(2)]		>>conv2D_scaling_factor)	* kernel[78])>>		conv2D_out_scal)    +   
		(	(	(zeropad	[i+(4)][j+(3)]		>>conv2D_scaling_factor)	* kernel[79])>>		conv2D_out_scal)    +   
		(	(	(zeropad	[i+(4)][j+(4)]		>>conv2D_scaling_factor)	* kernel[80])>>		conv2D_out_scal)    ;   
		}                                                                                         
	}                                                                                         
}                                                                                         
#endif
#if B_ORDER == 11
	void conv2D_Zeropad_excplicit_11(int size, int (*matrix)[size], int *kernel_tmp, int *out){									
	#define KERN_EXPLICIT_11 11                                                                                 
	#define ZEROPAD_EXPLICIT_struct_11 10                                                                       
	#define ZEROPAD_EXPLICIT_offset_11 5                                                                       
	int print=0;                                                                                              	
	int kernel[KERN_EXPLICIT_11*KERN_EXPLICIT_11];                                                              
	int conv2D_scaling_factor=8;                                                                              	
	int conv2D_out_scal=5;                                                                                    	
	for(int i=0;i<KERN_EXPLICIT_11*KERN_EXPLICIT_11;i++){                                                       
		kernel[i]=(kernel_tmp[i]>>conv2D_scaling_factor);                                                       	
	}                                                                                                         	
	int i, j;                                                                                                 	
	int pt=0;                                                                                                 	
	int zeropad[Z_ORDER][Z_ORDER]={0};                                                                        	
	for (int i=ZEROPAD_EXPLICIT_offset_11; i< size+ZEROPAD_EXPLICIT_struct_11-ZEROPAD_EXPLICIT_offset_11; i++)  
		for (int j=ZEROPAD_EXPLICIT_offset_11; j< size+ZEROPAD_EXPLICIT_struct_11-ZEROPAD_EXPLICIT_offset_11; j++)
			zeropad[i][j]=matrix[i-ZEROPAD_EXPLICIT_offset_11][j-ZEROPAD_EXPLICIT_offset_11];                       
	for (i = 5; i < (size+(10))-(5); i++)                                                          
	{                                                                                         
		for (j = (5); j < (size+(10))-(5); j++)                                                          
		{                                                                                         
			pt=(i+(-5))*size+(j+(-5));                                                                      
			out[pt] +=	                                                                              
		(	(	(zeropad	[i+(-5)][j+(-5)]		>>conv2D_scaling_factor)	* kernel[0])>>		conv2D_out_scal)    +   
		(	(	(zeropad	[i+(-5)][j+(-4)]		>>conv2D_scaling_factor)	* kernel[1])>>		conv2D_out_scal)    +   
		(	(	(zeropad	[i+(-5)][j+(-3)]		>>conv2D_scaling_factor)	* kernel[2])>>		conv2D_out_scal)    +   
		(	(	(zeropad	[i+(-5)][j+(-2)]		>>conv2D_scaling_factor)	* kernel[3])>>		conv2D_out_scal)    +   
		(	(	(zeropad	[i+(-5)][j+(-1)]		>>conv2D_scaling_factor)	* kernel[4])>>		conv2D_out_scal)    +   
		(	(	(zeropad	[i+(-5)][j+(0)]		>>conv2D_scaling_factor)	* kernel[5])>>		conv2D_out_scal)    +   
		(	(	(zeropad	[i+(-5)][j+(1)]		>>conv2D_scaling_factor)	* kernel[6])>>		conv2D_out_scal)    +   
		(	(	(zeropad	[i+(-5)][j+(2)]		>>conv2D_scaling_factor)	* kernel[7])>>		conv2D_out_scal)    +   
		(	(	(zeropad	[i+(-5)][j+(3)]		>>conv2D_scaling_factor)	* kernel[8])>>		conv2D_out_scal)    +   
		(	(	(zeropad	[i+(-5)][j+(4)]		>>conv2D_scaling_factor)	* kernel[9])>>		conv2D_out_scal)    +   
		(	(	(zeropad	[i+(-5)][j+(5)]		>>conv2D_scaling_factor)	* kernel[10])>>		conv2D_out_scal)    +   
		(	(	(zeropad	[i+(-4)][j+(-5)]		>>conv2D_scaling_factor)	* kernel[11])>>		conv2D_out_scal)    +   
		(	(	(zeropad	[i+(-4)][j+(-4)]		>>conv2D_scaling_factor)	* kernel[12])>>		conv2D_out_scal)    +   
		(	(	(zeropad	[i+(-4)][j+(-3)]		>>conv2D_scaling_factor)	* kernel[13])>>		conv2D_out_scal)    +   
		(	(	(zeropad	[i+(-4)][j+(-2)]		>>conv2D_scaling_factor)	* kernel[14])>>		conv2D_out_scal)    +   
		(	(	(zeropad	[i+(-4)][j+(-1)]		>>conv2D_scaling_factor)	* kernel[15])>>		conv2D_out_scal)    +   
		(	(	(zeropad	[i+(-4)][j+(0)]		>>conv2D_scaling_factor)	* kernel[16])>>		conv2D_out_scal)    +   
		(	(	(zeropad	[i+(-4)][j+(1)]		>>conv2D_scaling_factor)	* kernel[17])>>		conv2D_out_scal)    +   
		(	(	(zeropad	[i+(-4)][j+(2)]		>>conv2D_scaling_factor)	* kernel[18])>>		conv2D_out_scal)    +   
		(	(	(zeropad	[i+(-4)][j+(3)]		>>conv2D_scaling_factor)	* kernel[19])>>		conv2D_out_scal)    +   
		(	(	(zeropad	[i+(-4)][j+(4)]		>>conv2D_scaling_factor)	* kernel[20])>>		conv2D_out_scal)    +   
		(	(	(zeropad	[i+(-4)][j+(5)]		>>conv2D_scaling_factor)	* kernel[21])>>		conv2D_out_scal)    +   
		(	(	(zeropad	[i+(-3)][j+(-5)]		>>conv2D_scaling_factor)	* kernel[22])>>		conv2D_out_scal)    +   
		(	(	(zeropad	[i+(-3)][j+(-4)]		>>conv2D_scaling_factor)	* kernel[23])>>		conv2D_out_scal)    +   
		(	(	(zeropad	[i+(-3)][j+(-3)]		>>conv2D_scaling_factor)	* kernel[24])>>		conv2D_out_scal)    +   
		(	(	(zeropad	[i+(-3)][j+(-2)]		>>conv2D_scaling_factor)	* kernel[25])>>		conv2D_out_scal)    +   
		(	(	(zeropad	[i+(-3)][j+(-1)]		>>conv2D_scaling_factor)	* kernel[26])>>		conv2D_out_scal)    +   
		(	(	(zeropad	[i+(-3)][j+(0)]		>>conv2D_scaling_factor)	* kernel[27])>>		conv2D_out_scal)    +   
		(	(	(zeropad	[i+(-3)][j+(1)]		>>conv2D_scaling_factor)	* kernel[28])>>		conv2D_out_scal)    +   
		(	(	(zeropad	[i+(-3)][j+(2)]		>>conv2D_scaling_factor)	* kernel[29])>>		conv2D_out_scal)    +   
		(	(	(zeropad	[i+(-3)][j+(3)]		>>conv2D_scaling_factor)	* kernel[30])>>		conv2D_out_scal)    +   
		(	(	(zeropad	[i+(-3)][j+(4)]		>>conv2D_scaling_factor)	* kernel[31])>>		conv2D_out_scal)    +   
		(	(	(zeropad	[i+(-3)][j+(5)]		>>conv2D_scaling_factor)	* kernel[32])>>		conv2D_out_scal)    +   
		(	(	(zeropad	[i+(-2)][j+(-5)]		>>conv2D_scaling_factor)	* kernel[33])>>		conv2D_out_scal)    +   
		(	(	(zeropad	[i+(-2)][j+(-4)]		>>conv2D_scaling_factor)	* kernel[34])>>		conv2D_out_scal)    +   
		(	(	(zeropad	[i+(-2)][j+(-3)]		>>conv2D_scaling_factor)	* kernel[35])>>		conv2D_out_scal)    +   
		(	(	(zeropad	[i+(-2)][j+(-2)]		>>conv2D_scaling_factor)	* kernel[36])>>		conv2D_out_scal)    +   
		(	(	(zeropad	[i+(-2)][j+(-1)]		>>conv2D_scaling_factor)	* kernel[37])>>		conv2D_out_scal)    +   
		(	(	(zeropad	[i+(-2)][j+(0)]		>>conv2D_scaling_factor)	* kernel[38])>>		conv2D_out_scal)    +   
		(	(	(zeropad	[i+(-2)][j+(1)]		>>conv2D_scaling_factor)	* kernel[39])>>		conv2D_out_scal)    +   
		(	(	(zeropad	[i+(-2)][j+(2)]		>>conv2D_scaling_factor)	* kernel[40])>>		conv2D_out_scal)    +   
		(	(	(zeropad	[i+(-2)][j+(3)]		>>conv2D_scaling_factor)	* kernel[41])>>		conv2D_out_scal)    +   
		(	(	(zeropad	[i+(-2)][j+(4)]		>>conv2D_scaling_factor)	* kernel[42])>>		conv2D_out_scal)    +   
		(	(	(zeropad	[i+(-2)][j+(5)]		>>conv2D_scaling_factor)	* kernel[43])>>		conv2D_out_scal)    +   
		(	(	(zeropad	[i+(-1)][j+(-5)]		>>conv2D_scaling_factor)	* kernel[44])>>		conv2D_out_scal)    +   
		(	(	(zeropad	[i+(-1)][j+(-4)]		>>conv2D_scaling_factor)	* kernel[45])>>		conv2D_out_scal)    +   
		(	(	(zeropad	[i+(-1)][j+(-3)]		>>conv2D_scaling_factor)	* kernel[46])>>		conv2D_out_scal)    +   
		(	(	(zeropad	[i+(-1)][j+(-2)]		>>conv2D_scaling_factor)	* kernel[47])>>		conv2D_out_scal)    +   
		(	(	(zeropad	[i+(-1)][j+(-1)]		>>conv2D_scaling_factor)	* kernel[48])>>		conv2D_out_scal)    +   
		(	(	(zeropad	[i+(-1)][j+(0)]		>>conv2D_scaling_factor)	* kernel[49])>>		conv2D_out_scal)    +   
		(	(	(zeropad	[i+(-1)][j+(1)]		>>conv2D_scaling_factor)	* kernel[50])>>		conv2D_out_scal)    +   
		(	(	(zeropad	[i+(-1)][j+(2)]		>>conv2D_scaling_factor)	* kernel[51])>>		conv2D_out_scal)    +   
		(	(	(zeropad	[i+(-1)][j+(3)]		>>conv2D_scaling_factor)	* kernel[52])>>		conv2D_out_scal)    +   
		(	(	(zeropad	[i+(-1)][j+(4)]		>>conv2D_scaling_factor)	* kernel[53])>>		conv2D_out_scal)    +   
		(	(	(zeropad	[i+(-1)][j+(5)]		>>conv2D_scaling_factor)	* kernel[54])>>		conv2D_out_scal)    +   
		(	(	(zeropad	[i+(0)][j+(-5)]		>>conv2D_scaling_factor)	* kernel[55])>>		conv2D_out_scal)    +   
		(	(	(zeropad	[i+(0)][j+(-4)]		>>conv2D_scaling_factor)	* kernel[56])>>		conv2D_out_scal)    +   
		(	(	(zeropad	[i+(0)][j+(-3)]		>>conv2D_scaling_factor)	* kernel[57])>>		conv2D_out_scal)    +   
		(	(	(zeropad	[i+(0)][j+(-2)]		>>conv2D_scaling_factor)	* kernel[58])>>		conv2D_out_scal)    +   
		(	(	(zeropad	[i+(0)][j+(-1)]		>>conv2D_scaling_factor)	* kernel[59])>>		conv2D_out_scal)    +   
		(	(	(zeropad	[i+(0)][j+(0)]		>>conv2D_scaling_factor)	* kernel[60])>>		conv2D_out_scal)    +   
		(	(	(zeropad	[i+(0)][j+(1)]		>>conv2D_scaling_factor)	* kernel[61])>>		conv2D_out_scal)    +   
		(	(	(zeropad	[i+(0)][j+(2)]		>>conv2D_scaling_factor)	* kernel[62])>>		conv2D_out_scal)    +   
		(	(	(zeropad	[i+(0)][j+(3)]		>>conv2D_scaling_factor)	* kernel[63])>>		conv2D_out_scal)    +   
		(	(	(zeropad	[i+(0)][j+(4)]		>>conv2D_scaling_factor)	* kernel[64])>>		conv2D_out_scal)    +   
		(	(	(zeropad	[i+(0)][j+(5)]		>>conv2D_scaling_factor)	* kernel[65])>>		conv2D_out_scal)    +   
		(	(	(zeropad	[i+(1)][j+(-5)]		>>conv2D_scaling_factor)	* kernel[66])>>		conv2D_out_scal)    +   
		(	(	(zeropad	[i+(1)][j+(-4)]		>>conv2D_scaling_factor)	* kernel[67])>>		conv2D_out_scal)    +   
		(	(	(zeropad	[i+(1)][j+(-3)]		>>conv2D_scaling_factor)	* kernel[68])>>		conv2D_out_scal)    +   
		(	(	(zeropad	[i+(1)][j+(-2)]		>>conv2D_scaling_factor)	* kernel[69])>>		conv2D_out_scal)    +   
		(	(	(zeropad	[i+(1)][j+(-1)]		>>conv2D_scaling_factor)	* kernel[70])>>		conv2D_out_scal)    +   
		(	(	(zeropad	[i+(1)][j+(0)]		>>conv2D_scaling_factor)	* kernel[71])>>		conv2D_out_scal)    +   
		(	(	(zeropad	[i+(1)][j+(1)]		>>conv2D_scaling_factor)	* kernel[72])>>		conv2D_out_scal)    +   
		(	(	(zeropad	[i+(1)][j+(2)]		>>conv2D_scaling_factor)	* kernel[73])>>		conv2D_out_scal)    +   
		(	(	(zeropad	[i+(1)][j+(3)]		>>conv2D_scaling_factor)	* kernel[74])>>		conv2D_out_scal)    +   
		(	(	(zeropad	[i+(1)][j+(4)]		>>conv2D_scaling_factor)	* kernel[75])>>		conv2D_out_scal)    +   
		(	(	(zeropad	[i+(1)][j+(5)]		>>conv2D_scaling_factor)	* kernel[76])>>		conv2D_out_scal)    +   
		(	(	(zeropad	[i+(2)][j+(-5)]		>>conv2D_scaling_factor)	* kernel[77])>>		conv2D_out_scal)    +   
		(	(	(zeropad	[i+(2)][j+(-4)]		>>conv2D_scaling_factor)	* kernel[78])>>		conv2D_out_scal)    +   
		(	(	(zeropad	[i+(2)][j+(-3)]		>>conv2D_scaling_factor)	* kernel[79])>>		conv2D_out_scal)    +   
		(	(	(zeropad	[i+(2)][j+(-2)]		>>conv2D_scaling_factor)	* kernel[80])>>		conv2D_out_scal)    +   
		(	(	(zeropad	[i+(2)][j+(-1)]		>>conv2D_scaling_factor)	* kernel[81])>>		conv2D_out_scal)    +   
		(	(	(zeropad	[i+(2)][j+(0)]		>>conv2D_scaling_factor)	* kernel[82])>>		conv2D_out_scal)    +   
		(	(	(zeropad	[i+(2)][j+(1)]		>>conv2D_scaling_factor)	* kernel[83])>>		conv2D_out_scal)    +   
		(	(	(zeropad	[i+(2)][j+(2)]		>>conv2D_scaling_factor)	* kernel[84])>>		conv2D_out_scal)    +   
		(	(	(zeropad	[i+(2)][j+(3)]		>>conv2D_scaling_factor)	* kernel[85])>>		conv2D_out_scal)    +   
		(	(	(zeropad	[i+(2)][j+(4)]		>>conv2D_scaling_factor)	* kernel[86])>>		conv2D_out_scal)    +   
		(	(	(zeropad	[i+(2)][j+(5)]		>>conv2D_scaling_factor)	* kernel[87])>>		conv2D_out_scal)    +   
		(	(	(zeropad	[i+(3)][j+(-5)]		>>conv2D_scaling_factor)	* kernel[88])>>		conv2D_out_scal)    +   
		(	(	(zeropad	[i+(3)][j+(-4)]		>>conv2D_scaling_factor)	* kernel[89])>>		conv2D_out_scal)    +   
		(	(	(zeropad	[i+(3)][j+(-3)]		>>conv2D_scaling_factor)	* kernel[90])>>		conv2D_out_scal)    +   
		(	(	(zeropad	[i+(3)][j+(-2)]		>>conv2D_scaling_factor)	* kernel[91])>>		conv2D_out_scal)    +   
		(	(	(zeropad	[i+(3)][j+(-1)]		>>conv2D_scaling_factor)	* kernel[92])>>		conv2D_out_scal)    +   
		(	(	(zeropad	[i+(3)][j+(0)]		>>conv2D_scaling_factor)	* kernel[93])>>		conv2D_out_scal)    +   
		(	(	(zeropad	[i+(3)][j+(1)]		>>conv2D_scaling_factor)	* kernel[94])>>		conv2D_out_scal)    +   
		(	(	(zeropad	[i+(3)][j+(2)]		>>conv2D_scaling_factor)	* kernel[95])>>		conv2D_out_scal)    +   
		(	(	(zeropad	[i+(3)][j+(3)]		>>conv2D_scaling_factor)	* kernel[96])>>		conv2D_out_scal)    +   
		(	(	(zeropad	[i+(3)][j+(4)]		>>conv2D_scaling_factor)	* kernel[97])>>		conv2D_out_scal)    +   
		(	(	(zeropad	[i+(3)][j+(5)]		>>conv2D_scaling_factor)	* kernel[98])>>		conv2D_out_scal)    +   
		(	(	(zeropad	[i+(4)][j+(-5)]		>>conv2D_scaling_factor)	* kernel[99])>>		conv2D_out_scal)    +   
		(	(	(zeropad	[i+(4)][j+(-4)]		>>conv2D_scaling_factor)	* kernel[100])>>		conv2D_out_scal)    +   
		(	(	(zeropad	[i+(4)][j+(-3)]		>>conv2D_scaling_factor)	* kernel[101])>>		conv2D_out_scal)    +   
		(	(	(zeropad	[i+(4)][j+(-2)]		>>conv2D_scaling_factor)	* kernel[102])>>		conv2D_out_scal)    +   
		(	(	(zeropad	[i+(4)][j+(-1)]		>>conv2D_scaling_factor)	* kernel[103])>>		conv2D_out_scal)    +   
		(	(	(zeropad	[i+(4)][j+(0)]		>>conv2D_scaling_factor)	* kernel[104])>>		conv2D_out_scal)    +   
		(	(	(zeropad	[i+(4)][j+(1)]		>>conv2D_scaling_factor)	* kernel[105])>>		conv2D_out_scal)    +   
		(	(	(zeropad	[i+(4)][j+(2)]		>>conv2D_scaling_factor)	* kernel[106])>>		conv2D_out_scal)    +   
		(	(	(zeropad	[i+(4)][j+(3)]		>>conv2D_scaling_factor)	* kernel[107])>>		conv2D_out_scal)    +   
		(	(	(zeropad	[i+(4)][j+(4)]		>>conv2D_scaling_factor)	* kernel[108])>>		conv2D_out_scal)    +   
		(	(	(zeropad	[i+(4)][j+(5)]		>>conv2D_scaling_factor)	* kernel[109])>>		conv2D_out_scal)    +   
		(	(	(zeropad	[i+(5)][j+(-5)]		>>conv2D_scaling_factor)	* kernel[110])>>		conv2D_out_scal)    +   
		(	(	(zeropad	[i+(5)][j+(-4)]		>>conv2D_scaling_factor)	* kernel[111])>>		conv2D_out_scal)    +   
		(	(	(zeropad	[i+(5)][j+(-3)]		>>conv2D_scaling_factor)	* kernel[112])>>		conv2D_out_scal)    +   
		(	(	(zeropad	[i+(5)][j+(-2)]		>>conv2D_scaling_factor)	* kernel[113])>>		conv2D_out_scal)    +   
		(	(	(zeropad	[i+(5)][j+(-1)]		>>conv2D_scaling_factor)	* kernel[114])>>		conv2D_out_scal)    +   
		(	(	(zeropad	[i+(5)][j+(0)]		>>conv2D_scaling_factor)	* kernel[115])>>		conv2D_out_scal)    +   
		(	(	(zeropad	[i+(5)][j+(1)]		>>conv2D_scaling_factor)	* kernel[116])>>		conv2D_out_scal)    +   
		(	(	(zeropad	[i+(5)][j+(2)]		>>conv2D_scaling_factor)	* kernel[117])>>		conv2D_out_scal)    +   
		(	(	(zeropad	[i+(5)][j+(3)]		>>conv2D_scaling_factor)	* kernel[118])>>		conv2D_out_scal)    +   
		(	(	(zeropad	[i+(5)][j+(4)]		>>conv2D_scaling_factor)	* kernel[119])>>		conv2D_out_scal)    +   
		(	(	(zeropad	[i+(5)][j+(5)]		>>conv2D_scaling_factor)	* kernel[120])>>		conv2D_out_scal)    ;   
		}                                                                                         
	}                                                                                         
}                                                                                         // // --------------------------------------------------------------------------------------------------------------------------------
#endif
// // --------------------------------------------------------------------------------------------------------------------------------
// // --------------------------------------------------------------------------------------------------------------------------------
void convolution2D_SPM_off_NOB_zeropadded_11(void* spm_dest, 	 void* spm_fm,	 void* spm_krn,	 void* spm_temp,  int size){
	int print=0;
  //Pointers to Spms and other index that i'll need for the convolution
	// void* spmaddrAoff= (void*)((int*)spm_fm + mem_off );
	void* spmaddrAoff= (void*)(spm_fm);
	void* spmaddrBoff= (void*)(spm_krn );
	void* spmaddrCoff= (void*)(spm_dest);
	void* spmaddrDoff= (void*)(spmaddrBoff+7*7*SIZE_OF_INT);

	void* dest_in_C;
  void* dest_in_B;
  void* dest_in_D;

  int k_element=0;
  int mat_int_shift=0; //internal shifting for properly pointing insied the spms while making kaddv

	int jump_kr_row=3; // determina il salto della riga per la matrice kernel zeropadded
	int kern_offset=0;
	int fm_offset=0;
  int zero=0;
  //______________________________sub_kernel E
	int kernel_width=11;
	int str_diff=10; //structural difference
	int centering_offset=5;
  CSR_MVSIZE((size-str_diff)*SIZE_OF_INT);
	
	for(int i=centering_offset; i< (size)-centering_offset;i++)	{
		for (int rw_pt=-centering_offset; rw_pt	<= centering_offset; rw_pt++)	{
			//rw_pt is an index i use to point to the correct row, regarding this loop that is executed three times
			//instead of making 9 different ksvmulrf
			for (int clm_pt=0 ; clm_pt<kernel_width; clm_pt++){
				// printf("sono dentro!\n");
				ksvmulsc((void*)	(	(int*)(spmaddrDoff)	) + SIZE_OF_INT*(size*i)+centering_offset*SIZE_OF_INT,
								 (void*)	( (int*)	spmaddrAoff		+ (i+rw_pt)*size	+	clm_pt ),
								 (void*)	( (int*)	spmaddrBoff		+	k_element++) );

				ksrav((void*)(	(int*)	(spmaddrDoff)	) + SIZE_OF_INT*(size*i)+centering_offset*SIZE_OF_INT,
							(void*)(	(int*)	(spmaddrDoff)	) + SIZE_OF_INT*(size*i)+centering_offset*SIZE_OF_INT,
							(int*)	conv2D_out_scal	);

				kaddv ((void*)(	(int*)	(spmaddrCoff)	) + SIZE_OF_INT*(size*i)+centering_offset*SIZE_OF_INT,
							 (void*)(	(int*)	(spmaddrCoff)	) + SIZE_OF_INT*(size*i)+centering_offset*SIZE_OF_INT,
							 (void*)(	(int*)	(spmaddrDoff)	) + SIZE_OF_INT*(size*i)+centering_offset*SIZE_OF_INT	);

				// kbcast((void*)	( (int*)	spmaddrAoff		+ (i+rw_pt)*size	+	clm_pt), zero );

			}
		}
		
		k_element=0;
	}
	// CSR_MVSIZE(size*size*SIZE_OF_INT);
	ksvmulrf((void*)spmaddrDoff,(void*)spmaddrDoff,(void*)zero);
}
void convolution2D_SPM_off_NOB_zeropadded_9(void* spm_dest, 	 void* spm_fm,	 void* spm_krn,	 void* spm_temp,  int size){
	int print=0;
  //Pointers to Spms and other index that i'll need for the convolution
	// void* spmaddrAoff= (void*)((int*)spm_fm + mem_off );
	void* spmaddrAoff= (void*)(spm_fm);
	void* spmaddrBoff= (void*)(spm_krn );
	void* spmaddrCoff= (void*)(spm_dest);
	void* spmaddrDoff= (void*)(spmaddrBoff+7*7*SIZE_OF_INT);

	void* dest_in_C;
  void* dest_in_B;
  void* dest_in_D;

  int k_element=0;
  int mat_int_shift=0; //internal shifting for properly pointing insied the spms while making kaddv

	int jump_kr_row=3; // determina il salto della riga per la matrice kernel zeropadded
	int kern_offset=0;
	int fm_offset=0;
  int zero=0;
  //______________________________sub_kernel E
	int kernel_width=9;
	int str_diff=8; //structural difference
	int centering_offset=4;
  CSR_MVSIZE((size-str_diff)*SIZE_OF_INT);
	
	for(int i=centering_offset; i< (size)-centering_offset;i++)	{
		for (int rw_pt=-centering_offset; rw_pt	<= centering_offset; rw_pt++)	{
			//rw_pt is an index i use to point to the correct row, regarding this loop that is executed three times
			//instead of making 9 different ksvmulrf
			for (int clm_pt=0 ; clm_pt<kernel_width; clm_pt++){
				// printf("sono dentro!\n");
				ksvmulsc((void*)	(	(int*)(spmaddrDoff)	) + SIZE_OF_INT*(size*i)+centering_offset*SIZE_OF_INT,
								 (void*)	( (int*)	spmaddrAoff		+ (i+rw_pt)*size	+	clm_pt ),
								 (void*)	( (int*)	spmaddrBoff		+	k_element++) );

				ksrav((void*)(	(int*)	(spmaddrDoff)	) + SIZE_OF_INT*(size*i)+centering_offset*SIZE_OF_INT,
							(void*)(	(int*)	(spmaddrDoff)	) + SIZE_OF_INT*(size*i)+centering_offset*SIZE_OF_INT,
							(int*)	conv2D_out_scal	);

				kaddv ((void*)(	(int*)	(spmaddrCoff)	) + SIZE_OF_INT*(size*i)+centering_offset*SIZE_OF_INT,
							 (void*)(	(int*)	(spmaddrCoff)	) + SIZE_OF_INT*(size*i)+centering_offset*SIZE_OF_INT,
							 (void*)(	(int*)	(spmaddrDoff)	) + SIZE_OF_INT*(size*i)+centering_offset*SIZE_OF_INT	);

				// kbcast((void*)	( (int*)	spmaddrAoff		+ (i+rw_pt)*size	+	clm_pt), zero );

			}
		}
		
		k_element=0;
	}
	// CSR_MVSIZE(size*size*SIZE_OF_INT);
	ksvmulrf((void*)spmaddrDoff,(void*)spmaddrDoff,(void*)zero);
}
void convolution2D_SPM_off_NOB_zeropadded_7(void* spm_dest, 	 void* spm_fm,	 void* spm_krn,	 void* spm_temp,  int size){
	int print=0;
  //Pointers to Spms and other index that i'll need for the convolution
	// void* spmaddrAoff= (void*)((int*)spm_fm + mem_off );
	void* spmaddrAoff= (void*)(spm_fm);
	void* spmaddrBoff= (void*)(spm_krn );
	void* spmaddrCoff= (void*)(spm_dest);
	void* spmaddrDoff= (void*)(spmaddrBoff+7*7*SIZE_OF_INT);

	void* dest_in_C;
  void* dest_in_B;
  void* dest_in_D;

  int k_element=0;
  int mat_int_shift=0; //internal shifting for properly pointing insied the spms while making kaddv

	int jump_kr_row=3; // determina il salto della riga per la matrice kernel zeropadded
	int kern_offset=0;
	int fm_offset=0;
  int zero=0;
  //______________________________sub_kernel E
	int kernel_width=7;
	int str_diff=6; //structural difference
	int centering_offset=3;
  CSR_MVSIZE((size-str_diff)*SIZE_OF_INT);
	
	for(int i=centering_offset; i< (size)-centering_offset;i++)	{
		for (int rw_pt=-centering_offset; rw_pt	<= centering_offset; rw_pt++)	{
			//rw_pt is an index i use to point to the correct row, regarding this loop that is executed three times
			//instead of making 9 different ksvmulrf
			for (int clm_pt=0 ; clm_pt<kernel_width; clm_pt++){
				// printf("sono dentro!\n");
				ksvmulsc((void*)	(	(int*)(spmaddrDoff)	) + SIZE_OF_INT*(size*i)+centering_offset*SIZE_OF_INT,
								 (void*)	( (int*)	spmaddrAoff		+ (i+rw_pt)*size	+	clm_pt ),
								 (void*)	( (int*)	spmaddrBoff		+	k_element++) );

				ksrav((void*)(	(int*)	(spmaddrDoff)	) + SIZE_OF_INT*(size*i)+centering_offset*SIZE_OF_INT,
							(void*)(	(int*)	(spmaddrDoff)	) + SIZE_OF_INT*(size*i)+centering_offset*SIZE_OF_INT,
							(int*)	conv2D_out_scal	);

				kaddv ((void*)(	(int*)	(spmaddrCoff)	) + SIZE_OF_INT*(size*i)+centering_offset*SIZE_OF_INT,
							 (void*)(	(int*)	(spmaddrCoff)	) + SIZE_OF_INT*(size*i)+centering_offset*SIZE_OF_INT,
							 (void*)(	(int*)	(spmaddrDoff)	) + SIZE_OF_INT*(size*i)+centering_offset*SIZE_OF_INT	);

				// kbcast((void*)	( (int*)	spmaddrAoff		+ (i+rw_pt)*size	+	clm_pt), zero );

			}
		}
		
		k_element=0;
	}
	// CSR_MVSIZE(size*size*SIZE_OF_INT);
	ksvmulrf((void*)spmaddrDoff,(void*)spmaddrDoff,(void*)zero);
}
void convolution2D_SPM_off_NOB_zeropadded_5(void* spm_dest, 	 void* spm_fm,	 void* spm_krn,	 void* spm_temp,  int size){
	int print=0;
  //Pointers to Spms and other index that i'll need for the convolution
	// void* spmaddrAoff= (void*)((int*)spm_fm + mem_off );
	void* spmaddrAoff= (void*)(spm_fm);
	void* spmaddrBoff= (void*)(spm_krn );
	void* spmaddrCoff= (void*)(spm_dest);
	void* spmaddrDoff= (void*)(spmaddrBoff+5*5*SIZE_OF_INT);

	void* dest_in_C;
  void* dest_in_B;
  void* dest_in_D;

  int k_element=0;
  int mat_int_shift=0; //internal shifting for properly pointing insied the spms while making kaddv

	int jump_kr_row=3; // determina il salto della riga per la matrice kernel zeropadded
	int kern_offset=0;
	int fm_offset=0;
  int zero=0;
  //______________________________sub_kernel E
	int kernel_width=5;
	int str_diff=4; //structural difference
	int centering_offset=2;
  CSR_MVSIZE((size-str_diff)*SIZE_OF_INT);
	
	for(int i=centering_offset; i< (size)-centering_offset;i++)	{
		for (int rw_pt=-centering_offset; rw_pt	<= centering_offset; rw_pt++)	{
			//rw_pt is an index i use to point to the correct row, regarding this loop that is executed three times
			//instead of making 9 different ksvmulrf
			for (int clm_pt=0 ; clm_pt<kernel_width; clm_pt++){
				// printf("sono dentro!\n");
				ksvmulsc((void*)	(	(int*)(spmaddrDoff)	) + SIZE_OF_INT*(size*i)+centering_offset*SIZE_OF_INT,
								 (void*)	( (int*)	spmaddrAoff		+ (i+rw_pt)*size	+	clm_pt ),
								 (void*)	( (int*)	spmaddrBoff		+	k_element++) );

				ksrav((void*)(	(int*)	(spmaddrDoff)	) + SIZE_OF_INT*(size*i)+centering_offset*SIZE_OF_INT,
							(void*)(	(int*)	(spmaddrDoff)	) + SIZE_OF_INT*(size*i)+centering_offset*SIZE_OF_INT,
							(int*)	conv2D_out_scal	);

				kaddv ((void*)(	(int*)	(spmaddrCoff)	) + SIZE_OF_INT*(size*i)+centering_offset*SIZE_OF_INT,
							 (void*)(	(int*)	(spmaddrCoff)	) + SIZE_OF_INT*(size*i)+centering_offset*SIZE_OF_INT,
							 (void*)(	(int*)	(spmaddrDoff)	) + SIZE_OF_INT*(size*i)+centering_offset*SIZE_OF_INT	);

				// kbcast((void*)	( (int*)	spmaddrAoff		+ (i+rw_pt)*size	+	clm_pt), zero );

			}
		}
		
		k_element=0;
	}
	// CSR_MVSIZE(size*size*SIZE_OF_INT);
	ksvmulrf((void*)spmaddrDoff,(void*)spmaddrDoff,(void*)zero);
}
void convolution2D_SPM_off_NOB_zeropadded_3(void* spm_dest, 	 void* spm_fm,	 void* spm_krn,	 void* spm_temp,  int size){
	int print=0;
  //Pointers to Spms and other index that i'll need for the convolution
	// void* spmaddrAoff= (void*)((int*)spm_fm + mem_off );
	void* spmaddrAoff= (void*)(spm_fm);
	void* spmaddrBoff= (void*)(spm_krn );
	void* spmaddrCoff= (void*)(spm_dest);
	void* spmaddrDoff= (void*)(spmaddrBoff+3*3*SIZE_OF_INT);

	void* dest_in_C;
  void* dest_in_B;
  void* dest_in_D;

  int k_element=0;
  int mat_int_shift=0; //internal shifting for properly pointing insied the spms while making kaddv

	int jump_kr_row=3; // determina il salto della riga per la matrice kernel zeropadded
	int kern_offset=0;
	int fm_offset=0;
  int zero=0;
  //______________________________sub_kernel E
	int kernel_width=3;
	int str_diff=2; //structural difference
	int centering_offset=1;
  CSR_MVSIZE((size-str_diff)*SIZE_OF_INT);
	
	for(int i=centering_offset; i< (size)-centering_offset;i++)	{
		for (int rw_pt=-centering_offset; rw_pt	<= centering_offset; rw_pt++)	{
			//rw_pt is an index i use to point to the correct row, regarding this loop that is executed three times
			//instead of making 9 different ksvmulrf
			for (int clm_pt=0 ; clm_pt<kernel_width; clm_pt++){
				ksvmulsc((void*)(	(int*)	(spmaddrDoff)	) + SIZE_OF_INT*(size*i)+centering_offset*SIZE_OF_INT,
									(void*)	( (int*)spmaddrAoff + (i+rw_pt)*size	+	clm_pt ),
									(void*)	( (int*)spmaddrBoff+k_element++) );

				ksrav((void*)(	(int*)	(spmaddrDoff)	) + SIZE_OF_INT*(size*i)+centering_offset*SIZE_OF_INT,
								(void*)(	(int*)	(spmaddrDoff)	) + SIZE_OF_INT*(size*i)+centering_offset*SIZE_OF_INT,
								(int*)conv2D_out_scal);

				kaddv ((void*)(	(int*)		(spmaddrCoff)	) + SIZE_OF_INT*(size*i)+centering_offset*SIZE_OF_INT,
								(void*)(	(int*)	(spmaddrCoff)	) + SIZE_OF_INT*(size*i)+centering_offset*SIZE_OF_INT,
								(void*)(	(int*)	(spmaddrDoff)	) + SIZE_OF_INT*(size*i)+centering_offset*SIZE_OF_INT);
			}
		}
		k_element=0;
	}
	ksvmulrf((void*)spmaddrDoff,(void*)spmaddrDoff,(void*)zero);
}
void convolution2D_SPM_off_NOB_zeropadded(void* spm_dest, 	 void* spm_fm,	 void* spm_krn,	 void* spm_temp,  int size){
	int print=0;
  //Pointers to Spms and other index that i'll need for the convolution
	// void* spmaddrAoff= (void*)((int*)spm_fm + mem_off );
	void* spmaddrAoff= (void*)(spm_fm);
	void* spmaddrBoff= (void*)(spm_krn );
	void* spmaddrCoff= (void*)(spm_dest);
	void* spmaddrDoff= (void*)(spmaddrBoff+12*SIZE_OF_INT);

	void* dest_in_C;
  void* dest_in_B;
  void* dest_in_D;

  int k_element=0;
  int mat_int_shift=0; //internal shifting for properly pointing insied the spms while making kaddv

	int jump_kr_row=3; // determina il salto della riga per la matrice kernel zeropadded
	int kern_offset=0;
	int fm_offset=0;
  int zero=0;
  //______________________________sub_kernel E
  CSR_MVSIZE((size-2)*SIZE_OF_INT); //però in questo caso la dimesione è quella della matrice zeropadded
	for(int i=1; i< size-1;i++)
	{
		k_element=0;
		for (int rw_pt=-1; rw_pt<2; rw_pt++) //rw_pt is an index i use to point to the correct row, regarding this loop that is executed three times
		//instead of making 9 different ksvmulrf
		{
			ksvmulsc((void*)(	(int*)	(spmaddrDoff)	) + SIZE_OF_INT*(size*i)+1*SIZE_OF_INT,
        			  (void*)	( (int*)spmaddrAoff + (i+rw_pt)*size	+0 ),
                (void*)	( (int*)spmaddrBoff+k_element++) );

				ksrav((void*)(	(int*)	(spmaddrDoff)	) + SIZE_OF_INT*(size*i)+1*SIZE_OF_INT,
								(void*)(	(int*)	(spmaddrDoff)	) + SIZE_OF_INT*(size*i)+1*SIZE_OF_INT,
								(int*)conv2D_out_scal);

  		kaddv ((void*)(	(int*)	(spmaddrCoff)	) + SIZE_OF_INT*(size*i)+1*SIZE_OF_INT,
              (void*)(	(int*)	(spmaddrCoff)	) + SIZE_OF_INT*(size*i)+1*SIZE_OF_INT,
              (void*)(	(int*)	(spmaddrDoff)	) + SIZE_OF_INT*(size*i)+1*SIZE_OF_INT);

			ksvmulsc((void*)(	(int*)	(spmaddrDoff)	) + SIZE_OF_INT*(size*i)+1*SIZE_OF_INT,
        			  (void*)	( (int*)spmaddrAoff + (i+rw_pt)*size	+1 ),
                (void*)	( (int*)spmaddrBoff+k_element++) );

				ksrav((void*)(	(int*)	(spmaddrDoff)	) + SIZE_OF_INT*(size*i)+1*SIZE_OF_INT,
								(void*)(	(int*)	(spmaddrDoff)	) + SIZE_OF_INT*(size*i)+1*SIZE_OF_INT,
								(int*)conv2D_out_scal);

  		kaddv ((void*)(	(int*)	(spmaddrCoff)	) + SIZE_OF_INT*(size*i)+1*SIZE_OF_INT,
              (void*)(	(int*)	(spmaddrCoff)	) + SIZE_OF_INT*(size*i)+1*SIZE_OF_INT,
              (void*)(	(int*)	(spmaddrDoff)	) + SIZE_OF_INT*(size*i)+1*SIZE_OF_INT);

			ksvmulsc((void*)(	(int*)	(spmaddrDoff)	) + SIZE_OF_INT*(size*i)+1*SIZE_OF_INT,
        			  (void*)	( (int*)spmaddrAoff + (i+rw_pt)*size	+2 ),
                (void*)	( (int*)spmaddrBoff+k_element++) );

				ksrav((void*)(	(int*)	(spmaddrDoff)	) + SIZE_OF_INT*(size*i)+1*SIZE_OF_INT,
								(void*)(	(int*)	(spmaddrDoff)	) + SIZE_OF_INT*(size*i)+1*SIZE_OF_INT,
								(int*)conv2D_out_scal);

  		kaddv ((void*)(	(int*)	(spmaddrCoff)	) + SIZE_OF_INT*(size*i)+1*SIZE_OF_INT,
              (void*)(	(int*)	(spmaddrCoff)	) + SIZE_OF_INT*(size*i)+1*SIZE_OF_INT,
              (void*)(	(int*)	(spmaddrDoff)	) + SIZE_OF_INT*(size*i)+1*SIZE_OF_INT);
		}
	}
	// CSR_MVSIZE(size*size*SIZE_OF_INT);
	ksvmulrf((void*)spmaddrDoff,(void*)spmaddrDoff,(void*)zero);
}
// // --------------------------------------------------------------------------------------------------------------------------------
// // --------------------------------------------------------------------------------------------------------------------------------
void convolution2D_Zeropadding(int size, int (*matrix)[size], int *kernel_tmp, int *out){
	int print=0;
	int kernel[9];
	int conv2D_scaling_factor=8;
	int conv2D_out_scal=5;
	for(int i=0;i<9;i++){
    	kernel[i]=(kernel_tmp[i]>>conv2D_scaling_factor);
	}
	int i, j;
	int pt=0;
	int zeropad[Z_ORDER][Z_ORDER]={0};
	
	for (int i=1; i< size+2-1; i++)
		for (int j=1; j< size+2-1; j++)
			zeropad[i][j]=matrix[i-1][j-1];
	
	// for (int i=0; i<Z_ORDER; i++){
	//	 for (int j=0; j< Z_ORDER; j++){
	//		 printf("%d ",zeropadd[i][j]);
	//	 }
	//	 printf("\n");
	// }

	// kernel 										E centrale
	// for (i = 1; i < (size+2)-1; i++)
	// {
	//	 for (j = 1; j < (size+2)-1; j++)
	//	 {
	//		 pt=(i-1)*size+(j-1);
	//		 out[pt] +=	(	(	(zeropad[i-1][j-1]>>conv2D_scaling_factor) 	* kernel[0])>>		conv2D_out_scal)    +
									// (	(	(zeropad[i-1][j]	>>conv2D_scaling_factor)	* kernel[1])>>		conv2D_out_scal)    +
									// (	(	(zeropad[i-1][j+1]>>conv2D_scaling_factor)	* kernel[2])>>		conv2D_out_scal)    +
									// (	(	(zeropad[i][j-1]	>>conv2D_scaling_factor)	* kernel[3])>>		conv2D_out_scal)    +
									// (	(	(zeropad[i][j]	>>conv2D_scaling_factor)		* kernel[4])>>		conv2D_out_scal)    +
									// (	(	(zeropad[i][j+1]	>>conv2D_scaling_factor)	* kernel[5])>>		conv2D_out_scal)    +
									// (	(	(zeropad[i+1][j-1]>>conv2D_scaling_factor) 	* kernel[6])>>		conv2D_out_scal)    +
									// (	(	(zeropad[i+1][j] >>conv2D_scaling_factor)		* kernel[7])>>		conv2D_out_scal)    +
									// (	(	(zeropad[i+1][j+1]>>conv2D_scaling_factor) 	* kernel[8])>>		conv2D_out_scal)   ;
		// }
	// }
	for (i = 1; i < (size+2)-1; i++)
	{
		for (j = 1; j < (size+2)-1; j++)
			out[(i-1)*size+(j-1)] +=	(	(	(zeropad[i-1][j-1]>>conv2D_scaling_factor) 	* kernel[0])>>		conv2D_out_scal);
		for (j = 1; j < (size+2)-1; j++)
			out[(i-1)*size+(j-1)] +=	(	(	(zeropad[i-1][j]	>>conv2D_scaling_factor)	* kernel[1])>>		conv2D_out_scal);
		for (j = 1; j < (size+2)-1; j++)
			out[(i-1)*size+(j-1)] +=	(	(	(zeropad[i-1][j+1]>>conv2D_scaling_factor)	* kernel[2])>>		conv2D_out_scal);
		for (j = 1; j < (size+2)-1; j++)
			out[(i-1)*size+(j-1)] +=	(	(	(zeropad[i][j-1]	>>conv2D_scaling_factor)	* kernel[3])>>		conv2D_out_scal);
		for (j = 1; j < (size+2)-1; j++)
			out[(i-1)*size+(j-1)] +=	(	(	(zeropad[i][j]	>>conv2D_scaling_factor)		* kernel[4])>>		conv2D_out_scal);
		for (j = 1; j < (size+2)-1; j++)
			out[(i-1)*size+(j-1)] +=	(	(	(zeropad[i][j+1]	>>conv2D_scaling_factor)	* kernel[5])>>		conv2D_out_scal);
		for (j = 1; j < (size+2)-1; j++)
			out[(i-1)*size+(j-1)] +=	(	(	(zeropad[i+1][j-1]>>conv2D_scaling_factor) 	* kernel[6])>>		conv2D_out_scal);
		for (j = 1; j < (size+2)-1; j++)
			out[(i-1)*size+(j-1)] +=	(	(	(zeropad[i+1][j] >>conv2D_scaling_factor)		* kernel[7])>>		conv2D_out_scal);
		for (j = 1; j < (size+2)-1; j++)
			out[(i-1)*size+(j-1)] +=	(	(	(zeropad[i+1][j+1]>>conv2D_scaling_factor) 	* kernel[8])>>		conv2D_out_scal);
	}
}
void convolution2D_Zeropadding_3(int size, int (*matrix)[size], int *kernel_tmp, int *out){
	#define Krn_3 3
	#define structural_diff_3 2
		#define Z_MAT_3 A_ORDER + structural_diff_3
	int str_diff=2;
	int centering_offset=1;
	
	int print=0;
	int conv2D_scaling_factor=8;
	int conv2D_out_scal=5;
	int i, j;
	int pt=0;
	int k_index=0;
	
	int kernel[Krn_3*Krn_3];
	for(int i=0;i<Krn_3*Krn_3;i++){
    	kernel[i]=(kernel_tmp[i]>>conv2D_scaling_factor);
	}
	
	// int zeropad[A_ORDER+str_diff][A_ORDER+str_diff]={0};	
	int zeropad[Z_MAT_3][Z_MAT_3]={0};	
	for (int i= centering_offset; i	< A_ORDER	+	str_diff	-	centering_offset	; i++) {
		for (int j= centering_offset; j	< A_ORDER	+	str_diff	-	centering_offset	; j++) {
			zeropad[i][j]=matrix[i-centering_offset][j-centering_offset];
		}
	}
	
	// kernel 	E centrale
	for (i = centering_offset ; i < A_ORDER	+	str_diff	-centering_offset	; i++)
	{
		#pragma GCC unroll 3
		for (int ROW_k_SCAN= -centering_offset; ROW_k_SCAN <= centering_offset; ROW_k_SCAN++){
			#pragma GCC unroll 3
			for (int CLM_k_SCAN= -centering_offset; CLM_k_SCAN <= centering_offset; CLM_k_SCAN++){
				for (j = centering_offset ; j < A_ORDER	+	str_diff	-centering_offset	; j++){
					out[(i-centering_offset)*size+(j-centering_offset)] +=		(	(	(zeropad[i + ROW_k_SCAN][j + CLM_k_SCAN]>>conv2D_scaling_factor) 	* kernel[k_index])>>		conv2D_out_scal);
				}
				k_index++;
			}
		}
		k_index=0;
	}

}
void convolution2D_Zeropadding_5(int size, int (*matrix)[size], int *kernel_tmp, int *out){
	#define Krn_5 5
	#define structural_diff_5 4
		#define Z_MAT_5 A_ORDER + structural_diff_5
	int str_diff=4;
	int centering_offset=2;
	
	int print=0;
	int conv2D_scaling_factor=8;
	int conv2D_out_scal=5;
	int i, j;
	int pt=0;
	int k_index=0;
	
	int kernel[Krn_5*Krn_5];
	for(int i=0;i<Krn_5*Krn_5;i++){
    	kernel[i]=(kernel_tmp[i]>>conv2D_scaling_factor);
	}
	
	// int zeropad[A_ORDER+str_diff][A_ORDER+str_diff]={0};	
	int zeropad[Z_MAT_5][Z_MAT_5]={0};	
	for (int i= centering_offset; i	< A_ORDER	+	str_diff	-	centering_offset	; i++) {
		for (int j= centering_offset; j	< A_ORDER	+	str_diff	-	centering_offset	; j++) {
			zeropad[i][j]=matrix[ -centering_offset + i ][ -centering_offset + j];
		}
	}
	
	// kernel 	E centrale
	for (i = centering_offset ; i < A_ORDER	+	str_diff	-centering_offset	; i++)
	{
		#pragma GCC unroll 5
		for (int ROW_k_SCAN= -centering_offset; ROW_k_SCAN <= centering_offset; ROW_k_SCAN++){
			#pragma GCC unroll 5
			for (int CLM_k_SCAN= -centering_offset; CLM_k_SCAN <= centering_offset; CLM_k_SCAN++){
				for (j = centering_offset ; j < A_ORDER	+	str_diff	-centering_offset	; j++){
					out[(i-centering_offset)*size+(j-centering_offset)] +=		(	(	(zeropad[i + ROW_k_SCAN][j + CLM_k_SCAN]>>conv2D_scaling_factor) 	* kernel[k_index])>>		conv2D_out_scal);
				}
				k_index++;
			}
		}
		k_index=0;
	}

}
void convolution2D_Zeropadding_7(int size, int (*matrix)[size], int *kernel_tmp, int *out){
	#define Krn_7 7
	#define structural_diff_7 6
		#define Z_MAT_7 A_ORDER + structural_diff_7
	int str_diff=6;
	int centering_offset=3;
	
	int print=0;
	int conv2D_scaling_factor=8;
	int conv2D_out_scal=5;
	int i, j;
	int pt=0;
	int k_index=0;
	
	int kernel[Krn_7*Krn_7];
	for(int i=0;i<Krn_7*Krn_7;i++){
    	kernel[i]=(kernel_tmp[i]>>conv2D_scaling_factor);
	}
	
	// int zeropad[A_ORDER+str_diff][A_ORDER+str_diff]={0};	
	int zeropad[Z_MAT_7][Z_MAT_7]={0};	
	for (int i= centering_offset; i	< A_ORDER	+	str_diff	-	centering_offset	; i++) {
		for (int j= centering_offset; j	< A_ORDER	+	str_diff	-	centering_offset	; j++) {
			zeropad[i][j]=matrix[ -centering_offset + i ][ -centering_offset + j];
		}
	}
	
	// kernel 	E centrale
	for (i = centering_offset ; i < A_ORDER	+	str_diff	-centering_offset	; i++)
	{
		#pragma GCC unroll 7
		for (int ROW_k_SCAN= -centering_offset; ROW_k_SCAN <= centering_offset; ROW_k_SCAN++){
			#pragma GCC unroll 7
			for (int CLM_k_SCAN= -centering_offset; CLM_k_SCAN <= centering_offset; CLM_k_SCAN++){
				for (j = centering_offset ; j < A_ORDER	+	str_diff	-centering_offset	; j++){
					out[(i-centering_offset)*size+(j-centering_offset)] +=		(	(	(zeropad[i + ROW_k_SCAN][j + CLM_k_SCAN]>>conv2D_scaling_factor) 	* kernel[k_index])>>		conv2D_out_scal);
				}
				k_index++;
			}
		}
		k_index=0;
	}

}
void convolution2D_Zeropadding_9(int size, int (*matrix)[size], int *kernel_tmp, int *out){
	#define Krn_9 9
	#define structural_diff_9 8
		#define Z_MAT_9 A_ORDER + structural_diff_9
	int str_diff=8;
	int centering_offset=4;
	
	int print=0;
	int conv2D_scaling_factor=8;
	int conv2D_out_scal=5;
	int i, j;
	int pt=0;
	int k_index=0;
	
	int kernel[Krn_9*Krn_9];
	for(int i=0;i<Krn_9*Krn_9;i++){
    	kernel[i]=(kernel_tmp[i]>>conv2D_scaling_factor);
	}
	
	// int zeropad[A_ORDER+str_diff][A_ORDER+str_diff]={0};	
	int zeropad[Z_MAT_9][Z_MAT_9]={0};	
	for (int i= centering_offset; i	< A_ORDER	+	str_diff	-	centering_offset	; i++) {
		for (int j= centering_offset; j	< A_ORDER	+	str_diff	-	centering_offset	; j++) {
			zeropad[i][j]=matrix[ -centering_offset + i ][ -centering_offset + j];
		}
	}
	
	// kernel 	E centrale
	for (i = centering_offset ; i < A_ORDER	+	str_diff	-centering_offset	; i++)
	{
		for (int ROW_k_SCAN= -centering_offset; ROW_k_SCAN <= centering_offset; ROW_k_SCAN++){
			for (int CLM_k_SCAN= -centering_offset; CLM_k_SCAN <= centering_offset; CLM_k_SCAN++){
				for (j = centering_offset ; j < A_ORDER	+	str_diff	-centering_offset	; j++){
					out[(i-centering_offset)*size+(j-centering_offset)] +=		(	(	(zeropad[i + ROW_k_SCAN][j + CLM_k_SCAN]>>conv2D_scaling_factor) 	* kernel[k_index])>>		conv2D_out_scal);
				}
				k_index++;
			}
		}
		k_index=0;
	}

}
void convolution2D_Zeropadding_11(int size, int (*matrix)[size], int *kernel_tmp, int *out){
	#define Krn_11 11
	#define structural_diff_11 10
		#define Z_MAT_11 A_ORDER + structural_diff_11
	int str_diff=10;
	int centering_offset=5;
	
	int print=0;
	int conv2D_scaling_factor=8;
	int conv2D_out_scal=5;
	int i, j;
	int pt=0;
	int k_index=0;
	
	int kernel[Krn_11*Krn_11];
	for(int i=0;i<Krn_11*Krn_11;i++){
    	kernel[i]=(kernel_tmp[i]>>conv2D_scaling_factor);
	}
	
	// int zeropad[A_ORDER+str_diff][A_ORDER+str_diff]={0};	
	int zeropad[Z_MAT_11][Z_MAT_11]={0};	
	for (int i= centering_offset; i	< A_ORDER	+	str_diff	-	centering_offset	; i++) {
		for (int j= centering_offset; j	< A_ORDER	+	str_diff	-	centering_offset	; j++) {
			zeropad[i][j]=matrix[ -centering_offset + i ][ -centering_offset + j];
		}
	}
	
	// kernel 	E centrale
	for (i = centering_offset ; i < A_ORDER	+	str_diff	-centering_offset	; i++)
	{
		for (int ROW_k_SCAN= -centering_offset; ROW_k_SCAN <= centering_offset; ROW_k_SCAN++){
			for (int CLM_k_SCAN= -centering_offset; CLM_k_SCAN <= centering_offset; CLM_k_SCAN++){
				for (j = centering_offset ; j < A_ORDER	+	str_diff	-centering_offset	; j++){
					out[(i-centering_offset)*size+(j-centering_offset)] +=		(	(	(zeropad[i + ROW_k_SCAN][j + CLM_k_SCAN]>>conv2D_scaling_factor) 	* kernel[k_index])>>		conv2D_out_scal);
				}
				k_index++;
			}
		}
		k_index=0;
	}

}
// // --------------------------------------------------------------------------------------------------------------------------------
// // --------------------------------------------------------------------------------------------------------------------------------
// // --------------------------------------------------------------------------------------------------------------------------------
// // --------------------------------------------------------------------------------------------------------------------------------

// // --------------------------------------------------------------------------------------------------------------------------------
// // --------------------------------------------------------------------------------------------------------------------------------
void matrix_check( int* mat1, int* mat2, int size ){
	printf("Checking if there is an error...");
  int err=0;
	for(int i=0; i<size; i++)
	{
		for(int j=0; j<size; j++)
		{
			if ( *((int*)mat1+i*size+j) != *((int*)mat2+i*size+j) ) {
				// printf("\nERROR at elements [%d][%d] !\n",i,j);
        err++;

			}
		}
	}
  if (err==0){
    printf("Success.\n");
  }
	else printf("Errors!.\n");
}
// // --------------------------------------------------------------------------------------------------------------------------------
// // --------------------------------------------------------------------------------------------------------------------------------