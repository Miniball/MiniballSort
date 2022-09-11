#ifndef __MBSCONVERTER_HH
#define __MBSCONVERTER_HH

// MiniballConverter header
#ifndef __CONVERTER_HH
# include "Converter.hh"
#endif

// MBS Data Format header
#ifndef __MBSFORMAT_HH
# include "MbsFormat.hh"
#endif

class MiniballMbsConverter : public MiniballConverter {

public:
	
	MiniballMbsConverter( std::shared_ptr<MiniballSettings> myset )
		: MiniballConverter( myset ) {
			ev = nullptr;
			data = nullptr;
			n_double_hits = 0;
			n_single_hits = 0;
	};
	~MiniballMbsConverter() {};
	

	int ConvertFile( std::string input_file_name,
					unsigned long start_block = 0,
					long end_block = -1 );

	void ProcessBlock( unsigned long nblock );
	void ProcessFebexData( UInt_t &pos );
	bool GetFebexChanID( unsigned int x );
	void FinishFebexData();


private:

	// MBS Event holder and data pointers
	const MBSEvent *ev;
	const UInt_t *data;
	size_t ndata;
	
	// Counters
	unsigned long n_single_hits;
	unsigned long n_double_hits;

};

#endif
