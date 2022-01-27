#include "EventBuilder.hh"

EventBuilder::EventBuilder( std::shared_ptr<Settings> myset ){
	
	// First get the settings
	set = myset;
	
	// ------------------------------- //
	// Initialise variables and flags  //
	// ------------------------------- //
	build_window = set->GetEventWindow();

	n_sfp.resize( set->GetNumberOfFebexSfps() );

	febex_time_start.resize( set->GetNumberOfFebexSfps() );
	febex_time_stop.resize( set->GetNumberOfFebexSfps() );
	febex_dead_time.resize( set->GetNumberOfFebexSfps() );
	pause_time.resize( set->GetNumberOfFebexSfps() );
	resume_time.resize( set->GetNumberOfFebexSfps() );
	n_board.resize( set->GetNumberOfFebexSfps() );
	n_pause.resize( set->GetNumberOfFebexSfps() );
	n_resume.resize( set->GetNumberOfFebexSfps() );
	flag_pause.resize( set->GetNumberOfFebexSfps() );
	flag_resume.resize( set->GetNumberOfFebexSfps() );

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
	
	// Progress bar starts as false
	_prog_ = false;
	
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
	n_cd			= 0;

	gamma_ctr		= 0;
	gamma_ab_ctr	= 0;
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

void EventBuilder::SetInputFile( std::string input_file_name ) {
	
	/// Overloaded function for a single file or multiple files
	input_file = new TFile( input_file_name.data(), "read" );
	SetInputTree( (TTree*)input_file->Get("mb_sort") );
	StartFile();

	return;
	
}

void EventBuilder::SetInputTree( TTree *user_tree ){
	
	// Find the tree and set branch addresses
	input_tree = user_tree;
	input_tree->SetBranchAddress( "data", &in_data );

	StartFile();

	return;
	
}

void EventBuilder::SetOutput( std::string output_file_name ) {

	// These are the branches we need
	write_evts = std::make_unique<MiniballEvts>();
	gamma_evt = std::make_shared<GammaRayEvt>();
	gamma_ab_evt = std::make_shared<GammaRayAddbackEvt>();
	particle_evt = std::make_shared<ParticleEvt>();

	// ------------------------------------------------------------------------ //
	// Create output file and create events tree
	// ------------------------------------------------------------------------ //
	output_file = new TFile( output_file_name.data(), "recreate" );
	output_tree = new TTree( "evt_tree", "evt_tree" );
	output_tree->Branch( "MiniballEvts", "MiniballEvts", write_evts.get() );

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
	mb_ts_list.clear();
	mb_clu_list.clear();
	mb_cry_list.clear();
	mb_seg_list.clear();
	
	std::vector<float>().swap(mb_en_list);
	std::vector<unsigned long long>().swap(mb_ts_list);
	std::vector<unsigned char>().swap(mb_clu_list);
	std::vector<unsigned char>().swap(mb_cry_list);
	std::vector<unsigned char>().swap(mb_seg_list);
	
	cd_en_list.clear();
	cd_ts_list.clear();
	cd_det_list.clear();
	cd_sec_list.clear();
	cd_side_list.clear();
	cd_strip_list.clear();
	
	std::vector<float>().swap(cd_en_list);
	std::vector<unsigned long long>().swap(cd_ts_list);
	std::vector<unsigned char>().swap(cd_det_list);
	std::vector<unsigned char>().swap(cd_sec_list);
	std::vector<unsigned char>().swap(cd_side_list);
	std::vector<unsigned char>().swap(cd_strip_list);
	
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
			std::cout << input_tree->GetName() << std::endl;
			
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
				mb_clu_list.push_back( set->GetMiniballCluster( mysfp, myboard, mych ) );
				mb_cry_list.push_back( set->GetMiniballCrystal( mysfp, myboard, mych ) );
				mb_seg_list.push_back( set->GetMiniballSegment( mysfp, myboard, mych ) );
				
			}
			
			// Is it a partile from the CD?
			else if( set->IsCD( mysfp, myboard, mych ) && mythres ) {
				
				// Increment counts and open the event
				n_cd++;
				event_open = true;
				
				cd_en_list.push_back( myenergy );
				cd_ts_list.push_back( mytime );
				cd_det_list.push_back( set->GetCDDetector( mysfp, myboard, mych ) );
				cd_sec_list.push_back( set->GetCDSector( mysfp, myboard, mych ) );
				cd_side_list.push_back( set->GetCDSide( mysfp, myboard, mych ) );
				cd_strip_list.push_back( set->GetCDStrip( mysfp, myboard, mych ) );
				
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
		
		if( i % (n_entries/100) == 0 || i+1 == n_entries ) {
			
			// Percent complete
			float percent = (float)(i+1)*100.0/(float)n_entries;

			// Progress bar in GUI
			if( _prog_ ) prog->SetPosition( percent );

			// Progress bar in terminal
			std::cout << " " << std::setw(6) << std::setprecision(4);
			std::cout << percent << "%    \r";
			std::cout.flush();
			gSystem->ProcessEvents();

		}		
		
	} // End of main loop over TTree to process raw MIDAS data entries (for n_entries)
	
	//--------------------------
	// Clean up
	//--------------------------

	std::cout << "\n EventBuilder finished..." << std::endl;
	std::cout << "  FEBEX data packets = " << n_febex_data << std::endl;
	//for( unsigned int i = 0; i < set->GetNumberOfFebexSfps(); ++i ) {
	//	std::cout << "   SFP " << i << " events = " << n_sfp[i] << std::endl;
	//	for( unsigned int j = 0; j < set->GetNumberOfFebexBoards(); ++j ) {
	//		std::cout << "    Board " << j << " events = " << n_board[i][j] << std::endl;
	//		std::cout << "             pause = " << n_pause[i][j] << std::endl;
	//		std::cout << "            resume = " << n_resume[i][j] << std::endl;
	//		std::cout << "         dead time = " << (double)febex_dead_time[i][j]/1e9 << " s" << std::endl;
	//		std::cout << "         live time = " << (double)(febex_time_stop[i][j]-febex_time_start[i][j])/1e9 << " s" << std::endl;
	//	}
	//}
	std::cout << "  Info data packets = " << n_info_data << std::endl;
	std::cout << "   Pulser events = " << n_pulser << std::endl;
	std::cout << "   EBIS events = " << n_ebis << std::endl;
	std::cout << "   T1 events = " << n_t1 << std::endl;
	std::cout << "  Tree entries = " << output_tree->GetEntries() << std::endl;
	std::cout << "   Miniball events = " << n_miniball << std::endl;
	std::cout << "    Gamma singles events = " << gamma_ctr << std::endl;
	std::cout << "    Gamma addback events = " << gamma_ab_ctr << std::endl;
	std::cout << "   CD detector events = " << n_cd << std::endl;
	std::cout << "    Particle events = " << cd_ctr << std::endl;
	std::cout << "   Beam dump events = " << bd_ctr << std::endl;


	output_file->Write( 0, TObject::kWriteDelete );



	//output_file->Print();



	//output_file->Close();
	
	std::cout << std::endl;
	
	return n_entries;
	
}


void EventBuilder::GammaRayFinder() {
	
	// Temporary variables for addback
	unsigned long long MaxTime; // time of event with maximum energy
	unsigned char MaxSegId; // segment with maximum energy
	unsigned char MaxCryId; // crystal with maximum energy
	float MaxEnergy; // maximum segment energy
	float SegSumEnergy; // add segment energies
	float AbSumEnergy; // add core energies for addback
	unsigned char seg_mul; // segment multiplicity
	unsigned char ab_mul; // addback multiplicity
	std::vector<unsigned char> ab_index; // index of addback already used
	bool skip_event; // has this event been used already
	
	// Loop over all the events in Miniball detectors
	for( unsigned int i = 0; i < mb_en_list.size(); ++i ) {
	
		// Check if it's a core event
		if( mb_seg_list.at(i) != 0 ) continue;
		
		// Reset addback variables
		MaxSegId = 0; // initialise as core (if no segment hit (dead), use core!)
		MaxEnergy = 0.;
		SegSumEnergy = 0.;
		seg_mul = 0;
		
		// Loop again to find the matching segments
		for( unsigned int j = 0; j < mb_en_list.size(); ++j ) {

			// Skip if it's not the same crystal and cluster
			if( mb_clu_list.at(i) != mb_clu_list.at(j) ||
			    mb_cry_list.at(i) != mb_cry_list.at(j) ) continue;
			
			// Skip is it's the core again
			if( i == j ) continue;
			
			// Increment the segment multiplicity and sum energy
			seg_mul++;
			SegSumEnergy += mb_en_list.at(j);
			
			// Is this bigger than the current maximum energy?
			if( mb_en_list.at(j) > MaxEnergy ){
				
				MaxEnergy = mb_en_list.at(j);
				MaxSegId = mb_seg_list.at(j);
				
			}
			
		} // j: matching segments
		
		// Build the single crystal gamma-ray event
		gamma_ctr++;
		gamma_evt->SetEnergy( mb_en_list.at(i) );
		gamma_evt->SetCluster( mb_clu_list.at(i) );
		gamma_evt->SetCrystal( mb_cry_list.at(i) );
		gamma_evt->SetSegment( MaxSegId );
		gamma_evt->SetTime( mb_ts_list.at(i) );
		write_evts->AddEvt( gamma_evt );

	} // i: core events
	
	
	// Loop over all the gamma-ray singles for addback
	for( unsigned int i = 0; i < write_evts->GetGammaRayMultiplicity(); ++i ) {

		// Reset addback variables
		AbSumEnergy = write_evts->GetGammaRayEvt(i)->GetEnergy();
		MaxCryId = write_evts->GetGammaRayEvt(i)->GetCrystal();
		MaxSegId = write_evts->GetGammaRayEvt(i)->GetSegment();
		MaxEnergy = AbSumEnergy;
		MaxTime = write_evts->GetGammaRayEvt(i)->GetTime();
		ab_mul = 1;	// this is already the first event
		ab_index.clear();
		std::vector<unsigned char>().swap( ab_index );
		
		// Loop to find a matching event for addback
		for( unsigned int j = i+i; j < write_evts->GetGammaRayMultiplicity(); ++j ) {

			// Make sure we are in the same cluster
			// In the future we might consider a more intelligent
			// algorithm, which uses the line-of-sight idea
			if( write_evts->GetGammaRayEvt(i)->GetCluster() !=
				write_evts->GetGammaRayEvt(j)->GetCluster() ) continue;
			
			// Check we haven't already used this event
			skip_event = false;
			for( unsigned int k = 0; k < ab_index.size(); ++k ) {
			
				if( ab_index.at(k) == j ) skip_event = true;
			
			}
			if( skip_event ) continue;
			
			// Then we can add them back
			ab_mul++;
			AbSumEnergy += write_evts->GetGammaRayEvt(j)->GetEnergy();
			
			// Is this bigger than the current maximum energy?
			if( write_evts->GetGammaRayEvt(j)->GetEnergy() > MaxEnergy ){
				
				MaxEnergy = write_evts->GetGammaRayEvt(j)->GetEnergy();
				MaxCryId = write_evts->GetGammaRayEvt(j)->GetCrystal();
				MaxSegId = write_evts->GetGammaRayEvt(j)->GetSegment();
				MaxTime = write_evts->GetGammaRayEvt(j)->GetTime();

			}

			
		} // j: loop for matching addback

		// Build the single crystal gamma-ray event
		gamma_ab_ctr++;
		gamma_ab_evt->SetEnergy( AbSumEnergy );
		gamma_ab_evt->SetCluster( write_evts->GetGammaRayEvt(i)->GetCluster() );
		gamma_ab_evt->SetCrystal( MaxCryId );
		gamma_ab_evt->SetSegment( MaxSegId );
		gamma_ab_evt->SetTime( MaxTime );
		write_evts->AddEvt( gamma_ab_evt );
		
	} // i: gamma-ray singles
	
	return;
	
}


void EventBuilder::ParticleFinder() {

	// Variables for the finder algorithm
	std::vector<unsigned char> pindex;
	std::vector<unsigned char> nindex;

	// Loop over each detector and sector
	for( unsigned int i = 0; i < set->GetNumberOfCDDetectors(); ++i ){

		for( unsigned int j = 0; j < set->GetNumberOfCDSectors(); ++j ){
			
			// Reset variables for a new detector element
			pindex.clear();
			nindex.clear();
			std::vector<unsigned char>().swap(pindex);
			std::vector<unsigned char>().swap(nindex);
			
			// Calculate p/n side multiplicities and get indicies
			for( unsigned int k = 0; k < cd_en_list.size(); ++k ){
				
				if( cd_side_list.at(k) == 0 ) pindex.push_back(k);
				else if( cd_side_list.at(k) == 1 ) nindex.push_back(k);
					
			} // k: all CD events
			
			// ----------------------- //
			// Particle reconstruction //
			// ----------------------- //
			// 1 vs 1 - easiest situation
			if( pindex.size() == 1 && nindex.size() == 1 ) {

				cd_ctr++;
				particle_evt->SetEnergyP( cd_en_list.at( pindex[0] ) );
				particle_evt->SetEnergyN( cd_en_list.at( nindex[0] ) );
				particle_evt->SetTimeP( cd_ts_list.at( pindex[0] ) );
				particle_evt->SetTimeN( cd_ts_list.at( nindex[0] ) );
				particle_evt->SetDetector( i );
				particle_evt->SetSector( j );
				particle_evt->SetStripP( cd_strip_list.at( pindex[0] ) );
				particle_evt->SetStripN( cd_strip_list.at( nindex[0] ) );
				write_evts->AddEvt( particle_evt );
				
			} // 1 vs 1
			
		} // j: sector ID
		
	} // i: detector ID

	return;
	
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
	
	return;
	
}

