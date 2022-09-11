#include "MbsConverter.hh"

// Function to process header words and then the data
void MiniballMbsConverter::ProcessBlock( unsigned long nblock ){
		
	// Get number of 32-bit words and pointer to them
	ndata = ev->GetNData();
	data = ev->GetData();
	
	// Debug events
	//ev->Show(1);

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
	}
	
	// Padding - Nigel
	//while(1) {
	//	if( (data[pos] & 0xFFFF0000) != 0xADD00000 ) break;
	//	pos++;
	//}

	// Now the channel data
	while( pos < ndata ) ProcessFebexData( pos );

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
	while( (data[pos++] & 0xFFFF0000) == 0xADD00000 ) {}
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

		// Get the spec header
		auto specheader = data[pos++];
		
		my_tm_stp = data[pos++] & 0xFFFFFF;
		my_tm_stp <<= 32;
		my_tm_stp |= data[pos++];
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
				short hit_time = val32 & 0x7ff;
				if( hit_time_sign ) hit_time *= -1.0;

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
				febex_data->SetTime( my_tm_stp );
				febex_data->SetSfp( my_sfp_id );
				febex_data->SetBoard( my_board_id );
				febex_data->SetChannel( my_hit_ch_id );
				febex_data->SetFail( 0 );
				febex_data->SetVeto( 0 );
				febex_data->SetPileUp( more_than_1_hit_in_cha );
				
				// Close the data packet and clean up
				FinishFebexData();

			}
		
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
		//std::cout << "nsamples: " << nsamples << std::endl;
		
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

		FebexMWD mwd = cal->DoMWD( my_sfp_id, my_board_id, my_ch_id, febex_data->GetTrace() );
		for( unsigned int i = 0; i < mwd.NumberOfTriggers(); ++i ) {

			flag_febex_trace = true;

			// Make a FebexData item
			febex_data->SetQint( mwd.GetEnergy(i) );
			febex_data->SetTime( my_tm_stp ); // TODO: Timestamp of trace
			febex_data->SetSfp( my_sfp_id );
			febex_data->SetBoard( my_board_id );
			febex_data->SetChannel( my_ch_id );
			febex_data->SetFail( 0 );
			febex_data->SetVeto( 0 );
			febex_data->SetPileUp( 0 );

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
	time_corr  = my_tm_stp;
	time_corr += cal->FebexTime( my_sfp_id, my_board_id, my_ch_id );

	// Check if this is actually just a timestamp or info like event
	flag_febex_info = false;
	if( febex_data->GetSfp()     == set->GetPulserSfp()     &&
	    febex_data->GetBoard()   == set->GetPulserBoard()   &&
	    febex_data->GetChannel() == set->GetPulserChannel() ){
		
		flag_febex_info = true;
		my_info_code = 20; // Pulser is always 20 (defined here)
		hfebex_ext->Fill( ctr_febex_ext, febex_data->GetTime(), 1 );
		ctr_febex_ext++;
		
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
		info_data->SetSfp( febex_data->GetSfp() );
		info_data->SetBoard( febex_data->GetBoard() );
		info_data->SetCode( my_info_code );
		data_packet->SetData( info_data );
		output_tree->Fill();

	}
	
	// Otherwise it is real data, so fill a FEBEX event
	else if( flag_febex_data0 || flag_febex_trace ) {
		
		// Set this data and fill event to tree
		// Also add the time offset when we do this
		febex_data->SetTime( time_corr );
		data_packet->SetData( febex_data );
		output_tree->Fill();

		// Fill histograms
		my_energy = cal->FebexEnergy( febex_data->GetSfp(), febex_data->GetBoard(), febex_data->GetChannel(), febex_data->GetQint() );
		hfebex_cal[febex_data->GetSfp()][febex_data->GetBoard()][febex_data->GetChannel()]->Fill( my_energy );
		
		if( flag_febex_data0 ) hfebex[febex_data->GetSfp()][febex_data->GetBoard()][febex_data->GetChannel()]->Fill( febex_data->GetQint() );
		else if( flag_febex_trace ) hfebex_mwd[febex_data->GetSfp()][febex_data->GetBoard()][febex_data->GetChannel()]->Fill( febex_data->GetQint() );
		
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
							 unsigned long start_block,
							 long end_block ) {
	
	// Uncomment to force only a few blocks - debug
	//end_block = 1000;
	
	// Read the file.
	std::ifstream input_file( input_file_name, std::ios::in|std::ios::binary );
	if( !input_file.is_open() ){
		
		std::cout << "Cannot open " << input_file_name << std::endl;
		return -1;
		
	}

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
	mbs.Open( input_file_name );
	
	// Need to define total number of MBS Events
	unsigned long MBS_EVENTS = 1e6;

	// Loop over all the MBS Events.
	for( unsigned long mbsevt = 0; ; mbsevt++ ){
		
		// Take one block each time and analyze it.
		if( mbsevt % 200 == 0 ) {
			
			// Percent complete
			float percent = (float)(mbsevt+1)*100.0/(float)MBS_EVENTS;
			
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
		ev = mbs.GetNextEvent();
		if( !ev ) break;

		// Check if we are before the start block or after the end block
		if( mbsevt < start_block || ( (long)mbsevt > end_block && end_block > 0 ) )
			continue;

		// Process current block
		ProcessBlock( mbsevt );
		
	} // loop - mbsevt < MBS_EVENTS
	
	// Close the file
	mbs.Close();
	
	// Print stats
	std::cout << std::endl;
	std::cout << "Number of single hits = " << n_single_hits << std::endl;
	std::cout << "Number of double hits = " << n_double_hits << std::endl;

	
	return BLOCKS_NUM;
	
}
