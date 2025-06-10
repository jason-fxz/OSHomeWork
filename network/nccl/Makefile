all: nccl_test nccl_bandwidth_test

nccl_bandwidth_test: nccl_bandwidth_test.cpp
	nvcc nccl_bandwidth_test.cpp -o nccl_bandwidth_test -I${CONDA_PREFIX}/include -L${CONDA_PREFIX}/lib -lnccl
nccl_test: nccl_test.cpp
	nvcc nccl_test.cpp -o nccl_test -I${CONDA_PREFIX}/include -L${CONDA_PREFIX}/lib -lnccl
clean:
	rm -f nccl_test
nccl: nccl_test
	CUDA_VISIBLE_DEVICES=1,2,3 LD_LIBRARY_PATH=${CONDA_PREFIX}/lib ./nccl_test
bandwidth: nccl_bandwidth_test
	CUDA_VISIBLE_DEVICES=1,2,3 LD_LIBRARY_PATH=${CONDA_PREFIX}/lib ./nccl_bandwidth_test
.PHONY: all clean nccl bandwidth