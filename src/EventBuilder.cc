#include "EventBuilder.hh"

EventBuilder::EventBuilder( Settings *myset ){
	
	// First get the settings
	set = myset;
	
	// ------------------------------- //
	// Initialise variables and flags  //
	// ------------------------------- //
	build_window = set->GetEventWindow();

	n_sfp.resize( set->GetNumberOfFebexSfps() );

	for( unsigned int i = 0; i < set->GetNumberOfFebexSfps(); ++i ) {
		
		febex_time_start[i].resize( set->GetNumberOfFebexBoards() );
		febex_time_stop[i].resize( set->GetNumberOfFebexBoards() );
		febex_dead_time[i].resize( set->GetNumberOfFebexBoards() );
		pause_time[i].resize( set->GetNumberOfFebexBoards() );
		resume_time[i].resize( set->GetNumberOfFebexBoards() );
		n_board[i].resize( set->GetNumberOfFebexBoards() );
		n_pause[i].resize( set->GetNumberOfFebexBoards() );
		n_resume[i].resize( set->GetNumberOfFebexBoards() );
		flag_pause[i].resize( set->GetNumberOfFebexBoards() );
		flag_resume[i].resize( set->GetNumberOfFebexBoards() );
		
	}
	
}

void EventBuilder::StartFile(){
	
	// Call for every new file
	// Reset counters etc.
	
	time_prev		= 0;
	pulser_time		= 0;
	pulser_prev		= 0;
	ebis_prev		= 0;
	t1_prev			= 0;

	n_febex_data	= 0;
	n_info_data		= 0;

	n_pulser		= 0;
	n_ebis			= 0;
	n_t1			= 0;

	n_miniball		= 0;
	
	gamma_ctr		= 0;
	cd_ctr			= 0;
	bd_ctr			= 0;

	for( unsigned int i = 0; i < set->GetNumberOfFebexSfps(); ++i ) {

		n_sfp[i] = 0;

		for( unsigned int j = 0; j < set->GetNumberOfFebexBoards(); ++j ) {

			febex_time_start[i][j] = 0;
			febex_time_stop[i][j] = 0;
			febex_dead_time[i][j] = 0;
			pause_time[i][j] = 0;
			resume_time[i][j] = 0;
			n_board[i][j] = 0;
			n_pause[i][j] = 0;
			n_resume[i][j] = 0;
			flag_pause[i][j] = false;
			flag_resume[i][j] = false;

		}
	
	}
	
	// Some flags must be false to start
	event_open = false;
	flag_close_event = false;
	
}

void EventBuilder::SetInputFile( std::vector<std::string> input_file_names ) {
	
	/// Overloaded function for a single file or multiple files
	input_tree = new TChain( "mb_sort" );
	for( unsigned int i = 0; i < input_file_names.size(); i++ ) {
	
		input_tree->Add( input_file_names[i].data() );
		
	}
	input_tree->SetBranchAddress( "data", &in_data );
	
	StartFile();

	return;
	
}

void EventBuilder::SetInputFile( std::string input_file_name ) {
	
	/// Overloaded function for a single file or multiple files
	input_tree = new TChain( "mb_sort" );
	input_tree->Add( input_file_name.data() );
	input_tree->SetBranchAddress( "data", &in_data );

	StartFile();

	return;
	
}

void EventBuilder::SetInputTree( TTree *user_tree ){
	
	// Find the tree and set branch addresses
	input_tree = (TChain*)user_tree;
	input_tree->SetBranchAddress( "data", &in_data );

	StartFile();

	return;
	
}

void EventBuilder::SetOutput( std::string output_file_name ) {

	// These are the branches we need
	write_evts = new MiniballEvts();
	gamma_evt = new GammaRayEvt();
	gamma_ab_evt = new GammaRayAddbackEvt();

	// ------------------------------------------------------------------------ //
	// Create output file and create events tree
	// ------------------------------------------------------------------------ //
	output_file = new TFile( output_file_name.data(), "recreate" );
	output_tree = new TTree( "evt_tree", "evt_tree" );
	output_tree->Branch( "MiniballEvts", "MiniballEvts", &write_evts );

	// Hisograms in separate function
	MakeEventHists();
	
}

void EventBuilder::Initialise(){

	/// This is called at the end of every execution/loop
	
	flag_close_event = false;
	noise_flag = false;
	event_open = false;

	hit_ctr = 0;
	
	mb_en_list.clear();
	mb_en_list.clear();
	mb_ts_list.clear();
	mb_clu_list.clear();
	mb_cry_list.clear();
	mb_seg_list.clear();
	
	write_evts->ClearEvt();
	
	return;
	
}

unsigned long EventBuilder::BuildEvents( unsigned long start_build ) {
	
	/// Function to loop over the sort tree and build array and recoil events

	if( input_tree->LoadTree(0) < 0 ){
		
		std::cout << " Event Building: nothing to do" << std::endl;
		return 0;
		
	}
	
	// Get ready and go
	Initialise();
	n_entries = input_tree->GetEntries();

	std::cout << " Event Building: number of entries in input tree = ";
	std::cout << n_entries << std::endl;
	std::cout << " Start build at event " << start_build << std::endl;

	
	// ------------------------------------------------------------------------ //
	// Main loop over TTree to find events
	// ------------------------------------------------------------------------ //
	for( unsigned long i = start_build; i < n_entries; ++i ) {
		
		// Current event data
		input_tree->GetEntry(i);
		
		// Get the time of the event
		mytime = in_data->GetTime();
				
		// check time stamp monotonically increases!
		if( time_prev > mytime ) {
			
			std::cout << "Out of order event in file ";
			std::cout << input_tree->GetFile()->GetName() << std::endl;
			
		}
			
		// Sort out the timing for the event window
		// but only if it isn't an info event, i.e only for real data
		if( !in_data->IsInfo() ) {
			
			// if this is first datum included in Event
			if( hit_ctr == 0 ) {
				
				time_min	= mytime;
				time_max	= mytime;
				time_first	= mytime;
				
			}
			
			// Check if first event was noise
			// Reset the build window if so
			if( noise_flag && !event_open )
				time_first = mytime;
			
			noise_flag = false; // reset noise flag
			hit_ctr++; // increase counter for bits of data included in this event

			// record time of this event
			time_prev = mytime;
			
			// Update min and max
			if( mytime > time_max ) time_max = mytime;
			else if( mytime < time_min ) time_min = mytime;
			
		}


		// ------------------------------------------ //
		// Find FEBEX data
		// ------------------------------------------ //
		if( in_data->IsFebex() ) {
			
			// Increment event counter
			n_febex_data++;
			
			febex_data = in_data->GetFebexData();
			mysfp = febex_data->GetSfp();
			myboard = febex_data->GetBoard();
			mych = febex_data->GetChannel();
			if( overwrite_cal ) {
				
				myenergy = cal->FebexEnergy( mysfp, myboard, mych,
									febex_data->GetQint() );
				
				if( febex_data->GetQint() > cal->FebexThreshold( mysfp, myboard, mych ) )
					mythres = true;
				else mythres = false;

			}
			
			else {
				
				myenergy = febex_data->GetEnergy();
				mythres = febex_data->IsOverThreshold();

			}
			
			// If it's below threshold do not use as window opener
			if( !mythres ) noise_flag = true;


			// Is it a gamma ray from Miniball?
			else if( set->IsMiniball( mysfp, myboard, mych ) && mythres ) {
				
				// Increment counts and open the event
				n_miniball++;
				event_open = true;
				
				mb_en_list.push_back( myenergy );
				mb_ts_list.push_back( mytime );
				mb_clu_list.push_back( set->GetMiniballCluster( mysfp, myboard, mych) );
				mb_cry_list.push_back( set->GetMiniballCrystal( mysfp, myboard, mych) );
				mb_seg_list.push_back( set->GetMiniballSegment( mysfp, myboard, mych) );
				
			}
			
			// Is it the start event?
			if( febex_time_start.at( mysfp ).at( myboard ) == 0 )
				febex_time_start.at( mysfp ).at( myboard ) = mytime;
			
			// or is it the end event (we don't know so keep updating
			febex_time_stop.at( mysfp ).at( myboard ) = mytime;

		}
		
		
		// ------------------------------------------ //
		// Find info events, like timestamps etc
		// ------------------------------------------ //
		if( in_data->IsInfo() ) {
			
			// Increment event counter
			n_info_data++;
			
			info_data = in_data->GetInfoData();
			
			// Update EBIS time
			if( info_data->GetCode() == set->GetEBISCode() ) {
				
				ebis_time = info_data->GetTime();
				ebis_hz = 1e9 / ( (double)ebis_time - (double)ebis_prev );
				if( ebis_prev != 0 ) ebis_freq->Fill( ebis_time, ebis_hz );
				ebis_prev = ebis_time;
				n_ebis++;
				
			}
		
			// Update T1 time
			if( info_data->GetCode() == set->GetT1Code() ){
				
				t1_time = info_data->GetTime();
				t1_hz = 1e9 / ( (double)t1_time - (double)t1_prev );
				if( t1_prev != 0 ) t1_freq->Fill( t1_time, t1_hz );
				t1_prev = t1_time;
				n_t1++;

			}
			
			// Update pulser time
			if( info_data->GetCode() == set->GetPulserCode() ) {
				
				pulser_time = info_data->GetTime();
				pulser_hz = 1e9 / ( (double)pulser_time - (double)pulser_prev );
				if( pulser_prev != 0 ) pulser_freq->Fill( pulser_time, pulser_hz );

				n_pulser++;

			}

			// Check the pause events for each module
			if( info_data->GetCode() == set->GetPauseCode() ) {
				
				if( info_data->GetSfp() < set->GetNumberOfFebexSfps() &&
				    info_data->GetBoard() < set->GetNumberOfFebexBoards() ) {

					n_pause[info_data->GetSfp()][info_data->GetBoard()]++;
					flag_pause[info_data->GetSfp()][info_data->GetBoard()] = true;
					pause_time[info_data->GetSfp()][info_data->GetBoard()] = info_data->GetTime();
				
				}
				
				else {
					
					std::cerr << "Bad pause event in SFP " << (int)info_data->GetSfp();
					std::cerr << ", board " << (int)info_data->GetBoard() << std::endl;
				
				}

			}
			
			// Check the resume events for each module
			if( info_data->GetCode() == set->GetResumeCode() ) {
				
				if( info_data->GetSfp() < set->GetNumberOfFebexSfps() &&
				    info_data->GetBoard() < set->GetNumberOfFebexBoards() ) {
				
					n_resume[info_data->GetSfp()][info_data->GetBoard()]++;
					flag_resume[info_data->GetSfp()][info_data->GetBoard()] = true;
					resume_time[info_data->GetSfp()][info_data->GetBoard()] = info_data->GetTime();
					
					// Work out the dead time
					febex_dead_time[info_data->GetSfp()][info_data->GetBoard()] += resume_time[info_data->GetSfp()][info_data->GetBoard()];
					febex_dead_time[info_data->GetSfp()][info_data->GetBoard()] -= pause_time[info_data->GetSfp()][info_data->GetBoard()];

					// If we have didn't get the pause, module was stuck at start of run
					if( !flag_pause[info_data->GetSfp()][info_data->GetBoard()] ) {

						std::cout << "SFP " << info_data->GetSfp();
						std::cout << ", board " << info_data->GetBoard();
						std::cout << " was blocked at start of run for ";
						std::cout << (double)resume_time[info_data->GetSfp()][info_data->GetBoard()]/1e9;
						std::cout << " seconds" << std::endl;
					
					}
				
				}
				
				else {
					
					std::cerr << "Bad resume event in SFP " << (int)info_data->GetSfp();
					std::cerr << ", board " << (int)info_data->GetBoard() << std::endl;
				
				}
				
			}

		
			// Now reset previous timestamps
			if( info_data->GetCode() == set->GetPulserCode() )
				pulser_prev = pulser_time;

						
		}

		
		//------------------------------
		//  check if last datum from this event and do some cleanup
		//------------------------------
		
		if( (i+1) == n_entries )
			flag_close_event = true; // set flag to close this event
			
		else {  //check if next entry is beyond time window: close event!

			input_tree->GetEntry(i+1);
						
			time_diff = in_data->GetTime() - time_first;

			// window = time_stamp_first + time_window
			if( time_diff > build_window )
				flag_close_event = true; // set flag to close this event

			// we've gone on to the next file in the chain
			else if( time_diff < 0 )
				flag_close_event = true; // set flag to close this event
				
			// Fill tdiff hist only for real data
			if( !in_data->IsInfo() ) {
				
				tdiff->Fill( time_diff );
				if( !noise_flag )
					tdiff_clean->Fill( time_diff );
			
			}

		} // if next entry beyond time window: close event!
		
		
		//----------------------------
		// if close this event and number of datums in event>0
		//----------------------------
		if( flag_close_event && hit_ctr > 0 ) {

			//----------------------------------
			// Build array events, recoils, etc
			//----------------------------------
			GammaRayFinder();		// perform addback
			ParticleFinder();		// sort out CD n/p correlations

			// ------------------------------------
			// Add timing and fill the ISSEvts tree
			// ------------------------------------
			write_evts->SetEBIS( ebis_time );
			write_evts->SetT1( t1_time );
			if( write_evts->GetGammaRayMultiplicity() ||
			    write_evts->GetGammaRayAddbackMultiplicity() )
				output_tree->Fill();


			//--------------------------------------------------
			// clear values of arrays to store intermediate info
			//--------------------------------------------------
			Initialise();
			
		} // if close event && hit_ctr > 0
		
		if( i % 10000 == 0 || i+1 == n_entries ) {
			
			std::cout << " " << std::setw(8) << std::setprecision(4);
			std::cout << (float)(i+1)*100.0/(float)n_entries << "%    \r";
			std::cout.flush();
			
		}
		
		
	} // End of main loop over TTree to process raw MIDAS data entries (for n_entries)
	
	//--------------------------
	// Clean up
	//--------------------------

	std::cout << "\n EventBuilder finished..." << std::endl;
	std::cout << "  FEBEX data packets = " << n_febex_data << std::endl;
	for( unsigned int i = 0; i < set->GetNumberOfFebexSfps(); ++i ) {
		std::cout << "   SFP " << i << " events = " << n_sfp[i] << std::endl;
		for( unsigned int j = 0; j < set->GetNumberOfFebexBoards(); ++j ) {
			std::cout << "    Board " << i << " events = " << n_board[i][j] << std::endl;
			std::cout << "             pause = " << n_pause[i][j] << std::endl;
			std::cout << "            resume = " << n_resume[i][j] << std::endl;
			std::cout << "         dead time = " << (double)febex_dead_time[i][j]/1e9 << " s" << std::endl;
			std::cout << "         live time = " << (double)(febex_time_stop[i][j]-febex_time_start[i][j])/1e9 << " s" << std::endl;
		}
	}
	std::cout << "  Info data packets = " << n_info_data << std::endl;
	std::cout << "   Pulser events = " << n_pulser << std::endl;
	std::cout << "   EBIS events = " << n_ebis << std::endl;
	std::cout << "   T1 events = " << n_t1 << std::endl;
	std::cout << "  Tree entries = " << output_tree->GetEntries() << std::endl;

	output_file->Write( 0, TObject::kWriteDelete );
	//output_file->Print();
	//output_file->Close();
	
	std::cout << std::endl;
	
	return n_entries;
	
}


void EventBuilder::GammaRayFinder() {
	

	
}


void EventBuilder::ParticleFinder() {
	

	
}


void EventBuilder::MakeEventHists(){
	
	std::string hname, htitle;
	std::string dirname, maindirname, subdirname;
	
	// ----------------- //
	// Timing histograms //
	// ----------------- //
	dirname =  "timing";
	if( !output_file->GetDirectory( dirname.data() ) )
		output_file->mkdir( dirname.data() );
	output_file->cd( dirname.data() );

	tdiff = new TH1F( "tdiff", "Time difference to first trigger;#Delta t [ns]", 1e3, -10, 1e5 );
	tdiff_clean = new TH1F( "tdiff_clean", "Time difference to first trigger without noise;#Delta t [ns]", 1e3, -10, 1e5 );

	pulser_freq = new TProfile( "pulser_freq", "Frequency of pulser in FEBEX DAQ as a function of time;time [ns];f [Hz]", 10.8e4, 0, 10.8e12 );
	ebis_freq = new TProfile( "ebis_freq", "Frequency of EBIS events as a function of time;time [ns];f [Hz]", 10.8e4, 0, 10.8e12 );
	t1_freq = new TProfile( "t1_freq", "Frequency of T1 events (p+ on ISOLDE target) as a function of time;time [ns];f [Hz]", 10.8e4, 0, 10.8e12 );
	
}

void EventBuilder::CleanHists() {

	// Clean up the histograms to save memory for later
	delete tdiff;
	delete tdiff_clean;
	delete pulser_freq;
	delete ebis_freq;
	delete t1_freq;

	return;

}

