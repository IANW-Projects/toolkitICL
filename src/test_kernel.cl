// enable double precision (not enabled by default)

#ifdef cl_khr_fp64
    #pragma OPENCL EXTENSION cl_khr_fp64 : enable
#else
    #error "IEEE-754 double precision not supported by OpenCL implementation."
#endif

kernel void test(global REAL *d_test,global REAL *d_test2,global REAL *d_test3)
{
  uint idx = get_global_id(0);
  
  d_test2[idx]=d_test2[idx]*2;
  printf("%f\n",d_test2[idx]);
    d_test3[idx]=5;
 
};



// enable double precision (not enabled by default)

#ifdef cl_khr_fp64
    #pragma OPENCL EXTENSION cl_khr_fp64 : enable
#else
    #error "IEEE-754 double precision not supported by OpenCL implementation."
#endif

inline void atomic_add_global(volatile global float *source, const float operand) {
    union {
        unsigned int intVal;
        float floatVal;
    } newVal;
    union {
        unsigned int intVal;
        float floatVal;
    } prevVal;
 
    do {
        prevVal.floatVal = *source;
        newVal.floatVal = prevVal.floatVal + operand;
    } while (atomic_cmpxchg((volatile global unsigned int *)source, prevVal.intVal, newVal.intVal) != prevVal.intVal);
}

kernel void MM(const global REAL *A,const global REAL *B,global REAL *C)
{

  // Thread identifiers
    const uint globalRow = get_global_id(0); // Row ID of C (0..M)
    const uint globalCol = get_global_id(1); // Col ID of C (0..N)
	
	const uint num_rows=(uint)NR;
	const uint num_cols=(uint)NC;
	const uint num_i=(uint)NI;
	
    // Compute a single element (loop over K)
    REAL acc = 0.0f;
	
    for (uint k=0; k<num_i; k++) {
        acc += A[k*num_rows + globalRow] * B[globalCol*num_i + k];
    }
 
    // Store the result
    C[globalCol*num_rows + globalRow] = acc;
 

 
};


kernel void DP(global const float* A, global const float* B, global float* C) {

   const int gid = get_global_id(0);
   const int lid = get_local_id(0);
   const int group_size = get_local_size(0);
   const int wgid = get_group_id(0);

   local REAL partial_dot[W_SIZE];

   partial_dot[lid] = A[gid] * B[gid];
 
   barrier(CLK_LOCAL_MEM_FENCE);

   for(uint i = group_size/2; i > 0; i = i /2) {
      if(lid < i) {
         partial_dot[lid] += partial_dot[lid + i];
      }
      barrier(CLK_LOCAL_MEM_FENCE);
   }
#ifdef REDUCE
   if(lid == 0) {
	   atomic_add_global(&(C[0]), partial_dot[0]);	   
   }
   #endif 
#ifndef REDUCE
   if(lid == 0) {
	   C[wgid]= partial_dot[0];	   
   }
   #endif 

}

kernel void EN(global const float* A, global float* B) {

   const int gid = get_global_id(0);
   const int lid = get_local_id(0);
   const int group_size = get_local_size(0);
   const int wgid = get_group_id(0);   
   local REAL partial_n[W_SIZE];

   partial_n[lid] = pown(A[gid],2);
 
   barrier(CLK_LOCAL_MEM_FENCE);

   for(uint i = group_size/2; i > 0; i = i /2) {
      if(lid < i) {
         partial_n[lid] += partial_n[lid + i];
      }
      barrier(CLK_LOCAL_MEM_FENCE);
   }

#ifdef REDUCE
   if(lid == 0) {
	   atomic_add_global(&(B[0]), partial_n[0]);	   
   }
   #endif 
#ifndef REDUCE
   if(lid == 0) {
	   B[wgid]= partial_n[0];	   
   }
   #endif 


}


kernel void LN(global const REAL* A, global REAL* B) {

   const int gid = get_global_id(0);
   const int lid = get_local_id(0);
   const int group_size = get_local_size(0);
   const int wgid = get_group_id(0);  
   local REAL partial_n[W_SIZE];

   partial_n[lid] = fabs(A[gid]);
 
   barrier(CLK_LOCAL_MEM_FENCE);

   for(uint i = group_size/2; i > 0; i = i /2) {
      if(lid < i) {
         partial_n[lid] += partial_n[lid + i];
      }
      barrier(CLK_LOCAL_MEM_FENCE);
   }

  #ifdef REDUCE
   if(lid == 0) {
	   atomic_add_global(&(B[0]), partial_n[0]);	   
   }
   #endif 
#ifndef REDUCE
   if(lid == 0) {
	   B[wgid]= partial_n[0];	   
   }
   #endif 

}




kernel void Reduce(global const REAL* IN, global REAL* OUT) {

   const int gid = get_global_id(0);
   const int lid = get_local_id(0);
   const int group_size = get_local_size(0);
   const int wgid = get_group_id(0);  

   local REAL partial[W_SIZE];
 partial[lid]=IN[gid];
   barrier(CLK_LOCAL_MEM_FENCE);

   for(uint i = group_size/2; i > 0; i = i /2) {
      if(lid < i) {
         partial[lid] += partial[lid + i];
      }
      barrier(CLK_LOCAL_MEM_FENCE);
   }

   if(lid == 0) {
	//   atomic_add_global(&(OUT[0]), partial[0]);	  
    OUT[wgid]= partial[0];	 
   }
 
 
}






kernel void Reduce2(global const REAL* IN, global REAL* OUT) {

   const int gid = get_global_id(0);
   const int lid = get_local_id(0);
   const int group_size = get_local_size(0);
   const int wgid = get_group_id(0);  
   
   local REAL partial[W_SIZE];

 partial[lid]=IN[gid];
 
   barrier(CLK_LOCAL_MEM_FENCE);

   for(uint i = group_size/2; i > 0; i = i /2) {
      if(lid < i) {
         partial[lid] += partial[lid + i];
      }
      barrier(CLK_LOCAL_MEM_FENCE);
   }

   if(lid == 0) {
	   atomic_add_global(&(OUT[0]), partial[0]);	  
  	 
   }
 
 
}




