#ifndef _EventBuilder_hh
#define _EventBuilder_hh

#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include <TFile.h>
#include <TTree.h>
#include <TMath.h>
#include <TChain.h>
#include <TH1.h>
#include <TH2.h>
#include <TProfile.h>
#include <TVector2.h>
#include <TVector3.h>

// Settings header
#ifndef __SETTINGS_HH
# include "Settings.hh"
#endif

// Calibration header
#ifndef __CALIBRATION_HH
# include "Calibration.hh"
#endif

// Data packets header
#ifndef __DATAPACKETS_hh
# include "DataPackets.hh"
#endif

// Miniball Events tree
#ifndef __MINIBALLEVTS_HH
# include "MiniballEvts.hh"
#endif



class EventBuilder {
	
public:

	EventBuilder( Settings *myset );
	~EventBuilder() {};

	void	SetInputFile( std::string input_file_name );
	void	SetInputFile( std::vector<std::string> input_file_names );
	void	SetInputTree( TTree* user_tree );
	void	SetOutput( std::string output_file_name );
	void	StartFile();	///< called for every file
	void	Initialise();	///< called for every event
	void	MakeEventHists();
	
	inline void AddCalibration( Calibration *mycal ){
		cal = mycal;
		overwrite_cal = true;
	};
	
	unsigned long	BuildEvents( unsigned long start_build = 0 );

	// Resolve multiplicities and coincidences etc
	void GammaRayFinder();
	void ParticleFinder();

	inline TFile* GetFile(){ return output_file; };
	inline TTree* GetTree(){ return output_tree; };
	inline void CloseOutput(){
		output_file->Close();
	};
	void CleanHists();


private:
	
	/// Input tree
	TChain *input_tree;
	DataPackets *in_data;
	FebexData *febex_data;
	InfoData *info_data;

	/// Outputs
	TFile *output_file;
	TTree *output_tree;
	MiniballEvts *write_evts;
	GammaRayEvt *gamma_evt;
	GammaRayAddbackEvt *gamma_ab_evt;
	
	// Do calibration
	Calibration *cal;
	bool overwrite_cal;
	
	// Settings file
	Settings *set;
	
	// Build window which comes from the settings file
	long build_window;  /// length of build window in ns
	

	// Flags
	bool flag_close_event;
	std::vector<std::vector<bool>> flag_pause, flag_resume;
	bool noise_flag, event_open;

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


	// Counters
	unsigned long				hit_ctr, gamma_ctr, gamma_ab_ctr, cd_ctr, bd_ctr;
	unsigned long				n_entries, n_febex_data, n_info_data;
	unsigned long				n_ebis, n_t1, n_pulser;
	std::vector<unsigned long>	n_sfp;
	std::vector<std::vector<unsigned long>>	n_board;
	std::vector<std::vector<unsigned long>>	n_pause, n_resume;
	unsigned long				n_miniball;


	// Timing histograms
	TH1F *tdiff, *tdiff_clean;
	TProfile *pulser_freq, *ebis_freq, *t1_freq;

	
};

#endif

