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
#include <TKey.h>
#include <TCanvas.h>
#include <TROOT.h>

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

	void PlotDefaultHists();
	void PlotPhysicsHists();
	void SetSpyHists( std::vector<std::vector<std::string>> hists, short layout[2] );

	void SetInputFile( std::vector<std::string> input_file_names );
	void SetInputFile( std::string input_file_name );
	void SetInputTree( TTree *user_tree );

	inline void SetOutput( std::string output_file_name ){
		output_file = new TFile( output_file_name.data(), "recreate" );
		MakeHists();
		hists_ready = true;
		output_file->Write();
	};
	inline void CloseOutput( ){
		output_file->Write( nullptr, TObject::kOverwrite );
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
		if( (double)ptime - (double)g->GetTime() > react->GetParticleGammaPromptTime(0) &&
			(double)ptime - (double)g->GetTime() < react->GetParticleGammaPromptTime(1) )
			return true;
		else return false;
	};
	inline bool	RandomCoincidence( std::shared_ptr<GammaRayEvt> g, std::shared_ptr<ParticleEvt> p ){
		return RandomCoincidence( g, p->GetTime() );
	};
	inline bool	RandomCoincidence( std::shared_ptr<GammaRayEvt> g, unsigned long long ptime ){
		if( (double)ptime - (double)g->GetTime() > react->GetParticleGammaRandomTime(0) &&
			(double)ptime - (double)g->GetTime() < react->GetParticleGammaRandomTime(1) )
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
		if( (double)ptime - (double)s->GetTime() > react->GetParticleElectronPromptTime(0) &&
			(double)ptime - (double)s->GetTime() < react->GetParticleElectronPromptTime(1) )
			return true;
		else return false;
	};
	inline bool	RandomCoincidence( std::shared_ptr<SpedeEvt> s, std::shared_ptr<ParticleEvt> p ){
		return RandomCoincidence( s, p->GetTime() );
	};
	inline bool	RandomCoincidence( std::shared_ptr<SpedeEvt> s, unsigned long long ptime ){
		if( (double)ptime - (double)s->GetTime() > react->GetParticleElectronRandomTime(0) &&
			(double)ptime - (double)s->GetTime() < react->GetParticleElectronRandomTime(1) )
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
		if( (double)s1->GetTime() - (double)s2->GetTime() > react->GetElectronElectronPromptTime(0) &&
			(double)s1->GetTime() - (double)s2->GetTime() < react->GetElectronElectronPromptTime(1) )
			return true;
		else return false;
	};
	inline bool	RandomCoincidence( std::shared_ptr<SpedeEvt> s1, std::shared_ptr<SpedeEvt> s2 ){
		if( (double)s1->GetTime() - (double)s2->GetTime() > react->GetElectronElectronRandomTime(0) &&
			(double)s1->GetTime() - (double)s2->GetTime() < react->GetElectronElectronRandomTime(1) )
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
		if( (double)s->GetTime() - (double)g->GetTime() > react->GetGammaElectronPromptTime(0) &&
			(double)s->GetTime() - (double)g->GetTime() < react->GetGammaElectronPromptTime(1) )
			return true;
		else return false;
	};
	inline bool	PromptCoincidence( std::shared_ptr<GammaRayEvt> g, std::shared_ptr<SpedeEvt> s ){
		return PromptCoincidence( s, g );
	};
	inline bool	RandomCoincidence( std::shared_ptr<SpedeEvt> s, std::shared_ptr<GammaRayEvt> g ){
		if( (double)s->GetTime() - (double)g->GetTime() > react->GetGammaElectronRandomTime(0) &&
			(double)s->GetTime() - (double)g->GetTime() < react->GetGammaElectronRandomTime(1) )
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
	inline bool T1Cut(){
		if( (double)read_evts->GetEBIS() - (double)read_evts->GetT1() >= react->GetT1MinTime() &&
		    (double)read_evts->GetEBIS() - (double)read_evts->GetT1() < react->GetT1MaxTime() ) return true;
		else return false;
	}

	// Particle energy vs angle cuts
	inline bool TransferCut( std::shared_ptr<ParticleEvt> p ){
		double xval = p->GetEnergy();
		double yval = p->GetDeltaEnergy();
		if( react->GetTransferX() == "dE" ) xval = p->GetDeltaEnergy();
		else if( react->GetTransferX() == "E" ) xval = p->GetEnergy();
		else if( react->GetTransferX() == "theta" ) xval = react->GetParticleTheta(p) * TMath::RadToDeg();
		if( react->GetTransferY() == "dE" ) yval = p->GetDeltaEnergy();
		else if( react->GetTransferY() == "E" ) yval = p->GetEnergy();
		else if( react->GetTransferY() == "theta" ) yval = react->GetParticleTheta(p) * TMath::RadToDeg();
		return react->GetTransferCut()->IsInside( xval , yval );
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
	
	// Check if histograms are made
	bool hists_ready = false;

	// List of histograms for reset later
	TList *histlist;

	// Canvas and hist lists for the spy
	std::vector<std::vector<std::string>> spyhists;
	short spylayout[2];
	std::unique_ptr<TCanvas> c1, c2;
	bool spymode = false;

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
	unsigned int T1BIN = 2000;		// number of bins in T1 time difference histograms
	float T1MIN = 0;				// lower limit of T1 time difference
	float T1MAX = 20e9;				// upper limit of T1 time difference

	// EBIS
	TH1F *ebis_td_particle = nullptr, *ebis_td_gamma = nullptr;

	// Gamma-rays with and without addback
	TH1F *gE_singles = nullptr, *gE_singles_ebis = nullptr;
	TH1F *gE_singles_ebis_on = nullptr, *gE_singles_ebis_off = nullptr;
	TH1F *aE_singles = nullptr, *aE_singles_ebis = nullptr;
	TH1F *aE_singles_ebis_on = nullptr, *aE_singles_ebis_off = nullptr;
	TH1F *gE_singles_dc = nullptr, *gE_singles_dc_ebis = nullptr;
	TH1F *aE_singles_dc = nullptr, *aE_singles_dc_ebis = nullptr;
	TH2F *gE_singles_vs_crystal = nullptr;
	TH2F *aE_singles_vs_crystal = nullptr;
	TH2F *gamma_xy_map_forward = nullptr, *gamma_xy_map_backward = nullptr;
	TH2F *gamma_xz_map_left = nullptr, *gamma_xz_map_right = nullptr;
	TH2F *gamma_theta_phi_map = nullptr;

	// Electron singles
	TH1F *eE_singles = nullptr, *eE_singles_ebis = nullptr;
	TH1F *eE_singles_ebis_on = nullptr, *eE_singles_ebis_off = nullptr;
	TH2F *electron_xy_map = nullptr;

	// Gamma-ray coincidence matrices with and without addback
	TH1F *gamma_gamma_td, *gamma_gamma_td_prompt, *gamma_gamma_td_random = nullptr;
	TH2F *gE_gE, *gE_gE_ebis_on = nullptr;
	TH2F *aE_aE, *aE_aE_ebis_on = nullptr;

	// Electron coincidence matrices
	TH1F *electron_electron_td, *electron_electron_td_prompt, *electron_electron_td_random = nullptr;
	TH2F *eE_eE, *eE_eE_ebis_on = nullptr;

	// Gamma-Electron coincidence matrices
	TH1F *gamma_electron_td, *gamma_electron_td_prompt, *gamma_electron_td_random = nullptr;
	TH2F *gE_eE, *gE_eE_ebis_on = nullptr;
	TH2F *aE_eE, *aE_eE_ebis_on = nullptr;

	// Particles
	TH2F *pE_theta = nullptr, *pE_theta_coinc = nullptr;
	TH2F *pE_theta_ejectile = nullptr, *pE_theta_recoil = nullptr;
	TH2F *pE_theta_1p_ejectile = nullptr, *pE_theta_1p_recoil = nullptr;
	TH2F *pE_theta_2p_ejectile = nullptr, *pE_theta_2p_recoil = nullptr;
	std::vector<TH2F*> pE_dE, pE_dE_coinc, pE_dE_cut;
	std::vector<TH2F*> pE_theta_sec, pE_theta_coinc_sec;
	std::vector<TH2F*> pE_theta_ejectile_sec, pE_theta_recoil_sec;
	std::vector<std::vector<TH2F*>> pE_dE_sec, pE_dE_coinc_sec, pE_dE_cut_sec;
	TProfile *pBeta_theta_ejectile = nullptr, *pBeta_theta_recoil = nullptr;
	TH1F *particle_particle_td, *particle_particle_td_prompt, *particle_particle_td_random = nullptr;
	TH2F *particle_xy_map_forward = nullptr, *particle_xy_map_backward = nullptr;
	TH2F *particle_theta_phi_map = nullptr;
	
	// Particle-gamma coincidences with and without addback
	TH1F *gamma_particle_td, *gamma_particle_td_prompt, *gamma_particle_td_random = nullptr;
	TH2F *gamma_particle_E_vs_td = nullptr;
	std::vector<TH1F*> gamma_particle_td_sec;
	std::vector<TH2F*> gamma_particle_E_vs_td_sec;
	TH1F *gE_prompt = nullptr, *gE_prompt_1p = nullptr, *gE_prompt_2p = nullptr;
	TH1F *gE_random = nullptr, *gE_random_1p = nullptr, *gE_random_2p = nullptr;
	TH1F *aE_prompt = nullptr, *aE_prompt_1p = nullptr, *aE_prompt_2p = nullptr;
	TH1F *aE_random = nullptr, *aE_random_1p = nullptr, *aE_random_2p = nullptr;

	// Particle-electron coincidences
	TH1F *electron_particle_td, *electron_particle_td_prompt, *electron_particle_td_random = nullptr;
	TH1F *eE_prompt = nullptr, *eE_prompt_1p = nullptr, *eE_prompt_2p = nullptr;
	TH1F *eE_random = nullptr, *eE_random_1p = nullptr, *eE_random_2p = nullptr;

	// Doppler-corrected gamma-rays without addback
	TH1F *gE_ejectile_dc_none = nullptr,               *gE_ejectile_dc_ejectile = nullptr,               *gE_ejectile_dc_recoil = nullptr;
	TH1F *gE_recoil_dc_none = nullptr,                 *gE_recoil_dc_ejectile = nullptr,                 *gE_recoil_dc_recoil = nullptr;
	TH1F *gE_1p_ejectile_dc_none = nullptr,            *gE_1p_ejectile_dc_ejectile = nullptr,            *gE_1p_ejectile_dc_recoil = nullptr;
	TH1F *gE_1p_recoil_dc_none = nullptr,              *gE_1p_recoil_dc_ejectile = nullptr,              *gE_1p_recoil_dc_recoil = nullptr;
	TH1F *gE_2p_dc_none = nullptr,                     *gE_2p_dc_ejectile = nullptr,                     *gE_2p_dc_recoil = nullptr;
	TH2F *gE_ejectile_dc_none_t1 = nullptr,            *gE_ejectile_dc_ejectile_t1 = nullptr,            *gE_ejectile_dc_recoil_t1 = nullptr;
	TH2F *gE_recoil_dc_none_t1 = nullptr,              *gE_recoil_dc_ejectile_t1 = nullptr,              *gE_recoil_dc_recoil_t1 = nullptr;
	TH2F *gE_1p_ejectile_dc_none_t1 = nullptr,         *gE_1p_ejectile_dc_ejectile_t1 = nullptr,         *gE_1p_ejectile_dc_recoil_t1 = nullptr;
	TH2F *gE_1p_recoil_dc_none_t1 = nullptr,           *gE_1p_recoil_dc_ejectile_t1 = nullptr,           *gE_1p_recoil_dc_recoil_t1 = nullptr;
	TH2F *gE_2p_dc_none_t1 = nullptr,                  *gE_2p_dc_ejectile_t1 = nullptr,                  *gE_2p_dc_recoil_t1 = nullptr;
	TH2F *gE_vs_theta_ejectile_dc_none = nullptr,      *gE_vs_theta_ejectile_dc_ejectile = nullptr,      *gE_vs_theta_ejectile_dc_recoil = nullptr;
	TH2F *gE_vs_theta_recoil_dc_none = nullptr,        *gE_vs_theta_recoil_dc_ejectile = nullptr,        *gE_vs_theta_recoil_dc_recoil = nullptr;
	TH2F *gE_vs_theta_1p_ejectile_dc_none = nullptr,   *gE_vs_theta_1p_ejectile_dc_ejectile = nullptr,   *gE_vs_theta_1p_ejectile_dc_recoil = nullptr;
	TH2F *gE_vs_theta_1p_recoil_dc_none = nullptr,     *gE_vs_theta_1p_recoil_dc_ejectile = nullptr,     *gE_vs_theta_1p_recoil_dc_recoil = nullptr;
	TH2F *gE_vs_theta_2p_dc_none = nullptr,            *gE_vs_theta_2p_dc_ejectile = nullptr,            *gE_vs_theta_2p_dc_recoil = nullptr;
	TH2F *gE_vs_costheta_ejectile_dc_none = nullptr,   *gE_vs_costheta_ejectile_dc_ejectile = nullptr,   *gE_vs_costheta_ejectile_dc_recoil = nullptr;
	TH2F *gE_vs_costheta2_ejectile_dc_none = nullptr;
	TH2F *gE_vs_costheta_recoil_dc_none = nullptr,     *gE_vs_costheta_recoil_dc_ejectile = nullptr,     *gE_vs_costheta_recoil_dc_recoil = nullptr;
	TH2F *gE_vs_costheta2_recoil_dc_none = nullptr;
	TH2F *gE_vs_crystal_ejectile_dc_none = nullptr,    *gE_vs_crystal_ejectile_dc_ejectile = nullptr,    *gE_vs_crystal_ejectile_dc_recoil = nullptr;
	TH2F *gE_vs_crystal_recoil_dc_none = nullptr,      *gE_vs_crystal_recoil_dc_ejectile = nullptr,      *gE_vs_crystal_recoil_dc_recoil = nullptr;
	TH2F *gE_vs_crystal_1p_ejectile_dc_none = nullptr, *gE_vs_crystal_1p_ejectile_dc_ejectile = nullptr, *gE_vs_crystal_1p_ejectile_dc_recoil = nullptr;
	TH2F *gE_vs_crystal_1p_recoil_dc_none = nullptr,   *gE_vs_crystal_1p_recoil_dc_ejectile = nullptr,   *gE_vs_crystal_1p_recoil_dc_recoil = nullptr;
	TH2F *gE_vs_crystal_2p_dc_none = nullptr,          *gE_vs_crystal_2p_dc_ejectile = nullptr,          *gE_vs_crystal_2p_dc_recoil = nullptr;
	TH2F *ggE_ejectile_dc_none = nullptr,              *ggE_ejectile_dc_ejectile = nullptr,              *ggE_ejectile_dc_recoil = nullptr;
	TH2F *ggE_recoil_dc_none = nullptr,                *ggE_recoil_dc_ejectile = nullptr,                *ggE_recoil_dc_recoil = nullptr;

	// Doppler-corrected gamma-rays with addback
	TH1F *aE_ejectile_dc_none = nullptr,               *aE_ejectile_dc_ejectile = nullptr,               *aE_ejectile_dc_recoil = nullptr;
	TH1F *aE_recoil_dc_none = nullptr,                 *aE_recoil_dc_ejectile = nullptr,                 *aE_recoil_dc_recoil = nullptr;
	TH1F *aE_1p_ejectile_dc_none = nullptr,            *aE_1p_ejectile_dc_ejectile = nullptr,            *aE_1p_ejectile_dc_recoil = nullptr;
	TH1F *aE_1p_recoil_dc_none = nullptr,              *aE_1p_recoil_dc_ejectile = nullptr,              *aE_1p_recoil_dc_recoil = nullptr;
	TH1F *aE_2p_dc_none = nullptr,                     *aE_2p_dc_ejectile = nullptr,                     *aE_2p_dc_recoil = nullptr;
	TH2F *aE_ejectile_dc_none_t1 = nullptr,            *aE_ejectile_dc_ejectile_t1 = nullptr,            *aE_ejectile_dc_recoil_t1 = nullptr;
	TH2F *aE_recoil_dc_none_t1 = nullptr,              *aE_recoil_dc_ejectile_t1 = nullptr,              *aE_recoil_dc_recoil_t1 = nullptr;
	TH2F *aE_1p_ejectile_dc_none_t1 = nullptr,         *aE_1p_ejectile_dc_ejectile_t1 = nullptr,         *aE_1p_ejectile_dc_recoil_t1 = nullptr;
	TH2F *aE_1p_recoil_dc_none_t1 = nullptr,           *aE_1p_recoil_dc_ejectile_t1 = nullptr,           *aE_1p_recoil_dc_recoil_t1 = nullptr;
	TH2F *aE_2p_dc_none_t1 = nullptr,                  *aE_2p_dc_ejectile_t1 = nullptr,                  *aE_2p_dc_recoil_t1 = nullptr;
	TH2F *aE_vs_theta_ejectile_dc_none = nullptr,      *aE_vs_theta_ejectile_dc_ejectile = nullptr,      *aE_vs_theta_ejectile_dc_recoil = nullptr;
	TH2F *aE_vs_theta_recoil_dc_none = nullptr,        *aE_vs_theta_recoil_dc_ejectile = nullptr,        *aE_vs_theta_recoil_dc_recoil = nullptr;
	TH2F *aE_vs_theta_1p_ejectile_dc_none = nullptr,   *aE_vs_theta_1p_ejectile_dc_ejectile = nullptr,   *aE_vs_theta_1p_ejectile_dc_recoil = nullptr;
	TH2F *aE_vs_theta_1p_recoil_dc_none = nullptr,     *aE_vs_theta_1p_recoil_dc_ejectile = nullptr,     *aE_vs_theta_1p_recoil_dc_recoil = nullptr;
	TH2F *aE_vs_theta_2p_dc_none = nullptr,            *aE_vs_theta_2p_dc_ejectile = nullptr,            *aE_vs_theta_2p_dc_recoil = nullptr;
	TH2F *aE_vs_costheta_ejectile_dc_none = nullptr,   *aE_vs_costheta_ejectile_dc_ejectile = nullptr,   *aE_vs_costheta_ejectile_dc_recoil = nullptr;
	TH2F *aE_vs_costheta2_ejectile_dc_none = nullptr;
	TH2F *aE_vs_costheta_recoil_dc_none = nullptr,     *aE_vs_costheta_recoil_dc_ejectile = nullptr,     *aE_vs_costheta_recoil_dc_recoil = nullptr;
	TH2F *aE_vs_costheta2_recoil_dc_none = nullptr;
	TH2F *aE_vs_crystal_ejectile_dc_none = nullptr,    *aE_vs_crystal_ejectile_dc_ejectile = nullptr,    *aE_vs_crystal_ejectile_dc_recoil = nullptr;
	TH2F *aE_vs_crystal_recoil_dc_none = nullptr,      *aE_vs_crystal_recoil_dc_ejectile = nullptr,      *aE_vs_crystal_recoil_dc_recoil = nullptr;
	TH2F *aE_vs_crystal_1p_ejectile_dc_none = nullptr, *aE_vs_crystal_1p_ejectile_dc_ejectile = nullptr, *aE_vs_crystal_1p_ejectile_dc_recoil = nullptr;
	TH2F *aE_vs_crystal_1p_recoil_dc_none = nullptr,   *aE_vs_crystal_1p_recoil_dc_ejectile = nullptr,   *aE_vs_crystal_1p_recoil_dc_recoil = nullptr;
	TH2F *aE_vs_crystal_2p_dc_none = nullptr,          *aE_vs_crystal_2p_dc_ejectile = nullptr,          *aE_vs_crystal_2p_dc_recoil = nullptr;
	TH2F *aaE_ejectile_dc_none = nullptr,              *aaE_ejectile_dc_ejectile = nullptr,              *aaE_ejectile_dc_recoil = nullptr;
	TH2F *aaE_recoil_dc_none = nullptr,                *aaE_recoil_dc_ejectile = nullptr,                *aaE_recoil_dc_recoil = nullptr;

	// Segment phi determination
	std::vector<TH2F*> gE_vs_phi_dc_ejectile;
	std::vector<TH2F*> gE_vs_phi_dc_recoil;
	
	// Electron energy versus cos(theta)
	TH2F *eE_costheta_ejectile = nullptr, *eE_costheta_recoil = nullptr;

	// Doppler-corrected electrons
	TH1F *eE_ejectile_dc_none = nullptr,             *eE_ejectile_dc_ejectile = nullptr,             *eE_ejectile_dc_recoil = nullptr;
	TH1F *eE_recoil_dc_none = nullptr,               *eE_recoil_dc_ejectile = nullptr,               *eE_recoil_dc_recoil = nullptr;
	TH1F *eE_1p_ejectile_dc_none = nullptr,          *eE_1p_ejectile_dc_ejectile = nullptr,          *eE_1p_ejectile_dc_recoil = nullptr;
	TH1F *eE_1p_recoil_dc_none = nullptr,            *eE_1p_recoil_dc_ejectile = nullptr,            *eE_1p_recoil_dc_recoil = nullptr;
	TH1F *eE_2p_dc_none = nullptr,                   *eE_2p_dc_ejectile = nullptr,                   *eE_2p_dc_recoil = nullptr;
	TH2F *eE_vs_theta_ejectile_dc_none = nullptr,    *eE_vs_theta_ejectile_dc_ejectile = nullptr,    *eE_vs_theta_ejectile_dc_recoil = nullptr;
	TH2F *eE_vs_theta_recoil_dc_none = nullptr,      *eE_vs_theta_recoil_dc_ejectile = nullptr,      *eE_vs_theta_recoil_dc_recoil = nullptr;
	TH2F *eE_vs_theta_1p_ejectile_dc_none = nullptr, *eE_vs_theta_1p_ejectile_dc_ejectile = nullptr, *eE_vs_theta_1p_ejectile_dc_recoil = nullptr;
	TH2F *eE_vs_theta_1p_recoil_dc_none = nullptr,   *eE_vs_theta_1p_recoil_dc_ejectile = nullptr,   *eE_vs_theta_1p_recoil_dc_recoil = nullptr;
	TH2F *eE_vs_theta_2p_dc_none = nullptr,          *eE_vs_theta_2p_dc_ejectile = nullptr,          *eE_vs_theta_2p_dc_recoil = nullptr;
	TH2F *eE_vs_ejectile_dc_none_segment = nullptr,  *eE_vs_ejectile_dc_ejectile_segment = nullptr,  *eE_vs_ejectile_dc_recoil_segment = nullptr;
	TH2F *eE_vs_recoil_dc_none_segment = nullptr,    *eE_vs_recoil_dc_ejectile_segment = nullptr,    *eE_vs_recoil_dc_recoil_segment = nullptr;
	TH2F *egE_ejectile_dc_none = nullptr,            *egE_ejectile_dc_ejectile = nullptr,            *egE_ejectile_dc_recoil = nullptr;
	TH2F *egE_recoil_dc_none = nullptr,              *egE_recoil_dc_ejectile = nullptr,              *egE_recoil_dc_recoil = nullptr;
	TH2F *eaE_ejectile_dc_none = nullptr,            *eaE_ejectile_dc_ejectile = nullptr,            *eaE_ejectile_dc_recoil = nullptr;
	TH2F *eaE_recoil_dc_none = nullptr,              *eaE_recoil_dc_ejectile = nullptr,              *eaE_recoil_dc_recoil = nullptr;

	// Beam-dump histograms
	TH1F *bdE_singles = nullptr;
	std::vector<TH1F*> bdE_singles_det;
	TH1F *bd_bd_td = nullptr;
	TH2F *bdE_bdE = nullptr;

	// IonChamber histograms
	TH1F *ic_dE = nullptr, *ic_E = nullptr;
	TH2F *ic_dE_E = nullptr;

};

#endif
