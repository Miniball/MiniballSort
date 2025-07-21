void get_cd_calib(){
	
	//TFile *f = new TFile( "ne22_pt194_slowext_0001.root" );
	TFile *f = new TFile( "R8_10_12_sum.root" );
	TH1F *h;
	TCanvas *c = new TCanvas();

	TGraph* graph = new TGraph();
	TF1* func = new TF1("lin","pol1",0,60e3);

	// Particle energies calculated with LISE++ for scattering at each annular ring from ch.0 to ch.15
	std::vector<Double_t> energy_19F = {74.18,74.78,75.24,75.75,76.25,76.76,77.25,77.81,78.32,78.81,79.33,79.84,80.29,80.77,81.17,81.53};
	std::vector<Double_t> energy_80Zn = {210.2,215.8,221.9,227.7,234.1,240.8,247.6,254.7,262.4,270.0,278.0,285.9,293.8,301.6,308.8,315.6};

	ofstream output("cd_calib_params.dat",ios::out);
	
	// quadrants
	for( unsigned int q = 0; q < 4; ++q ) {
		
		// strips
		for( unsigned int s = 0; s < 16; ++s ) {
			
			std::string hname = "sfp_1/board_" + std::to_string(q*2);
			hname += "/febex1_" + std::to_string(q*2) + "_" + std::to_string(s)+"_qshort";

			// Get histogram
			h = (TH1F*)f->Get( hname.data() );

			// Rebin drastically
			h->Rebin(64);
			
			// Zoom a bit on 19F
			h->GetXaxis()->SetRangeUser( 8e3, 12e3 );
			
			// Get maximum
			unsigned int maxbin_f = h->GetMaximumBin();
			double maxch_f = h->GetBinCenter( maxbin_f );
			
			// Get mean
			Double_t mean_f = h->GetMean();


			// Zoom a bit on 80Zn
			h->GetXaxis()->SetRangeUser( 28e3, 37e3 );
			
			// Get maximum
			unsigned int maxbin_zn = h->GetMaximumBin();
			double maxch_zn = h->GetBinCenter( maxbin_zn );
			
			// Get mean
			Double_t mean_zn = h->GetMean();
			
			// Print
			// std::cout << "Q" << q << ", s" << s << " =\t" << maxch << std::endl;
			// std::cout << "Q" << q << ", s" << s << " =\t" << mean << std::endl;
			// std::cout<<mean_f<<" "<<mean_zn<<endl;

			graph->SetPoint(0,mean_f,energy_19F.at(s)*1000.);		
			graph->SetPoint(1,mean_zn,energy_80Zn.at(s)*1000.);
			graph->Fit(func,"QN");
			
			// Print CD calibration file
			output<<"febex_1_"+std::to_string(q*2)+"_"+std::to_string(s)+".Offset:"<<func->GetParameter(0)<<"\n"
			    "febex_1_"+std::to_string(q*2)+"_"+std::to_string(s)+".Gain:"<<func->GetParameter(1)<<endl;

			// Print fake CD calibration parameters for testing
			// output<<"febex_1_"+std::to_string(q*2)+"_"+std::to_string(s)+".Offset:"<<100.1<<"\n"
			//     "febex_1_"+std::to_string(q*2)+"_"+std::to_string(s)+".Gain:"<<10.1<<endl;


			// Draw
			// h->Draw();
			// c->Update();
			// gSystem->ProcessEvents();
			// gSystem->Sleep(400);
			graph->Clear();
			// cout<<graph->GetN()<<endl;
		}
		
	}
	
	return;
	
}
