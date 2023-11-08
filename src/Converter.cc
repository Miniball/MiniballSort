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

	tm_stp_febex.resize( set->GetNumberOfFebexSfps() );
	tm_stp_febex_ch.resize( set->GetNumberOfFebexSfps() );

	// Start counters at zero
	for( unsigned int i = 0; i < set->GetNumberOfFebexSfps(); ++i ) {
				
		ctr_febex_hit[i].resize( set->GetNumberOfFebexBoards() );
		ctr_febex_pause[i].resize( set->GetNumberOfFebexBoards() );
		ctr_febex_resume[i].resize( set->GetNumberOfFebexBoards() );

		tm_stp_febex[i].resize( set->GetNumberOfFebexBoards(), 0 );
		tm_stp_febex_ch[i].resize( set->GetNumberOfFebexBoards() );

		for( unsigned int j = 0; j < set->GetNumberOfFebexBoards(); ++j )
			tm_stp_febex_ch[i][j].resize( set->GetNumberOfFebexBoards(), 0 );
		
	}
	
	// Default that we do not have a source or EBIS only run
	flag_source = false;
	flag_ebis = false;
	
	// No progress bar by default
	_prog_ = false;
	
	// Maximum ADC value depends on MBS or MIDAS
	if( mbs_data ) qmax_default = 1 << 23;
	else qmax_default = 4294967296;

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
	
	jump_ctr = 0;	// timestamp jumps
	warp_ctr = 0;	// timestamp warps
	data_ctr = 0;	// total data items
	reject_ctr = 0;	// rejected buffers
	
	buffer_full = false;	// first buffer not yet assumed to be full

	return;
	
}

void MiniballConverter::SetOutput( std::string output_file_name ){
	
	// Open output file
	output_file = new TFile( output_file_name.data(), "recreate", "FEBEX raw data file" );

	return;

};


void MiniballConverter::MakeTree() {

	// Create Root tree
	const int splitLevel = 0; // don't split branches = 0, full splitting = 99
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
	
	output_tree->SetParallelUnzip();

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
	hfebex_qshort.resize( set->GetNumberOfFebexSfps() );
	hfebex_qint.resize( set->GetNumberOfFebexSfps() );
	hfebex_cal.resize( set->GetNumberOfFebexSfps() );
	hfebex_mwd.resize( set->GetNumberOfFebexSfps() );
	hfebex_hit.resize( set->GetNumberOfFebexSfps() );
	hfebex_pause.resize( set->GetNumberOfFebexSfps() );
	hfebex_resume.resize( set->GetNumberOfFebexSfps() );

	// Loop over FEBEX SFPs
	for( unsigned int i = 0; i < set->GetNumberOfFebexSfps(); ++i ) {
		
		hfebex_qshort[i].resize( set->GetNumberOfFebexBoards() );
		hfebex_qint[i].resize( set->GetNumberOfFebexBoards() );
		hfebex_cal[i].resize( set->GetNumberOfFebexBoards() );
		hfebex_mwd[i].resize( set->GetNumberOfFebexBoards() );
		hfebex_hit[i].resize( set->GetNumberOfFebexBoards() );
		hfebex_pause[i].resize( set->GetNumberOfFebexBoards() );
		hfebex_resume[i].resize( set->GetNumberOfFebexBoards() );

		// Loop over each FEBEX board
		for( unsigned int j = 0; j < set->GetNumberOfFebexBoards(); ++j ) {
			
			hfebex_qshort[i][j].resize( set->GetNumberOfFebexChannels() );
			hfebex_qint[i][j].resize( set->GetNumberOfFebexChannels() );
			hfebex_cal[i][j].resize( set->GetNumberOfFebexChannels() );
			hfebex_mwd[i][j].resize( set->GetNumberOfFebexChannels() );

			dirname  = maindirname + "sfp_" + std::to_string(i);
			dirname += "/board_" + std::to_string(j);
			
			if( !output_file->GetDirectory( dirname.data() ) )
				output_file->mkdir( dirname.data() );
			output_file->cd( dirname.data() );

			// Loop over channels of each FEBEX board
			for( unsigned int k = 0; k < set->GetNumberOfFebexChannels(); ++k ) {
				
				// Uncalibrated energy - 32-bit value
				hname = "febex_" + std::to_string(i);
				hname += "_" + std::to_string(j);
				hname += "_" + std::to_string(k);
				hname += "_qshort";
				
				htitle = "Raw FEBEX spectra for SFP " + std::to_string(i);
				htitle += ", board " + std::to_string(j);
				htitle += ", channel " + std::to_string(k);
				
				htitle += ";Charge value;Counts";

				if( output_file->GetListOfKeys()->Contains( hname.data() ) )
					hfebex_qshort[i][j][k] = (TH1F*)output_file->Get( hname.data() );
				
				else {
					
					hfebex_qshort[i][j][k] = new TH1F( hname.data(), htitle.data(),
													16384, 0, (unsigned long long)1<<16 );
					
					hfebex_qshort[i][j][k]->SetDirectory( output_file->GetDirectory( dirname.data() ) );
					
				}
				
				// Uncalibrated energy 16-bit value
				hname = "febex_" + std::to_string(i);
				hname += "_" + std::to_string(j);
				hname += "_" + std::to_string(k);
				hname += "_qint";
				
				htitle = "Raw FEBEX spectra for SFP " + std::to_string(i);
				htitle += ", board " + std::to_string(j);
				htitle += ", channel " + std::to_string(k);
				
				htitle += ";Charge value;Counts";
				
				if( output_file->GetListOfKeys()->Contains( hname.data() ) )
					hfebex_qint[i][j][k] = (TH1F*)output_file->Get( hname.data() );
				
				else {
					
					hfebex_qint[i][j][k] = new TH1F( hname.data(), htitle.data(),
													16384, 0, qmax_default );
					
					hfebex_qint[i][j][k]->SetDirectory( output_file->GetDirectory( dirname.data() ) );
					
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
				
				// Assume gamma-ray spectrum
				unsigned int ebins = 8000;
				float emin = -0.25;
				float emax = 4000.0 + emin; // 4 MeV range
				
				// Check if we have particles with low gain preamps (heavy ions, Coulex)
				if( ( cal->FebexType(i,j,k) == "Qshort" && cal->FebexGain(i,j,k) > 5 )
				   || ( cal->FebexType(i,j,k) == "Qint" && cal->FebexGain(i,j,k) > 0.0005 ) ) {
					
					ebins = 8000.0;
					emin = -125.0;
					emax = 2000000.0 + emin; // 2 GeV range
					
				}
				
				// Check if we have particles with high gain preamps (light ions, transfer)
				else if( ( cal->FebexType(i,j,k) == "Qshort" && cal->FebexGain(i,j,k) > 0.1 )
				   || ( cal->FebexType(i,j,k) == "Qint" && cal->FebexGain(i,j,k) > 0.00001 ) ) {
					
					ebins = 8000.0;
					emin = -12.5;
					emax = 200000.0 + emin; // 200 MeV range
					
				}
				
				if( output_file->GetListOfKeys()->Contains( hname.data() ) )
					hfebex_cal[i][j][k] = (TH1F*)output_file->Get( hname.data() );
				
				else {
					
					hfebex_cal[i][j][k] = new TH1F( hname.data(), htitle.data(),
												ebins, emin, emax );
					
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
												32768, -0.25, 16383.75 );
					
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
				
				hfebex_qshort[i][j][k]->Reset( "ICEMS" );
				hfebex_qint[i][j][k]->Reset( "ICEMS" );
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
	
	std::cout << "Filtering data, but not time ordering" << std::endl;
	
	// Loop on entries and fill sorted tree
	long long int n_ents = output_tree->GetEntries();
	for( long long int i = 0; i < n_ents; ++i ) {

		// Read entry
		output_tree->GetEntry(i);

		// Throw away any dodgy data
		//if( data_packet->GetSfp() >= set->GetNumberOfFebexSfps() ) continue;
		//if( data_packet->GetBoard() >= set->GetNumberOfFebexBoards() ) continue;
		//if( data_packet->GetChannel() >= set->GetNumberOfFebexChannels() ) continue;

		// Bodge the time maybe?
		//if( tm_stp_febex[data_packet->GetSfp()][data_packet->GetBoard()] != 0 &&
		//   TMath::Abs( tm_stp_febex[data_packet->GetSfp()][data_packet->GetBoard()] - data_packet->GetTime() ) > 300e9 ) {
					
			//std::cout << "Timestamp jump on SFP " << (int)my_sfp_id << ", board ";
			//std::cout << (int)my_board_id << ", channel " << (int)my_ch_id << std::endl;
			//continue;
			
		//}
		//tm_stp_febex[data_packet->GetSfp()][data_packet->GetBoard()] = data_packet->GetTime();

		// Write the data to the sorted tree
		sorted_tree->Fill();

		// Progress bar
		bool update_progress = false;
		if( n_ents < 200 )
			update_progress = true;
		else if( i % (n_ents/100) == 0 || i+1 == n_ents )
			update_progress = true;
		
		if( update_progress ) {
			
			// Percent complete
			float percent = (float)(i+1)*100.0/(float)n_ents;
			
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

	return;
	
}

unsigned long long int MiniballConverter::SortTree(){
	
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
	unsigned long long int nb_idx = att_index->GetN();
	std::cout << " Sorting: size of the sorted index = " << nb_idx << std::endl;

	// Loop on t_raw entries and fill t
	for( unsigned long i = 0; i < nb_idx; ++i ) {
		
		// Clean up old data
		data_packet->ClearData();
		
		// Get time-ordered event index
		unsigned long long int idx = att_index->GetIndex()[i];
		
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


