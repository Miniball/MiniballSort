#include "Reaction.hh"

ClassImp( MiniballParticle )
ClassImp( MiniballReaction )


// Reaction things
MiniballReaction::MiniballReaction( std::string filename, std::shared_ptr<MiniballSettings> myset ){
		
	// Read in mass tables
	ReadMassTables();

	// Get the info from the user input
	set = myset;
	SetFile( filename );
	ReadReaction();
	
}

void MiniballReaction::AddBindingEnergy( short Ai, short Zi, TString ame_be_str ) {
	
	// A key for the isotope
	std::string isotope_key;
	isotope_key = std::to_string( Ai ) + gElName.at( Zi );
	
	// Remove # from AME data and replace with decimal point
	if ( ame_be_str.Contains("#") )
		ame_be_str.ReplaceAll("#",".");

	// An * means there is no data, fill with a 0
	if ( ame_be_str.Contains("*") )
		ame_be.insert( std::make_pair( isotope_key, 0 ) );
	
	// Otherwise add the real data
	else
		ame_be.insert( std::make_pair( isotope_key, ame_be_str.Atof() ) );
	
	return;
	
}

void MiniballReaction::ReadMassTables() {

	// Input data file is in the source code
	// AME_FILE is passed as a definition at compilation time in Makefile
	std::ifstream input_file;
	input_file.open( AME_FILE );
	
	std::string line, BE_str, N_str, Z_str;
	std::string startline = "1N-Z";
	
	short Ai, Zi, Ni;

	// Loop over the file
	if( input_file.is_open() ){

		// Read first line
		std::getline( input_file, line );
		
		// Look for start of data
		while( line.substr( 0, startline.size() ) != startline ){
			
			// Read next line, but break if it's the end of the file
			if( !std::getline( input_file, line ) ){
				
				std::cout << "Can't read mass tables from ";
				std::cout << AME_FILE << std::endl;
				exit(1);
				
			}

		}
		
		// Read one more nonsense line with the units on
		std::getline( input_file, line );
		
		// Now process the data
		while( std::getline( input_file, line ) ){
			
			// Get mass excess from the line
			N_str = line.substr( 5, 5 );
			Z_str = line.substr( 9, 5 );
			BE_str = line.substr( 54, 13 );
			
			// Get N and Z
			Ni = std::stoi( N_str );
			Zi = std::stoi( Z_str );
			Ai = Ni + Zi;
			
			// Add mass value
			AddBindingEnergy( Ai, Zi, BE_str );
			
		}
		
	}
	
	else {
		
		std::cout << "Mass tables file doesn't exist: " << AME_FILE << std::endl;
		exit(1);
		
	}
	
	return;

}

void MiniballReaction::ReadReaction() {

	TEnv *config = new TEnv( fInputFile.data() );

	std::string isotope_key;

	// Get particle properties
	Beam.SetA( config->GetValue( "BeamA", 185 ) );
	Beam.SetZ( config->GetValue( "BeamZ", 80 ) );
	if( Beam.GetZ() < 0 || Beam.GetZ() >= (int)gElName.size() ){

		std::cout << "Not a recognised element with Z = ";
		std::cout << Beam.GetZ() << " (beam)" << std::endl;
		exit(1);

	}
	Beam.SetBindingEnergy( ame_be.at( Beam.GetIsotope() ) );
	Beam.SetEx( config->GetValue( "BeamEx", 0. ) );

	Eb = config->GetValue( "BeamE", 4500.0 ); // in keV per nucleon
	Eb *= Beam.GetMass_u(); // keV
	Beam.SetEnergy( Eb ); // keV

	Target.SetA( config->GetValue( "TargetA", 120 ) );
	Target.SetZ( config->GetValue( "TargetZ", 50 ) );
	Target.SetEnergy( 0.0 );
	if( Target.GetZ() < 0 || Target.GetZ() >= (int)gElName.size() ){

		std::cout << "Not a recognised element with Z = ";
		std::cout << Target.GetZ() << " (target)" << std::endl;
		exit(1);

	}
	Target.SetBindingEnergy( ame_be.at( Target.GetIsotope() ) );
	Target.SetEx( config->GetValue( "TargetEx", 0. ) );

	Ejectile.SetA( config->GetValue( "EjectileA", 185 ) );
	Ejectile.SetZ( config->GetValue( "EjectileZ", 80 ) );
	if( Ejectile.GetZ() < 0 || Ejectile.GetZ() >= (int)gElName.size() ){

		std::cout << "Not a recognised element with Z = ";
		std::cout << Ejectile.GetZ() << " (ejectile)" << std::endl;
		exit(1);

	}
	Ejectile.SetBindingEnergy( ame_be.at( Ejectile.GetIsotope() ) );
	Ejectile.SetEx( config->GetValue( "EjectileEx", 500. ) );

	Recoil.SetA( config->GetValue( "RecoilA", 120 ) );
	Recoil.SetZ( config->GetValue( "RecoilZ", 50 ) );
	if( Recoil.GetZ() < 0 || Recoil.GetZ() >= (int)gElName.size() ){

		std::cout << "Not a recognised element with Z = ";
		std::cout << Recoil.GetZ() << " (recoil)" << std::endl;
		exit(1);

	}
	Recoil.SetBindingEnergy( ame_be.at( Recoil.GetIsotope() ) );
	Recoil.SetEx( config->GetValue( "RecoilEx", 0. ) );

	// Get particle energy cut
	ejectilecutfile = config->GetValue( "EjectileCut.File", "NULL" );
	ejectilecutname = config->GetValue( "EjectileCut.Name", "CUTG" );
	recoilcutfile = config->GetValue( "RecoilCut.File", "NULL" );
	recoilcutname = config->GetValue( "RecoilCut.Name", "CUTG" );
	transfercutfile = config->GetValue( "TransferCut.File", "NULL" );
	transfercutname = config->GetValue( "TransferCut.Name", "CUTG" );
	transfercut_x = config->GetValue( "TransferCut.X", "E" );
	transfercut_y = config->GetValue( "TransferCut.Y", "dE" );

	// Check if ejectile/recoil/transfer cuts are given by the user
	ejectile_cut = ReadCutFile( ejectilecutfile, ejectilecutname );
	recoil_cut = ReadCutFile( recoilcutfile, recoilcutname );
	transfer_cut = ReadCutFile( transfercutfile, transfercutname );

	// Velocity calculation for Doppler correction
	doppler_mode = config->GetValue( "DopplerMode", 1 );

	// Laser mode
	laser_mode = config->GetValue( "LaserMode", 2 );

	// EBIS time window
	EBIS_On = config->GetValue( "EBIS.On", 1.2e6 );		// normally 1.2 ms in slow extraction
	EBIS_Off = config->GetValue( "EBIS.Off", 2.52e7 );	// this allows a off window 20 times bigger than on
	EBIS_ratio = config->GetValue( "EBIS.FillRatio", GetEBISTimeRatio() );	// this is the measured ratio of EBIS On/off. Default is just the time window ratio

	// T1 cuts
	t1_cut = config->GetValue( "T1.Cut", false );		// enable or disable the T1 cuts
	t1_time[0] = config->GetValue( "T1.Min", 0.0 );		// minimum T1 time for cut (ns), default 0
	t1_time[1] = config->GetValue( "T1.Max", 1.2e9 );	// maximum T1 time for cut (ns), default 1.2 seconds

	// Events tree options
	events_particle_gamma       = config->GetValue( "Events.ParticleGammaOnly", false );			// only do histogramming for particle-gamma coincidences to speed things up
	events_particle_cdpad_coinc = config->GetValue( "Events.CdPadCoincidence", false );				// only do histogramming for particles with CD-Pad coincidences
	events_particle_cdpad_veto  = config->GetValue( "Events.CdPadVeto", false );					// only do histogramming for particles without CD-Pad coincidences (Pad as veto)
	events_gamma_seg_energy     = config->GetValue( "Events.GammaUseSegmentEnergy", false );		// use the segment energy instead of the core energy for gamma-rays
	events_gamma_demand_seg     = config->GetValue( "Events.GammaWithSegment", false );				// only do histogramming for gamma-rays that have a segment, i.e. reject core-only gammas
	events_gamma_max_seg_mult   = config->GetValue( "Events.GammaMaxSegmentMultiplicity", 99 );		// only do histogramming for gamma-rays with a maximum segment multiplicity (all segments of a cluster = 18)
	events_gamma_seg_ediff      = config->GetValue( "Events.GammaCoreSegmentEnergyDifference", 9.9e9 );	// only do histogramming for gamma-rays where the core and sgement energies are less than this

	// Histogram options
	hist_wo_addback = config->GetValue( "Histograms.WithoutAddback", true );	// turn on histograms for gamma-rays without addback
	hist_w_addback = config->GetValue( "Histograms.WithAddback", false );	// turn on histograms for gamma-rays with addback
	hist_segment_phi = config->GetValue( "Histograms.SegmentPhi", false );	// turn on histograms for segment phi
	hist_by_crystal = config->GetValue( "Histograms.ByCrystal", false );	// turn on histograms for gamma-gamma
	hist_by_pmult = config->GetValue( "Histograms.ByMultiplicity", false );	// turn on particle-gamma(-electron) spectra by multiplicity, i.e. 1p and 2p spectra
	hist_by_sector = config->GetValue( "Histograms.BySector", false );	// turn on sector-by-sector histograms for particles
	hist_by_t1 = config->GetValue( "Histograms.ByT1", false );	// turn on histograms as a function of T1
	hist_gamma_gamma = config->GetValue( "Histograms.GammaGamma", true );	// turn on histograms for gamma-gamma
	hist_electron = config->GetValue( "Histograms.Electron", false );	// turn on histograms for electrons
	hist_electron_gamma = config->GetValue( "Histograms.ElectronGamma", false );	// turn on histograms for electron-gamma
	hist_beam_dump = config->GetValue( "Histograms.BeamDump", true );	// turn on histograms for beam dump
	hist_ion_chamb = config->GetValue( "Histograms.IonChamber", false );	// turn on histograms for ionisation chamber

	// Histogram ranges - gammas and electrons
	gamma_bins = config->GetValue( "Histograms.Gamma.Bins", 6000 );				// number of bins in gamma-ray spectra
	gamma_range[0] = config->GetValue( "Histograms.Gamma.Min", -0.5 );			// lower energy limit of gamma-ray spectra (keV)
	gamma_range[1] = config->GetValue( "Histograms.Gamma.Max", 5999.5 );		// upper energy limit of gamma-ray spectra (keV)
	electron_bins = config->GetValue( "Histograms.Electron.Bins", 2000 );		// number of bins in electron spectra
	electron_range[0] = config->GetValue( "Histograms.Electron.Min", -0.5 );	// lower energy limit of electron spectra (keV)
	electron_range[1] = config->GetValue( "Histograms.Electron.Max", 1999.5 );	// upper energy limit of electron spectra (keV)

	// Histogram ranges - particles
	double pmax_default = 2.0e6;
	if( Beam.GetIsotope() != Ejectile.GetIsotope() ) {
		if( Recoil.GetA() <= 12 ) pmax_default = 200e3;
		else if( Beam.GetEnergy() < 100e3 ) pmax_default = 200e3;
		else if( Beam.GetEnergy() < 200e3 ) pmax_default = 400e3;
		else if( Beam.GetEnergy() < 400e3 ) pmax_default = 800e3;
		else if( Beam.GetEnergy() < 800e3 ) pmax_default = 1600e3;
	}
	else {
		if( Beam.GetEnergy() < 100e3 ) pmax_default = 200e3;
		else if( Beam.GetEnergy() < 200e3 ) pmax_default = 400e3;
		else if( Beam.GetEnergy() < 400e3 ) pmax_default = 800e3;
		else if( Beam.GetEnergy() < 800e3 ) pmax_default = 1600e3;
		else if( Beam.GetEnergy() > 2000e3 ) pmax_default = 4000e3;
	}
	particle_bins = config->GetValue( "Histograms.Particle.Bins", 2000 );		// number of bins in particle spectra
	particle_range[0] = config->GetValue( "Histograms.Particle.Min", 0.0 );		// lower energy limit of particle spectra (keV)
	particle_range[1] = config->GetValue( "Histograms.Particle.Max", pmax_default );	// upper energy limit of particle spectra (keV)

	// Particle-Gamma time windows
	pg_prompt[0] = config->GetValue( "ParticleGamma_PromptTime.Min", -300 );	// lower limit for particle-gamma prompt time difference
	pg_prompt[1] = config->GetValue( "ParticleGamma_PromptTime.Max", 300 );		// upper limit for particle-gamma prompt time difference
	pg_random[0] = config->GetValue( "ParticleGamma_RandomTime.Min", 600 );		// lower limit for particle-gamma random time difference
	pg_random[1] = config->GetValue( "ParticleGamma_RandomTime.Max", 1200 );	// upper limit for particle-gamma random time difference
	gg_prompt[0] = config->GetValue( "GammaGamma_PromptTime.Min", -250 );		// lower limit for gamma-gamma prompt time difference
	gg_prompt[1] = config->GetValue( "GammaGamma_PromptTime.Max", 250 );		// upper limit for gamma-gamma prompt time difference
	gg_random[0] = config->GetValue( "GammaGamma_RandomTime.Min", 500 );		// lower limit for gamma-gamma random time difference
	gg_random[1] = config->GetValue( "GammaGamma_RandomTime.Max", 1000 );		// upper limit for gamma-gamma random time difference
	pp_prompt[0] = config->GetValue( "ParticleParticle_PromptTime.Min", -200 );	// lower limit for particle-particle prompt time difference
	pp_prompt[1] = config->GetValue( "ParticleParticle_PromptTime.Max", 200 );	// upper limit for particle-particle prompt time difference
	pp_random[0] = config->GetValue( "ParticleParticle_RandomTime.Min", 400 );	// lower limit for particle-particle random time difference
	pp_random[1] = config->GetValue( "ParticleParticle_RandomTime.Max", 800 );	// upper limit for particle-particle random time difference
	ee_prompt[0] = config->GetValue( "ElectronElectron_PromptTime.Min", -200 );	// lower limit for electron-electron prompt time difference
	ee_prompt[1] = config->GetValue( "ElectronElectron_PromptTime.Max", 200 );	// upper limit for electron-electron prompt time difference
	ee_random[0] = config->GetValue( "ElectronElectron_RandomTime.Min", 400 );	// lower limit for electron-electron random time difference
	ee_random[1] = config->GetValue( "ElectronElectron_RandomTime.Max", 800 );	// upper limit for electron-electron random time difference
	ge_prompt[0] = config->GetValue( "GammaElectron_PromptTime.Min", -200 );	// lower limit for gamma-electron prompt time difference
	ge_prompt[1] = config->GetValue( "GammaElectron_PromptTime.Max", 200 );		// upper limit for gamma-electron prompt time difference
	ge_random[0] = config->GetValue( "GammaElectron_RandomTime.Min", 400 );		// lower limit for gamma-electron random time difference
	ge_random[1] = config->GetValue( "GammaElectron_RandomTime.Max", 800 );		// upper limit for gamma-electron random time difference
	pe_prompt[0] = config->GetValue( "ParticleElectron_PromptTime.Min", -200 );	// lower limit for particle-electron prompt time difference
	pe_prompt[1] = config->GetValue( "ParticleElectron_PromptTime.Max", 200 );	// upper limit for particle-electron prompt time difference
	pe_random[0] = config->GetValue( "ParticleElectron_RandomTime.Min", 400 );	// lower limit for particle-electron random time difference
	pe_random[1] = config->GetValue( "ParticleElectron_RandomTime.Max", 800 );	// upper limit for particle-electron random time difference

	// Particle-Gamma fill ratios
	pg_ratio = config->GetValue( "ParticleGamma_FillRatio", GetParticleGammaTimeRatio() );
	gg_ratio = config->GetValue( "GammaGamma_FillRatio", GetGammaGammaTimeRatio() );
	pp_ratio = config->GetValue( "ParticleParticle_FillRatio", GetParticleParticleTimeRatio() );
	ee_ratio = config->GetValue( "ElectronElectron_FillRatio", GetElectronElectronTimeRatio() );
	ge_ratio = config->GetValue( "GammaElectron_FillRatio", GetGammaElectronTimeRatio() );
	pe_ratio = config->GetValue( "ParticleElectron_FillRatio", GetParticleElectronTimeRatio() );

	// Detector to target distances
	cd_dist.resize( set->GetNumberOfCDDetectors() );
	cd_offset.resize( set->GetNumberOfCDDetectors() );
	dead_layer.resize( set->GetNumberOfCDDetectors() );
	double d_tmp;
	for( unsigned int i = 0; i < set->GetNumberOfCDDetectors(); ++i ) {

		if( i == 0 ) d_tmp = 32.0; // standard CD
		else if( i == 1 ) d_tmp = -64.0; // TREX backwards CD
		cd_dist[i] = config->GetValue( Form( "CD_%d.Distance", i ), d_tmp );		// distance to target in mm
		cd_offset[i] = config->GetValue( Form( "CD_%d.PhiOffset", i ), 0.0 );		// phi rotation in degrees
		dead_layer[i] = config->GetValue( Form( "CD_%d.DeadLayer", i ), 0.0007 );	// dead layer thickness in mm of Si

	}

	// Target thickness and offsets
	target_thickness = config->GetValue( "TargetThickness", 2.0 ); // units of mg/cm^2
	x_offset = config->GetValue( "TargetOffset.X", 0.0 );	// of course this should be 0.0 if you centre the beam! Units of mm, vertical
	y_offset = config->GetValue( "TargetOffset.Y", 0.0 );	// of course this should be 0.0 if you centre the beam! Units of mm, horizontal
	z_offset = config->GetValue( "TargetOffset.Z", 0.0 );	// of course this should be 0.0 if you centre the beam! Units of mm, lateral

	// Degrader thickness and material
	degrader_thickness = config->GetValue( "DegraderThickness", -1.0 ); 	// units of mg/cm^2 - negative means it doesn't exist (only plunger runs)
	std::string degrader_material_tmp = config->GetValue( "DegraderMaterial", "197Au" );	// can be isotope name or other material name that matches SRIM file
	for( unsigned int i = 0; i < degrader_material_tmp.length(); i++ ){
		if( std::isspace( degrader_material_tmp[i] ) || degrader_material_tmp[i] == '#' )
			break;
		else degrader_material += degrader_material_tmp[i];
	}

	// Read in Miniball geometry
	mb_type = config->GetValue( "MiniballGeometry.Type", 1 ); // default = 1
	mb_geo.resize( set->GetNumberOfMiniballClusters() );
	mb_theta.resize( set->GetNumberOfMiniballClusters() );
	mb_phi.resize( set->GetNumberOfMiniballClusters() );
	mb_alpha.resize( set->GetNumberOfMiniballClusters() );
	mb_r.resize( set->GetNumberOfMiniballClusters() );
	for( unsigned int i = 0; i < set->GetNumberOfMiniballClusters(); ++i ) {
	
		mb_theta[i] = config->GetValue( Form( "MiniballCluster_%d.Theta", i ), 0. );
		mb_phi[i] 	= config->GetValue( Form( "MiniballCluster_%d.Phi", i ), 0. );
		mb_alpha[i] = config->GetValue( Form( "MiniballCluster_%d.Alpha", i ), 0. );
		mb_r[i]	 	= config->GetValue( Form( "MiniballCluster_%d.R", i ), 0. );

		mb_geo[i].SetGeometryType( mb_type );
		mb_geo[i].SetupCluster( mb_theta[i], mb_phi[i], mb_alpha[i], mb_r[i], z_offset );
	
	}
	
	// Read in SPEDE geometry
	spede_dist = config->GetValue( "Spede.Distance", -30.0 );	// distance to target in mm, it's negative because it's in the backwards direction
	spede_offset = config->GetValue( "Spede.PhiOffset", 0.0 );	// phi rotation in degrees
	if( spede_dist > 0 ) std::cout << " !! WARNING !! Spede.Distance should be negative" << std::endl;
	
	// Get the stopping powers
	stopping = true;
	for( unsigned int i = 0; i < 7; ++i )
		gStopping.push_back( std::make_unique<TGraph>() );
	stopping &= ReadStoppingPowers( Beam.GetIsotope(), Target.GetIsotope(), gStopping[0] );
	stopping &= ReadStoppingPowers( Ejectile.GetIsotope(), Target.GetIsotope(), gStopping[1] );
	stopping &= ReadStoppingPowers( Recoil.GetIsotope(), Target.GetIsotope(), gStopping[2] );
	stopping &= ReadStoppingPowers( Ejectile.GetIsotope(), "Si", gStopping[3] );
	stopping &= ReadStoppingPowers( Recoil.GetIsotope(), "Si", gStopping[4] );
	if( degrader_thickness > 0 ) {
		stopping &= ReadStoppingPowers( Ejectile.GetIsotope(), degrader_material, gStopping[5] );
		stopping &= ReadStoppingPowers( Recoil.GetIsotope(), degrader_material, gStopping[6] );
	}

	
	// Some diagnostics and info
	std::cout << std::endl << " +++  ";
	std::cout << Target.GetIsotope() << "(" << Beam.GetIsotope() << ",";
	std::cout << Ejectile.GetIsotope() << ")" << Recoil.GetIsotope();
	std::cout << "  +++" << std::endl;
	std::cout << "Q-value = " << GetQvalue()*0.001 << " MeV" << std::endl;
	std::cout << "Incoming beam energy = ";
	std::cout << Beam.GetEnergy()*0.001 << " MeV" << std::endl;
	std::cout << "Target thickness = ";
	std::cout << target_thickness << " mg/cm^2" << std::endl;

	// Calculate the energy loss
	if( stopping ){
		
		double eloss = GetEnergyLoss( Beam.GetEnergy(), 0.5 * target_thickness, gStopping[0] );
		Beam.SetEnergy( Beam.GetEnergy() - eloss );
		std::cout << "Beam energy at centre of target = ";
		std::cout << Beam.GetEnergy()*0.001 << " MeV" << std::endl;

	}
	else std::cout << "Stopping powers not calculated" << std::endl;

	std::cout << "Beam velocity at reaction position = ";
	std::cout << Beam.GetBeta() << "c" << std::endl;

	if( degrader_thickness > 0 ) {

		std::cout << "A " << degrader_material << " degrader of " << degrader_thickness;
		std::cout << " mg/cm2 has been included. Doppler correction will be performed";
		if( doppler_mode == 0 || doppler_mode == 1 || doppler_mode == 5 )
			std::cout << " BEFORE the degrader";
		else if( doppler_mode == 2 || doppler_mode == 3 || doppler_mode == 4 )
			std::cout << " AFTER the degrader";
		else
			std::cout << " with unknown DopplerMode = " << doppler_mode;
		std::cout << std::endl;

	}

	// Finished
	delete config;

}

void MiniballReaction::PrintReaction( std::ostream &stream, std::string opt = "" ) {

	// Check options (not yet implemented)
	if( opt.length() > 0 )
		stream << "# The following print options were used: " << opt << std::endl;
	
	// Loop over clusters
	for( unsigned int i = 0; i < set->GetNumberOfMiniballClusters(); ++i ) {

		stream << Form( "MiniballCluster_%d.Theta: %f", i, GetMiniballTheta(i) * TMath::RadToDeg() ) << std::endl;
		stream << Form( "MiniballCluster_%d.Phi: %f", i, GetMiniballPhi(i) * TMath::RadToDeg() ) << std::endl;
		stream << Form( "MiniballCluster_%d.Alpha: %f", i, GetMiniballAlpha(i) * TMath::RadToDeg() ) << std::endl;
		stream << Form( "MiniballCluster_%d.R: %f", i, GetMiniballR(i) ) << std::endl;

	}
	
}

////////////////////////////////////////////////////////////////////////////////
/// Loads a TCutG from file, either a saved TCutG from a ROOT file or the (x,y)
/// coordinates from a text file.
std::shared_ptr<TCutG> MiniballReaction::ReadCutFile( std::string cut_filename, std::string cut_name ) {

	std::shared_ptr<TCutG> cut;

	// Check if filename is given in the settings file.
	if( cut_filename != "NULL" ) {

		TFile *cut_file = new TFile( cut_filename.data(), "READ" );
		if( cut_file->IsZombie() )
			std::cout << "Couldn't open " << cut_filename << " correctly" << std::endl;

		else {

			if( !cut_file->GetListOfKeys()->Contains( cut_name.data() ) )
				std::cout << "Couldn't find " << cut_name << " in "
				<< cut_filename << std::endl;
			else
				cut = std::make_shared<TCutG>( *static_cast<TCutG*>( cut_file->Get( cut_name.data() )->Clone() ) );

		}

		cut_file->Close();

	}

	// Assign an empty cut file if none is given, so the code doesn't crash
	if( !cut ) cut = std::make_shared<TCutG>();

	return cut;

}


TVector3 MiniballReaction::GetCDVector( unsigned char det, unsigned char sec, float pid, float nid ){
	
	// Check that we have a real CD detector
	if( det >= set->GetNumberOfCDDetectors() ) {
	
		std::cerr << "Bad CD Detector requested = " << det << std::endl;
		det = 0;
		
	}
	
	// Create a TVector3 to handle the angles
	TVector3 vec( 0, 0, cd_dist[det] ); // set z now
	
	// Get the phi rotation of the quadrant
	float phi = 90.0 * sec;
	phi += cd_offset[det]; // left edge of first strip
	
	// Recalculate this points for the standard CD (not yet done for CREX/TREX)
	if( set->GetNumberOfCDNStrips() == 12 || set->GetNumberOfCDNStrips() == 24 ) { // standard CD

		// CD phi calculation using method from Tim Gray
		double alpha = 82.0;
		double offset_x = 1.6728;
		double offset_y = 1.5751;
		double grouping = 24.0 / (double)set->GetNumberOfCDNStrips();

		double phi_body = grouping * ( nid + 0.5 ) * alpha / 24.;

		double r_lab = 9.0;
		r_lab += ( 15.5 - pid ) * 2.0; // pid = 0 is outer ring and pid = 15 is inner ring
		  
		double beta = 180.0 - TMath::ATan(offset_y/offset_x) * TMath::RadToDeg();
		double bphi = beta + phi_body;
		if( bphi > 180.0) bphi = 360.0 - bphi;

		double r_d = TMath::Sqrt( offset_x * offset_x + offset_y * offset_y ); // from center of rings to center of sectors
		double delta = TMath::ASin( r_d * TMath::Sin( bphi * TMath::DegToRad() ) / r_lab );
		delta *= TMath::RadToDeg(); // angle between r_body and r_lab
		  
		double gamma = 180.0 - bphi - delta; // angle between r_d and r_lab

		double r_body;
		if (TMath::Abs(TMath::Sin( bphi * TMath::DegToRad() ) < 1e-5)) r_body = r_lab - r_d;
		else r_body = TMath::Sin( gamma * TMath::DegToRad() ) / ( TMath::Sin( bphi * TMath::DegToRad() ) / r_lab ); // between sector center and point of interest

		double x_body = r_body * TMath::Cos( phi_body * TMath::DegToRad() ); //in sector "body" coordinates
		double y_body = r_body * TMath::Sin( phi_body * TMath::DegToRad() );

		//transform back to ring "lab" coordinates
		double y = y_body + offset_y;
		double x = x_body + offset_x;

		//should have sqrt(x*x + y*y) = r_lab at this point
		vec.SetX(x);
		vec.SetY(y);

		/*
		 // New definition of CD segments - Konstantin Stoychev
		 //// calculate initial phi offset due to dead silicon layer
		 //float initial_offset = 0.000286 * TMath::Power(pid,4);
		 //initial_offset += -0.00541 * TMath::Power(pid,3);
		 //initial_offset += 0.04437 * TMath::Power(pid,2);
		 //initial_offset += 0.01679 * pid;
		 //initial_offset += 2.4137;
		 
		 //// calculate phi angular coverage for each annular strip and width of individual pixel
		 //double phi_coverage = -0.00625 * TMath::Power(pid,3);
		 //phi_coverage +=  0.07056 * TMath::Power(pid,2);
		 //phi_coverage += -0.54542 * pid;
		 //phi_coverage += 77.66278; // parametrization from Konstantin Stoychev
		 
		 //float pixel_width = phi_coverage / 12.;
		 
		 //phi += initial_offset;
		 //phi += pixel_width / 2.;
		 //phi += nid * pixel_width;
		 */
		
		/*
		 // Old definition of CD segments
		 //phi += 3.5; // move the centre of first strip to zero degrees
		 //phi += nid * 7.0;
		 */
	
	}
	
	else if( set->GetNumberOfCDNStrips() == 16 ) {		// CREX and TREX
		
		// Inner ring starts at 9.0 mm
		float x = 9.0;
		
		// Each strip centre is 2.0 mm apart
		x += ( pid + 0.5 ) * 2.0;
		vec.SetX(x);

		// Then find phi angle for each n-side strip
		phi += 1.75; // centre of first strip
		if( nid < 4 ) phi += nid * 3.5; // first 4 strips singles (=4 nid)
		else if( nid < 12 ) phi += 14. + ( nid - 4 ) * 7.0; // middle 16 strips doubles (=8 nids)
		else phi += 70. + ( nid - 12 ) * 3.5; // last 4 strips singles (=4 nid)
	
	}
	
	// Rotate to the correct phi angle
	vec.RotateZ( phi * TMath::DegToRad() );
	
	return vec;

}

TVector3 MiniballReaction::GetParticleVector( unsigned char det, unsigned char sec, unsigned char pid, unsigned char nid ){
	
	// Create a TVector3 to handle the angles
	TVector3 vec = GetCDVector( det, sec, pid, nid );
	
	// Apply the X and Y offsets directly to the TVector3 input
	// We move the CD opposite to the target, which replicates the same
	// geometrical shift that is observed with respect to the beam
	vec.SetX( vec.X() - x_offset );
	vec.SetY( vec.Y() - y_offset );

	//std::cout << "sec:pid:nid = " << (int)sec << ":" << (int)pid << ":" << (int)nid << std::endl;
	//std::cout << "\ttheta,phi = " << vec.Theta() * TMath::RadToDeg();
	//std::cout << ", " << vec.Phi() * TMath::RadToDeg() << std::endl;
	
	return vec;

}

TVector3 MiniballReaction::GetSpedeVector( unsigned char seg, bool random ){

	// Calculate the vertical distance from axis
	float x = 10.0;								// inner radius = 10.0 mm
	float mult = 0.5;							// go to the centre of each ring
	if( random ) mult = rand.Rndm();			// go to a random point along each ring
	if( seg < 8 ) x += 5.15 * mult;				// width of first ring = 5.2 mm including 0.05 mm inter-strip region
	else if( seg < 16 ) x += 5.2 + 3.85 * mult;	// width of second ring = 3.9 mm including 0.05 mm inter-strip region
	else if( seg < 24 ) x += 9.1 + 3.15 * mult;	// width of third ring = 3.2 mm

	// Create a TVector3 to handle the angles
	TVector3 vec( x, 0, spede_dist );

	// Rotate appropriately
	if( random ) mult = rand.Rndm();					// get a new random point for the phi angle
	float phi = (float)(seg%8) * TMath::Pi() / 4.0;		// rotate every 8 slices
	phi += 0.98 * mult * TMath::Pi() / 4.0; 			// rotate by half of one slice or random part thereof
	phi += 0.01 * TMath::Pi() / 4.0; 					// add a small slice for the interstrip region

	// Account for zero-degree offset
	phi += spede_offset * TMath::DegToRad();

	vec.RotateZ( phi );
	
	return vec;
	
}

TVector3 MiniballReaction::GetElectronVector( unsigned char seg ){

	// Create a TVector3 to handle the angles
	TVector3 vec = GetSpedeVector( seg );

	// Apply the X and Y offsets directly to the TVector3 input
	// We move the CD opposite to the target, which replicates the same
	// geometrical shift that is observed with respect to the beam
	vec.SetX( vec.X() - x_offset );
	vec.SetY( vec.Y() - y_offset );

	return vec;
	
}

double MiniballReaction::CosTheta( std::shared_ptr<GammaRayEvt> g, bool ejectile ) {

	/// Returns the CosTheta angle between particle and gamma ray.
	/// @param ejectile true for and to the ejectile or false for recoil
	MiniballParticle p;
	if( ejectile ) p = Ejectile;
	else p = Recoil;

	TVector3 gvec = mb_geo[g->GetCluster()].GetSegVector( g->GetCrystal(), g->GetSegment() );
	
	// Apply the X and Y offsets directly to the TVector3 input
	// We move Miniball opposite to the target, which replicates the same
	// geometrical shift that is observed with respect to the beam
	// z-offset is already applied to the vector when the geometry is setup
	gvec.SetX( gvec.X() - x_offset );
	gvec.SetY( gvec.Y() - y_offset );
	
	return TMath::Cos( gvec.Angle( p.GetVector() ) );
	
}

double MiniballReaction::CosTheta( std::shared_ptr<SpedeEvt> s, bool ejectile ) {

	/// Returns the CosTheta angle between particle and electron.
	/// @param ejectile true for and to the ejectile or false for recoil
	MiniballParticle p;
	if( ejectile ) p = Ejectile;
	else p = Recoil;

	TVector3 evec = GetElectronVector( s->GetSegment() );
	
	return TMath::Cos( evec.Angle( p.GetVector() ) );

}

double MiniballReaction::DopplerShift( double gen, double pbeta, double costheta ) {

	/// Returns Doppler shifted gamma-ray energy
	double gamma = 1.0 / TMath::Sqrt( 1.0 - TMath::Power( pbeta, 2.0 ) );
	double corr = 1.0 - pbeta * costheta;
	corr *= gamma;
	
	return gen / corr;
	
}

double MiniballReaction::DopplerCorrection( double gen, double gtheta, double gphi, double pbeta, double ptheta, double pphi ) {
	
	/// Returns Doppler corrected gamma-ray energy assuming particle at (β,θ,φ).
	TVector3 gvec = TVector3( 0., 0., 1.0 );
	gvec.SetTheta( gtheta );
	gvec.SetPhi( gphi);
	
	// Apply the X and Y offsets directly to the TVector3 input
	// We move Miniball opposite to the target, which replicates the same
	// geometrical shift that is observed with respect to the beam
	// z-offset is already applied to the vector when the geometry is setup
	gvec.SetX( gvec.X() - x_offset );
	gvec.SetY( gvec.Y() - y_offset );
	
	// Setup the particle vector, no shifts becuase theta,phi explicitly given
	TVector3 pvec( 0., 0., 1.0 );
	pvec.SetTheta( ptheta );
	pvec.SetPhi( pphi );
	
	double gamma = 1.0 / TMath::Sqrt( 1.0 - TMath::Power( pbeta, 2.0 ) );
	double corr = 1.0 - pbeta * TMath::Cos( gvec.Angle( pvec ) );
	corr *= gamma;
	
	return corr * gen;
	
}

double MiniballReaction::DopplerCorrection( double gen, double gtheta, double gphi, bool ejectile ) {
	
	/// Returns Doppler corrected gamma-ray energy assuming particle at (β,θ,φ).
	TVector3 gvec = TVector3( 0., 0., 1.0 );
	gvec.SetTheta( gtheta );
	gvec.SetPhi( gphi);
	
	// Apply the X and Y offsets directly to the TVector3 input
	// We move Miniball opposite to the target, which replicates the same
	// geometrical shift that is observed with respect to the beam
	// z-offset is already applied to the vector when the geometry is setup
	gvec.SetX( gvec.X() - x_offset );
	gvec.SetY( gvec.Y() - y_offset );
	
	/// Returns Doppler corrected gamma-ray energy for given particle and gamma combination.
	/// @param ejectile true for ejectile Doppler correction or false for recoil
	MiniballParticle p;
	if( ejectile ) p = Ejectile;
	else p = Recoil;
	
	double costheta = TMath::Cos( gvec.Angle( p.GetVector() ) );
	double corr = 1.0 - p.GetBeta() * costheta;
	corr *= p.GetGamma();

	return corr * gen;
	
}

double MiniballReaction::DopplerCorrection( std::shared_ptr<GammaRayEvt> g, double pbeta, double ptheta, double pphi ) {
	
	/// Returns Doppler corrected gamma-ray energy assuming particle at (β,θ,φ).
	TVector3 gvec = mb_geo[g->GetCluster()].GetSegVector( g->GetCrystal(), g->GetSegment() );
	
	// Apply the X and Y offsets directly to the TVector3 input
	// We move Miniball opposite to the target, which replicates the same
	// geometrical shift that is observed with respect to the beam
	// z-offset is already applied to the vector when the geometry is setup
	gvec.SetX( gvec.X() - x_offset );
	gvec.SetY( gvec.Y() - y_offset );
	
	// Setup the particle vector, no shifts becuase theta,phi explicitly given
	TVector3 pvec( 0., 0., 1.0 );
	pvec.SetTheta( ptheta );
	pvec.SetPhi( pphi );
	
	double gamma = 1.0 / TMath::Sqrt( 1.0 - TMath::Power( pbeta, 2.0 ) );
	double corr = 1.0 - pbeta * TMath::Cos( gvec.Angle( pvec ) );
	corr *= gamma;

	if( events_gamma_seg_energy )
		return corr * g->GetSegmentSumEnergy();
	else
		return corr * g->GetEnergy();

}

double MiniballReaction::DopplerCorrection( std::shared_ptr<GammaRayEvt> g, bool ejectile ) {

	/// Returns Doppler corrected gamma-ray energy for given particle and gamma combination.
	/// @param ejectile true for ejectile Doppler correction or false for recoil
	MiniballParticle p;
	if( ejectile ) p = Ejectile;
	else p = Recoil;
	
	double corr = 1.0 - p.GetBeta() * CosTheta( g, ejectile );
	corr *= p.GetGamma();
	
	if( events_gamma_seg_energy )
		return corr * g->GetSegmentSumEnergy();
	else
		return corr * g->GetEnergy();

}

double MiniballReaction::DopplerCorrection( std::shared_ptr<SpedeEvt> s, bool ejectile ) {

	/// Returns Doppler corrected electron energy for given particle and SPEDE combination.
	/// @param ejectile true for ejectile Doppler correction or false for recoil
	MiniballParticle p;
	if( ejectile ) p = Ejectile;
	else p = Recoil;
	
	// Joonas version
	double corr=((s->GetEnergy() + e_mass - p.GetBeta() * CosTheta( s, ejectile ) *
							 TMath::Sqrt(s->GetEnergy() * s->GetEnergy() + 2.0 * e_mass * s->GetEnergy())) /
							 TMath::Sqrt(1.0 - p.GetBeta() * p.GetBeta())) - e_mass;
	return corr;
	
	// Liam version
	//double corr = TMath::Power( s->GetEnergy(), 2.0 );
	//corr += 2.0 * e_mass * s->GetEnergy();
	//corr  = TMath::Sqrt( corr );
	//corr *= s->GetEnergy() + e_mass - p.GetBeta() * CosTheta( s, ejectile );
	//corr *= p.GetGamma();
	//corr -= e_mass;
	
	//return corr;
	
}

void MiniballReaction::IdentifyEjectile( std::shared_ptr<ParticleEvt> p, bool kinflag ){
	
	/// Set the ejectile particle and calculate the centre of mass angle too
	/// @param kinflag kinematics flag such that true is the backwards solution (i.e. CoM > 90 deg)
	double En = p->GetEnergy(), Eemit = En;
	double eloss = 0.0;
	if( stopping && ( doppler_mode == 3 || doppler_mode == 5 ) ) {

		double eff_thick = dead_layer[p->GetDetector()] / TMath::Abs( TMath::Cos( GetParticleTheta(p) ) );
		eloss = GetEnergyLoss( En, -1.0 * eff_thick, gStopping[3] ); // ejectile in dead layer
		En -= eloss;
		Eemit = En;

		// Correction for degrader, so we get energy after target
		if( ( doppler_mode == 3 || doppler_mode == 5 ) && degrader_thickness > 0 ) {

			eff_thick = degrader_thickness / TMath::Abs( TMath::Cos( GetParticleTheta(p) ) );
			eloss = GetEnergyLoss( En, -1.0 * eff_thick, gStopping[5] ); // ejectile in degrader
			En -= eloss;
			if( doppler_mode == 5 ) Eemit = En;
		}

	}
	
	// Correction for energy loss in second half of target, so we get energy in the centre of the target
	if( stopping && ( doppler_mode == 2 || doppler_mode == 3 || doppler_mode == 5 ) ) {
		double eff_thick = target_thickness / TMath::Abs( TMath::Cos( GetParticleTheta(p) ) );
		eloss = GetEnergyLoss( En, -0.5 * eff_thick, gStopping[1] ); // ejectile in target
		En -= eloss;
	}
	
	// Here, En is the energy at the centre of the target needed for the kinematic
	// calculation, while Eemit is the energy at the point where the gamma ray was
	// emitted (in the CD for mode 2, between degrader and CD for mode 5 and between
	// target and degrader for mode 3.
	
	// Set observables
	Ejectile.SetEnergy( En ); // eloss is negative
	Ejectile.SetTheta( GetParticleTheta(p) );
	Ejectile.SetPhi( GetParticlePhi(p) );

	// Calculate the centre of mass angle
	// TODO: Replace with the full relativisic kinematics
	double maxang = TMath::ASin( 1. / ( GetTau() * GetEpsilon() ) );
	double y = GetEpsilon() * GetTau();
	if( GetTau() * GetEpsilon() > 1 && GetParticleTheta(p) > maxang )
		y *= TMath::Sin( maxang );
	else
		y *= TMath::Sin( GetParticleTheta(p) );

	// Only one solution for the beam in normal kinematics, kinflag = false
	if( kinflag && GetTau() * GetEpsilon() < 1 ) kinflag = false;

	if( kinflag ) y = TMath::ASin( -y );
	else y = TMath::ASin( y );

	Ejectile.SetThetaCoM( GetParticleTheta(p) + y );
	
	ejectile_detected = true;
	
	// Now restore the energy to the value at the point of gamma-ray emission
	Ejectile.SetEnergy(Eemit);
	
	// Overwrite energy with kinematics if requested
	if( doppler_mode == 0 || doppler_mode == 1 || doppler_mode == 4 ) {

		// Energy of the ejectile from the centre of mass angle
		// TODO: Replace with the full relativisic kinematics
		double En = TMath::Power( GetTau() * GetEpsilon(), 2.0 ) + 1.0;
		En += 2.0 * GetTau() * GetEpsilon() * TMath::Cos( Ejectile.GetThetaCoM() );
		En *= TMath::Power( Recoil.GetMass() / ( Recoil.GetMass() + Ejectile.GetMass() ), 2.0 );
		En *= GetEnergyPrime();

		// Do energy loss out the back of target if requested
		if( stopping && doppler_mode == 1 ) {
			
			eloss = GetEnergyLoss( En, 0.5 * target_thickness / TMath::Abs( TMath::Cos( GetParticleTheta(p) ) ), gStopping[0] );
			En -= eloss;

		}

		// Do energy loss through the full degrader if requested
		if( stopping && doppler_mode == 4 && degrader_thickness > 0 ) {

			eloss = GetEnergyLoss( En, degrader_thickness / TMath::Abs( TMath::Cos( GetParticleTheta(p) ) ), gStopping[5] );
			En -= eloss;

		}

		// Finally set the energy
		Ejectile.SetEnergy( En );

	}
	
	//std::cout << "Theta = " << GetParticleTheta(p)*TMath::RadToDeg() << ": Energy = " << p->GetEnergy();
	//std::cout << ", Eloss = " << eloss << ", beta = " << Ejectile.GetBeta() << std::endl;

}

void MiniballReaction::IdentifyRecoil( std::shared_ptr<ParticleEvt> p, bool kinflag ){
	
	/// Set the recoil particle and calculate the centre of mass angle too
	/// @param kinflag kinematics flag such that true is the backwards solution (i.e. CoM > 90 deg)
	double En = p->GetEnergy();
	double eloss = 0.0;
	if( stopping && ( doppler_mode == 3 || doppler_mode == 5 ) ) {

		double eff_thick = dead_layer[p->GetDetector()] / TMath::Abs( TMath::Cos( GetParticleTheta(p) ) );
		eloss = GetEnergyLoss( En, -1.0 * eff_thick, gStopping[4] ); // recoil in dead layer
		En -= eloss;

		// Correction for degrader, so we get energy after target
		if( doppler_mode == 5 && degrader_thickness > 0 ) {

			eff_thick = degrader_thickness / TMath::Abs( TMath::Cos( GetParticleTheta(p) ) );
			eloss = GetEnergyLoss( En, -1.0 * eff_thick, gStopping[6] ); // recoil in degrader
			En -= eloss;

		}

	}

	// Set observables
	Recoil.SetEnergy( En ); // eloss is negative to add back the dead layer energy
	Recoil.SetTheta( GetParticleTheta(p) );
	Recoil.SetPhi( GetParticlePhi(p) );

	// Calculate the centre of mass angle
	double maxang = TMath::ASin( 1. / GetEpsilon() );
	double y = GetEpsilon();
	if( GetParticleTheta(p) > maxang )
		y *= TMath::Sin( maxang );
	else
		y *= TMath::Sin( GetParticleTheta(p) );
	
	if( kinflag ) y = TMath::ASin( -y );
	else y = TMath::ASin( y );

	Recoil.SetThetaCoM( GetParticleTheta(p) + y );
	recoil_detected = true;

	// Overwrite energy with kinematics if requested
	if( doppler_mode == 0 || doppler_mode == 1 || doppler_mode == 4 ) {

		// Energy of the recoil from the centre of mass angle
		En = TMath::Power( GetEpsilon(), 2.0 ) + 1.0;
		En += 2.0 * GetEpsilon() * TMath::Cos( Recoil.GetThetaCoM() );
		En *= Recoil.GetMass() * Ejectile.GetMass() / TMath::Power( Recoil.GetMass() + Ejectile.GetMass(), 2.0 );
		En *= GetEnergyPrime();

		// Do energy loss out the back of target if requested
		if( stopping && doppler_mode == 1 ) {

			eloss = GetEnergyLoss( En, 0.5 * target_thickness / TMath::Abs( TMath::Cos( GetParticleTheta(p) ) ), gStopping[2] );
			En -= eloss;

		}

		// Do energy loss through the full degrader if requested
		if( stopping && doppler_mode == 4 && degrader_thickness > 0 ) {

			eloss = GetEnergyLoss( En, degrader_thickness / TMath::Abs( TMath::Cos( GetParticleTheta(p) ) ), gStopping[6] );
			En -= eloss;

		}

		// Finally set the energy
		Recoil.SetEnergy( En );

	}

}

void MiniballReaction::CalculateEjectile(){

	/// Set the ejectile properties using the recoil data
	// Assume that the centre of mass angle is defined by the recoil
	Ejectile.SetThetaCoM( TMath::Pi() - Recoil.GetThetaCoM() );
	
	// Energy of the ejectile from the centre of mass angle
	double En = TMath::Power( GetTau() * GetEpsilon(), 2.0 ) + 1.0;
	En += 2.0 * GetTau() * GetEpsilon() * TMath::Cos( Ejectile.GetThetaCoM() );
	En *= TMath::Power( Recoil.GetMass() / ( Recoil.GetMass() + Ejectile.GetMass() ), 2.0 );
	En *= GetEnergyPrime();
	
	// Angle from the centre of mass angle
	// y = tan(theta_lab)
	double y = TMath::Sin( Ejectile.GetThetaCoM() );
	y /= TMath::Cos( Ejectile.GetThetaCoM() ) + GetTau() * GetEpsilon();
	
	double Th = TMath::ATan(y);
	if( Th < 0. ) Th += TMath::Pi();
	
	// Do energy loss out the back of target if requested
	double eloss = 0.0;
	if( stopping && doppler_mode > 0 ) {

		eloss = GetEnergyLoss( En, 0.5 * target_thickness / TMath::Abs( TMath::Cos(Th) ), gStopping[1] );
		En -= eloss;

		// Do energy loss through the full degrader if requested
		if( doppler_mode == 4 && degrader_thickness > 0 ) {

			eloss = GetEnergyLoss( En, degrader_thickness / TMath::Abs( TMath::Cos(Th) ), gStopping[5] );
			En -= eloss;

		}

	}

	// Set observables
	Ejectile.SetEnergy( En );
	Ejectile.SetTheta( Th );
	Ejectile.SetPhi( TMath::Pi() + Recoil.GetPhi() );
	ejectile_detected = false;

}

void MiniballReaction::CalculateRecoil(){

	/// Set the recoil properties using the ejectile data
	// Assume that the centre of mass angle is defined by the ejectile
	Recoil.SetThetaCoM( TMath::Pi() - Ejectile.GetThetaCoM() );

	// Energy of the recoil from the centre of mass angle
	double En = TMath::Power( GetEpsilon(), 2.0 ) + 1.0;
	En += 2.0 * GetEpsilon() * TMath::Cos( Recoil.GetThetaCoM() );
	En *= Recoil.GetMass() * Ejectile.GetMass() / TMath::Power( Recoil.GetMass() + Ejectile.GetMass(), 2.0 );
	En *= GetEnergyPrime();
	
	// Angle from the centre of mass angle
	// y = tan(theta_lab)
	double y = TMath::Sin( Recoil.GetThetaCoM() );
	y /= TMath::Cos( Recoil.GetThetaCoM() ) + GetEpsilon();
	
	double Th = TMath::ATan(y);
	if( Th < 0. ) Th += TMath::Pi();
	
	// Do energy loss out the back of target if requested
	double eloss = 0.0;
	if( stopping && doppler_mode > 0 ) {

		eloss = GetEnergyLoss( En, 0.5 * target_thickness / TMath::Abs( TMath::Cos(Th) ), gStopping[2] );
		En -= eloss;

		// Do energy loss through the full degrader if requested
		if( ( doppler_mode == 3 || doppler_mode == 4 ) && degrader_thickness > 0 ) {

			eloss = GetEnergyLoss( En, degrader_thickness / TMath::Abs( TMath::Cos(Th) ), gStopping[6] );
			En -= eloss;

		}

	}

	// Set observables
	Recoil.SetEnergy( En );
	Recoil.SetTheta( Th );
	Recoil.SetPhi( TMath::Pi() + Ejectile.GetPhi() );
	recoil_detected = false;

}

void MiniballReaction::TransferProduct( std::shared_ptr<ParticleEvt> p, bool kinflag ){

	/// Set the ejectile particle and calculate the centre of mass angle too
	/// @param kinflag kinematics flag such that true is the backwards solution (i.e. CoM > 90 deg)

	//this assumes the reaction product is emitted at the centre of the target
	double En = p->GetEnergy(); //get energy of the reaction product
	double eloss = 0.0;
	double after_target_recoil_energy = p->GetEnergy();
	double after_degrader_recoil_energy = p->GetEnergy();

	// Correcting energy loss in CD dead layer
	if( stopping && ( doppler_mode == 3 || doppler_mode == 5 ) ) {
		double eff_thick = dead_layer[p->GetDetector()] / TMath::Abs( TMath::Cos( GetParticleTheta(p) ) );
		eloss = GetEnergyLoss( En, -1.0 * eff_thick, gStopping[4] ); // recoil in dead layer
		En -= eloss;
		after_target_recoil_energy = En;
		after_degrader_recoil_energy = En;
	}

	// Correction for energy loss in the degrader
	if( stopping && degrader_thickness > 0 ) {
		double eff_thick = degrader_thickness / TMath::Abs( TMath::Cos( GetParticleTheta(p) ) );
		eloss = GetEnergyLoss( En, -1.0 * eff_thick, gStopping[6] ); // recoil in degrader
		En -= eloss;
		after_target_recoil_energy = En;
	}

	// Correction for energy loss through half of the target material
	if( stopping ) {
		double eff_thick = 0.5 * target_thickness / TMath::Abs( TMath::Cos( GetParticleTheta(p) ) );
		eloss = GetEnergyLoss( En, -1.0 * eff_thick, gStopping[2] ); // recoil in target
		En -= eloss;
	}

	// Set observables
	Recoil.SetEnergy( En );
	Recoil.SetTheta( GetParticleTheta(p) );
	Recoil.SetPhi( GetParticlePhi(p) );

	// Kinematics calculations assuming this energy
	double p4x = Recoil.GetMomentum() * TMath::Cos(Recoil.GetTheta());
	double p4y = Recoil.GetMomentum() * TMath::Sin(Recoil.GetTheta());
	double p3x = Beam.GetMomentum() - p4x;
	double p3y = p4y;
	double theta3 = TMath::ATan2(p3y, p3x);
	//double E3 = GetEnergyTotLab() - Recoil.GetEnergyTot();
	double E3 = Beam.GetEnergyTot() + Target.GetEnergyTot() - Recoil.GetEnergyTot(); // Total energy of ejectile

	// Kinetic energy at the reaction position in the centre of the target
	double beam_kinetic_energy = E3 - Ejectile.GetMass();

	// Calculate the centre-of-mass energy and angle
	// Vili's version
	double E3_CoM = GetGammaCoM()*(E3 - GetBetaCoM() * p3x);
	double p3x_CoM = GetGammaCoM()*(p3x - GetBetaCoM() * E3);
	double p3y_CoM = p3y;
	// double p_CoM = TMath::Sqrt(TMath::Power(p3x_CoM, 2.0) + TMath::Power(p3y_CoM, 2.0));
	double theta3_CoM = TMath::ATan2(p3y_CoM, p3x_CoM);

	// Lets check E4_CoM also with lorentz transfrom
	//double E4_CoM = Recoil.GetEnergyTotCM(); This is only calculated from projectile = target -> bad
	double E4_CoM = GetGammaCoM()*(Recoil.GetEnergyTot() - GetBetaCoM() * p4x);
	double p4x_CoM = GetGammaCoM()*(p4x - GetBetaCoM() * Recoil.GetEnergyTot());
	double p4y_CoM = p4y;
	double theta4_CoM = TMath::ATan2(p4y_CoM, p4x_CoM);

	Ejectile.SetEnergyCoM(E3_CoM - Ejectile.GetMass() ); // Energy of the ejectile in CoM frame
	Ejectile.SetThetaCoM(theta3_CoM); // theta of ejectile in CoM frame in radians
	Recoil.SetEnergyCoM(E4_CoM - Recoil.GetMass()); // Set Recoil energy in CoM
	Recoil.SetThetaCoM( TMath::Pi() - theta3_CoM ); // theta of recoil in CoM frame in radians

	// Calculate the ejectile stopping
	if( stopping && doppler_mode > 0 ) {

		double eff_thick = 0.5 * target_thickness / TMath::Abs( TMath::Cos(theta3) );
		eloss = GetEnergyLoss( beam_kinetic_energy, eff_thick, gStopping[1] );
		beam_kinetic_energy -= eloss;

		// Do energy loss through the full degrader if requested
		if( doppler_mode >= 2 && doppler_mode <= 4 && degrader_thickness > 0 ) {

			eff_thick = degrader_thickness / TMath::Abs( TMath::Cos(theta3) );
			eloss = GetEnergyLoss( beam_kinetic_energy, eff_thick, gStopping[5] );
			beam_kinetic_energy -= eloss;

		}

	}

	// Set the ejectile energy
	Ejectile.SetEnergy( beam_kinetic_energy );  // Kinetic energy of ejectile
	Ejectile.SetTheta( theta3 ); // Calculates ejectile theta angle from recoil information
	Ejectile.SetPhi( TMath::Pi() + Recoil.GetPhi() );

	// Calculate also the energy loss of the recoil as requested
	if( doppler_mode == 2 )
		Recoil.SetEnergy( p->GetEnergy() );
	else if( doppler_mode == 1 || doppler_mode == 3 )
		Recoil.SetEnergy( after_target_recoil_energy );
	else if( doppler_mode == 4 )
		Recoil.SetEnergy( after_degrader_recoil_energy );

	// Flag that we have a transfer product
	transfer_detected = true;

	return;

}



double MiniballReaction::GetEnergyLoss( double Ei, double dist, std::unique_ptr<TGraph> &g ) {

	/// Returns the energy loss at a given initial energy and distance travelled
	/// A negative distance will add the energy back on, i.e. travelling backwards
	/// This means that you will get a negative energy loss as a return value
	unsigned int Nmeshpoints = 50; // number of steps to take in integration
	double dx = dist/(double)Nmeshpoints;
	double E = Ei;
	
	for( unsigned int i = 0; i < Nmeshpoints; i++ ){

		double Eloss = g->Eval(E) * dx;
		if( Eloss > E ) {
			E = 0.01;
			break;
		}
		else
			E -= Eloss;
		
	}

	return Ei - E;

}

bool MiniballReaction::ReadStoppingPowers( std::string isotope1, std::string isotope2, std::unique_ptr<TGraph> &g ) {
	
	// Convert deuterium to CD2, and others
	if( isotope2 == "1H" ) isotope2 = "CH2";
	if( isotope2 == "2H" ) isotope2 = "CD2";
	if( isotope2 == "3H" ) isotope2 = "Ti";

	/// Open stopping power files and make TGraphs of data
	std::string title = "Stopping powers for ";
	title += isotope1 + " in " + isotope2;
	title += ";" + isotope1 + " energy [keV];";
	title += "Energy loss in " + isotope2;
	if( isotope2 == "Si" ) title += " [keV/mm]";
	else title += " [keV/(mg/cm^{2})]";
	
	// Initialise an empty TGraph
	g->SetTitle( title.c_str() );

	// Keep things quiet from ROOT
	gErrorIgnoreLevel = kWarning;

	// Open the data file
	// SRIM_DIR is defined at compilation and is in source code
	std::string srimfilename = std::string( SRIM_DIR ) + "/";
	srimfilename += isotope1 + "_" + isotope2 + ".txt";
	
	std::ifstream input_file;
	input_file.open( srimfilename, std::ios::in );

	// If it fails to open print an error
	if( !input_file.is_open() ) {
		
		std::cerr << "Cannot open " << srimfilename << std::endl;
		return false;
		  
	}


	std::string line, units, tmp_str;
	std::stringstream line_ss;
	double En, nucl, elec, total, tmp_dbl;
	 
	// Test file format
	std::getline( input_file, line );
	if( line.substr( 3, 5 ) == "=====" ) {
		
		// Advance
		while( std::getline( input_file, line ) && !input_file.eof() ) {
			
			// Skip over the really short lines
			if( line.length() < 10 ) continue;
			
			// Check for the start of the data
			if( line.substr( 3, 5 ) == "-----" ) break;
			
		}
		
	}
	else {
		
		std::cerr << "Not a srim file: " << srimfilename << std::endl;
		return false;
		
	}

	// Read in the data
	while( std::getline( input_file, line ) && !input_file.eof() ) {
		
		// Skip over the really short lines
		if( line.length() < 10 ) continue;

		// Read in data
		line_ss.str("");
		line_ss << line;
		line_ss >> En >> units >> nucl >> elec >> tmp_dbl >> tmp_str >> tmp_dbl >> tmp_str;
		
		if( units == "eV" ) En *= 1E-3;
		else if( units == "keV" ) En *= 1E0;
		else if( units == "MeV" ) En *= 1E3;
		else if( units == "GeV" ) En *= 1E6;
		
		total = nucl + elec ; // in some units, conversion done later
		
		// If we've reached the end, stop
		if( line.substr( 3, 9 ) == "---------" ) break;

		g->SetPoint( g->GetN(), En, total );
		
	}
	
	// Get next line and check there are conversion factors
	std::getline( input_file, line );
	if( line.substr( 0, 9 ) != " Multiply" ){
		
		std::cerr << "Couldn't get conversion factors from ";
		std::cerr << srimfilename << std::endl;
		return false;
		
	}
	std::getline( input_file, line ); // next line is just ------

	// Get conversion factors
	double conv, conv_keVum, conv_MeVmgcm2;
	std::getline( input_file, line ); // first conversion is eV / Angstrom
	std::getline( input_file, line ); // keV / micron
	conv_keVum = std::stod( line.substr( 0, 15 ) );
	std::getline( input_file, line ); // MeV / mm
	std::getline( input_file, line ); // keV / (ug/cm2)
	std::getline( input_file, line ); // MeV / (mg/cm2)
	conv_MeVmgcm2 = std::stod( line.substr( 0, 15 ) );
	
	// Now convert all the points in the plot
	if( isotope2 == "Si" ) conv = conv_keVum * 1E3; // silicon thickness in mm, energy in keV
	else conv = conv_MeVmgcm2 * 1E3; // target thickness in mg/cm2, energy in keV
	for( Int_t i = 0; i < g->GetN(); ++i ){
		
		g->GetPoint( i, En, total );
		g->SetPoint( i, En, total*conv );
		
	}
	
	// Draw the plot and save it somewhere
	TCanvas *c = new TCanvas();
	c->SetLogx();
	//c->SetLogy();
	g->Draw("A*");
	std::string pdfname = srimfilename.substr( 0, srimfilename.find_last_of(".") ) + ".pdf";
	c->SaveAs( pdfname.c_str() );
	
	delete c;
	input_file.close();
	
	// ROOT can be noisey again
	gErrorIgnoreLevel = kInfo;

	return true;
	 
}
