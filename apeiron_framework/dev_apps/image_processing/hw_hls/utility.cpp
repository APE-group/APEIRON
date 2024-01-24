template <typename T>   // S is the size, in bits, of the pixel component. W is the width, in bit, of data type T
T maxVal(T a, T b)
{
#pragma HLS inline
	T ret;

	if (a > b)
	  ret = a;
	else
		ret = b;
	return ret;
}

template <typename T>   // S is the size, in bits, of the pixel component. W is the width, in bit, of data type T
T minVal(T a, T b)
{
#pragma HLS inline
	T ret;

	if (a < b)
	  ret = a;
	else
		ret = b;
	return ret;
}

void setNbImages(unsigned int n, unsigned int &nA, unsigned int &nB)
{
	if ((n&1) == 0)  //n is even
	{
		nA = n>>1;   // the two "long" functions perform the same number of iterations
		nB = n>>1;
	}
	else // n is odd
	{
		nA = (n>>1)+1;  // the first of the two "long" functions performs one additional iteration
		nB = n>>1;
	}
}

template <typename T>
static void sinkStream(hls::stream<T>& in_stream,unsigned int ImgSize, unsigned int NbImages)
{
	T tmp;
	for (int k=0; k<NbImages; k++)
	{
	mem_wr1:
		for (int i = 0; i < (ImgSize*SIZE/N); i++) {
	#pragma HLS pipeline
			tmp = in_stream.read();
		}
	}

}

template <typename T, int S, int W>  // S is the size, in bits, of the pixel component. W is the width, in bit, of data type T
void PassThrough(hls::stream<T>& in_stream,  hls::stream<T>& out_stream1,  unsigned int ImgSize, unsigned int NbImages)
{
	T tmp;
	for (int k=0; k<NbImages; k++)
	{
mem_wr1:
		for (int i = 0; i < (ImgSize*S/W); i++) {
	#pragma HLS pipeline
			tmp = in_stream.read();
			out_stream1.write(tmp);
			// debug section
//			if ((i==200)||(i==201))
//			{
//				printf("PTH in addr: %x out addr: %x i: %d data %x\n",&in_stream, &out_stream1, i, (unsigned int)tmp.range(31,0));
//				printf("PTH in addr: %x out addr: %x i: %d data %x\n",&in_stream, &out_stream1, i, (unsigned int)tmp.range(63,32));
//				printf("PTH in addr: %x out addr: %x i: %d data %x\n",&in_stream, &out_stream1, i, (unsigned int)tmp.range(95,64));
//				printf("PTH in addr: %x out addr: %x i: %d data %x\n",&in_stream, &out_stream1, i, (unsigned int)tmp.range(127,96));
//			}
			// end debug section
		}
	}
}


template <typename T, int S, int W>   // S is the size, in bits, of the pixel component. W is the width, in bit, of data type T
void printStreamToTextFile(hls::stream<T>& inStream, unsigned int streamId, unsigned int ImgSize, unsigned short int ImgCols, unsigned int NbImages)
{
	for (unsigned int k=0; k<NbImages; k++)
	{
		T tmp;
#ifndef __SYNTHESIS__
		 FILE *fp1; // The following code is ignored for synthesis
		 char filename[255];
		 sprintf(filename,"s%d_%d.csv",streamId,k);
		 fp1=fopen(filename,"wt");
#endif
		unsigned int i,j;
		for (i = 0; i < (ImgSize*S/W); i++)
		{
			tmp = inStream.read();
#ifndef __SYNTHESIS__
			for (j=0; j<(W/S); j++)
			 {
				fprintf(fp1,"%d, ", tmp.range(S*(j+1)-1,S*j).to_int() );
				if (i==0)
					printf("%d, ", tmp.range(S*(j+1)-1,S*j).to_int() );
			 }
			if ((i*W/S)%ImgCols == 0)
				fprintf(fp1,"\n");
#endif
		}
#ifndef __SYNTHESIS__
		 fclose(fp1);
#endif
	}
}
template <typename T, int S, int W>   // S is the size, in bits, of the pixel component. W is the width, in bit, of data type T
void printStreamToBinaryFile(hls::stream<T>& inStream, unsigned int streamId, unsigned int ImgSize, unsigned int NbImages)
{
	for (unsigned int k=0; k<NbImages; k++)
	{
		T tmp;
#ifndef __SYNTHESIS__
		 FILE *fp1; // The following code is ignored for synthesis
		 char filename[255];
		 sprintf(filename,"s%d_%d.raw",streamId,k);
		 fp1=fopen(filename,"wb");
#endif
		unsigned int i,j;
		for (i = 0; i < (ImgSize*S/W); i++)
		{
			tmp = inStream.read();
#ifndef __SYNTHESIS__
			for (j=0; j<(W/S); j++)
			 {
				unsigned int tmpChar;
				tmpChar = tmp.range(S*(j+1)-1,S*j).to_int();
				fwrite((const void *)&tmpChar,1,1,fp1);
			 }

#endif
		}
#ifndef __SYNTHESIS__
		 fclose(fp1);
#endif
	}
}

void setStreamId(unsigned int &id)
{
	static unsigned int internalID = 0;
	internalID++;
	id = internalID;
}

