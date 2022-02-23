void read_traces( std::string filename = "test/R4_13.root" ) {
	
	// Open file
	TFile *f = new TFile( filename.data() );
	
	// Get Tree
	TTree *t = (TTree*)f->Get("mb");
	
	// Get entries
	unsigned long long nentries = t->GetEntries();
	
	// Branches, etc
	std::shared_ptr<FebexData> febex;
	DataPackets *data = new DataPackets;
	t->SetBranchAddress( "data", &data );
	
	// Canvas
	TCanvas *c1 = new TCanvas();
	
	// Loop
	for( unsigned long long i = 0; i < nentries; ++i ){
		
		// Get entry
		t->GetEntry(i);
		
		// Check if it is FebexData
		if( !data->IsFebex() ) continue;
		
		// If it is febex, get the data packet
		febex = data->GetFebexData();
		
		// Draw trace
		febex.get()->GetTraceGraph()->Draw("ac");
		c1->Update();
		
		// Update the canvas and wait 500 ms
		gSystem->ProcessEvents();
		gSystem->Sleep(500);
		
	}
	
	return;
	
}

