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
#include "TRandom3.h"
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
	inline void SetRiseTime( unsigned int t ){ rise_time = t; }; // L
	inline void SetDecayTime( float t ){ decay_time = t; }; // torr
	inline void SetFlatTop( unsigned int t ){ flat_top = t; }; // not in James' code
	inline void SetBaseline( unsigned int t ){ baseline_length = t; }; // BLFL (baseline filter length?)
	inline void SetWindow( unsigned int t ){ window = t; }; // M
	inline void SetDelayTime( unsigned int t ){ cfd_delay = t; };
	inline void SetHoldOff( unsigned int t ){ cfd_hold = t; };
	inline void SetShapingTime( unsigned int t ){ cfd_shaping_time = t; };
	inline void SetIntegrationTime( unsigned int t ){ cfd_integration_time = t; };
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
	std::vector<float> differential, shaper, cfd;

	// Values of MWD
	unsigned int rise_time, flat_top, window, baseline_length, decay_time;

	// Values for CFD
	unsigned int cfd_delay, cfd_hold, cfd_shaping_time, cfd_integration_time;
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
	
	// ADC calibrations
	float			AdcEnergy( unsigned char mod, unsigned char ch, unsigned int raw );
	double			AdcOffset( unsigned char mod, unsigned char ch );
	double			AdcGain( unsigned char mod, unsigned char ch );
	double			AdcGainQuadr( unsigned char mod, unsigned char ch );
	unsigned int	AdcThreshold( unsigned char mod, unsigned char ch );
	long			AdcTime( unsigned char mod, unsigned char ch );
	
	// DGF calibrations
	float			DgfEnergy( unsigned char mod, unsigned char ch, unsigned int raw );
	double			DgfOffset( unsigned char mod, unsigned char ch );
	double			DgfGain( unsigned char mod, unsigned char ch );
	double			DgfGainQuadr( unsigned char mod, unsigned char ch );
	unsigned int	DgfThreshold( unsigned char mod, unsigned char ch );
	long			DgfTime( unsigned char mod, unsigned char ch );
	
	// Febex calibrations
	float			FebexEnergy( unsigned char sfp, unsigned char board, unsigned char ch, unsigned int raw );
	double			FebexOffset( unsigned char sfp, unsigned char board, unsigned char ch );
	double			FebexGain( unsigned char sfp, unsigned char board, unsigned char ch );
	double			FebexGainQuadr( unsigned char sfp, unsigned char board, unsigned char ch );
	unsigned int	FebexThreshold( unsigned char sfp, unsigned char board, unsigned char ch );
	std::string		FebexType( unsigned char sfp, unsigned char board, unsigned char ch );
	long			FebexTime( unsigned char sfp, unsigned char board, unsigned char ch );
	FebexMWD		DoMWD( unsigned char sfp, unsigned char board, unsigned char ch, std::vector<unsigned short> trace );
	
	// Set functions for MWD
	void SetMWDDecay( unsigned char sfp, unsigned char board, unsigned char ch, unsigned int decay );
	void SetMWDRise( unsigned char sfp, unsigned char board, unsigned char ch, unsigned int rise );
	void SetMWDFlatTop( unsigned char sfp, unsigned char board, unsigned char ch, unsigned int top );
	void SetMWDBaseline( unsigned char sfp, unsigned char board, unsigned char ch, unsigned int baseline_length );
	void SetMWDWindow( unsigned char sfp, unsigned char board, unsigned char ch, unsigned int window );
	void SetCFDFraction( unsigned char sfp, unsigned char board, unsigned char ch, float fraction );
	void SetCFDDelay( unsigned char sfp, unsigned char board, unsigned char ch, unsigned int delay );
	void SetCFDHoldOff( unsigned char sfp, unsigned char board, unsigned char ch, unsigned int hold );
	void SetCFDShapingTime( unsigned char sfp, unsigned char board, unsigned char ch, unsigned int shaping );
	void SetCFDIntegrationTime( unsigned char sfp, unsigned char board, unsigned char ch, unsigned int integration );
	void SetCFDThreshold( unsigned char sfp, unsigned char board, unsigned char ch, int threshold );
	
	// Get functions for MWD
	unsigned int GetMWDDecay( unsigned char sfp, unsigned char board, unsigned char ch );
	unsigned int GetMWDRise( unsigned char sfp, unsigned char board, unsigned char ch );
	unsigned int GetMWDFlatTop( unsigned char sfp, unsigned char board, unsigned char ch );
	unsigned int GetMWDBaseline( unsigned char sfp, unsigned char board, unsigned char ch );
	unsigned int GetMWDWindow( unsigned char sfp, unsigned char board, unsigned char ch );
	float GetCFDFraction( unsigned char sfp, unsigned char board, unsigned char ch );
	unsigned int GetCFDDelay( unsigned char sfp, unsigned char board, unsigned char ch );
	unsigned int GetCFDHoldOff( unsigned char sfp, unsigned char board, unsigned char ch );
	unsigned int GetCFDShapingTime( unsigned char sfp, unsigned char board, unsigned char ch );
	unsigned int GetCFDIntegrationTime( unsigned char sfp, unsigned char board, unsigned char ch );
	int GetCFDThreshold( unsigned char sfp, unsigned char board, unsigned char ch );

	// Global default to Qint instead of Qshort
	inline void SetDefaultQint(){ default_qint = true; };

	
private:

	std::string fInputFile;
	std::unique_ptr<TRandom3> fRand;
	
	bool default_qint;
	
	std::shared_ptr<MiniballSettings> set;
	
	// Mesytec or CAEN ADC calibration
	std::vector<std::vector<long>> fAdcTime;
	std::vector<std::vector<double>> fAdcOffset;
	std::vector<std::vector<double>> fAdcGain;
	std::vector<std::vector<double>> fAdcGainQuadr;
	std::vector<std::vector<unsigned int>> fAdcThreshold;

	// DGF calibration
	std::vector<std::vector<long>> fDgfTime;
	std::vector<std::vector<double>> fDgfOffset;
	std::vector<std::vector<double>> fDgfGain;
	std::vector<std::vector<double>> fDgfGainQuadr;
	std::vector<std::vector<unsigned int>> fDgfThreshold;

	// Febex
	std::vector< std::vector<std::vector<std::string>> > fFebexType; // Qint or Qshort
	std::vector< std::vector<std::vector<long>> > fFebexTime;
	std::vector< std::vector<std::vector<double>> > fFebexOffset;
	std::vector< std::vector<std::vector<double>> > fFebexGain;
	std::vector< std::vector<std::vector<double>> > fFebexGainQuadr;
	std::vector< std::vector<std::vector<unsigned int>> > fFebexThreshold;

	// MWD + CFD
	std::vector< std::vector<std::vector<unsigned int>> > fFebexMWD_Decay;
	std::vector< std::vector<std::vector<unsigned int>> > fFebexMWD_Rise;
	std::vector< std::vector<std::vector<unsigned int>> > fFebexMWD_Top;
	std::vector< std::vector<std::vector<unsigned int>> > fFebexMWD_Baseline;
	std::vector< std::vector<std::vector<unsigned int>> > fFebexMWD_Window;
	std::vector< std::vector<std::vector<unsigned int>> > fFebexCFD_Delay;
	std::vector< std::vector<std::vector<unsigned int>> > fFebexCFD_HoldOff;
	std::vector< std::vector<std::vector<unsigned int>> > fFebexCFD_Shaping;
	std::vector< std::vector<std::vector<unsigned int>> > fFebexCFD_Integration;
	std::vector< std::vector<std::vector<int>> > fFebexCFD_Threshold; // polarity of CFD selected by a negative threshold
	std::vector< std::vector<std::vector<float>> > fFebexCFD_Fraction;

	// MWD defaults
	unsigned int default_FebexMWD_Decay;
	unsigned int default_FebexMWD_Rise;
	unsigned int default_FebexMWD_Top;
	unsigned int default_FebexMWD_Baseline;
	unsigned int default_FebexMWD_Window;
	float default_FebexCFD_Fraction;
	unsigned int default_FebexCFD_Delay;
	unsigned int default_FebexCFD_HoldOff;
	unsigned int default_FebexCFD_Shaping;
	unsigned int default_FebexCFD_Integration;
	int default_FebexCFD_Threshold;

	
	ClassDef( MiniballCalibration, 4 )
   
};

#endif
