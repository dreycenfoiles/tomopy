//  Copyright (c) 2015, UChicago Argonne, LLC. All rights reserved.
//  Copyright 2015. UChicago Argonne, LLC. This software was produced
//  under U.S. Government contract DE-AC02-06CH11357 for Argonne National
//  Laboratory (ANL), which is operated by UChicago Argonne, LLC for the
//  U.S. Department of Energy. The U.S. Government has rights to use,
//  reproduce, and distribute this software.  NEITHER THE GOVERNMENT NOR
//  UChicago Argonne, LLC MAKES ANY WARRANTY, EXPRESS OR IMPLIED, OR
//  ASSUMES ANY LIABILITY FOR THE USE OF THIS SOFTWARE.  If software is
//  modified to produce derivative works, such modified software should
//  be clearly marked, so as not to confuse it with the version available
//  from ANL.
//  Additionally, redistribution and use in source and binary forms, with
//  or without modification, are permitted provided that the following
//  conditions are met:
//      * Redistributions of source code must retain the above copyright
//        notice, this list of conditions and the following disclaimer.
//      * Redistributions in binary form must reproduce the above copyright
//        notice, this list of conditions and the following disclaimer in
//        the documentation andwith the
//        distribution.
//      * Neither the name of UChicago Argonne, LLC, Argonne National
//        Laboratory, ANL, the U.S. Government, nor the names of its
//        contributors may be used to endorse or promote products derived
//        from this software without specific prior written permission.
//  THIS SOFTWARE IS PROVIDED BY UChicago Argonne, LLC AND CONTRIBUTORS
//  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
//  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
//  FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL UChicago
//  Argonne, LLC OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
//  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
//  BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
//  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
//  CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
//  LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
//  ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
//  POSSIBILITY OF SUCH DAMAGE.
//  ---------------------------------------------------------------
//   TOMOPY header

#pragma once

//--------------------------------------------------------------------------------------//

#include "common_GPU.hh"
#include "macros.hh"
#include "utils_GPU.hh"

BEGIN_EXTERN_C
#include "cxx_extern.h"
END_EXTERN_C

//--------------------------------------------------------------------------------------//
#if defined(WIN32)
#    define _Complex
#endif


//======================================================================================//
// interpolation types
#    define GPU_NN NPPI_INTER_NN
#    define GPU_LINEAR NPPI_INTER_LINEAR
#    define GPU_CUBIC NPPI_INTER_CUBIC

//======================================================================================//

inline int
GetNppInterpolationMode(const std::string& preferred)
{
    EnvChoiceList<int> choices = {
        EnvChoice<int>(GPU_NN, "NN", "nearest neighbor interpolation"),
        EnvChoice<int>(GPU_LINEAR, "LINEAR", "bilinear interpolation"),
        EnvChoice<int>(GPU_CUBIC, "CUBIC", "bicubic interpolation")
    };
    return GetChoice<int>(choices, preferred);
}

//======================================================================================//
//
#    if defined(__NVCC__)
//
//======================================================================================//

inline int
GetBlockSize(const int& init = 32)
{
    static thread_local int _instance = GetEnv<int>("TOMOPY_BLOCK_SIZE", init);
    return _instance;
}

//======================================================================================//

inline int
GetGridSize(const int& init = 0)
{
    // default value of zero == calculated according to block and loop size
    static thread_local int _instance = GetEnv<int>("TOMOPY_GRID_SIZE", init);
    return _instance;
}

//======================================================================================//

inline int
ComputeGridSize(const int& size, const int& block_size = GetBlockSize())
{
    return (size + block_size - 1) / block_size;
}

//======================================================================================//

inline dim3
GetBlockDims(const dim3& init = dim3(32, 32, 1))
{
    int _x = GetEnv<int>("TOMOPY_BLOCK_SIZE_X", init.x);
    int _y = GetEnv<int>("TOMOPY_BLOCK_SIZE_Y", init.y);
    int _z = GetEnv<int>("TOMOPY_BLOCK_SIZE_Z", init.z);
    return dim3(_x, _y, _z);
}

//======================================================================================//

inline dim3
GetGridDims(const dim3& init = dim3(0, 0, 0))
{
    // default value of zero == calculated according to block and loop size
    int _x = GetEnv<int>("TOMOPY_GRID_SIZE_X", init.x);
    int _y = GetEnv<int>("TOMOPY_GRID_SIZE_Y", init.y);
    int _z = GetEnv<int>("TOMOPY_GRID_SIZE_Z", init.z);
    return dim3(_x, _y, _z);
}

//======================================================================================//

inline dim3
ComputeGridDims(const dim3& dims, const dim3& blocks = GetBlockDims())
{
    return dim3(ComputeGridSize(dims.x, blocks.x), ComputeGridSize(dims.y, blocks.y),
                ComputeGridSize(dims.z, blocks.z));
}

//======================================================================================//

inline int&
this_thread_device()
{
    // this creates a globally accessible function for determining the device
    // the thread is assigned to
    //
    static std::atomic<int> _ntid(0);
    static thread_local int _instance =
        (cuda_device_count() > 0) ? ((_ntid++) % cuda_device_count()) : 0;
    return _instance;

}

//======================================================================================//

inline void
event_sync(cudaEvent_t _event)
{
    cudaEventSynchronize(_event);
    CUDA_CHECK_LAST_ERROR();
}

//======================================================================================//

template <typename _Tp>
_Tp*
gpu_malloc(uintmax_t size)
{
    _Tp* _gpu;
    CUDA_CHECK_CALL(cudaMalloc(&_gpu, size * sizeof(_Tp)));
    if(_gpu == nullptr)
    {
        int _device = 0;
        cudaGetDevice(&_device);
        std::stringstream ss;
        ss << "Error allocating memory on GPU " << _device << " of size "
           << (size * sizeof(_Tp)) << " and type " << typeid(_Tp).name()
           << " (type size = " << sizeof(_Tp) << ")";
        throw std::runtime_error(ss.str().c_str());
    }
    return _gpu;
}

//--------------------------------------------------------------------------------------//

template <typename _Tp>
void
cpu2gpu_memcpy(_Tp* _gpu, const _Tp* _cpu, uintmax_t size, cudaStream_t stream)
{
    cudaMemcpyAsync(_gpu, _cpu, size * sizeof(_Tp), cudaMemcpyHostToDevice, stream);
    CUDA_CHECK_LAST_STREAM_ERROR(stream);
}

//--------------------------------------------------------------------------------------//

template <typename _Tp>
void
gpu2cpu_memcpy(_Tp* _cpu, const _Tp* _gpu, uintmax_t size, cudaStream_t stream)
{
    cudaMemcpyAsync(_cpu, _gpu, size * sizeof(_Tp), cudaMemcpyDeviceToHost, stream);
    CUDA_CHECK_LAST_STREAM_ERROR(stream);
}

//--------------------------------------------------------------------------------------//

template <typename _Tp>
void
gpu2gpu_memcpy(_Tp* _dst, const _Tp* _src, uintmax_t size, cudaStream_t stream)
{
    cudaMemcpyAsync(_dst, _src, size * sizeof(_Tp), cudaMemcpyDeviceToDevice, stream);
    CUDA_CHECK_LAST_STREAM_ERROR(stream);
}

//--------------------------------------------------------------------------------------//

template <typename _Tp>
void
gpu_memset(_Tp* _gpu, int value, uintmax_t size, cudaStream_t stream)
{
    cudaMemsetAsync(_gpu, value, size * sizeof(_Tp), stream);
    CUDA_CHECK_LAST_STREAM_ERROR(stream);
}

//======================================================================================//

template <typename _Tp>
_Tp*
gpu_malloc_and_memcpy(const _Tp* _cpu, uintmax_t size, cudaStream_t stream)
{
    _Tp* _gpu = gpu_malloc<_Tp>(size);
    cudaMemcpyAsync(_gpu, _cpu, size * sizeof(_Tp), cudaMemcpyHostToDevice, stream);
    CUDA_CHECK_LAST_STREAM_ERROR(stream);
    return _gpu;
}

//--------------------------------------------------------------------------------------//

template <typename _Tp>
_Tp*
gpu_malloc_and_memset(uintmax_t size, int value, cudaStream_t stream)
{
    _Tp* _gpu = gpu_malloc<_Tp>(size);
    cudaMemsetAsync(_gpu, value, size * sizeof(_Tp), stream);
    CUDA_CHECK_LAST_STREAM_ERROR(stream);
    return _gpu;
}

//--------------------------------------------------------------------------------------//

template <typename _Tp>
void
gpu2cpu_memcpy_and_free(_Tp* _cpu, _Tp* _gpu, uintmax_t size, cudaStream_t stream)
{
    cudaMemcpyAsync(_cpu, _gpu, size * sizeof(_Tp), cudaMemcpyDeviceToHost, stream);
    CUDA_CHECK_LAST_STREAM_ERROR(stream);
    cudaFree(_gpu);
    CUDA_CHECK_LAST_ERROR();
}

//======================================================================================//

inline cudaStream_t*
create_streams(const int nstreams, unsigned int flag = cudaStreamDefault)
{
    cudaStream_t* streams = new cudaStream_t[nstreams];
    for(int i = 0; i < nstreams; ++i)
    {
        cudaStreamCreateWithFlags(&streams[i], flag);
        CUDA_CHECK_LAST_STREAM_ERROR(streams[i]);
    }
    return streams;
}

//======================================================================================//

inline void
destroy_streams(cudaStream_t* streams, const int nstreams)
{
    for(int i = 0; i < nstreams; ++i)
    {
        cudaStreamSynchronize(streams[i]);
        CUDA_CHECK_LAST_STREAM_ERROR(streams[i]);
        cudaStreamDestroy(streams[i]);
        CUDA_CHECK_LAST_ERROR();
    }
    delete[] streams;
}

//======================================================================================//
// compute the sum_dist for the rotations

uint32_t*
cuda_compute_sum_dist(int dy, int dt, int dx, int nx, int ny, const float* theta);

//======================================================================================//
// warm up
//======================================================================================//

template <typename _Tp>
__global__ void
cuda_warmup_kernel(_Tp* _dst, uintmax_t size, const _Tp factor)
{
    auto i0      = blockIdx.x * blockDim.x + threadIdx.x;
    auto istride = blockDim.x * gridDim.x;
    for(auto i = i0; i < size; i += istride)
        *_dst += static_cast<_Tp>(factor);
}

//======================================================================================//
// sum kernels
//======================================================================================//

template <typename _Tp, typename _Up = _Tp>
__global__ void
cuda_sum_kernel(_Tp* dst, const _Up* src, uintmax_t size, const _Tp factor)
{
    auto i0      = blockIdx.x * blockDim.x + threadIdx.x;
    auto istride = blockDim.x * gridDim.x;
    for(auto i = i0; i < size; i += istride)
        dst[i] += static_cast<_Tp>(factor * src[i]);
}

//--------------------------------------------------------------------------------------//

template <typename _Tp, typename _Up = _Tp>
__global__ void
cuda_atomic_sum_kernel(_Tp* dst, const _Up* src, uintmax_t size, const _Tp factor)
{
    auto i0      = blockIdx.x * blockDim.x + threadIdx.x;
    auto istride = blockDim.x * gridDim.x;
    for(auto i = i0; i < size; i += istride)
        atomicAdd(&dst[i], static_cast<_Tp>(factor * src[i]));
}

//======================================================================================//
//  reduction
//======================================================================================//

__global__ void
deviceReduceKernel(const float* in, float* out, int N);

//--------------------------------------------------------------------------------------//

__global__ void
sum_kernel_block(float* sum, const float* input, int n);

//--------------------------------------------------------------------------------------//

DLL float
deviceReduce(const float* in, float* out, int N);

//--------------------------------------------------------------------------------------//

DLL float
reduce(float* _in, float* _out, int size);

//======================================================================================//
//  rotate
//======================================================================================//

DLL int32_t*
    cuda_rotate(const int32_t* src, const float theta_rad, const float theta_deg,
                const int nx, const int ny, cudaStream_t stream, const int eInterp);

//--------------------------------------------------------------------------------------//

DLL float*
cuda_rotate(const float* src, const float theta_rad, const float theta_deg, const int nx,
            const int ny, cudaStream_t stream, const int eInterp);

//--------------------------------------------------------------------------------------//

DLL void
cuda_rotate_ip(int32_t* dst, const int32_t* src, const float theta_rad,
               const float theta_deg, const int nx, const int ny, cudaStream_t stream,
               const int eInterp);

//--------------------------------------------------------------------------------------//

DLL void
cuda_rotate_ip(float* dst, const float* src, const float theta_rad, const float theta_deg,
               const int nx, const int ny, cudaStream_t stream, const int eInterp);

//======================================================================================//

#    endif  // NVCC
