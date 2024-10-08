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
	phiconst.resize( myset->GetNumberOfMiniballClusters() );
	energy.resize( myset->GetNumberOfMiniballClusters() );
	err.resize( myset->GetNumberOfMiniballClusters() );
	phic.resize( myset->GetNumberOfMiniballClusters() );
	phie.resize( myset->GetNumberOfMiniballClusters() );
	for( unsigned int clu = 0; clu < myset->GetNumberOfMiniballClusters(); ++clu ) {
		
		cluster.push_back(false);
		present[clu].resize( myset->GetNumberOfMiniballCrystals() );
		phiconst[clu].resize( myset->GetNumberOfMiniballCrystals() );
		energy[clu].resize( myset->GetNumberOfMiniballCrystals() );
		err[clu].resize( myset->GetNumberOfMiniballCrystals() );
		phic[clu].resize( myset->GetNumberOfMiniballCrystals() );
		phie[clu].resize( myset->GetNumberOfMiniballCrystals() );

		// Loop over crystals
		for( unsigned int cry = 0; cry < myset->GetNumberOfMiniballCrystals(); ++cry ) {
			
			present[clu][cry].resize( myset->GetNumberOfMiniballSegments() );
			phiconst[clu][cry].resize( myset->GetNumberOfMiniballSegments() );
			energy[clu][cry].resize( myset->GetNumberOfMiniballSegments() );
			err[clu][cry].resize( myset->GetNumberOfMiniballSegments() );
			phic[clu][cry].resize( myset->GetNumberOfMiniballSegments() );
			phie[clu][cry].resize( myset->GetNumberOfMiniballSegments() );

			// Loop over segments
			for( unsigned int seg = 0; seg < myset->GetNumberOfMiniballSegments(); ++seg ) {
				
				present[clu][cry].push_back(false);
				phiconst[clu][cry].push_back(false);
				energy[clu][cry].push_back(0.0);
				err[clu][cry].push_back(9.9);
				phic[clu][cry].push_back(0.0);
				phie[clu][cry].push_back(9.9);

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

	// Loop over the whole file line-by-line
	std::string data_line;
	while( std::getline( energyfile, data_line ) ) {

		// Make a string stream to get the data
		std::stringstream data_stream( data_line );
		
		// And the package it up in to a vector
		int parsed_int;
		double parsed_double;
		std::vector<int> detid;
		std::vector<double> data;
		
		// Just the detector id first
		while( data_stream >> parsed_int ) {

			detid.push_back( parsed_int );
			if( detid.size() == 3 ) break;
			
		}
		
		// If we didn't get a full ID, skip it
		if( detid.size() != 3 ) continue;
		
		// Otherwise we can split the id
		int clu = detid[0], cry = detid[1], seg = detid[2];
		
		// Check the cluster number is sensible
		if( clu >= (int)myset->GetNumberOfMiniballClusters() || clu < 0 ) {
			
			std::cerr << "Bad cluster number = " << clu << std::endl;
			continue;
			
		}

		// Check the crystal number is sensible
		if( cry >= (int)myset->GetNumberOfMiniballCrystals() || cry < 0 ) {
			
			std::cerr << "Bad crystal number = " << cry << std::endl;
			continue;
			
		}

		// Check the segment number is sensible
		if( seg >= (int)myset->GetNumberOfMiniballSegments() || seg < 0 ) {
			
			std::cerr << "Bad segment number = " << seg << std::endl;
			continue;
			
		}
		
		// Then let's take the data for that segment
		while( data_stream >> parsed_double )
			data.push_back( parsed_double );
		
		// We should have 2 or 4 items
		if( data.size() != 2 && data.size() != 4 ) continue;
		
		// Then we at least have the energy
		present[clu][cry][seg] = true;
		energy[clu][cry][seg] = data[0];
		err[clu][cry][seg] = data[1];
		cluster[clu] = true;
		
		// If we have a phi constraint, we have 4 data items
		if( data.size() == 4 ) {
			
			phiconst[clu][cry][seg] = true;
			phic[clu][cry][seg] = data[2];
			phie[clu][cry][seg] = data[3];
			
		}

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

	// Loop over clusters
	for( unsigned int clu = 0; clu < myset->GetNumberOfMiniballClusters(); ++clu ) {

		int indx = 1 + 4*clu;
		myreact->SetupCluster( clu, p[indx], p[indx+1], p[indx+2], p[indx+3], user_z );

	}
	
	double chisq = 0;
	
	// Loop over clusters
	for( unsigned int clu = 0; clu < myset->GetNumberOfMiniballClusters(); ++clu ) {

		// Loop over crystals
		for( unsigned int cry = 0; cry < myset->GetNumberOfMiniballCrystals(); ++cry ) {

			// Loop over segments
			for( unsigned int seg = 0; seg < myset->GetNumberOfMiniballSegments(); ++seg ) {
				
				// Only include segments with data
				if( !present[clu][cry][seg] ) continue;
				
				// Get the theta and phi of the reaction
				double theta = myreact->GetGammaTheta( clu, cry, seg );
				double phi = myreact->GetGammaPhi( clu, cry, seg );
				
				// Phi should be compared to user input, which will be 0˚-360˚
				phi *= TMath::RadToDeg();
				if( phi < 0 ) phi += 360.;
				
				// Zeroth parameter is beta
				double beta = p[0];
				
				// Doppler corrected energy
				double edop = myreact->DopplerShift( eref, beta, TMath::Cos(theta) );
				
				// Compare to fitted energy, increase chisq value
				chisq += TMath::Power( ( energy[clu][cry][seg] - edop ) / err[clu][cry][seg], 2.0 );

				// Check if we have a phi constraint for this segment
				if( !phiconst[clu][cry][seg] ) continue;

				// If so, then increase chisq value
				chisq += TMath::Power( ( phic[clu][cry][seg] - phi ) / phie[clu][cry][seg], 2.0 );
				
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
	LL[0] = myreact->GetBeta() * 0.5;
	UL[0] = myreact->GetBeta() * 1.2;
	
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
		LL[indx+0]	= pars[indx+0] - 60.0;
		LL[indx+1]	= pars[indx+1] - 60.0;
		LL[indx+2]	= 0.0;
		LL[indx+3]	= 50.0;
		
		// Upper limits
		UL[indx+0]	= pars[indx+0] + 60.0;
		UL[indx+1]	= pars[indx+1] + 60.0;
		UL[indx+2]	= 360.0;
		UL[indx+3]	= 180.0;
		
		// Couple of consistency checks
		if( LL[indx+0] < 0.0 ) LL[indx+0] = 0.0;
		if( LL[indx+1] < 0.0 ) LL[indx+1] = 0.0;
		if( UL[indx+0] > 360.0 ) UL[indx+0] = 360.0;
		if( UL[indx+1] > 360.0 ) UL[indx+1] = 360.0;

	}
	
	// Some colours and marker styles
	mystyles.push_back( kFullCircle );
	mystyles.push_back( kFullSquare );
	mystyles.push_back( kFullTriangleUp );
	mystyles.push_back( kFullTriangleDown );
	mystyles.push_back( kFullCross );
	mystyles.push_back( kFullStar );
	mystyles.push_back( kFullDiamond );
	mystyles.push_back( kFullCrossX );
	mystyles.push_back( kFullFourTrianglesX );
	mystyles.push_back( kFullThreeTriangles );
	mystyles.push_back( kFullDoubleDiamond );
	mystyles.push_back( kFourSquaresX );
	mystyles.push_back( kFourSquaresPlus );
	mycolors.push_back( kRed+1 );
	mycolors.push_back( kBlue+1 );
	mycolors.push_back( kGreen+1 );
	mycolors.push_back( kMagenta+1 );
	mycolors.push_back( kYellow+1 );
	mycolors.push_back( kCyan+1 );

}

void MiniballAngleFitter::DoFit() {
	
	// First we need to fit all of the segment energies
	// or read them in from a file
	if( flag_fit_peaks ) ff.FitSegmentEnergies( input_root_file );
	else ff.LoadExpEnergies( input_data_filename );
		
	// Create minimizer
	ROOT::Math::Minimizer *min =
		ROOT::Math::Factory::CreateMinimizer("Minuit2", "Combined");

	// Set print level
	//min->SetPrintLevel(1);
	
	// Create function for the fitting
	ROOT::Math::Functor f_init( ff, npars );
	
	// Some fit controls
	min->SetErrorDef(1.);
	min->SetMaxFunctionCalls(1e7);
	min->SetMaxIterations(1e8);
	min->SetPrecision(1e-12);
	min->SetTolerance(1e-9);
	min->SetStrategy(2); // 0: low, 1: medium, 2: high
	min->SetFunction(f_init);

	// Set limits in fit
	for( unsigned int i = 0; i < npars; ++i ) {
		min->SetLimitedVariable( i, names.at(i), pars.at(i), 0.0001, LL.at(i), UL.at(i) );
		min->SetVariableStepSize( i, 1.0 );
	}
	
	// Set the variable step size for beta
	min->SetVariableStepSize( 0, 0.001 );
	
	// If a cluster is missing, fix the parameters
	for( unsigned int clu = 0; clu < myset->GetNumberOfMiniballClusters(); ++clu ) {

		int indx = 1 + 4*clu;
		if( !ff.IsPresent(clu) ){

			min->FixVariable( indx+0 );
			min->FixVariable( indx+1 );
			min->FixVariable( indx+2 );
			min->FixVariable( indx+3 );

		}
		
		// Uncomment below to fix phi
		//min->FixVariable( indx+1 );
		
	} // clu

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
	calcgraph->SetTitle("Calculated energies; Channel index; Energy [keV]");

	TGraphErrors *resgraph = new TGraphErrors();
	resgraph->SetName("resgraph");
	resgraph->SetTitle("Residuals; Channel index; Energy [keV]");

	TGraphErrors *corrgraph = new TGraphErrors();
	corrgraph->SetName("corrgraph");
	corrgraph->SetTitle("Doppler-corrected energies; Channel index; Energy [keV]");
	
	TGraphErrors *phigraph = new TGraphErrors();
	phigraph->SetName("phigraph");
	phigraph->SetTitle("Phi residuals; Channel index; Energy [keV]");
	

	// Loop over clusters
	for( unsigned int clu = 0; clu < myset->GetNumberOfMiniballClusters(); ++clu ) {

		// Loop over crystals
		for( unsigned int cry = 0; cry < myset->GetNumberOfMiniballCrystals(); ++cry ) {

			// Loop over segments
			for( unsigned int seg = 0; seg < myset->GetNumberOfMiniballSegments(); ++seg ) {

				// Check if it's present
				if( !ff.IsPresent( clu, cry, seg ) ) continue;
				
				// Get Doppler shifted energy, theta and phi
				double theta = myreact->GetGammaTheta( clu, cry, seg );
				double phi = myreact->GetGammaPhi( clu, cry, seg );
				phi *= TMath::RadToDeg();
				if( phi < 0.0 ) phi += 360.0;
				double edop = myreact->DopplerShift( ff.GetReferenceEnergy(),
													beta, TMath::Cos(theta) );
				double corr = ff.GetReferenceEnergy() / edop;

				// channel index
				double indx = seg;
				indx += cry * myset->GetNumberOfMiniballSegments();
				indx += clu * myset->GetNumberOfMiniballCrystals() * myset->GetNumberOfMiniballSegments();
				
				engraph->SetPoint(engraph->GetN(), indx, ff.GetExpEnergy( clu, cry, seg ) );
				engraph->SetPointError( engraph->GetN()-1, 0, ff.GetExpError( clu, cry, seg ) );
				
				corrgraph->SetPoint(corrgraph->GetN(), indx, ff.GetExpEnergy( clu, cry, seg ) * corr );
				corrgraph->SetPointError( engraph->GetN()-1, 0, ff.GetExpError( clu, cry, seg ) );
				
				calcgraph->SetPoint(calcgraph->GetN(), indx, edop );
				
				resgraph->SetPoint(resgraph->GetN(), indx, edop - ff.GetExpEnergy( clu, cry, seg ) );
				resgraph->SetPointError( resgraph->GetN()-1, 0, ff.GetExpError( clu, cry, seg ) );

				// Get experimental phi
				if( !ff.HasPhiConstraint( clu, cry, seg ) ) continue;
				phigraph->SetPoint(phigraph->GetN(), indx, phi - ff.GetExpPhi( clu, cry, seg ) );
				phigraph->SetPointError( phigraph->GetN()-1, 0, ff.GetExpPhiError( clu, cry, seg ) );

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
	c1->Print("position_cal.pdf(","pdf");
	
	// Draw the residuals
	resgraph->SetMarkerStyle(kFullCircle);
	resgraph->SetMarkerSize(0.5);
	resgraph->Draw("AP");
	//resgraph->Write();
	auto func = std::make_unique<TF1>("func", "[0]", 0, 168);
	func->SetParameter( 0, 0.0 );
	func->Draw("same");
	
	// Save second plot as a PDF
	c1->Print("position_cal.pdf","pdf");
	
	// Draw the corrected energies compared to reference
	corrgraph->SetMarkerStyle(kFullCircle);
	corrgraph->SetMarkerSize(0.5);
	corrgraph->Draw("AP");
	//corrgraph->Write();
	func->SetParameter( 0, ff.GetReferenceEnergy() );
	func->Draw("same");
	
	// Save third plot as a PDF
	c1->Print("position_cal.pdf","pdf");
	
	// Draw the phi residuals
	phigraph->SetMarkerStyle(kFullCircle);
	phigraph->SetMarkerSize(0.5);
	phigraph->Draw("AP");
	func->SetParameter( 0, 0.0 );
	func->Draw("same");

	// Save fourth plot as a PDF
	c1->Print("position_cal.pdf","pdf");

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

	// Add a legend and make space for it on the pad
	std::string leg_lab;
	auto leg2 = std::make_unique<TLegend>( 0.79, 0.15, 0.95, 0.90 );
	auto p1 = c1->cd();
	p1->SetMargin( 0.15, 0.21, 0.15, 0.10 );
	
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
			
			leg_lab = "MB" + std::to_string(clu) + static_cast<char>(cry+65);
			leg2->AddEntry( theta_phi[clu][cry], leg_lab.data() );

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
				theta_phi[clu][cry]->SetPoint(theta_phi[clu][cry]->GetN(), theta, phi );
				if( z > 0 ) xy_f[clu][cry]->SetPoint(xy_f[clu][cry]->GetN(), y, x );
				else xy_b[clu][cry]->SetPoint(xy_b[clu][cry]->GetN(), y, x );
				if( y > 0 ) xz_r[clu][cry]->SetPoint(xz_r[clu][cry]->GetN(), z, x );
				else xz_l[clu][cry]->SetPoint(xz_l[clu][cry]->GetN(), z, x );
				
			} // seg
			
			// Set colours etc
			theta_phi[clu][cry]->SetMarkerSize(1.0);
			theta_phi[clu][cry]->SetMarkerStyle(mystyles[clu]);
			theta_phi[clu][cry]->SetMarkerColor(mycolors[cry]);
			xy_f[clu][cry]->SetMarkerSize(1.0);
			xy_f[clu][cry]->SetMarkerStyle(mystyles[clu]);
			xy_f[clu][cry]->SetMarkerColor(mycolors[cry]);
			xy_b[clu][cry]->SetMarkerSize(1.0);
			xy_b[clu][cry]->SetMarkerStyle(mystyles[clu]);
			xy_b[clu][cry]->SetMarkerColor(mycolors[cry]);
			xz_r[clu][cry]->SetMarkerSize(1.0);
			xz_r[clu][cry]->SetMarkerStyle(mystyles[clu]);
			xz_r[clu][cry]->SetMarkerColor(mycolors[cry]);
			xz_l[clu][cry]->SetMarkerSize(1.0);
			xz_l[clu][cry]->SetMarkerStyle(mystyles[clu]);
			xz_l[clu][cry]->SetMarkerColor(mycolors[cry]);
			
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
	tp_mg->GetXaxis()->SetLimits(0,180);
	tp_mg->GetYaxis()->SetLimits(-180,180);
	tp_mg->GetXaxis()->SetTitle("Reaction Theta [deg]");
	tp_mg->GetYaxis()->SetTitle("Reaction Phi [deg]");
	tp_mg->Draw("AP");
	leg2->Draw();
	//tp_mg->Write();
	c1->Print("position_cal.pdf","pdf");
	
	// Draw the multigraph for xy-forward
	xy_f_mg->GetYaxis()->SetTitle("x [mm]");
	xy_f_mg->GetXaxis()->SetTitle("y [mm]");
	xy_f_mg->Draw("AP");
	leg2->Draw();
	//xy_f_mg->Write();
	c1->Print("position_cal.pdf","pdf");

	// Draw the multigraph for xy-backwards
	xy_b_mg->GetYaxis()->SetTitle("x [mm]");
	xy_b_mg->GetXaxis()->SetTitle("y [mm]");
	xy_b_mg->Draw("AP");
	leg2->Draw();
	//xy_b_mg->Write();
	c1->Print("position_cal.pdf","pdf");

	// Draw the multigraph for xz-right
	xz_r_mg->GetYaxis()->SetTitle("x [mm]");
	xz_r_mg->GetXaxis()->SetTitle("z [mm]");
	xz_r_mg->Draw("AP");
	leg2->Draw();
	//xz_r_mg->Write();
	c1->Print("position_cal.pdf","pdf");
	
	// Draw the multigraph for xz-left
	xz_l_mg->GetYaxis()->SetTitle("x [mm]");
	xz_l_mg->GetXaxis()->SetTitle("z [mm]");
	xz_l_mg->Draw("AP");
	leg2->Draw();
	//xz_l_mg->Write();
	c1->Print("position_cal.pdf)","pdf");
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
