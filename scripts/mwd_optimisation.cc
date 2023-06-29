#include "../include/DataPackets.hh"
#include "../include/Calibration.hh"

void mwd_optimisation( std::string filename = "test/R4_13.root", unsigned int sfp = 0,
			   unsigned board = 0, unsigned int ch = 0, std::string calfile = "default" ) {
	
	// Open input file
	TFile *f = new TFile( filename.data() );
	
	// Open output file
	TFile *fout = new TFile( "mwd_scan.root", "RECREATE" );
	
	// Get Tree
	TTree *t = (TTree*)f->Get("mb_sort");
	
	// Settings file - needed for calibration, just use defaults
	std::shared_ptr<MiniballSettings> myset = std::make_shared<MiniballSettings>( "default" );
	
	// Calibration file
	MiniballCalibration *cal = new MiniballCalibration( calfile.data(), myset );
		
	// Get entries
	unsigned long long n_entries = t->GetEntries();
	//if( nentries > 1000 ) n_entries = 1000;
	
	// Branches, etc
	std::shared_ptr<FebexData> febex;
	MiniballDataPackets *data = new MiniballDataPackets;
	t->SetBranchAddress( "data", &data );
	
	// Define range of parameters to scan
	const int Nscan = 5;
	int tau_low = 40000, tau_upp = 60000;
	int step = ( tau_upp - tau_low ) / Nscan;

	// Energy histograms
	std::vector<TH1F*> hmwd;
	hmwd.resize( Nscan+1, nullptr );
	
	// Loop
	for( unsigned long long i = 0; i < n_entries; ++i ){
		        
		// Get entry
		t->GetEntry(i);
		
		// Check if it is FebexData
		if( !data->IsFebex() ) continue;
		
		// If it is febex, get the data packet
		febex = data->GetFebexData();
		
		// Check if it matches the channel we want
		if( febex->GetSfp() == sfp &&
		    febex->GetBoard() == board &&
		    febex->GetChannel() == ch ) {
		
			// Scan the paramters of your choosing, e.g. tau
			for( unsigned int j = 0; j <= Nscan; ++j ){
				
				// Calculate the parameter from the range
				int tau = tau_low + j * step;

				// Set some parameters
				cal->SetMWDDecay( sfp, board, ch, tau );
				
				// MWD data
				FebexMWD mwd = cal->DoMWD( sfp, board, ch, febex->GetTrace() );

				// Make the histogram if it doesn't exist
				if( hmwd[j] == nullptr ) {
				
					std::string hname = "mwd_energy_" + std::to_string(j);
					std::string htitle = "Moving window energy spectrum with tau = ";
					htitle += std::to_string(tau) + ";Energy [arb. units];Counts";
					hmwd[j] = new TH1F( hname.data(), htitle.data(), 65536, -0.5, 65535.5 );
					
				}
				
				// Fill histogram
				for( unsigned int k = 0; k < mwd.NumberOfTriggers(); ++k )
					hmwd[j]->Fill( mwd.GetEnergy(k) );
				
				// Fit histogram
				

			} // tau scan

		} // correct channel
		
		// Progress bar
		bool update_progress = false;
		if( n_entries < 200 )
			update_progress = true;
		else if( i % (n_entries/100) == 0 || i+1 == n_entries )
			update_progress = true;
		
		if( update_progress ) {
			
			// Percent complete
			float percent = (float)(i+1)*100.0/(float)n_entries;
			
			// Progress bar in terminal
			std::cout << " " << std::setw(6) << std::setprecision(4);
			std::cout << percent << "%    \r";
			std::cout.flush();
			
		}
		
	} // nentries loop
	
	fout->Write();
	
	return;
	
}

