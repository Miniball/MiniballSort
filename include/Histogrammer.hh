#ifndef __HISTOGRAMMER_HH
#define __HISTOGRAMMER_HH

#include <fstream>
#include <iostream>
#include <iomanip>
#include <string>
#include <vector>

#include <TFile.h>
#include <TTree.h>
#include <TMath.h>
#include <TChain.h>
#include <TProfile.h>
#include <TH1.h>
#include <TH2.h>
#include <TCutG.h>

// Reaction header
#ifndef __REACTION_HH
# include "Reaction.hh"
#endif

// Miniball Events tree
#ifndef __MINIBALLEVTS_HH
# include "MiniballEvts.hh"
#endif

// Settings file
#ifndef __SETTINGS_HH
# include "Settings.hh"
#endif

class Histogrammer {
	
public:

	Histogrammer( Reaction *myreact, Settings *myset );
	~Histogrammer();
	
	void MakeHists();
	unsigned long FillHists( unsigned long start_fill = 0 );
	void Terminate();
	
	void SetInputFile( std::vector<std::string> input_file_names );
	void SetInputFile( std::string input_file_name );
	void SetInputTree( TTree* user_tree );

	inline void SetOutput( std::string output_file_name ){
		output_file = new TFile( output_file_name.data(), "recreate" );
		MakeHists();
	};
	inline void CloseOutput( ){
		output_file->Close();
	};

	inline TFile* GetFile(){ return output_file; };
	
	// Coincidence conditions (to be put in settings file eventually)
	inline bool	PromptCoincidence( GammaRayEvt *g, ParticleEvt *p ){
		if( (double)g->GetTime() - (double)p->GetTime() > -350 &&
			(double)g->GetTime() - (double)p->GetTime() < 250 ) return true;
		else return false;
	};
	inline bool	RandomCoincidence( GammaRayEvt *g, ParticleEvt *p ){
		if( (double)g->GetTime() - (double)p->GetTime() > 600 &&
			(double)g->GetTime() - (double)p->GetTime() < 1200 ) return true;
		else return false;
	};
	inline bool	PromptCoincidence( GammaRayAddbackEvt *g, ParticleEvt *p ){
		if( (double)g->GetTime() - (double)p->GetTime() > -350 &&
			(double)g->GetTime() - (double)p->GetTime() < 250 ) return true;
		else return false;
	};
	inline bool	RandomCoincidence( GammaRayAddbackEvt *g, ParticleEvt *p ){
		if( (double)g->GetTime() - (double)p->GetTime() > 600 &&
			(double)g->GetTime() - (double)p->GetTime() < 1200 ) return true;
		else return false;
	};
	inline bool	OnBeam( GammaRayEvt *g ){
		if( (double)g->GetTime() - (double)read_evts->GetEBIS() >= 0 &&
			(double)g->GetTime() - (double)read_evts->GetEBIS() < react->GetEBISOnTime() ) return true;
		else return false;
	};
	inline bool	OnBeam( GammaRayAddbackEvt *g ){
		if( (double)g->GetTime() - (double)read_evts->GetEBIS() >= 0 &&
			(double)g->GetTime() - (double)read_evts->GetEBIS() < react->GetEBISOnTime() ) return true;
		else return false;
	};
	inline bool	OnBeam( ParticleEvt *p ){
		if( (double)p->GetTime() - (double)read_evts->GetEBIS() >= 0 &&
			(double)p->GetTime() - (double)read_evts->GetEBIS() < react->GetEBISOnTime() ) return true;
		else return false;
	};
	inline bool	OffBeam( GammaRayEvt *g ){
		if( (double)g->GetTime() - (double)read_evts->GetEBIS() >= react->GetEBISOnTime() &&
			(double)g->GetTime() - (double)read_evts->GetEBIS() < react->GetEBISOffTime() ) return true;
		else return false;
	};
	inline bool	OffBeam( GammaRayAddbackEvt *g ){
		if( (double)g->GetTime() - (double)read_evts->GetEBIS() >= react->GetEBISOnTime() &&
			(double)g->GetTime() - (double)read_evts->GetEBIS() < react->GetEBISOffTime() ) return true;
		else return false;
	};
	inline bool	OffBeam( ParticleEvt *p ){
		if( (double)p->GetTime() - (double)read_evts->GetEBIS() >= react->GetEBISOnTime() &&
			(double)p->GetTime() - (double)read_evts->GetEBIS() < react->GetEBISOffTime() ) return true;
		else return false;
	};

	// Particle energy vs angle cuts
	inline bool BeamCut( ParticleEvt *p ){
		if( react->GetBeamCut()->IsInside( react->GetParticleTheta(p), p->GetEnergy() ) )
			return true;
		else return false;
	}
	inline bool TargetCut( ParticleEvt *p ){
		if( react->GetTargetCut()->IsInside( react->GetParticleTheta(p), p->GetEnergy() ) )
			return true;
		else return false;
	}

private:
	
	// Reaction
	Reaction *react;
	
	// Settings file
	Settings *set;
	
	/// Input tree
	TChain *input_tree;
	MiniballEvts *read_evts;
	GammaRayEvt *gamma_evt;
	GammaRayAddbackEvt *gamma_ab_evt;
	ParticleEvt *particle_evt;
	
	/// Output file
	TFile *output_file;
	
	
	// Counters
	unsigned long n_entries;
	
	//------------//
	// Histograms //
	//------------//
	
	// Histogram limits
	const unsigned int GBIN = 8000;		// number of bins in gamma spectra
	const float GMIN = -0.25;			// lower limit of energy in gamma spectra
	const float GMAX = 3999.75;			// upper limit of energy in gamma spectra
	const unsigned int PBIN = 300;		// number of bins in particle spectra
	const float PMIN = 0.0;				// lower limit of energy in particle spectra
	const float PMAX = 1.2;				// upper limit of energy in particle spectra

	// Timing
	TH1F *ebis_td_particle, *ebis_td_gamma;
	TH1F *gamma_particle_td;
	
	// Gamma-rays
	TH1F *gE_singles, *gE_singles_ebis, *gE_singles_ebis_on, *gE_singles_ebis_off;

	// Particles
	TH2F *pE_theta, *pE_theta_coinc, *pE_theta_beam, *pE_theta_target;

	
};

#endif
