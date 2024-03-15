#ifndef __MEDCONVERTER_HH
#define __MEDCONVERTER_HH

// MiniballConverter header
#ifndef __CONVERTER_HH
# include "Converter.hh"
#endif

// MBS Data Format header
#ifndef __MBSFORMAT_HH
# include "MbsFormat.hh"
#endif

class MiniballMedConverter : public MiniballConverter {

public:
	
	MiniballMedConverter( std::shared_ptr<MiniballSettings> myset )
		: MiniballConverter( myset ) {
			ev = nullptr;
			data = nullptr;
			mbs_data = false;
			midas_data = false;
			med_data = true;
	};
	~MiniballMedConverter() {};
	

	int ConvertFile( std::string input_file_name,
					unsigned long start_block = 0,
					long end_block = -1 );

	void ProcessBlock( unsigned long nblock );
	void ProcessFebexData( UInt_t &pos );
	bool GetFebexChanID( unsigned int x );
	void FinishFebexData();

	void SetMBSEvent( const MBSEvent *myev ){ ev = myev; };


private:

	// MBS Event holder and data pointers
	const MBSEvent *ev;
	const UInt_t *data;
	size_t ndata;
	MBSSubEvent *mbs_sevt;

};

#endif
