//#include "DataPackets.hh"
//#include "Settings.hh"

//R__LOAD_LIBRARY(libmb_sort.so)

void check_repeated_events( std::string filename = "test/R4_13.root",
						    std::string settingsname = "default" ){
	
	// Open file
	TFile *f = new TFile( filename.data() );
	
	// Get Tree
	TTree *t = (TTree*)f->Get("mb_sort");
	
	// Get entries
	unsigned long long nentries = t->GetEntries();
	
	// Settings file
	std::shared_ptr<MiniballSettings> myset = std::make_shared<MiniballSettings>( settingsname.data() );
	
	// Branches, etc
	std::shared_ptr<FebexData> febex;
	MiniballDataPackets *data = new MiniballDataPackets;
	t->SetBranchAddress( "data", &data );
	
	// Counter for repeat timestamps
	unsigned long ctr = 0;
	
	// Vectors for timestamps
	std::vector<std::vector<std::vector<unsigned long long int>>> timestamp;
	timestamp.resize( myset->GetNumberOfFebexSfps() ); // sfps
	for( unsigned char i = 0; i < myset->GetNumberOfFebexSfps(); ++i ){
		
		timestamp[i].resize( myset->GetNumberOfFebexBoards() ); // boards
		for( unsigned char j = 0; j < myset->GetNumberOfFebexBoards(); ++j ){
			
			timestamp[i][j].resize( myset->GetNumberOfFebexChannels(), 0 ); // channels
			
		} // j: boards

	} // i: sfps

	// Loop
	for( unsigned long long i = 0; i < nentries; ++i ){
	
		// Get entry
		t->GetEntry(i);
		
		// Check if it is FebexData
		if( !data->IsFebex() ) continue;
		
		// If it is febex, get the data packet
		febex = data->GetFebexData();

		// Confirm it's in the right channel
		if( febex->GetSfp() < myset->GetNumberOfFebexSfps() &&
		   febex->GetBoard() < myset->GetNumberOfFebexBoards() &&
		   febex->GetChannel() < myset->GetNumberOfFebexChannels() ) {
			
			// Check if the timestamp in this channel is different
			if( febex->GetTime() == timestamp[febex->GetSfp()][febex->GetBoard()][febex->GetChannel()] ) {
				
				std::cout << "Repeat timestamp in event " << std::dec << i << ":" << std::endl;
				std::cout << "\t" << std::hex << febex->GetTime() << " == ";
				std::cout << std::hex << timestamp[febex->GetSfp()][febex->GetBoard()][febex->GetChannel()];
				std::cout << std::endl;
				
				ctr++;

			}
			
			// Update the current timestamp of this channel
			timestamp[febex->GetSfp()][febex->GetBoard()][febex->GetChannel()] = febex->GetTime();
			
		}

		// Or else spit out an error
		else {
		
			std::cout << "Bad FEBEX chan ID!" << std::endl;
			std::cout << "\tSFP = " << febex->GetSfp();
			std::cout << ", board = " << febex->GetBoard();
			std::cout << ", channel = " << febex->GetChannel() << std::endl;
		
		}
	
	} // i
	
	std::cout << "Total number of repeated events = " << std::dec << ctr;
	std::cout << " / " << nentries << " = " << (double)ctr*100.0/(double)nentries;
	std::cout << "\%" << std::endl;
		
}
