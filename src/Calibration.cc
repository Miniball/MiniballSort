#include "Calibration.hh"

ClassImp(FebexMWD)
ClassImp(Calibration)

void FebexMWD::DoMWD() {
		
	// Define the peaking time for this channel based on rise time then go to centre of flat top
	float mwd_peaking_time = rise_time;
	mwd_peaking_time += flat_top / 2.0;
	
	// Define the CFD peaking time for this channel based on rise time then go to centre of flat top
	float cfd_peaking_time = rise_time / 2.0;
	cfd_peaking_time += flat_top / 2.0;

	// Get the trace length
	unsigned int trace_length = trace.size();
	
	// resize vectors
	stage1.resize( trace_length, 0.0 );
	stage2.resize( trace_length, 0.0 );
	stage3.resize( trace_length, 0.0 );
	stage1_int.resize( trace_length, 0 );
	stage2_int.resize( trace_length, 0 );
	stage3_int.resize( trace_length, 0 );
	diff1.resize( trace_length, 0 );
	diff2.resize( trace_length, 0 );
	
	
	// Loop over trace and analyse
	for( unsigned int i = 0; i < trace_length; ++i ) {
		
		// MWD stage 1 - remove decay
		if( i > 0 ) {
			
			stage1[i]  = 1.0 / decay_time;
			stage1[i] -= 1.0;
			stage1[i] *= trace[i-1];
			stage1[i] += trace[i];
			stage1[i] += stage1[i-1];
			stage1_int[i] = (unsigned int)(stage1[i]+0.5);
			
		}
		
		// MWD stage 2 - difference
		if( i > (unsigned int)(flat_top+0.5) ) {
			
			stage2[i]  = stage1[i];
			stage2[i] -= stage1[i-(unsigned int)(flat_top+0.5)];
			stage2_int[i] = (unsigned int)(stage2[i]+0.5);
			
		}
		
		// MWD stage 3 - moving average
		if( i >= (unsigned int)(rise_time+0.5) ) {
			
			for( unsigned int j = 0; j < (unsigned int)(rise_time+0.5); ++j )
				stage3[i] += stage2[i-j];
			
			stage3[i] /= rise_time;
			stage3_int[i] = (unsigned int)(stage3[i]+0.5);
			
		}
		
		// some kind of cfd trigger for thresholding
		if( i >= (unsigned int)(diff_width+0.5) ) {
			
			unsigned int buff = trace[i] - trace[i-(unsigned int)(diff_width+0.5)];
			if( buff > 0 ) diff1[i] = buff;
			buff = diff1[i] - diff1[i-(unsigned int)(diff_width+0.5)];
			diff2[i] = buff + 1000;
			
		}
		
	} // loop over trace
	
	
	// Loop now over the CFD trace until we trigger
	for( unsigned int i = 0; i < trace_length; ++i ) {
		
		// Trigger when we pass the threshold on the CFD
		if( diff2[i] > threshold ) {
			
			// intialise energy to be zero to start
			float energy = 0.0;
			
			// move to centre of flat top
			i += (int)(cfd_peaking_time+0.5);
			
			// Go back to the start of the averaging window
			i -= window / 2.0;
			
			// average energy over window
			for( unsigned int j = i; j < i + window; ++j )
				energy += stage3_int[j];
			
			energy_list.push_back( energy / (float)window );
			
			// move to the back to the centre then the end of the peak
			i -= window / 2.0;
			i += (int)(cfd_peaking_time+0.5);
			
		}
		
	} // loop over CFD
	
	return;
	
}



Calibration::Calibration() {

	SetFile( "dummy" );
	set = std::make_shared<Settings>();
	ReadCalibration();

}

Calibration::Calibration( std::string filename, std::shared_ptr<Settings> myset ) {

	SetFile( filename );
	set = myset;
	ReadCalibration();
		
}

void Calibration::ReadCalibration() {

	std::unique_ptr<TEnv> config( new TEnv( fInputFile.data() ) );
	
	default_MWD_Decay		= 6000.0;
	default_MWD_Rise		= 100.;
	default_MWD_Top			= 250.;
	default_MWD_Window		= 10;
	default_MWD_Diff		= 175;
	default_MWD_Threshold	= 1100;

	
	// FEBEX initialisation
	fFebexOffset.resize( set->GetNumberOfFebexSfps() );
	fFebexGain.resize( set->GetNumberOfFebexSfps() );
	fFebexGainQuadr.resize( set->GetNumberOfFebexSfps() );
	fFebexThreshold.resize( set->GetNumberOfFebexSfps() );
	fFebexTime.resize( set->GetNumberOfFebexSfps() );
	fFebexMWD_Decay.resize( set->GetNumberOfFebexSfps() );
	fFebexMWD_Rise.resize( set->GetNumberOfFebexSfps() );
	fFebexMWD_Top.resize( set->GetNumberOfFebexSfps() );
	fFebexMWD_Window.resize( set->GetNumberOfFebexSfps() );
	fFebexMWD_Diff.resize( set->GetNumberOfFebexSfps() );
	fFebexMWD_Threshold.resize( set->GetNumberOfFebexSfps() );

	// FEBEX parameter read
	for( unsigned int i = 0; i < set->GetNumberOfFebexSfps(); i++ ){

		fFebexOffset[i].resize( set->GetNumberOfFebexBoards() );
		fFebexGain[i].resize( set->GetNumberOfFebexBoards() );
		fFebexGainQuadr[i].resize( set->GetNumberOfFebexBoards() );
		fFebexThreshold[i].resize( set->GetNumberOfFebexBoards() );
		fFebexTime[i].resize( set->GetNumberOfFebexBoards() );
		fFebexMWD_Decay[i].resize( set->GetNumberOfFebexBoards() );
		fFebexMWD_Rise[i].resize( set->GetNumberOfFebexBoards() );
		fFebexMWD_Top[i].resize( set->GetNumberOfFebexBoards() );
		fFebexMWD_Window[i].resize( set->GetNumberOfFebexBoards() );
		fFebexMWD_Diff[i].resize( set->GetNumberOfFebexBoards() );
		fFebexMWD_Threshold[i].resize( set->GetNumberOfFebexBoards() );

		for( unsigned int j = 0; j < set->GetNumberOfFebexBoards(); j++ ){

			fFebexOffset[i][j].resize( set->GetNumberOfFebexChannels() );
			fFebexGain[i][j].resize( set->GetNumberOfFebexChannels() );
			fFebexGainQuadr[i][j].resize( set->GetNumberOfFebexChannels() );
			fFebexThreshold[i][j].resize( set->GetNumberOfFebexChannels() );
			fFebexTime[i][j].resize( set->GetNumberOfFebexChannels() );
			fFebexMWD_Decay[i][j].resize( set->GetNumberOfFebexChannels() );
			fFebexMWD_Rise[i][j].resize( set->GetNumberOfFebexChannels() );
			fFebexMWD_Top[i][j].resize( set->GetNumberOfFebexChannels() );
			fFebexMWD_Window[i][j].resize( set->GetNumberOfFebexChannels() );
			fFebexMWD_Diff[i][j].resize( set->GetNumberOfFebexChannels() );
			fFebexMWD_Threshold[i][j].resize( set->GetNumberOfFebexChannels() );

			for( unsigned int k = 0; k < set->GetNumberOfFebexChannels(); k++ ){
				
				fFebexOffset[i][j][k] = config->GetValue( Form( "febex_%d_%d_%d.Offset", i, j, k ), 0. );
				fFebexGain[i][j][k] = config->GetValue( Form( "febex_%d_%d_%d.Gain", i, j, k ), 1. );
				fFebexGainQuadr[i][j][k] = config->GetValue( Form( "febex_%d_%d_%d.GainQuadr", i, j, k ), 0. );
				fFebexThreshold[i][j][k] = config->GetValue( Form( "febex_%d_%d_%d.Threshold", i, j, k ), 0. );
				fFebexTime[i][j][k] = config->GetValue( Form( "febex_%d_%d_%d.Time", i, j, k ), 0 );
				fFebexMWD_Decay[i][j][k] = config->GetValue( Form( "febex_%d_%d_%d.MWD.DecayTime", i, j, k ), default_MWD_Decay );
				fFebexMWD_Rise[i][j][k] = config->GetValue( Form( "febex_%d_%d_%d.MWD.RiseTime", i, j, k ), default_MWD_Rise );
				fFebexMWD_Top[i][j][k] = config->GetValue( Form( "febex_%d_%d_%d.MWD.FlatTop", i, j, k ), default_MWD_Top );
				fFebexMWD_Window[i][j][k] = config->GetValue( Form( "febex_%d_%d_%d.MWD.Window", i, j, k ), (int)default_MWD_Window );
				fFebexMWD_Diff[i][j][k] = config->GetValue( Form( "febex_%d_%d_%d.MWD.Diff", i, j, k ), (int)default_MWD_Diff );
				fFebexMWD_Threshold[i][j][k] = config->GetValue( Form( "febex_%d_%d_%d.MWD.Threshold", i, j, k ), (int)default_MWD_Threshold );

			} // k: channel
			
		} // j: board
		
	} // i: sfp

}

float Calibration::FebexEnergy( unsigned int sfp, unsigned int board, unsigned int ch, unsigned short raw ) {
	
	float energy, raw_rand;
	TRandom *fRand = new TRandom();
	
	if(   sfp < set->GetNumberOfFebexSfps() &&
	    board < set->GetNumberOfFebexBoards() &&
	       ch < set->GetNumberOfFebexChannels() ) {

		raw_rand = raw + 0.5 - fRand->Uniform();

		energy  = fFebexGainQuadr[sfp][board][ch] * raw_rand * raw_rand;
		energy += fFebexGain[sfp][board][ch] * raw_rand;
		energy += fFebexOffset[sfp][board][ch];

		// Check if we have defaults
		if( TMath::Abs( fFebexGainQuadr[sfp][board][ch] ) < 1e-6 &&
		    TMath::Abs( fFebexGain[sfp][board][ch] - 1.0 ) < 1e-6 &&
		    TMath::Abs( fFebexOffset[sfp][board][ch] ) < 1e-6 )
			
			return raw;
		
		else return energy;
		
	}
	
	delete fRand;
	
	return -1;
	
}

FebexMWD Calibration::DoMWD( unsigned int sfp, unsigned int board, unsigned int ch, std::vector<unsigned short> trace ) {
	
	// Create a FebexMWD class to hold the info
	FebexMWD mwd;
	
	// Check if it's a valid event first
	if(   sfp < set->GetNumberOfFebexSfps() &&
	    board < set->GetNumberOfFebexBoards() &&
	       ch < set->GetNumberOfFebexChannels() ) {

		// Set the parameters of the MWD
		mwd.SetTrace( trace );
		mwd.SetRiseTime( fFebexMWD_Rise[sfp][board][ch] );
		mwd.SetDecayTime( fFebexMWD_Decay[sfp][board][ch] );
		mwd.SetFlatTop( fFebexMWD_Top[sfp][board][ch] );
		mwd.SetDiffWidth( fFebexMWD_Diff[sfp][board][ch] );
		mwd.SetWindow( fFebexMWD_Window[sfp][board][ch] );
		mwd.SetThreshold( fFebexMWD_Threshold[sfp][board][ch] );
		
		// Run the MWD
		mwd.DoMWD();
		
	}

	return mwd;
	
}


float Calibration::FebexThreshold( unsigned int sfp, unsigned int board, unsigned int ch ) {
	
	if(   sfp < set->GetNumberOfFebexSfps() &&
	    board < set->GetNumberOfFebexBoards() &&
	       ch < set->GetNumberOfFebexChannels() ) {

		return fFebexThreshold[sfp][board][ch];
		
	}
	
	return -1;
	
}

long Calibration::FebexTime( unsigned int sfp, unsigned int board, unsigned int ch ){
	
	if(   sfp < set->GetNumberOfFebexSfps() &&
	    board < set->GetNumberOfFebexBoards() &&
	       ch < set->GetNumberOfFebexChannels() ) {

		return fFebexTime[sfp][board][ch];
		
	}
	
	return 0;
	
}
