#include <cstdio>
#include <cstdlib>
#include <vector>
#include <cuda_runtime.h>
#include <nccl.h>

#define CHECK(cmd) do {                                 \
  cudaError_t e = cmd;                                  \
  if(e != cudaSuccess) {                                \
    printf("Failed: Cuda error %s:%d '%s'\n",           \
        __FILE__,__LINE__,cudaGetErrorString(e));       \
    exit(EXIT_FAILURE);                                 \
  }                                                     \
} while(0)

#define NCCL_CHECK(cmd) do {                            \
  ncclResult_t r = cmd;                                 \
  if(r != ncclSuccess) {                                \
    printf("Failed, NCCL error %s:%d '%s'\n",           \
        __FILE__,__LINE__,ncclGetErrorString(r));       \
    exit(EXIT_FAILURE);                                 \
  }                                                     \
} while(0)

int main(int argc, char* argv[]) {
    int nGPUs = 4;
    size_t dataSize = 256*1024*1024;
    int nIters = 100;

    // 1. 选取设备
    std::vector<int> devs(nGPUs);
    for(int i = 0; i < nGPUs; ++i) devs[i] = i;

    // 2. 分配 CUDA 资源
    std::vector<float*> d_send(nGPUs), d_recv(nGPUs);
    std::vector<cudaStream_t> streams(nGPUs);
    for(int i = 0; i < nGPUs; ++i) {
        CHECK(cudaSetDevice(devs[i]));
        CHECK(cudaMalloc(&d_send[i], dataSize * sizeof(float)));
        CHECK(cudaMalloc(&d_recv[i], dataSize * sizeof(float)));
        CHECK(cudaStreamCreate(&streams[i]));
    }

    // 3. 初始化 NCCL
    ncclComm_t comms[4];
    NCCL_CHECK(ncclCommInitAll(comms, nGPUs, devs.data()));

    // 4. warmup
    NCCL_CHECK(ncclGroupStart());
    for(int i = 0; i < nGPUs; ++i) {
        CHECK(cudaSetDevice(devs[i]));
        NCCL_CHECK(ncclAllReduce(
            d_send[i], d_recv[i], dataSize, ncclFloat, ncclSum, comms[i], streams[i]));
    }
    NCCL_CHECK(ncclGroupEnd());
    for(int i = 0; i < nGPUs; ++i) {
        CHECK(cudaSetDevice(devs[i]));
        CHECK(cudaStreamSynchronize(streams[i]));
    }

    // 5. 测试
    cudaEvent_t start, stop;
    CHECK(cudaSetDevice(0));
    CHECK(cudaEventCreate(&start));
    CHECK(cudaEventCreate(&stop));
    CHECK(cudaEventRecord(start, streams[0]));

    for(int iter = 0; iter < nIters; ++iter) {
        NCCL_CHECK(ncclGroupStart());
        for(int i = 0; i < nGPUs; ++i) {
            CHECK(cudaSetDevice(devs[i]));
            NCCL_CHECK(ncclAllReduce(
                d_send[i], d_recv[i], dataSize, ncclFloat, ncclSum, comms[i], streams[i]));
        }
        NCCL_CHECK(ncclGroupEnd());
    }
    CHECK(cudaSetDevice(0));
    CHECK(cudaEventRecord(stop, streams[0]));
    for(int i = 0; i < nGPUs; ++i) {
        CHECK(cudaSetDevice(devs[i]));
        CHECK(cudaStreamSynchronize(streams[i]));
    }
    CHECK(cudaEventSynchronize(stop));

    float ms = 0;
    CHECK(cudaEventElapsedTime(&ms, start, stop));

    // 6. 计算带宽
    double totalBytes = double(nIters) * dataSize * sizeof(float);
    double gb = totalBytes / (1024 * 1024 * 1024);
    double sec = ms / 1000.0;
    printf("AllReduce average bandwidth: %.2f GB/s per GPU (total time %.2f ms)\n", gb/sec*(2*nGPUs-2) / double(nGPUs), ms);

    // 7. 清理
    for(int i = 0; i < nGPUs; ++i) {
        CHECK(cudaSetDevice(devs[i]));
        cudaFree(d_send[i]);
        cudaFree(d_recv[i]);
        cudaStreamDestroy(streams[i]);
        ncclCommDestroy(comms[i]);
    }
    cudaEventDestroy(start);
    cudaEventDestroy(stop);
    return 0;
}
