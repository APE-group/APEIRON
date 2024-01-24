#include "ape_hls/hapecom.hpp"

#define BUFFER_SIZE 256

/*
 * receives from inStream an image and stores it in Buffer. Once completely stored, reads image from Buffer and sends it to outStream
 */
template <typename T, int S, int W>   // S is the size, in bits, of the pixel component. W is the width, in bit, of data type T
void imageStoreAndForward(T* Buffer, hls::stream<T>& inStream, hls::stream<T>& outStream, unsigned int ImgSize, unsigned int NbImages)
{
	T tmp;
	for (unsigned int k=0; k<NbImages; k++)
	{
		for (int i = 0; i < (ImgSize*S/W); i++) {
	#pragma HLS pipeline
			tmp = inStream.read();  //read the image and stores it into an external memory bank
			Buffer[i] = tmp;
		}

			// read again image from memory and sends it to the output stream
		for (int i = 0; i < (ImgSize*S/W); i++) {
	#pragma HLS pipeline
			tmp = Buffer[i];
			outStream.write(tmp);
		}
	}
}

void readBufferFromStream(hls::stream<dt>& inStream, dtx4 *Buff, unsigned int index, unsigned int ImgSize)
{
	dt tmp1, tmp2, tmp3, tmp4;
	dtx4 tmp;
	// read from stream into Buff
	for (unsigned int i1 = 0; i1<INTERNAL_BUFF_SIZE/4; i1++)
	{
#pragma HLS pipeline
		if ((index+i1) < (ImgSize*SIZE/(4*N)))
		{
			tmp1 = inStream.read();  //read buffsize elements and stores them into an external memory bank
			tmp2 = inStream.read();  //read buffsize elements and stores them into an external memory bank
			tmp3 = inStream.read();  //read buffsize elements and stores them into an external memory bank
			tmp4 = inStream.read();  //read buffsize elements and stores them into an external memory bank
			tmp.range(N-1,0)=tmp1;
			tmp.range(2*N-1,N)=tmp2;
			tmp.range(3*N-1,2*N)=tmp3;
			tmp.range(4*N-1,3*N)=tmp4;
			Buff[i1] = tmp;
		}
	}
}
void writeBufferToStream(hls::stream<dt>& outStream, dtx4 *Buff, unsigned int index, unsigned int ImgSize)
{
	dt tmp1, tmp2, tmp3, tmp4;
	dtx4 tmp;

	// read from stream into Buff
	for (unsigned int i1 = 0; i1<INTERNAL_BUFF_SIZE/4; i1++)
	{
#pragma HLS pipeline
		if ((index+i1) < (ImgSize*SIZE/(4*N)))
		{
				tmp = Buff[i1];
				outStream.write(tmp.range(N-1,0));
				outStream.write(tmp.range(2*N-1,N));
				outStream.write(tmp.range(3*N-1,2*N));
				outStream.write(tmp.range(4*N-1,3*N));
		}
	}
}

void copyBuffToMem(dtx4 *mem, dtx4 *Buff, unsigned int index, unsigned int ImgSize)
{
	for (unsigned int i1 = 0; i1<INTERNAL_BUFF_SIZE/4; i1++)
	{
#pragma HLS pipeline
		if ((index+i1) < (ImgSize*SIZE/(4*N)))
		{
				mem[index+i1] = Buff[i1];
		}
	}
}

void copyMemToBuff(dtx4 *mem, dtx4 *Buff, unsigned int index, unsigned int ImgSize)
{
	for (unsigned int i1 = 0; i1<INTERNAL_BUFF_SIZE/4; i1++)
	{
#pragma HLS pipeline
		if ((index+i1) < (ImgSize*SIZE/(4*N)))
		{
			Buff[i1] = mem[index+i1];
		}
	}
}

// the moveBufferFromToMem read data from Buff and writes them into mem, starting at address write_index;
// after this, reads data from mem, starting at address read_index, and writes them into Buff
void moveBufferFromToMem(dtx4 *mem, dtx4 *Buff, unsigned int read_index, unsigned int write_index, unsigned int ImgSize)
{
	copyBuffToMem(mem, Buff, write_index, ImgSize);
	copyMemToBuff(mem, Buff, read_index, ImgSize);

}
/*
 * receives an image from the input stream;
 * after having received bufferSize elements (bytes), start sending the content of the buffer to the output stream
 * bufferSize and ImgSize (in Bytes) are supposed to be multiple of N*4/SIZE (if I read 16 Bytes/cycle, must be multiple of 64)
 */
void buffer(dtx4* mem, hls::stream<dt>& inStream, hls::stream<dt>& outStream, unsigned int bufferSize, unsigned int ImgSize, unsigned int NbImages)
{
	dt tmp1, tmp2, tmp3, tmp4;
	dtx4 tmp;
	dtx4 BuffA[INTERNAL_BUFF_SIZE];
	dtx4 BuffB[INTERNAL_BUFF_SIZE];
	dtx4 BuffC[INTERNAL_BUFF_SIZE];

	char phase = 0;
	int i, count;

	for (unsigned int k=0; k<NbImages; k++)
	{
		for (i = 0; i < ((bufferSize*SIZE/(4*N))); i++)
		{
	#pragma HLS pipeline
			tmp1 = inStream.read();  //read buffsize elements and stores them into an external memory bank
			tmp2 = inStream.read();  //read buffsize elements and stores them into an external memory bank
			tmp3 = inStream.read();  //read buffsize elements and stores them into an external memory bank
			tmp4 = inStream.read();  //read buffsize elements and stores them into an external memory bank
			tmp.range(N-1,0)=tmp1;
			tmp.range(2*N-1,N)=tmp2;
			tmp.range(3*N-1,2*N)=tmp3;
			tmp.range(4*N-1,3*N)=tmp4;
			mem[i] = tmp;
		}

		count = 0;
		phase = 1;
		// first stage of pipeline preamble
		// read from stream into BuffA
		readBufferFromStream(inStream, BuffA, i, ImgSize);
		i+=(INTERNAL_BUFF_SIZE/4);
		phase = 2;

		// second stage of pipeline preamble
		// copy BuffA into mem
//		copyBuffToMem(mem, BuffA, i-(INTERNAL_BUFF_SIZE/4), ImgSize);
//		// copy BuffA from mem
//		copyMemToBuff(mem, BuffA, count, ImgSize);
		moveBufferFromToMem(mem, BuffA, count, i-(INTERNAL_BUFF_SIZE/4), ImgSize);
		readBufferFromStream(inStream, BuffB, i, ImgSize);

		i+=(INTERNAL_BUFF_SIZE/4);
		count += (INTERNAL_BUFF_SIZE/4);

		phase = 3;

			// read again image from memory and sends it to the output stream
		for (; i < (ImgSize*SIZE/(4*N)); i+=(INTERNAL_BUFF_SIZE/4))
		{
			if (phase == 1)
			{
				// read from stream into BuffA
				readBufferFromStream(inStream, BuffA, i, ImgSize);
				// copy BuffC into mem
//				copyBuffToMem(mem, BuffC, i-(INTERNAL_BUFF_SIZE/4), ImgSize);
//				// copy BuffC from mem
//				copyMemToBuff(mem, BuffC, count, ImgSize);
				moveBufferFromToMem(mem, BuffC, count, i-(INTERNAL_BUFF_SIZE/4), ImgSize);
				//write to output stream from BuffB
				writeBufferToStream(outStream, BuffB, count-(INTERNAL_BUFF_SIZE/4), ImgSize);
				count += (INTERNAL_BUFF_SIZE/4);
				phase = 2;
			}
			else if (phase == 2){
				// read from stream into BuffB
				readBufferFromStream(inStream, BuffB, i, ImgSize);
				// copy BuffA into mem
//				copyBuffToMem(mem, BuffA, i-(INTERNAL_BUFF_SIZE/4), ImgSize);
//				// copy BuffA from mem
//				copyMemToBuff(mem, BuffA, count, ImgSize);
				moveBufferFromToMem(mem, BuffA, count, i-(INTERNAL_BUFF_SIZE/4), ImgSize);
				//write to output stream from BuffC
				writeBufferToStream(outStream, BuffC, count-(INTERNAL_BUFF_SIZE/4), ImgSize);
				count += (INTERNAL_BUFF_SIZE/4);
				phase = 3;
			}
			else if (phase == 3){
				// read from stream into BuffC
				readBufferFromStream(inStream, BuffC, i, ImgSize);
				// copy BuffB into mem
//				copyBuffToMem(mem, BuffB, i-(INTERNAL_BUFF_SIZE/4), ImgSize);
//				// copy BuffB from mem
//				copyMemToBuff(mem, BuffB, count, ImgSize);
				// merge copyBuffToMem() and copyMemToBuff() in one function, so that they are scheduled sequentially
				// without this merging, the copyMemToBuff() is scheduled alone, after the end of readBufferFromStream()
				moveBufferFromToMem(mem, BuffB, count, i-(INTERNAL_BUFF_SIZE/4), ImgSize);
				//write to output stream from BuffA
				writeBufferToStream(outStream, BuffA, count-(INTERNAL_BUFF_SIZE/4), ImgSize);
				count += (INTERNAL_BUFF_SIZE/4);
				phase = 1;
			}
		}

		// two stages of pipeline postamble

		// first stage of postamble
		if (phase == 1)
		{
//			// copy BuffC into mem
//			copyBuffToMem(mem, BuffC, i-(INTERNAL_BUFF_SIZE/4), ImgSize);
//			// copy BuffC from mem
//			copyMemToBuff(mem, BuffC, count, ImgSize);
			moveBufferFromToMem(mem, BuffC, count, i-(INTERNAL_BUFF_SIZE/4), ImgSize);
			//write to output stream from BuffB
			writeBufferToStream(outStream, BuffB, count-(INTERNAL_BUFF_SIZE/4), ImgSize);
			phase = 2;
		}
		else if (phase == 2){
//			// copy BuffA into mem
//			copyBuffToMem(mem, BuffA, i-(INTERNAL_BUFF_SIZE/4), ImgSize);
//			// copy BuffA from mem
//			copyMemToBuff(mem, BuffA, count, ImgSize);
			moveBufferFromToMem(mem, BuffA, count, i-(INTERNAL_BUFF_SIZE/4), ImgSize);
			//write to output stream from BuffC
			writeBufferToStream(outStream, BuffC, count-(INTERNAL_BUFF_SIZE/4), ImgSize);
			phase = 3;
		}
		else if (phase == 3){
			// copy BuffB into mem
//			copyBuffToMem(mem, BuffB, i-(INTERNAL_BUFF_SIZE/4), ImgSize);
//			// copy BuffB from mem
//			copyMemToBuff(mem, BuffB, count, ImgSize);
			moveBufferFromToMem(mem, BuffB, count, i-(INTERNAL_BUFF_SIZE/4), ImgSize);
			//write to output stream from BuffA
			writeBufferToStream(outStream, BuffA, count-(INTERNAL_BUFF_SIZE/4), ImgSize);
			phase = 1;
		}
		count += (INTERNAL_BUFF_SIZE/4);
		i+=(INTERNAL_BUFF_SIZE/4);

		// second stage of postamble
		if (phase == 1)
		{
			//write to output stream from BuffB
			writeBufferToStream(outStream, BuffB, count-(INTERNAL_BUFF_SIZE/4), ImgSize);
//			count += (INTERNAL_BUFF_SIZE/4);  // this last count increment must not be done to properly indicate the transmitted elements
			phase = 2;
		}
		else if (phase == 2){
			//write to output stream from BuffC
			writeBufferToStream(outStream, BuffC, count-(INTERNAL_BUFF_SIZE/4), ImgSize);
//			count += (INTERNAL_BUFF_SIZE/4);
			phase = 3;
		}
		else if (phase == 3){
			//write to output stream from BuffA
			writeBufferToStream(outStream, BuffA, count-(INTERNAL_BUFF_SIZE/4), ImgSize);
//			count += (INTERNAL_BUFF_SIZE/4);
			phase = 1;
		}
		i+=(INTERNAL_BUFF_SIZE/4);

		for (; count < (ImgSize*SIZE/(4*N)); count++)
		{
	#pragma HLS pipeline
			outStream.write(mem[count].range(N-1,0));
			outStream.write(mem[count].range(2*N-1,N));
			outStream.write(mem[count].range(3*N-1,2*N));
			outStream.write(mem[count].range(4*N-1,3*N));
		}
	}
}

template <typename T>
void stream2Buffer(hls::stream<T>& inStream, T Buff[BUFFER_SIZE], unsigned int size)
{
#pragma HLS inline off
	T tmp;

	// read from stream into Buff
	for (unsigned int i1 = 0; i1<size; i1++)
	{
#pragma HLS pipeline
			tmp = inStream.read();  //read buffsize elements and stores them into an external memory bank
			Buff[i1] = tmp;
	}
}

template <typename T>
void buffer2Stream(hls::stream<T>& outStream, T Buff[BUFFER_SIZE], unsigned int size)
{
#pragma HLS inline off
	// copy Buff to stream
	for (unsigned int i = 0; i<size; i++)
	{
#pragma HLS pipeline
			outStream.write(Buff[i]);  //read buffsize elements and stores them into an external memory bank
	}
}

/*
 * bufferedCommunication reads from inStream filling an internal buffer
  *                       and, when the buffer is full, it is written to the output stream with a burst access
 */
template <typename T, int S, int W>
void bufferedCommunication(hls::stream<T>& inStream, hls::stream<T>& outStream, unsigned int ImgSize, unsigned int NbImages)
{
	bool phase = true;
	unsigned int i,size, writeSize;
	T Buff1[BUFFER_SIZE];
	T Buff2[BUFFER_SIZE];
	unsigned int NbWordToTransfer = ImgSize/(W/S)*NbImages;
	if (NbWordToTransfer <= BUFFER_SIZE)
		size = NbWordToTransfer;
	else
		size = BUFFER_SIZE;
	stream2Buffer(inStream, Buff1, size);
	NbWordToTransfer -= size;

	while (NbWordToTransfer > BUFFER_SIZE)
	{
		if (phase)
		{
			buffer2Stream(outStream, Buff1, BUFFER_SIZE);
			stream2Buffer(inStream, Buff2, BUFFER_SIZE);
		}
		else
		{
			buffer2Stream(outStream, Buff2, BUFFER_SIZE);
			stream2Buffer(inStream, Buff1, BUFFER_SIZE);

		}
		phase = !phase;
		NbWordToTransfer -= BUFFER_SIZE;
	}

	// last transfer from the input stream

	if (phase)
	{
		buffer2Stream(outStream, Buff1, NbWordToTransfer);
		stream2Buffer(inStream, Buff2, BUFFER_SIZE);
	}
	else
	{
		buffer2Stream(outStream, Buff2, NbWordToTransfer);
		stream2Buffer(inStream, Buff1, BUFFER_SIZE);

	}
	phase = !phase;
	// last write to the output stream
	if (phase)
		buffer2Stream(outStream, Buff1, NbWordToTransfer);
	else
		buffer2Stream(outStream, Buff2, NbWordToTransfer);
}


// APEIRON bufferedCommunication
template <typename T, int S, int W>
void bufferedCommunication(hls::stream<T>& inStream, message_stream_t message_data_out[N_OUTPUT_CHANNELS], unsigned int ImgSize, unsigned int NbImages, unsigned int ch_id)
{
#pragma HLS inline off
	unsigned int coord = 0;
	bool phase = true;
	unsigned int i,size, writeSize;
	T Buff1[BUFFER_SIZE];
	T Buff2[BUFFER_SIZE];
	unsigned int NbWordToTransfer = ImgSize/(W/S)*NbImages;
	if (NbWordToTransfer <= BUFFER_SIZE)
		size = NbWordToTransfer;
	else
		size = BUFFER_SIZE;
	stream2Buffer(inStream, Buff1, size);
	NbWordToTransfer -= size;

	while (NbWordToTransfer > BUFFER_SIZE)
	{
		if (phase)
		{
			//buffer2Stream(outStream, Buff1, BUFFER_SIZE);
			send(Buff1, BUFFER_SIZE*sizeof(word_t), coord, ch_id+1, ch_id, message_data_out);
			stream2Buffer(inStream, Buff2, BUFFER_SIZE);
		}
		else
		{
			//buffer2Stream(outStream, Buff2, BUFFER_SIZE);
			send(Buff2, BUFFER_SIZE*sizeof(word_t), coord, ch_id+1, ch_id, message_data_out);
			stream2Buffer(inStream, Buff1, BUFFER_SIZE);

		}
		phase = !phase;
		NbWordToTransfer -= BUFFER_SIZE;
	}

	// last transfer from the input stream

	if (phase)
	{
		//buffer2Stream(outStream, Buff1, NbWordToTransfer);
		send(Buff1, NbWordToTransfer*sizeof(word_t), coord, ch_id+1, ch_id, message_data_out);
		stream2Buffer(inStream, Buff2, BUFFER_SIZE);
	}
	else
	{
		//buffer2Stream(outStream, Buff2, NbWordToTransfer);
		send(Buff2, NbWordToTransfer*sizeof(word_t), coord, ch_id+1, ch_id, message_data_out);
		stream2Buffer(inStream, Buff1, BUFFER_SIZE);

	}
	phase = !phase;
	// last write to the output stream
	if (phase)
		//buffer2Stream(outStream, Buff1, NbWordToTransfer);
		send(Buff1, NbWordToTransfer*sizeof(word_t), coord, ch_id+1, ch_id, message_data_out);
	else
		//buffer2Stream(outStream, Buff2, NbWordToTransfer);
		send(Buff2, NbWordToTransfer*sizeof(word_t), coord, ch_id+1, ch_id, message_data_out);
}

template <typename T, int S, int W>
void bufferedCommunication(hls::stream<T>& inStream, message_stream_t message_data_out[N_OUTPUT_CHANNELS], unsigned int ImgSize, unsigned int NbImages, unsigned int task_id, unsigned int ch_id, unsigned int packet_size)
{
#pragma HLS inline off
	unsigned int coord = 0;
	bool phase = true;
	unsigned int i,size, writeSize;
	T Buff1[BUFFER_SIZE];
	T Buff2[BUFFER_SIZE];
	unsigned int NbWordToTransfer = ImgSize/(W/S)*NbImages;
	if (NbWordToTransfer <= packet_size)
		size = NbWordToTransfer;
	else
		size = packet_size;
	stream2Buffer(inStream, Buff1, size);
	NbWordToTransfer -= size;

	while (NbWordToTransfer > packet_size)
	{
		if (phase)
		{
			//buffer2Stream(outStream, Buff1, BUFFER_SIZE);
			send(Buff1, packet_size*sizeof(word_t), coord, task_id, ch_id, message_data_out);
			stream2Buffer(inStream, Buff2, packet_size);
		}
		else
		{
			//buffer2Stream(outStream, Buff2, BUFFER_SIZE);
			send(Buff2, packet_size*sizeof(word_t), coord, task_id, ch_id, message_data_out);
			stream2Buffer(inStream, Buff1, packet_size);

		}
		phase = !phase;
		NbWordToTransfer -= packet_size;
	}

	// last transfer from the input stream

	if (phase)
	{
		//buffer2Stream(outStream, Buff1, NbWordToTransfer);
		send(Buff1, NbWordToTransfer*sizeof(word_t), coord, task_id, ch_id, message_data_out);
		stream2Buffer(inStream, Buff2, packet_size);
	}
	else
	{
		//buffer2Stream(outStream, Buff2, NbWordToTransfer);
		send(Buff2, NbWordToTransfer*sizeof(word_t), coord, task_id, ch_id, message_data_out);
		stream2Buffer(inStream, Buff1, packet_size);

	}
	phase = !phase;
	// last write to the output stream
	if (phase)
		//buffer2Stream(outStream, Buff1, NbWordToTransfer);
		send(Buff1, NbWordToTransfer*sizeof(word_t), coord, task_id, ch_id, message_data_out);
	else
		//buffer2Stream(outStream, Buff2, NbWordToTransfer);
		send(Buff2, NbWordToTransfer*sizeof(word_t), coord, task_id, ch_id, message_data_out);
}

template <typename T, int S, int W>
void bufferedCommunicationAll(hls::stream<T>& inStream0, hls::stream<T>& inStream1, hls::stream<T>& inStream2, message_stream_t message_data_out[N_OUTPUT_CHANNELS], unsigned int ImgSize, unsigned int NbImages, unsigned int packet_size)
{
#pragma HLS inline off
	unsigned int coord = 0;
	bool phase = true;
	unsigned int i,size, writeSize;
	T Buff0_1[BUFFER_SIZE];
	T Buff0_2[BUFFER_SIZE];
	
	T Buff1_1[BUFFER_SIZE];
	T Buff1_2[BUFFER_SIZE];
	
	T Buff2_1[BUFFER_SIZE];
	T Buff2_2[BUFFER_SIZE];
	
	unsigned int NbWordToTransfer = ImgSize/(W/S)*NbImages;
	if (NbWordToTransfer <= packet_size)
		size = NbWordToTransfer;
	else
		size = packet_size;
	stream2Buffer(inStream0, Buff0_1, size);
	stream2Buffer(inStream1, Buff1_1, size);
	stream2Buffer(inStream2, Buff2_1, size);
	NbWordToTransfer -= size;

	while (NbWordToTransfer > packet_size)
	{
		if (phase)
		{
			//buffer2Stream(outStream, Buff1, BUFFER_SIZE);
			send(Buff0_1, packet_size*sizeof(word_t), coord, 1, 0, message_data_out);
			send(Buff1_1, packet_size*sizeof(word_t), coord, 2, 1, message_data_out);
			send(Buff2_1, packet_size*sizeof(word_t), coord, 3, 2, message_data_out);
			stream2Buffer(inStream0, Buff0_2, packet_size);
			stream2Buffer(inStream1, Buff1_2, packet_size);
			stream2Buffer(inStream2, Buff2_2, packet_size);
		}
		else
		{
			//buffer2Stream(outStream, Buff2, BUFFER_SIZE);
			send(Buff0_2, packet_size*sizeof(word_t), coord, 1, 0, message_data_out);
			send(Buff1_2, packet_size*sizeof(word_t), coord, 2, 1, message_data_out);
			send(Buff2_2, packet_size*sizeof(word_t), coord, 3, 2, message_data_out);
			stream2Buffer(inStream0, Buff0_1, packet_size);
			stream2Buffer(inStream1, Buff1_1, packet_size);
			stream2Buffer(inStream2, Buff2_1, packet_size);

		}
		phase = !phase;
		NbWordToTransfer -= packet_size;
	}

	// last transfer from the input stream

	if (phase)
	{
		//buffer2Stream(outStream, Buff1, NbWordToTransfer);
		send(Buff0_1, NbWordToTransfer*sizeof(word_t), coord, 1, 0, message_data_out);
		send(Buff1_1, NbWordToTransfer*sizeof(word_t), coord, 2, 1, message_data_out);
		send(Buff2_1, NbWordToTransfer*sizeof(word_t), coord, 3, 2, message_data_out);
		stream2Buffer(inStream0, Buff0_2, packet_size);
		stream2Buffer(inStream1, Buff1_2, packet_size);
		stream2Buffer(inStream2, Buff2_2, packet_size);
	}
	else
	{
		//buffer2Stream(outStream, Buff2, NbWordToTransfer);
		send(Buff0_2, NbWordToTransfer*sizeof(word_t), coord, 1, 0, message_data_out);
		send(Buff1_2, NbWordToTransfer*sizeof(word_t), coord, 2, 1, message_data_out);
		send(Buff2_2, NbWordToTransfer*sizeof(word_t), coord, 3, 2, message_data_out);
		stream2Buffer(inStream0, Buff0_1, packet_size);
		stream2Buffer(inStream1, Buff1_1, packet_size);
		stream2Buffer(inStream2, Buff2_1, packet_size);

	}
	phase = !phase;
	// last write to the output stream
	if (phase){
		//buffer2Stream(outStream, Buff1, NbWordToTransfer);
		send(Buff0_1, NbWordToTransfer*sizeof(word_t), coord, 1, 0, message_data_out);
		send(Buff1_1, NbWordToTransfer*sizeof(word_t), coord, 2, 1, message_data_out);
		send(Buff2_1, NbWordToTransfer*sizeof(word_t), coord, 3, 2, message_data_out);
		}
	else{
		//buffer2Stream(outStream, Buff2, NbWordToTransfer);
		send(Buff0_2, NbWordToTransfer*sizeof(word_t), coord, 1, 0, message_data_out);
		send(Buff1_2, NbWordToTransfer*sizeof(word_t), coord, 2, 1, message_data_out);
		send(Buff2_2, NbWordToTransfer*sizeof(word_t), coord, 3, 2, message_data_out);
		}
}

template <typename T, int S, int W>
void bufferedCommunicationAll_multiDevice(hls::stream<T>& inStream0, hls::stream<T>& inStream1, hls::stream<T>& inStream2, message_stream_t message_data_out[N_OUTPUT_CHANNELS], unsigned int ImgSize, unsigned int NbImages, unsigned int packet_size)
{
#pragma HLS inline off
	//unsigned int coord = 0;
	bool phase = true;
	unsigned int i,size, writeSize;
	T Buff0_1[BUFFER_SIZE];
	T Buff0_2[BUFFER_SIZE];
	
	T Buff1_1[BUFFER_SIZE];
	T Buff1_2[BUFFER_SIZE];
	
	T Buff2_1[BUFFER_SIZE];
	T Buff2_2[BUFFER_SIZE];
	
	unsigned int NbWordToTransfer = ImgSize/(W/S)*NbImages;
	if (NbWordToTransfer <= packet_size)
		size = NbWordToTransfer;
	else
		size = packet_size;
	stream2Buffer(inStream0, Buff0_1, size);
	stream2Buffer(inStream1, Buff1_1, size);
	stream2Buffer(inStream2, Buff2_1, size);
	NbWordToTransfer -= size;

	while (NbWordToTransfer > packet_size)
	{
		if (phase)
		{
			//buffer2Stream(outStream, Buff1, BUFFER_SIZE);
			
			send(Buff0_1, packet_size*sizeof(word_t), 1, 1, 0, message_data_out);
			send(Buff1_1, packet_size*sizeof(word_t), 2, 1, 1, message_data_out);
			send(Buff2_1, packet_size*sizeof(word_t), 3, 1, 2, message_data_out);
			stream2Buffer(inStream0, Buff0_2, packet_size);
			stream2Buffer(inStream1, Buff1_2, packet_size);
			stream2Buffer(inStream2, Buff2_2, packet_size);
		}
		else
		{
			//buffer2Stream(outStream, Buff2, BUFFER_SIZE);
			send(Buff0_2, packet_size*sizeof(word_t), 1, 1, 0, message_data_out);
			send(Buff1_2, packet_size*sizeof(word_t), 2, 1, 1, message_data_out);
			send(Buff2_2, packet_size*sizeof(word_t), 3, 1, 2, message_data_out);
			stream2Buffer(inStream0, Buff0_1, packet_size);
			stream2Buffer(inStream1, Buff1_1, packet_size);
			stream2Buffer(inStream2, Buff2_1, packet_size);

		}
		phase = !phase;
		NbWordToTransfer -= packet_size;
	}

	// last transfer from the input stream

	if (phase)
	{
		//buffer2Stream(outStream, Buff1, NbWordToTransfer);
		send(Buff0_1, NbWordToTransfer*sizeof(word_t), 1, 1, 0, message_data_out);
		send(Buff1_1, NbWordToTransfer*sizeof(word_t), 2, 1, 1, message_data_out);
		send(Buff2_1, NbWordToTransfer*sizeof(word_t), 3, 1, 2, message_data_out);
		stream2Buffer(inStream0, Buff0_2, packet_size);
		stream2Buffer(inStream1, Buff1_2, packet_size);
		stream2Buffer(inStream2, Buff2_2, packet_size);
	}
	else
	{
		//buffer2Stream(outStream, Buff2, NbWordToTransfer);
		send(Buff0_2, NbWordToTransfer*sizeof(word_t), 1, 1, 0, message_data_out);
		send(Buff1_2, NbWordToTransfer*sizeof(word_t), 2, 1, 1, message_data_out);
		send(Buff2_2, NbWordToTransfer*sizeof(word_t), 3, 1, 2, message_data_out);
		stream2Buffer(inStream0, Buff0_1, packet_size);
		stream2Buffer(inStream1, Buff1_1, packet_size);
		stream2Buffer(inStream2, Buff2_1, packet_size);

	}
	phase = !phase;
	// last write to the output stream
	if (phase){
		//buffer2Stream(outStream, Buff1, NbWordToTransfer);
		send(Buff0_1, NbWordToTransfer*sizeof(word_t), 1, 1, 0, message_data_out);
		send(Buff1_1, NbWordToTransfer*sizeof(word_t), 2, 1, 1, message_data_out);
		send(Buff2_1, NbWordToTransfer*sizeof(word_t), 3, 1, 2, message_data_out);
		}
	else{
		//buffer2Stream(outStream, Buff2, NbWordToTransfer);
		send(Buff0_2, NbWordToTransfer*sizeof(word_t), 1, 1, 0, message_data_out);
		send(Buff1_2, NbWordToTransfer*sizeof(word_t),  2, 1, 1, message_data_out);
		send(Buff2_2, NbWordToTransfer*sizeof(word_t), 3, 1, 2, message_data_out);
		}
}

template <typename T, int S, int W>
void bufferedCommunicationAll_multiDevice(hls::stream<T>& inStream0, hls::stream<T>& inStream1, hls::stream<T>& inStream2, message_stream_t message_data_out[N_OUTPUT_CHANNELS], unsigned int ImgSize, unsigned int NbImages, unsigned int dest_coord, unsigned int packet_size)
{
#pragma HLS inline off
	//unsigned int coord = 0;
	bool phase = true;
	unsigned int i,size, writeSize;
	T Buff0_1[BUFFER_SIZE];
	T Buff0_2[BUFFER_SIZE];
	
	T Buff1_1[BUFFER_SIZE];
	T Buff1_2[BUFFER_SIZE];
	
	T Buff2_1[BUFFER_SIZE];
	T Buff2_2[BUFFER_SIZE];
	
	unsigned int NbWordToTransfer = ImgSize/(W/S)*NbImages;
	if (NbWordToTransfer <= packet_size)
		size = NbWordToTransfer;
	else
		size = packet_size;
	stream2Buffer(inStream0, Buff0_1, size);
	stream2Buffer(inStream1, Buff1_1, size);
	stream2Buffer(inStream2, Buff2_1, size);
	NbWordToTransfer -= size;

	while (NbWordToTransfer > packet_size)
	{
		if (phase)
		{
			//buffer2Stream(outStream, Buff1, BUFFER_SIZE);
			
			send(Buff0_1, packet_size*sizeof(word_t), dest_coord, 1, 0, message_data_out);
			send(Buff1_1, packet_size*sizeof(word_t), dest_coord, 2, 1, message_data_out);
			send(Buff2_1, packet_size*sizeof(word_t), dest_coord, 3, 2, message_data_out);
			stream2Buffer(inStream0, Buff0_2, packet_size);
			stream2Buffer(inStream1, Buff1_2, packet_size);
			stream2Buffer(inStream2, Buff2_2, packet_size);
		}
		else
		{
			//buffer2Stream(outStream, Buff2, BUFFER_SIZE);
			send(Buff0_2, packet_size*sizeof(word_t), dest_coord, 1, 0, message_data_out);
			send(Buff1_2, packet_size*sizeof(word_t), dest_coord, 2, 1, message_data_out);
			send(Buff2_2, packet_size*sizeof(word_t), dest_coord, 3, 2, message_data_out);
			stream2Buffer(inStream0, Buff0_1, packet_size);
			stream2Buffer(inStream1, Buff1_1, packet_size);
			stream2Buffer(inStream2, Buff2_1, packet_size);

		}
		phase = !phase;
		NbWordToTransfer -= packet_size;
	}

	// last transfer from the input stream

	if (phase)
	{
		//buffer2Stream(outStream, Buff1, NbWordToTransfer);
		send(Buff0_1, NbWordToTransfer*sizeof(word_t), dest_coord, 1, 0, message_data_out);
		send(Buff1_1, NbWordToTransfer*sizeof(word_t), dest_coord, 2, 1, message_data_out);
		send(Buff2_1, NbWordToTransfer*sizeof(word_t), dest_coord, 3, 2, message_data_out);
		stream2Buffer(inStream0, Buff0_2, packet_size);
		stream2Buffer(inStream1, Buff1_2, packet_size);
		stream2Buffer(inStream2, Buff2_2, packet_size);
	}
	else
	{
		//buffer2Stream(outStream, Buff2, NbWordToTransfer);
		send(Buff0_2, NbWordToTransfer*sizeof(word_t), dest_coord, 1, 0, message_data_out);
		send(Buff1_2, NbWordToTransfer*sizeof(word_t), dest_coord, 2, 1, message_data_out);
		send(Buff2_2, NbWordToTransfer*sizeof(word_t), dest_coord, 3, 2, message_data_out);
		stream2Buffer(inStream0, Buff0_1, packet_size);
		stream2Buffer(inStream1, Buff1_1, packet_size);
		stream2Buffer(inStream2, Buff2_1, packet_size);

	}
	phase = !phase;
	// last write to the output stream
	if (phase){
		//buffer2Stream(outStream, Buff1, NbWordToTransfer);
		send(Buff0_1, NbWordToTransfer*sizeof(word_t), dest_coord, 1, 0, message_data_out);
		send(Buff1_1, NbWordToTransfer*sizeof(word_t), dest_coord, 2, 1, message_data_out);
		send(Buff2_1, NbWordToTransfer*sizeof(word_t), dest_coord, 3, 2, message_data_out);
		}
	else{
		//buffer2Stream(outStream, Buff2, NbWordToTransfer);
		send(Buff0_2, NbWordToTransfer*sizeof(word_t), dest_coord, 1, 0, message_data_out);
		send(Buff1_2, NbWordToTransfer*sizeof(word_t), dest_coord, 2, 1, message_data_out);
		send(Buff2_2, NbWordToTransfer*sizeof(word_t), dest_coord, 3, 2, message_data_out);
		}
}


//receive
template <typename T, int S, int W>
void bufferedCommunication(message_stream_t message_data_in[N_INPUT_CHANNELS], hls::stream<T>& outStream, unsigned int ImgSize, unsigned int NbImages, unsigned int ch_id, unsigned int packet_size)
{
#pragma HLS inline off
	bool phase = true;
	unsigned int i,size, writeSize;
	
	unsigned int NbWordToTransfer = ImgSize/(W/S)*NbImages;
	
	unsigned int npackets_full = (unsigned int) NbWordToTransfer / packet_size;
	unsigned int nwords_left = NbWordToTransfer - (packet_size * npackets_full);
	

	for(int i=0; i < npackets_full*packet_size; i++){
	#pragma HLS pipeline II=1
		int nwords = receive_streaming(ch_id, outStream, message_data_in);
		
	}
	
	// last transfer from the input stream
	for(int i=0; i<nwords_left; i++){
	#pragma HLS pipeline II=1
		int nwords = receive_streaming(ch_id, outStream, message_data_in);
	}
}


template <typename T, int S, int W>
void bufferedCommunicationAll(message_stream_t message_data_in[N_INPUT_CHANNELS], hls::stream<T>& outStream0, hls::stream<T>& outStream1, hls::stream<T>& outStream2, unsigned int ImgSize, unsigned int NbImages, unsigned int packet_size)
{
#pragma HLS inline off
	bool phase = true;
	unsigned int i,size, writeSize;

	unsigned int NbWordToTransfer = ImgSize/(W/S)*NbImages;
	if (NbWordToTransfer <= packet_size)
		size = NbWordToTransfer;
	else
		size = packet_size;
	//stream2Buffer(inStream, Buff1, size);
/*	receive(0, outStream0, message_data_in);
	receive(1, outStream1, message_data_in);
	receive(2, outStream2, message_data_in);
*/
	receive_all(outStream0, outStream1, outStream2, message_data_in, message_data_in+1, message_data_in+2);	
	NbWordToTransfer -= size;

	while (NbWordToTransfer > packet_size)
	{
			//receive(0, outStream0, message_data_in);
			//receive(1, outStream1, message_data_in);
			//receive(2, outStream2, message_data_in);
			
			receive_all(outStream0, outStream1, outStream2, message_data_in, message_data_in+1, message_data_in+2);
			
			NbWordToTransfer -= packet_size;
	}

	// last transfer from the input stream
		
		//receive(0, outStream0, message_data_in);
		//receive(1, outStream1, message_data_in);
		//receive(2, outStream2, message_data_in);
		
		receive_all(outStream0, outStream1, outStream2, message_data_in, message_data_in+1, message_data_in+2);
	
}


/*****************************************************/


// buffered communication converting from internal stream to AXI stream - 16Bytes

void stream2Buffer(hls::stream<dt16>& inStream, dt16 Buff[BUFFER_SIZE], unsigned int size)
{
#pragma HLS inline off
	dt16 tmp;

	// read from stream into Buff
	for (unsigned int i1 = 0; i1<size; i1++)
	{
#pragma HLS pipeline
			tmp = inStream.read();  //read buffsize elements and stores them into an external memory bank
			Buff[i1] = tmp;
	}
}

void buffer2Stream(hls::stream<io_stream_16B>& outStream, dt16 Buff[BUFFER_SIZE], unsigned int size)
{
#pragma HLS inline off
	io_stream_16B tmp;
	tmp.keep = 0xFFFF;
	tmp.last = false;
	// copy Buff to stream
	for (unsigned int i = 0; i<size; i++)
	{
#pragma HLS pipeline
			tmp.data = Buff[i];
			outStream.write(tmp);
	}
}
void bufferedCommunication(hls::stream<dt16>& inStream, hls::stream<io_stream_16B>& outStream, unsigned int ImgSize, unsigned int NbImages)
{
	bool phase = true;
	unsigned int i,size, writeSize;
	dt16 Buff1[BUFFER_SIZE];
	dt16 Buff2[BUFFER_SIZE];
	unsigned int NbWordToTransfer = ImgSize/(16)*NbImages;
	if (NbWordToTransfer <= BUFFER_SIZE)
		size = NbWordToTransfer;
	else
		size = BUFFER_SIZE;
	stream2Buffer(inStream, Buff1, size);
	NbWordToTransfer -= size;

	while (NbWordToTransfer > BUFFER_SIZE)
	{
		if (phase)
		{
			buffer2Stream(outStream, Buff1, BUFFER_SIZE);
			stream2Buffer(inStream, Buff2, BUFFER_SIZE);
		}
		else
		{
			buffer2Stream(outStream, Buff2, BUFFER_SIZE);
			stream2Buffer(inStream, Buff1, BUFFER_SIZE);

		}
		phase = !phase;
		NbWordToTransfer -= BUFFER_SIZE;
	}

	// last transfer from the input stream

	if (phase)
	{
		buffer2Stream(outStream, Buff1, NbWordToTransfer);
		stream2Buffer(inStream, Buff2, BUFFER_SIZE);
	}
	else
	{
		buffer2Stream(outStream, Buff2, NbWordToTransfer);
		stream2Buffer(inStream, Buff1, BUFFER_SIZE);

	}
	phase = !phase;
	// last write to the output stream
	if (phase)
		buffer2Stream(outStream, Buff1, NbWordToTransfer);
	else
		buffer2Stream(outStream, Buff2, NbWordToTransfer);
}

// buffered communication converting from AXI stream to internal stream - 16Bytes

void stream2Buffer(hls::stream<io_stream_16B>& inStream, dt16 Buff[BUFFER_SIZE], unsigned int size)
{
#pragma HLS inline off
	io_stream_16B tmp;

	// read from stream into Buff
	for (unsigned int i1 = 0; i1<size; i1++)
	{
#pragma HLS pipeline
			tmp = inStream.read();  //read buffsize elements and stores them into an external memory bank
			Buff[i1] = tmp.data;
	}
}

void bufferedCommunication(hls::stream<io_stream_16B>& inStream, hls::stream<dt16>& outStream, unsigned int ImgSize, unsigned int NbImages)
{
	bool phase = true;
	unsigned int i,size, writeSize;
	dt16 Buff1[BUFFER_SIZE];
	dt16 Buff2[BUFFER_SIZE];
	unsigned int NbWordToTransfer = ImgSize/(16)*NbImages;
	if (NbWordToTransfer <= BUFFER_SIZE)
		size = NbWordToTransfer;
	else
		size = BUFFER_SIZE;
	stream2Buffer(inStream, Buff1, size);
	NbWordToTransfer -= size;

	while (NbWordToTransfer > BUFFER_SIZE)
	{
		if (phase)
		{
			buffer2Stream(outStream, Buff1, BUFFER_SIZE);
			stream2Buffer(inStream, Buff2, BUFFER_SIZE);
		}
		else
		{
			buffer2Stream(outStream, Buff2, BUFFER_SIZE);
			stream2Buffer(inStream, Buff1, BUFFER_SIZE);

		}
		phase = !phase;
		NbWordToTransfer -= BUFFER_SIZE;
	}

	// last transfer from the input stream

	if (phase)
	{
		buffer2Stream(outStream, Buff1, NbWordToTransfer);
		stream2Buffer(inStream, Buff2, BUFFER_SIZE);
	}
	else
	{
		buffer2Stream(outStream, Buff2, NbWordToTransfer);
		stream2Buffer(inStream, Buff1, BUFFER_SIZE);

	}
	phase = !phase;
	// last write to the output stream
	if (phase)
		buffer2Stream(outStream, Buff1, NbWordToTransfer);
	else
		buffer2Stream(outStream, Buff2, NbWordToTransfer);
}

