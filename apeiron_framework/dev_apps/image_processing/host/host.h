/*******************************************************************************
Vendor: Xilinx
Associated Filename: vadd.h
Purpose: VITIS vector addition
Revision History: January 28, 2016

*******************************************************************************
Copyright (C) 2019 XILINX, Inc.

This file contains confidential and proprietary information of Xilinx, Inc. and
is protected under U.S. and international copyright and other intellectual
property laws.

DISCLAIMER
This disclaimer is not a license and does not grant any rights to the materials
distributed herewith. Except as otherwise provided in a valid license issued to
you by Xilinx, and to the maximum extent permitted by applicable law:
(1) THESE MATERIALS ARE MADE AVAILABLE "AS IS" AND WITH ALL FAULTS, AND XILINX
HEREBY DISCLAIMS ALL WARRANTIES AND CONDITIONS, EXPRESS, IMPLIED, OR STATUTORY,
INCLUDING BUT NOT LIMITED TO WARRANTIES OF MERCHANTABILITY, NON-INFRINGEMENT, OR
FITNESS FOR ANY PARTICULAR PURPOSE; and (2) Xilinx shall not be liable (whether
in contract or tort, including negligence, or under any other theory of
liability) for any loss or damage of any kind or nature related to, arising under
or in connection with these materials, including for any direct, or any indirect,
special, incidental, or consequential loss or damage (including loss of data,
profits, goodwill, or any type of loss or damage suffered as a result of any
action brought by a third party) even if such damage or loss was reasonably
foreseeable or Xilinx had been advised of the possibility of the same.

CRITICAL APPLICATIONS
Xilinx products are not designed or intended to be fail-safe, or for use in any
application requiring fail-safe performance, such as life-support or safety
devices or systems, Class III medical devices, nuclear facilities, applications
related to the deployment of airbags, or any other applications that could lead
to death, personal injury, or severe property or environmental damage
(individually and collectively, "Critical Applications"). Customer assumes the
sole risk and liability of any use of Xilinx products in Critical Applications,
subject only to applicable laws and regulations governing limitations on product
liability.

THIS COPYRIGHT NOTICE AND DISCLAIMER MUST BE RETAINED AS PART OF THIS FILE AT
ALL TIMES.

*******************************************************************************/

#pragma once

#define CL_HPP_CL_1_2_DEFAULT_BUILD
#define CL_HPP_TARGET_OPENCL_VERSION 120
#define CL_HPP_MINIMUM_OPENCL_VERSION 120
#define CL_HPP_ENABLE_PROGRAM_CONSTRUCTION_FROM_ARRAY_COMPATIBILITY 1

//#include <CL/cl2.hpp>
#include <sys/types.h>
//#include <xrt/buffers.h>


#include <experimental/xrt_xclbin.h>
#include <xrt/xrt_device.h>
#include <xrt/xrt_bo.h>
#include <xrt/xrt_kernel.h>
#include <experimental/xrt_ip.h>
#include <experimental/xrt_profile.h>
#include <pthread.h>


//Customized buffer allocation for 4K boundary alignment
template <typename T>
struct aligned_allocator
{
  using value_type = T;
  T* allocate(std::size_t num)
  {
    void* ptr = nullptr;
    if (posix_memalign(&ptr,4096,num*sizeof(T)))
      throw std::bad_alloc();
    return reinterpret_cast<T*>(ptr);
  }
  void deallocate(T* p, std::size_t num)
  {
    free(p);
  }
};

#ifndef ZLIBMANAGER_H
#define ZLIBMANAGER_H

#include <string>

//header zlib
typedef struct _ZlibHeader
{
    unsigned char   cmf_cm:4,       // Compression method
                    cmf_cinfo:4;    // Compression info
    unsigned char   flg_fcheck:5,   //FCHECK  (check bits for CMF and FLG)
                    flg_fdict:1,    //FDICT   (preset dictionary)
                    flg_flevel:2;   //FLEVEL  (compression level)
}ZlibHeader;

//The FCHECK value must be such that CMF and FLG,
//when viewed as a 16-bit unsigned integer stored in MSB order
//(CMF*256 + FLG), is a multiple of 31.

//compression level:
// 0 - compressor used fastest algorithm
// 1 - compressor used fast algorithm
// 2 - compressor used default algorithm
// 3 - compressor used maximum compression, slowest algorithm
#define COMPRESSION_LEVEL__FASTEST  0
#define COMPRESSION_LEVEL__FAST     1
#define COMPRESSION_LEVEL__DEFAULT  2
#define COMPRESSION_LEVEL__SLOWEST  3

namespace ZlibManager
{
    //! \brief La funzione salva su file i dati compressi aggiungendo l'header compatibile con zlib
    //
    //! \param OutputFilePath path del file da creare
    //! \param UncompressedBuf buffer contenente i dati non compressi (viene utilizzato per calcolare l'adler32).
    //! \param UncompressedBufferSize dimensione del buffer contenente i dati non compressi.
    //! \param CompressedBuf buffer contenente i dati compressi.
    //! \param CpmpressedBufferSize dimensione del buffer contenente i dati compressi.
    //
    //! \return true in caso di successo; false in caso di errore
    bool CreateZlibFile(std::string OutputFilePath, unsigned char *CompressedBuf, uint64_t CpmpressedBufferSize, uint32_t adler);

    //! \brief La funzione restituisce l'header compatibile con zlib
    //
    //! \return header compatibile con zlib
    ZlibHeader GetZlibHeader();

    //! \brief La funzione restituisce l'Adler32 del buffer passato
    //
    //! \param OutputFilePath path del file da creare
    //! \param UncompressedBuf buffer contenente i dati non compressi (viene utilizzato per calcolare l'adler32).
    //! \param UncompressedBufferSize dimensione del buffer contenente i dati non compressi.
    //
    //! \return Adler32 del buffer passato
    uint32_t GetAdler32(unsigned char *UncompressedBuf, uint64_t UncompressedBufferSize);
};

#endif /* ZLIBMANAGER_H */


