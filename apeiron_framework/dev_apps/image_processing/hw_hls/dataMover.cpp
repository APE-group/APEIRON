#include "type_definitions.h"
#define LOCAL_BUFF_SIZE 512

template <typename T, int S, int W>   // S is the size, in bits, of the pixel component. W is the width, in bit, of data type T
void Stream2Buff(hls::stream<T>& inStream, T* Buffer, int NbData)
{
	T tmp;
	unsigned int i=0;
	tmp = inStream.read();
    for (i=0;i< NbData-1; i++) {
        Buffer[i] = tmp;
        tmp = inStream.read();
    }
    Buffer[i] = tmp;
}

template <typename T, int S, int W>   // S is the size, in bits, of the pixel component. W is the width, in bit, of data type T
void Buff2Mem(T* Buffer,T* Mem, int NbData, unsigned int offset)
{
	for (unsigned int i=0; i<NbData; i++)
		Mem[offset + i] = Buffer[i];
}


template <typename T, int S, int W>   // S is the size, in bits, of the pixel component. W is the width, in bit, of data type T
void bufferedStream2Memory(	T* outMem,
							hls::stream<T>& inStream,
							unsigned int ImgSize, unsigned int NbImages)
{
	unsigned int offset = 0;
	unsigned int i=0;
	int NbData, NbData1, NbData2;
	bool phase = false;
	T Buffer1[LOCAL_BUFF_SIZE];
	T Buffer2[LOCAL_BUFF_SIZE];
	int remaining = (ImgSize/(W/S)*NbImages); // number of data words to be transfered

	NbData1 = minVal<int>(LOCAL_BUFF_SIZE, remaining);
	Stream2Buff<T,S,W>(inStream, Buffer1, NbData1);
	remaining -= NbData1;
	NbData2 = minVal<int>(LOCAL_BUFF_SIZE, remaining);

	while (remaining > 0)
	{
		if (phase == false)
		{
			Buff2Mem<T,S,W>(Buffer1, outMem, NbData1, offset);
			Stream2Buff<T,S,W>(inStream, Buffer2, NbData2);
			remaining -= NbData2;
			offset += NbData1;
			NbData1 = minVal<int>(LOCAL_BUFF_SIZE, remaining);
		}
		else
		{
			Buff2Mem<T,S,W>(Buffer2, outMem, NbData2, offset);
			Stream2Buff<T,S,W>(inStream, Buffer1, NbData1);
			remaining -= NbData1;
			offset += NbData2;
			NbData2 = minVal<int>(LOCAL_BUFF_SIZE, remaining);
		}
		phase = !phase;
	}

	if (phase == false)
		Buff2Mem<T,S,W>(Buffer1, outMem, NbData1, offset);
	else
		Buff2Mem<T,S,W>(Buffer2, outMem, NbData2, offset);
}

template <typename T, int S, int W>   // S is the size, in bits, of the pixel component. W is the width, in bit, of data type T
void stream2Memory(T* outMem,
				   hls::stream<T>& inStream,
				   unsigned int ImgSize, unsigned int NbImages)
{
		T tmp;
		unsigned int i=0;
		for(int j=0; j<NbImages; j++){
			for (i = 0; i < ImgSize/(W/S); i++)
			{
				tmp = inStream.read();
				outMem[i] = tmp;
			}
		}
}

template <typename T, int S, int W>   // S is the size, in bits, of the pixel component. W is the width, in bit, of data type T
void Buff2Stream(T* Buffer, hls::stream<T>& inStream, int NbData)
{
	T tmp;
	unsigned int i=0;
    for (i=0;i< NbData; i++)
        inStream << Buffer[i];
}

template <typename T, int S, int W>   // S is the size, in bits, of the pixel component. W is the width, in bit, of data type T
void Mem2Buff(T* Mem, T* Buffer,int NbData, unsigned int offset)
{
	for (unsigned int i=0; i<NbData; i++)
		Buffer[i] = Mem[offset + i];
}


template <typename T, int S, int W>   // S is the size, in bits, of the pixel component. W is the width, in bit, of data type T
void bufferedMemory2Stream(	T* inMem,
							hls::stream<T>& outStream,
							unsigned int ImgSize, unsigned int NbImages)
{
	unsigned int offset = 0;
	unsigned int i=0;
	int NbData, NbData1, NbData2;
	bool phase = false;
	T Buffer1[LOCAL_BUFF_SIZE];
	T Buffer2[LOCAL_BUFF_SIZE];
	int remaining = (ImgSize/(W/S)*NbImages); // number of data words to be transfered

	NbData1 = minVal<int>(LOCAL_BUFF_SIZE, remaining);

	Mem2Buff<T,S,W>(inMem, Buffer1, NbData1, offset);
	offset += NbData1;
	remaining -= NbData1;
	NbData2 = minVal<int>(LOCAL_BUFF_SIZE, remaining);

	while (remaining > 0)
	{
		if (phase == false)
		{
			Buff2Stream<T,S,W>(Buffer1, outStream, NbData1);
			Mem2Buff<T,S,W>(inMem, Buffer2, NbData2, offset);
			remaining -= NbData2;
			NbData1 = minVal<int>(LOCAL_BUFF_SIZE, remaining);
			offset += NbData2;
		}
		else
		{
			Buff2Stream<T,S,W>(Buffer2, outStream, NbData2);
			Mem2Buff<T,S,W>(inMem, Buffer1, NbData1, offset);
			remaining -= NbData1;
			NbData2 = minVal<int>(LOCAL_BUFF_SIZE, remaining);
			offset += NbData1;
		}
		phase = !phase;
	}


	if (phase == false)
		Buff2Stream<T,S,W>(Buffer1, outStream, NbData1);
	else
		Buff2Stream<T,S,W>(Buffer2, outStream, NbData2);
}
template <typename T, int S, int W>   // S is the size, in bits, of the pixel component. W is the width, in bit, of data type T
void memory2Stream(T* inMem, hls::stream<T>& outStream, unsigned int ImgSize, unsigned int NbImages)
{
	unsigned int upperBound=ImgSize/(W/S);
	mem_rd1:
		for(int j=0; j<NbImages; j++){	
			for (int i = 0; i < upperBound; i++) {
				outStream << inMem[i];
			}
		}
}
