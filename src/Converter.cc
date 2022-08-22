#include "Converter.hh"

Converter::Converter( std::shared_ptr<Settings> myset ) {

	// We need to do initialise, but only after Settings are added
	set = myset;

	my_tm_stp_msb = 0;
	my_tm_stp_hsb = 0;

	ctr_febex_hit.resize( set->GetNumberOfFebexSfps() );
	ctr_febex_pause.resize( set->GetNumberOfFebexSfps() );
	ctr_febex_resume.resize( set->GetNumberOfFebexSfps() );

	ctr_febex_ext = 0;	// pulser trigger

	// Start counters at zero
	for( unsigned int i = 0; i < set->GetNumberOfFebexSfps(); ++i ) {
				
		// Start counters at zero
		for( unsigned int j = 0; j < set->GetNumberOfFebexBoards(); ++j ) {
					
			ctr_febex_hit[i].push_back(0);	// hits on each module
			ctr_febex_pause[i].push_back(0);
			ctr_febex_resume[i].push_back(0);
			
		}

	}
	
	// Default that we do not have a source only run
	flag_source = false;
	
	// No progress bar by default
	_prog_ = false;

}

void Converter::SetOutput( std::string output_file_name ){
	
	// Open output file
	output_file = new TFile( output_file_name.data(), "recreate", "FEBEX raw data file" );

	return;

};


void Converter::MakeTree() {

	// Create Root tree
	const int splitLevel = 2; // don't split branches = 0, full splitting = 99
	const int bufsize = sizeof(FebexData) + sizeof(InfoData);
	output_tree = new TTree( "mb", "mb" );
	data_packet = std::make_unique<DataPackets>();
	output_tree->Branch( "data", "DataPackets", data_packet.get(), bufsize, splitLevel );

	sorted_tree = (TTree*)output_tree->CloneTree(0);
	sorted_tree->SetName("mb_sort");
	sorted_tree->SetTitle( "Time sorted, calibrated Miniball data" );
	sorted_tree->SetDirectory( output_file->GetDirectory("/") );
	output_tree->SetDirectory( output_file->GetDirectory("/") );
	
	output_tree->SetAutoFlush(-10e6);
	sorted_tree->SetAutoFlush(-10e6);

	febex_data = std::make_shared<FebexData>();
	info_data = std::make_shared<InfoData>();
	
	febex_data->ClearData();
	info_data->ClearData();
	
	return;
	
}

void Converter::MakeHists() {
	
	std::string hname, htitle;
	std::string dirname, maindirname, subdirname;
	
	// Make directories - just one DAQ type for now, no sub directories
	// if you do add a directory here, please use a trailing slash
	maindirname = "";
	
	// Resize vectors
	hfebex.resize( set->GetNumberOfFebexSfps() );
	hfebex_cal.resize( set->GetNumberOfFebexSfps() );
	hfebex_mwd.resize( set->GetNumberOfFebexSfps() );
	hfebex_hit.resize( set->GetNumberOfFebexSfps() );
	hfebex_pause.resize( set->GetNumberOfFebexSfps() );
	hfebex_resume.resize( set->GetNumberOfFebexSfps() );

	// Loop over FEBEX SFPs
	for( unsigned int i = 0; i < set->GetNumberOfFebexSfps(); ++i ) {
		
		hfebex[i].resize( set->GetNumberOfFebexBoards() );
		hfebex_cal[i].resize( set->GetNumberOfFebexBoards() );
		hfebex_mwd[i].resize( set->GetNumberOfFebexBoards() );
		hfebex_hit[i].resize( set->GetNumberOfFebexBoards() );
		hfebex_pause[i].resize( set->GetNumberOfFebexBoards() );
		hfebex_resume[i].resize( set->GetNumberOfFebexBoards() );

		// Loop over each FEBEX board
		for( unsigned int j = 0; j < set->GetNumberOfFebexBoards(); ++j ) {
			
			hfebex[i][j].resize( set->GetNumberOfFebexChannels() );
			hfebex_cal[i][j].resize( set->GetNumberOfFebexChannels() );
			hfebex_mwd[i][j].resize( set->GetNumberOfFebexChannels() );

			dirname  = maindirname + "sfp_" + std::to_string(i);
			dirname += "/board_" + std::to_string(j);
			
			if( !output_file->GetDirectory( dirname.data() ) )
				output_file->mkdir( dirname.data() );
			output_file->cd( dirname.data() );

			// Loop over channels of each FEBEX board
			for( unsigned int k = 0; k < set->GetNumberOfFebexChannels(); ++k ) {
				
				// Uncalibrated energy
				hname = "febex_" + std::to_string(i);
				hname += "_" + std::to_string(j);
				hname += "_" + std::to_string(k);
				
				htitle = "Raw FEBEX spectra for SFP " + std::to_string(i);
				htitle += ", board " + std::to_string(j);
				htitle += ", channel " + std::to_string(k);

				htitle += ";Charge value;Counts";
				
				if( output_file->GetListOfKeys()->Contains( hname.data() ) )
					hfebex[i][j][k] = (TH1F*)output_file->Get( hname.data() );
				
				else {
					
					hfebex[i][j][k] = new TH1F( hname.data(), htitle.data(),
											65536, -0.5, 65535.5 );
					
					hfebex[i][j][k]->SetDirectory( output_file->GetDirectory( dirname.data() ) );
					
				}
				
				// Calibrated energy
				hname = "febex_" + std::to_string(i);
				hname += "_" + std::to_string(j);
				hname += "_" + std::to_string(k);
				hname += "_cal";
				
				htitle = "Calibrated FEBEX spectra for SFP " + std::to_string(i);
				htitle += ", board " + std::to_string(j);
				htitle += ", channel " + std::to_string(k);

				htitle += ";Energy (keV);Counts per 0.5 keV";
				
				if( output_file->GetListOfKeys()->Contains( hname.data() ) )
					hfebex_cal[i][j][k] = (TH1F*)output_file->Get( hname.data() );
				
				else {
					
					hfebex_cal[i][j][k] = new TH1F( hname.data(), htitle.data(),
												8000, -0.25, 3999.75 );
					
					hfebex_cal[i][j][k]->SetDirectory( output_file->GetDirectory( dirname.data() ) );
					
				}
				
				// MWD energy
				hname = "febex_" + std::to_string(i);
				hname += "_" + std::to_string(j);
				hname += "_" + std::to_string(k);
				hname += "_mwd";
				
				htitle = "MWD FEBEX spectra for SFP " + std::to_string(i);
				htitle += ", board " + std::to_string(j);
				htitle += ", channel " + std::to_string(k);

				htitle += ";Energy (keV);Counts per 0.5 keV";
				
				if( output_file->GetListOfKeys()->Contains( hname.data() ) )
					hfebex_mwd[i][j][k] = (TH1F*)output_file->Get( hname.data() );
				
				else {
					
					hfebex_mwd[i][j][k] = new TH1F( hname.data(), htitle.data(),
												65536, -0.5, 65535.5 );
					
					hfebex_mwd[i][j][k]->SetDirectory( output_file->GetDirectory( dirname.data() ) );
					
				}
				
			} // k - channel

		// Hit ID vs timestamp
			hname  = "hfebex_hit_" + std::to_string(i);
			hname += "_" + std::to_string(j);
			htitle = "Profile of ts versus hit_id in SFP " + std::to_string(i);
			htitle += ", board " + std::to_string(j);

			if( output_file->GetListOfKeys()->Contains( hname.data() ) )
				hfebex_hit[i][j] = (TProfile*)output_file->Get( hname.data() );
			
			else {
				
				hfebex_hit[i][j] = new TProfile( hname.data(), htitle.data(), 10800, 0., 108000., "" );
				hfebex_hit[i][j]->SetDirectory( output_file->GetDirectory( dirname.data() ) );
				
			}

			// Pause events vs timestamp
			hname = "hfebex_pause_" + std::to_string(i);
			hname += "_" + std::to_string(j);
			htitle = "Profile of ts versus pause events in SFP " + std::to_string(i);
			htitle += ", board " + std::to_string(j);

			if( output_file->GetListOfKeys()->Contains( hname.data() ) )
				hfebex_pause[i][j] = (TProfile*)output_file->Get( hname.data() );
			
			else {
				
				hfebex_pause[i][j] = new TProfile( hname.data(), htitle.data(), 1000, 0., 10000., "" );
				hfebex_pause[i][j]->SetDirectory( output_file->GetDirectory( dirname.data() ) );
				
			}
			
			// Resume events vs timestamp
			hname = "hfebex_resume_" + std::to_string(i);
			hname += "_" + std::to_string(j);
			htitle = "Profile of ts versus resume events in SFP " + std::to_string(i);
			htitle += ", board " + std::to_string(j);

			if( output_file->GetListOfKeys()->Contains( hname.data() ) )
				hfebex_resume[i][j] = (TProfile*)output_file->Get( hname.data() );
			
			else {
				
				hfebex_resume[i][j] = new TProfile( hname.data(), htitle.data(), 1000, 0., 10000., "" );
				hfebex_resume[i][j]->SetDirectory( output_file->GetDirectory( dirname.data() ) );
				
			}
				
		} // j - board
		
	} // i - SFP
	
	// External trigger vs timestamp
	hname = "hfebex_ext_ts";
	htitle = "Profile of external trigger ts versus hit_id";

	if( output_file->GetListOfKeys()->Contains( hname.data() ) )
		hfebex_ext = (TProfile*)output_file->Get( hname.data() );
	
	else {
		
		hfebex_ext = new TProfile( hname.data(), htitle.data(), 10800, 0., 108000., "" );
		hfebex_ext->SetDirectory( output_file->GetDirectory( dirname.data() ) );
		
	}

	return;
	
}

// Function to copy the header from a DataSpy, for example
void Converter::SetBlockHeader( char *input_header ){
	
	// Copy header
	for( unsigned int i = 0; i < HEADER_SIZE; i++ )
		block_header[i] = input_header[i];

	return;
	
}

// Function to process header words
void Converter::ProcessBlockHeader( unsigned long nblock ){
		
	// For each new header, reset the swap mode
	swap = 0;

	// Flags for FEBEX data items
	flag_febex_data0 = false;
	flag_febex_data1 = false;
	flag_febex_data2 = false;
	flag_febex_data3 = false;
	flag_febex_trace = false;

	// Flag when we find the end of the data
	flag_terminator = false;

	// Process header.
	for( UInt_t i = 0; i < 8; i++ )
		header_id[i] = block_header[i];
	
	header_sequence =
	(block_header[8] & 0xFF) << 24 | (block_header[9]& 0xFF) << 16 |
	(block_header[10]& 0xFF) << 8  | (block_header[11]& 0xFF);
	
	header_stream = (block_header[12] & 0xFF) << 8 | (block_header[13]& 0xFF);
	
	header_tape = (block_header[14] & 0xFF) << 8 | (block_header[15]& 0xFF);
	
	header_MyEndian = (block_header[16] & 0xFF) << 8 | (block_header[17]& 0xFF);
	
	header_DataEndian = (block_header[18] & 0xFF) << 8 | (block_header[19]& 0xFF);
	
	header_DataLen =
	(block_header[20] & 0xFF) | (block_header[21]& 0xFF) << 8 |
	(block_header[22] & 0xFF) << 16  | (block_header[23]& 0xFF) << 24 ;
	
	if( std::string(header_id).substr(0,8) != "EBYEDATA" ) {
	
		std::cerr << "Bad header in block " << nblock << std::endl;
		exit(0);
	
	}
	
	return;
	
}


// Function to copy the main data from a DataSpy, for example
void Converter::SetBlockData( char *input_data ){
	
	// Copy header
	for( UInt_t i = 0; i < MAIN_SIZE; i++ )
		block_data[i] = input_data[i];

	return;
	
}


// Function to process data words
void Converter::ProcessBlockData( unsigned long nblock ){
	
	// Get the data in 64-bit words and check endieness and swap if needed
	// Data format here: http://npg.dl.ac.uk/documents/edoc504/edoc504.html
	// Unpack in to two 32-bit words for purposes of data format
		
	// Swap mode is unknown for the first block of data, so let's work it out
	if( (swap & SWAP_KNOWN) == 0 ) {

		// See if we can figure out the swapping - the DataEndian word of the
		// header is 256 if the endianness is correct, otherwise swap endianness
		if( header_DataEndian != 256 ) swap |= SWAP_ENDIAN;
		
		// However, that is not all, the words may also be swapped, so check
		// for that. Bits 31:30 should always be zero in the timestamp word
		for( UInt_t i = 0; i < WORD_SIZE; i++ ) {
			ULong64_t word = (swap & SWAP_ENDIAN) ? Swap64(data[i]) : data[i];
			if( word & 0xC000000000000000LL ) {
				swap |= SWAP_KNOWN;
				break;
			}
			if( word & 0x00000000C0000000LL ) {
				swap |= SWAP_KNOWN;
				swap |= SWAP_WORDS;
				break;
			}
		}
		
	}

	
	// Process all words
	for( UInt_t i = 0; i < WORD_SIZE; i++ ) {
		
		word = GetWord(i);
		word_0 = (word & 0xFFFFFFFF00000000) >> 32;
		word_1 = (word & 0x00000000FFFFFFFF);

		// Check the trailer: reject or keep the block.
		if( ( word_0 & 0xFFFFFFFF ) == 0xFFFFFFFF ||
		    ( word_0 & 0xFFFFFFFF ) == 0x5E5E5E5E ||
		    ( word_1 & 0xFFFFFFFF ) == 0xFFFFFFFF ||
		    ( word_1 & 0xFFFFFFFF ) == 0x5E5E5E5E ){
			
			flag_terminator = true;
			return;
			
		}
		else if( i >= header_DataLen/sizeof(ULong64_t) ){
			
			flag_terminator = true;
			return;

		}
		
			
		// Data type is highest two bits
		my_type = ( word_0 >> 30 ) & 0x3;
		
		// ADC data - we always assume it comes from FEBEX
		if( my_type == 0x3 ){
			
			ProcessFebexData();
			FinishFebexData();

		}
		
		// Information data
		else if( my_type == 0x2 ){
			
			ProcessInfoData();

		}
		
		// Trace header
		else if( my_type == 0x1 ){
			
			i = ProcessTraceData(i);
			FinishFebexData();

		}
		
		else {
			
			// output error message!
			std::cerr << "WARNING: WRONG TYPE! word 0: " << word_0;
			std::cerr << ", my_type: " << my_type << std::endl;
			std::cerr << ", in bloc: " << nblock << std::endl;

		}

	} // loop - i < header_DataLen
	
	return;

}

bool Converter::GetFebexChanID(){
	
	// ADCchannelIdent are bits 27:16 of word_0
	// sfp_id= bit 11:10, board_id= bit 9:6, data_id= bit 5:4, ch_id= bit 3:0
	// data_id: fast mode readout: =0 16 bit integer; =1 16 bit float;
	//				=2 32 bit float (low 16 bits); =3 32 bit float (high 16 bits)
	// 			standard mode readout: 0= 16 bit integer
	unsigned int ADCchanIdent = (word_0 >> 16) & 0x0FFF; // 12 bits from 16
	my_sfp_id = (ADCchanIdent >> 10) & 0x0003; // 2 bits from 10
	my_board_id = (ADCchanIdent >> 6) & 0x000F; // 4 bits from 6
	my_data_id = (ADCchanIdent >> 4) & 0x0003; // 2 bits from 4
	my_ch_id = ADCchanIdent & 0x000F; // 4 bits from 0
	
	// Check things make sense
	if( my_sfp_id >= set->GetNumberOfFebexSfps() ||
	    my_board_id >= set->GetNumberOfFebexBoards() ||
	    my_ch_id >= set->GetNumberOfFebexChannels() ) {
		
		std::cout << "Bad FEBEX event with sfp_id=" << my_sfp_id;
		std::cout << " board_id=" << my_board_id;
		std::cout << " ch_id=" << my_ch_id;
		std::cout << " data_id=" << my_data_id << std::endl;
		return false;

	}

	else return true;
	
}

int Converter::ProcessTraceData( int pos ){
	
	// Channel ID, etc
	if( !GetFebexChanID() ) return pos;

	// reconstruct time stamp= MSB+LSB
	my_tm_stp_lsb = word_1 & 0x0FFFFFFF;  // 28 bits from 0
	my_tm_stp = ( my_tm_stp_msb << 28 ) | my_tm_stp_lsb;

	// Make a FebexData item
	febex_data->SetTime( my_tm_stp );
	febex_data->SetSfp( my_sfp_id );
	febex_data->SetBoard( my_board_id );
	febex_data->SetChannel( my_ch_id );
	febex_data->SetFail( 0 );
	febex_data->SetVeto( 0 );

	// sample length
	nsamples = word_0 & 0xFFFF; // 16 bits from 0
	
	// Get the samples from the trace
	for( UInt_t j = 0; j < nsamples; j++ ){
		
		// get next word
		pos++;
		sample_packet = GetWord(pos);
		
		block_test = ( sample_packet >> 32 ) & 0x00000000FFFFFFFF;
		trace_test = ( sample_packet >> 62 ) & 0x0000000000000003;
		
		if( trace_test == 0 && block_test != 0x5E5E5E5E ){
			
			febex_data->AddSample( ( sample_packet >> 48 ) & 0x0000000000003FFF );
			febex_data->AddSample( ( sample_packet >> 32 ) & 0x0000000000003FFF );
			febex_data->AddSample( ( sample_packet >> 16 ) & 0x0000000000003FFF );
			febex_data->AddSample( sample_packet & 0x0000000000003FFF );
			
		}
		
		else {
			
			//std::cout << "This isn't a trace anymore..." << std::endl;
			//std::cout << "Sample #" << j << " of " << nsamples << std::endl;
			//std::cout << " trace_test = " << (int)trace_test << std::endl;

			pos--;
			break;
			
		}

	}
	
	FebexMWD mwd = cal->DoMWD( my_sfp_id, my_board_id, my_ch_id, febex_data->GetTrace() );
	for( unsigned int i = 0; i < mwd.NumberOfTriggers(); ++i )
		hfebex_mwd[my_sfp_id][my_board_id][my_ch_id]->Fill( mwd.GetEnergy(i) );

	
	flag_febex_trace = true;
	
	return pos;

}

void Converter::ProcessFebexData(){

	// Febex data format
	my_adc_data = word_0 & 0xFFFF; // 16 bits from 0
	
	// Fail and veto bits (not used in FEBEX?)
	my_veto = (word_0 >> 28) & 0x0001;
	my_fail = (word_0 >> 29) & 0x0001;

	// Channel ID, etc
	if( !GetFebexChanID() ) return;
	
	// reconstruct time stamp= MSB+LSB
	my_tm_stp_lsb = word_1 & 0x0FFFFFFF;  // 28 bits from 0
	my_tm_stp = ( my_tm_stp_msb << 28 ) | my_tm_stp_lsb;
	
	// FEBEX timestamps are what precision?
	// Can we multiply here to get to ns
	//my_tm_stp = my_tm_stp*20;
	
	// First of the data items
	if( !flag_febex_data0 && !flag_febex_data1 &&
	    !flag_febex_data2 && !flag_febex_data3 ){
		
		// Make a FebexData item
		febex_data->SetTime( my_tm_stp );
		febex_data->SetSfp( my_sfp_id );
		febex_data->SetBoard( my_board_id );
		febex_data->SetChannel( my_ch_id );
		febex_data->SetFail( my_fail );
		febex_data->SetVeto( my_veto );

	}
	
	// If we already have all the data items, then the next event has
	// already occured before we found traces. This means that there
	// is not trace data. So set the flag to be true and finish the
	// event with an empty trace.
	else if( flag_febex_data0 && flag_febex_data1 &&
			 flag_febex_data2 && flag_febex_data3 ){
		
		// Finish up the previous event
		FinishFebexData();

		// Then set the info correctly for this event
		febex_data->SetTime( my_tm_stp );
		febex_data->SetSfp( my_sfp_id );
		febex_data->SetBoard( my_board_id );
		febex_data->SetChannel( my_ch_id );
		febex_data->SetFail( my_fail );
		febex_data->SetVeto( my_veto );

	}
	
	// If we're in standard readout mode, the next event will be with
	// data_id of 0 again. So we close the event by faking a full set
	else if( flag_febex_data0 && !flag_febex_data1 && my_data_id == 0 ){
		
		// Fake all other data items
		flag_febex_data1 = true;
		flag_febex_data2 = true;
		flag_febex_data3 = true;

		// Finish up the previous event
		FinishFebexData();

		// Then set the info correctly for this event
		febex_data->SetTime( my_tm_stp );
		febex_data->SetSfp( my_sfp_id );
		febex_data->SetBoard( my_board_id );
		febex_data->SetChannel( my_ch_id );
		febex_data->SetFail( my_fail );
		febex_data->SetVeto( my_veto );
		
	}
	
	// 16-bit integer energy
	if( my_data_id == 0 ) {
		
		// Fill histograms
		my_energy = cal->FebexEnergy( my_sfp_id, my_board_id, my_ch_id, my_adc_data );
		hfebex[my_sfp_id][my_board_id][my_ch_id]->Fill( my_adc_data );
		hfebex_cal[my_sfp_id][my_board_id][my_ch_id]->Fill( my_energy );
		
		febex_data->SetQint( my_adc_data );
		febex_data->SetEnergy( my_energy );

		// Check if it's over threshold
		if( my_adc_data > cal->FebexThreshold( my_sfp_id, my_board_id, my_ch_id ) )
			febex_data->SetThreshold( true );
		else febex_data->SetThreshold( false );

		flag_febex_data0 = true;

	}
	
	// 16-bit float
	if( my_data_id == 1 ) {
		
		febex_data->SetQhalf( (Float16_t)(my_adc_data&0xFFFF) );
		flag_febex_data1 = true;

	}

	// 32-bit float (low 16 bits)
	if( my_data_id == 2 ) {
		
		my_adc_data_lsb = my_adc_data&0xFFFF;
		flag_febex_data2 = true;

	}

	// 32-bit float (high 16 bits)
	if( my_data_id == 3 ) {
		
		my_adc_data_hsb = my_adc_data&0xFFFF;
		flag_febex_data3 = true;

	}


	return;

}

void Converter::FinishFebexData(){
	
	// Timestamp with offset
	unsigned long long time_corr;
	
	// Got all items in fast readout mode or trace only mode
	if( ( flag_febex_data0 && flag_febex_data1 &&
	    flag_febex_data2 && flag_febex_data3 ) || flag_febex_trace ){

		// Add the time offset to this channel
		time_corr  = febex_data->GetTime();
		time_corr += cal->FebexTime( febex_data->GetSfp(), febex_data->GetBoard(), febex_data->GetChannel() );

		// Combine the two halfs of the floating point ADC energy
		my_adc_data_float = ( my_adc_data_hsb << 16 ) | ( my_adc_data_lsb & 0xFFFF );
		
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
			info_data->Clear();

		}

		// Otherwise it is real data, so fill a FEBEX event
		else {
			
			// Set this data and fill event to tree
			// Also add the time offset when we do this
			febex_data->SetTime( time_corr );
			febex_data->SetQfloat( my_adc_data_float );
			data_packet->SetData( febex_data );
			output_tree->Fill();
			
		}

	}

	// missing something
	else if( my_tm_stp != febex_data->GetTime() ) {
		
		std::cout << "Missing something in FEBEX data and new event occured" << std::endl;
		std::cout << " Qint          = " << flag_febex_data0 << std::endl;
		std::cout << " Qhalf         = " << flag_febex_data1 << std::endl;
		std::cout << " Qfloat (low)  = " << flag_febex_data2 << std::endl;
		std::cout << " Qfloat (high) = " << flag_febex_data3 << std::endl;
		std::cout << " trace data    = " << flag_febex_trace << std::endl;

	}

	// This is normal, just not finished yet
	else return;
	
	// Fill histograms
	hfebex_hit[febex_data->GetSfp()][febex_data->GetBoard()]->Fill(
		ctr_febex_hit[febex_data->GetSfp()][febex_data->GetBoard()],
		febex_data->GetTime(), 1 );

	// Count the hit, even if it's bad
	ctr_febex_hit[febex_data->GetSfp()][febex_data->GetBoard()]++;
	
	// Assuming it did finish, in a good way or bad, clean up.
	flag_febex_data0 = false;
	flag_febex_data1 = false;
	flag_febex_data2 = false;
	flag_febex_data3 = false;
	flag_febex_trace = false;
	febex_data->ClearData();
	
	return;

}

void Converter::ProcessInfoData(){

	// MIDAS info data format
	my_sfp_id	= (word_0 >> 28) & 0x0003; // bits 28:29
	my_board_id	= (word_0 >> 24) & 0x000F; // bits 24:27

	my_info_field	= word_0 & 0x000FFFFF; // bits 0:19
	my_info_code	= (word_0 >> 20) & 0x0000000F; // bits 20:23
	my_tm_stp_lsb	= word_1 & 0x0FFFFFFF;  // bits 0:27

	// HSB of FEBEX extended timestamp
	if( my_info_code == set->GetTimestampCode() ) {
		
		my_tm_stp_hsb = my_info_field & 0x000FFFFF;

	}
	
	// MSB of FEBEX extended timestamp
	if( my_info_code == set->GetSyncCode() ) {
		
		// In FEBEX this would be the extended timestamp
		my_tm_stp_msb = my_info_field & 0x000FFFFF;
		my_tm_stp = ( my_tm_stp_hsb << 48 ) | ( my_tm_stp_msb << 28 ) | ( my_tm_stp_lsb & 0x0FFFFFFF );

	}
		
	// Pause
    if( my_info_code == set->GetPauseCode() ) {
         
		my_tm_stp_msb = my_info_field & 0x000FFFFF;
		my_tm_stp = ( my_tm_stp_hsb << 48 ) | ( my_tm_stp_msb << 28 ) | ( my_tm_stp_lsb & 0x0FFFFFFF );
		hfebex_pause[my_sfp_id][my_board_id]->Fill( ctr_febex_pause[my_sfp_id][my_board_id], my_tm_stp, 1 );
		ctr_febex_pause[my_sfp_id][my_board_id]++;

    }

	// Resume
	if( my_info_code == set->GetResumeCode() ) {
         
		my_tm_stp_msb = my_info_field & 0x000FFFFF;
		my_tm_stp = ( my_tm_stp_hsb << 48 ) | ( my_tm_stp_msb << 28 ) | ( my_tm_stp_lsb & 0x0FFFFFFF );
		hfebex_resume[my_sfp_id][my_board_id]->Fill( ctr_febex_resume[my_sfp_id][my_board_id], my_tm_stp, 1 );
		ctr_febex_resume[my_sfp_id][my_board_id]++;

    }

	// Create an info event and fill the tree for external triggers and pause/resume
	if( my_info_code == set->GetPauseCode() ||
	    my_info_code == set->GetResumeCode() ) {

		info_data->SetSfp( my_sfp_id );
		info_data->SetBoard( my_board_id );
		info_data->SetTime( my_tm_stp );
		info_data->SetCode( my_info_code );
		data_packet->SetData( info_data );
		output_tree->Fill();
		info_data->Clear();

	}

	return;
	
}

// Common function called to process data in a block from file or DataSpy
bool Converter::ProcessCurrentBlock( int nblock ) {
	
	// Process header.
	ProcessBlockHeader( nblock );

	// Process the main block data until terminator found
	data = (ULong64_t *)(block_data);
	ProcessBlockData( nblock );
			
	// Check once more after going over left overs....
	if( !flag_terminator ){

		std::cout << std::endl << __PRETTY_FUNCTION__ << std::endl;
		std::cout << "\tERROR - Terminator sequence not found in data.\n";
		return false;
		
	}

	return true;

}

// Function to convert a block of data from DataSpy
int Converter::ConvertBlock( char *input_block, int nblock ) {
	
	// Get the header.
	std::memmove( &block_header, &input_block[0], HEADER_SIZE );
	
	// Get the block
	std::memmove( &block_data, &input_block[HEADER_SIZE], MAIN_SIZE );
	
	// Process the data
	ProcessCurrentBlock( nblock );

	return nblock+1;
	
}

// Function to run the conversion for a single file
int Converter::ConvertFile( std::string input_file_name,
							 unsigned long start_block,
							 long end_block ) {
	
	// Read the file.
	std::ifstream input_file( input_file_name, std::ios::in|std::ios::binary );
	if( !input_file.is_open() ){
		
		std::cout << "Cannot open " << input_file_name << std::endl;
		return -1;
		
	}

	// Conversion starting
	std::cout << "Converting file: " << input_file_name;
	std::cout << " from block " << start_block << std::endl;
	
	// Calculate the size of the file.
	input_file.seekg( 0, input_file.end );
	unsigned long long size_end = input_file.tellg();
	input_file.seekg( 0, input_file.beg );
	unsigned long long size_beg = input_file.tellg();
	unsigned long long FILE_SIZE = size_end - size_beg;
	
	// Calculate the number of blocks in the file.
	unsigned long BLOCKS_NUM = FILE_SIZE / DATA_BLOCK_SIZE;
	
	//a sanity check for file size...
	//QQQ: add more strict test?
	if( FILE_SIZE % DATA_BLOCK_SIZE != 0 ){
		
		std::cout << " *WARNING* " << __PRETTY_FUNCTION__;
		std::cout << "\tMissing data blocks?" << std::endl;

	}
	
	sslogs << "\t File size = " << FILE_SIZE << std::endl;
	sslogs << "\tBlock size = " << DATA_BLOCK_SIZE << std::endl;
	sslogs << "\t  N blocks = " << BLOCKS_NUM << std::endl;

	std::cout << sslogs.str() << std::endl;
	sslogs.str( std::string() ); // clean up
	

	// Data format: http://npg.dl.ac.uk/documents/edoc504/edoc504.html
	// The information is split into 2 words of 32 bits (4 byte).
	// We will collect the data in 64 bit words and split later
	
	// Loop over all the blocks.
	for( unsigned long nblock = 0; nblock < BLOCKS_NUM ; nblock++ ){
		
		// Take one block each time and analyze it.
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
		
		
		// Get the header.
		input_file.read( (char*)&block_header, HEADER_SIZE );
		// Get the block
		input_file.read( (char*)&block_data, MAIN_SIZE );


		// Check if we are before the start block or after the end block
		if( nblock < start_block || ( (long)nblock > end_block && end_block > 0 ) )
			continue;


		// Process current block. If it's the end, stop.
		if( !ProcessCurrentBlock( nblock ) ) break;
		
	} // loop - nblock < BLOCKS_NUM
	
	input_file.close();

	return BLOCKS_NUM;
	
}

unsigned long long Converter::SortTree(){
	
	// Reset the sorted tree so it's empty before we start
	sorted_tree->Reset();
	
	// Load the full tree if possible
	output_tree->SetMaxVirtualSize(2e9); // 2GB
	sorted_tree->SetMaxVirtualSize(2e9); // 2GB
	output_tree->LoadBaskets(1e9); 		 // Load 1 GB of data to memory
	
	// Check we have entries and build time-ordered index
	if( output_tree->GetEntries() ){

		std::cout << "Building time-ordered index of events..." << std::endl;
		output_tree->BuildIndex( "data.GetTime()" );

	}
	else return 0;
	
	// Get index and prepare for sorting
	TTreeIndex *att_index = (TTreeIndex*)output_tree->GetTreeIndex();
	unsigned long long nb_idx = att_index->GetN();
	std::cout << " Sorting: size of the sorted index = " << nb_idx << std::endl;

	// Loop on t_raw entries and fill t
	for( unsigned long i = 0; i < nb_idx; ++i ) {
		
		// Clean up old data
		data_packet->ClearData();
		
		// Get time-ordered event index
		unsigned long long idx = att_index->GetIndex()[i];
		
		// Check if the input or output trees are filling
		if( output_tree->MemoryFull(30e6) )
			output_tree->DropBaskets();
		if( sorted_tree->MemoryFull(30e6) )
			sorted_tree->FlushBaskets();
		
		// Get entry from unsorted tree and fill to sorted tree
		output_tree->GetEntry( idx );
		sorted_tree->Fill();

		// Optimise filling tree
		if( i == 100 ) sorted_tree->OptimizeBaskets(30e6);	 // sorted tree basket size max 30 MB

		// Progress bar
		bool update_progress = false;
		if( nb_idx < 200 )
			update_progress = true;
		else if( i % (nb_idx/100) == 0 || i+1 == nb_idx )
			update_progress = true;
		
		if( update_progress ) {
			
			// Percent complete
			float percent = (float)(i+1)*100.0/(float)nb_idx;
			
			// Progress bar in GUI
			if( _prog_ ) {
				
				prog->SetPosition( percent );
				gSystem->ProcessEvents();
				
			}
			
			// Progress bar in terminal
			std::cout << " " << std::setw(6) << std::setprecision(4);
			std::cout << percent << "%    \r";
			std::cout.flush();

		}

	}
	
	// Reset the output tree so it's empty after we've finished
	output_tree->FlushBaskets();
	output_tree->Reset();

	return nb_idx;
	
}
