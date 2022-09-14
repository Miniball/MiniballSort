#ifndef __EVENTBUILDER_HH
#define __EVENTBUILDER_HH

#include <fstream>
#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <memory>

#include <TFile.h>
#include <TTree.h>
#include <TMath.h>
#include <TChain.h>
#include <TH1.h>
#include <TH2.h>
#include <TProfile.h>
#include <TVector2.h>
#include <TVector3.h>
#include <TGProgressBar.h>
#include <TSystem.h>

// Settings header
#ifndef __SETTINGS_HH
# include "Settings.hh"
#endif

// Calibration header
#ifndef __CALIBRATION_HH
# include "Calibration.hh"
#endif

// Data packets header
#ifndef __DATAPACKETS_HH
# include "DataPackets.hh"
#endif

// Miniball Events tree
#ifndef __MINIBALLEVTS_HH
# include "MiniballEvts.hh"
#endif



class MiniballEventBuilder {
	
public:

	MiniballEventBuilder( std::shared_ptr<MiniballSettings> myset );
	~MiniballEventBuilder() {};

	void	SetInputFile( std::string input_file_name );
	void	SetInputTree( TTree *user_tree );
	void	SetOutput( std::string output_file_name );
	void	StartFile();	///< called for every file
	void	Initialise();	///< called for every event
	void	MakeEventHists();
	
	inline void AddCalibration( std::shared_ptr<MiniballCalibration> mycal ){
		cal = mycal;
		overwrite_cal = true;
	};
	inline void AddProgressBar( std::shared_ptr<TGProgressBar> myprog ){
		prog = myprog;
		_prog_ = true;
	};
	
	unsigned long	BuildEvents();

	// Resolve multiplicities and coincidences etc
	void GammaRayFinder();
	void ParticleFinder();
	void BeamDumpFinder();
	void SpedeFinder();

	inline TFile* GetFile(){ return output_file; };
	inline TTree* GetTree(){ return output_tree; };
	inline void CloseOutput(){
		output_tree->ResetBranchAddresses();
		output_file->Close();
		input_tree->ResetBranchAddresses();
		input_file->Close();
		delete in_data;
		log_file.close(); //?? to close or not to close?
	}; ///< Closes the output files from this class
	void CleanHists(); ///< Deletes histograms from memory and clears vectors that store histograms


private:
	
	/// Input tree
	TFile *input_file;
	TTree *input_tree;
	MiniballDataPackets *in_data;
	std::shared_ptr<FebexData> febex_data;
	std::shared_ptr<InfoData> info_data;

	/// Outputs
	TFile *output_file;
	TTree *output_tree;
	std::unique_ptr<MiniballEvts> write_evts;
	std::shared_ptr<GammaRayEvt> gamma_evt;
	std::shared_ptr<GammaRayAddbackEvt> gamma_ab_evt;
	std::shared_ptr<ParticleEvt> particle_evt;
	std::shared_ptr<SpedeEvt> spede_evt;
	std::shared_ptr<BeamDumpEvt> bd_evt;

	// Do calibration
	std::shared_ptr<MiniballCalibration> cal;
	bool overwrite_cal;
	
	// Settings file
	std::shared_ptr<MiniballSettings> set;
	
	// Progress bar
	bool _prog_;
	std::shared_ptr<TGProgressBar> prog;
	
	// Log file
	std::stringstream ss_log;
	std::ofstream log_file; ///< Log file for recording the results of the MiniballEventBuilder
	
	// Flag to know we've opened a file on disk
	bool flag_input_file;

	// Build window which comes from the settings file
	long build_window;  /// length of build window in ns

	// Flags
	bool flag_close_event;
	std::vector<std::vector<bool>> flag_pause, flag_resume;
	bool event_open;

	// Time variables
	long		 		time_diff;
	unsigned long long	time_prev, time_min, time_max, time_first;
	unsigned long long  ebis_time, ebis_prev;
	unsigned long long  t1_time, t1_prev;
	unsigned long long  pulser_time, pulser_prev;
	double pulser_hz, ebis_hz, t1_hz;
	std::vector<std::vector<unsigned long long>> pause_time, resume_time;
	std::vector<std::vector<unsigned long long>> febex_dead_time;
	std::vector<std::vector<unsigned long long>> febex_time_start, febex_time_stop;

	// Data variables - generic
	unsigned char		mysfp;		///< sfp number
	unsigned char		myboard;	///< febex board number
	unsigned char		mych;		///< channel number
	unsigned long long	mytime;		///< absolute timestamp
	float 				myenergy;	///< calibrated energy
	bool				mythres;	///< above threshold?


	// Miniball specific variables
	std::vector<float>					mb_en_list;		///< list of Miniball energies for GammaRayFinder
	std::vector<unsigned long long>		mb_ts_list;		///< list of Miniball timestamps for GammaRayFinder
	std::vector<unsigned char>			mb_clu_list;	///< list of cluster IDs
	std::vector<unsigned char>			mb_cry_list;	///< list of crystal IDs
	std::vector<unsigned char>			mb_seg_list;	///< list of segment IDs

	// CD detector specific variables
	std::vector<float>					cd_en_list;		///< list of CD energies for ParticleFinder
	std::vector<unsigned long long>		cd_ts_list;		///< list of CD timestamps for ParticleFinder
	std::vector<unsigned char>			cd_det_list;	///< list of CD detector IDs
	std::vector<unsigned char>			cd_sec_list;	///< list of CD sector IDs
	std::vector<unsigned char>			cd_side_list;	///< list of CD side IDs; 0 = p, 1 = n
	std::vector<unsigned char>			cd_strip_list;	///< list of CD strip IDs

	// Beam dumo detector specific variables
	std::vector<float>					bd_en_list;		///< list of beam dump energies for ParticleFinder
	std::vector<unsigned long long>		bd_ts_list;		///< list of beam dump timestamps for ParticleFinder
	std::vector<unsigned char>			bd_det_list;	///< list of beam dump detector IDs

	// Spede detector specific variables
	std::vector<float>					spede_en_list;		///< list of Spede energies for ParticleFinder
	std::vector<unsigned long long>		spede_ts_list;		///< list of Spede timestamps for ParticleFinder
	std::vector<unsigned char>			spede_seg_list;		///< list of Spede segment IDs


	// Counters
	unsigned long				hit_ctr, gamma_ctr, gamma_ab_ctr, cd_ctr, bd_ctr, spede_ctr;
	unsigned long				n_entries, n_febex_data, n_info_data;
	unsigned long				n_ebis, n_t1, n_pulser;
	std::vector<unsigned long>	n_sfp;
	std::vector<std::vector<unsigned long>>	n_board;
	std::vector<std::vector<unsigned long>>	n_pause, n_resume;
	unsigned long				n_miniball, n_cd, n_spede, n_bd;


	// Timing histograms
	TH1F *tdiff, *tdiff_clean;
	TProfile *pulser_freq, *ebis_freq, *t1_freq;
	
	// Miniball histograms
	TH1F *mb_td_core_seg;
	TH1F *mb_td_core_core;
	std::vector<std::vector<std::vector<TH1F*>>> mb_en_core_seg;

	// CD histograms
	
};

#endif

