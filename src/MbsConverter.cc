#include "MbsConverter.hh"

// Function to process header words and then the data
void MiniballMbsConverter::ProcessBlock( unsigned long nblock ){
		
	// Get number of 32-bit words and pointer to them
	ndata = ev->GetNData();
	data = ev->GetData();
	
	// Debug events
	//ev->Show(1);
	
	// Suppress unused warnings
	(void)nblock;

	// Decode header
	UInt_t pos = 3;
	UInt_t nboards = data[pos++];
	unsigned int trace = data[pos++];
	unsigned int filter = data[pos++];
	
	// Debug header
	//std::cout << "nboards: " << nboards << std::endl;
	//std::cout << "trace: " << trace << std::endl;
	//std::cout << "filter: " << filter << std::endl;

	for( UInt_t i = 0; i < 4; i++ ) {
		auto pola = data[pos++];
		(void)pola; // suppress unused warnings
	}
	
	// Padding - Nigel
	//while(1) {
	//	if( (data[pos] & 0xFFFF0000) != 0xADD00000 ) break;
	//	pos++;
	//}

	// Now the channel data
	while( pos < ndata ) ProcessFebexData( pos );
	
	// suppress unused warnings
	(void)nboards;
	(void)trace;
	(void)filter;

	return;
	
}

//-----------------------------------------------------------------------------
// Treat a channel - this should always start with a byte 0x34. If chan is 0 to
// 15, that is a trace for the corresponding channel. If it is 255, this is the
// special channel, where the energies from the FPGA are stored.
void MiniballMbsConverter::ProcessFebexData( UInt_t &pos ) {
	
	flag_febex_data0 = false;
	flag_febex_trace = false;

	// Check for padding - Liam
	while( (data[pos++] & 0xFFFF0000) == 0xADD00000 ) {

		// Make sure we can still read the channel ID and data header after this
		if( pos + 2 >= ndata ) {
			std::cerr << "No data, only padding in this event" << std::endl;
			pos = ndata;
			return;
		}

	}
	pos--;
	
	// Padding - Nik
	//unsigned int first_word = data[pos++];
	//if( ( first_word & 0xFFF00000 ) == 0xADD00000 ) {
	//
	//	//std::cout << "Padding found" << std::endl;
	//	pos += ((first_word & 0xFF00) >> 8 );
	//	pos--;
	//
	//}
	
	// Get channel header
	if( !GetFebexChanID( data[pos++] ) ){
		pos = ndata;
		return;
	}
	
	// Special channel
	if( my_ch_id == 255 ) {
		
		// Get the length in bytes
		auto length = data[pos++];
		nsamples = (length - 16) >> 3; // Length in 64-bit (8-byte) words
		if( pos + 4 + 2*nsamples > ndata ) {
			std::cerr << "Wrong number of data words (" << (int)nsamples+5;
			std::cerr << ") for data remaning data size (";
			std::cerr << ndata-pos << ")" << std::endl;
			pos = ndata;
			return;
		}

		// Get the spec header
		auto specheader = data[pos++];
		
		my_tm_stp = data[pos++] & 0xFFFFFF;
		my_tm_stp <<= 32;
		my_tm_stp |= data[pos++];
		my_tm_stp *= 10;
		//std::cout << "my_tm_stp = " << my_tm_stp << std::endl;

		for( UInt_t i = 0; i < nsamples; i++ ) {
			
			// hit times and energies
			// First 32-bit word of data.
			auto val32 = data[pos++];
			unsigned char my_hit_ch_id = (val32 & 0xf0000000) >> 28;
			unsigned char n_hit_in_cha = (val32 & 0xf000000) >> 24;
			bool more_than_1_hit_in_cha = (val32 & 0x400000) >> 22;
			bool only_one_hit_in_cha = (val32 & 0x100000) >> 20;
			
			if( more_than_1_hit_in_cha ){
				
				//std::cout << "More than one hit found in SFP " << (int)my_sfp_id;
				//std::cout << ", board " << (int)my_board_id;
				//std::cout << ", channel " << (int)my_hit_ch_id << std::endl;
				
				if( only_one_hit_in_cha )
					std::cerr << "Error: One hit and multiple hits flagged" << std::endl;
				
				n_double_hits++;
				
			}
			
			else if( only_one_hit_in_cha ) {
			
				// Hit time negative is before trigger
				// Hit time positive is after trigger
				bool hit_time_sign = (val32 & 0x8000) >> 15;
				my_hit_time = val32 & 0x1fff; // Nik 14/09/2022 confirms mask is 0x1fff by email
				if( hit_time_sign ) my_hit_time *= -1;
				my_hit_time *= 10;
				
				hhit_time->Fill( my_hit_time );

				n_single_hits++;

			}

			// Second 32-bit word of data
			val32 = data[pos++];
			unsigned char my_hit_ch_id2 = (val32 & 0xf0000000) >> 28;
			my_adc_data_int = val32 & 0x7fffff;
			bool my_adc_sign = (val32 & 0x800000) >> 23;
			
			// Make sure that the hit ids match
			if( my_hit_ch_id != my_hit_ch_id2 ){
				std::cerr << "Hit IDs don't match in channels: ";
				std::cerr << (int)my_hit_ch_id << " & ";
				std::cerr << (int)my_hit_ch_id2 << std::endl;
				pos = ndata;
				return;
			}
			
			// Test that the channel number is good
			if( my_hit_ch_id >= set->GetNumberOfFebexChannels() ) {
				std::cerr << "Bad hit ID: " << (int)my_hit_ch_id;
				std::cerr << std::endl;
				pos = ndata;
				return;
			}
			
			// Make an event if we have one trigger
			if( only_one_hit_in_cha ) {
		
				// Don't make events for pileups, as they will have a trace?
				flag_febex_data0 = true;
				
				if( my_adc_sign ) my_adc_data_int *= -1.0;
				
				// Make a FebexData item
				febex_data->SetQint( my_adc_data_int );
				if( set->GetMbsEventMode() )
					febex_data->SetTime( my_hit_time );
				else febex_data->SetTime( my_tm_stp + my_hit_time );
				febex_data->SetEventID( my_event_id );
				febex_data->SetSfp( my_sfp_id );
				febex_data->SetBoard( my_board_id );
				febex_data->SetChannel( my_hit_ch_id );
				febex_data->SetPileup( more_than_1_hit_in_cha );
				
				if( my_sfp_id == 0 ) my_good_tm_stp = my_tm_stp;
				
				// Close the data packet and clean up
				FinishFebexData();

				// suppress unused warnings
				(void)n_hit_in_cha;

			}
		
			// suppress unused warnings
			(void)specheader;

		}
		
		// Spec trailer
		auto spectrailer = data[pos++];
		if( ((spectrailer & 0xff000000) >> 24) != 0xbf ){
			std::cerr << "Invalid special trailer: ";
			std::cerr << ((spectrailer & 0xff000000) >> 24);
			std::cerr << std::endl;
			pos = ndata;
			return;
		}
		
	}
	
	else { // Trace

		// Trace size
		nsamples = (data[pos++]/4) - 2; // In 32-bit words - 2 samples per word?
		if( pos + 2 + nsamples > ndata ) {
			std::cerr << "Wrong number of trace samples (" << (int)nsamples+2;
			std::cerr << ") for data remaning data size (";
			std::cerr << ndata-pos << ")" << std::endl;
			pos = ndata;
			return;
		}

		// Trace header
		unsigned int trace_header = data[pos++];
		if( ((trace_header & 0xff000000) >> 24) != 0xaa ){
			std::cerr << "Invalid trace header: ";
			std::cerr << ((trace_header & 0xff000000) >> 24);
			std::cerr << std::endl;
			pos = ndata;
			return;
		}
		
		bool adc_type = (trace_header & 0x800000) >> 23;
		bool filter_on = (trace_header & 0x80000) >> 19;
		bool filter_mode = (trace_header & 0x40000) >> 18;

		for( UInt_t i = 0; i < nsamples; i++ ) {

			auto sample_packet = data[pos++];

			if( nsamples > 0 ) {

				//std::cout << pos << " : " << sample_packet << std::endl;
				//std::cout << " " << (( sample_packet >> 16 ) & 0x0000FFFF) << std::endl;
				//std::cout << " " << (sample_packet & 0x0000FFFF) << std::endl;

			}
			
			if( filter_on ) {
			
				if( adc_type ) febex_data->AddSample( ( sample_packet >> 16 ) & 0x00003FFF ); // 14 bit
				else febex_data->AddSample( ( sample_packet >> 16 ) & 0x00000FFF ); // 12 bit
				
				bool filter_sign = (sample_packet & 0x800000) >> 23;
				int filter_energy = sample_packet & 0x7fffff;
				if( filter_sign ) filter_energy *= -1.0;
				febex_data->SetQint( filter_energy );
				
			}
			
			else {
				
				if( adc_type ) { // 14 bit
			
					febex_data->AddSample( sample_packet & 0x00003FFF );
					febex_data->AddSample( ( sample_packet >> 16 ) & 0x00003FFF );
			
				}
				
				else { // 12 bit
					
					febex_data->AddSample( sample_packet & 0x00000FFF );
					febex_data->AddSample( ( sample_packet >> 16 ) & 0x00000FFF );

				}
			
			}
			
		}
		
		// suppress unused warnings
		(void)filter_mode;

		FebexMWD mwd = cal->DoMWD( my_sfp_id, my_board_id, my_ch_id, febex_data->GetTrace() );
		for( unsigned int i = 0; i < mwd.NumberOfTriggers(); ++i ) {

			flag_febex_trace = true;

			// Make a FebexData item
			febex_data->SetQint( mwd.GetEnergy(i) );
			if( set->GetMbsEventMode() )
				febex_data->SetTime( mwd.GetCfdTime(i) );
			else febex_data->SetTime( my_tm_stp + mwd.GetCfdTime(i) );
			febex_data->SetSfp( my_sfp_id );
			febex_data->SetBoard( my_board_id );
			febex_data->SetChannel( my_ch_id );
			if( mwd.NumberOfTriggers() > 1 )
				febex_data->SetPileup( true );
			else febex_data->SetPileup( false );


			// Close the data packet and clean up
			FinishFebexData();
			
		}

		// Trace trailer
		auto tracetrailer = data[pos++];
		if( ((tracetrailer & 0xff000000) >> 24) != 0xbb ){
			std::cerr << "Invalid trace trailer: ";
			std::cerr << ((tracetrailer & 0xff000000) >> 24);
			std::cerr << std::endl;
			pos = ndata;
			return;
		}

	}

	return;
	
}


bool MiniballMbsConverter::GetFebexChanID( unsigned int x ){
	
	// Decode the channel ID
	my_tag_id = (x & 0xFF);
	my_trig_id = (x & 0xF00) >> 8;
	my_sfp_id = (x & 0xF000) >> 12;
	my_board_id = (x & 0xFF0000) >> 16;
	my_ch_id = (x & 0xFF000000) >> 24;

	// Debug channel ID
	//std::cout << "channel header: " << (unsigned int)x << std::endl;
	//std::cout << "my_tag_id: " << (unsigned int)my_tag_id << std::endl;
	//std::cout << "my_trig_id: " << (unsigned int)my_trig_id << std::endl;
	//std::cout << "my_sfp_id: " << (unsigned int)my_sfp_id << std::endl;
	//std::cout << "my_board_id: " << (unsigned int)my_board_id << std::endl;
	//std::cout << "my_ch_id: " << (unsigned int)my_ch_id << std::endl;

	// Make sure it is valid
	if( my_tag_id != 0x34 ) {
		
		std::cerr << "Invalid channel header: ";
		std::cerr << (int)my_tag_id << std::endl;
		return false;
		
	}
	
	// Check things make sense
	if( my_sfp_id >= set->GetNumberOfFebexSfps() ||
	    my_board_id >= set->GetNumberOfFebexBoards() ||
	    ( my_ch_id >= set->GetNumberOfFebexChannels()
		 && my_ch_id != 255 ) ) {
		
		std::cerr << "Bad FEBEX event with sfp_id=" << (unsigned int)my_sfp_id;
		std::cerr << " board_id=" << (unsigned int)my_board_id;
		std::cerr << " ch_id=" << (unsigned int)my_ch_id << std::endl;
		return false;

	}

	else return true;
	
}
	
void MiniballMbsConverter::FinishFebexData(){
	
	// Timestamp with offset
	unsigned long long time_corr;
	time_corr  = febex_data->GetTime();
	time_corr += cal->FebexTime( febex_data->GetSfp(), febex_data->GetBoard(), febex_data->GetChannel() );

	// Check if this is actually just a timestamp or info like event
	flag_febex_info = false;
	if( set->IsPulser( febex_data->GetSfp(), febex_data->GetBoard(), febex_data->GetChannel() ) ) {

		flag_febex_info = true;
		unsigned int pulserID = set->GetPulser( febex_data->GetSfp(), febex_data->GetBoard(), febex_data->GetChannel() );
		my_info_code = set->GetPulserCode() + pulserID;
		
	}
	
	else if( febex_data->GetSfp()     == set->GetEBISSfp()     &&
			 febex_data->GetBoard()   == set->GetEBISBoard()   &&
			 febex_data->GetChannel() == set->GetEBISChannel() ){
		
		flag_febex_info = true;
		my_info_code = 21; // EBIS is always 21 (defined here)
		
	}
	
	else if( febex_data->GetSfp()     == set->GetT1Sfp()     &&
			 febex_data->GetBoard()   == set->GetT1Board()   &&
			 febex_data->GetChannel() == set->GetT1Channel() ){
		
		flag_febex_info = true;
		my_info_code = 22; // T1 is always 22 (defined here)
		
	}
	
	// If this is a timestamp, fill an info event
	if( flag_febex_info ) {
		
		info_data->SetTime( time_corr );
		info_data->SetEventID( febex_data->GetEventID() );
		info_data->SetSfp( febex_data->GetSfp() );
		info_data->SetBoard( febex_data->GetBoard() );
		info_data->SetCode( my_info_code );
		data_packet->SetData( info_data );
		output_tree->Fill();

	}
	
	// Otherwise it is real data, so fill a FEBEX event
	else if( flag_febex_data0 ) {
		
		// Calibrate
		my_energy = cal->FebexEnergy( febex_data->GetSfp(), febex_data->GetBoard(), febex_data->GetChannel(), febex_data->GetQint() );
		febex_data->SetEnergy( my_energy );
		if( febex_data->GetQint() > cal->FebexThreshold( febex_data->GetSfp(), febex_data->GetBoard(), febex_data->GetChannel() ) )
			febex_data->SetThreshold( true );
		else
			febex_data->SetThreshold( false );
		
		// Fill histograms
		hfebex_cal[febex_data->GetSfp()][febex_data->GetBoard()][febex_data->GetChannel()]->Fill( my_energy );
		hfebex_qint[febex_data->GetSfp()][febex_data->GetBoard()][febex_data->GetChannel()]->Fill( febex_data->GetQint() );
		
		// Set this data and fill event to tree
		// Also add the time offset when we do this
		febex_data->SetTime( time_corr );
		data_packet->SetData( febex_data );
		output_tree->Fill();

	}
	
	else if( flag_febex_trace ) {
		
		// TODO: Find a safe way to put these in the event stream (requires separate energy calibration)
		hfebex_mwd[febex_data->GetSfp()][febex_data->GetBoard()][febex_data->GetChannel()]->Fill( febex_data->GetQint() );
		return;

	}


	// Fill histograms
	hfebex_hit[febex_data->GetSfp()][febex_data->GetBoard()]->Fill(
		ctr_febex_hit[febex_data->GetSfp()][febex_data->GetBoard()],
		febex_data->GetTime(), 1 );

	// Count the hit, even if it's bad
	ctr_febex_hit[febex_data->GetSfp()][febex_data->GetBoard()]++;
	
	// Clean up.
	data_packet->ClearData();
	febex_data->ClearData();
	info_data->ClearData();

	return;

}

// Function to run the conversion for a single file
int MiniballMbsConverter::ConvertFile( std::string input_file_name,
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

	// Create an MBS data instance and set block/buffer size etc
	std::cout << "Opening file: " << input_file_name << std::endl;
	MBS mbs;
	mbs.SetBufferSize( set->GetBlockSize() );
	mbs.OpenLmdFile( input_file_name );

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
		ev = mbs.GetNextLmdEvent();
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
	std::cout << "Number of single hits = " << n_single_hits << std::endl;
	std::cout << "Number of double hits = " << n_double_hits << std::endl;

	return mbsevt;
	
}
