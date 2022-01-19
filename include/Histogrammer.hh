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
	void FillParticleGammaHists( GammaRayEvt *g );
	void FillParticleGammaHists( GammaRayAddbackEvt *g );
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
	
	// Coincidence conditions (to be put in settings file eventually?)
	inline bool	PromptCoincidence( GammaRayEvt *g, ParticleEvt *p ){
		return PromptCoincidence( g, p->GetTime() );
	};
	inline bool	PromptCoincidence( GammaRayEvt *g, unsigned long long ptime ){
		if( (double)g->GetTime() - (double)ptime > react->GetParticleGammaPromptTime(0) &&
			(double)g->GetTime() - (double)ptime < react->GetParticleGammaPromptTime(1) )
			return true;
		else return false;
	};
	inline bool	RandomCoincidence( GammaRayEvt *g, ParticleEvt *p ){
		return RandomCoincidence( g, p->GetTime() );
	};
	inline bool	RandomCoincidence( GammaRayEvt *g, unsigned long long ptime ){
		if( (double)g->GetTime() - (double)ptime > react->GetParticleGammaRandomTime(0) &&
			(double)g->GetTime() - (double)ptime < react->GetParticleGammaRandomTime(1) )
			return true;
		else return false;
	};
	inline bool	PromptCoincidence( GammaRayAddbackEvt *g, ParticleEvt *p ){
		return PromptCoincidence( g, p->GetTime() );
	};
	inline bool	PromptCoincidence( GammaRayAddbackEvt *g, unsigned long long ptime ){
		if( (double)g->GetTime() - (double)ptime > react->GetParticleGammaPromptTime(0) &&
			(double)g->GetTime() - (double)ptime < react->GetParticleGammaPromptTime(1) )
			return true;
		else return false;
	};
	inline bool	RandomCoincidence( GammaRayAddbackEvt *g, ParticleEvt *p ){
		return RandomCoincidence( g, p->GetTime() );
	};
	inline bool	RandomCoincidence( GammaRayAddbackEvt *g, unsigned long long ptime ){
		if( (double)g->GetTime() - (double)ptime > react->GetParticleGammaRandomTime(0) &&
			(double)g->GetTime() - (double)ptime < react->GetParticleGammaRandomTime(1) )
			return true;
		else return false;
	};
	inline bool	PromptCoincidence( GammaRayEvt *g1, GammaRayEvt *g2 ){
		if( (double)g1->GetTime() - (double)g2->GetTime() > react->GetGammaGammaPromptTime(0) &&
			(double)g1->GetTime() - (double)g2->GetTime() < react->GetGammaGammaPromptTime(1) )
			return true;
		else return false;
	};
	inline bool	RandomCoincidence( GammaRayEvt *g1, GammaRayEvt *g2 ){
		if( (double)g1->GetTime() - (double)g2->GetTime() > react->GetGammaGammaRandomTime(0) &&
			(double)g1->GetTime() - (double)g2->GetTime() < react->GetGammaGammaRandomTime(1) )
			return true;
		else return false;
	};
	inline bool	PromptCoincidence( GammaRayAddbackEvt *g1, GammaRayAddbackEvt *g2 ){
		if( (double)g1->GetTime() - (double)g2->GetTime() > react->GetGammaGammaPromptTime(0) &&
			(double)g1->GetTime() - (double)g2->GetTime() < react->GetGammaGammaPromptTime(1) )
			return true;
		else return false;
	};
	inline bool	RandomCoincidence( GammaRayAddbackEvt *g1, GammaRayAddbackEvt *g2 ){
		if( (double)g1->GetTime() - (double)g2->GetTime() > react->GetGammaGammaRandomTime(0) &&
			(double)g1->GetTime() - (double)g2->GetTime() < react->GetGammaGammaRandomTime(1) )
			return true;
		else return false;
	};
	inline bool	PromptCoincidence( ParticleEvt *p1, ParticleEvt *p2 ){
		if( (double)p1->GetTime() - (double)p2->GetTime() > react->GetParticleParticlePromptTime(0) &&
			(double)p1->GetTime() - (double)p2->GetTime() < react->GetParticleParticlePromptTime(1) )
			return true;
		else return false;
	};
	inline bool	RandomCoincidence( ParticleEvt *p1, ParticleEvt *p2 ){
		if( (double)p1->GetTime() - (double)p2->GetTime() > react->GetParticleParticleRandomTime(0) &&
			(double)p1->GetTime() - (double)p2->GetTime() < react->GetParticleParticleRandomTime(1) )
			return true;
		else return false;
	};
	inline bool	PromptCoincidence( BeamDumpEvt *g1, BeamDumpEvt *g2 ){
		if( (double)g1->GetTime() - (double)g2->GetTime() > react->GetGammaGammaPromptTime(0) &&
			(double)g1->GetTime() - (double)g2->GetTime() < react->GetGammaGammaPromptTime(1) )
			return true;
		else return false;
	};
	inline bool	RandomCoincidence( BeamDumpEvt *g1, BeamDumpEvt *g2 ){
		if( (double)g1->GetTime() - (double)g2->GetTime() > react->GetGammaGammaRandomTime(0) &&
			(double)g1->GetTime() - (double)g2->GetTime() < react->GetGammaGammaRandomTime(1) )
			return true;
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
	inline bool EjectileCut( ParticleEvt *p ){
		if( react->GetEjectileCut()->IsInside( react->GetParticleTheta(p), p->GetEnergy() ) )
			return true;
		else return false;
	}
	inline bool RecoilCut( ParticleEvt *p ){
		if( react->GetRecoilCut()->IsInside( react->GetParticleTheta(p), p->GetEnergy() ) )
			return true;
		else return false;
	}
	inline bool TwoParticleCut( ParticleEvt *p1, ParticleEvt *p2 ){
		if( EjectileCut(p1) && RecoilCut(p2) && PromptCoincidence( p1, p2 ) &&
		    TMath::Abs( react->GetParticlePhi(p1) - react->GetParticlePhi(p2) ) < 1.1*TMath::Pi() &&
		    TMath::Abs( react->GetParticlePhi(p1) - react->GetParticlePhi(p2) ) > 0.9*TMath::Pi() )
			return true;
		else return false;
	}

private:
	
	// Reaction
	Reaction *react;
	
	// Settings file
	Settings *set;
	
	// Input tree
	TChain *input_tree;
	MiniballEvts *read_evts;
	GammaRayEvt *gamma_evt, *gamma_evt2;
	GammaRayAddbackEvt *gamma_ab_evt, *gamma_ab_evt2;
	ParticleEvt *particle_evt, *particle_evt2;
	BeamDumpEvt *bd_evt, *bd_evt2;

	// Output file
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

	// EBIS
	TH1F *ebis_td_particle, *ebis_td_gamma;
	
	// Gamma-rays with and without addback
	TH1F *gE_singles, *gE_singles_ebis, *gE_singles_ebis_on, *gE_singles_ebis_off;
	TH1F *aE_singles, *aE_singles_ebis, *aE_singles_ebis_on, *aE_singles_ebis_off;

	// Gamma-ray coincidence matrices with and without addback
	TH1F *gamma_gamma_td;
	TH2F *gE_gE, *gE_gE_ebis_on;
	TH2F *aE_aE, *aE_aE_ebis_on;

	// Particles
	TH2F *pE_theta, *pE_theta_coinc, *pE_theta_beam, *pE_theta_target;
	TH1F *particle_particle_td;

	// Particle-gamma coincidences with and without addback
	TH1F *gamma_particle_td;
	TH1F *gE_prompt, *gE_prompt_1p, *gE_prompt_2p;
	TH1F *gE_random, *gE_random_1p, *gE_random_2p;
	TH1F *aE_prompt, *aE_prompt_1p, *aE_prompt_2p;
	TH1F *aE_random, *aE_random_1p, *aE_random_2p;

	// Doppler-corrected gamma-rays without addback
	TH1F *gE_ejectile_dc_none, *gE_ejectile_dc_ejectile, *gE_ejectile_dc_recoil;
	TH1F *gE_recoil_dc_none,   *gE_recoil_dc_ejectile,   *gE_recoil_dc_recoil;
	TH1F *gE_2p_dc_none,       *gE_2p_dc_ejectile,       *gE_2p_dc_recoil;
	TH2F *gE_vs_theta_ejectile_dc_none, *gE_vs_theta_ejectile_dc_ejectile, *gE_vs_theta_ejectile_dc_recoil;
	TH2F *gE_vs_theta_recoil_dc_none,   *gE_vs_theta_recoil_dc_ejectile,   *gE_vs_theta_recoil_dc_recoil;
	TH2F *gE_vs_theta_2p_dc_none,       *gE_vs_theta_2p_dc_ejectile,       *gE_vs_theta_2p_dc_recoil;

	// Doppler-corrected gamma-rays with addback
	TH1F *aE_ejectile_dc_none, *aE_ejectile_dc_ejectile, *aE_ejectile_dc_recoil;
	TH1F *aE_recoil_dc_none,   *aE_recoil_dc_ejectile,   *aE_recoil_dc_recoil;
	TH1F *aE_2p_dc_none,       *aE_2p_dc_ejectile,       *aE_2p_dc_recoil;
	TH2F *aE_vs_theta_ejectile_dc_none, *aE_vs_theta_ejectile_dc_ejectile, *aE_vs_theta_ejectile_dc_recoil;
	TH2F *aE_vs_theta_recoil_dc_none,   *aE_vs_theta_recoil_dc_ejectile,   *aE_vs_theta_recoil_dc_recoil;
	TH2F *aE_vs_theta_2p_dc_none,       *aE_vs_theta_2p_dc_ejectile,       *aE_vs_theta_2p_dc_recoil;

	// Beam-dump histograms
	TH1F *bdE_singles;
	std::vector<TH1F*> bdE_singles_det;
	TH1F *bd_bd_td;
	TH2F *bdE_bdE;

};

#endif
