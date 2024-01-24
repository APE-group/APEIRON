template <typename T, int S, int W>   // S is the size, in bits, of the pixel component. W is the width, in bit, of data type T
void extractRectangularImage(hls::stream<T>& inStream, hls::stream<T>& outStream,
		                     short int row, short int col,
							 unsigned short int length_x, unsigned short int length_y,
							 unsigned short int ImgRows, unsigned short int ImgCols,
							 unsigned int NbImages)
{
	int count, i, j;
	T tmp;

	for (count=0; count<NbImages; count++)
	{
		for (i=0; i<ImgRows; i++)
		{
			for (j=0; j<ImgCols; j+= (W/S))
			{
#pragma HLS pipeline
				tmp = inStream.read();
				if ((i >= row)&&(i < row+length_y)&&(j >= col)&&(j < col+length_x))
					outStream.write(tmp);
			}
		}

	}
}

template <typename T, int S, int W>   // S is the size, in bits, of the pixel component. W is the width, in bit, of data type T
void insertRectangularImage(hls::stream<T>& inStream1, hls::stream<T>& inStream2, hls::stream<T>& outStream,
		                     short int row, short int col,
							 unsigned short int length_x, unsigned short int length_y,
							 unsigned short int ImgRows, unsigned short int ImgCols,
							 unsigned int NbImages)
{
	int count, i, j;
	T tmp1, tmp2;

	for (count=0; count<NbImages; count++)
	{
		for (i=0; i<ImgRows; i++)
		{
			for (j=0; j<ImgCols; j+= (W/S))
			{
#pragma HLS pipeline
				tmp1 = inStream1.read();
				if ((i >= row)&&(i < row+length_y)&&(j >= col)&&(j < col+length_x))
				{
					tmp2 = inStream2.read();
					outStream.write(tmp2);
				}
				else
					outStream.write(tmp1);
			}
		}

	}
}

template <typename T, int S, int W>   // S is the size, in bits, of the pixel component. W is the width, in bit, of data type T
void extractRGBRectangularImage(hls::stream<T>& inStream, hls::stream<T>& outStream,
		                     short int row, short int col,
							 unsigned short int length_x, unsigned short int length_y,
							 unsigned short int ImgRows, unsigned short int ImgCols,
							 unsigned int NbImages)
{
	int count, i, j;
	T tmp1, tmp2, tmp3;

	for (count=0; count<NbImages; count++)
	{
		for (i=0; i<ImgRows; i++)
		{
			for (j=0; j<ImgCols; j+= (W/S))
			{
#pragma HLS pipeline
				tmp1 = inStream.read();
				tmp2 = inStream.read();
				tmp3 = inStream.read();
				if ((i >= row)&&(i < row+length_y)&&(j >= col)&&(j < col+length_x))
				{
					outStream.write(tmp1);
					outStream.write(tmp2);
					outStream.write(tmp3);
				}
			}
		}

	}
}

template <typename T, int S, int W>   // S is the size, in bits, of the pixel component. W is the width, in bit, of data type T
void insertRGBRectangularImage(hls::stream<T>& inStream1, hls::stream<T>& inStream2, hls::stream<T>& outStream,
		                     short int row, short int col,
							 unsigned short int length_x, unsigned short int length_y,
							 unsigned short int ImgRows, unsigned short int ImgCols,
							 unsigned int NbImages)
{
	int count, i, j;
	T tmpA1, tmpA2, tmpA3, tmpB;

	for (count=0; count<NbImages; count++)
	{
		for (i=0; i<ImgRows; i++)
		{
			for (j=0; j<ImgCols; j+= (W/S))
			{
#pragma HLS pipeline
				tmpA1 = inStream1.read();
				tmpA2 = inStream1.read();
				tmpA3 = inStream1.read();
				if ((i >= row)&&(i < row+length_y)&&(j >= col)&&(j < col+length_x))
				{
					tmpB = inStream2.read();
					outStream.write(tmpB);
					tmpB = inStream2.read();
					outStream.write(tmpB);
					tmpB = inStream2.read();
					outStream.write(tmpB);
				}
				else
				{
					outStream.write(tmpA1);
					outStream.write(tmpA2);
					outStream.write(tmpA3);
				}
			}
		}
	}
}

/*
 * we assume the top left vertex to have coordinate (0,0) and the bottom right pixel (ImgRows-1, ImgCols-1)
 */

template <typename T, int S, int W>   // S is the size, in bits, of the pixel component. W is the width, in bit, of data type T
void drawCircularMask(hls::stream<T>& outStream,
		                     unsigned short int radius,
							 unsigned short int x0, unsigned short int y0,
							 unsigned short int ImgRows, unsigned short int ImgCols,
							 unsigned int NbImages)
{
	int count, i, j;
	T outImg;

	for (count=0; count<NbImages; count++)
	{
		for (i=0; i<ImgRows; i++)
		{
			for (j=0; j<ImgCols; j+= (W/S))
			{
#pragma HLS pipeline
				for (int k=0; k<(W/S); k++)
				 {
			#pragma HLS unroll
					if ((j*j+k*k+x0*x0-j*2*x0-k*2*x0+j*2*k+i*i+y0*y0-i*2*y0-radius*radius)<=0)
					//(the pixel has coordinates (j+k, i)) and belongs to the circle
						outImg.range(S*(k+1)-1,S*k) = 255;
					else
						outImg.range(S*(k+1)-1,S*k) = 0;
				 }
				outStream.write(outImg);
			}
		}
	}
}
/*
 * generate an image (single component) with all the pixels set to val
 */
template <typename T, int S, int W>   // S is the size, in bits, of the pixel component. W is the width, in bit, of data type T
void generateConstantGreyImage(char val, hls::stream<T>& outStream, unsigned int ImgSize,  unsigned int NbImages)
{
	T res;
	for (int k=0; k<NbImages; k++)
	{
		for (int j=0; j<(W/S); j++)
		 {
	#pragma HLS unroll
			res.range(S*(j+1)-1,S*j) = val;
		 }

		for (int i = 0; i < (ImgSize*S/W); i++) {
	#pragma HLS pipeline
			outStream.write(res);
		}
	}
}


