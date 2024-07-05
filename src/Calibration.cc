#include "Calibration.hh"

ClassImp(FebexMWD)
ClassImp(MiniballCalibration)

void FebexMWD::DoMWD() {
	
	// For now use James' naming convention and switch later
	unsigned int L = rise_time + 3; // 3 clock cycles delay in VHDL
	unsigned int M = window + 3; // 3 clock cycles delay in VHDL
	unsigned int torr = decay_time;
	//unsigned int cfd_trig_delay = M + flat_top;

	// Get the trace length
	unsigned int trace_length = trace.size();
	
	// Baseline energy estimation
	float baseline_energy = 0.0;
	
	// resize vectors
	stage1.resize( trace_length, 0.0 );
	stage2.resize( trace_length, 0.0 );
	stage3.resize( trace_length, 0.0 );
	stage4.resize( trace_length, 0.0 );
	differential.resize( trace_length, 0.0 );
	shaper.resize( trace_length, 0.0 );
	cfd.resize( trace_length, 0.0 );
	
	// Initialise the clipped flag to false
	clipped = false;

	// skip first few samples?
	unsigned int skip = 8;
	
	// Loop over trace and analyse
	for( unsigned int i = 0; i < trace_length; ++i ) {
		
		// Check if we are clipped
		if( trace[i] == 0 || (trace[i] & 0x0000FFFF) == 0x0000FFFF )
			clipped = true;

		// Make some default values for derived pulses
		differential[i] = 0;
		shaper[i] = 0;
		cfd[i] = 0;
		//stage1[i] = 0;
		//stage2[i] = 0;
		//stage3[i] = 0;
		//stage4[i] = 0;
		
		// James' simple CFD currently on firmware
		shaper[i] = trace[i];
		fraction = 1.0;

		// Shaped pulse
		if( i >= cfd_shaping_time + skip && i >= cfd_integration_time + skip ) {
			
			// James - differential-integrating shaper
			differential[i] = trace[i] - trace[i-cfd_shaping_time];
			for( unsigned int j = 1; j <= cfd_integration_time; ++j )
				shaper[i] += differential[i-j];
			shaper[i] /= cfd_integration_time;
			
			
			// Liam - simple differential shaper
			//shaper[i] = trace[i] - trace[i-cfd_shaping_time];
			
		}

		// CFD trace, do triggering later
		if( i >= cfd_delay + skip ) {

			// James + Liam both the same here
			cfd[i]  = fraction * shaper[i];
			cfd[i] -= shaper[i-cfd_delay];

		}
		
		// Now we need to be longer than M
		if( i >= M + skip ) {
			
			// MWD stage 1 - difference
			// this is 'D' in James' MATLAB code
			stage1[i]  = (int)trace[i];
			stage1[i] -= (int)trace[i-M];
			
		
			// MWD stage 2 - remove decay and average
			// this is 'MA' in James' MATLAB code
			stage2[i] = 0;
			for( unsigned int j = 1; j <= M; ++j )
				stage2[i] += (int)trace[i-j];
			stage2[i] /= torr;
			
			// MWD stage 3 - moving average
			// this is 'MWD' in James' MATLAB code
			stage3[i] = stage1[i] + stage2[i];
			
		}
		
		// MWD stage 4 - energy averaging
		// This is 'T' in James' MWD code
		if( i >= L + skip ){
			
			for( unsigned int j = 1; j <= L; ++j )
				stage4[i] += stage3[i-j];
			
			stage4[i] /= L;
			
		}
				
	} // loop over trace
	
	
	// Loop now over the CFD trace until we trigger
	// This is not the same as James' trigger, but it's better
	// plus he has updated his CFD and I don't have the new one
	for( unsigned int i = skip; i < trace_length; ++i ) {
		
		// Trigger when we pass the threshold on the CFD
		if( i > cfd_delay + skip &&
		   ( ( cfd[i] > threshold && threshold > 0 ) ||
			( cfd[i] < threshold && threshold < 0 ) ) ) {
			
			// Mark the arming threshold point
			unsigned int armed_at = i;
			
			// Find zero crossing - Liam version, but James effects the same thing
			bool xing = false;
			while( !xing && i < trace_length ) {
			
				// Move to the next sample
				i++;
				
				// Reject incorrect polarity - Liam version, but James effects the same thing
				if( threshold < 0 && cfd[i-1] > 0 ) continue;
				if( threshold > 0 && cfd[i-1] < 0 ) continue;
			
				// Found a zero-crossing
				if( cfd[i] * cfd[i-1] < 0 ) xing = true;
				
			}
			
			// If we didn't find a crossing, stop now
			if( !xing ) continue;
			

			// Check we have enough trace left to analyse
			if( trace_length - i < flat_top )
				break;
			
			// Mark the CFD time - James
			cfd_list.push_back( i );

			// Mark the CFD time - Liam
			//float cfd_time = (float)i / TMath::Abs(cfd[i]);
			//cfd_time += (float)(i-1) / TMath::Abs(cfd[i-1]);
			//cfd_time /= 1.0 / TMath::Abs(cfd[i]) + 1.0 / TMath::Abs(cfd[i-1]);
			//cfd_list.push_back( cfd_time );
			
			// Baseline estimation comes from averaged trace
			// Just in case a trigger comes before the baseline length, use whatever we can
			if( cfd_list.size() == 1 ) {
				
				// First trigger with good baseline
				if( i >= baseline_length )
					baseline_energy = stage4[i-baseline_length];
				
				// Or just use the first sample
				else baseline_energy = stage4[0];
			}
			
			else {
				
				// Not the first trigger but still with good baseline
				if( i >= baseline_length + cfd_list.back() )
					baseline_energy = stage4[i-baseline_length];
				
				// Otherwise don't bother updating baseline, assume its the same
				
			}
			
			// move to peak of the flat top
			i += flat_top;

			// assess the energy from stage 4 and push back
			energy_list.push_back( stage4[i] - baseline_energy );
			//energy_list.push_back( stage4[i] );
			
			// Move to the end of the whole thing
			//i += M + L - flat_top;
			
			// Check we are beyond the trigger hold off
			if( i < armed_at + cfd_hold )
				i = armed_at + cfd_hold;

		} // threshold passed
		
	} // loop over CFD
	
	return;
	
}

MiniballCalibration::MiniballCalibration( std::string filename, std::shared_ptr<MiniballSettings> myset ) {

	SetFile( filename );
	set = myset;
	fRand = std::make_unique<TRandom>();
	default_qint = false;

}

void MiniballCalibration::ReadCalibration() {

	std::unique_ptr<TEnv> config = std::make_unique<TEnv>( fInputFile.data() );
	
	default_MWD_Decay		= 5000;
	default_MWD_Rise		= 780; // L
	default_MWD_Top			= 870; // mwd_cfd_trig_delay
	default_MWD_Baseline	= 60;  // delay MWD in James' firmware
	default_MWD_Window		= 880; // M
	default_CFD_Delay		= 30;
	default_CFD_HoldOff		= 100; // prevent double triggering?
	default_CFD_Shaping		= 15;
	default_CFD_Integration	= 10;
	default_CFD_Threshold	= 500;
	default_CFD_Fraction	= 0.3;
	
	std::string default_type = config->GetValue( "febex_default.Type", "Qshort" );
	if( default_qint ) default_type = "Qint"; // override for MBS

	
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
	fFebexMWD_Baseline.resize( set->GetNumberOfFebexSfps() );
	fFebexMWD_Window.resize( set->GetNumberOfFebexSfps() );
	fFebexCFD_Delay.resize( set->GetNumberOfFebexSfps() );
	fFebexCFD_HoldOff.resize( set->GetNumberOfFebexSfps() );
	fFebexCFD_Shaping.resize( set->GetNumberOfFebexSfps() );
	fFebexCFD_Integration.resize( set->GetNumberOfFebexSfps() );
	fFebexCFD_Threshold.resize( set->GetNumberOfFebexSfps() );
	fFebexCFD_Fraction.resize( set->GetNumberOfFebexSfps() );

	// FEBEX parameter read
	for( unsigned char i = 0; i < set->GetNumberOfFebexSfps(); i++ ){

		fFebexOffset[i].resize( set->GetNumberOfFebexBoards() );
		fFebexGain[i].resize( set->GetNumberOfFebexBoards() );
		fFebexGainQuadr[i].resize( set->GetNumberOfFebexBoards() );
		fFebexThreshold[i].resize( set->GetNumberOfFebexBoards() );
		fFebexType[i].resize( set->GetNumberOfFebexBoards() );
		fFebexTime[i].resize( set->GetNumberOfFebexBoards() );
		fFebexMWD_Decay[i].resize( set->GetNumberOfFebexBoards() );
		fFebexMWD_Rise[i].resize( set->GetNumberOfFebexBoards() );
		fFebexMWD_Top[i].resize( set->GetNumberOfFebexBoards() );
		fFebexMWD_Baseline[i].resize( set->GetNumberOfFebexBoards() );
		fFebexMWD_Window[i].resize( set->GetNumberOfFebexBoards() );
		fFebexCFD_Delay[i].resize( set->GetNumberOfFebexBoards() );
		fFebexCFD_HoldOff[i].resize( set->GetNumberOfFebexBoards() );
		fFebexCFD_Shaping[i].resize( set->GetNumberOfFebexBoards() );
		fFebexCFD_Integration[i].resize( set->GetNumberOfFebexBoards() );
		fFebexCFD_Threshold[i].resize( set->GetNumberOfFebexBoards() );
		fFebexCFD_Fraction[i].resize( set->GetNumberOfFebexBoards() );

		for( unsigned char j = 0; j < set->GetNumberOfFebexBoards(); j++ ){

			fFebexOffset[i][j].resize( set->GetNumberOfFebexChannels() );
			fFebexGain[i][j].resize( set->GetNumberOfFebexChannels() );
			fFebexGainQuadr[i][j].resize( set->GetNumberOfFebexChannels() );
			fFebexThreshold[i][j].resize( set->GetNumberOfFebexChannels() );
			fFebexType[i][j].resize( set->GetNumberOfFebexChannels() );
			fFebexTime[i][j].resize( set->GetNumberOfFebexChannels() );
			fFebexMWD_Decay[i][j].resize( set->GetNumberOfFebexChannels() );
			fFebexMWD_Rise[i][j].resize( set->GetNumberOfFebexChannels() );
			fFebexMWD_Top[i][j].resize( set->GetNumberOfFebexChannels() );
			fFebexMWD_Baseline[i][j].resize( set->GetNumberOfFebexChannels() );
			fFebexMWD_Window[i][j].resize( set->GetNumberOfFebexChannels() );
			fFebexCFD_Delay[i][j].resize( set->GetNumberOfFebexChannels() );
			fFebexCFD_HoldOff[i][j].resize( set->GetNumberOfFebexChannels() );
			fFebexCFD_Shaping[i][j].resize( set->GetNumberOfFebexChannels() );
			fFebexCFD_Integration[i][j].resize( set->GetNumberOfFebexChannels() );
			fFebexCFD_Threshold[i][j].resize( set->GetNumberOfFebexChannels() );
			fFebexCFD_Fraction[i][j].resize( set->GetNumberOfFebexChannels() );

			for( unsigned char k = 0; k < set->GetNumberOfFebexChannels(); k++ ){
				
				fFebexOffset[i][j][k] = (double)config->GetValue( Form( "febex_%d_%d_%d.Offset", i, j, k ), (double)0 );
				fFebexGain[i][j][k] = (double)config->GetValue( Form( "febex_%d_%d_%d.Gain", i, j, k ), 0.25 );
				fFebexGainQuadr[i][j][k] = (double)config->GetValue( Form( "febex_%d_%d_%d.GainQuadr", i, j, k ), (double)0 );
				fFebexThreshold[i][j][k] = (unsigned int)config->GetValue( Form( "febex_%d_%d_%d.Threshold", i, j, k ), (double)0 );
				fFebexType[i][j][k] = config->GetValue( Form( "febex_%d_%d_%d.Type", i, j, k ), default_type.data() );
				fFebexTime[i][j][k] = (long)config->GetValue( Form( "febex_%d_%d_%d.Time", i, j, k ), (double)0 );
				fFebexMWD_Decay[i][j][k] = (unsigned int)config->GetValue( Form( "febex_%d_%d_%d.MWD.DecayTime", i, j, k ), (double)default_MWD_Decay );
				fFebexMWD_Rise[i][j][k] = (unsigned int)config->GetValue( Form( "febex_%d_%d_%d.MWD.RiseTime", i, j, k ), (double)default_MWD_Rise );
				fFebexMWD_Top[i][j][k] = (unsigned int)config->GetValue( Form( "febex_%d_%d_%d.MWD.FlatTop", i, j, k ), (double)default_MWD_Top );
				fFebexMWD_Baseline[i][j][k] = (unsigned int)config->GetValue( Form( "febex_%d_%d_%d.MWD.Baseline", i, j, k ), (double)default_MWD_Baseline );
				fFebexMWD_Window[i][j][k] = (unsigned int)config->GetValue( Form( "febex_%d_%d_%d.MWD.Window", i, j, k ), (double)default_MWD_Window );
				fFebexCFD_Delay[i][j][k] = (unsigned int)config->GetValue( Form( "febex_%d_%d_%d.CFD.DelayTime", i, j, k ), (double)default_CFD_Delay );
				fFebexCFD_HoldOff[i][j][k] = (unsigned int)config->GetValue( Form( "febex_%d_%d_%d.CFD.HoldOff", i, j, k ), (double)default_CFD_HoldOff );
				fFebexCFD_Shaping[i][j][k] = (unsigned int)config->GetValue( Form( "febex_%d_%d_%d.CFD.ShapingTime", i, j, k ), (double)default_CFD_Shaping );
				fFebexCFD_Integration[i][j][k] = (unsigned int)config->GetValue( Form( "febex_%d_%d_%d.CFD.IntegrationTime", i, j, k ), (double)default_CFD_Integration );
				fFebexCFD_Threshold[i][j][k] = (int)config->GetValue( Form( "febex_%d_%d_%d.CFD.Threshold", i, j, k ), (double)default_CFD_Threshold );
				fFebexCFD_Fraction[i][j][k] = (float)config->GetValue( Form( "febex_%d_%d_%d.CFD.Fraction", i, j, k ), (double)default_CFD_Fraction );

			} // k: channel
			
		} // j: board
		
	} // i: sfp

	// ADC initialisation
	fAdcOffset.resize( set->GetNumberOfAdcModules() );
	fAdcGain.resize( set->GetNumberOfAdcModules() );
	fAdcGainQuadr.resize( set->GetNumberOfAdcModules() );
	fAdcThreshold.resize( set->GetNumberOfAdcModules() );
	fAdcTime.resize( set->GetNumberOfAdcModules() );

	// ADC parameter read
	for( unsigned char i = 0; i < set->GetNumberOfAdcModules(); i++ ){

		fAdcOffset[i].resize( set->GetMaximumNumberOfAdcChannels() );
		fAdcGain[i].resize( set->GetMaximumNumberOfAdcChannels() );
		fAdcGainQuadr[i].resize( set->GetMaximumNumberOfAdcChannels() );
		fAdcThreshold[i].resize( set->GetMaximumNumberOfAdcChannels() );
		fAdcTime[i].resize( set->GetMaximumNumberOfAdcChannels() );
	
		for( unsigned char j = 0; j < set->GetMaximumNumberOfAdcChannels(); j++ ){

			fAdcOffset[i][j] = config->GetValue( Form( "adc_%d_%d.Offset", i, j ), (double)0.0 );
			fAdcGain[i][j] = config->GetValue( Form( "adc_%d_%d.Gain", i, j ), (double)1.0 );
			fAdcGainQuadr[i][j] = config->GetValue( Form( "adc_%d_%d.GainQuadr", i, j ), (double)0.0 );
			fAdcThreshold[i][j] = (unsigned int)config->GetValue( Form( "adc_%d_%d.Threshold", i, j ), (double)0 );
			fAdcTime[i][j] = (long)config->GetValue( Form( "adc_%d_%d.Time", i, j ), (double)0 );
			
		}

	}

	// DGF initialisation
	fDgfOffset.resize( set->GetNumberOfDgfModules() );
	fDgfGain.resize( set->GetNumberOfDgfModules() );
	fDgfGainQuadr.resize( set->GetNumberOfDgfModules() );
	fDgfThreshold.resize( set->GetNumberOfDgfModules() );
	fDgfTime.resize( set->GetNumberOfDgfModules() );

	// DGF parameter read
	for( unsigned char i = 0; i < set->GetNumberOfDgfModules(); i++ ){

		fDgfOffset[i].resize( set->GetNumberOfDgfChannels() );
		fDgfGain[i].resize( set->GetNumberOfDgfChannels() );
		fDgfGainQuadr[i].resize( set->GetNumberOfDgfChannels() );
		fDgfThreshold[i].resize( set->GetNumberOfDgfChannels() );
		fDgfTime[i].resize( set->GetNumberOfDgfChannels() );
	
		for( unsigned char j = 0; j < set->GetNumberOfDgfChannels(); j++ ){

			fDgfOffset[i][j] = config->GetValue( Form( "dgf_%d_%d.Offset", i, j ), (double)0.0 );
			fDgfGain[i][j] = config->GetValue( Form( "dgf_%d_%d.Gain", i, j ), (double)1.0 );
			fDgfGainQuadr[i][j] = config->GetValue( Form( "dgf_%d_%d.GainQuadr", i, j ), (double)0.0 );
			fDgfThreshold[i][j] = (unsigned int)config->GetValue( Form( "dgf_%d_%d.Threshold", i, j ), (double)0 );
			fDgfTime[i][j] = (long)config->GetValue( Form( "dgf_%d_%d.Time", i, j ), (double)0 );
			
		}

	}

}

float MiniballCalibration::FebexEnergy( unsigned char sfp, unsigned char board, unsigned char ch, unsigned int raw ) {
	
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

float MiniballCalibration::DgfEnergy( unsigned char mod, unsigned char ch, unsigned int raw ) {
	
	float energy, raw_rand;
	
	if( mod < set->GetNumberOfDgfModules() &&
	     ch < set->GetNumberOfDgfChannels() ) {

		raw_rand = raw + 0.5 - fRand->Uniform();

		energy  = fDgfGain[mod][ch] * raw_rand;
		energy += fDgfOffset[mod][ch];

		// Check if we have defaults
		if( TMath::Abs( fDgfGain[mod][ch] - 1.0 ) < 1e-6 &&
		    TMath::Abs( fDgfOffset[mod][ch] ) < 1e-6 )
			
			return raw;
		
		else return energy;
		
	}
	
	return -1;
	
}

float MiniballCalibration::AdcEnergy( unsigned char mod, unsigned char ch, unsigned int raw ) {
	
	float energy, raw_rand;
	
	if( mod < set->GetNumberOfAdcModules() &&
	     ch < set->GetMaximumNumberOfAdcChannels() ) {

		raw_rand = raw + 0.5 - fRand->Uniform();

		energy  = fAdcGain[mod][ch] * raw_rand;
		energy += fAdcOffset[mod][ch];

		// Check if we have defaults
		if( TMath::Abs( fAdcGain[mod][ch] - 1.0 ) < 1e-6 &&
		    TMath::Abs( fAdcOffset[mod][ch] ) < 1e-6 )
			
			return raw;
		
		else return energy;
		
	}
	
	return -1;
	
}

FebexMWD MiniballCalibration::DoMWD( unsigned char sfp, unsigned char board, unsigned char ch, std::vector<unsigned short> trace ) {
	
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
		mwd.SetBaseline( fFebexMWD_Baseline[sfp][board][ch] );
		mwd.SetWindow( fFebexMWD_Window[sfp][board][ch] );
		mwd.SetDelayTime( fFebexCFD_Delay[sfp][board][ch] );
		mwd.SetHoldOff( fFebexCFD_HoldOff[sfp][board][ch] );
		mwd.SetShapingTime( fFebexCFD_Shaping[sfp][board][ch] );
		mwd.SetIntegrationTime( fFebexCFD_Integration[sfp][board][ch] );
		mwd.SetThreshold( fFebexCFD_Threshold[sfp][board][ch] );
		mwd.SetFraction( fFebexCFD_Fraction[sfp][board][ch] );

		// Run the MWD
		mwd.DoMWD();
		
	}

	return mwd;
	
}

double MiniballCalibration::FebexOffset( unsigned char sfp, unsigned char board, unsigned char ch ) {
	
	if(   sfp < set->GetNumberOfFebexSfps() &&
	    board < set->GetNumberOfFebexBoards() &&
	       ch < set->GetNumberOfFebexChannels() ) {

		return fFebexOffset[sfp][board][ch];
		
	}
	
	return 0.0;
	
}

double MiniballCalibration::FebexGain( unsigned char sfp, unsigned char board, unsigned char ch ) {
	
	if(   sfp < set->GetNumberOfFebexSfps() &&
	    board < set->GetNumberOfFebexBoards() &&
	       ch < set->GetNumberOfFebexChannels() ) {

		return fFebexGain[sfp][board][ch];
		
	}
	
	return 1.0;
	
}

double MiniballCalibration::FebexGainQuadr( unsigned char sfp, unsigned char board, unsigned char ch ) {
	
	if(   sfp < set->GetNumberOfFebexSfps() &&
	    board < set->GetNumberOfFebexBoards() &&
	       ch < set->GetNumberOfFebexChannels() ) {

		return fFebexGainQuadr[sfp][board][ch];
		
	}
	
	return 0.0;
	
}

unsigned int MiniballCalibration::FebexThreshold( unsigned char sfp, unsigned char board, unsigned char ch ) {
	
	if(   sfp < set->GetNumberOfFebexSfps() &&
	    board < set->GetNumberOfFebexBoards() &&
	       ch < set->GetNumberOfFebexChannels() ) {

		return fFebexThreshold[sfp][board][ch];
		
	}
	
	return -1;
	
}

long MiniballCalibration::FebexTime( unsigned char sfp, unsigned char board, unsigned char ch ){
	
	if(   sfp < set->GetNumberOfFebexSfps() &&
	    board < set->GetNumberOfFebexBoards() &&
	       ch < set->GetNumberOfFebexChannels() ) {

		return fFebexTime[sfp][board][ch];
		
	}
	
	return 0;
	
}

std::string MiniballCalibration::FebexType( unsigned char sfp, unsigned char board, unsigned char ch ){
	
	if(   sfp < set->GetNumberOfFebexSfps() &&
	    board < set->GetNumberOfFebexBoards() &&
	       ch < set->GetNumberOfFebexChannels() ) {
		
		return fFebexType[sfp][board][ch];
		
	}
	
	return 0;
	
}

double MiniballCalibration::DgfOffset( unsigned char mod, unsigned char ch ) {
	
	if( mod < set->GetNumberOfDgfModules() &&
	     ch < set->GetNumberOfDgfChannels() ) {

		return fDgfOffset[mod][ch];
		
	}
	
	return 0.0;
	
}

double MiniballCalibration::DgfGain( unsigned char mod, unsigned char ch ) {
	
	if( mod < set->GetNumberOfDgfModules() &&
	     ch < set->GetNumberOfDgfChannels() ) {

		return fDgfGain[mod][ch];
		
	}
	
	return 1.0;
	
}

double MiniballCalibration::DgfGainQuadr( unsigned char mod, unsigned char ch ) {
	
	if( mod < set->GetNumberOfDgfModules() &&
	     ch < set->GetNumberOfDgfChannels() ) {

		return fDgfGainQuadr[mod][ch];
		
	}
	
	return 0.0;
	
}

unsigned int MiniballCalibration::DgfThreshold( unsigned char mod, unsigned char ch ) {
	
	if( mod < set->GetNumberOfDgfModules() &&
	     ch < set->GetNumberOfDgfChannels() ) {

		return fDgfThreshold[mod][ch];
		
	}
	
	return -1;
	
}

long MiniballCalibration::DgfTime( unsigned char mod, unsigned char ch ){
	
	if( mod < set->GetNumberOfDgfModules() &&
	     ch < set->GetNumberOfDgfChannels() ) {

		return fDgfTime[mod][ch];
		
	}
	
	return 0;
	
}
double MiniballCalibration::AdcOffset( unsigned char mod, unsigned char ch ) {
	
	if( mod < set->GetNumberOfAdcModules() &&
	     ch < set->GetMaximumNumberOfAdcChannels() ) {

		return fAdcOffset[mod][ch];
		
	}
	
	return 0.0;
	
}

double MiniballCalibration::AdcGain( unsigned char mod, unsigned char ch ) {
	
	if( mod < set->GetNumberOfAdcModules() &&
	     ch < set->GetMaximumNumberOfAdcChannels() ) {

		return fAdcGain[mod][ch];
		
	}
	
	return 1.0;
	
}

double MiniballCalibration::AdcGainQuadr( unsigned char mod, unsigned char ch ) {
	
	if( mod < set->GetNumberOfAdcModules() &&
	     ch < set->GetMaximumNumberOfAdcChannels() ) {

		return fAdcGainQuadr[mod][ch];
		
	}
	
	return 0.0;
	
}

unsigned int MiniballCalibration::AdcThreshold( unsigned char mod, unsigned char ch ) {
	
	if( mod < set->GetNumberOfAdcModules() &&
	     ch < set->GetMaximumNumberOfAdcChannels() ) {

		return fAdcThreshold[mod][ch];
		
	}
	
	return -1;
	
}

long MiniballCalibration::AdcTime( unsigned char mod, unsigned char ch ){
	
	if( mod < set->GetNumberOfAdcModules() &&
	     ch < set->GetMaximumNumberOfAdcChannels() ) {

		return fAdcTime[mod][ch];
		
	}
	
	return 0;
	
}


// MWD
void MiniballCalibration::SetMWDDecay( unsigned char sfp, unsigned char board, unsigned char ch, unsigned int decay ){
	
	if(   sfp < set->GetNumberOfFebexSfps() &&
	    board < set->GetNumberOfFebexBoards() &&
	       ch < set->GetNumberOfFebexChannels() ) {
		
		fFebexMWD_Decay[sfp][board][ch] = decay;
		return;
		
	}
	
	else return;
	
}

void MiniballCalibration::SetMWDRise( unsigned char sfp, unsigned char board, unsigned char ch, unsigned int rise ){
	
	if(   sfp < set->GetNumberOfFebexSfps() &&
		board < set->GetNumberOfFebexBoards() &&
	       ch < set->GetNumberOfFebexChannels() ) {
		
		fFebexMWD_Rise[sfp][board][ch] = rise;
		return;
		
	}
	
	else return;
	
}

void MiniballCalibration::SetMWDFlatTop( unsigned char sfp, unsigned char board, unsigned char ch, unsigned int top ){
	
	if(   sfp < set->GetNumberOfFebexSfps() &&
	    board < set->GetNumberOfFebexBoards() &&
	       ch < set->GetNumberOfFebexChannels() ) {
		
		fFebexMWD_Top[sfp][board][ch] = top;
		return;
		
	}
	
	else return;
	
}

void MiniballCalibration::SetMWDBaseline( unsigned char sfp, unsigned char board, unsigned char ch, unsigned int baseline_length ){
	
	if(   sfp < set->GetNumberOfFebexSfps() &&
	    board < set->GetNumberOfFebexBoards() &&
	       ch < set->GetNumberOfFebexChannels() ) {
		
		fFebexMWD_Baseline[sfp][board][ch] = baseline_length;
		return;
		
	}
	
	else return;
	
}

void MiniballCalibration::SetMWDWindow( unsigned char sfp, unsigned char board, unsigned char ch, unsigned int window ){
	
	if(   sfp < set->GetNumberOfFebexSfps() &&
	    board < set->GetNumberOfFebexBoards() &&
	       ch < set->GetNumberOfFebexChannels() ) {
		
		fFebexMWD_Window[sfp][board][ch] = window;
		return;
		
	}
	
	else return;
	
}

void MiniballCalibration::SetCFDFraction( unsigned char sfp, unsigned char board, unsigned char ch, float fraction ){
	
	if(   sfp < set->GetNumberOfFebexSfps() &&
	    board < set->GetNumberOfFebexBoards() &&
	       ch < set->GetNumberOfFebexChannels() ) {
		
		fFebexCFD_Fraction[sfp][board][ch] = fraction;
		return;
		
	}
	
	else return;
	
}

void MiniballCalibration::SetCFDDelay( unsigned char sfp, unsigned char board, unsigned char ch, unsigned int delay ){
	
	if(   sfp < set->GetNumberOfFebexSfps() &&
	    board < set->GetNumberOfFebexBoards() &&
	       ch < set->GetNumberOfFebexChannels() ) {
		
		fFebexCFD_Delay[sfp][board][ch] = delay;
		return;
		
	}
	
	else return;
	
}

void MiniballCalibration::SetCFDHoldOff( unsigned char sfp, unsigned char board, unsigned char ch, unsigned int hold ){
	
	if(   sfp < set->GetNumberOfFebexSfps() &&
		board < set->GetNumberOfFebexBoards() &&
	       ch < set->GetNumberOfFebexChannels() ) {
		
		fFebexCFD_HoldOff[sfp][board][ch] = hold;
		return;
		
	}
	
	else return;
	
}

void MiniballCalibration::SetCFDShapingTime( unsigned char sfp, unsigned char board, unsigned char ch, unsigned int shaping ){
	
	if(   sfp < set->GetNumberOfFebexSfps() &&
	    board < set->GetNumberOfFebexBoards() &&
	       ch < set->GetNumberOfFebexChannels() ) {
		
		fFebexCFD_Shaping[sfp][board][ch] = shaping;
		return;
		
	}
	
	else return;
	
}

void MiniballCalibration::SetCFDIntegrationTime( unsigned char sfp, unsigned char board, unsigned char ch, unsigned int integration ){
	
	if(   sfp < set->GetNumberOfFebexSfps() &&
	    board < set->GetNumberOfFebexBoards() &&
	       ch < set->GetNumberOfFebexChannels() ) {
		
		fFebexCFD_Integration[sfp][board][ch] = integration;
		return;
		
	}
	
	else return;
	
}

void MiniballCalibration::SetCFDThreshold( unsigned char sfp, unsigned char board, unsigned char ch, int threshold ){
	
	if(   sfp < set->GetNumberOfFebexSfps() &&
	    board < set->GetNumberOfFebexBoards() &&
	       ch < set->GetNumberOfFebexChannels() ) {
		
		fFebexCFD_Threshold[sfp][board][ch] = threshold;
		return;
		
	}
	
	else return;
	
}

unsigned int MiniballCalibration::GetMWDDecay( unsigned char sfp, unsigned char board, unsigned char ch ){
	
	if(   sfp < set->GetNumberOfFebexSfps() &&
	    board < set->GetNumberOfFebexBoards() &&
	       ch < set->GetNumberOfFebexChannels() ) {
		
		return fFebexMWD_Decay[sfp][board][ch];
		
	}
	
	else return 0;
	
}

unsigned int MiniballCalibration::GetMWDRise( unsigned char sfp, unsigned char board, unsigned char ch ){
	
	if(   sfp < set->GetNumberOfFebexSfps() &&
	    board < set->GetNumberOfFebexBoards() &&
	       ch < set->GetNumberOfFebexChannels() ) {
		
		return fFebexMWD_Rise[sfp][board][ch];
		
	}
	
	else return 0;
	
}

unsigned int MiniballCalibration::GetMWDFlatTop( unsigned char sfp, unsigned char board, unsigned char ch ){
	
	if(   sfp < set->GetNumberOfFebexSfps() &&
	    board < set->GetNumberOfFebexBoards() &&
	       ch < set->GetNumberOfFebexChannels() ) {
		
		return fFebexMWD_Top[sfp][board][ch];
		
	}
	
	else return 0;
	
}

unsigned int MiniballCalibration::GetMWDBaseline( unsigned char sfp, unsigned char board, unsigned char ch ){
	
	if(   sfp < set->GetNumberOfFebexSfps() &&
	    board < set->GetNumberOfFebexBoards() &&
	       ch < set->GetNumberOfFebexChannels() ) {
		
		return fFebexMWD_Baseline[sfp][board][ch];
		
	}
	
	else return 0;
	
}

unsigned int MiniballCalibration::GetMWDWindow( unsigned char sfp, unsigned char board, unsigned char ch ){
	
	if(   sfp < set->GetNumberOfFebexSfps() &&
	    board < set->GetNumberOfFebexBoards() &&
	       ch < set->GetNumberOfFebexChannels() ) {
		
		return fFebexMWD_Window[sfp][board][ch];
		
	}
	
	else return 0;
	
}

float MiniballCalibration::GetCFDFraction( unsigned char sfp, unsigned char board, unsigned char ch ){
	
	if(   sfp < set->GetNumberOfFebexSfps() &&
	    board < set->GetNumberOfFebexBoards() &&
	       ch < set->GetNumberOfFebexChannels() ) {
		
		return fFebexCFD_Fraction[sfp][board][ch];
		
	}
	
	else return 0;
	
}

unsigned int MiniballCalibration::GetCFDDelay( unsigned char sfp, unsigned char board, unsigned char ch ){
	
	if(   sfp < set->GetNumberOfFebexSfps() &&
	    board < set->GetNumberOfFebexBoards() &&
		   ch < set->GetNumberOfFebexChannels() ) {
		
		return fFebexCFD_Delay[sfp][board][ch];
		
	}
	
	else return 0;
	
}

unsigned int MiniballCalibration::GetCFDHoldOff( unsigned char sfp, unsigned char board, unsigned char ch ){
	
	if(   sfp < set->GetNumberOfFebexSfps() &&
	    board < set->GetNumberOfFebexBoards() &&
	       ch < set->GetNumberOfFebexChannels() ) {
		
		return fFebexCFD_HoldOff[sfp][board][ch];
		
	}
	
	else return 0;
	
}

unsigned int MiniballCalibration::GetCFDShapingTime( unsigned char sfp, unsigned char board, unsigned char ch ){
	
	if(   sfp < set->GetNumberOfFebexSfps() &&
	    board < set->GetNumberOfFebexBoards() &&
	       ch < set->GetNumberOfFebexChannels() ) {
		
		return fFebexCFD_Shaping[sfp][board][ch];
		
	}
	
	else return 0;
	
}

unsigned int MiniballCalibration::GetCFDIntegrationTime( unsigned char sfp, unsigned char board, unsigned char ch ){
	
	if(   sfp < set->GetNumberOfFebexSfps() &&
	    board < set->GetNumberOfFebexBoards() &&
	       ch < set->GetNumberOfFebexChannels() ) {
		
		return fFebexCFD_Integration[sfp][board][ch];
		
	}
	
	else return 0;
	
}

int MiniballCalibration::GetCFDThreshold( unsigned char sfp, unsigned char board, unsigned char ch ){
	
	if(   sfp < set->GetNumberOfFebexSfps() &&
	    board < set->GetNumberOfFebexBoards() &&
	       ch < set->GetNumberOfFebexChannels() ) {
		
		return fFebexCFD_Threshold[sfp][board][ch];
		
	}
	
	else return 0;
	
}

