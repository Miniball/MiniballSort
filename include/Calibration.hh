#ifndef __CALIBRATION_HH
#define __CALIBRATION_HH

#include <iostream>
#include <iomanip>
#include <memory>
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


class FebexMWD : public TObject {
	
public:
	
	// Constructor/destructor
	inline FebexMWD() {};
	virtual inline ~FebexMWD() {};

	// Main algorithm
	void DoMWD();
	
	// Set functions
	inline void SetTrace( std::vector<unsigned short> t ){ trace = t; };
	inline void SetRiseTime( float t ){ rise_time = t; };
	inline void SetDecayTime( float t ){ decay_time = t; };
	inline void SetFlatTop( float t ){ flat_top = t; };
	inline void SetDiffWidth( float t ){ diff_width = t; };
	inline void SetWindow( unsigned int t ){ window = t; };
	inline void SetThreshold( unsigned int t ){ threshold = t; };

	// Get functions
	inline unsigned int NumberOfTriggers(){ return energy_list.size(); };
	inline float GetEnergy( unsigned int i ){
		if( i < energy_list.size() ) return energy_list.at(i);
		else return -99.9;
	};
	inline std::vector<float> GetEnergies(){ return energy_list; };
	inline std::vector<float> GetStage1(){ return stage1; };
	inline std::vector<float> GetStage2(){ return stage2; };
	inline std::vector<float> GetStage3(){ return stage3; };
	inline std::vector<unsigned int> GetDiff1(){ return diff1; };
	inline std::vector<unsigned int> GetDiff2(){ return diff2; };

private:
	
	// List of energies to return
	std::vector<float> energy_list;

	// Vector to hold the trace
	std::vector<unsigned short> trace;
	
	// Initialise some vectors for holding the differentials etc.
	std::vector<float> stage1, stage2, stage3;
	std::vector<unsigned int> stage1_int, stage2_int, stage3_int;
	std::vector<unsigned int> diff1, diff2;

	// Values of the rise time, decay time, flat top and differential width
	float rise_time, decay_time, flat_top, diff_width;
	
	// Values for the averaging window and threshold
	unsigned int window, threshold;
	
	ClassDef( FebexMWD, 1 );
	
};


/// A class to read in the calibration file in ROOT's TConfig format.
/// Each ASIC channel can have offset, gain and quadratic terms.
/// Each channel also has a threshold (not implemented)
/// and there is a time offset parameter for each ASIC module, too.

class Calibration : public TObject {

public:

	Calibration();
	Calibration( std::string filename, std::shared_ptr<Settings> myset );
	~Calibration() {};
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
	FebexMWD DoMWD( unsigned int sfp, unsigned int board, unsigned int ch, std::vector<unsigned short> trace );

private:

	std::string fInputFile;
	
	std::shared_ptr<Settings> set;

	std::vector< std::vector<std::vector<long>> > fFebexTime;
	std::vector< std::vector<std::vector<float>> > fFebexOffset;
	std::vector< std::vector<std::vector<float>> > fFebexGain;
	std::vector< std::vector<std::vector<float>> > fFebexGainQuadr;
	std::vector< std::vector<std::vector<float>> > fFebexThreshold;
	std::vector< std::vector<std::vector<float>> > fFebexMWD_Decay;
	std::vector< std::vector<std::vector<float>> > fFebexMWD_Rise;
	std::vector< std::vector<std::vector<float>> > fFebexMWD_Top;
	std::vector< std::vector<std::vector<unsigned int>> > fFebexMWD_Window;
	std::vector< std::vector<std::vector<unsigned int>> > fFebexMWD_Diff;
	std::vector< std::vector<std::vector<unsigned int>> > fFebexMWD_Threshold;

	float default_MWD_Decay;
	float default_MWD_Rise;
	float default_MWD_Top;
	unsigned int default_MWD_Window;
	unsigned int default_MWD_Diff;
	unsigned int default_MWD_Threshold;

	
	ClassDef( Calibration, 10 )
   
};

#endif
