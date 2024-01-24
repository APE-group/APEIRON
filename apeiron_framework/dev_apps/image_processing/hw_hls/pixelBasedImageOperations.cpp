template <typename T, int S, int W>   // S is the size, in bits, of the pixel component. W is the width, in bit, of data type T
static void sumImages(hls::stream<T>& in_stream1,  hls::stream<T>& in_stream2,  hls::stream<T>& out_stream, unsigned int ImgSize, unsigned int NbImages)
{
	T tmp1, tmp2, res;
	short int d1, d2, r, aux;
	for (unsigned int k=0; k<NbImages; k++)
	{
	mem_wr1:
		for (int i = 0; i < (ImgSize*S/W); i++) {
	#pragma HLS pipeline
	#pragma HLS ALLOCATION instances=add limit=PIXEL_PROCESSED_IN_PARALLEL
			tmp1 = in_stream1.read();
			tmp2 = in_stream2.read();

			for (int j=0; j<(W/S); j++)
			 {
		#pragma HLS unroll

				d1 = (tmp1.range(S * (j + 1) - 1, S * j).to_int());
				d2 = (tmp2.range(S * (j + 1) - 1, S * j).to_int());
				aux = d1+d2;
				if (aux < 0)
					aux = 0;
				else if (aux>MAX_OUT_VALUE)
					aux = MAX_OUT_VALUE;
				res.range(S*(j+1)-1,S*j) = aux;

			 }
			out_stream.write(res);
		}
	}
}

/*
 * the kernel receives two images I1 and I2; output image Out=|I1-I2| (component by component)
 */
template <typename T, int S, int W>   // S is the size, in bits, of the pixel component. W is the width, in bit, of data type T
void diffImages(hls::stream<T>& inStream1, hls::stream<T>& inStream2, hls::stream<T>& outStream, unsigned int ImgSize, unsigned int NbImages)
{
	T img1, img2, outImg;
	unsigned char img1Val, img2Val, maskVal, outVal;

	for (unsigned int k=0; k<NbImages; k++)
	{

		for (int i = 0; i < (ImgSize*S/W); i++) {
	#pragma HLS pipeline
	#pragma HLS ALLOCATION instances=sub limit=PIXEL_PROCESSED_IN_PARALLEL

			img1 = inStream1.read();
			img2 = inStream2.read();

			for (int j=0; j<(W/S); j++)
			 {
		#pragma HLS unroll
				img1Val = (img1.range(S * (j + 1) - 1, S * j).to_int());
				img2Val = (img2.range(S * (j + 1) - 1, S * j).to_int());
				if (img1Val > img2Val)
					outVal = img1Val-img2Val;
				else
					outVal = img2Val-img1Val;
				outImg.range(S*(j+1)-1,S*j) = outVal;

			 }
			outStream.write(outImg);
		}
	}
}

/*
 * the kernel multiplies two images (expected S = 8 bit) and produces the output image which is twice larger (16 bits)
 * Tout must be two times larger than T (for example T is 128 bits and Tout must be 256 bits
 */
template <typename T, typename Tout, int S, int W>   // S is the size, in bits, of the pixel component. W is the width, in bit, of data type T
static void mulImages(hls::stream<T>& in_stream1,  hls::stream<T>& in_stream2,  hls::stream<Tout>& out_stream, unsigned int ImgSize, unsigned int NbImages)
{
	T tmp1, tmp2;
	Tout res;
	short int d1, d2, r, aux;
	for (unsigned int k=0; k<NbImages; k++)
	{
	mem_wr1:
		for (int i = 0; i < (ImgSize*S/W); i++) {
	#pragma HLS pipeline
	#pragma HLS ALLOCATION instances=add limit=PIXEL_PROCESSED_IN_PARALLEL
			tmp1 = in_stream1.read();
			tmp2 = in_stream2.read();

			for (int j=0; j<(W/S); j++)
			 {
		#pragma HLS unroll

				d1 = (tmp1.range(S * (j + 1) - 1, S * j).to_int());
				d2 = (tmp2.range(S * (j + 1) - 1, S * j).to_int());
				aux = d1*d2;
				if (aux < 0)
					aux = 0;
				else if (aux>(256*256-1))
					aux = (256*256-1);
				res.range(2*S*(j+1)-1,2*S*j) = aux;

			 }
			out_stream.write(res);
		}
	}
}

/*
 * given the parameter 0<=alpha<=255, computes Iout = (alpha*(I1) + (255 - alpha)*(I2))/256
 */
template <typename T, int S, int W>   // S is the size, in bits, of the pixel component. W is the width, in bit, of data type T
static void convexSumImages(hls::stream<T>& in_stream1,  hls::stream<T>& in_stream2,  hls::stream<T>& out_stream, unsigned char alpha, unsigned int ImgSize, unsigned int NbImages)
{
	T tmp1, tmp2, res;
	short int d1, d2, r, aux;
	for (unsigned int k=0; k<NbImages; k++)
	{
	mem_wr1:
		for (int i = 0; i < (ImgSize*S/W); i++) {
	#pragma HLS pipeline
	#pragma HLS ALLOCATION instances=add limit=PIXEL_PROCESSED_IN_PARALLEL
			tmp1 = in_stream1.read();
			tmp2 = in_stream2.read();

			for (int j=0; j<(W/S); j++)
			 {
		#pragma HLS unroll

				d1 = (tmp1.range(S * (j + 1) - 1, S * j).to_int());
				d2 = (tmp2.range(S * (j + 1) - 1, S * j).to_int());
				aux = (alpha*d1+(255-alpha)*d2)/256;
				if (aux < 0)
					aux = 0;
				else if (aux>MAX_OUT_VALUE)
					aux = MAX_OUT_VALUE;
				res.range(S*(j+1)-1,S*j) = aux;

			 }
			out_stream.write(res);
		}
	}
}

/*
 * given the parameter 0<=alpha<=255, computes Iout = (alpha*(I1) + (255 - alpha)*(I2))/256
 */
template <typename T, int S, int W>   // S is the size, in bits, of the pixel component. W is the width, in bit, of data type T
static void weightedSumImages(hls::stream<T>& in_stream1,  hls::stream<T>& in_stream2,  hls::stream<T>& out_stream, unsigned char w1, unsigned char w2, unsigned int ImgSize, unsigned int NbImages)
{
	T tmp1, tmp2, res;
	short int d1, d2, r, aux;
	for (unsigned int k=0; k<NbImages; k++)
	{
	mem_wr1:
		for (int i = 0; i < (ImgSize*S/W); i++) {
	#pragma HLS pipeline
	#pragma HLS ALLOCATION instances=add limit=PIXEL_PROCESSED_IN_PARALLEL
			tmp1 = in_stream1.read();
			tmp2 = in_stream2.read();

			for (int j=0; j<(W/S); j++)
			 {
		#pragma HLS unroll

				d1 = (tmp1.range(S * (j + 1) - 1, S * j).to_int());
				d2 = (tmp2.range(S * (j + 1) - 1, S * j).to_int());
				aux = (w1*d1+w2*d2)/256;
				if (aux < 0)
					aux = 0;
				else if (aux>MAX_OUT_VALUE)
					aux = MAX_OUT_VALUE;
				res.range(S*(j+1)-1,S*j) = aux;

			 }
			out_stream.write(res);
		}
	}
}

/*
 * NbColorBitsKept = 1, 2, 3, 4, 5, 6, 7, ...,
 * each pixel component is masked through a mask which has set only the NbColorBitsKept most significant bits
 */
template <typename T, int S, int W>   // S is the size, in bits, of the pixel component. W is the width, in bit, of data type T
void decreaseColorDepth(hls::stream<T>& inStream, hls::stream<T>& outStream, unsigned int ImgSize, unsigned short int NbColorBitsKept, unsigned int NbImages)
{
	T img, outImg;
	unsigned char imgVal, outVal;
	unsigned char mask = 0xFF;
	for (unsigned int k=0; k<NbImages; k++)
	{
		mask >>= (S-NbColorBitsKept);
		mask <<= (S-NbColorBitsKept);  // now mask has only the most significant NbColorBitsKept set
		for (int i = 0; i < (ImgSize*S/W); i++) {
	#pragma HLS pipeline
			img = inStream.read();

			for (int j=0; j<(W/S); j++)
			 {
		#pragma HLS unroll
				imgVal = (img.range(S * (j + 1) - 1, S * j).to_int());
				outVal = imgVal&mask;
				outImg.range(S*(j+1)-1,S*j) = outVal;
			 }
			outStream.write(outImg);
		}
	}
}

template <typename T, int S, int W>   // S is the size, in bits, of the pixel component. W is the width, in bit, of data type T
void convertToBinary(hls::stream<T>& inStream, hls::stream<T>& outStream, unsigned char Th, unsigned int ImgSize, unsigned int NbImages)
{
	T tmp, res;
	unsigned char tmpVal, outVal;

	for (unsigned int k=0; k<NbImages; k++)
	{
		for (int i = 0; i < (ImgSize*S/W); i++)
		{
	#pragma HLS pipeline
			tmp = inStream.read();

			for (int j=0; j<(W/S); j++)
			 {
		#pragma HLS unroll
				tmpVal = (tmp.range(S * (j + 1) - 1, S * j).to_int());
				if (tmpVal > Th)
					outVal = 255;
				else
					outVal = 0;
				res.range(S*(j+1)-1,S*j) = outVal;

			 }
			outStream.write(res);
		}
	}
}

/*
 * the kernel receives two images and a mask image as input; the output image is equal to inputImg1 where mask=0 and to inputImg2 where mask=1
 */
template <typename T, int S, int W>   // S is the size, in bits, of the pixel component. W is the width, in bit, of data type T
void mix2ImagesWithMask(hls::stream<T>& inStream1, hls::stream<T>& inStream2, hls::stream<T>& inStreamMask, hls::stream<T>& outStream, unsigned int ImgSize, unsigned int NbImages)
{
	T img1, img2, mask, outImg;
	unsigned char img1Val, img2Val, maskVal, outVal;
	for (unsigned int k=0; k<NbImages; k++)
	{
		for (int i = 0; i < (ImgSize*S/W); i++)
		{
	#pragma HLS pipeline
			img1 = inStream1.read();
			img2 = inStream2.read();
			mask = inStreamMask.read();

			for (int j=0; j<(W/S); j++)
			 {
		#pragma HLS unroll
				img1Val = (img1.range(S * (j + 1) - 1, S * j).to_int());
				img2Val = (img2.range(S * (j + 1) - 1, S * j).to_int());
				maskVal = (mask.range(S * (j + 1) - 1, S * j).to_int());
				if (maskVal == 0)
					outVal = img1Val;
				else
					outVal = img2Val;
				outImg.range(S*(j+1)-1,S*j) = outVal;

			 }
			outStream.write(outImg);
		}
	}
}



template <typename T, int S, int W>   // S is the size, in bits, of the pixel component. W is the width, in bit, of data type T
void doNegative(hls::stream<T>& inStream, hls::stream<T>& outStream, unsigned int ImgSize, unsigned int NbImages)
{
	T img, outImg;
	unsigned char imgVal, outVal;
	for (unsigned int k=0; k<NbImages; k++)
	{
		for (int i = 0; i < (ImgSize*S/W); i++) {
	#pragma HLS pipeline
	#pragma HLS ALLOCATION instances=sub limit=PIXEL_PROCESSED_IN_PARALLEL

			img = inStream.read();

			for (int j=0; j<(W/S); j++)
			 {
		#pragma HLS unroll
				imgVal = (img.range(S * (j + 1) - 1, S * j).to_int());
				outVal = (MAX_Y-1)-imgVal;
				outImg.range(S*(j+1)-1,S*j) = outVal;
			 }
			outStream.write(outImg);
		}
	}
}

// 0<= a <=2^SIZE-1
// for the time being, S=8 (dependence on MAX_OUT_VALUE)
template <typename T, int S, int W>   // S is the size, in bits, of the pixel component. W is the width, in bit, of data type T
void linearScaling(hls::stream<T>& inStream, hls::stream<T>& outStream, short int a, short int b, unsigned int ImgSize)
{
	T img, outImg;
	unsigned char imgVal, outVal;
	int intOutVal;
	for (int i = 0; i < (ImgSize*S/W); i++) {
#pragma HLS pipeline
		img = inStream.read();
#pragma HLS ALLOCATION instances=mul limit=PIXEL_PROCESSED_IN_PARALLEL
#pragma HLS ALLOCATION instances=add limit=PIXEL_PROCESSED_IN_PARALLEL


		for (int j=0; j<(W/S); j++)
	     {
	#pragma HLS unroll
			imgVal = (img.range(S * (j + 1) - 1, S * j).to_int());
			intOutVal = (imgVal*a)>>S+b;
			if (intOutVal>MAX_OUT_VALUE)
				outVal = MAX_OUT_VALUE;
			else if (intOutVal<0)
				outVal = 0;
			else
				outVal = intOutVal;
			outImg.range(S*(j+1)-1,S*j) = outVal;
	     }
		outStream.write(outImg);
	}
}
