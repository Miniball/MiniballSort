void get_segment_phi() {
	
	// Open file
	TFile *f = new TFile( "tmp_212Rn_104Pd_sum.root" );
	
	// Histogram pointer
	TH2F *gE_vs_phi;
	TH1F *phi_proj;
	
	// Canvas
	TCanvas *c1 = new TCanvas();
	
	// Loop over segments
	for( unsigned int i = 0; i < 8; i++ ){
		
		for( unsigned int j = 0; j < 3; j++ ){
			
			for( unsigned int k = 1; k < 7; k++ ){
				
				// Segment id
				unsigned int segID = i*3*7 + j*7 + k;
				
				// Get histogram
				std::string hname = "SegmentPhiDetermination/gE_vs_phi_dc_recoil_seg";
				hname += std::to_string(segID);
				gE_vs_phi = (TH2F*)f->Get( hname.data() );
				
				// Project on energy of peak
				hname = "phi_proj_seg" + std::to_string(segID);
				phi_proj = (TH1F*)gE_vs_phi->ProjectionX( hname.data(), 432, 436 );
				
				// Get the maximum
				int max_bin = phi_proj->GetMaximumBin();
				double max_val = phi_proj->GetMaximum();
				double phi_max = phi_proj->GetBinCenter( max_bin );
				
				// Make some sanity checks
				if( max_val < 10 ) continue;
				if( phi_proj->Integral() < 100 ) continue;
				
				// Find the error bar
				double drop_factor = 0.975; // 97.5% of the maximum is error (arbitrary)
				int low_bin = max_bin, upp_bin = max_bin;
				
				// Upper side
				for( unsigned int m = max_bin; m < phi_proj->GetNbinsX(); m++ ){
					
					if( phi_proj->GetBinContent(m) < max_val * drop_factor ){
						
						upp_bin = m;
						break;
						
					}
					
				}
				
				// Low side
				for( unsigned int m = max_bin; m > 1; m-- ){
					
					if( phi_proj->GetBinContent(m) < max_val * drop_factor ){
						
						low_bin = m;
						break;
						
					}
					
				}

				// Average them
				double phi_err = phi_max - phi_proj->GetBinCenter( low_bin );
				phi_err += phi_proj->GetBinCenter( upp_bin ) - phi_max;
				phi_err /= 2.0;
				
				// Print results
				std::cout << i << "\t" << j << "\t" << k << "\t";
				std::cout << phi_max << "\t" << phi_err << std::endl;
				
				// Show plot
				phi_proj->Draw();
				c1->Update();
				gSystem->ProcessEvents();
				gSystem->Sleep(10);
				
			}
			
		}
		
	}
	
}
