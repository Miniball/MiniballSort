// This code will take a reaction file and calculate the energies of
// the quadruple alpha source for each ring of the CD.
//
// Written by Liam Gaffney (liam.gaffney@liverpool.ac.uk) - 18/07/2025
//
// To run you need to do the following in ROOT:
// root [0]: .L calculate_alpha_energies.cc
// root [1]: calculate_alpha_energies( "reaction.dat", "settings.dat" )
// where the settings and reaction files match those from your experiment

#include "../include/Settings.hh"
#include "../include/Calibration.hh"
#include "../include/Reaction.hh"
#include "TRandom3.h"
#include "TF1.h"
#include "TH1.h"
#include <vector>

R__LOAD_LIBRARY(libmb_sort.so)

// List of alpha energies
std::vector<double> AlphaEnergy;

double single_alpha( double *x, double *par ){

	double gaus = TMath::Exp( -0.5 * TMath::Power( ( x[0]-par[1] ) / par[2], 2.0 ) );
	gaus *= par[0] / ( TMath::Sqrt( TMath::TwoPi() ) * par[2] );
	return gaus;

}

double spectrum_fit( double *x, double *par ){

	double total = 0;
	for( unsigned int i = 0; i < AlphaEnergy.size(); i++ )
		total += single_alpha( x, &par[i*3] );

	return total;

}

void simulate_alpha_spectrum_pad( std::string reactionfile = "default", std::string settingsfile = "default" ) {

	// Settings file - needed for reaction file
	std::shared_ptr<MiniballSettings> myset = std::make_shared<MiniballSettings>( settingsfile );
	
	// Reaction file
	std::shared_ptr<MiniballReaction> myreact = std::make_shared<MiniballReaction>( reactionfile, myset );

	// Define the alpha energies
	AlphaEnergy.push_back( 3182.69 );	// % 148Gd
	AlphaEnergy.push_back( 5148.31 );	// % 239Pu
	AlphaEnergy.push_back( 5478.62 );	// % 241Am
	AlphaEnergy.push_back( 5795.04 );	// % 244Cm

	// Get distance and dead layers
	double cd_dist = myreact->GetCDDistance(0);
	double pad_dist = cd_dist + 10.0; // mm
	double cd_dead = myreact->GetCDDeadLayer(0);
	//double cd_dist = 28.5; // manual hack [mm] (doesn't work yet, see below)
	//double cd_dead = 0.0007; // manual hack [mm] (doesn't work, see below)
	//myreact->SetCDDistance( 0, cd_dist ); // not yet implemented in MiniballSort
	//myreact->SetCDDeadLayer( 0, cd_dead ); // not yet implemented in MiniballSort

	// Get the stopping powers
	std::unique_ptr<TGraph> g = std::make_unique<TGraph>();
	myreact->ReadStoppingPowers( "4He", "Si", g );

	// Make a histogram
	TH1F *hpad = new TH1F( "hpad", "alphas in pad including energy loss", 14000, 0, 7000 );

	// Calculate PAD angles
	double low_theta = TMath::ATan( 9.0 / pad_dist );
	double upp_theta = TMath::ATan( 41.0 / pad_dist );

	// Randomise over theta between 0˚ and 90˚, assuming symmetry in phi
	double Npoints = 1e6;
	double eres = 10.0; // keV detector resolutions
	TRandom3 rand;
	for( unsigned int i = 0; i < Npoints; i++ ){

		// Get theta and check it's in the pad range
		double theta = TMath::ACos( 1.0 - rand.Rndm() );
		if( theta < low_theta || theta > upp_theta ) continue;

		// Effective thickness
		double thick = cd_dead / TMath::Abs( TMath::Cos( theta ) );

		// Loop over alpha energies and get energy for each
		for( unsigned int j = 0; j < AlphaEnergy.size(); j++ ){

			// Calculate energy loss
			double eloss = myreact->GetEnergyLoss( AlphaEnergy[j], thick, g );
			double edet = AlphaEnergy[j] - eloss;

			// Add some Gaussian broadening
			hpad->Fill( rand.Gaus( edet, eres ) );

		} // j

	} // i

	// Make a canvas and draw histogram
	TCanvas *c1 = new TCanvas();
	hpad->Draw("hist");

	// Define the fit
	TF1 *fitalphas = new TF1( "fitalphas", spectrum_fit, 0, 7000, 3*AlphaEnergy.size() );
	for( unsigned int j = 0; j < AlphaEnergy.size(); j++ ) {
		fitalphas->SetParameter( 3*j+0, Npoints );
		fitalphas->SetParameter( 3*j+1, AlphaEnergy[j] );
		fitalphas->SetParameter( 3*j+2, eres );
	}

	// Fit the histogram
	hpad->Fit( fitalphas, "WLR" );
	fitalphas->SetNpx(1e5);
	fitalphas->Draw("same L");

	// Print the results
	std::cout << "Alpha Energy (keV)\tDetected Energy (keV)" << std::endl;
	for( unsigned int j = 0; j < AlphaEnergy.size(); j++ )
		std::cout << AlphaEnergy[j] << "\t" << fitalphas->GetParameter(j*3+1) << std::endl;

	c1->Print("pad_spectrum.pdf");

	return;
	
}

