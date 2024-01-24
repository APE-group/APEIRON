template <typename T>   // S is the size, in bits, of the pixel component. W is the width, in bit, of data type T
T maxVal(T a, T b);

template <typename T>   // S is the size, in bits, of the pixel component. W is the width, in bit, of data type T
T minVal(T a, T b);

void setNbImages(unsigned int n, unsigned int &nA, unsigned int &nB);

template <typename T>
static void sinkStream(hls::stream<T>& in_stream,unsigned int ImgSize, unsigned int NbImages);

template <typename T, int S, int W>  // S is the size, in bits, of the pixel component. W is the width, in bit, of data type T
void PassThrough(hls::stream<T>& in_stream,  hls::stream<T>& out_stream1,  unsigned int ImgSize, unsigned int NbImages);

template <typename T, int S, int W>   // S is the size, in bits, of the pixel component. W is the width, in bit, of data type T
void printStreamToTextFile(hls::stream<T>& inStream, unsigned int streamId, unsigned int ImgSize, unsigned short int ImgCols, unsigned int NbImages);


template <typename T, int S, int W>   // S is the size, in bits, of the pixel component. W is the width, in bit, of data type T
void printStreamToBinaryFile(hls::stream<T>& inStream, unsigned int streamId, unsigned int ImgSize, unsigned int NbImages);

