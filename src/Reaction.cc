#include "Reaction.hh"

ClassImp( Particle )
ClassImp( Reaction )

// Particles
Particle::Particle(){}
Particle::~Particle(){}



// Reaction things
Reaction::Reaction( std::string filename, Settings *myset ){
		
	// Read in mass tables
	ReadMassTables();

	// Get the info from the user input
	set = myset;
	SetFile( filename );
	ReadReaction();
		
}

void Reaction::AddBindingEnergy( short Ai, short Zi, TString ame_be_str ) {
	
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

void Reaction::ReadMassTables() {

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

void Reaction::ReadReaction() {

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

	Recoil.SetA( config->GetValue( "RecoilA", 120 ) );
	Recoil.SetZ( config->GetValue( "RecoilZ", 50 ) );
	if( Recoil.GetZ() < 0 || Recoil.GetZ() >= (int)gElName.size() ){
		
		std::cout << "Not a recognised element with Z = ";
		std::cout << Recoil.GetZ() << " (recoil)" << std::endl;
		exit(1);
		
	}
	Recoil.SetBindingEnergy( ame_be.at( Recoil.GetIsotope() ) );
	
	// Get particle energy cut
	ejectilecutfile = config->GetValue( "EjectileCut.File", "NULL" );
	ejectilecutname = config->GetValue( "EjectileCut.Name", "NULL" );
	recoilcutfile = config->GetValue( "RecoilCut.File", "NULL" );
	recoilcutname = config->GetValue( "RecoilCut.Name", "NULL" );

	// Check if beam cut is given by the user
	if( ejectilecutfile != "NULL" ) {
	
		cut_file = new TFile( ejectilecutfile.data(), "READ" );
		if( cut_file->IsZombie() )
			std::cout << "Couldn't open " << ejectilecutfile << " correctly" << std::endl;
			
		else {
		
			if( !cut_file->GetListOfKeys()->Contains( ejectilecutname.data() ) )
				std::cout << "Couldn't find " << ejectilecutname << " in " << ejectilecutfile << std::endl;
			else
				ejectile_cut = (TCutG*)cut_file->Get( ejectilecutname.data() )->Clone();

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
			else
				recoil_cut = (TCutG*)cut_file->Get( recoilcutname.data() )->Clone();

		}
		
		cut_file->Close();
		
	}

	// Assign an empty cut file if none is given, so the code doesn't crash
	if( !ejectile_cut ) ejectile_cut = new TCutG();
	if( !recoil_cut ) recoil_cut = new TCutG();

	
	// EBIS time window
	EBIS_On = config->GetValue( "EBIS.On", 1.2e6 );		// normally 1.2 ms in slow extraction
	EBIS_Off = config->GetValue( "EBIS.Off", 2.52e7 );	// this allows a off window 20 times bigger than on

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

	// Particle-Gamma fill ratios
	pg_ratio = config->GetValue( "ParticleGamma_RandomTime.Max", GetParticleGammaTimeRatio() );
	gg_ratio = config->GetValue( "GammaGamma_RandomTime.Max", GetGammaGammaTimeRatio() );
	pp_ratio = config->GetValue( "ParticleParticle_RandomTime.Max", GetParticleParticleTimeRatio() );

	// Detector to target distances
	cd_dist.resize( set->GetNumberOfCDDetectors() );
	cd_offset.resize( set->GetNumberOfCDDetectors() );
	float d_tmp;
	for( unsigned int i = 0; i < set->GetNumberOfCDDetectors(); ++i ) {
	
		if( i == 0 ) d_tmp = 32.0; // standard CD
		else if( i == 1 ) d_tmp = -64.0; // TREX backwards CD
		cd_dist[i] = config->GetValue( Form( "CD_%d.Distance", i ), d_tmp );	// distance to target in mm
		cd_offset[i] = config->GetValue( Form( "CD_%d.PhiOffset", i ), 0.0 );	// phi rotation in degrees

	}
	
	// Target offset
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
	
	
	// Some diagnostics and info
	std::cout << std::endl << " +++  ";
	std::cout << Beam.GetIsotope() << "(" << Target.GetIsotope() << ",";
	std::cout << Ejectile.GetIsotope() << ")" << Recoil.GetIsotope();
	std::cout << "  +++" << std::endl << "Beam energy = ";
	std::cout << Beam.GetEnergy()*0.001 << " MeV" << std::endl;
	std::cout << "Q-value = " << GetQvalue()*0.001 << " MeV" << std::endl;

	// Finished
	delete config;

}

TVector3 Reaction::GetCDVector( unsigned char det, unsigned char sec, unsigned char pid, unsigned char nid ){
	
	// Create a TVector3 to handle the angles
	float x = 9.0;
	if( set->GetNumberOfCDNStrips() == 12 ) 		// standard CD
		x += ( 15.5 - pid ) * 2.0;
	else if( set->GetNumberOfCDNStrips() == 16 )	// CREX and TREX
		x += ( pid + 0.5 ) * 2.0;

	TVector3 vec( x, 0, cd_dist[det] );
	
	// Rotate by the phi angle
	float phi = 90.0 * sec;
	phi += cd_offset[det];
	if( set->GetNumberOfCDNStrips() == 12 )				// standard CD
		phi += nid * 7.0;
	else if( set->GetNumberOfCDNStrips() == 16 ) {		// CREX and TREX
		
		phi += 1.75; // centre of first strip
		if( nid < 4 ) phi += nid * 3.5; // first 4 strips singles (=4 nid)
		else if( nid < 12 ) phi += 14. + ( nid - 4 ) * 7.0; // middle 16 strips doubles (=8 nids)
		else phi += 70. + ( nid - 12 ) * 3.5; // last 4 strips singles (=4 nid)
	
	}
	vec.RotateZ( phi * TMath::DegToRad() );
	
	return vec;

}

TVector3 Reaction::GetParticleVector( unsigned char det, unsigned char sec, unsigned char pid, unsigned char nid ){
	
	// Create a TVector3 to handle the angles
	TVector3 vec = GetCDVector( det, sec, pid, nid );
	
	// Apply the X and Y offsets directly to the TVector3 input
	// We move the CD opposite to the target, which replicates the same
	// geometrical shift that is observed with respect to the beam
	vec.SetX( vec.X() - x_offset );
	vec.SetY( vec.Y() - y_offset );

	return vec;

}

TVector3 Reaction::GetSpedeVector( unsigned char seg ){

	// Calculate the vertical distance from axis
	float x = 0.0;
	if( seg < 8 ) x = 25.2;			// distance to centre of first ring
	else if( seg < 16 ) x = 34.3;	// distance to centre of second ring
	else if( seg < 24 ) x = 41.4;	// distance to centre of third ring

	// Create a TVector3 to handle the angles
	TVector3 vec( x, 0, spede_dist );

	// Rotate appropriately
	float phi = TMath::Pi() / 8.0;			// rotate by half of one slice
	phi += (seg%8) * TMath::Pi() / 4.0;		// rotate every 8 slices
	
	return vec;
	
}

TVector3 Reaction::GetElectronVector( unsigned char seg ){

	// Create a TVector3 to handle the angles
	TVector3 vec = GetSpedeVector( seg );

	// Apply the X and Y offsets directly to the TVector3 input
	// We move the CD opposite to the target, which replicates the same
	// geometrical shift that is observed with respect to the beam
	vec.SetX( vec.X() - x_offset );
	vec.SetY( vec.Y() - y_offset );

}

double Reaction::CosTheta( GammaRayEvt *g, bool ejectile ) {

	/// Returns the CosTheta angle between particle and gamma ray.
	/// @param ejectile true for and to the ejectile or false for recoil
	Particle *p;
	if( ejectile ) p = &Ejectile;
	else p = &Recoil;

	TVector3 gvec = mb_geo[g->GetCluster()].GetSegVector( g->GetCrystal(), g->GetSegment() );

	return gvec.Cross( p->GetVector() ).CosTheta();
	
}

double Reaction::DopplerCorrection( GammaRayEvt *g, bool ejectile ) {

	/// Returns Doppler corrected gamma-ray energy for given particle and gamma combination.
	/// @param ejectile true for ejectile Doppler correction or false for recoil
	Particle *p;
	if( ejectile ) p = &Ejectile;
	else p = &Recoil;
	
	double corr = 1. - p->GetBeta() * CosTheta( g, ejectile );
	corr *= p->GetGamma();
	
	return corr * g->GetEnergy();
	
}

void Reaction::IdentifyEjectile( ParticleEvt *p, bool kinflag ){
	
	/// Set the ejectile particle and calculate the centre of mass angle too
	/// @param kinflag kinematics flag such that true is the backwards solution (i.e. CoM > 90 deg)
	Ejectile.SetEnergy( p->GetEnergy() );
	Ejectile.SetTheta( GetParticleTheta(p) );
	Ejectile.SetPhi( GetParticlePhi(p) );

	// Calculate the centre of mass angle
	float maxang = TMath::ASin( 1. / ( GetTau() * GetEpsilon() ) );
	float y = GetEpsilon() * GetTau();
	if( GetTau() * GetEpsilon() > 1 && GetParticleTheta(p) > maxang )
		y *= TMath::Sin( maxang );
	else
		y *= TMath::Sin( GetParticleTheta(p) );

	// Only one solution for the beam in normal kinematics, kinflag = false
	if( kinflag && GetTau() * GetEpsilon() < 1 ) kinflag = false;

	if( kinflag ) y = TMath::ASin( -y );
	else y = TMath::ASin( y );

	Ejectile.SetThetaCoM( GetParticleTheta(p) + y );

}

void Reaction::IdentifyRecoil( ParticleEvt *p, bool kinflag ){
	
	/// Set the recoil particle and calculate the centre of mass angle too
	/// @param kinflag kinematics flag such that true is the backwards solution (i.e. CoM > 90 deg)
	Recoil.SetEnergy( p->GetEnergy() );
	Recoil.SetTheta( GetParticleTheta(p) );
	Recoil.SetPhi( GetParticlePhi(p) );

	// Calculate the centre of mass angle
	float maxang = TMath::ASin( 1. / GetEpsilon() );
	float y = GetEpsilon();
	if( GetParticleTheta(p) > maxang )
		y *= TMath::Sin( maxang );
	else
		y *= TMath::Sin( GetParticleTheta(p) );
	
	if( kinflag ) y = TMath::ASin( -y );
	else y = TMath::ASin( y );

	Recoil.SetThetaCoM( GetParticleTheta(p) + y );

}

void Reaction::CalculateEjectile(){

	/// Set the ejectile properties using the recoil data
	// Assume that the centre of mass angle is defined by the recoil
	Ejectile.SetThetaCoM( TMath::Pi() - Recoil.GetThetaCoM() );
	
	// Energy of the ejectile from the centre of mass angle
	float En = TMath::Power( GetTau() * GetEpsilon(), 2.0 ) + 1.0;
	En += 2.0 * GetTau() * GetEpsilon() * TMath::Cos( Ejectile.GetThetaCoM() );
	En *= TMath::Power( Target.GetMass() / ( Target.GetMass() + Beam.GetMass() ), 2.0 );
	En *= GetEnergyPrime();
	
	Ejectile.SetEnergy( En );
	
	// Angle from the centre of mass angle
	// y = tan(theta_lab)
	float y = TMath::Sin( Ejectile.GetThetaCoM() );
	y /= TMath::Cos( Ejectile.GetThetaCoM() ) + GetTau() * GetEpsilon();
	
	float Th = TMath::ATan(y);
	if( Th < 0. ) Th += TMath::Pi();
	
	Ejectile.SetTheta( Th );

}

void Reaction::CalculateRecoil(){

	/// Set the recoil properties using the ejectile data
	// Assume that the centre of mass angle is defined by the ejectile
	Recoil.SetThetaCoM( TMath::Pi() - Ejectile.GetThetaCoM() );

	// Energy of the recoil from the centre of mass angle
	float En = TMath::Power( GetEpsilon(), 2.0 ) + 1.0;
	En += 2.0 * GetEpsilon() * TMath::Cos( Recoil.GetThetaCoM() );
	En *= Target.GetMass() * Beam.GetMass() / TMath::Power( Target.GetMass() + Beam.GetMass(), 2.0 );
	En *= GetEnergyPrime();
	
	Recoil.SetEnergy( En );

	// Angle from the centre of mass angle
	// y = tan(theta_lab)
	float y = TMath::Sin( Recoil.GetThetaCoM() );
	y /= TMath::Cos( Recoil.GetThetaCoM() ) + GetEpsilon();
	
	float Th = TMath::ATan(y);
	if( Th < 0. ) Th += TMath::Pi();

	Recoil.SetTheta( Th );

}
