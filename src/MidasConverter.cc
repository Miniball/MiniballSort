#include "MidasConverter.hh"


// Function to copy the header from a DataSpy, for example
void MiniballMidasConverter::SetBlockHeader( char *input_header ){
	
	// Copy header
	for( unsigned int i = 0; i < HEADER_SIZE; i++ )
		block_header[i] = input_header[i];

	return;
	
}

// Function to process header words
void MiniballMidasConverter::ProcessBlockHeader( long nblock ){
		
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
void MiniballMidasConverter::ProcessBlockData( long nblock ){
	
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
	
	// If the previous buffer was full and we want to reject the
	// next buffer, because of the readout bugs in September 2023,
	// this is the place to do it. Next buffer might be good again!
	if( buffer_full ){
		
		buffer_full = false;
		buffer_part = true;

	}
	else buffer_part = false;

	
	// Get the size of real data
	UInt_t real_DataLen = header_DataLen/sizeof(ULong64_t);

	// Check if this is a full buffer
	if( real_DataLen == WORD_SIZE ) buffer_full = true;
	
	// Check if we should reject this event
	if( ( buffer_full && set->GetBufferFullRejection() ) ||
        ( buffer_part && set->GetBufferPartRejection() ) ) {
		
		reject_ctr++;
		return;
	
	}
		
	// Process data in the buffer
	NewBuffer();
	for( UInt_t i = 0; i < real_DataLen; i++ ) {
	
		word = GetWord(i);
		word_0 = (word & 0xFFFFFFFF00000000) >> 32;
		word_1 = (word & 0x00000000FFFFFFFF);

		// Data type is highest two bits
		my_type = ( word_0 >> 30 ) & 0x3;
		
		// ADC data - we always assume it comes from FEBEX
		if( my_type == 0x3 ){
			
			ProcessFebexData(nblock);
			if( flag_febex_data0 || !flag_febex_data1 || flag_febex_data2 || flag_febex_data3 )
				FinishFebexData(nblock);

		}
		
		// Information data
		else if( my_type == 0x2 ){
			
			ProcessInfoData(nblock);

		}
		
		// Trace header
		else if( my_type == 0x1 ){
			
			i = ProcessTraceData(i);
			FinishFebexData(nblock);

		}
		
		else {
			
			// output error message!
			std::cerr << "WARNING: WRONG TYPE! word 0: " << std::bitset<32>{word_0};
			std::cerr << ", my_type: " << (int)my_type;
			std::cerr << ", in block: " << nblock << std::endl;

		}

	} // loop - i < header_DataLen
	
	return;

}

bool MiniballMidasConverter::GetFebexChanID(){
	
	// ADCchannelIdent are bits 28:16 of word_0
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
		
		std::cerr << "Bad FEBEX event with SFP = " << (int)my_sfp_id;
		std::cerr << ", board = " << (int)my_board_id;
		std::cerr << ", ch_id = " << (int)my_ch_id;
		std::cerr << ", data_id = " << (int)my_data_id << std::endl;
		std::cerr << "  word_0 = " << std::hex << word_0 << std::dec << " = ";
		std::cerr << std::bitset<32>{word_0} << std::endl;
		std::cerr << "  word_1 = " << std::hex << word_1 << std::dec << " = ";
		std::cerr << std::bitset<32>{word_1} << std::endl;

		return false;

	}

	else return true;
	
}

int MiniballMidasConverter::ProcessTraceData( int pos ){
	
	// Channel ID, etc
	if( !GetFebexChanID() ) return pos;

	// reconstruct time stamp= MSB+LSB
	my_tm_stp_lsb = word_1 & 0x0FFFFFFF;  // 28 bits from 0
	my_tm_stp = ( my_tm_stp_hsb << 48 ) | ( my_tm_stp_msb << 28 ) | my_tm_stp_lsb;

	// FEBEX timestamps are in 10ns precision?
	my_tm_stp = my_tm_stp*10;
	
	// Make a FebexData item
	febex_data->SetTime( my_tm_stp );
	febex_data->SetSfp( my_sfp_id );
	febex_data->SetBoard( my_board_id );
	febex_data->SetChannel( my_ch_id );
	febex_data->SetPileup( false );
	febex_data->SetFlag( my_flagbit );

	// sample length
	nsamples = word_0 & 0xFFFF; // 16 bits from 0
	
	// Get the samples from the trace
	for( UInt_t j = 0; j < nsamples/4; j++ ){
		
		// get next word
		ULong64_t sample_packet = GetWord(++pos);
		
		UInt_t block_test = ( sample_packet >> 32 ) & 0x00000000FFFFFFFF;
		
		// Note from Carl Unsworth in elog:22769 referring to note in edoc504.
		// Basically the trace_test as defined below is not applicable for FEBEX data.
		// We test if the two uppermost bits are 00, but this isn't always the case.
		// Therefore, we are hacking for now to define it as so in case we change our mind
		// then at least the code still exists to go back to some type of test.
		// unsigned char trace_test = ( sample_packet >> 62 ) & 0x0000000000000003;
		unsigned char trace_test = 0;

		if( trace_test == 0 && block_test != 0x5E5E5E5E ){
			
			// Usually the top two bits are zero and we mask them off
			//febex_data->AddSample( ( sample_packet >> 48 ) & 0x0000000000003FFF );
			//febex_data->AddSample( ( sample_packet >> 32 ) & 0x0000000000003FFF );
			//febex_data->AddSample( ( sample_packet >> 16 ) & 0x0000000000003FFF );
			//febex_data->AddSample( sample_packet & 0x0000000000003FFF );
			
			// FEBEX might not be masking the top two bits with zero
			// And the pairs need to be swapped. First sample goes to lower bits
			febex_data->AddSample( ( sample_packet >> 32 ) & 0x000000000000FFFF );
			febex_data->AddSample( ( sample_packet >> 48 ) & 0x000000000000FFFF );
			febex_data->AddSample( sample_packet & 0x000000000000FFFF );
			febex_data->AddSample( ( sample_packet >> 16 ) & 0x000000000000FFFF );

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
	febex_data->SetClipped( mwd.IsClipped() );

	for( unsigned int i = 0; i < mwd.NumberOfTriggers(); ++i )
		hfebex_mwd[my_sfp_id][my_board_id][my_ch_id]->Fill( mwd.GetEnergy(i) );

	flag_febex_trace = true;
	
	return pos;

}

void MiniballMidasConverter::ProcessFebexData( long nblock ){

	// Channel ID, etc
	if( !GetFebexChanID() ) return;
	
	// Febex data format
	my_adc_data = word_0 & 0xFFFF; // 16 bits from 0
	
	// Pileup and other info bits from James' firmware
	my_pileup = (word_0 >> 29) & 0x0001;
	my_clip = (word_0 >> 28) & 0x0001;

	// reconstruct time stamp= MSB+LSB
	my_tm_stp_lsb = word_1 & 0x0FFFFFFF;  // 28 bits from 0
	my_tm_stp = ( my_tm_stp_hsb << 48 ) | ( my_tm_stp_msb << 28 ) | my_tm_stp_lsb; // commented out 09/02/2023
	//my_tm_stp = my_tm_stp_lsb; // matching Vic's format of 09/02/2023
	
	// FEBEX timestamps are in 10ns precision?
	my_tm_stp = my_tm_stp*10;
	
	// If this is the first full data item of the buffer,
	// we need to update the read timestamp to check things are good
	if( first_data[my_sfp_id] ) {
		
		// Update timestamp and set first data
		tm_stp_read[my_sfp_id] = my_tm_stp;
		first_data[my_sfp_id] = false;
		//std::cout << std::hex << my_tm_stp << std::endl;
		
	}
	
	// First of the data items
	if( !flag_febex_data0 && !flag_febex_data1 &&
	    !flag_febex_data2 && !flag_febex_data3 ){
		
		// Make a FebexData item
		febex_data->SetTime( my_tm_stp );
		febex_data->SetSfp( my_sfp_id );
		febex_data->SetBoard( my_board_id );
		febex_data->SetChannel( my_ch_id );
		febex_data->SetPileup( my_pileup );
		febex_data->SetClipped( my_clip );
		febex_data->SetFlag( my_flagbit );

	}
	
	// If we already have all the data items, then the next event has
	// already occured before we found traces. This means that there
	// is not trace data. So set the flag to be true and finish the
	// event with an empty trace.
	// Note 10/02/2022, we don't always get data type 1
	else if( flag_febex_data0 &&
			 flag_febex_data2 && flag_febex_data3 ){
		
		// Finish up the previous event
		FinishFebexData(nblock);

		// Then set the info correctly for this event
		febex_data->SetTime( my_tm_stp );
		febex_data->SetSfp( my_sfp_id );
		febex_data->SetBoard( my_board_id );
		febex_data->SetChannel( my_ch_id );
		febex_data->SetPileup( my_pileup );
		febex_data->SetClipped( my_clip );
		febex_data->SetFlag( my_flagbit );

	}
	
	// If we're in the old readout mode, the next event will be with
	// data_id of 0 again. So we close the event by faking a full set
	else if( flag_febex_data0 && !flag_febex_data2 &&
			 !flag_febex_data3 && my_data_id == 0 ){
	
		// Fake all other data items
		flag_febex_data1 = true;
		flag_febex_data2 = true;
		flag_febex_data3 = true;
	
		// Finish up the previous event
		FinishFebexData(nblock);
	
		// Then set the info correctly for this event
		febex_data->SetTime( my_tm_stp );
		febex_data->SetSfp( my_sfp_id );
		febex_data->SetBoard( my_board_id );
		febex_data->SetChannel( my_ch_id );
		febex_data->SetPileup( my_pileup );
		febex_data->SetClipped( my_clip );
		febex_data->SetFlag( my_flagbit );
	
	}
	
	// 16-bit integer (energy but rebinned)
	if( my_data_id == 0 ) {
		
		my_adc_data = my_adc_data&0xFFFF;
		febex_data->SetQshort( my_adc_data );
		flag_febex_data0 = true;

	}
	
	// 16-bit integer (time difference to previous data group)
	if( my_data_id == 1 ) {
		
		my_tdiff_data = my_adc_data&0xFFFF;
		flag_febex_data1 = true;

	}
	
	// 32-bit integer (low 16 bits of energy)
	if( my_data_id == 2 ) {
		
		my_adc_data_lsb = my_adc_data&0xFFFF;
		flag_febex_data2 = true;

	}
	
	// 32-bit integer (high 16 bits of energy)
	if( my_data_id == 3 ) {
		
		my_adc_data_hsb = my_adc_data&0xFFFF;
		flag_febex_data3 = true;

	}

	return;

}

void MiniballMidasConverter::FinishFebexData( long nblock ){
	
	// Timestamp with offset
	unsigned long long int time_corr;

	// Got all items in fast readout mode or trace only mode
	//if( ( flag_febex_data0 && flag_febex_data1 &&
	//    flag_febex_data2 && flag_febex_data3 ) || flag_febex_trace ){

	// James says (22/08/2022) that we only get the 32-bit integer now
	// Update, Carl reports that the two halves are in data_id = 0, 1, not 2, 3 as documented
	// Update again on 10/02/2022, Vic has made edits to the format and we get 0, 2 and 3 always
	// if( ( flag_febex_data0 && flag_febex_data1 ) || flag_febex_trace ){
	if( ( flag_febex_data0 && flag_febex_data2 && flag_febex_data3 ) || flag_febex_trace ){

		// Add the time offset to this channel
		time_corr  = febex_data->GetTime();
		time_corr += cal->FebexTime( febex_data->GetSfp(), febex_data->GetBoard(), febex_data->GetChannel() );

		// Timestamp checks
		long long int sfp_check		= febex_data->GetTime() - tm_stp_read[febex_data->GetSfp()];
		long long int board_check	= febex_data->GetTime() - tm_stp_febex[febex_data->GetSfp()][febex_data->GetBoard()];
		long long int channel_check	= febex_data->GetTime() - tm_stp_febex_ch[febex_data->GetSfp()][febex_data->GetBoard()][febex_data->GetChannel()];
		
		// Check how we compare to the first timestamp from this buffer (1000 seconds)
		if( !first_data[febex_data->GetSfp()] && ( sfp_check > 1000e9 || sfp_check < -1000e9 ) && nblock > 1 ){
			
			std::cerr << "Timestamp mash in SFP = " << std::dec << (int)febex_data->GetSfp();
			std::cerr << ", board = " << (int)febex_data->GetBoard();
			std::cerr << ", channel = " << (int)febex_data->GetChannel();
			std::cerr << ":\n\t" << std::hex << febex_data->GetTime()/10;
			std::cerr << " ~/~ " << tm_stp_read[febex_data->GetSfp()]/10;
			std::cerr << std::dec << std::endl;
			
			mash_ctr++;
			data_ctr++;
			
		}
		
		// Skip large forwards time jumps in same board, maybe?
		else if( tm_stp_febex[febex_data->GetSfp()][febex_data->GetBoard()] != 0 &&
		   board_check > 240e9 ) {
			
			std::cerr << "Timestamp jump in SFP = " << std::dec << (int)febex_data->GetSfp();
			std::cerr << ", board = " << (int)febex_data->GetBoard();
			std::cerr << ", channel = " << (int)febex_data->GetChannel();
			std::cerr << ":\n\t" << std::hex << febex_data->GetTime()/10;
			std::cerr << " >> " << tm_stp_febex[febex_data->GetSfp()][febex_data->GetBoard()]/10;
			std::cerr << std::dec << std::endl;
			
			// Update board time in case of slow counting channels
			// but don't update the channel time in case it's anomalous
			tm_stp_febex[febex_data->GetSfp()][febex_data->GetBoard()] = febex_data->GetTime();
			
			jump_ctr++;
			data_ctr++;

		}
		
		// What if we jump backwards on an individual channel?
		else if( tm_stp_febex_ch[febex_data->GetSfp()][febex_data->GetBoard()][febex_data->GetChannel()] != 0 &&
				channel_check < 0 ) {
			
			std::cerr << "Timestamp warp in SFP = " << std::dec << (int)febex_data->GetSfp();
			std::cerr << ", board = " << (int)febex_data->GetBoard();
			std::cerr << ", channel = " << (int)febex_data->GetChannel();
			std::cerr << ":\n\t" << std::hex << febex_data->GetTime()/10;
			std::cerr << " < " << tm_stp_febex_ch[febex_data->GetSfp()][febex_data->GetBoard()][febex_data->GetChannel()]/10;
			std::cerr << std::dec << std::endl;
			
			warp_ctr++;
			data_ctr++;

		}
			
		// Otherwise we can carry on because it's good data
		else {
		
			// Combine the two halfs of the 32-bit integer point ADC energy
			my_adc_data_int = ( my_adc_data_hsb << 16 ) | ( my_adc_data_lsb & 0xFFFF );
			febex_data->SetQint( my_adc_data_int );
			
			// Bodge to match Vic's data format on 09/02/2023
			//my_adc_data_int = my_adc_data_lsb & 0xFFFF;
			//febex_data->SetQint( my_adc_data_int );
			
			// Calibrate and set energies
			unsigned int adc_tmp_value;
			if( cal->FebexType( febex_data->GetSfp(), febex_data->GetBoard(), febex_data->GetChannel() ) == "Qshort" )
				adc_tmp_value = febex_data->GetQshort();
			
			else if( cal->FebexType( febex_data->GetSfp(), febex_data->GetBoard(), febex_data->GetChannel() ) == "Qint" )
				adc_tmp_value = febex_data->GetQint();
			
			else {
				
				std::cerr << "Unrecognised data type: " << cal->FebexType( febex_data->GetSfp(), febex_data->GetBoard(), febex_data->GetChannel() );
				std::cerr << " in SFP " << (int)febex_data->GetSfp() << std::endl;
				std::cerr << ", board " << (int)febex_data->GetBoard() << std::endl;
				std::cerr << ", channel " << (int)febex_data->GetChannel() << std::endl;
				std::cout << "\tDefault to Qshort" << std::endl;
				adc_tmp_value = febex_data->GetQshort();
				
			}
			
			my_energy = cal->FebexEnergy( febex_data->GetSfp(), febex_data->GetBoard(), febex_data->GetChannel(), adc_tmp_value );
			febex_data->SetEnergy( my_energy );
			
			// Check if it's over threshold
			if( adc_tmp_value > cal->FebexThreshold( febex_data->GetSfp(), febex_data->GetBoard(), febex_data->GetChannel() ) )
				febex_data->SetThreshold( true );
			else febex_data->SetThreshold( false );
			
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
				
				// Check EBIS time and period
				if( ebis_tm_stp != 0 && ebis_period == 0 &&
				   febex_data->GetTime() > (long long)ebis_tm_stp ){
					
					ebis_period = febex_data->GetTime() - ebis_tm_stp;
					ebis_first = ebis_tm_stp;
					std::cout << "EBIS period detected = " << ebis_period;
					std::cout << " ns" << std::endl;
					
				}
				
				ebis_tm_stp = febex_data->GetTime();
				
			}
			
			else if( febex_data->GetSfp()     == set->GetT1Sfp()     &&
					febex_data->GetBoard()   == set->GetT1Board()   &&
					febex_data->GetChannel() == set->GetT1Channel() ){
				
				flag_febex_info = true;
				my_info_code = 22; // T1 is always 22 (defined here)
				
			}
			
			else if( febex_data->GetSfp()     == set->GetSCSfp()     &&
					febex_data->GetBoard()   == set->GetSCBoard()   &&
					febex_data->GetChannel() == set->GetSCChannel() ){
				
				flag_febex_info = true;
				my_info_code = 23; // SC is always 23 (defined here)
				
			}
			
			else if( febex_data->GetSfp()     == set->GetRILISSfp()     &&
					febex_data->GetBoard()   == set->GetRILISBoard()   &&
					febex_data->GetChannel() == set->GetRILISChannel() ){
				
				flag_febex_info = true;
				my_info_code = 24; // RILIS is always 24 (defined here)
				
			}
			
			// If this is a timestamp, fill an info event
			if( flag_febex_info ) {
				
				info_data->SetTime( time_corr );
				info_data->SetSfp( febex_data->GetSfp() );
				info_data->SetBoard( febex_data->GetBoard() );
				info_data->SetCode( my_info_code );
				std::shared_ptr<MiniballDataPackets> data_packet = std::make_shared<MiniballDataPackets>( info_data );

				// Fill only if we are not doing a source run
				if( !flag_source ) data_vector.emplace_back( data_packet ); // std::vector method for time ordering
				data_ctr++;

			}
			
			// Otherwise it is real data, so fill a FEBEX event
			// but only if we are in an EBIS time window or we want all data
			else if( !flag_ebis || EBISWindow( febex_data->GetTime() ) ) {
				
				// Set this data and fill event to tree
				// Also add the time offset when we do this
				febex_data->SetTime( time_corr );
				std::shared_ptr<MiniballDataPackets> data_packet = std::make_shared<MiniballDataPackets>( febex_data );

				// Fill only if we are not doing a source run
				if( !flag_source ) data_vector.emplace_back( data_packet ); // std::vector method for time ordering
				data_ctr++;
				
			}
			
			// Fill histograms and check clipped etc
			bool fillflag = true;
			if( febex_data->IsClipped() && set->GetClippedRejection() )
				fillflag = false;
			
			if( febex_data->IsPileup() && set->GetPileupRejection() )
				fillflag = false;

			// Only fill if we haven't rejected it, but data still goes to the tree
			if( fillflag ){
				
				hfebex_qshort[febex_data->GetSfp()][febex_data->GetBoard()][febex_data->GetChannel()]->Fill( febex_data->GetQshort() );
				hfebex_qint[febex_data->GetSfp()][febex_data->GetBoard()][febex_data->GetChannel()]->Fill( febex_data->GetQint() );
				hfebex_cal[febex_data->GetSfp()][febex_data->GetBoard()][febex_data->GetChannel()]->Fill( my_energy );
				
			}
			
			// Reset the latest board and channel timestamps
			tm_stp_febex[febex_data->GetSfp()][febex_data->GetBoard()] = time_corr;
			tm_stp_febex_ch[febex_data->GetSfp()][febex_data->GetBoard()][febex_data->GetChannel()] = time_corr;

		} // not being rejecting for jumps or warps
		
	}

	// missing something
	else if( (long long int)my_tm_stp != febex_data->GetTime() ) {
		
		std::cerr << "Missing something in FEBEX data and new event occured" << std::endl;
		std::cerr << " Current timestamp  = " << std::hex << my_tm_stp << std::endl;
		std::cerr << " Previous timestamp = " << std::hex << febex_data->GetTime() << std::endl;
		std::cerr << " Qshort        = " << std::boolalpha << flag_febex_data0 << std::endl;
		std::cerr << " nonsense item = " << std::boolalpha << flag_febex_data1 << std::endl; // missing anyway
		std::cerr << " Qint (low)    = " << std::boolalpha << flag_febex_data2 << std::endl;
		std::cerr << " Qint (high)   = " << std::boolalpha << flag_febex_data3 << std::endl;
		std::cerr << " trace data    = " << std::boolalpha << flag_febex_trace << std::endl;
		std::cerr << std::dec;

	}

	// This is normal, just not finished yet
	else return;
	
	// Fill histograms
	if( febex_data->GetSfp() >= set->GetNumberOfFebexSfps() ||
	   febex_data->GetBoard() >= set->GetNumberOfFebexBoards() ||
	   febex_data->GetChannel() >= set->GetNumberOfFebexChannels() ) {

		std::cerr << "Bad event ID: SFP = " << (int)febex_data->GetSfp();
		std::cerr << ", board = " << (int)febex_data->GetBoard();
		std::cerr << ", channel = " << (int)febex_data->GetChannel() << std::endl;
		std::cerr << "  word_0 = " << std::hex << word_0 << std::dec << " = ";
		std::cerr << std::bitset<32>{word_0} << std::endl;
		std::cerr << "  word_1 = " << std::hex << word_1 << std::dec << " = ";
		std::cerr << std::bitset<32>{word_1} << std::endl;

	}
	
	else {
		
		hfebex_hit[febex_data->GetSfp()][febex_data->GetBoard()]->Fill(
																	   ctr_febex_hit[febex_data->GetSfp()][febex_data->GetBoard()],
																	   febex_data->GetTime(), 1 );
		
		// Count the hit, even if it's bad
		ctr_febex_hit[febex_data->GetSfp()][febex_data->GetBoard()]++;
		
	}
	
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

void MiniballMidasConverter::ProcessInfoData( long nblock ){

	// Module number from MIDAS
	my_sfp_id	= (word_0 >> 28) & 0x0003; // bits 28:29
	my_board_id	= (word_0 >> 24) & 0x000F; // bits 24:27

	// MIDAS info data format
	my_info_field	= word_0 & 0x000FFFFF; // bits 0:19
	my_info_code	= (word_0 >> 20) & 0x0000000F; // bits 20:23
	my_tm_stp_lsb	= word_1 & 0x0FFFFFFF;  // bits 0:27
	
	// Error catching
	if( my_sfp_id >= set->GetNumberOfFebexSfps() ||
	   my_board_id >= set->GetNumberOfFebexBoards() ) {
		
		std::cerr << "Bad info event with SFP = " << (int)my_sfp_id;
		std::cerr << ", board = " << (int)my_board_id;
		std::cerr << ", code = " << (int)my_info_code;
		std::cerr << ", field = " << (int)my_info_field << std::endl;
		std::cerr << " word_0 = " << std::hex << word_0 << std::dec << " = ";
		std::cerr << std::bitset<32>{word_0} << std::endl;
		std::cerr << "  word_1 = " << std::hex << word_1 << std::dec << " = ";
		std::cerr << std::bitset<32>{word_1} << std::endl;
		
		return;
		
	}


	// HSB of FEBEX extended timestamp
	if( my_info_code == set->GetHsbTimestampCode() ) {
		
		// Check that the timestamp isn't weird
		int tmp_tm_stp = my_info_field & 0x0000FFFF;
		if( tmp_tm_stp == 0x0000A5A5 &&
		   ( my_tm_stp_hsb & 0x0000FF00 ) == 0x0000A500 ) {
		
			//my_tm_stp_hsb = tmp_tm_stp;
			my_tm_stp_hsb = 0;
			
		}

		// Huh? What is this?
		if( ( my_info_field & 0x0000FFFF ) > 0 )
			my_flagbit = true;
		else my_flagbit = false;

	}
	
	// MSB of FEBEX extended timestamp
	if( my_info_code == set->GetMsbTimestampCode() ) {
		
		//if(my_tm_stp_hsb>0)
		//	std::cout << my_tm_stp_hsb << " " << my_tm_stp_msb << std::endl;
		
		// Check that the timestamp isn't weird
		int tmp_tm_stp = my_info_field & 0x000FFFFF;
		if( ( tmp_tm_stp & 0x00000FF0 ) == 0x00000A50 &&
		    ( my_tm_stp_msb & 0x0000FF0 ) != 0x0000A40 &&
		    ( my_tm_stp_msb & 0x0000FF0 ) != 0x0000A50 &&
		    ( my_tm_stp_msb & 0x0000FF0 ) != 0x0000A60 &&
		   my_tm_stp_msb > 0 ) {
			
			long tmp_full = ( (long)tmp_tm_stp << 48 ) | ( my_tm_stp_msb << 28 ) | ( my_tm_stp_lsb & 0x0FFFFFFF );
			std::cout << "Corrupt MSB timestamp? 0x" << std::hex;
			std::cout << tmp_tm_stp << std::endl << "\t";
			std::cout << tmp_full << std::dec << std::endl;
			
		}
		
		else {
		
			// In FEBEX this would be the extended timestamp
			my_tm_stp_msb = tmp_tm_stp;
			my_tm_stp = ( my_tm_stp_hsb << 48 ) | ( my_tm_stp_msb << 28 ) | ( my_tm_stp_lsb & 0x0FFFFFFF );

		}

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
	
	// Sync pulse in FEBEX (HSB)
	if( my_info_code == set->GetHsbSyncCode() ) {
		
		sync_tm_stp_hsb = my_info_field & 0x000000FF;
		my_tm_stp = ( sync_tm_stp_hsb << 48 ) | ( sync_tm_stp_msb << 28 ) | ( my_tm_stp_lsb & 0x0FFFFFFF );
		sync_tm_stp = my_tm_stp;
		
		hfebex_sync[my_sfp_id][my_board_id]->Fill( ctr_febex_sync[my_sfp_id][my_board_id], sync_tm_stp, 1 );
		ctr_febex_sync[my_sfp_id][my_board_id]++;

	}
	
	// Sync pulse in FEBEX (MSB)
	if( my_info_code == set->GetMsbSyncCode() ) {
		
		sync_tm_stp_msb = my_info_field & 0x000FFFFF;
		
	}
	
	// Create an info event and fill the tree for external triggers and pause/resume
	if( my_info_code != set->GetMsbTimestampCode() &&
	    my_info_code != set->GetHsbTimestampCode() &&
	    my_info_code != set->GetHsbSyncCode()) {

		info_data->SetSfp( my_sfp_id );
		info_data->SetBoard( my_board_id );
		info_data->SetTime( my_tm_stp*10 );
		info_data->SetCode( my_info_code );
		std::shared_ptr<MiniballDataPackets> data_packet = std::make_shared<MiniballDataPackets>( info_data );

		// Fill only if we are not doing a source run
		// Or comment out if we want to skip them because we're not debugging
		if( !flag_source ) data_vector.emplace_back( data_packet ); // std::vector method for time ordering
		info_data->Clear();

	}

	return;
	
}

// Common function called to process data in a block from file or DataSpy
bool MiniballMidasConverter::ProcessCurrentBlock( long nblock ) {
	
	// Process header.
	ProcessBlockHeader( nblock );

	// Process the main block data until terminator found
	data = (ULong64_t *)(block_data);
	ProcessBlockData( nblock );
			
	// Note 08/11/2023 - This isn't the right thing to do
	// Check once more after going over left overs....
	//if( !flag_terminator ){
	//
	//	std::cout << std::endl << __PRETTY_FUNCTION__ << std::endl;
	//	std::cout << "\tERROR - Terminator sequence not found in data.\n";
	//	return false;
	//
	//}

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
	
	// Reset counters and data vectors to zero for every file
	StartFile();

	// Calculate the size of the file.
	input_file.seekg( 0, input_file.end );
	unsigned long long int size_end = input_file.tellg();
	input_file.seekg( 0, input_file.beg );
	unsigned long long int size_beg = input_file.tellg();
	unsigned long long int FILE_SIZE = size_end - size_beg;
	
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

	// Print the number of warps and jumps
	sslogs << "Number of timestamp jumps  = " << jump_ctr;
	sslogs << " / " << data_ctr << " = ";
	sslogs << 100.0 * (double)jump_ctr / (double)data_ctr;
	sslogs << "\%" << std::endl;
	
	sslogs << "Number of timestamp warps  = " << warp_ctr;
	sslogs << " / " << data_ctr << " = ";
	sslogs << 100.0 * (double)warp_ctr / (double)data_ctr;
	sslogs << "\%" << std::endl;
	
	sslogs << "Number of timestamp mashes = " << mash_ctr;
	sslogs << " / " << data_ctr << " = ";
	sslogs << 100.0 * (double)mash_ctr / (double)data_ctr;
	sslogs << "\%" << std::endl;
	
	sslogs << "Number of rejected blocks  = " << reject_ctr;
	sslogs << " / " << BLOCKS_NUM << " = ";
	sslogs << 100.0 * (double)reject_ctr / (double)BLOCKS_NUM;
	sslogs << "\%" << std::endl;
	
	std::cout << sslogs.str() << std::endl;
	sslogs.str( std::string() ); // clean up
	
	return BLOCKS_NUM;
	
}
