// An abstract class for the MBS or MIDAS data conversion
#include "Converter.hh"

MiniballConverter::MiniballConverter( std::shared_ptr<MiniballSettings> myset ) {

	// We need to do initialise, but only after Settings are added
	set = myset;

	my_tm_stp_msb = 0;
	my_tm_stp_hsb = 0;

	ctr_febex_hit.resize( set->GetNumberOfFebexSfps() );
	ctr_febex_pause.resize( set->GetNumberOfFebexSfps() );
	ctr_febex_resume.resize( set->GetNumberOfFebexSfps() );

	// Start counters at zero
	for( unsigned int i = 0; i < set->GetNumberOfFebexSfps(); ++i ) {
				
		ctr_febex_hit[i].resize( set->GetNumberOfFebexBoards() );
		ctr_febex_pause[i].resize( set->GetNumberOfFebexBoards() );
		ctr_febex_resume[i].resize( set->GetNumberOfFebexBoards() );

	}
	
	// Default that we do not have a source only run
	flag_source = false;
	
	// No progress bar by default
	_prog_ = false;
	
	// Maximum ADC value depends on MBS or MIDAS
	if( mbs_data ) qmax = 1 << 23;
	else qmax = 1 << 30;
	
}

void MiniballConverter::StartFile(){
	
	// Start counters at zero
	for( unsigned int i = 0; i < set->GetNumberOfFebexSfps(); ++i ) {
				
		// Start counters at zero
		for( unsigned int j = 0; j < set->GetNumberOfFebexBoards(); ++j ) {
					
			ctr_febex_hit[i][j] = 0;	// hits on each module
			ctr_febex_pause[i][j] = 0;
			ctr_febex_resume[i][j] = 0;
			
		}

	}

	ctr_febex_ext = 0;	// pulser trigger

	return;
	
}

void MiniballConverter::SetOutput( std::string output_file_name ){
	
	// Open output file
	output_file = new TFile( output_file_name.data(), "recreate", "FEBEX raw data file" );

	return;

};


void MiniballConverter::MakeTree() {

	// Create Root tree
	const int splitLevel = 2; // don't split branches = 0, full splitting = 99
	const int bufsize = sizeof(FebexData) + sizeof(InfoData);
	output_tree = new TTree( "mb", "mb" );
	mbsinfo_tree = new TTree( "mbsinfo", "mbsinfo" );
	data_packet = std::make_unique<MiniballDataPackets>();
	mbsinfo_packet = std::make_unique<MBSInfoPackets>();
	output_tree->Branch( "data", "MiniballDataPackets", data_packet.get(), bufsize, splitLevel );
	mbsinfo_tree->Branch( "mbsinfo", "MBSInfoPackets", mbsinfo_packet.get(), sizeof(MBSInfoPackets), 0 );
	
	sorted_tree = (TTree*)output_tree->CloneTree(0);
	sorted_tree->SetName("mb_sort");
	sorted_tree->SetTitle( "Time sorted, calibrated Miniball data" );
	sorted_tree->SetDirectory( output_file->GetDirectory("/") );
	output_tree->SetDirectory( output_file->GetDirectory("/") );
	mbsinfo_tree->SetDirectory( output_file->GetDirectory("/") );
	
	output_tree->SetAutoFlush(-10e6);
	sorted_tree->SetAutoFlush(-10e6);
	mbsinfo_tree->SetAutoFlush(-10e6);

	febex_data = std::make_shared<FebexData>();
	info_data = std::make_shared<InfoData>();
	
	febex_data->ClearData();
	info_data->ClearData();
	
	return;
	
}

void MiniballConverter::MakeHists() {
	
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
											32768, 0, qmax );
					
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
	
	// Hit time plot
	hhit_time = new TH1F( "hhit_time", "Hit time distribution", 3200, -16000, 16000 );

	return;
	
}

void MiniballConverter::ResetHists() {

	// Loop over FEBEX SFPs
	for( unsigned int i = 0; i < set->GetNumberOfFebexSfps(); ++i ) {

		// Loop over each FEBEX board
		for( unsigned int j = 0; j < set->GetNumberOfFebexBoards(); ++j ) {
			
			// Loop over channels of each FEBEX board
			for( unsigned int k = 0; k < set->GetNumberOfFebexChannels(); ++k ) {
				
				hfebex[i][j][k]->Reset( "ICEMS" );
				hfebex_cal[i][j][k]->Reset( "ICEMS" );
				hfebex_mwd[i][j][k]->Reset( "ICEMS" );
				
			} // k - channel

			hfebex_hit[i][j]->Reset( "ICEMS" );
			hfebex_pause[i][j]->Reset( "ICEMS" );
			hfebex_resume[i][j]->Reset( "ICEMS" );

		} // j - board
		
	} // i - SFP
	
	hfebex_ext->Reset( "ICEMS" );
	hhit_time->Reset( "ICEMS" );
	
	return;
	
}

void MiniballConverter::BuildMbsIndex(){
	
	// Make the index for the MBS info tree
	mbsinfo_tree->BuildIndex( "mbsinfo.GetEventID()" );

	return;
	
}

void MiniballConverter::BodgeMidasSort(){
	
	// Bodge the time maybe?
	
	// Loop on entries and fill sorted tree
	for( long long i = 0; i < output_tree->GetEntries(); ++i ) {

		output_tree->GetEntry(i);
		sorted_tree->Fill();

	}
	
	// Reset the output tree so it's empty after we've finished
	output_tree->FlushBaskets();
	output_tree->Reset();

	return;
	
}

unsigned long long MiniballConverter::SortTree(){
	
	// Reset the sorted tree so it's empty before we start
	sorted_tree->Reset();

	// Load the full tree if possible
	//output_tree->SetMaxVirtualSize(200e6); // 200 MB
	//sorted_tree->SetMaxVirtualSize(200e6); // 200 MB
	//output_tree->LoadBaskets(200e6); 	 // Load 6 MB of data to memory
	
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
		//if( output_tree->MemoryFull(30e6) )
		//	output_tree->DropBaskets();
		//if( sorted_tree->MemoryFull(30e6) )
		//	sorted_tree->FlushBaskets();
		
		// Get entry from unsorted tree and fill to sorted tree
		output_tree->GetEntry( idx );
		sorted_tree->Fill();

		// Optimise filling tree
		//if( i == 100 ) sorted_tree->OptimizeBaskets(30e6);	 // sorted tree basket size max 30 MB

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


