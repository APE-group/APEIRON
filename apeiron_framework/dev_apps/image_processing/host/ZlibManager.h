/* 
 * File:   ZlibManager.h
 * Author: evo
 *
 * Created on 22 settembre 2022, 16:56
 */

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
    bool CreateZlibFile(std::string OutputFilePath, unsigned char *UncompressedBuf, uint64_t UncompressedBufferSize, unsigned char *CompressedBuf, uint64_t CpmpressedBufferSize);

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

