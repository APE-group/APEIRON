/*
 * ImgSize is given in pixels
 * Tailored for reading 16 byte/cycle
 */
// spliRGBChannels for the case N = 128 - 16 Byte
static void splitRGBChannels(hls::stream<dt16>& in_stream1,  hls::stream<dt16>& sR,  hls::stream<dt16>& sG,  hls::stream<dt16>& sB, unsigned int ImgSize, unsigned int NbImages)
{
	dt16 tmp1, tmp2, tmp3, resR, resG, resB;
	unsigned char d1, d2, r, g, b, aux;
	for (unsigned int k=0; k<NbImages; k++)
	{
	mem_wr1:
		for (int i = 0; i < (ImgSize*3*SIZE/128); i+=3) {  // for each pixel, read the three components
	#pragma HLS pipeline
			tmp1 = in_stream1.read();
			tmp2 = in_stream1.read();
			tmp3 = in_stream1.read();
			for (int j=0; j<16-2; j+=3)  //PIXEL_IN_INPUT_WORD==16
			 {
		#pragma HLS unroll
				r = (tmp1.range(SIZE * (j + 1) - 1, SIZE * j).to_int());
				g = (tmp1.range(SIZE * (j + 2) - 1, SIZE * (j+1)).to_int());
				b = (tmp1.range(SIZE * (j + 3) - 1, SIZE * (j+2)).to_int());
				resR.range(SIZE*(j/3+1)-1,SIZE*j/3) = r;
				resG.range(SIZE*(j/3+1)-1,SIZE*j/3) = g;
				resB.range(SIZE*(j/3+1)-1,SIZE*j/3) = b;
			 }
			// after previous for loop 5 pixels have been read => 5 r / g / b components have been read . Still 1 component (r) to be read to consume the 32 values available
			r = (tmp1.range(128 - 1, (128-SIZE)).to_int());


			for (int j=0; j<16-2; j+=3)
			 {
		#pragma HLS unroll
				g = (tmp2.range(SIZE * (j + 1) - 1, SIZE * (j)).to_int());
				b = (tmp2.range(SIZE * (j + 2) - 1, SIZE * (j+1)).to_int());
				resR.range(SIZE*(5+j/3+1)-1,SIZE*(5+j/3)) = r;
				resG.range(SIZE*(5+j/3+1)-1,SIZE*(5+j/3)) = g;
				resB.range(SIZE*(5+j/3+1)-1,SIZE*(5+j/3)) = b;
				r = (tmp2.range(SIZE * (j + 3) - 1, SIZE * (j+2)).to_int());
			 }
			// this time the component still to be read is g
			g = (tmp2.range(128 - 1, (128-SIZE)).to_int());

			for (int j=0; j<16-2; j+=3)
			 {
		#pragma HLS unroll
				b = (tmp3.range(SIZE * (j + 1) - 1, SIZE * j).to_int());
				resR.range(SIZE*(10+j/3+1)-1,SIZE*(10+j/3)) = r;
				resG.range(SIZE*(10+j/3+1)-1,SIZE*(10+j/3)) = g;
				resB.range(SIZE*(10+j/3+1)-1,SIZE*(10+j/3)) = b;
				r = (tmp3.range(SIZE * (j + 2) - 1, SIZE * (j+1)).to_int());
				g = (tmp3.range(SIZE * (j + 3) - 1, SIZE * (j+2)).to_int());

			 }
			b = (tmp3.range(128 - 1, (128-SIZE)).to_int());
			resR.range(128 - 1, (128-SIZE)) = r;
			resG.range(128 - 1, (128-SIZE)) = g;
			resB.range(128 - 1, (128-SIZE)) = b;

			sR.write(resR);
			sG.write(resG);
			sB.write(resB);
		}
	}
}

/*
 * ImgSize is given in pixels
 * Tailored for reading 32 byte/cycle
 */
// splitRGBChannels - N=256 - 32 Byte
static void splitRGBChannels(hls::stream<dt32>& in_stream1,  hls::stream<dt32>& sR,  hls::stream<dt32>& sG,  hls::stream<dt32>& sB, unsigned int ImgSize, unsigned int NbImages)
{
	dt32 tmp1, tmp2, tmp3, resR, resG, resB;
	unsigned char d1, d2, r, g, b, aux;
	for (unsigned int k=0; k<NbImages; k++)
	{
		// code tailored for case n=256 => 32 components/input word => 10 pixels + 2 more components (r and g)
	mem_wr1:
		for (int i = 0; i < (ImgSize*3*SIZE/256); i+=3) {  // for each pixel, read the three components
	#pragma HLS pipeline
			tmp1 = in_stream1.read();
			tmp2 = in_stream1.read();
			tmp3 = in_stream1.read();
			for (int j=0; j<32-2; j+=3)
			 {
		#pragma HLS unroll
				r = (tmp1.range(SIZE * (j + 1) - 1, SIZE * j).to_int());
				g = (tmp1.range(SIZE * (j + 2) - 1, SIZE * (j+1)).to_int());
				b = (tmp1.range(SIZE * (j + 3) - 1, SIZE * (j+2)).to_int());
				resR.range(SIZE*(j/3+1)-1,SIZE*j/3) = r;
				resG.range(SIZE*(j/3+1)-1,SIZE*j/3) = g;
				resB.range(SIZE*(j/3+1)-1,SIZE*j/3) = b;
			 }
			// after previous for loop 10 pixels have been read => 10 r / g / b components have been read . Still 2 components (r and g) to be read to consume the 32 values available
			r = (tmp1.range((256-SIZE) - 1, (256-2*SIZE)).to_int());
			g = (tmp1.range(256 - 1, (256-SIZE)).to_int());


			for (int j=0; j<32-2; j+=3)
			 {
		#pragma HLS unroll
				b = (tmp2.range(SIZE * (j + 1) - 1, SIZE * (j)).to_int());

				resR.range(SIZE*(10+j/3+1)-1,SIZE*(10+j/3)) = r;
				resG.range(SIZE*(10+j/3+1)-1,SIZE*(10+j/3)) = g;
				resB.range(SIZE*(10+j/3+1)-1,SIZE*(10+j/3)) = b;
				r = (tmp2.range(SIZE * (j + 2) - 1, SIZE * (j+1)).to_int());
				g = (tmp2.range(SIZE * (j + 3) - 1, SIZE * (j+2)).to_int());

			 }
			// this time the component still to be read are b and r
			b = (tmp2.range((256-SIZE) - 1, (256-2*SIZE)).to_int());
			resR.range(SIZE*(21)-1,SIZE*(20)) = r;
			resG.range(SIZE*(21)-1,SIZE*(20)) = g;
			resB.range(SIZE*(21)-1,SIZE*(20)) = b;

			r = (tmp2.range(256 - 1, (256-SIZE)).to_int());

			for (int j=0; j<32-2; j+=3)
			 {
		#pragma HLS unroll
				g = (tmp3.range(SIZE * (j + 1) - 1, SIZE * j).to_int());
				b = (tmp3.range(SIZE * (j + 2) - 1, SIZE * (j + 1)).to_int());
				resR.range(SIZE*(21+j/3+1)-1,SIZE*(21+j/3)) = r;
				resG.range(SIZE*(21+j/3+1)-1,SIZE*(21+j/3)) = g;
				resB.range(SIZE*(21+j/3+1)-1,SIZE*(21+j/3)) = b;
				r = (tmp3.range(SIZE * (j + 3) - 1, SIZE * (j+2)).to_int());

			 }
			g = (tmp3.range((256-SIZE) - 1,(256-2*SIZE)).to_int());
			b = (tmp3.range(256 - 1, (256-SIZE)).to_int());
			resR.range(256-1,(256-SIZE)) = r;
			resG.range(256-1,(256-SIZE)) = g;
			resB.range(256-1,(256-SIZE)) = b;

			sR.write(resR);
			sG.write(resG);
			sB.write(resB);
		}
	}
}

/*
 * ImgSize is given in pixels
 * Tailored for reading 64 byte/cycle
 */
// splitRGBChannels - N = 512, 64 Byte
static void splitRGBChannels(hls::stream<dt64>& in_stream1,  hls::stream<dt64>& sR,  hls::stream<dt64>& sG,  hls::stream<dt64>& sB, unsigned int ImgSize, unsigned int NbImages)
{
	dt64 tmp1, tmp2, tmp3, resR, resG, resB;
	unsigned char d1, d2, r, g, b, aux;
	for (unsigned int k=0; k<NbImages; k++)
	{
		// code tailored for case n=512 => 64 components/input word => 21 pixels + 1 residual component
	mem_wr1:
		for (int i = 0; i < (ImgSize*3*SIZE/512); i+=3) {  // for each pixel, read the three components
	#pragma HLS pipeline
			tmp1 = in_stream1.read();
			tmp2 = in_stream1.read();
			tmp3 = in_stream1.read();
			for (int j=0; j<64-2; j+=3)
			 {
		#pragma HLS unroll
				r = (tmp1.range(SIZE * (j + 1) - 1, SIZE * j).to_int());
				g = (tmp1.range(SIZE * (j + 2) - 1, SIZE * (j+1)).to_int());
				b = (tmp1.range(SIZE * (j + 3) - 1, SIZE * (j+2)).to_int());
				resR.range(SIZE*(j/3+1)-1,SIZE*j/3) = r;
				resG.range(SIZE*(j/3+1)-1,SIZE*j/3) = g;
				resB.range(SIZE*(j/3+1)-1,SIZE*j/3) = b;
			 }
			// after previous for loop 64 pixels have been read => 21 r / g / b components have been read. Still 1 components (r) to be read to consume the 64 values available
			r = (tmp1.range(512 - 1, 512-SIZE).to_int());


			for (int j=0; j<64-2; j+=3)
			 {
		#pragma HLS unroll
				g = (tmp2.range(SIZE * (j + 1) - 1, SIZE * j).to_int());
				b = (tmp2.range(SIZE * (j + 2) - 1, SIZE * (j+1)).to_int());

				resR.range(SIZE*(21+j/3+1)-1,SIZE*(21+j/3)) = r;
				resG.range(SIZE*(21+j/3+1)-1,SIZE*(21+j/3)) = g;
				resB.range(SIZE*(21+j/3+1)-1,SIZE*(21+j/3)) = b;
				r = (tmp2.range(SIZE * (j + 3) - 1, SIZE * (j+2)).to_int());

			 }
			// this time the component still to be read is gr
			g = (tmp2.range(512 - 1, 512-SIZE).to_int());

			for (int j=0; j<64-2; j+=3)
			 {
		#pragma HLS unroll
				b = (tmp3.range(SIZE * (j + 1) - 1, SIZE * j).to_int());
				resR.range(SIZE*(42+j/3+1)-1,SIZE*(42+j/3)) = r;
				resG.range(SIZE*(42+j/3+1)-1,SIZE*(42+j/3)) = g;
				resB.range(SIZE*(42+j/3+1)-1,SIZE*(42+j/3)) = b;
				r = (tmp3.range(SIZE * (j + 2) - 1, SIZE * (j + 1)).to_int());
				g = (tmp3.range(SIZE * (j + 3) - 1, SIZE * (j+2)).to_int());

			 }
			b = (tmp3.range(512 - 1, 512-SIZE).to_int());
			resR.range(512-1,512-SIZE) = r;
			resG.range(512-1,512-SIZE) = g;
			resB.range(512-1,512-SIZE) = b;

			sR.write(resR);
			sG.write(resG);
			sB.write(resB);
		}
	}
}

//template <int W>
//static void splitRGBChannels(hls::stream<dt>& in_stream,  hls::stream<dt>& sR,  hls::stream<dt>& sG,  hls::stream<dt>& sB, unsigned int ImgSize, unsigned int NbImages)
//{
//#if W==512
//	splitRGBChannels_64B(in_stream, sR, sG, sB, ImgSize, NbImages);
//#elif W==256
//	splitRGBChannels_32B(in_stream, sR, sG, sB, ImgSize, NbImages);
//#elif W==128
//	splitRGBChannels_16B(in_stream, sR, sG, sB, ImgSize, NbImages);
//#endif
//}

/*
 * ImgSize is given in pixels
 */
// mergeRGBChannels for the case N = 128 - 16 Byte
static void mergeRGBChannels(hls::stream<dt16>& sR,  hls::stream<dt16>& sG,  hls::stream<dt16>& sB, hls::stream<dt16>& out_stream,  unsigned int ImgSize, unsigned int NbImages)
{
	dt16 res1, res2, res3, R, G, B;
	unsigned char d1, d2, rcomp, gcomp, bcomp, aux;
	for (unsigned int k=0; k<NbImages; k++)
	{
	mem_wr1:
		for (int i = 0; i < (ImgSize*3*SIZE/128); i+=3) {  // for each pixel, read the three components
	#pragma HLS pipeline
			R = sR.read();
			G = sG.read();
			B = sB.read();
			for (int j=0; j<16-2; j+=3)
			 {
		#pragma HLS unroll
				rcomp = (R.range(SIZE*(j/3+1)-1,SIZE*j/3).to_int());
				gcomp = (G.range(SIZE*(j/3+1)-1,SIZE*j/3).to_int());
				bcomp = (B.range(SIZE*(j/3+1)-1,SIZE*j/3).to_int());
				res1.range(SIZE * (j + 1) - 1, SIZE * j) = rcomp;
				res1.range(SIZE * (j + 2) - 1, SIZE * (j+1)) = gcomp;
				res1.range(SIZE * (j + 3) - 1, SIZE * (j+2)) = bcomp;
			 }
			// after previous for loop 15 pixel components have been read/written => 5 r / g / b components have been read/written. Still 21 component (r) to be read/written to finish consuming the 16 values available
			rcomp = (R.range(SIZE*6-1,SIZE*5).to_int());
			gcomp = (G.range(SIZE*6-1,SIZE*5).to_int());
			res1.range(128 - 1, (128-SIZE)) = rcomp;
			out_stream.write(res1);

			bcomp = (B.range(SIZE*6-1,SIZE*5).to_int());

			for (int j=0; j<16-2; j+=3)
			 {
		#pragma HLS unroll
				rcomp = (R.range(SIZE*(6+(j/3+1))-1,SIZE*(6+j/3)).to_int());
				res2.range(SIZE * (j + 1) - 1, SIZE * j) = gcomp;
				res2.range(SIZE * (j + 2) - 1, SIZE * (j+1)) = bcomp;
				res2.range(SIZE * (j + 3) - 1, SIZE * (j+2)) = rcomp;
				gcomp = (G.range(SIZE*(6+(j/3+1))-1,SIZE*(6+j/3)).to_int());
				bcomp = (B.range(SIZE*(6+(j/3+1))-1,SIZE*(6+j/3)).to_int());

			 }
			// this time the component still to be read are r and g
			res2.range(128 - 1, (128-SIZE)) = gcomp;

			out_stream.write(res2);

			for (int j=0; j<16-2; j+=3)
			 {
		#pragma HLS unroll
				rcomp = (R.range(SIZE*(11+(j/3+1))-1,SIZE*(11+j/3)).to_int());
				gcomp = (G.range(SIZE*(11+(j/3+1))-1,SIZE*(11+j/3)).to_int());

				res3.range(SIZE * (j + 1) - 1, SIZE * j) = bcomp;
				res3.range(SIZE * (j + 2) - 1, SIZE * (j+1)) = rcomp;
				res3.range(SIZE * (j + 3) - 1, SIZE * (j+2)) = gcomp;
				bcomp = (B.range(SIZE*(11+(j/3+1))-1,SIZE*(11+j/3)).to_int());
			 }
			res3.range(128 - 1, (128-SIZE)) = bcomp;
			out_stream.write(res3);
		}
	}
}


///****APE******************//
/*
   * ImgSize is given in pixels
   */
  // mergeRGBChannels for the case N = 128 - 16 Byte
/*static void mergeRGBChannels(hls::stream<dt16>& sR,  hls::stream<dt16>& sG,  hls::stream<dt16>& sB, hls::stream<dt16>& out_stream,  unsigned int ImgSize, unsigned int NbImages)
  {
    dt16 res1,R, G, B;
    unsigned char d1, d2, rcomp, gcomp, bcomp, aux;
    for (unsigned int k=0; k<NbImages; k++)
      {
      mem_wr1:
        for (int i = 0; i < (ImgSize*(SIZE/128)); i++) {  // for each pixel, read the th  ree components
#pragma HLS pipeline
          //dataflow?
          R = sR.read();
          G = sG.read();
          B = sB.read();
          for (int j=0; j<16; j++)
            {
#pragma HLS unroll
              rcomp = (R.range(SIZE*(j/3+1)-1,SIZE*j/3).to_int());
              gcomp = 0; //(G.range(SIZE*(j/3+1)-1,SIZE*j/3).to_int());
              bcomp = 0; //(B.range(SIZE*(j/3+1)-1,SIZE*j/3).to_int());
              res1.range(SIZE * (j + 1) - 1, SIZE * j) = rcomp;
              res1.range(SIZE * (j + 2) - 1, SIZE * (j+1)) = gcomp;
              res1.range(SIZE * (j + 3) - 1, SIZE * (j+2)) = bcomp;
            }
  
          out_stream.write(res1);
  
        }
      }
  }
*/


///////////////////


/*
 * ImgSize is given in pixels
 */
// mergeRGBChannels for the case N = 256 - 32 Byte
static void mergeRGBChannels(hls::stream<dt32>& sR,  hls::stream<dt32>& sG,  hls::stream<dt32>& sB, hls::stream<dt32>& out_stream,  unsigned int ImgSize, unsigned int NbImages)
{
	dt32 res1, res2, res3, R, G, B;
	unsigned char d1, d2, rcomp, gcomp, bcomp, aux;
	for (unsigned int k=0; k<NbImages; k++)
	{
	mem_wr1:
		for (int i = 0; i < (ImgSize*3*SIZE/256); i+=3) {  // for each pixel, read the three components
	#pragma HLS pipeline
			R = sR.read();
			G = sG.read();
			B = sB.read();
			for (int j=0; j<32-2; j+=3)
			 {
		#pragma HLS unroll
				rcomp = (R.range(SIZE*(j/3+1)-1,SIZE*j/3).to_int());
				gcomp = (G.range(SIZE*(j/3+1)-1,SIZE*j/3).to_int());
				bcomp = (B.range(SIZE*(j/3+1)-1,SIZE*j/3).to_int());
				res1.range(SIZE * (j + 1) - 1, SIZE * j) = rcomp;
				res1.range(SIZE * (j + 2) - 1, SIZE * (j+1)) = gcomp;
				res1.range(SIZE * (j + 3) - 1, SIZE * (j+2)) = bcomp;
			 }
			// after previous for loop 30 pixel components have been read/written => 10 r / g / b components have been read/written. Still 2 components (r and g) to be read/written to finish consuming the 32 values available
			rcomp = (R.range(SIZE*11-1,SIZE*10).to_int());
			gcomp = (G.range(SIZE*11-1,SIZE*10).to_int());
			res1.range((256-SIZE) - 1, (256-2*SIZE)) = rcomp;
			res1.range(256 - 1, (256-SIZE)) = gcomp;
			out_stream.write(res1);

			bcomp = (B.range(SIZE*11-1,SIZE*10).to_int());

			for (int j=0; j<32-2; j+=3)
			 {
		#pragma HLS unroll
				rcomp = (R.range(SIZE*(11+(j/3+1))-1,SIZE*(11+j/3)).to_int());
				gcomp = (G.range(SIZE*(11+(j/3+1))-1,SIZE*(11+j/3)).to_int());
				res2.range(SIZE * (j + 1) - 1, SIZE * j) = bcomp;
				res2.range(SIZE * (j + 2) - 1, SIZE * (j+1)) = rcomp;
				res2.range(SIZE * (j + 3) - 1, SIZE * (j+2)) = gcomp;
				bcomp = (B.range(SIZE*(11+(j/3+1))-1,SIZE*(11+j/3)).to_int());

			 }
			// this time the component still to be read are r and g
			rcomp = (R.range(SIZE*22-1,SIZE*21).to_int());
			gcomp = (G.range(SIZE*22-1,SIZE*21).to_int());
			bcomp = (B.range(SIZE*22-1,SIZE*21).to_int());
			res2.range((256-SIZE) - 1, (256-SIZE*2)) = bcomp;
			res2.range(256 - 1, (256-SIZE)) = rcomp;

			out_stream.write(res2);

			for (int j=0; j<32-2; j+=3)
			 {
		#pragma HLS unroll
				rcomp = (R.range(SIZE*(22+(j/3+1))-1,SIZE*(22+j/3)).to_int());
				res3.range(SIZE * (j + 1) - 1, SIZE * j) = gcomp;
				res3.range(SIZE * (j + 2) - 1, SIZE * (j+1)) = bcomp;
				res3.range(SIZE * (j + 3) - 1, SIZE * (j+2)) = rcomp;
				bcomp = (B.range(SIZE*(22+(j/3+1))-1,SIZE*(22+j/3)).to_int());
				gcomp = (G.range(SIZE*(22+(j/3+1))-1,SIZE*(22+j/3)).to_int());
			 }
			res3.range((256-SIZE) - 1, (256-2*SIZE)) = gcomp;
			res3.range(256 - 1, (256-SIZE)) = bcomp;
			out_stream.write(res3);
		}
	}
}

/*
 * ImgSize is given in pixels
 */
// mergeRGBChannels for the case N = 512 - 64 Byte
static void mergeRGBChannels(hls::stream<dt64>& sR,  hls::stream<dt64>& sG,  hls::stream<dt64>& sB, hls::stream<dt64>& out_stream,  unsigned int ImgSize, unsigned int NbImages)
{
	dt64 res1, res2, res3, R, G, B;
	unsigned char d1, d2, rcomp, gcomp, bcomp, aux;
	for (unsigned int k=0; k<NbImages; k++)
	{
	mem_wr1:
		for (int i = 0; i < (ImgSize*3*SIZE/512); i+=3) {  // for each pixel, read the three components
	#pragma HLS pipeline
			R = sR.read();
			G = sG.read();
			B = sB.read();
			for (int j=0; j<64-2; j+=3)
			 {
		#pragma HLS unroll
				rcomp = (R.range(SIZE*(j/3+1)-1,SIZE*j/3).to_int());  //RIPARTIRE DA QUA
				gcomp = (G.range(SIZE*(j/3+1)-1,SIZE*j/3).to_int());
				bcomp = (B.range(SIZE*(j/3+1)-1,SIZE*j/3).to_int());
				res1.range(SIZE * (j + 1) - 1, SIZE * j) = rcomp;
				res1.range(SIZE * (j + 2) - 1, SIZE * (j+1)) = gcomp;
				res1.range(SIZE * (j + 3) - 1, SIZE * (j+2)) = bcomp;
			 }
			// after previous for loop 63 pixel components have been read/written => 21 r / g / b components have been read/written. Still 1 components (r ) to be read/written to finish consuming the 64 values available
			rcomp = (R.range(SIZE*22-1,SIZE*21).to_int());
			res1.range(512 - 1, 512-SIZE) = rcomp;
			out_stream.write(res1);

			for (int j=0; j<64-2; j+=3)
			 {
		#pragma HLS unroll
				gcomp = (G.range(SIZE*(21+(j/3+1))-1,SIZE*(21+j/3)).to_int());
				bcomp = (B.range(SIZE*(21+(j/3+1))-1,SIZE*(21+j/3)).to_int());
				rcomp = (R.range(SIZE*(22+(j/3+1))-1,SIZE*(22+j/3)).to_int());
				res2.range(SIZE * (j + 1) - 1, SIZE * j) = gcomp;
				res2.range(SIZE * (j + 2) - 1, SIZE * (j+1)) = bcomp;
				res2.range(SIZE * (j + 3) - 1, SIZE * (j+2)) = rcomp;
			 }
			// this time the component still to be read is g
			gcomp = (G.range(SIZE*43-1,SIZE*42).to_int());
			res2.range(512 - 1, 512-SIZE) = gcomp;

			out_stream.write(res2);

			for (int j=0; j<64-2; j+=3)
			 {
		#pragma HLS unroll
				bcomp = (B.range(SIZE*(42+(j/3+1))-1,SIZE*(42+j/3)).to_int());
				rcomp = (R.range(SIZE*(43+(j/3+1))-1,SIZE*(43+j/3)).to_int());
				gcomp = (G.range(SIZE*(43+(j/3+1))-1,SIZE*(43+j/3)).to_int());

				res3.range(SIZE * (j + 1) - 1, SIZE * j) = bcomp;
				res3.range(SIZE * (j + 2) - 1, SIZE * (j+1)) = rcomp;
				res3.range(SIZE * (j + 3) - 1, SIZE * (j+2)) = gcomp;
			 }
			bcomp = (B.range(512 - 1, 512-SIZE).to_int());
			res3.range(512 - 1, 512-SIZE) = bcomp;

			out_stream.write(res3);
		}
	}
}

//template <int W>
//static void mergeRGBChannels(hls::stream<dt>& sR,  hls::stream<dt>& sG,  hls::stream<dt>& sB, hls::stream<dt>& out_stream,  unsigned int ImgSize, unsigned int NbImages)
//{
//#if W==512
//	mergeRGBChannels_64B(sR, sG, sB, out_stream, ImgSize, NbImages);
//#elif W==256
//	mergeRGBChannels_32B(sR, sG, sB, out_stream, ImgSize, NbImages);
//#elif W==128
//	mergeRGBChannels_16B(sR, sG, sB, out_stream, ImgSize, NbImages);
//#endif
//}

