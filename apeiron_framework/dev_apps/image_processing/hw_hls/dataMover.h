#include "type_definitions.h"

template <typename T, int S, int W>   // S is the size, in bits, of the pixel component. W is the width, in bit, of data type T
void Stream2Buff(hls::stream<T>& inStream, T* Buffer, int NbData);;


template <typename T, int S, int W>   // S is the size, in bits, of the pixel component. W is the width, in bit, of data type T
void Buff2Mem(T* Buffer,T* Mem, int NbData, unsigned int offset);;


template <typename T, int S, int W>   // S is the size, in bits, of the pixel component. W is the width, in bit, of data type T
void bufferedStream2Memory(	T* outMem,
							hls::stream<T>& inStream,
							unsigned int ImgSize, unsigned int NbImages);

template <typename T, int S, int W>   // S is the size, in bits, of the pixel component. W is the width, in bit, of data type T
void stream2Memory(T* outMem,
				   hls::stream<T>& inStream,
				   unsigned int ImgSize, unsigned int NbImages);


template <typename T, int S, int W>   // S is the size, in bits, of the pixel component. W is the width, in bit, of data type T
void Buff2Stream(T* Buffer, hls::stream<T>& inStream, int NbData);


template <typename T, int S, int W>   // S is the size, in bits, of the pixel component. W is the width, in bit, of data type T
void Mem2Buff(T* Mem, T* Buffer,int NbData, unsigned int offset);



template <typename T, int S, int W>   // S is the size, in bits, of the pixel component. W is the width, in bit, of data type T
void bufferedMemory2Stream(	T* inMem,
							hls::stream<T>& outStream,
							unsigned int ImgSize, unsigned int NbImages);

template <typename T, int S, int W>   // S is the size, in bits, of the pixel component. W is the width, in bit, of data type T
void memory2Stream(T* inMem, hls::stream<T>& outStream, unsigned int ImgSize, unsigned int NbImages);
