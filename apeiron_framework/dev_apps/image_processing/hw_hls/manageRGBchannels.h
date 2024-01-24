// spliRGBChannels for the case N = 128 - 16 Byte
static void splitRGBChannels(hls::stream<dt16>& in_stream1,  hls::stream<dt16>& sR,  hls::stream<dt16>& sG,  hls::stream<dt16>& sB, unsigned int ImgSize, unsigned int NbImages);

// splitRGBChannels - N=256 - 32 Byte
static void splitRGBChannels(hls::stream<dt32>& in_stream1,  hls::stream<dt32>& sR,  hls::stream<dt32>& sG,  hls::stream<dt32>& sB, unsigned int ImgSize, unsigned int NbImages);

// splitRGBChannels - N = 512, 64 Byte
static void splitRGBChannels(hls::stream<dt64>& in_stream1,  hls::stream<dt64>& sR,  hls::stream<dt64>& sG,  hls::stream<dt64>& sB, unsigned int ImgSize, unsigned int NbImages);

// mergeRGBChannels for the case N = 128 - 16 Byte
static void mergeRGBChannels(hls::stream<dt16>& sR,  hls::stream<dt16>& sG,  hls::stream<dt16>& sB, hls::stream<dt16>& out_stream,  unsigned int ImgSize, unsigned int NbImages);

// mergeRGBChannels for the case N = 256 - 32 Byte
static void mergeRGBChannels(hls::stream<dt32>& sR,  hls::stream<dt32>& sG,  hls::stream<dt32>& sB, hls::stream<dt32>& out_stream,  unsigned int ImgSize, unsigned int NbImages);

// mergeRGBChannels for the case N = 512 - 64 Byte
static void mergeRGBChannels(hls::stream<dt64>& sR,  hls::stream<dt64>& sG,  hls::stream<dt64>& sB, hls::stream<dt64>& out_stream,  unsigned int ImgSize, unsigned int NbImages);

