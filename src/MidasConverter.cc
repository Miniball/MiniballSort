#include "MidasConverter.hh"


// Function to copy the header from a DataSpy, for example
void MiniballMidasConverter::SetBlockHeader( char *input_header ){
	
	// Copy header
	for( unsigned int i = 0; i < HEADER_SIZE; i++ )
		block_header[i] = input_header[i];

	return;
	
}

// Function to process header words
void MiniballMidasConverter::ProcessBlockHeader( unsigned long nblock ){
		
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
void MiniballMidasConverter::SetBlockData( char *input_data ){
	
	// Copy header
	for( UInt_t i = 0; i < MAIN_SIZE; i++ )
		block_data[i] = input_data[i];

	return;
	
}


// Function to process data words
void MiniballMidasConverter::ProcessBlockData( unsigned long nblock ){
	
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

bool MiniballMidasConverter::GetFebexChanID(){
	
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

int MiniballMidasConverter::ProcessTraceData( int pos ){
	
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
	febex_data->SetPileUp( 0 );

	// sample length
	nsamples = word_0 & 0xFFFF; // 16 bits from 0
	
	// Get the samples from the trace
	for( UInt_t j = 0; j < nsamples; j++ ){
		
		// get next word
		ULong64_t sample_packet = GetWord(pos++);
		
		UInt_t block_test = ( sample_packet >> 32 ) & 0x00000000FFFFFFFF;
		unsigned char trace_test = ( sample_packet >> 62 ) & 0x0000000000000003;
		
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

void MiniballMidasConverter::ProcessFebexData(){

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
		febex_data->SetPileUp( false ); // not implemented in the MIDAS firmware

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
		febex_data->SetPileUp( false ); // not implemented in the MIDAS firmware

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
		febex_data->SetPileUp( false ); // not implemented in the MIDAS firmware

	}
	
	// 16-bit integer (short) energy
	if( my_data_id == 0 ) {
		
		febex_data->SetQshort( my_adc_data );
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

void MiniballMidasConverter::FinishFebexData(){
	
	// Timestamp with offset
	unsigned long long time_corr;
	
	// Got all items in fast readout mode or trace only mode
	//if( ( flag_febex_data0 && flag_febex_data1 &&
	//    flag_febex_data2 && flag_febex_data3 ) || flag_febex_trace ){

	// James says (22/08/2022) that we only get the 32-bit integer now
	if( ( flag_febex_data2 && flag_febex_data3 ) || flag_febex_trace ){

		// Add the time offset to this channel
		time_corr  = febex_data->GetTime();
		time_corr += cal->FebexTime( febex_data->GetSfp(), febex_data->GetBoard(), febex_data->GetChannel() );

		// Combine the two halfs of the 32-bit integer point ADC energy
		my_adc_data_int = ( my_adc_data_hsb << 16 ) | ( my_adc_data_lsb & 0xFFFF );
		
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
		else {
			
			// Set this data and fill event to tree
			// Also add the time offset when we do this
			febex_data->SetTime( time_corr );
			febex_data->SetQint( my_adc_data_int );
			data_packet->SetData( febex_data );
			output_tree->Fill();

			// Fill histograms
			my_energy = cal->FebexEnergy( febex_data->GetSfp(), febex_data->GetBoard(), febex_data->GetChannel(), febex_data->GetQint() );
			hfebex[febex_data->GetSfp()][febex_data->GetBoard()][febex_data->GetChannel()]->Fill( febex_data->GetQint() );
			hfebex_cal[febex_data->GetSfp()][febex_data->GetBoard()][febex_data->GetChannel()]->Fill( my_energy );
			
		}

	}

	// missing something
	else if( my_tm_stp != febex_data->GetTime() ) {
		
		std::cout << "Missing something in FEBEX data and new event occured" << std::endl;
		std::cout << " Qshort        = " << flag_febex_data0 << std::endl;
		std::cout << " Qhalf         = " << flag_febex_data1 << std::endl;
		std::cout << " Qint (low)    = " << flag_febex_data2 << std::endl;
		std::cout << " Qint (high)   = " << flag_febex_data3 << std::endl;
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
	info_data->ClearData();

	return;

}

void MiniballMidasConverter::ProcessInfoData(){

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
	//if( my_info_code == set->GetPauseCode() ||
	//    my_info_code == set->GetResumeCode() ) {

		info_data->SetSfp( my_sfp_id );
		info_data->SetBoard( my_board_id );
		info_data->SetTime( my_tm_stp );
		info_data->SetCode( my_info_code );
		data_packet->SetData( info_data );
		output_tree->Fill();
		info_data->Clear();

	//}

	return;
	
}

// Common function called to process data in a block from file or DataSpy
bool MiniballMidasConverter::ProcessCurrentBlock( long nblock ) {
	
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
int MiniballMidasConverter::ConvertBlock( char *input_block, long nblock ) {
	
	// Get the header.
	std::memmove( &block_header, &input_block[0], HEADER_SIZE );
	
	// Get the block
	std::memmove( &block_data, &input_block[HEADER_SIZE], MAIN_SIZE );
	
	// Process the data
	ProcessCurrentBlock( nblock );

	return nblock+1;
	
}

// Function to run the conversion for a single file
int MiniballMidasConverter::ConvertFile( std::string input_file_name,
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

	// Conversion starting
	std::cout << "Converting MIDAS file: " << input_file_name;
	std::cout << " from block " << start_block << std::endl;
	
	// Calculate the size of the file.
	input_file.seekg( 0, input_file.end );
	unsigned long long size_end = input_file.tellg();
	input_file.seekg( 0, input_file.beg );
	unsigned long long size_beg = input_file.tellg();
	unsigned long long FILE_SIZE = size_end - size_beg;
	
	// Calculate the number of blocks in the file.
	unsigned long BLOCKS_NUM = FILE_SIZE / DATA_BLOCK_SIZE;
	
	// a sanity check for file size...
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
