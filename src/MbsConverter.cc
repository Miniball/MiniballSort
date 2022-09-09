#include "MbsConverter.hh"

MiniballMbsConverter::MiniballMbsConverter( std::shared_ptr<MiniballSettings> myset ) {

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

void MiniballMbsConverter::SetOutput( std::string output_file_name ){
	
	// Open output file
	output_file = new TFile( output_file_name.data(), "recreate", "FEBEX raw data file" );

	return;

};


void MiniballMbsConverter::MakeTree() {

	// Create Root tree
	const int splitLevel = 2; // don't split branches = 0, full splitting = 99
	const int bufsize = sizeof(FebexData) + sizeof(InfoData);
	output_tree = new TTree( "mb", "mb" );
	data_packet = std::make_unique<MiniballDataPackets>();
	output_tree->Branch( "data", "MiniballDataPackets", data_packet.get(), bufsize, splitLevel );

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

void MiniballMbsConverter::MakeHists() {
	
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
	output_file->cd( maindirname.data() );
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

// Function to process header words
void MiniballMbsConverter::ProcessBlockHeader( unsigned long nblock ){
		
	// Flags for FEBEX data items
	flag_febex_data0 = false;
	flag_febex_data1 = false;
	flag_febex_data2 = false;
	flag_febex_data3 = false;
	flag_febex_trace = false;

	return;
	
}


// Function to process data words
void MiniballMbsConverter::ProcessBlockData( unsigned long nblock ){
	
		
	
	return;

}

bool MiniballMbsConverter::GetFebexChanID(){
	
	/// TODO: To be updated to get Channel ID data from MBS sub event
	my_sfp_id = 0; // 2 bits from 10
	my_board_id = 0; // 4 bits from 6
	my_ch_id = 0;
	
	// Check things make sense
	if( my_sfp_id >= set->GetNumberOfFebexSfps() ||
	    my_board_id >= set->GetNumberOfFebexBoards() ||
	    my_ch_id >= set->GetNumberOfFebexChannels() ) {
		
		std::cout << "Bad FEBEX event with sfp_id=" << my_sfp_id;
		std::cout << " board_id=" << my_board_id;
		std::cout << " ch_id=" << my_ch_id;
		return false;

	}

	else return true;
	
}

int MiniballMbsConverter::ProcessTraceData( int pos ){
	
	// Channel ID, etc
	if( !GetFebexChanID() ) return pos;

	// reconstruct time stamp= MSB+LSB
	my_tm_stp = ( my_tm_stp_msb << 28 ) | my_tm_stp_lsb;

	// Make a FebexData item
	febex_data->SetTime( my_tm_stp );
	febex_data->SetSfp( my_sfp_id );
	febex_data->SetBoard( my_board_id );
	febex_data->SetChannel( my_ch_id );
	febex_data->SetFail( 0 );
	febex_data->SetVeto( 0 );
	febex_data->SetPileUp( 0 ); // there should be a flag in the MBS data

	// sample length
	nsamples = 0;
	
	// Get the samples from the trace
	for( UInt_t j = 0; j < nsamples; j++ ){
		

	}
	
	FebexMWD mwd = cal->DoMWD( my_sfp_id, my_board_id, my_ch_id, febex_data->GetTrace() );
	for( unsigned int i = 0; i < mwd.NumberOfTriggers(); ++i )
		hfebex_mwd[my_sfp_id][my_board_id][my_ch_id]->Fill( mwd.GetEnergy(i) );

	
	flag_febex_trace = true;
	
	return pos;

}

void MiniballMbsConverter::ProcessFebexData(){

	// TODO: Get sub event data from MBS
	my_adc_data = 0;
	
	// Channel ID, etc
	if( !GetFebexChanID() ) return;
	
	// reconstruct time stamp= MSB+LSB
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
		febex_data->SetFail( 0 );
		febex_data->SetVeto( 0 );
		febex_data->SetPileUp( 0 ); // there should be a flag in the MBS data

	}
	
	// If we already have all the data items, then the next event has
	// already occured before we found traces. This means that there
	// is not trace data. Finish the event with an empty trace.
	else if( flag_febex_data0 && flag_febex_data1 &&
			 flag_febex_data2 && flag_febex_data3 ){
		
		// Finish up the previous event
		FinishFebexData();

		// Then set the info correctly for this event
		febex_data->SetTime( my_tm_stp );
		febex_data->SetSfp( my_sfp_id );
		febex_data->SetBoard( my_board_id );
		febex_data->SetChannel( my_ch_id );
		febex_data->SetFail( 0 );
		febex_data->SetVeto( 0 );
		febex_data->SetPileUp( 0 ); // there should be a flag in the MBS data


	}

	return;

}

void MiniballMbsConverter::FinishFebexData(){
	
	// Timestamp with offset
	unsigned long long time_corr;
	
	// James says (22/08/2022) that we only get the 32-bit integer now
	if( ( flag_febex_data2 && flag_febex_data3 ) || flag_febex_trace ){

		// Add the time offset to this channel
		time_corr  = febex_data->GetTime();
		time_corr += cal->FebexTime( febex_data->GetSfp(), febex_data->GetBoard(), febex_data->GetChannel() );
		
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
			febex_data->SetQint( my_adc_data );
			data_packet->SetData( febex_data );
			output_tree->Fill();
			
			// Fill histograms
			my_energy = cal->FebexEnergy( febex_data->GetSfp(), febex_data->GetBoard(), febex_data->GetChannel(), my_adc_data );
			hfebex[febex_data->GetSfp()][febex_data->GetBoard()][febex_data->GetChannel()]->Fill( my_adc_data );
			hfebex_cal[febex_data->GetSfp()][febex_data->GetBoard()][febex_data->GetChannel()]->Fill( my_energy );
			
		}

	}

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
// Common function called to process data in a block from file or DataSpy
bool MiniballMbsConverter::ProcessCurrentBlock( int nblock ) {
	
	// Process header.
	ProcessBlockHeader( nblock );

	// Process the main block data until terminator found
	ProcessBlockData( nblock );

	return true;

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

	// Conversion starting
	std::cout << "Converting file: " << input_file_name;
	std::cout << " from block " << start_block << std::endl;
	
	// TODO: Use the MBS API to open the file

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
		
		
		// TODO: Use the MBS API to read the file


		// Check if we are before the start block or after the end block
		if( nblock < start_block || ( (long)nblock > end_block && end_block > 0 ) )
			continue;


		// Process current block. If it's the end, stop.
		if( !ProcessCurrentBlock( nblock ) ) break;
		
	} // loop - nblock < BLOCKS_NUM
	
	input_file.close();

	return BLOCKS_NUM;
	
}

unsigned long long MiniballMbsConverter::SortTree(){
	
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
