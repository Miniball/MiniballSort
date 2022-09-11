#include "../include/DataPackets.hh"

void read_traces( std::string filename = "test/R4_13.root" ) {
	
	// Open file
	TFile *f = new TFile( filename.data() );
	
	// Get Tree
	TTree *t = (TTree*)f->Get("mb_sort");
	
	// Get entries
	unsigned long long nentries = t->GetEntries();
	
	// Branches, etc
	std::shared_ptr<FebexData> febex;
	MiniballDataPackets *data = new MiniballDataPackets;
	t->SetBranchAddress( "data", &data );
	
	// Canvas
	TCanvas *c1 = new TCanvas();
	
	// Loop
	for( unsigned long long i = 0; i < nentries; ++i ){
		
        // Skip except for every 16
        //if( i%16 != 0 ) continue;
        
		// Get entry
		t->GetEntry(i);
		
		// Check if it is FebexData
		if( !data->IsFebex() ) continue;
		
		// If it is febex, get the data packet
		febex = data->GetFebexData();
		
		// Draw trace
		febex.get()->GetTraceGraph()->Draw("ac");
        
        // Add label
        TText lab;
        std::string lab_str = "Event #" + std::to_string(i);
        lab.DrawTextNDC( 0.2, 0.2, lab_str.data() );
		
		// Update the canvas and wait 50 ms
        c1->Update();
		gSystem->ProcessEvents();
		gSystem->Sleep(200);
		
	}
	
	return;
	
}

