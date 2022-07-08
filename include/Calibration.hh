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
#include "TGraph.h"

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
	inline void SetRiseTime( unsigned int t ){ rise_time = t; };
	inline void SetDecayTime( float t ){ decay_time = t; };
	inline void SetFlatTop( unsigned int t ){ flat_top = t; };
	inline void SetWindow( unsigned int t ){ window = t; };
	inline void SetDelayTime( unsigned int t ){ delay_time = t; };
	inline void SetThreshold( unsigned int t ){ threshold = t; };
	inline void SetFraction( float f ){ fraction = f; };

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
	inline std::vector<float> GetCfd(){ return cfd; };
	
	// Graphs
	inline TGraph* GetTraceGraph() {
		return GetGraph( trace );
	};
	inline TGraph* GetStage1Graph() {
		return GetGraph( stage1 );
	};
	inline TGraph* GetStage2Graph() {
		return GetGraph( stage2 );
	};
	inline TGraph* GetStage3Graph() {
		return GetGraph( stage3 );
	};
	inline TGraph* GetCfdGraph() {
		return GetGraph( cfd );
	};

private:
	
	// List of energies to return
	std::vector<float> energy_list;

	// Vector to hold the trace
	std::vector<unsigned short> trace;
	
	// Initialise some vectors for holding the MWD and CFD etc.
	std::vector<float> stage1, stage2, stage3;
	std::vector<float> shaper, cfd;

	// Values of MWD
	unsigned int rise_time, flat_top, window;
	float decay_time;

	// Values for CFD
	unsigned int delay_time, threshold;
	float fraction;
	
	// Graphs
	inline TGraph* GetGraph( std::vector<float> &t ) {
		std::vector<float> x;
		for( unsigned short i = 0; i < t.size(); ++i )
			x.push_back( i );
		std::unique_ptr<TGraph> g = std::make_unique<TGraph>(
                            t.size(), x.data(), t.data() );
 		return (TGraph*)g.get()->Clone();
	};
	inline TGraph* GetGraph( std::vector<unsigned short> &t ) {
		std::vector<float> y;
		for( unsigned short i = 0; i < t.size(); ++i )
			y.push_back( static_cast<float>(t[i]) );
 		return GetGraph(y);
	};

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
	std::vector< std::vector<std::vector<float>> > fFebexCFD_Fraction;
	std::vector< std::vector<std::vector<unsigned int>> > fFebexMWD_Rise;
	std::vector< std::vector<std::vector<unsigned int>> > fFebexMWD_Top;
	std::vector< std::vector<std::vector<unsigned int>> > fFebexMWD_Window;
	std::vector< std::vector<std::vector<unsigned int>> > fFebexCFD_Delay;
	std::vector< std::vector<std::vector<unsigned int>> > fFebexCFD_Threshold;

	float default_MWD_Decay;
	float default_CFD_Fraction;
	unsigned int default_MWD_Rise;
	unsigned int default_MWD_Top;
	unsigned int default_MWD_Window;
	unsigned int default_CFD_Delay;
	unsigned int default_CFD_Threshold;

	
	ClassDef( Calibration, 10 )
   
};

#endif
