#ifndef __CALIBRATION_HH
#define __CALIBRATION_HH

#include <iostream>
#include <iomanip>
#include <fstream>
#include <string>
#include <array>
#include <cstdlib>

#include "TSystem.h"
#include "TEnv.h"
#include "TRandom.h"
#include "TMath.h"

// Settings header
#ifndef __SETTINGS_HH
# include "Settings.hh"
#endif

/// A class to read in the calibration file in ROOT's TConfig format.
/// Each ASIC channel can have offset, gain and quadratic terms.
/// Each channel also has a threshold (not implemented)
/// and there is a time offset parameter for each ASIC module, too.

class Calibration {

public:

	Calibration( std::string filename, Settings *myset );
	virtual ~Calibration();
	void ReadCalibration();
	void PrintCalibration();
	void SetFile( std::string filename ){
		fInputFile = filename;
	}
	const std::string InputFile(){
		return fInputFile;
	}
	float FebexEnergy( unsigned int sfp, unsigned int board, unsigned int ch, unsigned short raw );
	float FebexThreshold( unsigned int sfp, unsigned int board, unsigned int ch );
	long FebexTime( unsigned int sfp, unsigned int board, unsigned int ch );
	


private:

	std::string fInputFile;
	
	Settings *set;

	std::vector< std::vector<std::vector<float>> > fFebexOffset;
	std::vector< std::vector<std::vector<float>> > fFebexGain;
	std::vector< std::vector<std::vector<float>> > fFebexGainQuadr;
	std::vector< std::vector<std::vector<float>> > fFebexThreshold;
	std::vector< std::vector<std::vector<long>> > fFebexTime;

	//ClassDef(Calibration, 1)
   
};

#endif
