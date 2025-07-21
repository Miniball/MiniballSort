//#include "DataPackets.hh"

R__LOAD_LIBRARY(libmb_sort.so)

void plot_dead_time( std::string filename = "test/R4_13.root", unsigned char sfp = 1,
					 unsigned char board = 1, unsigned char channel = 0 ){
	
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
	TCanvas *c1 = new TCanvas();
	
	// Histogram
	std::string htitle = "Time difference between consecutive events in SFP ";
	htitle += std::to_string(sfp) + ", board " << std::to_string(board);
	htitle += ", channel " << std::to_string(channel) << ";#Deltat [ns]";
	TH1F *h = new TH1F( "h", htitle.data(), 10000, -5, 99995 );
	
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
		   board != febex1->GetBoard() ||
		   channel != febex1->GetChannel() )
			continue;
				
	
		// Loop on second event
		for( unsigned long long j = i+1; j < nentries; ++j ){

			// Get entry
			t->GetEntry(j);

			// Check if it is FebexData
			if( !data->IsFebex() ) continue;
			
			// If it is febex, get the data packet
			febex2 = data->GetFebexData();
			
			// Check it's in the same channel
			if( sfp != febex2->GetSfp() ||
			   board != febex2->GetBoard() ||
			   channel != febex2->GetChannel() )
				continue;
			
			// Plot time difference
			h->Fill( (double)febex2->GetTime() - (double)febex1->GetTime() );

			// Once we have the next event in this channel, we're finished
			break;
			
		} // j

	} // i
	
	// Draw it
	h->Draw();
	c->SetLogy();
	
}
