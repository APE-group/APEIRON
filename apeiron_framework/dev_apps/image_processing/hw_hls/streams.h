template <typename T, int S, int W>   // S is the size, in bits, of the pixel component. W is the width, in bit, of data type T
void streamCopy(hls::stream<T>& in_stream,  hls::stream<T>& out_stream1,  hls::stream<T>& out_stream2, unsigned int ImgSize, unsigned int NbImages);


template <typename T, int S, int W>   // S is the size, in bits, of the pixel component. W is the width, in bit, of data type T
void streamCopy(hls::stream<T>& in_stream,  hls::stream<T>& out_stream1,  hls::stream<T>& out_stream2, hls::stream<T>& out_stream3, unsigned int ImgSize, unsigned int NbImages);


template <typename T, int S, int W>   // S is the size, in bits, of the pixel component. W is the width, in bit, of data type T
void streamCopy(hls::stream<T>& in_stream,  hls::stream<T>& out_stream1,  hls::stream<T>& out_stream2, hls::stream<T>& out_stream3, hls::stream<T>& out_stream4, unsigned int ImgSize, unsigned int NbImages);


template <typename T, int S, int W>   // S is the size, in bits, of the pixel component. W is the width, in bit, of data type T
void streamCopy(hls::stream<T>& in_stream,  hls::stream<T>& out_stream1,  hls::stream<T>& out_stream2, hls::stream<T>& out_stream3, hls::stream<T>& out_stream4, hls::stream<T>& out_stream5, unsigned int ImgSize, unsigned int NbImages);


template <typename T, int S, int W>   // S is the size, in bits, of the pixel component. W is the width, in bit, of data type T
void split2Channels(hls::stream<T>& sin, hls::stream<T>& soutA, hls::stream<T>& soutB, unsigned int ImgSize, unsigned int NbImages);


template <typename T, int S, int W>   // S is the size, in bits, of the pixel component. W is the width, in bit, of data type T
void merge2Channels(hls::stream<T>& sinA, hls::stream<T>& sinB, hls::stream<T>& sout, unsigned int ImgSize, unsigned int NbImages);


// input stream is an axistream
void doubleStreamWidth(hls::stream<io_stream_16B>& in_stream,  hls::stream<dt32>& out_stream,  unsigned int ImgSize, unsigned int NbImages);


void doubleStreamWidth(hls::stream<dt16>& in_stream,  hls::stream<dt32>& out_stream,  unsigned int ImgSize, unsigned int NbImages);


void halveStreamWidth(hls::stream<dt32>& in_stream,  hls::stream<dt16>& out_stream,  unsigned int ImgSize, unsigned int NbImages);


// output stream is an axistream
void halveStreamWidth(hls::stream<dt32>& in_stream,  hls::stream<io_stream_16B>& out_stream,  unsigned int ImgSize, unsigned int NbImages);

void convertToAxiStream(hls::stream<dt32>& in_stream,  hls::stream<io_stream_32B>& out_stream,  unsigned int ImgSize, unsigned int NbImages);

void convertToAxiStream(hls::stream<dt16>& in_stream,  hls::stream<io_stream_16B>& out_stream,  unsigned int ImgSize, unsigned int NbImages);

void convertFromAxiStream(hls::stream<io_stream_32B>& in_stream,  hls::stream<dt32>& out_stream,  unsigned int ImgSize, unsigned int NbImages);

void convertFromAxiStream(hls::stream<dt16>& in_stream,  hls::stream<io_stream_16B>& out_stream,  unsigned int ImgSize, unsigned int NbImages);
