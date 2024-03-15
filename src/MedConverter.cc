#include "MedConverter.hh"

// Function to process header words and then the data
void MiniballMedConverter::ProcessBlock( unsigned long nblock ){
		
	// Get number of subevents in the data and loop over them
	ndata = ev->GetNumberOfSubEvents();
	for( unsigned int i = 0; i < ndata; i++ ){
		
		// Pointer to this subevent
		mbs_sevt = ev->GetSubEvent(i);
		if( !mbs_sevt ) continue;

		// Check what module it is coming from and direct the sorting
		
		// Normal DGF data
		if( mbs_sevt->GetSubEventType() == MBS_STYPE_CAMAC_DGF_1 ||
		    mbs_sevt->GetSubEventType() == MBS_STYPE_CAMAC_DGF_2 )
			ProcessDgfData();
		
		// Timestamper DGFs
		else if( mbs_sevt->GetSubEventType() == MBS_STYPE_CAMAC_DGF_3 )
			ProcessDgfTimeStamp();

		// DGF Scalers
		else if( mbs_sevt->GetSubEventType() == MBS_STYPE_CAMAC_WO_ID_1 )
			ProcessDgfScaler();

		// Mesytec ADCs
		else if( mbs_sevt->GetSubEventType() == MBS_STYPE_DATA_SHORT ||
				 mbs_sevt->GetSubEventType() == MBS_STYPE_DATA_SHORT ||
				 mbs_sevt->GetSubEventType() == MBS_STYPE_DATA_SHORT )
			ProcessMesytecAdcData();

		// CAEN ADCs
		else if( mbs_sevt->GetSubEventType() == MBS_STYPE_VME_CAEN_V556_1 ||
				 mbs_sevt->GetSubEventType() == MBS_STYPE_VME_CAEN_V556_2 ||
				 mbs_sevt->GetSubEventType() == MBS_STYPE_VME_CAEN_V556_3 )
			ProcessCaenAdcData();
		
		// Pattern unit
		else if( mbs_sevt->GetSubEventType() == MBS_STYPE_VME_SIS_1 ||
				 mbs_sevt->GetSubEventType() == MBS_STYPE_VME_SIS_2 ||
				 mbs_sevt->GetSubEventType() == MBS_STYPE_VME_SIS_3 )
			ProcessPatternUnitData();
		
		// Scaler data
		else if( mbs_sevt->GetSubEventType() == MBS_STYPE_DATA_SHORT )
			ProcessScalerData();
		
		// Otherwise print an error for now (expand more types later)
		else {
			
			std::cerr << "Unrecognised sub-event type: " << std::hex;
			std::cerr <<  mbs_sevt->GetSubEventType() << std::dec << std::endl;
			continue;
			
		}

	}

	// Suppress unused warnings
	(void)nblock;

	return;
	
}

//-----------------------------------------------------------------------------
// Treat a Mesytec ADC data item
void MiniballMedConverter::ProcessMesytecAdcData() {

	// Do nothing with it for now, because I'm lazy
	return;
	
}

//-----------------------------------------------------------------------------
// Treat a CAEN ADC data item
void MiniballMedConverter::ProcessCaenAdcData() {

	// Do nothing with it for now, because I'm lazy
	return;
	
}

//-----------------------------------------------------------------------------
// Treat a pattern unit data item
void MiniballMedConverter::ProcessPatternUnitData() {

	// Do nothing with it for now, because I'm lazy
	return;
	
}

//-----------------------------------------------------------------------------
// Treat a scaler data item
void MiniballMedConverter::ProcessScalerData() {

	// Do nothing with it for now, because I'm lazy
	return;
	
}

//-----------------------------------------------------------------------------
// Treat a DGF scaler data item
void MiniballMedConverter::ProcessDgfScaler() {

	// Do nothing with it for now, because I'm lazy
	return;
	
}


//-----------------------------------------------------------------------------
// Treat a DGF timestamp data item
void MiniballMedConverter::ProcessDgfTimeStamp() {

	// Do nothing with it for now, because I'm lazy
	return;
	
}

//-----------------------------------------------------------------------------
// Treat a DGF data item
void MiniballMedConverter::ProcessDgfData() {
	
	

	return;
	
}


// Function to run the conversion for a single file
int MiniballMedConverter::ConvertFile( std::string input_file_name,
							 unsigned long start_subevt,
							 long end_subevt ) {
	
	// Uncomment to force only a few subevts - debug
	//end_subevt = 1000;
	
	// Read the file.
	std::ifstream input_file( input_file_name, std::ios::in|std::ios::binary );
	if( !input_file.is_open() ){
		
		std::cout << "Cannot open " << input_file_name << std::endl;
		return -1;
		
	}

	// Reset counters to zero for every file
	StartFile();

	// Calculate the size of the file.
	input_file.seekg( 0, input_file.end );
	unsigned long long size_end = input_file.tellg();
	input_file.seekg( 0, input_file.beg );
	unsigned long long size_beg = input_file.tellg();
	unsigned long long FILE_SIZE = size_end - size_beg;

	// Calculate the number of blocks in the file.
	unsigned long BLOCKS_NUM = FILE_SIZE / set->GetBlockSize();
	
	// a sanity check for file size...
	if( FILE_SIZE % set->GetBlockSize() != 0 ){
		
		std::cout << " *WARNING* " << __PRETTY_FUNCTION__;
		std::cout << "\tMissing data blocks?" << std::endl;

	}
	
	sslogs << "\t File size = " << FILE_SIZE << std::endl;
	sslogs << "\tBlock size = " << set->GetBlockSize() << std::endl;
	sslogs << "\t  N blocks = " << BLOCKS_NUM << std::endl;

	std::cout << sslogs.str() << std::endl;
	sslogs.str( std::string() ); // clean up

	// Close the file
	input_file.close();

	// Create an MED data instance and set block/buffer size etc
	std::cout << "Opening file: " << input_file_name << std::endl;
	MBS mbs;
	mbs.SetBufferSize( set->GetBlockSize() );
	mbs.OpenMedFile( input_file_name );

	// Loop over all the MBS Events.
	unsigned long mbsevt = 0, nblock = 0;
	for( mbsevt = 0; ; mbsevt++ ){
		
		// Calculate how many blocks we have used and progress
		nblock = mbs.GetBufferCount();
		if( nblock % 200 == 0 || nblock+1 == BLOCKS_NUM ) {
			
			// Percent complete
			float percent = (float)(nblock+1)*100.0/(float)BLOCKS_NUM;
			
			// Progress bar in GUI
			if( _prog_ ){
				
				prog->SetPosition( percent );
				gSystem->ProcessEvents();

			}
			
			// Progress bar in terminal
			std::cout << " " << std::setw(8) << std::setprecision(4);
			std::cout << percent << "%\r";
			std::cout.flush();

		}
		
		// Get the next event - returns nullptr at the end of the file
		ev = mbs.GetNextMedEvent();
		if( !ev ) break;
		my_event_id = ev->GetEventID();
		
		if( my_event_id == 0 )
			std::cout << "Bad event ID in data" << std::endl;

		// Write the MBS event info
		mbsinfo_packet->SetTime( my_good_tm_stp );
		mbsinfo_packet->SetEventID( my_event_id );
		mbsinfo_tree->Fill();

		// Check if we are before the start sub event or after the end sub events
		if( mbsevt < start_subevt || ( (long)mbsevt > end_subevt && end_subevt > 0 ) )
			continue;

		// Process current block
		ProcessBlock( mbsevt );
		
	} // loop - mbsevt < MBS_EVENTS
	
	// Close the file
	mbs.CloseFile();
	
	// Print stats
	std::cout << std::endl;

	return mbsevt;
	
}
