#ifndef __MEDCONVERTER_HH
#define __MEDCONVERTER_HH

// MiniballConverter header
#ifndef __CONVERTER_HH
# include "Converter.hh"
#endif

// MBS Defines header
#ifndef __MBSDEFINES_HH
# include "MbsDefines.hh"
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
			mbs_sevt = nullptr;
			mbs_data = false;
			midas_data = false;
			med_data = true;
	};
	~MiniballMedConverter() {};
	

	int ConvertFile( std::string input_file_name,
					unsigned long start_block = 0,
					long end_block = -1 );

	void ProcessEvent( unsigned long nblock );
	void ProcessMesytecAdcData();
	void ProcessCaenAdcData();
	void ProcessDgfData();
	void ProcessDgfScaler();
	void ProcessPatternUnitData();
	void ProcessScalerData();
	void ClearDataBuffer();

	void SetMBSEvent( const MBSEvent *myev ){ ev = myev; };


private:

	// MBS Event holder and data pointers
	const MBSEvent *ev;
	const MBSSubEvent *mbs_sevt;
	size_t ndata;
	
	// Need a buffer before filling the tree
	std::vector<MiniballDataPackets> output_buffer;

};

#endif
