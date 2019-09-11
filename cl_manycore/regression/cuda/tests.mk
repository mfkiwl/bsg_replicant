REGRESSION_TESTS_TYPE = cuda
TILE_GROUP_DIM_X = 2
TILE_GROUP_DIM_Y = 2

UNIFIED_TESTS = test_scalar_print
UNIFIED_TESTS += test_empty
UNIFIED_TESTS += test_tile_info
UNIFIED_TESTS += test_barrier

INDEPENDENT_TESTS += test_empty_parallel
INDEPENDENT_TESTS += test_multiple_binary_load
INDEPENDENT_TESTS += test_memset
INDEPENDENT_TESTS += test_stack_load
INDEPENDENT_TESTS += test_dram_load_store
INDEPENDENT_TESTS += test_dram_host_allocated
INDEPENDENT_TESTS += test_dram_device_allocated
INDEPENDENT_TESTS += test_vec_add
INDEPENDENT_TESTS += test_vec_add_parallel
INDEPENDENT_TESTS += test_vec_add_parallel_multi_grid
INDEPENDENT_TESTS += test_vec_add_serial_multi_grid
INDEPENDENT_TESTS += test_vec_add_shared_mem
INDEPENDENT_TESTS += test_max_pool2d
INDEPENDENT_TESTS += test_shared_mem
INDEPENDENT_TESTS += test_shared_mem_load_store
INDEPENDENT_TESTS += test_matrix_mul
INDEPENDENT_TESTS += test_matrix_mul_shared_mem

INDEPENDENT_TESTS += test_float_vec_add
INDEPENDENT_TESTS += test_float_vec_add_shared_mem

REGRESSION_TESTS = $(UNIFIED_TESTS) $(INDEPENDENT_TESTS)

DEFINES += -DBSG_MANYCORE_DIR=$(abspath $(BSG_MANYCORE_DIR))
DEFINES += -DCL_MANYCORE_DIM_X=$(CL_MANYCORE_DIM_X) 
DEFINES += -DCL_MANYCORE_DIM_Y=$(CL_MANYCORE_DIM_Y)
DEFINES += -D_XOPEN_SOURCE=500 -D_BSD_SOURCE

CDEFINES   += $(DEFINES)
CXXDEFINES += $(DEFINES)

FLAGS     = -g -Wall
CFLAGS   += -std=c99 $(FLAGS) 
CXXFLAGS += -std=c++11 $(FLAGS)
LDFLAGS  += -lbsg_manycore_runtime -lm