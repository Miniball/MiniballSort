#include "TimeSorter.hh"

TimeSorter::TimeSorter(){
	
	// Progress bar starts as false
	_prog_ = false;

}

bool TimeSorter::SetInputFile( std::string input_file_name ){
	
	// Open next Root input file.
	input_file = new TFile( input_file_name.data(), "read" );
	if( input_file->IsZombie() ) {
		
		std::cout << "Cannot open " << input_file_name << std::endl;
		return false;
		
	}
	
	// Set the input tree
	SetInputTree( (TTree*)input_file->Get("mb") );

	std::cout << "Sorting file by timestamp: " << input_file->GetName() << std::endl;

	return true;
	
}

void TimeSorter::SetInputTree( TTree *user_tree ){

	// Find the tree and set branch addresses
	input_tree = user_tree;
	input_tree->SetCacheSize(200000000); // 200 MB
	input_tree->SetCacheEntryRange(0,input_tree->GetEntries()-1);
	input_tree->AddBranchToCache( "*", kTRUE );
	return;
	
}

void TimeSorter::SetOutput( std::string output_file_name ){
	
	// Open root file
	output_file = new TFile( output_file_name.data(), "recreate", "Time sorted ISS data" );

	// Create output Root file and Tree.
	output_file->cd();
	output_tree = (TTree*)input_tree->CloneTree(0);
	output_tree->SetDirectory( output_file->GetDirectory("/") );
	output_tree->SetName( "mb_sort" );
	output_tree->SetTitle( "Time sorted, calibrated Miniball data" );
	//output_tree->SetBasketSize( "*", 16000 );
	//output_tree->SetAutoFlush( 30*1024*1024 );	// 30 MB
	//output_tree->SetAutoSave( 100*1024*1024 );	// 100 MB
	output_tree->AutoSave();
	
	return;
	
};

unsigned long TimeSorter::SortFile( unsigned long start_sort ) {

	// Start timer
	time( &t_start );
	
	// Time sort all entries of the tree
	n_entries = input_tree->GetEntries();
	std::cout << " Sorting: number of entries in input tree = " << n_entries << std::endl;

	if( n_entries > 0 && start_sort < n_entries  ) {
		
		nb_idx = input_tree->BuildIndex( "data.GetTimeMSB()", "data.GetTimeLSB()" );
		//nb_idx = input_tree->BuildIndex( "0", "data.GetTime()" );
		att_index = (TTreeIndex*)input_tree->GetTreeIndex();
	
		std::cout << " Sorting: size of the sorted index = " << nb_idx << std::endl;

		// Loop on t_raw entries and fill t
		for( unsigned long i = 0; i < nb_idx; ++i ) {
			
			idx = att_index->GetIndex()[i];
			if( idx < start_sort ) continue;
			input_tree->GetEntry( idx );
			output_tree->Fill();
			
			if( i % (nb_idx/100) == 0 || i+1 == nb_idx ) {
				
				// Percent complete
				float percent = (float)(i+1)*100.0/(float)nb_idx;
				
				// Progress bar in GUI
				if( _prog_ ) prog->SetPosition( percent );

				// Progress bar in terminal
				std::cout << " " << std::setw(6) << std::setprecision(4);
				std::cout << percent << "%    \r";
				std::cout.flush();
				gSystem->ProcessEvents();

			}

		}

	}
	
	else std::cout << " Sorting: nothing to sort " << std::endl;	

	// Write histograms, trees and clean up
	output_file->cd();
	output_tree->Write( 0, TObject::kWriteDelete );
	output_file->SaveSelf();
	//input_file->Get( "settings" )->Write( "settings", TObject::kWriteDelete );
	//input_file->Get( "calibration" )->Write( "calibration", TObject::kWriteDelete );
	//output_file->Print();
	
	
	std::cout << "End TimeSorter: time elapsed = " << time(NULL)-t_start << " sec." << std::endl;
	
	return n_entries;
	
}
