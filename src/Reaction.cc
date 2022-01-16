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

	// Detector to target distances
	cd_dist = config->GetValue( "CDDistance", 32.0 );
	
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
	Beam.SetEnergyLab( Eb ); // keV
	
	Target.SetA( config->GetValue( "TargetA", 120 ) );
	Target.SetZ( config->GetValue( "TargetZ", 50 ) );
	Target.SetEnergyLab( 0.0 );
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
	beamcutfile = config->GetValue( "BeamCut.File", "NULL" );
	beamcutname = config->GetValue( "BeamCut.Name", "NULL" );
	targetcutfile = config->GetValue( "TargetCut.File", "NULL" );
	targetcutname = config->GetValue( "TargetCut.Name", "NULL" );

	// Check if beam cut is given by the user
	if( beamcutfile != "NULL" ) {
	
		cut_file = new TFile( beamcutfile.data(), "READ" );
		if( cut_file->IsZombie() )
			std::cout << "Couldn't open " << beamcutfile << " correctly" << std::endl;
			
		else {
		
			if( !cut_file->GetListOfKeys()->Contains( beamcutname.data() ) )
				std::cout << "Couldn't find " << beamcutname << " in " << beamcutfile << std::endl;
			else
				beam_cut = (TCutG*)cut_file->Get( beamcutname.data() )->Clone();

		}
		
		cut_file->Close();
		
	}

	// Check if target cut is given by the user
	if( targetcutfile != "NULL" ) {
	
		cut_file = new TFile( targetcutfile.data(), "READ" );
		if( cut_file->IsZombie() )
			std::cout << "Couldn't open " << targetcutfile << " correctly" << std::endl;
			
		else {
		
			if( !cut_file->GetListOfKeys()->Contains( targetcutname.data() ) )
				std::cout << "Couldn't find " << targetcutname << " in " << targetcutfile << std::endl;
			else
				target_cut = (TCutG*)cut_file->Get( targetcutname.data() )->Clone();

		}
		
		cut_file->Close();
		
	}

	// Assign an empty cut file if none is given, so the code doesn't crash
	if( !beam_cut ) beam_cut = new TCutG();
	if( !target_cut ) target_cut = new TCutG();

	
	// EBIS time window
	EBIS_On = config->GetValue( "EBIS_On", 1.2e6 );		// normally 1.2 ms in slow extraction
	EBIS_Off = config->GetValue( "EBIS_Off", 2.52e7 );	// this allows a off window 20 times bigger than on

	// Target offset
	x_offset = config->GetValue( "TargetOffset.X", 0.0 );	// of course this should be 0.0 if you centre the beam! Units of mm, vertical
	y_offset = config->GetValue( "TargetOffset.Y", 0.0 );	// of course this should be 0.0 if you centre the beam! Units of mm, horizontal

	
	// Some diagnostics and info
	std::cout << std::endl << " +++  ";
	std::cout << Beam.GetIsotope() << "(" << Target.GetIsotope() << ",";
	std::cout << Ejectile.GetIsotope() << ")" << Recoil.GetIsotope();
	std::cout << "  +++" << std::endl << "Beam energy = ";
	std::cout << Beam.GetEnergyLab()*0.001 << " MeV" << std::endl;
	std::cout << "Q-value = " << GetQvalue()*0.001 << " MeV" << std::endl;

	// Finished
	delete config;

}

float Reaction::GetParticleTheta( ParticleEvt *p ){
	
	return GetParticleTheta( p->GetDetector(), p->GetSector(), p->GetStripP() );
	
}

float Reaction::GetParticleTheta( unsigned char det, unsigned char sec, unsigned char strip ){
	
	// Create a TVector3 to handle the angles
	float x = 9.0 + ( 15.5 - strip ) * 2.0;
	TVector3 vec( x, 0, cd_dist );
	if( det == 1 ) vec.SetZ( -1.0 * cd_dist ); // backwards CD
	
	// Rotate by the phi angle
	float phi = TMath::PiOver2() * sec;
	phi += cd_offset * TMath::DegToRad();
	vec.RotateZ( phi );
	
	// Apply the X and Y offsets directly to the TVector3 input
	// We move the CD opposite to the target, which replicates the same
	// geometrical shift that is observed with respect to the beam
	vec.SetX( vec.X() - x_offset );
	vec.SetY( vec.Y() - y_offset );

	return vec.Theta();

}

