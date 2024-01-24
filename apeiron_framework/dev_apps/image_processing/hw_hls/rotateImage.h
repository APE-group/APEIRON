
/*
 * receives from inStream an image and stores it in Buffer. Once completely stored, reads image from Buffer, rotates by 180 degree and sends it to outStream
 */
template <typename T, int S, int W>   // S is the size, in bits, of the pixel component. W is the width, in bit, of data type T
void rotateImage180(T* Buffer, hls::stream<T>& inStream, hls::stream<T>& outStream, unsigned int ImgSize, unsigned int NbImages)
{
	T tmp, tmp2;
	for (unsigned int k=0; k<NbImages; k++)
	{
		for (int i = 0; i < (ImgSize*S/W); i++) {
	#pragma HLS pipeline
			tmp = inStream.read();  //read the image and stores it into an external memory bank
			Buffer[i] = tmp;
		}

			// read again image from memory and sends it to the output stream
		for (int i = (ImgSize*S/W)-1; i >=0; i--) {
	#pragma HLS pipeline
			tmp = Buffer[i];
			for (unsigned int j=0; j<W/S; j++)
			{
			#pragma HLS unroll
				tmp2.range(S * (((W/S -1)-j) + 1) - 1, S * ((W/S -1)-j)) = tmp.range(S * (j + 1) - 1, S * j);
			}
			outStream.write(tmp2);
		}
	}
}

template <typename T, int S, int W>   // S is the size, in bits, of the pixel component. W is the width, in bit, of data type T
void writeReversedLine(hls::stream<T>& outStream, T *line, unsigned short int size)
{
	T tmp1, tmp2;
	unsigned short int j;

	for (short int i=size-1; i>=0; i--)
	{
		tmp1 = line[i];
		for (j=0; j<(W/S); j++)
		{
		#pragma HLS unroll
			tmp2.range(S * (((W/S -1)-j) + 1) - 1, S * ((W/S -1)-j)) = tmp1.range(S * (j + 1) - 1, S * j);
		}
		outStream.write(tmp2);
	}

}



template <typename T, int S, int W>   // S is the size, in bits, of the pixel component. W is the width, in bit, of data type T
void horizontalMirrorImage(hls::stream<T>& inStream,hls::stream<T>& outStream,
				  	  	  unsigned short int ImgRows, unsigned short int ImgCols, unsigned int NbImages)
{	int i;

T line1[MAX_COMPONENT_LINE_SIZE];
T line2[MAX_COMPONENT_LINE_SIZE];
unsigned int phase = 1;
const unsigned short int NbParallelInputWords = ImgCols*S/W;
readline<T>(inStream,line1,NbParallelInputWords);
for (int i=0; i<ImgRows*NbImages-1; i++)
{
	if (phase == 1)
	{
		writeReversedLine<T,S,W>(outStream, line1, NbParallelInputWords);
		readline<T>(inStream,line2,NbParallelInputWords);
		phase = 2;
	}
	else
	{
		writeReversedLine<T,S,W>(outStream, line2, NbParallelInputWords);
		readline<T>(inStream,line1,NbParallelInputWords);
		phase = 1;
	}
}
// write last line
if (phase == 1)
	writeReversedLine<T,S,W>(outStream, line1, NbParallelInputWords);
else
	writeReversedLine<T,S,W>(outStream, line2, NbParallelInputWords);
}


template <typename T, int S, int W>   // S is the size, in bits, of the pixel component. W is the width, in bit, of data type T
void verticalMirrorImage(T* Buffer, hls::stream<T>& inStream, hls::stream<T>& outStream, unsigned int ImgSize, unsigned short int ImgRows, unsigned short int ImgCols, unsigned int NbImages)
{
	T tmp;
	for (unsigned int k=0; k<NbImages; k++)
	{
		for (int i = 0; i < (ImgSize*S/W); i++) {
	#pragma HLS pipeline
			tmp = inStream.read();  //read the image and stores it into an external memory bank
			Buffer[i] = tmp;
		}

			// read again image from memory and sends it to the output stream
		for (int i = ImgRows-1; i >= 0; i--) {
			for (int k=0; k<ImgCols/(W/S); k++) {
		#pragma HLS pipeline
				tmp = Buffer[i*ImgCols/(W/S)+k];
				outStream.write(tmp);
			}
		}
	}
}

