// size - for the time being - must be S=8 (8 bits components)
template <typename T, int S, int W>   // S is the size, in bits, of the pixel component. W is the width, in bit, of data type T
static void RGB2Y(hls::stream<T>& sR, hls::stream<T>& sG, hls::stream<T>& sB, hls::stream<T>& outStreamY, unsigned int ImgSize, unsigned int NbImages)
{
	T tmpR, tmpG, tmpB, resY;
	unsigned char r, g, b, Y;
	unsigned char cY;
	for (unsigned int k=0; k<NbImages; k++)
	{
	mem_wr1:
		for (int i = 0; i < (ImgSize*S/W); i++) {
	#pragma HLS pipeline
			tmpR = sR.read();
			tmpG = sG.read();
			tmpB = sB.read();

			for (int j=0; j<(W/S); j++)
			 {
		#pragma HLS unroll

				r = (tmpR.range(S * (j + 1) - 1, S * j).to_int());
				g = (tmpG.range(S * (j + 1) - 1, S * j).to_int());
				b = (tmpB.range(S * (j + 1) - 1, S * j).to_int());
				cY = (871*r+2929*g+296*b)>>12;
				resY.range(S*(j+1)-1,S*j) = cY;

			 }
			outStreamY.write(resY);
		}
	}
}

// size - for the time being - must be S=8 (8 bits components)
template <typename T, int S, int W>   // S is the size, in bits, of the pixel component. W is the width, in bit, of data type T
static void RGB2YUV(hls::stream<T>& sR, hls::stream<T>& sG, hls::stream<T>& sB, hls::stream<T>& outStreamY, hls::stream<T>& outStreamU, hls::stream<T>& outStreamV, unsigned int ImgSize, unsigned int NbImages)
{
	T tmpR, tmpG, tmpB, resY, resU, resV;
	unsigned char r, g, b, Y;
	ap_int<S> intY, intU, intV;
	for (unsigned int k=0; k<NbImages; k++)
	{
	mem_wr1:
		for (int i = 0; i < (ImgSize*S/W); i++) {
	#pragma HLS pipeline
			tmpR = sR.read();
			tmpG = sG.read();
			tmpB = sB.read();

			for (int j=0; j<(W/S); j++)
			 {
		#pragma HLS unroll

				r = (tmpR.range(S * (j + 1) - 1, S * j).to_int());
				g = (tmpG.range(S * (j + 1) - 1, S * j).to_int());
				b = (tmpB.range(S * (j + 1) - 1, S * j).to_int());
				intY = (871*r+2929*g+296*b)>>12;
				intU = (-409*r-1377*g+1786*b)>>12;
				intV = (2519*r-2288*g-231*b)>>12;
				resY.range(S*(j+1)-1,S*j) = intY;
				resU.range(S*(j+1)-1,S*j) = intU;
				resV.range(S*(j+1)-1,S*j) = intV;
			 }
			outStreamY.write(resY);
			outStreamU.write(resU);
			outStreamV.write(resV);
		}
	}
}

// size - for the time being - must be S=8 (8 bits components)
template <typename T, int S, int W>   // S is the size, in bits, of the pixel component. W is the width, in bit, of data type T
static void YUV2RGB(hls::stream<T>& sY, hls::stream<T>& sU, hls::stream<T>& sV, hls::stream<T>& sR, hls::stream<T>& sG, hls::stream<T>& sB, unsigned int ImgSize, unsigned int NbImages)
{
	T tmpY, tmpU, tmpV, resR, resG, resB;
	unsigned char y;
	ap_int<S> u, v;
	short int intR, intG, intB;
	unsigned char R,G,B;
	for (unsigned int k=0; k<NbImages; k++)
	{
	mem_wr1:
		for (int i = 0; i < (ImgSize*S/W); i++) {
	#pragma HLS pipeline
			tmpY = sY.read();
			tmpU = sU.read();
			tmpV = sV.read();

			for (int j=0; j<(W/S); j++)
			 {
		#pragma HLS unroll

				y = (tmpY.range(S * (j + 1) - 1, S * j).to_int());
				u = (tmpU.range(S * (j + 1) - 1, S * j).to_int());
				v = (tmpV.range(S * (j + 1) - 1, S * j).to_int());
				intR = (4096*y + 5244*v)>>12;
				intG = (4096*y-880*u-1559*v)>>12;
				intB = (4096*y+8716*u)>>12;
				if (intR>MAX_OUT_VALUE)
					R=MAX_OUT_VALUE;
				else if (intR<0)
					R=0;
				else
					R = intR;
				if (intG>MAX_OUT_VALUE)
					G=MAX_OUT_VALUE;
				else if (intG<0)
					G=0;
				else
					G = intG;
				if (intB>MAX_OUT_VALUE)
					B=MAX_OUT_VALUE;
				else if (intB<0)
					B=0;
				else
					B = intB;
				resR.range(S*(j+1)-1,S*j) = R;
				resG.range(S*(j+1)-1,S*j) = G;
				resB.range(S*(j+1)-1,S*j) = B;
			 }
			sR.write(resR);
			sG.write(resG);
			sB.write(resB);
		}
	}
}

