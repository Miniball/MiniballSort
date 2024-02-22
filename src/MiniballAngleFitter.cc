#include "MiniballAngleFitter.hh"

MiniballAngleFunction::MiniballAngleFunction( std::shared_ptr<MiniballSettings> _myset, std::shared_ptr<MiniballReaction> _myreact ) {

	// Settings and reaction files
	myset = _myset;
	myreact = _myreact;

	// Now initialise everything
	Initialise();

};

void MiniballAngleFunction::Initialise() {
	
	// Loop over clusters
	present.resize( myset->GetNumberOfMiniballClusters() );
	energy.resize( myset->GetNumberOfMiniballClusters() );
	err.resize( myset->GetNumberOfMiniballClusters() );
	for( unsigned int clu = 0; clu < myset->GetNumberOfMiniballClusters(); ++clu ) {
		
		cluster.push_back(false);
		present[clu].resize( myset->GetNumberOfMiniballCrystals() );
		energy[clu].resize( myset->GetNumberOfMiniballCrystals() );
		err[clu].resize( myset->GetNumberOfMiniballCrystals() );

		// Loop over crystals
		for( unsigned int cry = 0; cry < myset->GetNumberOfMiniballCrystals(); ++cry ) {
			
			present[clu][cry].resize( myset->GetNumberOfMiniballSegments() );
			energy[clu][cry].resize( myset->GetNumberOfMiniballSegments() );
			err[clu][cry].resize( myset->GetNumberOfMiniballSegments() );

			// Loop over segments
			for( unsigned int seg = 0; seg < myset->GetNumberOfMiniballSegments(); ++seg ) {
				
				present[clu][cry].push_back(false);
				energy[clu][cry].push_back(0.0);
				err[clu][cry].push_back(0.0);
			
			} // seg
		
		} // cry
	
	} // clu

	// Set the user z
	user_z = myreact->GetOffsetZ();

	
}

bool MiniballAngleFunction::FitPeak( TH1D *h, double &en, double &er ){
	
	// Work out some limits
	double low_lim = en*0.92 - 5.;
	double upp_lim = en*1.08 + 5.;
	if( eref < 500. && upp_lim > 500. ) upp_lim = 500.;
	
	// Draw the thing
	auto c1 = std::make_unique<TCanvas>();
	h->GetXaxis()->SetRangeUser( low_lim - 20., upp_lim + 20. );
	h->Draw();
	
	// Make a TF1
	auto peakfit = std::make_unique<TF1>( "peakfit", "gaus(0)+pol1(3)", low_lim, upp_lim );
	
	// Set initial parameters
	double en_est = h->GetBinCenter( h->GetMaximumBin() );
	double sig_est = 1.7; // 1.7 keV guess for sigma = 4.0 keV for FWHM
	double bg_est = h->GetBinContent( h->FindBin( en_est - 5.0 * sig_est ) );
	double amp_est = h->GetBinContent( h->GetMaximumBin() );
	amp_est -= bg_est;
	if( amp_est < 1.0 ) amp_est = h->GetBinContent( h->GetMaximumBin() ) + 1.0;
	peakfit->SetParameter( 0, amp_est );	// amplitude
	peakfit->SetParameter( 1, en_est );		// centroid
	peakfit->SetParameter( 2, sig_est );	// sigma width
	peakfit->SetParameter( 3, bg_est );		// bg const
	peakfit->SetParameter( 4, 1e-9 );		// bg gradient
	
	// Parameter limits
	double integral = h->Integral( h->FindBin(low_lim), h->FindBin(upp_lim) );
	peakfit->SetParLimits( 0, 1.0, integral );				// amplitude limit
	peakfit->SetParLimits( 1, low_lim, upp_lim );			// centroid limit
	peakfit->SetParLimits( 2, sig_est*0.25, sig_est*4.0 );	// sigma limit

	// Make fit and check for success
	gErrorIgnoreLevel = kError;
	auto res = h->Fit( peakfit.get(), "LRSQ" );
	c1->Print("peak_fits.pdf");
	gErrorIgnoreLevel = kInfo;
	if( res == 0 && integral > 150. ) {
		
		// Set parameters back
		en = peakfit->GetParameter(1);
		er = peakfit->GetParError(1);
		return true;

	}
	
	else return false;

}

void MiniballAngleFunction::FitSegmentEnergies( TFile *infile ){

	// Names of the spectra in the events file
	std::string hname, folder = "/miniball/cluster_", base = "mb_en_core_seg_";
	
	// Histogram objects
	TH1D *h1;
	TH2D *h2;
	
	// Open the pdf file for peak fits
	auto c1 = std::make_unique<TCanvas>();
	gErrorIgnoreLevel = kWarning;
	c1->Print("peak_fits.pdf(");

	// Loop over all clusters
	for( unsigned int clu = 0; clu < myset->GetNumberOfMiniballClusters(); ++clu ) {
		
		// Loop over crystals
		for( unsigned int cry = 0; cry < myset->GetNumberOfMiniballCrystals(); ++cry ) {
			
			// Loop over segments
			for( unsigned int seg = 1; seg < myset->GetNumberOfMiniballSegments(); ++seg ) {

				// Build up the 2D histogram name
				hname  = folder + std::to_string(clu) + "/";
				hname += base + std::to_string(clu) + "_";
				hname += std::to_string(cry) + "_ebis_on";
				
				// Get 2D histogram from file
				h2 = static_cast<TH2D*>( infile->Get( hname.data() ) );
				
				// Build up the 1D histogram name
				hname  = std::to_string(clu) + "_";
				hname += std::to_string(cry) + "_";
				hname += std::to_string(seg) + "_ebis_on";

				// Project a 1D histogram from this
				h1 = static_cast<TH1D*>( h2->ProjectionY( hname.data(), seg+1, seg+1 ) );
				
				// Check if we have some counts
				if( h1->Integral() > 0 ) {
					cluster[clu] = true;
					present[clu][cry][seg] = true;
				}
				else {
					present[clu][cry][seg] = false;
					continue;
				}
				
				// Predict the centroid from intial guesses
				double en_init = myreact->DopplerShift( eref, myreact->GetBeta(),
								TMath::Cos( myreact->GetGammaTheta( clu, cry, seg ) ) );
				
				// Fit peak and update the energy and error
				energy[clu][cry][seg] = en_init;
				if( !FitPeak( h1, energy[clu][cry][seg], err[clu][cry][seg] ) )
					present[clu][cry][seg] = false;

				// Don't include data with big errors, probably a fitting issue
				if( err[clu][cry][seg] > 0.6 )
					present[clu][cry][seg] = false;

			} // seg
			
		} // cry
		
	} // clu
	
	// Close the pdf file for peak fits
	c1->Print("peak_fits.pdf)");

	// Clean up
	delete h1;
	delete h2;

	gErrorIgnoreLevel = kInfo;

}

void MiniballAngleFunction::LoadExpEnergies( std::string energy_file ){
	
	// Open file
	std::ifstream energyfile( energy_file );

	// Some parameters to read in
	int cl, cr, sg;
	double en, er;

	// Loop over the whole file
	while( energyfile >> cl >> cr >> sg >> en >> er ) {

		if( cl >= (int)myset->GetNumberOfMiniballClusters() || cl < 0 ) {
			
			std::cerr << "Bad cluster number = " << cl << std::endl;
			continue;
			
		}

		if( cr >= (int)myset->GetNumberOfMiniballCrystals() || cr < 0 ) {
			
			std::cerr << "Bad crystal number = " << cr << std::endl;
			continue;
			
		}

		if( sg >= (int)myset->GetNumberOfMiniballSegments() || sg < 0 ) {
			
			std::cerr << "Bad segment number = " << sg << std::endl;
			continue;
			
		}

		present[cl][cr][sg] = true;
		energy[cl][cr][sg] = en;
		err[cl][cr][sg] = er;
		cluster[cl] = true;

	}
	
	energyfile.close();
	
};

void MiniballAngleFunction::SaveExpEnergies( std::string energy_file ){

	// Open file
	std::ofstream energyfile( energy_file, std::ios::out );

	// Loop over all clusters
	for( unsigned int clu = 0; clu < myset->GetNumberOfMiniballClusters(); ++clu ) {
		
		// Loop over crystals
		for( unsigned int cry = 0; cry < myset->GetNumberOfMiniballCrystals(); ++cry ) {
			
			// Loop over segments
			for( unsigned int seg = 1; seg < myset->GetNumberOfMiniballSegments(); ++seg ) {
				
				if( present[clu][cry][seg] ){
					
					energyfile << clu << "\t" << cry << "\t" << seg << "\t";
					energyfile << energy[clu][cry][seg] << "\t" << err[clu][cry][seg] << std::endl;
				
				}
				
			} // seg
			
		} // cry
		
	} // clu
	
	energyfile.close();
				
}

double MiniballAngleFunction::operator() ( const double *p ) {

	// change angles
	// p[0] = beta
	// p[1] = theta1, p[2] = phi1, p[3] = alpha1, p[4] = r1,
	// p[5] = theta2, p[6] = phi2, p[7] = alpha2, p[8] = r2, etc.
	int indx = 1;

	// Loop over clusters
	for( unsigned int clu = 0; clu < myset->GetNumberOfMiniballClusters(); ++clu ) {

		myreact->SetupCluster( clu, p[indx], p[indx+1], p[indx+2], p[indx+3], user_z );
		indx += 4;

	}
	
	double chisq = 0;
	
	// Loop over clusters
	for( unsigned int clu = 0; clu < myset->GetNumberOfMiniballClusters(); ++clu ) {

		// Loop over crystals
		for( unsigned int cry = 0; cry < myset->GetNumberOfMiniballCrystals(); ++cry ) {

			// Loop over segments
			for( unsigned int seg = 0; seg < myset->GetNumberOfMiniballSegments(); ++seg ) {
				
				if( !present[clu][cry][seg] ) continue;
				double theta = myreact->GetGammaTheta( clu, cry, seg );
				double beta = p[0];
				double edop = myreact->DopplerShift( eref, beta, TMath::Cos(theta) );
				chisq += TMath::Power( ( energy[clu][cry][seg] - edop ) / err[clu][cry][seg], 2.0 );

			}

		}

	}
	
	return chisq;
	
}

MiniballAngleFitter::MiniballAngleFitter() {

	// Just use dummy files
	MiniballAngleFitter( "default", "default" );
	
}

MiniballAngleFitter::MiniballAngleFitter( std::string settings_file, std::string reaction_file ) {

	// Settings and reaction files
	myset = std::make_shared<MiniballSettings>( settings_file );
	myreact = std::make_shared<MiniballReaction>( reaction_file, myset );
	
	// Now call main function
	MiniballAngleFitter( myset, myreact );

}

MiniballAngleFitter::MiniballAngleFitter( std::shared_ptr<MiniballSettings> _myset, std::shared_ptr<MiniballReaction> _myreact ) {

	// Settings and reaction files
	myset = _myset ;
	myreact = _myreact;
	
	// Now initialise
	Initialise();

}

// Input ROOT file
bool MiniballAngleFitter::SetInputROOTFile( std::string fname ){
	
	// Open input file
	input_root_file = new TFile( fname.data(), "read" );
	if( input_root_file->IsZombie() ) {
		
		std::cout << "Cannot open " << fname << std::endl;
		return false;
		
	}
	
	// Set the flag for fitting the peaks from the ROOT file
	flag_fit_peaks = true;
	
	// Of course, when it works, we should return true here
	return true;
	
}

// Input data file
bool MiniballAngleFitter::SetInputEnergiesFile( std::string fname ){
	
	// Open input file.
	input_data_filename = fname;
	std::ifstream ftest;
	ftest.open( input_data_filename );
	if( !ftest.is_open() ) {
		
		std::cout << "Cannot open " << input_data_filename << std::endl;
		return false;
		
	}
	ftest.close();
	
	// Set the flag for taking the data from a list file
	flag_fit_peaks = false;
	
	return true;
	
}

void MiniballAngleFitter::Initialise() {
	
	// Setup the fit function
	ff = MiniballAngleFunction( myset, myreact );
	
	// Resize the vectors for parameters and limits
	npars = 1 + 4 * myset->GetNumberOfMiniballClusters();
	pars.resize( npars );
	names.resize( npars );
	LL.resize( npars );
	UL.resize( npars );
	
	// set the initial velocity from the reaction file
	pars[0] = myreact->GetBeta();
	names[0] = "beta";
	LL[0] = 0.01;
	UL[0] = 0.5;
	
	// Set the initial guesses for angles of each cluster from the reaction file
	for( unsigned int clu = 0; clu < myset->GetNumberOfMiniballClusters(); ++clu ) {

		// work out the parameter numbers
		int indx = 1 + 4 * clu;
		
		// get starting guesses from the reaction file
		pars[indx+0]	= myreact->GetMiniballTheta(clu) * TMath::RadToDeg();
		pars[indx+1]	= myreact->GetMiniballPhi(clu) * TMath::RadToDeg();
		pars[indx+2]	= myreact->GetMiniballAlpha(clu) * TMath::RadToDeg();
		pars[indx+3]	= myreact->GetMiniballR(clu);
		
		// Names of the parameters
		names[indx+0]	= "theta_" + std::to_string(clu);
		names[indx+1]	= "phi_"   + std::to_string(clu);
		names[indx+2]	= "alpha_" + std::to_string(clu);
		names[indx+3]	= "r_"     + std::to_string(clu);

		// Lower limits
		// these may cause issue if we need to cross over the 0/360 boundary
		LL[indx+0]	= 0.0;
		LL[indx+1]	= 0.0;
		LL[indx+2]	= 0.0;
		LL[indx+3]	= 10.0;
		
		// Upper limits
		UL[indx+0]	= 180.0;
		UL[indx+1]	= 360.0;
		UL[indx+2]	= 360.0;
		UL[indx+3]	= 400.0;

	}
	
}

void MiniballAngleFitter::DoFit() {
	
	// First we need to fit all of the segment energies
	// or read them in from a file
	if( flag_fit_peaks ) ff.FitSegmentEnergies( input_root_file );
	else ff.LoadExpEnergies( input_data_filename );
		
	// Create minimizer
	ROOT::Math::Minimizer *min =
		ROOT::Math::Factory::CreateMinimizer("Minuit2", "Migrad");
	
	// Create function for the fitting
	ROOT::Math::Functor f_init( ff, npars );
	
	// Some fit controls
	min->SetErrorDef(1.);
	min->SetMaxFunctionCalls(1000);
	min->SetMaxIterations(1000);
	min->SetTolerance(0.001);
	min->SetPrecision(1e-6);
	min->SetFunction(f_init);

	// Set limits in fit
	for( unsigned int i = 0; i < npars; ++i ) {
		min->SetLimitedVariable( i, names.at(i), pars.at(i), 0.0001, LL.at(i), UL.at(i) );
		min->SetVariableStepSize( i, 1.0 );
	}
	
	// Set the variable step size for beta
	min->SetVariableStepSize( 0, 0.001 );

	// Call the minimisation procedure
	min->Minimize();
	
	// Print the results to the terminal
	min->PrintResults();
	
	// Copy results into reaction object
	double beta = min->X()[0];
	for( unsigned int clu = 0; clu < myset->GetNumberOfMiniballClusters(); ++clu ) {

		// work out the parameter numbers
		int indx = 1 + 4 * clu;
		
		// Setup the cluster again
		myreact->SetupCluster( clu, min->X()[indx], min->X()[indx+1], min->X()[indx+2], min->X()[indx+3], 0.0 );

	}
	
	// print fit + residuals to pdf
	TGraphErrors *engraph = new TGraphErrors();
	engraph->SetName("engraph");
	engraph->SetTitle("Experimental energies; Channel index; Energy [keV]");

	TGraph *calcgraph = new TGraph();
	calcgraph->SetName("calcgraph");
	engraph->SetTitle("Calculated energies; Channel index; Energy [keV]");

	TGraphErrors *resgraph = new TGraphErrors();
	resgraph->SetName("resgraph");
	engraph->SetTitle("Residuals; Channel index; Energy [keV]");

	TGraphErrors *corrgraph = new TGraphErrors();
	corrgraph->SetName("corrgraph");
	engraph->SetTitle("Doppler-corrected energies; Channel index; Energy [keV]");

	
	// Loop over clusters
	for( unsigned int clu = 0; clu < myset->GetNumberOfMiniballClusters(); ++clu ) {

		// Loop over crystals
		for( unsigned int cry = 0; cry < myset->GetNumberOfMiniballCrystals(); ++cry ) {

			// Loop over segments
			for( unsigned int seg = 0; seg < myset->GetNumberOfMiniballSegments(); ++seg ) {

				// Check if it's present
				if( !ff.IsPresent( clu, cry, seg ) ) continue;
				
				// Get Doppler shifted energy
				double theta = myreact->GetGammaTheta( clu, cry, seg );
				double edop = myreact->DopplerShift( ff.GetReferenceEnergy(),
													beta, TMath::Cos(theta) );
				double corr = ff.GetReferenceEnergy() / edop;
				
				// channel index
				double indx = seg;
				indx += cry * myset->GetNumberOfMiniballSegments();
				indx += clu * myset->GetNumberOfMiniballCrystals() * myset->GetNumberOfMiniballSegments();
				
				engraph->AddPoint( indx, ff.GetExpEnergy( clu, cry, seg ) );
				engraph->SetPointError( engraph->GetN()-1, 0, ff.GetExpError( clu, cry, seg ) );
				
				corrgraph->AddPoint( indx, ff.GetExpEnergy( clu, cry, seg ) * corr );
				corrgraph->SetPointError( engraph->GetN()-1, 0, ff.GetExpError( clu, cry, seg ) );
				
				calcgraph->AddPoint( indx, edop );
				
				resgraph->AddPoint( indx, edop - ff.GetExpEnergy( clu, cry, seg ) );
				resgraph->SetPointError( resgraph->GetN()-1, 0, ff.GetExpError( clu, cry, seg ) );

			} // seg
			
		} // cry
		
	} // clu
	
	// Draw the absolute experimental/calculated energies
	auto c1 = std::make_unique<TCanvas>();
	engraph->SetMarkerStyle(kFullCircle);
	engraph->SetMarkerSize(0.5);
	engraph->SetMarkerColor(kRed);
	engraph->SetLineColor(kRed);
	engraph->Draw("AP");
	//engraph->Write();
	calcgraph->SetMarkerStyle(kFullCircle);
	calcgraph->SetMarkerSize(0.5);
	calcgraph->Draw("P");
	//calcgraph->Write();

	// Add a legend
	auto leg = std::make_unique<TLegend>(0.1,0.75,0.3,0.9);
	leg->AddEntry(engraph, "Experimental energies");
	leg->AddEntry(calcgraph, "Calculated energies");
	leg->Draw();
	
	// Save first plot as a PDF
	gErrorIgnoreLevel = kWarning;
	c1->Print("position_cal.pdf(");
	
	// Draw the residuals
	resgraph->SetMarkerStyle(kFullCircle);
	resgraph->SetMarkerSize(0.5);
	resgraph->Draw("AP");
	//resgraph->Write();
	auto func = std::make_unique<TF1>("func", "[0]", 0, 168);
	func->SetParameter( 0, 0.0 );
	func->Draw("same");
	
	// Save second plot as a PDF
	c1->Print("position_cal.pdf");
	
	// Draw the corrected energies compared to reference
	corrgraph->SetMarkerStyle(kFullCircle);
	corrgraph->SetMarkerSize(0.5);
	corrgraph->Draw("AP");
	//corrgraph->Write();
	func->SetParameter( 0, ff.GetReferenceEnergy() );
	func->Draw("same");
	
	// Save third plot as a PDF
	c1->Print("position_cal.pdf");
	
	// Define a colour scheme for the next bit
	int colors[8] = {632, 416, 600, 400, 616, 432, 800, 900};

	// A multi-graph showing all the positions in theta-phi, xy, xz, etc
	auto tp_mg = std::make_unique<TMultiGraph>();
	auto xy_f_mg = std::make_unique<TMultiGraph>();
	auto xy_b_mg = std::make_unique<TMultiGraph>();
	auto xz_r_mg = std::make_unique<TMultiGraph>();
	auto xz_l_mg = std::make_unique<TMultiGraph>();
	tp_mg->SetName("theta_phi_map");
	xy_f_mg->SetName("xy_f_mg");
	xy_b_mg->SetName("xy_b_mg");
	xz_r_mg->SetName("xz_r_mg");
	xz_l_mg->SetName("xz_l_mg");
	std::vector< std::vector<TGraph*> > theta_phi;
	std::vector< std::vector<TGraph*> > xy_f;
	std::vector< std::vector<TGraph*> > xy_b;
	std::vector< std::vector<TGraph*> > xz_l;
	std::vector< std::vector<TGraph*> > xz_r;
	theta_phi.resize( myset->GetNumberOfMiniballClusters() );
	xy_f.resize( myset->GetNumberOfMiniballClusters() );
	xy_b.resize( myset->GetNumberOfMiniballClusters() );
	xz_l.resize( myset->GetNumberOfMiniballClusters() );
	xz_r.resize( myset->GetNumberOfMiniballClusters() );

	// Loop over clusters
	for( unsigned int clu = 0; clu < myset->GetNumberOfMiniballClusters(); ++clu ) {

		theta_phi[clu].resize( myset->GetNumberOfMiniballCrystals() );
		xy_f[clu].resize( myset->GetNumberOfMiniballCrystals() );
		xy_b[clu].resize( myset->GetNumberOfMiniballCrystals() );
		xz_l[clu].resize( myset->GetNumberOfMiniballCrystals() );
		xz_r[clu].resize( myset->GetNumberOfMiniballCrystals() );

		// Loop over crystals
		for( unsigned int cry = 0; cry < myset->GetNumberOfMiniballCrystals(); ++cry ) {

			// Make the graphs to prevent any issues
			theta_phi[clu][cry] = new TGraph();
			xy_f[clu][cry] = new TGraph();
			xy_b[clu][cry] = new TGraph();
			xz_l[clu][cry] = new TGraph();
			xz_r[clu][cry] = new TGraph();
			
			// But then skip if there's no data
			if( !ff.IsPresent( clu ) ) continue;

			// Loop over segments
			for( unsigned int seg = 0; seg < myset->GetNumberOfMiniballSegments(); ++seg ) {

				// Get position
				double theta = myreact->GetGammaTheta( clu, cry, seg ) * TMath::RadToDeg();
				double phi = myreact->GetGammaPhi( clu, cry, seg ) * TMath::RadToDeg();
				double x = myreact->GetGammaX( clu, cry, seg );
				double y = myreact->GetGammaY( clu, cry, seg );
				double z = myreact->GetGammaZ( clu, cry, seg );

				// Plot positions
				theta_phi[clu][cry]->AddPoint( theta, phi );
				if( z > 0 ) xy_f[clu][cry]->AddPoint( y, x );
				else xy_b[clu][cry]->AddPoint( y, x );
				if( y > 0 ) xz_r[clu][cry]->AddPoint( z, x );
				else xz_l[clu][cry]->AddPoint( z, x );
				
			} // seg
			
			// Set colours etc
			theta_phi[clu][cry]->SetMarkerSize(1.0);
			theta_phi[clu][cry]->SetMarkerStyle(kFullCircle);
			theta_phi[clu][cry]->SetMarkerColor(colors[clu]+cry);
			xy_f[clu][cry]->SetMarkerSize(1.0);
			xy_f[clu][cry]->SetMarkerStyle(kFullCircle);
			xy_f[clu][cry]->SetMarkerColor(colors[clu]+cry);
			xy_b[clu][cry]->SetMarkerSize(1.0);
			xy_b[clu][cry]->SetMarkerStyle(kFullCircle);
			xy_b[clu][cry]->SetMarkerColor(colors[clu]+cry);
			xz_r[clu][cry]->SetMarkerSize(1.0);
			xz_r[clu][cry]->SetMarkerStyle(kFullCircle);
			xz_r[clu][cry]->SetMarkerColor(colors[clu]+cry);
			xz_l[clu][cry]->SetMarkerSize(1.0);
			xz_l[clu][cry]->SetMarkerStyle(kFullCircle);
			xz_l[clu][cry]->SetMarkerColor(colors[clu]+cry);
			
			// Add to multi-graph
			if( theta_phi[clu][cry]->GetN() > 0 ) tp_mg->Add( theta_phi[clu][cry] );
			if( xy_f[clu][cry]->GetN() > 0 ) xy_f_mg->Add( xy_f[clu][cry] );
			if( xy_b[clu][cry]->GetN() > 0 ) xy_b_mg->Add( xy_b[clu][cry] );
			if( xz_r[clu][cry]->GetN() > 0 ) xz_r_mg->Add( xz_r[clu][cry] );
			if( xz_l[clu][cry]->GetN() > 0 ) xz_l_mg->Add( xz_l[clu][cry] );

		} // cry
		
		// Skip if there's no data
		if( !ff.IsPresent( clu ) ) continue;
		
	} // clu
	
	// Draw the multigraph for theta-phi
	tp_mg->Draw("AP");
	tp_mg->GetXaxis()->SetLimits(0,180);
	tp_mg->GetYaxis()->SetLimits(-180,180);
	tp_mg->GetXaxis()->SetTitle("Reaction Theta [deg]");
	tp_mg->GetYaxis()->SetTitle("Reaction Phi [deg]");
	//tp_mg->Write();
	c1->Print("position_cal.pdf");
	
	// Draw the multigraph for xy-forward
	xy_f_mg->Draw("AP");
	xy_f_mg->GetYaxis()->SetTitle("x [mm]");
	xy_f_mg->GetXaxis()->SetTitle("y [mm]");
	//xy_f_mg->Write();
	c1->Print("position_cal.pdf");

	// Draw the multigraph for xy-backwards
	xy_b_mg->Draw("AP");
	xy_b_mg->GetYaxis()->SetTitle("x [mm]");
	xy_b_mg->GetXaxis()->SetTitle("y [mm]");
	//xy_b_mg->Write();
	c1->Print("position_cal.pdf");

	// Draw the multigraph for xz-right
	xz_r_mg->Draw("AP");
	xz_r_mg->GetYaxis()->SetTitle("x [mm]");
	xz_r_mg->GetXaxis()->SetTitle("z [mm]");
	//xz_r_mg->Write();
	c1->Print("position_cal.pdf");
	
	// Draw the multigraph for xz-left
	xz_l_mg->Draw("AP");
	xz_l_mg->GetYaxis()->SetTitle("x [mm]");
	xz_l_mg->GetXaxis()->SetTitle("z [mm]");
	//xz_l_mg->Write();
	c1->Print("position_cal.pdf)");
	gErrorIgnoreLevel = kInfo;

	// Print final results to terminal
	std::cout << "fitted beta = " << beta << std::endl;
	printf("       theta       phi     alpha         R\n");
	for( unsigned int clu = 0; clu < myset->GetNumberOfMiniballClusters(); ++clu ) {

		if( !ff.IsPresent(clu) ) continue;
		int indx = 1 + 4*clu;
		printf("MB%i   %6.2f    %6.2f    %6.2f    %6.2f\n", clu, min->X()[indx], min->X()[indx+1], min->X()[indx+2], min->X()[indx+3]);

	}
	std::cout << std::endl << std::endl;
	
	// Print the angles in the reaction file format
	myreact->PrintReaction( std::cout, "a" );

	return;
	
}

void MiniballAngleFitter::SaveReactionFile( std::string fname ){
	
	// Output
	std::ofstream react_file;
	react_file.open( fname, std::ios::out );
	if( react_file.is_open() ) {

		myreact->PrintReaction( react_file, "ae" );
		react_file.close();

	}

	else std::cerr << "Couldn't open " << fname << std::endl;
	
}
