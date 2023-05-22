#include "Calibration.hh"

ClassImp(FebexMWD)
ClassImp(MiniballCalibration)

void FebexMWD::DoMWD() {
		
	// Define the peaking time for this channel based on rise time then go to centre of flat top
	float peaking_time = flat_top - (float)rise_time * fraction;

	// Get the trace length
	unsigned int trace_length = trace.size();
	
	// resize vectors
	stage1.resize( trace_length, 0.0 );
	stage2.resize( trace_length, 0.0 );
	stage3.resize( trace_length, 0.0 );
	shaper.resize( trace_length, 0.0 );
	cfd.resize( trace_length, 0.0 );
	
	
	// Loop over trace and analyse
	for( unsigned int i = 1; i < trace_length; ++i ) {
		
		// MWD stage 1 - remove decay
		stage1[i]  = 1.0 / decay_time;
		stage1[i] -= 1.0;
		stage1[i] *= trace[i-1];
		stage1[i] += trace[i];
		stage1[i] += stage1[i-1];
		
		// MWD stage 2 - difference
		if( i > flat_top ) {
			
			stage2[i]  = stage1[i];
			stage2[i] -= stage1[i-flat_top];
			
		}
		
		// MWD stage 3 - moving average
		if( i >= rise_time ) {
			
			for( unsigned int j = 0; j < rise_time; ++j )
				stage3[i] += stage2[i-j];
			
			stage3[i] /= (float)rise_time;

		}
		
		// some kind of cfd trigger for thresholding
		if( i >= delay_time ) {
			
			shaper[i] = trace[i] - trace[i-delay_time];
			//if( shaper[i] < 0 ) shaper[i] = 0.0;
			cfd[i]  = fraction * shaper[i];
			cfd[i] -= shaper[i-delay_time];

		}
		
	} // loop over trace
	
	
	// Loop now over the CFD trace until we trigger
	for( unsigned int i = delay_time*2+1; i < trace_length; ++i ) {
		
		// Trigger when we pass the threshold on the CFD
		if( ( cfd[i] > threshold && threshold > 0 ) ||
		    ( cfd[i] < threshold && threshold < 0 ) ) {
			
			// Find zero crossing
			while( cfd[i] * cfd[i-1] > 0 && i < trace_length ) i++;
			
			// Reject incorrect polarity
			if( threshold < 0 && cfd[i-1] > 0 ) continue;
			if( threshold > 0 && cfd[i-1] < 0 ) continue;

			// Check we have enough trace left to analyse
			if( trace_length - i < peaking_time + window/2 )
				break;
			
			// Mark the CFD time
			float cfd_time = (float)i / cfd[i];
			cfd_time += (float)(i-1) / cfd[i-1];
			cfd_time /= 1.0 / cfd[i] + 1.0 / cfd[i-1];
			cfd_list.push_back( cfd_time );
			
			// intialise energy to be zero to start
			float energy = 0.0;
			
			// move to peak of the flat top
			i += peaking_time;
			
			// Go back to the start of the averaging window
			i -= window;
			
			// average energy over window
			for( unsigned int j = i; j < i + window; ++j )
				energy += stage3[j];
			
			energy_list.push_back( TMath::Abs(energy) / (float)window );
			
			// move back to the peak, then to the end of the trapezoid
			i += peaking_time/2;
							
		} // threshold passed
		
	} // loop over CFD
	
	return;
	
}

MiniballCalibration::MiniballCalibration( std::string filename, std::shared_ptr<MiniballSettings> myset ) {

	SetFile( filename );
	set = myset;
	ReadCalibration();
	fRand = std::make_unique<TRandom>();
		
}

void MiniballCalibration::ReadCalibration() {

	std::unique_ptr<TEnv> config = std::make_unique<TEnv>( fInputFile.data() );
	
	default_MWD_Decay		= 14000.0;
	default_MWD_Rise		= 25;
	default_MWD_Top			= 150;
	default_MWD_Window		= 12;
	default_CFD_Delay		= 5;
	default_CFD_Threshold	= 150;
	default_CFD_Fraction	= 0.5;

	
	// FEBEX initialisation
	fFebexOffset.resize( set->GetNumberOfFebexSfps() );
	fFebexGain.resize( set->GetNumberOfFebexSfps() );
	fFebexGainQuadr.resize( set->GetNumberOfFebexSfps() );
	fFebexThreshold.resize( set->GetNumberOfFebexSfps() );
	fFebexType.resize( set->GetNumberOfFebexSfps() );
	fFebexTime.resize( set->GetNumberOfFebexSfps() );
	fFebexMWD_Decay.resize( set->GetNumberOfFebexSfps() );
	fFebexMWD_Rise.resize( set->GetNumberOfFebexSfps() );
	fFebexMWD_Top.resize( set->GetNumberOfFebexSfps() );
	fFebexMWD_Window.resize( set->GetNumberOfFebexSfps() );
	fFebexCFD_Delay.resize( set->GetNumberOfFebexSfps() );
	fFebexCFD_Threshold.resize( set->GetNumberOfFebexSfps() );
	fFebexCFD_Fraction.resize( set->GetNumberOfFebexSfps() );

	// FEBEX parameter read
	for( unsigned int i = 0; i < set->GetNumberOfFebexSfps(); i++ ){

		fFebexOffset[i].resize( set->GetNumberOfFebexBoards() );
		fFebexGain[i].resize( set->GetNumberOfFebexBoards() );
		fFebexGainQuadr[i].resize( set->GetNumberOfFebexBoards() );
		fFebexThreshold[i].resize( set->GetNumberOfFebexBoards() );
		fFebexType[i].resize( set->GetNumberOfFebexBoards() );
		fFebexTime[i].resize( set->GetNumberOfFebexBoards() );
		fFebexMWD_Decay[i].resize( set->GetNumberOfFebexBoards() );
		fFebexMWD_Rise[i].resize( set->GetNumberOfFebexBoards() );
		fFebexMWD_Top[i].resize( set->GetNumberOfFebexBoards() );
		fFebexMWD_Window[i].resize( set->GetNumberOfFebexBoards() );
		fFebexCFD_Delay[i].resize( set->GetNumberOfFebexBoards() );
		fFebexCFD_Threshold[i].resize( set->GetNumberOfFebexBoards() );
		fFebexCFD_Fraction[i].resize( set->GetNumberOfFebexBoards() );

		for( unsigned int j = 0; j < set->GetNumberOfFebexBoards(); j++ ){

			fFebexOffset[i][j].resize( set->GetNumberOfFebexChannels() );
			fFebexGain[i][j].resize( set->GetNumberOfFebexChannels() );
			fFebexGainQuadr[i][j].resize( set->GetNumberOfFebexChannels() );
			fFebexThreshold[i][j].resize( set->GetNumberOfFebexChannels() );
			fFebexType[i][j].resize( set->GetNumberOfFebexChannels() );
			fFebexTime[i][j].resize( set->GetNumberOfFebexChannels() );
			fFebexMWD_Decay[i][j].resize( set->GetNumberOfFebexChannels() );
			fFebexMWD_Rise[i][j].resize( set->GetNumberOfFebexChannels() );
			fFebexMWD_Top[i][j].resize( set->GetNumberOfFebexChannels() );
			fFebexMWD_Window[i][j].resize( set->GetNumberOfFebexChannels() );
			fFebexCFD_Delay[i][j].resize( set->GetNumberOfFebexChannels() );
			fFebexCFD_Threshold[i][j].resize( set->GetNumberOfFebexChannels() );
			fFebexCFD_Fraction[i][j].resize( set->GetNumberOfFebexChannels() );

			for( unsigned int k = 0; k < set->GetNumberOfFebexChannels(); k++ ){
				
				fFebexOffset[i][j][k] = config->GetValue( Form( "febex_%d_%d_%d.Offset", i, j, k ), 0. );
				fFebexGain[i][j][k] = config->GetValue( Form( "febex_%d_%d_%d.Gain", i, j, k ), 0.0015 );
				fFebexGainQuadr[i][j][k] = config->GetValue( Form( "febex_%d_%d_%d.GainQuadr", i, j, k ), 0. );
				fFebexThreshold[i][j][k] = config->GetValue( Form( "febex_%d_%d_%d.Threshold", i, j, k ), 15000 );
				fFebexType[i][j][k] = config->GetValue( Form( "febex_%d_%d_%d.Type", i, j, k ), "Qshort" );
				fFebexTime[i][j][k] = config->GetValue( Form( "febex_%d_%d_%d.Time", i, j, k ), (double)0 );
				fFebexMWD_Decay[i][j][k] = config->GetValue( Form( "febex_%d_%d_%d.MWD.DecayTime", i, j, k ), default_MWD_Decay );
				fFebexMWD_Rise[i][j][k] = config->GetValue( Form( "febex_%d_%d_%d.MWD.RiseTime", i, j, k ), (int)default_MWD_Rise );
				fFebexMWD_Top[i][j][k] = config->GetValue( Form( "febex_%d_%d_%d.MWD.FlatTop", i, j, k ), (int)default_MWD_Top );
				fFebexMWD_Window[i][j][k] = config->GetValue( Form( "febex_%d_%d_%d.MWD.Window", i, j, k ), (int)default_MWD_Window );
				fFebexCFD_Delay[i][j][k] = config->GetValue( Form( "febex_%d_%d_%d.CFD.DelayTime", i, j, k ), (int)default_CFD_Delay );
				fFebexCFD_Threshold[i][j][k] = config->GetValue( Form( "febex_%d_%d_%d.CFD.Threshold", i, j, k ), (int)default_CFD_Threshold );
				fFebexCFD_Fraction[i][j][k] = config->GetValue( Form( "febex_%d_%d_%d.CFD.Fraction", i, j, k ), default_CFD_Fraction );

			} // k: channel
			
		} // j: board
		
	} // i: sfp

}

float MiniballCalibration::FebexEnergy( unsigned int sfp, unsigned int board, unsigned int ch, unsigned int raw ) {
	
	float energy, raw_rand;
	
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
	
	return -1;
	
}

FebexMWD MiniballCalibration::DoMWD( unsigned int sfp, unsigned int board, unsigned int ch, std::vector<unsigned short> trace ) {
	
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
		mwd.SetWindow( fFebexMWD_Window[sfp][board][ch] );
		mwd.SetDelayTime( fFebexCFD_Delay[sfp][board][ch] );
		mwd.SetThreshold( fFebexCFD_Threshold[sfp][board][ch] );
		mwd.SetFraction( fFebexCFD_Fraction[sfp][board][ch] );

		// Run the MWD
		mwd.DoMWD();
		
	}

	return mwd;
	
}

unsigned int MiniballCalibration::FebexThreshold( unsigned int sfp, unsigned int board, unsigned int ch ) {
	
	if(   sfp < set->GetNumberOfFebexSfps() &&
	    board < set->GetNumberOfFebexBoards() &&
	       ch < set->GetNumberOfFebexChannels() ) {

		return fFebexThreshold[sfp][board][ch];
		
	}
	
	return -1;
	
}

long MiniballCalibration::FebexTime( unsigned int sfp, unsigned int board, unsigned int ch ){
	
	if(   sfp < set->GetNumberOfFebexSfps() &&
	    board < set->GetNumberOfFebexBoards() &&
	       ch < set->GetNumberOfFebexChannels() ) {

		return fFebexTime[sfp][board][ch];
		
	}
	
	return 0;
	
}

std::string MiniballCalibration::FebexType( unsigned int sfp, unsigned int board, unsigned int ch ){
	
	if(   sfp < set->GetNumberOfFebexSfps() &&
	   board < set->GetNumberOfFebexBoards() &&
	   ch < set->GetNumberOfFebexChannels() ) {
		
		return fFebexType[sfp][board][ch];
		
	}
	
	return 0;
	
}
