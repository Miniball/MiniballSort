#ifndef __MIDASCONVERTER_HH
#define __MIDASCONVERTER_HH

// MiniballConverter header
#ifndef __CONVERTER_HH
# include "Converter.hh"
#endif


class MiniballMidasConverter : public MiniballConverter {

public:
	
	MiniballMidasConverter( std::shared_ptr<MiniballSettings> myset )
		: MiniballConverter( myset ) {};
	~MiniballMidasConverter() {};

	int ConvertFile( std::string input_file_name,
					unsigned long start_block = 0,
					long end_block = -1 );
	int ConvertBlock( char *input_block, long nblock );

	bool ProcessCurrentBlock( long nblock );

	void SetBlockHeader( char *input_header );
	void ProcessBlockHeader( unsigned long nblock );

	void SetBlockData( char *input_data );
	void ProcessBlockData( unsigned long nblock );

	bool GetFebexChanID();
	int  ProcessTraceData( int pos );
	void ProcessFebexData();
	void FinishFebexData();
	void ProcessInfoData();



private:

	// enumeration for swapping modes
	enum swap_t {
		SWAP_KNOWN  = 1,  // We know the swapping mode already
		SWAP_WORDS  = 2,  // We need to swap pairs of 32-bit words
		SWAP_ENDIAN = 4   // We need to swap endianness
	};
	Int_t swap;

	// Swap endianness of a 32-bit integer 0x01234567 -> 0x67452301
	inline UInt_t Swap32(UInt_t datum) {
		return(((datum & 0xFF000000) >> 24) |
			   ((datum & 0x00FF0000) >>  8) |
			   ((datum & 0x0000FF00) <<  8) |
			   ((datum & 0x000000FF) << 24));
	};
	
	// Swap the two halves of a 64-bit integer 0x0123456789ABCDEF ->
	// 0x89ABCDEF01234567
	inline ULong64_t SwapWords(ULong64_t datum) {
		return(((datum & 0xFFFFFFFF00000000LL) >> 32) |
			   ((datum & 0x00000000FFFFFFFFLL) << 32));
	};
	
	// Swap endianness of a 64-bit integer 0x0123456789ABCDEF ->
	// 0xEFCDAB8967452301
	inline ULong64_t Swap64(ULong64_t datum) {
		return(((datum & 0xFF00000000000000LL) >> 56) |
			   ((datum & 0x00FF000000000000LL) >> 40) |
			   ((datum & 0x0000FF0000000000LL) >> 24) |
			   ((datum & 0x000000FF00000000LL) >>  8) |
			   ((datum & 0x00000000FF000000LL) <<  8) |
			   ((datum & 0x0000000000FF0000LL) << 24) |
			   ((datum & 0x000000000000FF00LL) << 40) |
			   ((datum & 0x00000000000000FFLL) << 56));
	};
	
	// Get nth word
	inline ULong64_t GetWord( UInt_t n = 0 ){

		// If word number is out of range, return zero
		if( n >= WORD_SIZE ) return(0);

		// Perform byte swapping according to swap mode
		ULong64_t result = data[n];
		if (swap & SWAP_ENDIAN) result = Swap64(result);
		if (swap & SWAP_WORDS)  result = SwapWords(result);
		return(result);
		
	};

	
	// Set the size of the block and its components.
	static const int HEADER_SIZE = 24; // Size of header in bytes
	static const int DATA_BLOCK_SIZE = 0x10000; // Block size for FEBEX data = 64 kB?
	static const int MAIN_SIZE = DATA_BLOCK_SIZE - HEADER_SIZE;
	static const int WORD_SIZE = MAIN_SIZE / sizeof(ULong64_t);

	// Set the arrays for the block components.
	char block_header[HEADER_SIZE];
	char block_data[MAIN_SIZE];
	
	// Data words - 1 word of 64 bits (8 bytes)
	ULong64_t word;
	
	// Data words - 2 words of 32 bits (4 byte).
	UInt_t word_0;
	UInt_t word_1;
	
	// Pointer to the data words
	ULong64_t *data;
	
	// End of data in  a block looks like:
	// word_0 = 0xFFFFFFFF, word_1 = 0xFFFFFFFF.
	// This flag is set true when we hit that point
	bool flag_terminator;

	// Raw data variables
	Char_t header_id[8]; // 8 byte. Must be this string 'EBYEDATA'.
	UInt_t header_sequence; // 4 byte.
	UShort_t header_stream; // 2 byte.
	UShort_t header_tape; // 2 byte.
	UShort_t header_MyEndian; // 2 byte. If 1 then correct endianess.
	UShort_t header_DataEndian; // 2 byte.
	UInt_t header_DataLen; // 4 byte.
	
};

#endif
