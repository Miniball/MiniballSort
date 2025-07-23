// This code will take a reaction file and calculate the energies of
// the quadruple alpha source for each ring of the CD.
//
// Written by Liam Gaffney (liam.gaffney@liverpool.ac.uk) - 18/07/2025
//
// To run you need to do the following in ROOT:
// root [0]: .L show_miniball_positions.cc
// root [1]: show_miniball_positions( "reaction.dat", "settings.dat" )
// where the settings and reaction files match those from your experiment

R__LOAD_LIBRARY(libmb_sort.so)

void calculate_alpha_energies( std::string reactionfile = "default", std::string settingsfile = "default" ) {

	// Settings file - needed for reaction file
	std::shared_ptr<MiniballSettings> myset = std::make_shared<MiniballSettings>( settingsfile );
	
	// Reaction file
	std::shared_ptr<MiniballReaction> myreact = std::make_shared<MiniballReaction>( reactionfile, myset );

	// Get distance and dead layers
	double cd_dist = myreact->GetCDDistance(0);
	double cd_dead = myreact->GetCDDeadLayer(0);
	//double cd_dist = 28.5; // manual hack [mm] (doesn't work yet, see below)
	//double cd_dead = 0.0007; // manual hack [mm] (doesn't work, see below)
	//myreact->SetCDDistance( 0, cd_dist ); // not yet implemented in MiniballSort
	//myreact->SetCDDeadLayer( 0, cd_dead ); // not yet implemented in MiniballSort

	// Get the stopping powers
	std::unique_ptr<TGraph> g = std::make_unique<TGraph>();
	myreact->ReadStoppingPowers( "4He", "Si", g );

	// Get the alpha energies
	std::vector<double> AlphaEnergy;
	AlphaEnergy.push_back( 3182.69 );	// % 148Gd
	AlphaEnergy.push_back( 5148.31 );	// % 239Pu
	AlphaEnergy.push_back( 5478.62 );	// % 241Am
	AlphaEnergy.push_back( 5795.04 );	// % 244Cm


	// Loop over angles
	for( unsigned int i = 0; i <  myset->GetNumberOfCDPStrips(); i++ ){

		// Get theta
		double theta = myreact->GetParticleTheta( 0, 0, i, 0 );

		// Effective thickness
		double thick = cd_dead / TMath::Abs( TMath::Cos( theta ) );

		std::cout << "Ring " << i << ", theta = " << theta*TMath::RadToDeg() << std::endl;

		// Loop over alpha energies
		for( unsigned int j = 0; j < AlphaEnergy.size(); j++ ){

			double eloss = myreact->GetEnergyLoss( AlphaEnergy[j], thick, g );

			std::cout << "\tE_" << j << ": " << AlphaEnergy[j] << " - " << eloss << " = ";
			std::cout << AlphaEnergy[j] - eloss << " keV" << std::endl;

		} // j

	} // i

	return;
	
}

