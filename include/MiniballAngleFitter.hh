#ifndef __MINIBALLANGLEFITTER_HH
#define __MINIBALLANGLEFITTER_HH

// C++ includes
#include <fstream>
#include <iostream>
#include <string>
#include <sstream>
#include <vector>

// ROOT includes
#include "TFile.h"
#include "TMath.h"
#include "TCanvas.h"
#include "TGraph.h"
#include "TMultiGraph.h"
#include "TGraphErrors.h"
#include "TF1.h"
#include "TH1.h"
#include "TH2.h"
#include "TLegend.h"
#include "Math/Functor.h"
#include "Math/Factory.h"
#include "Math/Minimizer.h"
#include "Math/MinimizerOptions.h"

// Settings header
#ifndef __SETTINGS_HH
# include "Settings.hh"
#endif

// Reaction header
#ifndef __REACTION_HH
# include "Reaction.hh"
#endif


class MiniballAngleFunction {

public:
	
	// ctors and dtors
	MiniballAngleFunction(){};
	~MiniballAngleFunction(){};
	MiniballAngleFunction( std::shared_ptr<MiniballSettings> _myset, std::shared_ptr<MiniballReaction> _myreact );

	// Initial setup, called from constructors
	void Initialise();
		
	// Fit the segment spectra to get the energies
	bool FitPeak( TH1D *h, double &en, double &er );
	void FitSegmentEnergies( std::shared_ptr<TFile> infile );

	// Load energies
	void LoadExpEnergies( std::string energy_file );
	
	// Check if segment is present
	inline bool IsPresent( unsigned int clu ){
		return cluster[clu];
	};
	inline bool IsPresent( unsigned int clu, unsigned int cry, unsigned int seg ){
		return present[clu][cry][seg];
	};

	// Setter and getter for reference energy
	inline void SetReferenceEnergy( double e ){ eref = e; };
	inline double GetReferenceEnergy(){ return eref; };
	
	// Getters for the experimental energies
	inline double GetExpEnergy( unsigned int clu, unsigned int cry, unsigned int seg ){
		return energy[clu][cry][seg];
	};
	inline double GetExpError( unsigned int clu, unsigned int cry, unsigned int seg ){
		return err[clu][cry][seg];
	};

	// operator for the fit function
	double operator() ( const double *p );

private:
	
	// Vectors for data, etc
	std::vector<std::vector<std::vector<bool>>> present;
	std::vector<std::vector<std::vector<double>>> energy;
	std::vector<std::vector<std::vector<double>>> err;
	std::vector<bool> cluster;
	
	// Settings files, etc
	std::shared_ptr<MiniballSettings> myset;
	std::shared_ptr<MiniballReaction> myreact;
	
	double user_z = 0;
	double eref = 440.2;

};

class MiniballAngleFitter {

public:
	
	// ctors and dtors
	MiniballAngleFitter();
	~MiniballAngleFitter(){};
	MiniballAngleFitter( std::string settings_file, std::string reaction_file );
	MiniballAngleFitter( std::shared_ptr<MiniballSettings> _myset, std::shared_ptr<MiniballReaction> _myreact );
	
	// Initialise at the start, called from ctors
	void Initialise();
	
	// Call the actual fitting
	void DoFit();
	
	// Write the results to a reaction file
	void SaveReactionFile( std::string fname );
	
	// Set the input ROOT file
	bool SetInputROOTFile( std::string fname );
	
	// Set the input data file with energies
	bool SetInputEnergiesFile( std::string fname );
	
	// Set the output directory
	inline void SetOutputDirectory( std::string dir ){
		datadir = dir;
	};

private:

	// Settings files, etc
	std::shared_ptr<MiniballSettings> myset;
	std::shared_ptr<MiniballReaction> myreact;
	
	// Input ROOT file
	std::shared_ptr<TFile> input_root_file;
	
	// Input data file
	std::string input_data_filename;
	
	// This is the main fit function
	MiniballAngleFunction ff;
	
	// Output directory
	std::string datadir;
	
	// Flag if we need to fit the peaks ourselves, or they are given
	bool flag_fit_peaks = false;
	
	// Paramaters and limits
	unsigned npars = 1;
	std::vector<double> pars;
	std::vector<std::string> names;
	std::vector<double> LL;
	std::vector<double> UL;

};


#endif

