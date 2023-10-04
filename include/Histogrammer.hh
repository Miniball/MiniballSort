#ifndef __HISTOGRAMMER_HH
#define __HISTOGRAMMER_HH

#include <fstream>
#include <iostream>
#include <iomanip>
#include <string>
#include <vector>
#include <memory>

#include <TFile.h>
#include <TTree.h>
#include <TMath.h>
#include <TChain.h>
#include <TRandom.h>
#include <TProfile.h>
#include <TH1.h>
#include <TH2.h>
#include <TCutG.h>
#include <TGProgressBar.h>
#include <TSystem.h>

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

class MiniballHistogrammer {
	
public:

	MiniballHistogrammer( std::shared_ptr<MiniballReaction> myreact, std::shared_ptr<MiniballSettings> myset );
	~MiniballHistogrammer() {};
	
	void MakeHists();
	void ResetHists();
	unsigned long FillHists();
	void FillParticleGammaHists( std::shared_ptr<GammaRayEvt> g );
	void FillParticleGammaHists( std::shared_ptr<GammaRayAddbackEvt> g );
	void FillParticleElectronHists( std::shared_ptr<SpedeEvt> s );
	void FillParticleGammaGammaHists( std::shared_ptr<GammaRayEvt> g1, std::shared_ptr<GammaRayEvt> g2 );
	void FillParticleGammaGammaHists( std::shared_ptr<GammaRayAddbackEvt> g1, std::shared_ptr<GammaRayAddbackEvt> g2 );
	void FillParticleElectronGammaHists( std::shared_ptr<SpedeEvt> s, std::shared_ptr<GammaRayEvt> g );
	void FillParticleElectronGammaHists( std::shared_ptr<SpedeEvt> s, std::shared_ptr<GammaRayAddbackEvt> g );

	void SetInputFile( std::vector<std::string> input_file_names );
	void SetInputFile( std::string input_file_name );
	void SetInputTree( TTree *user_tree );

	inline void SetOutput( std::string output_file_name ){
		output_file = new TFile( output_file_name.data(), "recreate" );
		MakeHists();
	};
	inline void CloseOutput( ){
		PurgeOutput();
		output_file->Close();
		input_tree->ResetBranchAddresses();
		delete read_evts;
	};
	inline void PurgeOutput(){
		input_tree->Reset();
		output_file->Purge(2);
	}

	inline TFile* GetFile(){ return output_file; };

	inline void AddProgressBar( std::shared_ptr<TGProgressBar> myprog ){
		prog = myprog;
		_prog_ = true;
	};
	
	// Coincidence conditions (to be put in settings file eventually?)
	inline bool	PromptCoincidence( std::shared_ptr<GammaRayEvt> g, std::shared_ptr<ParticleEvt> p ){
		return PromptCoincidence( g, p->GetTime() );
	};
	inline bool	PromptCoincidence( std::shared_ptr<GammaRayEvt> g, unsigned long long ptime ){
		if( (double)g->GetTime() - (double)ptime > react->GetParticleGammaPromptTime(0) &&
			(double)g->GetTime() - (double)ptime < react->GetParticleGammaPromptTime(1) )
			return true;
		else return false;
	};
	inline bool	RandomCoincidence( std::shared_ptr<GammaRayEvt> g, std::shared_ptr<ParticleEvt> p ){
		return RandomCoincidence( g, p->GetTime() );
	};
	inline bool	RandomCoincidence( std::shared_ptr<GammaRayEvt> g, unsigned long long ptime ){
		if( (double)g->GetTime() - (double)ptime > react->GetParticleGammaRandomTime(0) &&
			(double)g->GetTime() - (double)ptime < react->GetParticleGammaRandomTime(1) )
			return true;
		else return false;
	};
	inline bool	PromptCoincidence( std::shared_ptr<GammaRayAddbackEvt> g, std::shared_ptr<ParticleEvt> p ){
		return PromptCoincidence( g, p->GetTime() );
	};
	inline bool	PromptCoincidence( std::shared_ptr<SpedeEvt> s, std::shared_ptr<ParticleEvt> p ){
		return PromptCoincidence( s, p->GetTime() );
	};
	inline bool	PromptCoincidence( std::shared_ptr<SpedeEvt> s, unsigned long long ptime ){
		if( (double)s->GetTime() - (double)ptime > react->GetParticleGammaPromptTime(0) &&
			(double)s->GetTime() - (double)ptime < react->GetParticleGammaPromptTime(1) )
			return true;
		else return false;
	};
	inline bool	RandomCoincidence( std::shared_ptr<SpedeEvt> s, std::shared_ptr<ParticleEvt> p ){
		return RandomCoincidence( s, p->GetTime() );
	};
	inline bool	RandomCoincidence( std::shared_ptr<SpedeEvt> s, unsigned long long ptime ){
		if( (double)s->GetTime() - (double)ptime > react->GetParticleGammaRandomTime(0) &&
			(double)s->GetTime() - (double)ptime < react->GetParticleGammaRandomTime(1) )
			return true;
		else return false;
	};
	inline bool	PromptCoincidence( std::shared_ptr<GammaRayEvt> g1, std::shared_ptr<GammaRayEvt> g2 ){
		if( (double)g1->GetTime() - (double)g2->GetTime() > react->GetGammaGammaPromptTime(0) &&
			(double)g1->GetTime() - (double)g2->GetTime() < react->GetGammaGammaPromptTime(1) )
			return true;
		else return false;
	};
	inline bool	RandomCoincidence( std::shared_ptr<GammaRayEvt> g1, std::shared_ptr<GammaRayEvt> g2 ){
		if( (double)g1->GetTime() - (double)g2->GetTime() > react->GetGammaGammaRandomTime(0) &&
			(double)g1->GetTime() - (double)g2->GetTime() < react->GetGammaGammaRandomTime(1) )
			return true;
		else return false;
	};
	inline bool	PromptCoincidence( std::shared_ptr<SpedeEvt> s1, std::shared_ptr<SpedeEvt> s2 ){
		if( (double)s1->GetTime() - (double)s2->GetTime() > react->GetGammaGammaPromptTime(0) &&
			(double)s1->GetTime() - (double)s2->GetTime() < react->GetGammaGammaPromptTime(1) )
			return true;
		else return false;
	};
	inline bool	RandomCoincidence( std::shared_ptr<SpedeEvt> s1, std::shared_ptr<SpedeEvt> s2 ){
		if( (double)s1->GetTime() - (double)s2->GetTime() > react->GetGammaGammaRandomTime(0) &&
			(double)s1->GetTime() - (double)s2->GetTime() < react->GetGammaGammaRandomTime(1) )
			return true;
		else return false;
	};
	inline bool	PromptCoincidence( std::shared_ptr<ParticleEvt> p1, std::shared_ptr<ParticleEvt> p2 ){
		if( (double)p1->GetTime() - (double)p2->GetTime() > react->GetParticleParticlePromptTime(0) &&
			(double)p1->GetTime() - (double)p2->GetTime() < react->GetParticleParticlePromptTime(1) )
			return true;
		else return false;
	};
	inline bool	RandomCoincidence( std::shared_ptr<ParticleEvt> p1, std::shared_ptr<ParticleEvt> p2 ){
		if( (double)p1->GetTime() - (double)p2->GetTime() > react->GetParticleParticleRandomTime(0) &&
			(double)p1->GetTime() - (double)p2->GetTime() < react->GetParticleParticleRandomTime(1) )
			return true;
		else return false;
	};
	inline bool	PromptCoincidence( std::shared_ptr<BeamDumpEvt> g1, std::shared_ptr<BeamDumpEvt> g2 ){
		if( (double)g1->GetTime() - (double)g2->GetTime() > react->GetGammaGammaPromptTime(0) &&
			(double)g1->GetTime() - (double)g2->GetTime() < react->GetGammaGammaPromptTime(1) )
			return true;
		else return false;
	};
	inline bool	RandomCoincidence( std::shared_ptr<BeamDumpEvt> g1, std::shared_ptr<BeamDumpEvt> g2 ){
		if( (double)g1->GetTime() - (double)g2->GetTime() > react->GetGammaGammaRandomTime(0) &&
			(double)g1->GetTime() - (double)g2->GetTime() < react->GetGammaGammaRandomTime(1) )
			return true;
		else return false;
	};
	inline bool	PromptCoincidence( std::shared_ptr<SpedeEvt> s, std::shared_ptr<GammaRayEvt> g ){
		if( (double)s->GetTime() - (double)g->GetTime() > react->GetGammaGammaPromptTime(0) &&
			(double)s->GetTime() - (double)g->GetTime() < react->GetGammaGammaPromptTime(1) )
			return true;
		else return false;
	};
	inline bool	PromptCoincidence( std::shared_ptr<GammaRayEvt> g, std::shared_ptr<SpedeEvt> s ){
		return PromptCoincidence( s, g );
	};
	inline bool	RandomCoincidence( std::shared_ptr<SpedeEvt> s, std::shared_ptr<GammaRayEvt> g ){
		if( (double)s->GetTime() - (double)g->GetTime() > react->GetGammaGammaRandomTime(0) &&
			(double)s->GetTime() - (double)g->GetTime() < react->GetGammaGammaRandomTime(1) )
			return true;
		else return false;
	};
	inline bool	RandomCoincidence( std::shared_ptr<GammaRayEvt> g, std::shared_ptr<SpedeEvt> s ){
		return RandomCoincidence( s, g );
	};
	inline bool	OnBeam( std::shared_ptr<GammaRayEvt> g ){
		if( (double)g->GetTime() - (double)read_evts->GetEBIS() >= 0 &&
			(double)g->GetTime() - (double)read_evts->GetEBIS() < react->GetEBISOnTime() ) return true;
		else return false;
	};
	inline bool	OnBeam( std::shared_ptr<SpedeEvt> s ){
		if( (double)s->GetTime() - (double)read_evts->GetEBIS() >= 0 &&
			(double)s->GetTime() - (double)read_evts->GetEBIS() < react->GetEBISOnTime() ) return true;
		else return false;
	};
	inline bool	OnBeam( std::shared_ptr<ParticleEvt> p ){
		if( (double)p->GetTime() - (double)read_evts->GetEBIS() >= 0 &&
			(double)p->GetTime() - (double)read_evts->GetEBIS() < react->GetEBISOnTime() ) return true;
		else return false;
	};
	inline bool	OffBeam( std::shared_ptr<GammaRayEvt> g ){
		if( (double)g->GetTime() - (double)read_evts->GetEBIS() >= react->GetEBISOnTime() &&
			(double)g->GetTime() - (double)read_evts->GetEBIS() < react->GetEBISOffTime() ) return true;
		else return false;
	};
	inline bool	OffBeam( std::shared_ptr<ParticleEvt> p ){
		if( (double)p->GetTime() - (double)read_evts->GetEBIS() >= react->GetEBISOnTime() &&
			(double)p->GetTime() - (double)read_evts->GetEBIS() < react->GetEBISOffTime() ) return true;
		else return false;
	};
	inline bool	OffBeam( std::shared_ptr<SpedeEvt> s ){
		if( (double)s->GetTime() - (double)read_evts->GetEBIS() >= react->GetEBISOnTime() &&
			(double)s->GetTime() - (double)read_evts->GetEBIS() < react->GetEBISOffTime() ) return true;
		else return false;
	};

	// Particle energy vs angle cuts
	inline bool TransferCut( std::shared_ptr<ParticleEvt> p ){
		return react->GetTransferCut()->IsInside( p->GetEnergy(), p->GetDeltaEnergy() );
	}
	inline bool EjectileCut( std::shared_ptr<ParticleEvt> p ){
		return react->GetEjectileCut()->IsInside( react->GetParticleTheta(p) * TMath::RadToDeg(), p->GetEnergy() );
	}
	inline bool RecoilCut( std::shared_ptr<ParticleEvt> p ){
		return react->GetRecoilCut()->IsInside( react->GetParticleTheta(p) * TMath::RadToDeg(), p->GetEnergy() );
	}
	inline bool TwoParticleCut( std::shared_ptr<ParticleEvt> p1, std::shared_ptr<ParticleEvt> p2 ){
		if( EjectileCut(p1) && RecoilCut(p2) && PromptCoincidence( p1, p2 ) &&
		    TMath::Abs( react->GetParticleVector(p1).DeltaPhi( react->GetParticleVector(p2) ) ) < 1.5*TMath::Pi() &&
		    TMath::Abs( react->GetParticleVector(p1).DeltaPhi( react->GetParticleVector(p2) ) ) > 0.5*TMath::Pi() )
			return true;
		else return false;
	}

private:
	
	// Reaction
	std::shared_ptr<MiniballReaction> react;
	
	// Settings file
	std::shared_ptr<MiniballSettings> set;
	
	// Input tree
	TChain *input_tree;
	MiniballEvts *read_evts = 0;
	std::shared_ptr<GammaRayEvt> gamma_evt, gamma_evt2;
	std::shared_ptr<GammaRayAddbackEvt> gamma_ab_evt, gamma_ab_evt2;
	std::shared_ptr<ParticleEvt> particle_evt, particle_evt2;
	std::shared_ptr<BeamDumpEvt> bd_evt, bd_evt2;
	std::shared_ptr<SpedeEvt> spede_evt, spede_evt2;
	std::shared_ptr<IonChamberEvt> ic_evt;

	// Output file
	TFile *output_file;
	
	// Progress bar
	bool _prog_;
	std::shared_ptr<TGProgressBar> prog;
	
	// Counters
	unsigned long n_entries;
	
	// Random number
	TRandom rand;
	
	//------------//
	// Histograms //
	//------------//
	
	// Histogram limits
	unsigned int GBIN = 6000;		// number of bins in gamma spectra
	float GMIN = -0.5;				// lower limit of energy in gamma spectra
	float GMAX = 5999.5;			// upper limit of energy in gamma spectra
	unsigned int EBIN = 2000;		// number of bins in electron spectra
	float EMIN = -0.5;				// lower limit of energy in electron spectra
	float EMAX = 1999.5;			// upper limit of energy in electron spectra
	unsigned int PBIN = 2000;		// number of bins in particle spectra
	float PMIN = 0.0;				// lower limit of energy in particle spectra
	float PMAX = 2000e3;			// upper limit of energy in particle spectra
	unsigned int TBIN;				// number of bins in time difference histograms
	float TMIN;						// lower limit of time difference
	float TMAX;						// upper limit of time difference
	
	// EBIS
	TH1F *ebis_td_particle, *ebis_td_gamma;
	
	// Gamma-rays with and without addback
	TH1F *gE_singles, *gE_singles_ebis, *gE_singles_ebis_on, *gE_singles_ebis_off;
	TH1F *aE_singles, *aE_singles_ebis, *aE_singles_ebis_on, *aE_singles_ebis_off;
	TH1F *gE_singles_dc, *gE_singles_dc_ebis;
	TH1F *aE_singles_dc, *aE_singles_dc_ebis;
	TH2F *gamma_xy_map_forward, *gamma_xy_map_backward;
	TH2F *gamma_xz_map_left, *gamma_xz_map_right;
	TH2F *gamma_theta_phi_map;

	// Electron singles
	TH1F *eE_singles, *eE_singles_ebis, *eE_singles_ebis_on, *eE_singles_ebis_off;
	TH2F *electron_xy_map;

	// Gamma-ray coincidence matrices with and without addback
	TH1F *gamma_gamma_td;
	TH2F *gE_gE, *gE_gE_ebis_on;
	TH2F *aE_aE, *aE_aE_ebis_on;

	// Electron coincidence matrices
	TH1F *electron_electron_td;
	TH2F *eE_eE, *eE_eE_ebis_on;

	// Gamma-Electron coincidence matrices
	TH1F *gamma_electron_td;
	TH2F *gE_eE, *gE_eE_ebis_on;
	TH2F *aE_eE, *aE_eE_ebis_on;

	// Particles
	TH2F *pE_theta, *pE_theta_coinc, *pE_theta_ejectile, *pE_theta_recoil;
	std::vector<std::vector<TH2F*>> pE_dE, pE_dE_coinc, pE_dE_cut;
	TProfile *pBeta_theta_ejectile, *pBeta_theta_recoil;
	TH1F *particle_particle_td;
	TH2F *particle_xy_map_forward, *particle_xy_map_backward, *particle_theta_phi_map;

	// Particle-gamma coincidences with and without addback
	TH1F *gamma_particle_td;
	TH2F *gamma_particle_E_vs_td;
	TH1F *gE_prompt, *gE_prompt_1p, *gE_prompt_2p;
	TH1F *gE_random, *gE_random_1p, *gE_random_2p;
	TH1F *aE_prompt, *aE_prompt_1p, *aE_prompt_2p;
	TH1F *aE_random, *aE_random_1p, *aE_random_2p;

	// Particle-electron coincidences
	TH1F *electron_particle_td;
	TH1F *eE_prompt, *eE_prompt_1p, *eE_prompt_2p;
	TH1F *eE_random, *eE_random_1p, *eE_random_2p;

	// Gamma-ray energy versus cos(theta)
	TH2F *gE_costheta_ejectile, *gE_costheta_recoil;
	TH2F *aE_costheta_ejectile, *aE_costheta_recoil;

	// Doppler-corrected gamma-rays without addback
	TH1F *gE_ejectile_dc_none, *gE_ejectile_dc_ejectile, *gE_ejectile_dc_recoil;
	TH1F *gE_recoil_dc_none,   *gE_recoil_dc_ejectile,   *gE_recoil_dc_recoil;
	TH1F *gE_2p_dc_none,       *gE_2p_dc_ejectile,       *gE_2p_dc_recoil;
	TH2F *gE_vs_theta_ejectile_dc_none, *gE_vs_theta_ejectile_dc_ejectile, *gE_vs_theta_ejectile_dc_recoil;
	TH2F *gE_vs_theta_recoil_dc_none,   *gE_vs_theta_recoil_dc_ejectile,   *gE_vs_theta_recoil_dc_recoil;
	TH2F *gE_vs_theta_2p_dc_none,       *gE_vs_theta_2p_dc_ejectile,       *gE_vs_theta_2p_dc_recoil;
	TH2F *ggE_ejectile_dc_none, *ggE_ejectile_dc_ejectile, *ggE_ejectile_dc_recoil;
	TH2F *ggE_recoil_dc_none,   *ggE_recoil_dc_ejectile,   *ggE_recoil_dc_recoil;

	// Doppler-corrected gamma-rays with addback
	TH1F *aE_ejectile_dc_none, *aE_ejectile_dc_ejectile, *aE_ejectile_dc_recoil;
	TH1F *aE_recoil_dc_none,   *aE_recoil_dc_ejectile,   *aE_recoil_dc_recoil;
	TH1F *aE_2p_dc_none,       *aE_2p_dc_ejectile,       *aE_2p_dc_recoil;
	TH2F *aE_vs_theta_ejectile_dc_none, *aE_vs_theta_ejectile_dc_ejectile, *aE_vs_theta_ejectile_dc_recoil;
	TH2F *aE_vs_theta_recoil_dc_none,   *aE_vs_theta_recoil_dc_ejectile,   *aE_vs_theta_recoil_dc_recoil;
	TH2F *aE_vs_theta_2p_dc_none,       *aE_vs_theta_2p_dc_ejectile,       *aE_vs_theta_2p_dc_recoil;
	TH2F *aaE_ejectile_dc_none, *aaE_ejectile_dc_ejectile, *aaE_ejectile_dc_recoil;
	TH2F *aaE_recoil_dc_none,   *aaE_recoil_dc_ejectile,   *aaE_recoil_dc_recoil;

	// Electron energy versus cos(theta)
	TH2F *eE_costheta_ejectile, *eE_costheta_recoil;
  
	// Doppler-corrected electrons
	TH1F *eE_ejectile_dc_none, *eE_ejectile_dc_ejectile, *eE_ejectile_dc_recoil;
	TH1F *eE_recoil_dc_none,   *eE_recoil_dc_ejectile,   *eE_recoil_dc_recoil;
	TH1F *eE_2p_dc_none,       *eE_2p_dc_ejectile,       *eE_2p_dc_recoil;
	TH2F *eE_vs_theta_ejectile_dc_none, *eE_vs_theta_ejectile_dc_ejectile, *eE_vs_theta_ejectile_dc_recoil;
	TH2F *eE_vs_theta_recoil_dc_none,   *eE_vs_theta_recoil_dc_ejectile,   *eE_vs_theta_recoil_dc_recoil;
	TH2F *eE_vs_theta_2p_dc_none,       *eE_vs_theta_2p_dc_ejectile,       *eE_vs_theta_2p_dc_recoil;
	TH2F *eE_vs_ejectile_dc_none_segment, *eE_vs_ejectile_dc_ejectile_segment, *eE_vs_ejectile_dc_recoil_segment;
	TH2F *eE_vs_recoil_dc_none_segment,   *eE_vs_recoil_dc_ejectile_segment,   *eE_vs_recoil_dc_recoil_segment;
	TH2F *egE_ejectile_dc_none, *egE_ejectile_dc_ejectile, *egE_ejectile_dc_recoil;
	TH2F *egE_recoil_dc_none,   *egE_recoil_dc_ejectile,   *egE_recoil_dc_recoil;
	TH2F *eaE_ejectile_dc_none, *eaE_ejectile_dc_ejectile, *eaE_ejectile_dc_recoil;
	TH2F *eaE_recoil_dc_none,   *eaE_recoil_dc_ejectile,   *eaE_recoil_dc_recoil;

	// Beam-dump histograms
	TH1F *bdE_singles;
	std::vector<TH1F*> bdE_singles_det;
	TH1F *bd_bd_td;
	TH2F *bdE_bdE;
	
	// IonChamber histograms
	TH1F *ic_dE, *ic_E;
	TH2F *ic_dE_E;

};

#endif
