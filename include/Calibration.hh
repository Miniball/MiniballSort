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
	inline void SetRiseTime( unsigned int t ){ rise_time = t; }; // M
	inline void SetDecayTime( float t ){ decay_time = t; }; // torr
	inline void SetFlatTop( unsigned int t ){ flat_top = t; }; // not in James' code
	inline void SetBaseline( unsigned int t ){ baseline_length = t; }; // BLFL (baseline filter length?)
	inline void SetWindow( unsigned int t ){ window = t; }; // L
	inline void SetDelayTime( unsigned int t ){ delay_time = t; };
	inline void SetThreshold( unsigned int t ){ threshold = t; };
	inline void SetFraction( float f ){ fraction = f; };

	// Get functions
	inline unsigned int NumberOfTriggers(){ return energy_list.size(); };
	inline float GetEnergy( unsigned int i ){
		if( i < energy_list.size() ) return energy_list.at(i);
		else return -99.9;
	};
	inline float GetCfdTime( unsigned int i ){
		if( i < cfd_list.size() ) return cfd_list.at(i);
		else return 0;
	};
	inline std::vector<float> GetEnergies(){ return energy_list; };
	inline std::vector<float> GetCfdTimes(){ return cfd_list; };
	inline std::vector<float> GetStage1(){ return stage1; };
	inline std::vector<float> GetStage2(){ return stage2; };
	inline std::vector<float> GetStage3(){ return stage3; };
	inline std::vector<float> GetStage4(){ return stage4; };
	inline std::vector<float> GetCfd(){ return cfd; };
	
	// Is it clipped?
	inline bool IsClipped(){ return clipped; };
	
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
	inline TGraph* GetStage4Graph() {
		return GetGraph( stage4 );
	};
	inline TGraph* GetCfdGraph() {
		return GetGraph( cfd );
	};

private:
	
	// List of energies and time to return
	std::vector<float> energy_list;
	std::vector<float> cfd_list;

	// Vector to hold the trace
	std::vector<unsigned short> trace;
	
	// Initialise some vectors for holding the MWD and CFD etc.
	std::vector<float> stage1, stage2, stage3, stage4;
	std::vector<float> shaper, cfd;

	// Values of MWD
	unsigned int rise_time, flat_top, window, baseline_length, decay_time;

	// Values for CFD
	unsigned int delay_time;
	int threshold;
	float fraction;
	
	// Are any of the samples clipped?
	bool clipped;
	
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

	ClassDef( FebexMWD, 3 );
	
};


/// A class to read in the calibration file in ROOT's TConfig format.
/// Each ASIC channel can have offset, gain and quadratic terms.
/// Each channel also has a threshold (not implemented)
/// and there is a time offset parameter for each ASIC module, too.

class MiniballCalibration : public TObject {

public:

	MiniballCalibration( std::string filename, std::shared_ptr<MiniballSettings> myset );
	~MiniballCalibration() {};
	void ReadCalibration();
	void PrintCalibration();
	void SetFile( std::string filename ){
		fInputFile = filename;
	}
	const std::string InputFile(){
		return fInputFile;
	}
	float FebexEnergy( unsigned char sfp, unsigned char board, unsigned char ch, unsigned int raw );
	unsigned int FebexThreshold( unsigned char sfp, unsigned char board, unsigned char ch );
	std::string FebexType( unsigned char sfp, unsigned char board, unsigned char ch );
	long FebexTime( unsigned char sfp, unsigned char board, unsigned char ch );
	FebexMWD DoMWD( unsigned char sfp, unsigned char board, unsigned char ch, std::vector<unsigned short> trace );
	
	// Set functions
	void SetMWDDecay( unsigned char sfp, unsigned char board, unsigned char ch, unsigned int decay );
	void SetMWDRise( unsigned char sfp, unsigned char board, unsigned char ch, unsigned int rise );
	void SetMWDTop( unsigned char sfp, unsigned char board, unsigned char ch, unsigned int top );
	void SetMWDBaseline( unsigned char sfp, unsigned char board, unsigned char ch, unsigned int baseline_length );
	void SetMWDWindow( unsigned char sfp, unsigned char board, unsigned char ch, unsigned int window );
	void SetCFDFraction( unsigned char sfp, unsigned char board, unsigned char ch, float fraction );
	void SetCFDDelay( unsigned char sfp, unsigned char board, unsigned char ch, unsigned int delay );
	void SetCFDThreshold( unsigned char sfp, unsigned char board, unsigned char ch, int threshold );
	
	// Get functions
	unsigned int GetMWDDecay( unsigned char sfp, unsigned char board, unsigned char ch );
	unsigned int GetMWDRise( unsigned char sfp, unsigned char board, unsigned char ch );
	unsigned int GetMWDTop( unsigned char sfp, unsigned char board, unsigned char ch );
	unsigned int GetMWDBaseline( unsigned char sfp, unsigned char board, unsigned char ch );
	unsigned int GetMWDWindow( unsigned char sfp, unsigned char board, unsigned char ch );
	float GetCFDFraction( unsigned char sfp, unsigned char board, unsigned char ch );
	unsigned int GetCFDDelay( unsigned char sfp, unsigned char board, unsigned char ch );
	int GetCFDThreshold( unsigned char sfp, unsigned char board, unsigned char ch );

	
private:

	std::string fInputFile;
	std::unique_ptr<TRandom> fRand;

	
	std::shared_ptr<MiniballSettings> set;

	std::vector< std::vector<std::vector<std::string>> > fFebexType; // Qint or Qshort
	std::vector< std::vector<std::vector<long>> > fFebexTime;
	std::vector< std::vector<std::vector<float>> > fFebexOffset;
	std::vector< std::vector<std::vector<float>> > fFebexGain;
	std::vector< std::vector<std::vector<float>> > fFebexGainQuadr;
	std::vector< std::vector<std::vector<unsigned int>> > fFebexThreshold;

	std::vector< std::vector<std::vector<unsigned int>> > fFebexMWD_Decay;
	std::vector< std::vector<std::vector<unsigned int>> > fFebexMWD_Rise;
	std::vector< std::vector<std::vector<unsigned int>> > fFebexMWD_Top;
	std::vector< std::vector<std::vector<unsigned int>> > fFebexMWD_Baseline;
	std::vector< std::vector<std::vector<unsigned int>> > fFebexMWD_Window;
	std::vector< std::vector<std::vector<unsigned int>> > fFebexCFD_Delay;
	std::vector< std::vector<std::vector<int>> > fFebexCFD_Threshold; // polarity of CFD selected by a negative threshold
	std::vector< std::vector<std::vector<float>> > fFebexCFD_Fraction;

	unsigned int default_MWD_Decay;
	unsigned int default_MWD_Rise;
	unsigned int default_MWD_Top;
	unsigned int default_MWD_Baseline;
	unsigned int default_MWD_Window;
	float default_CFD_Fraction;
	unsigned int default_CFD_Delay;
	int default_CFD_Threshold;

	
	ClassDef( MiniballCalibration, 3 )
   
};

#endif
