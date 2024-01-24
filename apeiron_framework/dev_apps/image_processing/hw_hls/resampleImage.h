template <typename T, int S, int W>   // S is the size, in bits, of the pixel component. W is the width, in bit, of data type T
void subsampleImageBy2(hls::stream<T>& inStream, hls::stream<T>& outStream, unsigned short int ImgRows, unsigned short int ImgCols, unsigned int NbImages)
{
	T tmp1, tmp2, outImg;
	unsigned int count = 0;
	unsigned char imgVal, outVal;
	for (unsigned int k=0; k<NbImages; k++)
	{
		for (unsigned short int rows = 0; rows< ImgRows; rows++)
		{
			for (unsigned short int cols = 0; cols < (ImgCols*S/W)/2; cols++) {
		#pragma HLS pipeline

				tmp1 = inStream.read();  // read N/SIZE components from stream
				tmp2 = inStream.read();  // read N/SIZE components from stream

				if ((rows&0xFFFE)==rows)  // even row; transmit half of the components
				{
					for (int j=0; j<(W/S); j+=2)
					 {
				#pragma HLS unroll
						imgVal = (tmp1.range(S * (j + 1) - 1, S * j).to_int());
						outImg.range(S*(j/2+1)-1,S*(j/2)) = imgVal;
						imgVal = (tmp2.range(S * (j + 1) - 1, S * j).to_int());
						outImg.range(S*(W/S)/2+S*(j/2+1)-1,S*(W/S)/2+S*(j/2)) = imgVal;
					 }
					outStream.write(outImg);
					count++;
				}
			}
		}
	}
}

template <typename T, int S, int W>   // S is the size, in bits, of the pixel component. W is the width, in bit, of data type T
void upsampleLine(T l[MAX_COMPONENT_LINE_SIZE], T ul[MAX_COMPONENT_LINE_SIZE], unsigned short int cols)
{
	T tmp, outImg1, outImg2;
	short int imgVal1, imgVal2;
	unsigned int i;

	for (i=0; i<cols*(S/W); i++)
	{
		tmp = l[i];  // read N/SIZE components from mmeory
		for (int j=0; j<(W/S)/2; j++)
		 {
	#pragma HLS unroll
			imgVal1 = (tmp.range(S * (j + 1) - 1, S * j).to_int());
			imgVal2 = (tmp.range(S * (j + 2) - 1, S * (j+1)).to_int());

			outImg1.range(S*(j*2+1)-1,S*(j*2)) = imgVal1;
			outImg1.range(S*(j*2+2)-1,S*(j*2+1)) = (imgVal1+imgVal2)/2;
		 }
		for (int j=(W/S)/2; j<(W/S); j++)
		 {
	#pragma HLS unroll
			imgVal1 = (tmp.range(S * (j + 1) - 1, S * j).to_int());
			if (j<(W/S)-1)
				imgVal2 = (tmp.range(S * (j + 2) - 1, S * (j+1)).to_int());
			else
				imgVal2 = imgVal1;

			outImg2.range(S*((j-(W/S)/2)*2+1)-1,S*((j-(W/S)/2)*2)) = imgVal1;
			outImg2.range(S*((j-(W/S)/2)*2+2)-1,S*((j-(W/S)/2)*2+1)) = (imgVal1+imgVal2)/2;
		 }

		ul[2*i] = outImg1;
		ul[2*i+1] = outImg2;
	}
}

/*
 * ul1 contains the already interpolated values from the previously read line
 * l2 is the new read line (to be upsampled)
 * values from l2 are upsampled (in the same line) and are interpolated with values in ul1, replacing ul1 values with the new ones
 */
template <typename T, int S, int W>   // S is the size, in bits, of the pixel component. W is the width, in bit, of data type T
void upsampleAndBilinearInterpolate(T l2[MAX_COMPONENT_LINE_SIZE],
		                            T ul1[MAX_COMPONENT_LINE_SIZE],
									T ul2[MAX_COMPONENT_LINE_SIZE],
									unsigned short int cols)
{
	T tmp, tmpNext, out_ul1_1, out_ul1_2, out_ul2_1, out_ul2_2, l1Val1, l1Val2;
	short int imgVal1, imgVal2, ul1Val, v1, v2, res;
	unsigned int i;

	tmp = l2[0];
	for (i=0; i<cols*S/W-1; i++)
	{
		l1Val1 = ul1[2*i];
		l1Val2 = ul1[2*i+1];
		tmpNext = l2[i+1];  // read N/SIZE components from memory
		for (int j=0; j<(W/S)/2; j++)
		 {
	#pragma HLS unroll
			imgVal1 = (tmp.range(S * (j + 1) - 1, S * j).to_int());
			imgVal2 = (tmp.range(S * (j + 2) - 1, S * (j+1)).to_int());

			out_ul2_1.range(S*(j*2+1)-1,S*(j*2)) = imgVal1;
			out_ul2_1.range(S*(j*2+2)-1,S*(j*2+1)) = (imgVal1+imgVal2)/2;
		 }
		for (int j=(W/S)/2; j<(W/S); j++)
		 {
	#pragma HLS unroll
			imgVal1 = (tmp.range(S * (j + 1) - 1, S * j).to_int());
			if (j<(W/S)-1)
				imgVal2 = (tmp.range(S * (j + 2) - 1, S * (j+1)).to_int());
			else
				imgVal2 = (tmpNext.range(S - 1, 0).to_int());;

			out_ul2_2.range(S*((j-(W/S)/2)*2+1)-1,S*((j-(W/S)/2)*2)) = imgVal1;
			out_ul2_2.range(S*((j-(W/S)/2)*2+2)-1,S*((j-(W/S)/2)*2+1)) = (imgVal1+imgVal2)/2;
		 }

		for (int j=0; j<(W/S); j++)
		 {
	#pragma HLS unroll
			v1 = (l1Val1.range(S * (j + 1) - 1, S * j).to_int());
			v2 = (out_ul2_1.range(S * (j + 1) - 1, S * j).to_int());
			res = v1/2+v2/2;
			out_ul1_1.range(S * (j + 1) - 1, S * j) = res;
		 }
		for (int j=0; j<(W/S); j++)
		 {
	#pragma HLS unroll
			v1 = (l1Val2.range(S * (j + 1) - 1, S * j).to_int());
			v2 = (out_ul2_2.range(S * (j + 1) - 1, S * j).to_int());
			res = v1/2+v2/2;
			out_ul1_2.range(S * (j + 1) - 1, S * j) = res;
		 }
		ul1[2*i] = out_ul1_1;
		ul1[2*i+1] = out_ul1_2;

		ul2[2*i] = out_ul2_1;
		ul2[2*i+1] = out_ul2_2;
		tmp = tmpNext;
	}
// process the last read values; i = ImgCols*Size/N-1
	l1Val1 = ul1[2*i];
	l1Val2 = ul1[2*i+1];
	for (int j=0; j<(W/S)/2; j++)
	 {
#pragma HLS unroll
		imgVal1 = (tmp.range(S * (j + 1) - 1, S * j).to_int());
		imgVal2 = (tmp.range(S * (j + 2) - 1, S * (j+1)).to_int());

		out_ul2_1.range(S*(j*2+1)-1,S*(j*2)) = imgVal1;
		out_ul2_1.range(S*(j*2+2)-1,S*(j*2+1)) = (imgVal1+imgVal2)/2;
	 }
	for (int j=(W/S)/2; j<(W/S); j++)
	 {
#pragma HLS unroll
		imgVal1 = (tmp.range(S * (j + 1) - 1, S * j).to_int());
		if (j<(W/S)-1)
			imgVal2 = (tmp.range(S * (j + 2) - 1, S * (j+1)).to_int());
		else
			imgVal2 = imgVal1;

		out_ul2_2.range(S*((j-(W/S)/2)*2+1)-1,S*((j-(W/S)/2)*2)) = imgVal1;
		out_ul2_2.range(S*((j-(W/S)/2)*2+2)-1,S*((j-(W/S)/2)*2+1)) = (imgVal1+imgVal2)/2;
	 }

	for (int j=0; j<(W/S); j++)
	 {
#pragma HLS unroll
		v1 = (l1Val1.range(S * (j + 1) - 1, S * j).to_int());
		v2 = (out_ul2_1.range(S * (j + 1) - 1, S * j).to_int());
		res = v1/2+v2/2;
		out_ul1_1.range(S * (j + 1) - 1, S * j) = res;
	 }
	for (int j=0; j<(W/S); j++)
	 {
#pragma HLS unroll
		v1 = (l1Val2.range(S * (j + 1) - 1, S * j).to_int());
		v2 = (out_ul2_2.range(S * (j + 1) - 1, S * j).to_int());
		res = v1/2+v2/2;
		out_ul1_2.range(S * (j + 1) - 1, S * j) = res;
	 }
	ul1[2*i] = out_ul1_1;
	ul1[2*i+1] = out_ul1_2;

	ul2[2*i] = out_ul2_1;
	ul2[2*i+1] = out_ul2_2;

}

template <typename T, int S, int W>   // S is the size, in bits, of the pixel component. W is the width, in bit, of data type T
void writeAndUpdateLines(T l1[MAX_COMPONENT_LINE_SIZE],T l2[MAX_COMPONENT_LINE_SIZE], hls::stream<dt>& outStream, unsigned short int cols)
{
	T tmp;

	unsigned int i;
	for (i=0; i<cols*S/W; i++)
	{
		tmp = l2[i];
		outStream.write(l1[i]);
		l1[i] = tmp;
	}
	for (i=0; i<cols*S/W; i++)
	{
		outStream.write(l2[i]);
	}
}

template <typename T, int S, int W>   // S is the size, in bits, of the pixel component. W is the width, in bit, of data type T
void upsampleImageX2(hls::stream<T>& inStream, hls::stream<T>& outStream, unsigned short int ImgRows, unsigned short int ImgCols, unsigned int NbImages)
{
	T lineA[MAX_COMPONENT_LINE_SIZE];
	T uline1A[MAX_COMPONENT_LINE_SIZE];
	T uline2A[MAX_COMPONENT_LINE_SIZE];
	T lineB[MAX_COMPONENT_LINE_SIZE];
	T uline1B[MAX_COMPONENT_LINE_SIZE];
	T uline2B[MAX_COMPONENT_LINE_SIZE];

	unsigned int i;
	const unsigned short int NbParallelInputWordsX2 = ((int)(ImgCols))*2*S/W;
	const unsigned short int NbParallelInputWords = ((int)(ImgCols))*S/W;
	const unsigned short int upsampledCols = 2*ImgCols;
	unsigned char phase;

	for (unsigned int k=0; k<NbImages; k++)
	{
		readline<T>(inStream,lineA, NbParallelInputWords);
		upsampleLine<T,S,W>(lineA,uline1A, ImgCols);
		writeline<T>(outStream, uline1A, NbParallelInputWordsX2);

		// start of first stage
		readline<T>(inStream,lineB, NbParallelInputWords);

		// second stage
		readline<T>(inStream,lineA, NbParallelInputWords);
		upsampleAndBilinearInterpolate<T,S,W>(lineB,uline1B,uline2B, ImgCols);

		phase = 0;
		for (i=3; i<ImgRows; i++)
		{
			if (phase == 0)
			{
				readline<T>(inStream,lineB, NbParallelInputWords);
				upsampleAndBilinearInterpolate<T,S,W>(lineA,uline1A,uline2A, ImgCols);
				writeAndUpdateLines<T,S,W>(uline1B,uline2B, outStream, upsampledCols);
				phase = 1;
			}
			else
			{
				readline<T>(inStream,lineA, NbParallelInputWords);
				upsampleAndBilinearInterpolate<T,S,W>(lineB,uline1B,uline2B, ImgCols);
				writeAndUpdateLines<T,S,W>(uline1A,uline2A, outStream, upsampledCols);
				phase = 0;
			}
		}

		// tail of the pipeline
		if (phase == 0)
		{
			upsampleAndBilinearInterpolate<T,S,W>(lineA,uline1A,uline2A, ImgCols);
			writeAndUpdateLines<T,S,W>(uline1B,uline2B, outStream, upsampledCols);
			phase = 1;
		}
		else
		{
			upsampleAndBilinearInterpolate<T,S,W>(lineB,uline1B,uline2B, ImgCols);
			writeAndUpdateLines<T,S,W>(uline1A,uline2A, outStream, upsampledCols);
			phase = 0;
		}

		// tail of the pipeline - last stage
		if (phase == 0)
		{
			writeAndUpdateLines<T,S,W>(uline1B,uline2B, outStream, upsampledCols);
			phase = 1;
		}
		else
		{
			writeAndUpdateLines<T,S,W>(uline1A,uline2A, outStream, upsampledCols);
			phase = 0;
		}

		if (phase == 1)
			writeline<T>(outStream, uline2B, NbParallelInputWordsX2);
		else
			writeline<T>(outStream, uline2A, NbParallelInputWordsX2);
	}

}
