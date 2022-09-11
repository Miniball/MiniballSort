#include "../include/DataPackets.hh"
#include "../include/Calibration.hh"

void mwd_plots( std::string filename = "test/R4_13.root", unsigned int sfp = 0,
			   unsigned board = 0, unsigned int ch = 0, std::string calfile = "default" ) {
	
	// Open file
	TFile *f = new TFile( filename.data() );
	
	// Get Tree
	TTree *t = (TTree*)f->Get("mb");
	
	// Settings file - needed for calibration, just use defaults
	std::shared_ptr<MiniballSettings> myset = std::make_shared<MiniballSettings>( "default" );
	
	// Calibration file
	MiniballCalibration *cal = new MiniballCalibration( calfile.data(), myset );
		
	// Get entries
	unsigned long long nentries = t->GetEntries();
	//if( nentries > 1000 ) nentries = 1000;
	
	// Branches, etc
	std::shared_ptr<FebexData> febex;
	MiniballDataPackets *data = new MiniballDataPackets;
	t->SetBranchAddress( "data", &data );
	
	// Canvas
	TCanvas *c1 = new TCanvas( "c1", filename.data(), 900, 1000 );
	c1->Divide(2,3);
	TGraph *g1, *g2, *g3, *g4, *g5, *g6;
	std::string title;
	
	// Energy histogram
	TH1F *h = new TH1F( "mwd_energy", "Energy spectrum", 65536, -0.5, 65535.5 );
	
	// Loop
	for( unsigned long long i = 0; i < nentries; ++i ){
		        
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
		
			// MWD data
			FebexMWD mwd = cal->DoMWD( sfp, board, ch, febex->GetTrace() );
			
			// Fill histogram
			for( unsigned int i = 0; i < mwd.NumberOfTriggers(); ++i )
				h->Fill( mwd.GetEnergy(i) );
			
			// Draw trace - graph1
			c1->cd(1);
			title = "Waveform - #" + std::to_string(i);
			g1 = mwd.GetTraceGraph();
			g1->SetTitle( title.data() );
			g1->Draw("ac");

			// Draw stage1 - graph2
			c1->cd(2);
			title = "Decay subtracted - #" + std::to_string(i);
			g2 = mwd.GetStage1Graph();
			g2->SetTitle( title.data() );
			g2->Draw("ac");

			// Draw stage2 - graph3
			c1->cd(3);
			title = "Difference - #" + std::to_string(i);
			g3 = mwd.GetStage2Graph();
			g3->SetTitle( title.data() );
			g3->Draw("ac");

			// Draw stage3 - graph4
			c1->cd(4);
			title = "Moving average - #" + std::to_string(i);
			g4 = mwd.GetStage3Graph();
			g4->SetTitle( title.data() );
			g4->Draw("ac");

			// Draw diff2 - graph5
			c1->cd(5);
			title = "CFD - #" + std::to_string(i);
			g5 = mwd.GetCfdGraph();
			g5->SetTitle( title.data() );
			g5->Draw("ac");
			
			// Add CFD triggers
			std::vector<TArrow> arr( mwd.NumberOfTriggers() );
			for( unsigned int i = 0; i < mwd.NumberOfTriggers(); ++i ) {

				arr[i].SetLineColor(kRed+1);
				arr[i].DrawArrow( mwd.GetCfdTime(i), 0,
								  mwd.GetCfdTime(i), -1.0 * mwd.GetEnergy(i),
								  0.01, "-|>" );

			}

			// Draw energy histogram - graph6
			c1->cd(6);
			if( h->Integral() > 50 )
				h->GetXaxis()->SetRangeUser(
						h->GetMean() - 10.* h->GetStdDev(),
						h->GetMean() + 10.* h->GetStdDev() );
			h->Draw("hist");
			
			// Update the canvas and wait 5 ms
			c1->Update();
			gSystem->ProcessEvents();
			gSystem->Sleep(700);

		} // correct channel
		
	} // nentries loop
	
	return;
	
}

