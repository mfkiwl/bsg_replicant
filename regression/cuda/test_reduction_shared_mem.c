// Copyright (c) 2019, University of Washington All rights reserved.
// 
// Redistribution and use in source and binary forms, with or without modification,
// are permitted provided that the following conditions are met:
// 
// Redistributions of source code must retain the above copyright notice, this list
// of conditions and the following disclaimer.
// 
// Redistributions in binary form must reproduce the above copyright notice, this
// list of conditions and the following disclaimer in the documentation and/or
// other materials provided with the distribution.
// 
// Neither the name of the copyright holder nor the names of its contributors may
// be used to endorse or promote products derived from this software without
// specific prior written permission.
// 
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
// ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
// ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
// (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
// LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
// ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
// SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#include "test_reduction_shared_mem.h"

#define ALLOC_NAME "default_allocator"

/*!
 * Runs the reduction in tile group shared memory test on a 1x1 grid of 4x4 tile group.
 * Grid dimensions are fixed to 1x1.
 * This tests uses the software/spmd/bsg_cuda_lite_runtime/reduction_shared_mem/ Manycore binary in the BSG Manycore bitbucket repository.  
*/


void host_reduction (int *A, int N) { 
        for (int i = 1; i < N; i ++) { 
                A[0] += A[i];
        }
        return;
}


int kernel_reduction_shared_mem (int argc, char **argv) {
        int rc;
        char *bin_path, *test_name;
        struct arguments_path args = {NULL, NULL};

        argp_parse (&argp_path, argc, argv, 0, 0, &args);
        bin_path = args.path;
        test_name = args.name;

        bsg_pr_test_info("Running the CUDA Reduction Shared Memory Kernel on a grid of 4x4 tile groups.\n\n");

        srand(time); 

        /*****************************************************************************************************************
        * Define path to binary.
        * Initialize device, load binary and unfreeze tiles.
        ******************************************************************************************************************/
        hb_mc_device_t device;
        rc = hb_mc_device_init(&device, test_name, 0);
        if (rc != HB_MC_SUCCESS) { 
                bsg_pr_err("failed to initialize device.\n");
                return rc;
        }

        rc = hb_mc_device_program_init(&device, bin_path, ALLOC_NAME, 0);
        if (rc != HB_MC_SUCCESS) { 
                bsg_pr_err("failed to initialize program.\n");
                return rc;
        }

        /*****************************************************************************************************************
        * Allocate memory on the device for A, B and C.
        ******************************************************************************************************************/
        uint32_t N = 256;

        eva_t A_device;
        rc = hb_mc_device_malloc(&device, N * sizeof(uint32_t), &A_device); /* allocate A[N] on the device */
        if (rc != HB_MC_SUCCESS) { 
                bsg_pr_err("failed to allcoate memory on device.\n");
                return rc;
        }


        /*****************************************************************************************************************
        * Allocate memory on the host for A and initialize with random values.
        ******************************************************************************************************************/
        uint32_t A_host[N]; /* allocate A[N] on the host */ 
        for (int i = 0; i < N; i++) { /* fill A with arbitrary data */
                A_host[i] = i;     //rand() & 0xFFFF;
        }



        /*****************************************************************************************************************
        * Copy A from host onto device DRAM.
        ******************************************************************************************************************/
        void *dst = (void *) ((intptr_t) A_device);
        void *src = (void *) &A_host[0];
        rc = hb_mc_device_memcpy (&device, dst, src, N * sizeof(uint32_t), HB_MC_MEMCPY_TO_DEVICE); /* Copy A to the device  */ 
        if (rc != HB_MC_SUCCESS) { 
                bsg_pr_err("failed to copy memory to device.\n");
                return rc;
        }


        /*****************************************************************************************************************
        * Define tg_dim_x/y: number of tiles in each tile group
        * Grid dimensions are fixed to 1x1
        ******************************************************************************************************************/
        hb_mc_dimension_t tg_dim = { .x = 1, .y = 1 }; 

        hb_mc_dimension_t grid_dim = { .x = 1 , .y = 1 };


        /*****************************************************************************************************************
        * Prepare list of input arguments for kernel.
        ******************************************************************************************************************/
        int cuda_argv[5] = {A_device, N};

        /*****************************************************************************************************************
        * Enquque grid of tile groups, pass in grid and tile group dimensions, kernel name, number and list of input arguments
        ******************************************************************************************************************/
        rc = hb_mc_kernel_enqueue (&device, grid_dim, tg_dim, "kernel_reduction_shared_mem", 2, cuda_argv);
        if (rc != HB_MC_SUCCESS) { 
                bsg_pr_err("failed to initialize grid.\n");
                return rc;
        }


        /*****************************************************************************************************************
        * Launch and execute all tile groups on device and wait for all to finish. 
        ******************************************************************************************************************/
        rc = hb_mc_device_tile_groups_execute(&device);
        if (rc != HB_MC_SUCCESS) { 
                bsg_pr_err("failed to execute tile groups.\n");
                return rc;
        }


        /*****************************************************************************************************************
        * Copy result (the first element of A) back to host 
        ******************************************************************************************************************/
        uint32_t R_device;
        src = (void *) ((intptr_t) A_device);
        dst = (void *) &R_device;
        rc = hb_mc_device_memcpy (&device, (void *) dst, src, 1 * sizeof(uint32_t), HB_MC_MEMCPY_TO_HOST); /* copy C to the host */
        if (rc != HB_MC_SUCCESS) { 
                bsg_pr_err("failed to copy memory from device.\n");
                return rc;
        }


        /*****************************************************************************************************************
        * Freeze the tiles and memory manager cleanup. 
        ******************************************************************************************************************/
        rc = hb_mc_device_finish(&device); 
        if (rc != HB_MC_SUCCESS) { 
                bsg_pr_err("failed to de-initialize device.\n");
                return rc;
        }


        /*****************************************************************************************************************
        * Calculate the expected result using host code and compare the results. 
        ******************************************************************************************************************/
        uint32_t R_host; 
        host_reduction(A_host, N);
        R_host = A_host[0];


        int mismatch = 0; 
        if (R_host != R_device) {
                bsg_pr_err(BSG_RED("Mismatch: ") "R_host:  0x%08" PRIx32 "\t R_device: 0x%08" PRIx32 "\n", R_host, R_device);
                mismatch = 1;
        }
        else {
                bsg_pr_test_info(BSG_GREEN("Match: ") "R_host:  0x%08" PRIx32 "\t R_device: 0x%08" PRIx32 "\n", R_host, R_device);
        }


        if (mismatch) { 
                return HB_MC_FAIL;
        }
        return HB_MC_SUCCESS;
}

#ifdef COSIM
void cosim_main(uint32_t *exit_code, char * args) {
        // We aren't passed command line arguments directly so we parse them
        // from *args. args is a string from VCS - to pass a string of arguments
        // to args, pass c_args to VCS as follows: +c_args="<space separated
        // list of args>"
        int argc = get_argc(args);
        char *argv[argc];
        get_argv(args, argc, argv);

#ifdef VCS
        svScope scope;
        scope = svGetScopeFromName("tb");
        svSetScope(scope);
#endif
        bsg_pr_test_info("test_reduction_shared_mem Regression Test (COSIMULATION)\n");
        int rc = kernel_reduction_shared_mem(argc, argv);
        *exit_code = rc;
        bsg_pr_test_pass_fail(rc == HB_MC_SUCCESS);
        return;
}
#else
int main(int argc, char ** argv) {
        bsg_pr_test_info("test_reduction_shared_mem Regression Test (F1)\n");
        int rc = kernel_reduction_shared_mem(argc, argv);
        bsg_pr_test_pass_fail(rc == HB_MC_SUCCESS);
        return rc;
}
#endif

