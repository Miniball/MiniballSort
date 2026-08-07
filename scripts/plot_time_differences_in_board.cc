//#include "DataPackets.hh"

R__LOAD_LIBRARY(libmb_sort.so)

void plot_time_differences_in_board( std::string filename = "test/R4_13.root",
									unsigned char sfp = 1, unsigned char board = 1,
									unsigned int event_depth = 1000 ){

	// Open file
	TFile *f = new TFile( filename.data() );
	
	// Get Tree
	TTree *t = (TTree*)f->Get("mb_sort");
	
	// Get entries
	unsigned long long nentries = t->GetEntries();

	// Branches, etc
	std::shared_ptr<FebexData> febex1;
	std::shared_ptr<FebexData> febex2;
	MiniballDataPackets *data = new MiniballDataPackets;
	t->SetBranchAddress( "data", &data );
	
	// Canvas
	TCanvas *c1 = new TCanvas("c","c",800,800);
	c1->Divide(4,4);

	// Histogram
	TH1F *h[16];
	for( unsigned int i = 0; i < 16; i++ ) {
		std::string hname = "h" + std::to_string(i);
		std::string htitle = "Time difference between consecutive events in SFP ";
		htitle += std::to_string(sfp) + ", board " + std::to_string(board);
		htitle += ", channel 0 and " + std::to_string(i) + ";#Deltat [ns]";
		h[i] = new TH1F( hname.data(), htitle.data(), 1e5, -1e6, 1e6 );
	}

	// Loop
	for( unsigned long long i = 0; i < nentries; ++i ){
	
		// Get entry
		t->GetEntry(i);
		
		// Check if it is FebexData
		if( !data->IsFebex() ) continue;
		
		// If it is febex, get the data packet
		febex1 = data->GetFebexData();
		
		// Confirm it's in the right channel
		if( sfp != febex1->GetSfp() ||
		   board != febex1->GetBoard() )
			continue;
				
	
		// Loop on second event
		for( unsigned long long j = i+1; j < event_depth; ++j ){

			// Get entry
			t->GetEntry(j);

			// Check if it is FebexData
			if( !data->IsFebex() ) continue;
			
			// If it is febex, get the data packet
			febex2 = data->GetFebexData();
			
			// Check it's in the same channel
			if( sfp != febex2->GetSfp() ||
			   board != febex2->GetBoard() )
				continue;

			// Get time difference
			double tdiff;
			unsigned int ch2;
			if( febex1->GetChannel() == 0 ) {
				tdiff = febex2->GetTime() - febex1->GetTime();
				ch2 = febex2->GetChannel();
			}
			else if( febex2->GetChannel() == 0 ) {
				tdiff = febex1->GetTime() - febex2->GetTime();
				ch2 = febex1->GetChannel();
			}
			else continue;

			// Plot time difference
			if( ch2 < 16 ) h[ch2]->Fill( tdiff );

		} // j

	} // i
	
	// Draw it
	for( unsigned int i = 0; i < 16; i++ ) {

		c1->cd(i+1);
		h[i]->Draw();
		c1->SetLogy();

	}

}
