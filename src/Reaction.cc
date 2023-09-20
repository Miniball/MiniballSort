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

	Eb = config->GetValue( "BeamE", 4500.0 ); // in keV per nucleon
	Eb *= Beam.GetA(); // keV
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
	bool ejectile_cut_flag = false;
	bool recoil_cut_flag = false;
	bool transfer_cut_flag = false;
	ejectilecutfile = config->GetValue( "EjectileCut.File", "NULL" );
	ejectilecutname = config->GetValue( "EjectileCut.Name", "CUTG" );
	recoilcutfile = config->GetValue( "RecoilCut.File", "NULL" );
	recoilcutname = config->GetValue( "RecoilCut.Name", "CUTG" );
	transfercutfile = config->GetValue( "TransferCut.File", "NULL" );
	transfercutname = config->GetValue( "TransferCut.Name", "CUTG" );

	// Check if beam cut is given by the user
	if( ejectilecutfile != "NULL" ) {
	
		cut_file = new TFile( ejectilecutfile.data(), "READ" );
		if( cut_file->IsZombie() )
			std::cout << "Couldn't open " << ejectilecutfile << " correctly" << std::endl;
			
		else {
		
			if( !cut_file->GetListOfKeys()->Contains( ejectilecutname.data() ) )
				std::cout << "Couldn't find " << ejectilecutname << " in " << ejectilecutfile << std::endl;
			else {
				ejectile_cut = (TCutG*)cut_file->Get( ejectilecutname.data() )->Clone();
				ejectile_cut_flag = true;
			}
		}
		
		cut_file->Close();
		
	}

	// Check if target cut is given by the user
	if( recoilcutfile != "NULL" ) {
	
		cut_file = new TFile( recoilcutfile.data(), "READ" );
		if( cut_file->IsZombie() )
			std::cout << "Couldn't open " << recoilcutfile << " correctly" << std::endl;
			
		else {
		
			if( !cut_file->GetListOfKeys()->Contains( recoilcutname.data() ) )
				std::cout << "Couldn't find " << recoilcutname << " in " << recoilcutfile << std::endl;
			else {
				recoil_cut = (TCutG*)cut_file->Get( recoilcutname.data() )->Clone();
				recoil_cut_flag = true;
			}
		}
		
		cut_file->Close();
		
	}

	// Check if transfer cut is given by the user
	if( transfercutfile != "NULL" ) {
	
		cut_file = new TFile( transfercutfile.data(), "READ" );
		if( cut_file->IsZombie() )
			std::cout << "Couldn't open " << transfercutfile << " correctly" << std::endl;
			
		else {
		
			if( !cut_file->GetListOfKeys()->Contains( transfercutname.data() ) )
				std::cout << "Couldn't find " << transfercutname << " in " << transfercutfile << std::endl;
			else {
				transfer_cut = (TCutG*)cut_file->Get( transfercutname.data() )->Clone();
				transfer_cut_flag = true;
			}
		}
		
		cut_file->Close();
		
	}

	// Assign an empty cut file if none is given, so the code doesn't crash
	if( !ejectile_cut_flag ) ejectile_cut = new TCutG();
	if( !recoil_cut_flag ) recoil_cut = new TCutG();
	if( !transfer_cut_flag ) transfer_cut = new TCutG();

	
	// EBIS time window
	EBIS_On = config->GetValue( "EBIS.On", 1.2e6 );		// normally 1.2 ms in slow extraction
	EBIS_Off = config->GetValue( "EBIS.Off", 2.52e7 );	// this allows a off window 20 times bigger than on
	EBIS_ratio = config->GetValue( "EBIS.FillRatio", GetEBISTimeRatio() );	// this is the measured ratio of EBIS On/off. Default is just the time window ratio
	
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

	// Read in Miniball geometry
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

		mb_geo[i].SetupCluster( mb_theta[i], mb_phi[i], mb_alpha[i], mb_r[i], z_offset );
	
	}
	
	// Read in SPEDE geometry
	spede_dist = config->GetValue( "Spede.Distance", -30.0 );	// distance to target in mm, it's negative because it's in the backwards direction
	spede_offset = config->GetValue( "Spede.PhiOffset", 0.0 );	// phi rotation in degrees
	if( spede_dist > 0 ) std::cout << " !! WARNING !! Spede.Distance should be negative" << std::endl;
	
	// Get the stopping powers
	stopping = true;
	for( unsigned int i = 0; i < 4; ++i )
		gStopping.push_back( std::make_unique<TGraph>() );
	stopping &= ReadStoppingPowers( Beam.GetIsotope(), Target.GetIsotope(), gStopping[0] );
	stopping &= ReadStoppingPowers( Target.GetIsotope(), Target.GetIsotope(), gStopping[1] );
	stopping &= ReadStoppingPowers( Beam.GetIsotope(), "Si", gStopping[2] );
	stopping &= ReadStoppingPowers( Target.GetIsotope(), "Si", gStopping[3] );


	
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

	// Finished
	delete config;

}

TVector3 MiniballReaction::GetCDVector( unsigned char det, unsigned char sec, float pid, float nid ){
	
	// Check that we have a real CD detector
	if( det >= set->GetNumberOfCDDetectors() ) {
	
		std::cerr << "Bad CD Detector requested = " << det << std::endl;
		det = 0;
		
	}
	
	// Create a TVector3 to handle the angles
	float x = 9.0;
	if( set->GetNumberOfCDNStrips() == 12 ) 		// standard CD
		x += ( 15.5 - pid + std::floor(pid) - std::ceil(pid) ) * 2.0;
	else if( set->GetNumberOfCDNStrips() == 16 )	// CREX and TREX
		x += ( pid + 0.5 ) * 2.0;

	TVector3 vec( x, 0, cd_dist[det] );
	
	// Rotate by the phi angle
	float phi = 90.0 * sec;
	phi += cd_offset[det]; // left edge of first strip
	if( set->GetNumberOfCDNStrips() == 12 )	{			// standard CD

		// New definition of CD segments
		// calculate phi angular coverage for each annular strip and width of individual pixel
		float coverage=-0.0044*TMath::Power(pid,3)+0.0451*TMath::Power(pid,2)-0.3646*pid+78.2188; // note parametrization is not 100% accurate
		float pixel_width=coverage/12.;
		phi += pixel_width/2.; // move the centre of first strip to zero degrees
		phi += nid * pixel_width;
		
		// Old definition of CD segments
		// phi += 3.5; // move the centre of first strip to zero degrees
		// phi += nid * 7.0;
	
	}
	
	else if( set->GetNumberOfCDNStrips() == 16 ) {		// CREX and TREX
		
		phi += 1.75; // centre of first strip
		if( nid < 4 ) phi += nid * 3.5; // first 4 strips singles (=4 nid)
		else if( nid < 12 ) phi += 14. + ( nid - 4 ) * 7.0; // middle 16 strips doubles (=8 nids)
		else phi += 70. + ( nid - 12 ) * 3.5; // last 4 strips singles (=4 nid)
	
	}
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

double MiniballReaction::DopplerCorrection( std::shared_ptr<GammaRayEvt> g, double pbeta, double ptheta, double pphi ) {

	/// Returns Doppler corrected gamma-ray energy assuming particle at (β,θ,φ).
	TVector3 gvec = mb_geo[g->GetCluster()].GetSegVector( g->GetCrystal(), g->GetSegment() );
	TVector3 pvec( 0., 0., 1.0 );
	pvec.SetTheta( ptheta );
	pvec.SetPhi( pphi );
	
	double gamma = 1.0 / TMath::Sqrt( 1.0 - TMath::Power( pbeta, 2.0 ) );
	double corr = 1. - pbeta * TMath::Cos( gvec.Angle( pvec ) );
	corr *= gamma;
	
	return corr * g->GetEnergy();
	
}

double MiniballReaction::DopplerCorrection( std::shared_ptr<GammaRayEvt> g, bool ejectile ) {

	/// Returns Doppler corrected gamma-ray energy for given particle and gamma combination.
	/// @param ejectile true for ejectile Doppler correction or false for recoil
	MiniballParticle p;
	if( ejectile ) p = Ejectile;
	else p = Recoil;
	
	double corr = 1. - p.GetBeta() * CosTheta( g, ejectile );
	corr *= p.GetGamma();
	
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
	
	return corr;
	
}

void MiniballReaction::IdentifyEjectile( std::shared_ptr<ParticleEvt> p, bool kinflag ){
	
	/// Set the ejectile particle and calculate the centre of mass angle too
	/// @param kinflag kinematics flag such that true is the backwards solution (i.e. CoM > 90 deg)
	double eloss = 0;
	if( stopping ) {
		eloss  = GetEnergyLoss( p->GetEnergy(), -1.0 * dead_layer[p->GetDetector()], gStopping[2] ); // ejectile in dead layer
		eloss += GetEnergyLoss( p->GetEnergy() - eloss, -0.5 * target_thickness, gStopping[0] ); // ejectile in target
	}
	Ejectile.SetEnergy( p->GetEnergy() - eloss ); // eloss is negative
	Ejectile.SetTheta( GetParticleTheta(p) );
	Ejectile.SetPhi( GetParticlePhi(p) );
	//std::cout << "Theta = " << GetParticleTheta(p)*TMath::RadToDeg() << ": Energy = " << p->GetEnergy();
	//std::cout << ", Eloss = " << eloss << ", beta = " << Ejectile.GetBeta() << std::endl;

	// Calculate the centre of mass angle
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

}

void MiniballReaction::IdentifyRecoil( std::shared_ptr<ParticleEvt> p, bool kinflag ){
	
	/// Set the recoil particle and calculate the centre of mass angle too
	/// @param kinflag kinematics flag such that true is the backwards solution (i.e. CoM > 90 deg)
	double eloss = 0;
	if( stopping ) {
		eloss  = GetEnergyLoss( p->GetEnergy(), -1.0 * dead_layer[p->GetDetector()], gStopping[3] ); // recoil in dead layer
		eloss += GetEnergyLoss( p->GetEnergy() - eloss, -0.5 * target_thickness, gStopping[1] ); // recoil in target
	}
	Recoil.SetEnergy( p->GetEnergy() - eloss ); // eloss is negative to add back the dead layer energy
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

}

void MiniballReaction::CalculateEjectile(){

	/// Set the ejectile properties using the recoil data
	// Assume that the centre of mass angle is defined by the recoil
	Ejectile.SetThetaCoM( TMath::Pi() - Recoil.GetThetaCoM() );
	
	// Energy of the ejectile from the centre of mass angle
	double En = TMath::Power( GetTau() * GetEpsilon(), 2.0 ) + 1.0;
	En += 2.0 * GetTau() * GetEpsilon() * TMath::Cos( Ejectile.GetThetaCoM() );
	En *= TMath::Power( Target.GetMass() / ( Target.GetMass() + Beam.GetMass() ), 2.0 );
	En *= GetEnergyPrime();
	
	Ejectile.SetEnergy( En );
	
	// Angle from the centre of mass angle
	// y = tan(theta_lab)
	double y = TMath::Sin( Ejectile.GetThetaCoM() );
	y /= TMath::Cos( Ejectile.GetThetaCoM() ) + GetTau() * GetEpsilon();
	
	double Th = TMath::ATan(y);
	if( Th < 0. ) Th += TMath::Pi();
	
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
	En *= Target.GetMass() * Beam.GetMass() / TMath::Power( Target.GetMass() + Beam.GetMass(), 2.0 );
	En *= GetEnergyPrime();
	
	Recoil.SetEnergy( En );

	// Angle from the centre of mass angle
	// y = tan(theta_lab)
	double y = TMath::Sin( Recoil.GetThetaCoM() );
	y /= TMath::Cos( Recoil.GetThetaCoM() ) + GetEpsilon();
	
	double Th = TMath::ATan(y);
	if( Th < 0. ) Th += TMath::Pi();

	Recoil.SetTheta( Th );
	Recoil.SetPhi( TMath::Pi() + Ejectile.GetPhi() );
	recoil_detected = false;

}

void MiniballReaction::TransferProduct( std::shared_ptr<ParticleEvt> p, bool kinflag ){
	
	/// Set the ejectile particle and calculate the centre of mass angle too
	/// @param kinflag kinematics flag such that true is the backwards solution (i.e. CoM > 90 deg)
	double eloss = 0;
	if( stopping ) {
		eloss  = GetEnergyLoss( p->GetEnergy(), -1.0 * dead_layer[p->GetDetector()], gStopping[2] ); // transfer product in dead layers
		eloss += GetEnergyLoss( p->GetEnergy() - eloss, -0.5 * target_thickness, gStopping[0] ); // transfer product in target
	}
	Recoil.SetEnergy( p->GetEnergy() - eloss ); // eloss is negative
	Recoil.SetTheta( GetParticleTheta(p) );
	Recoil.SetPhi( GetParticlePhi(p) );

	// Do something for the ejectile too, this needs some work
	if( stopping ) {
		eloss = GetEnergyLoss( Beam.GetEnergy(), 0.5 * target_thickness, gStopping[0] ); // transfer product in target
	}
	Ejectile.SetEnergy( Beam.GetEnergy() - eloss ); // eloss is positive
	Ejectile.SetTheta( 0.0 ); // asume is goes straight for now
	Ejectile.SetPhi( TMath::Pi() + Recoil.GetPhi() );

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
	Ejectile.SetThetaCoM( TMath::Pi() - Recoil.GetThetaCoM() );
	transfer_detected = true;

}



double MiniballReaction::GetEnergyLoss( double Ei, double dist, std::unique_ptr<TGraph> &g ) {

	/// Returns the energy loss at a given initial energy and distance travelled
	/// A negative distance will add the energy back on, i.e. travelling backwards
	/// This means that you will get a negative energy loss as a return value
	unsigned int Nmeshpoints = 50; // number of steps to take in integration
	double dx = dist/(double)Nmeshpoints;
	double E = Ei;
	
	for( unsigned int i = 0; i < Nmeshpoints; i++ ){

		if( E < 100. ) break; // when we fall below 100 keV we assume maximum energy loss
		E -= g->Eval(E) * dx;
		
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
		
		g->SetPoint( g->GetN(), En, total );
		
		// If we've reached the end, stop
		if( line.substr( 3, 9 ) == "---------" ) break;
		
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
