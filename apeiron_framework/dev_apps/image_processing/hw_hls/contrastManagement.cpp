/*
 * receives an image from inStream and stores it in Mem. While storing the image, it computes Min and Max pixel values
 * After having received the whole image, sends out through outStream the image with the pixels scaled so that Min=0 and Max=255
 * New_pixelV = 255/(maxV-minV))*(currPixelV - minV))
 */
// for the time being, S=8 (pixel component uses 1 byte
template <typename T, int S, int W>   // S is the size, in bits, of the pixel component. W is the width, in bit, of data type T
void contrastEnhance(T* Mem, hls::stream<T>& inStream, hls::stream<T>& outStream, unsigned int ImgSize, unsigned int NbImages)
{
	T tmp, minValues, maxValues, outVal;
//	unsigned char minV,maxV,currV,tmpMin, tmpMax;
	ap_uint<S> minV,maxV,currV,tmpMin, tmpMax;
	unsigned short int value;
	for (unsigned int k=0; k<NbImages; k++)
	{
		for (int j=0; j<(W/S); j++)
		 {
		#pragma HLS unroll
			minValues.range(S*(j+1)-1,S*j) = 255;
			maxValues.range(S*(j+1)-1,S*j) = 0;
		 }

		for (int i = 0; i < (ImgSize*S/W); i++) {
	#pragma HLS pipeline
			tmp = inStream.read();  //read the image, search for the maximum and the minimum, and stores the image in an external memory bank
			Mem[i] = tmp;
			for (int j=0; j<(W/S); j++)
			 {
		#pragma HLS unroll

				currV = (tmp.range(S * (j + 1) - 1, S * j).to_int());
				tmpMin = (minValues.range(S * (j + 1) - 1, S * j).to_int());
				tmpMax = (maxValues.range(S * (j + 1) - 1, S * j).to_int());
				if (currV < tmpMin)
					tmpMin = currV;
				if (currV > tmpMax)
					tmpMax = currV;
				minValues.range(S*(j+1)-1,S*j) = tmpMin;
				maxValues.range(S*(j+1)-1,S*j) = tmpMax;
			 }
		}

		minV = 255;
		maxV = 0;
		for (int j=0; j<(W/S); j++)
		 {
		#pragma HLS unroll
			tmpMin = minValues.range(S*(j+1)-1,S*j);
			tmpMax = maxValues.range(S*(j+1)-1,S*j);
			if (minV > tmpMin)
				minV = tmpMin;
			if (maxV < tmpMax)
				maxV = tmpMax;
		  }  //minV and maxV are minimum and maximum values in the image;

		// read again image from memory and changes the pixel values to cover all the channel dynamics
		for (int i = 0; i < (ImgSize*S/W); i++) {
	#pragma HLS pipeline
	#pragma HLS ALLOCATION instances=mul limit=PIXEL_PROCESSED_IN_PARALLEL
	#pragma HLS ALLOCATION instances=sub limit=PIXEL_PROCESSED_IN_PARALLEL

			tmp = Mem[i];
			for (int j=0; j<(W/S); j++)
			 {
		#pragma HLS unroll

				currV = (tmp.range(S * (j + 1) - 1, S * j).to_int());
				if (maxV != minV)
				{
					unsigned short tmp1, tmp2, tmp3;
					tmp1 = (unsigned short)(maxV-minV);
					tmp2 = (unsigned short)(currV - minV);
					value = (((255*256)/tmp1)*tmp2)/256;
//					value = (((255*256)/(unsigned short)(maxV-minV))*(unsigned short)(currV - minV))/256;
					if (value > 255)
						value = 255;
				}
				else
					value = currV;
				outVal.range(S*(j+1)-1,S*j) = value;
			 }
			outStream.write(outVal);
		}
	}
}

unsigned int sum(unsigned int a0,
				 unsigned int a1,
				 unsigned int a2,
				 unsigned int a3,
				 unsigned int a4,
				 unsigned int a5,
				 unsigned int a6,
				 unsigned int a7,
				 unsigned int a8,
				 unsigned int a9,
				 unsigned int a10,
				 unsigned int a11,
				 unsigned int a12,
				 unsigned int a13,
				 unsigned int a14,
				 unsigned int a15)
{
#pragma HLS inline
	return (a0+a1+a2+a3+a4+a5+a6+a7+a8+a9+a10+a11+a12+a13+a14+a15);
}
/*
 * receives an image from inStream and stores it in Mem. While storing the image, it computes the histogram of the pixel valuesvalues
 * After having received the whole image, sends out through outStream the image with the pixels
 */
// for the time being, must be called with S=8 (due to the fized 256 levels in the histogram)
template <typename T, int S, int W>   // S is the size, in bits, of the pixel component. W is the width, in bit, of data type T
void histogramEqualization(T* Mem, hls::stream<T>& inStream, hls::stream<T>& outStream, unsigned int ImgSize, unsigned int NbImages)
{
	T tmp, outVal;
	T tmpMem;
	unsigned char minV,maxV,currV,tmpMin, tmpMax;
	unsigned short int value;
	int i;
	unsigned int histogram[W/S][256];  //create PIXEL_IN_INPUT_WORD replicas of an array of 256 integers
	#pragma HLS array_partition histogram dim=1
	unsigned int h[W/S][256];  //create PIXEL_IN_INPUT_WORD replicas of an array of 256 integers
	#pragma HLS array_partition h dim=1
	unsigned int cdf[256];  //create PIXEL_IN_INPUT_WORD replicas of an array of 256 integers
	unsigned int equalizedValue;
	unsigned char equalized[W/S][256];
	#pragma HLS array_partition equalized dim=1

	for (unsigned int k=0; k<NbImages; k++)
	{
		for (int i=0; i<256;i++)
			for (int j=0; j<(W/S); j++)
			{
			#pragma HLS unroll
				h[j][i] = 0;
				cdf[i] = 0;
			}

		tmp = inStream.read();  //read the image, search for the maximum and the minimum, and stores the image in an external memory bank
		for (i = 1; i < (ImgSize*S/W); i++) {
		#pragma HLS pipeline
				Mem[i-1] = tmp;
				for (int j=0; j<(W/S); j++)
				 {
					#pragma HLS unroll
					currV = (tmp.range(S * (j + 1) - 1, S * j).to_int());
					h[j][currV]++;
				 }
				tmp = inStream.read();  //read the image, search for the maximum and the minimum, and stores the image in an external memory bank
		}
		Mem[i-1] = tmp;
		for (int j=0; j<(W/S); j++)
		 {
			#pragma HLS unroll
			currV = (tmp.range(S * (j + 1) - 1, S * j).to_int());
			h[j][currV]++;
		 }

		// merge the histograms together
		for (int i=0; i<256;i++)
		{
			unsigned int currValue;
			currValue = sum(h[0][i],h[1][i],h[2][i],h[3][i],h[4][i],h[5][i],h[6][i],h[7][i],h[8][i],h[9][i],h[10][i],h[11][i],h[12][i],h[13][i],h[14][i],h[15][i]);
			for (int j=0; j<(W/S); j++)
			 {
				#pragma HLS unroll
				histogram[j][i] = currValue;
			 }
		}

		//compute cdf and the equalization array
		cdf[0] = histogram[0][0];
		for (i = 1; i < 256; i++) {
			cdf[i] = cdf[i-1] + histogram[0][i];
			equalizedValue = (unsigned int)(255 * cdf[i] / (ImgSize) );
			if (equalizedValue > 255)
				equalizedValue = 255;

			 if (equalizedValue < 0)
				 equalizedValue = 0;
			for (int j=0; j<(W/S); j++)
			{
				#pragma HLS unroll
				equalized[j][i] = equalizedValue;
			}
		}


	//         for (i = 0; i < image_size_bytes; i++) {
	//             outimage[i] = equalized[image[i]];
	//         }
		// image has been completely read from input stream and stored in Mem. Find min a max among the PIXEL_IN_INPUT_WORD min and max values
		// read again image from memory and changes the pixel values to cover all the channel dynamics
		tmpMem = Mem[0];
		for (int i = 1; i < (ImgSize*(S/W)); i++) {
	#pragma HLS pipeline
			for (int j=0; j<(W/S); j++)
			 {
			#pragma HLS unroll
				outVal.range(S*(j+1)-1,S*j) = equalized[j][tmpMem.range(S*(j+1)-1,S*j)];
			 }
			outStream.write(outVal);
			tmpMem = Mem[i];
		}

		for (int j=0; j<(W/S); j++)
		{
		#pragma HLS unroll
			outVal.range(S*(j+1)-1,S*j) = equalized[j][tmpMem.range(S*(j+1)-1,S*j)];
		 }
		outStream.write(outVal);
	}
}
