template <typename T, int S, int W>   // S is the size, in bits, of the pixel component. W is the width, in bit, of data type T
void streamCopy(hls::stream<T>& in_stream,  hls::stream<T>& out_stream1,  hls::stream<T>& out_stream2, unsigned int ImgSize, unsigned int NbImages)
{
	T tmp;
	for (int k=0; k<NbImages; k++)
	{
mem_wr1:
		for (int i = 0; i < (ImgSize*S/W); i++) {
	#pragma HLS pipeline
			tmp = in_stream.read();
			out_stream1.write(tmp);
			out_stream2.write(tmp);
		}
	}
}

template <typename T, int S, int W>   // S is the size, in bits, of the pixel component. W is the width, in bit, of data type T
void streamCopy(hls::stream<T>& in_stream,  hls::stream<T>& out_stream1,  hls::stream<T>& out_stream2, hls::stream<T>& out_stream3, unsigned int ImgSize, unsigned int NbImages)
{
	T tmp;
	for (int k=0; k<NbImages; k++)
	{
mem_wr1:
		for (int i = 0; i < (ImgSize*S/W); i++) {
	#pragma HLS pipeline
			tmp = in_stream.read();
			out_stream1.write(tmp);
			out_stream2.write(tmp);
			out_stream3.write(tmp);
		}
	}
}

template <typename T, int S, int W>   // S is the size, in bits, of the pixel component. W is the width, in bit, of data type T
void streamCopy(hls::stream<T>& in_stream,  hls::stream<T>& out_stream1,  hls::stream<T>& out_stream2, hls::stream<T>& out_stream3, hls::stream<T>& out_stream4, unsigned int ImgSize, unsigned int NbImages)
{
	T tmp;
	for (int k=0; k<NbImages; k++)
	{
mem_wr1:
		for (int i = 0; i < (ImgSize*S/W); i++) {
	#pragma HLS pipeline
			tmp = in_stream.read();
			out_stream1.write(tmp);
			out_stream2.write(tmp);
			out_stream3.write(tmp);
			out_stream4.write(tmp);
		}
	}
}

template <typename T, int S, int W>   // S is the size, in bits, of the pixel component. W is the width, in bit, of data type T
void streamCopy(hls::stream<T>& in_stream,  hls::stream<T>& out_stream1,  hls::stream<T>& out_stream2, hls::stream<T>& out_stream3, hls::stream<T>& out_stream4, hls::stream<T>& out_stream5, unsigned int ImgSize, unsigned int NbImages)
{
	T tmp;
	for (int k=0; k<NbImages; k++)
	{
mem_wr1:
		for (int i = 0; i < (ImgSize*S/W); i++) {
	#pragma HLS pipeline
			tmp = in_stream.read();
			out_stream1.write(tmp);
			out_stream2.write(tmp);
			out_stream3.write(tmp);
			out_stream4.write(tmp);
			out_stream5.write(tmp);
		}
	}
}

template <typename T, int S, int W>   // S is the size, in bits, of the pixel component. W is the width, in bit, of data type T
void split2Channels(hls::stream<T>& sin, hls::stream<T>& soutA, hls::stream<T>& soutB, unsigned int ImgSize, unsigned int NbImages)
{
	T tmp;
	unsigned short phase = 1;
	for (int k=0; k<NbImages; k++)
	{
mem_wr1:
		if (phase == 1)
		{
			for (int i = 0; i < (ImgSize*S/W); i++) {
		#pragma HLS pipeline
				tmp = sin.read();
				soutA.write(tmp);
			}
			phase = 2;
		}
		else
		{
			for (int i = 0; i < (ImgSize*S/W); i++) {
		#pragma HLS pipeline
				tmp = sin.read();
				soutB.write(tmp);
			}
			phase = 1;
		}
	}
}

template <typename T, int S, int W>   // S is the size, in bits, of the pixel component. W is the width, in bit, of data type T
void merge2Channels(hls::stream<T>& sinA, hls::stream<T>& sinB, hls::stream<T>& sout, unsigned int ImgSize, unsigned int NbImages)
{
	T tmp;
	unsigned short phase = 1;
	for (int k=0; k<NbImages; k++)
	{
mem_wr1:
		if (phase == 1)
		{
			for (int i = 0; i < (ImgSize*S/W); i++) {
		#pragma HLS pipeline
				tmp = sinA.read();
				sout.write(tmp);
			}
			phase = 2;
		}
		else
		{
			for (int i = 0; i < (ImgSize*S/W); i++) {
		#pragma HLS pipeline
				tmp = sinB.read();
				sout.write(tmp);
			}
			phase = 1;
		}
	}
}

/********************************************************************************
 *                               doubleStreamWidth                               *
 ********************************************************************************/

/********************************************************************************
 *                               16 up to 32 bytes                               *
 ********************************************************************************/
// input stream is an axistream
void doubleStreamWidth(hls::stream<io_stream_16B>& in_stream,  hls::stream<dt32>& out_stream,  unsigned int ImgSize, unsigned int NbImages)
{
	io_stream_16B tmp1, tmp2;
	dt32 tmp;
	for (int k=0; k<NbImages; k++)
	{
mem_wr1:
		for (int i = 0; i < (ImgSize*SIZE/(256)); i++) {
	#pragma HLS pipeline
			tmp1 = in_stream.read();
			tmp2 = in_stream.read();
			tmp.range(16*SIZE-1, 0) = tmp1.data;
			tmp.range(32*SIZE-1, 16*SIZE) = tmp2.data;
			out_stream.write(tmp);
		}
	}
}

void doubleStreamWidth(hls::stream<dt16>& in_stream,  hls::stream<io_stream_32B>& out_stream,  unsigned int ImgSize, unsigned int NbImages)
{
	dt16 tmp1, tmp2;
	io_stream_32B tmp;
	for (int k=0; k<NbImages; k++)
	{
		tmp.last = false;
		for (int i = 0; i < (ImgSize*SIZE/(256)); i++) {
	#pragma HLS pipeline
			tmp1 = in_stream.read();
			tmp2 = in_stream.read();
			tmp.data.range(16*SIZE-1, 0) = tmp1;
			tmp.data.range(32*SIZE-1, 16*SIZE) = tmp2;
			out_stream.write(tmp);
		}
	}
}
void doubleStreamWidth(hls::stream<dt16>& in_stream,  hls::stream<dt32>& out_stream,  unsigned int ImgSize, unsigned int NbImages)
{
	dt16 tmp1, tmp2;
	dt32 tmp;
	for (int k=0; k<NbImages; k++)
	{
mem_wr1:
		for (int i = 0; i < (ImgSize*SIZE/(256)); i++) {
	#pragma HLS pipeline
			tmp1 = in_stream.read();
			tmp2 = in_stream.read();
			tmp.range(16*SIZE-1, 0) = tmp1;
			tmp.range(32*SIZE-1, 16*SIZE) = tmp2;
			out_stream.write(tmp);
		}
	}
}

/********************************************************************************
 *                               8 up to 32 bytes                               *
 ********************************************************************************/
// input stream is an axistream
void doubleStreamWidth(hls::stream<io_stream_8B>& in_stream,  hls::stream<dt16>& out_stream,  unsigned int ImgSize, unsigned int NbImages)
{
	io_stream_8B tmp1, tmp2;
	dt16 tmp;
	for (int k=0; k<NbImages; k++)
	{
mem_wr1:
		for (int i = 0; i < (ImgSize*SIZE/(128)); i++) {
	#pragma HLS pipeline
			tmp1 = in_stream.read();
			tmp2 = in_stream.read();
			tmp.range(8*SIZE-1, 0) = tmp1.data;
			tmp.range(16*SIZE-1, 8*SIZE) = tmp2.data;
			out_stream.write(tmp);
		}
	}
}

void doubleStreamWidth(hls::stream<dt8>& in_stream,  hls::stream<io_stream_16B>& out_stream,  unsigned int ImgSize, unsigned int NbImages)
{
	dt8 tmp1, tmp2;
	io_stream_16B tmp;
	for (int k=0; k<NbImages; k++)
	{
		tmp.last = false;
		for (int i = 0; i < (ImgSize*SIZE/(128)); i++) {
	#pragma HLS pipeline
			tmp1 = in_stream.read();
			tmp2 = in_stream.read();
			tmp.data.range(8*SIZE-1, 0) = tmp1;
			tmp.data.range(16*SIZE-1, 8*SIZE) = tmp2;
			out_stream.write(tmp);
		}
	}
}
void doubleStreamWidth(hls::stream<dt8>& in_stream,  hls::stream<dt16>& out_stream,  unsigned int ImgSize, unsigned int NbImages)
{
	dt8 tmp1, tmp2;
	dt16 tmp;
	for (int k=0; k<NbImages; k++)
	{
mem_wr1:
		for (int i = 0; i < (ImgSize*SIZE/(128)); i++) {
	#pragma HLS pipeline
			tmp1 = in_stream.read();
			tmp2 = in_stream.read();
			tmp.range(8*SIZE-1, 0) = tmp1;
			tmp.range(16*SIZE-1, 8*SIZE) = tmp2;
			out_stream.write(tmp);
		}
	}
}
/********************************************************************************
 *                               halveStreamWidth                               *
 ********************************************************************************/

/********************************************************************************
 *                               32 downto 16 bytes                               *
 ********************************************************************************/
void halveStreamWidth(hls::stream<dt32>& in_stream,  hls::stream<dt16>& out_stream,  unsigned int ImgSize, unsigned int NbImages)
{
	dt32 tmp;
	dt16 tmp1, tmp2;

	for (int k=0; k<NbImages; k++)
	{
mem_wr1:
		for (int i = 0; i < (ImgSize*SIZE/256); i++) {
	#pragma HLS pipeline
			tmp = in_stream.read();
			tmp1 = tmp.range(16*SIZE-1, 0);
			tmp2 = tmp.range(32*SIZE-1, 16*SIZE);
			out_stream.write(tmp1);
			out_stream.write(tmp2);
		}
	}
}

void halveStreamWidth(hls::stream<io_stream_32B>& in_stream,  hls::stream<dt16>& out_stream,  unsigned int ImgSize, unsigned int NbImages)
{
	io_stream_32B tmp;
	dt16 tmp1, tmp2;

	for (int k=0; k<NbImages; k++)
	{
		for (int i = 0; i < ((ImgSize*SIZE)/256); i++) {
	#pragma HLS pipeline
			tmp = in_stream.read();
			tmp1 = tmp.data.range(16*SIZE-1, 0);
			tmp2 = tmp.data.range(32*SIZE-1, 16*SIZE);
			out_stream.write(tmp1);
			out_stream.write(tmp2);
		}
	}
}
// output stream is an axistream
void halveStreamWidth(hls::stream<dt32>& in_stream,  hls::stream<io_stream_16B>& out_stream,  unsigned int ImgSize, unsigned int NbImages)
{
	dt32 tmp;
	io_stream_16B tmp1, tmp2;

	for (int k=0; k<NbImages; k++)
	{
		tmp1.last = 0;
		tmp2.last = 0;
		for (int i = 0; i < ((ImgSize*SIZE)/256); i++) {
	#pragma HLS pipeline
			tmp = in_stream.read();
			tmp1.data = tmp.range(16*SIZE-1, 0);
			tmp2.data = tmp.range(32*SIZE-1, 16*SIZE);
			out_stream.write(tmp1);
			out_stream.write(tmp2);
		}
	}
}


/********************************************************************************
 *                               16 downto 8 bytes                              *
 ********************************************************************************/
void halveStreamWidth(hls::stream<dt16>& in_stream,  hls::stream<dt8>& out_stream,  unsigned int ImgSize, unsigned int NbImages)
{
	dt16 tmp;
	dt8 tmp1, tmp2;

	for (int k=0; k<NbImages; k++)
	{
mem_wr1:
		for (int i = 0; i < (ImgSize*SIZE/128); i++) {
	#pragma HLS pipeline
			tmp = in_stream.read();
			tmp1 = tmp.range(8*SIZE-1, 0);
			tmp2 = tmp.range(16*SIZE-1, 8*SIZE);
			out_stream.write(tmp1);
			out_stream.write(tmp2);
		}
	}
}

void halveStreamWidth(hls::stream<io_stream_16B>& in_stream,  hls::stream<dt8>& out_stream,  unsigned int ImgSize, unsigned int NbImages)
{
	io_stream_16B tmp;
	dt8 tmp1, tmp2;

	for (int k=0; k<NbImages; k++)
	{
		for (int i = 0; i < ((ImgSize*SIZE)/128); i++) {
	#pragma HLS pipeline
			tmp = in_stream.read();
			tmp1 = tmp.data.range(8*SIZE-1, 0);
			tmp2 = tmp.data.range(16*SIZE-1, 8*SIZE);
			out_stream.write(tmp1);
			out_stream.write(tmp2);
		}
	}
}
// output stream is an axistream
void halveStreamWidth(hls::stream<dt16>& in_stream,  hls::stream<io_stream_8B>& out_stream,  unsigned int ImgSize, unsigned int NbImages)
{
	dt16 tmp;
	io_stream_8B tmp1, tmp2;

	for (int k=0; k<NbImages; k++)
	{
		tmp1.last = 0;
		tmp2.last = 0;
		for (int i = 0; i < ((ImgSize*SIZE)/128); i++) {
	#pragma HLS pipeline
			tmp = in_stream.read();
			tmp1.data = tmp.range(8*SIZE-1, 0);
			tmp2.data = tmp.range(16*SIZE-1, 8*SIZE);
			out_stream.write(tmp1);
			out_stream.write(tmp2);
		}
	}
}

void convertToAxiStream(hls::stream<dt32>& in_stream,  hls::stream<io_stream_32B>& out_stream,  unsigned int ImgSize, unsigned int NbImages)
{
	dt32 tmp;
	io_stream_32B tmp1;

	for (int k=0; k<NbImages; k++)
	{
		tmp1.last = 0;
		tmp1.keep = 0xFFFFFFFF;
		for (int i = 0; i < ((ImgSize*SIZE)/256); i++) {
	#pragma HLS pipeline
			tmp = in_stream.read();
			tmp1.data = tmp;
			out_stream.write(tmp1);
		}
	}

}


void convertToAxiStream(hls::stream<dt16>& in_stream,  hls::stream<io_stream_16B>& out_stream,  unsigned int ImgSize, unsigned int NbImages)
{
	dt16 tmp;
	io_stream_16B tmp1;

	for (int k=0; k<NbImages; k++)
	{
		tmp1.last = 0;
		tmp1.keep = 0xFFFF;
		for (int i = 0; i < ((ImgSize*SIZE)/128); i++) {
	#pragma HLS pipeline
			tmp = in_stream.read();
			tmp1.data = tmp;
			out_stream.write(tmp1);
		}
	}

}

void convertFromAxiStream(hls::stream<io_stream_32B>& in_stream,  hls::stream<dt32>& out_stream,  unsigned int ImgSize, unsigned int NbImages)
{
	dt32 tmp;
	io_stream_32B tmp1;

	for (int k=0; k<NbImages; k++)
	{
		for (int i = 0; i < ((ImgSize*SIZE)/256); i++) {
	#pragma HLS pipeline
			tmp1 = in_stream.read();
			tmp = tmp1.data ;
			out_stream.write(tmp);
		}
	}

}

void convertFromAxiStream(hls::stream<io_stream_16B>& in_stream,  hls::stream<dt16>& out_stream,  unsigned int ImgSize, unsigned int NbImages)
{
	dt16 tmp;
	io_stream_16B tmp1;

	for (int k=0; k<NbImages; k++)
	{
		for (int i = 0; i < ((ImgSize*SIZE)/128); i++) {
	#pragma HLS pipeline
			tmp1 = in_stream.read();
			tmp = tmp1.data ;
			out_stream.write(tmp);
		}
	}
}
