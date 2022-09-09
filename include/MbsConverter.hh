#ifndef __MBSCONVERTER_HH
#define __MBSCONVERTER_HH

#include <bitset>
#include <memory>
#include <fstream>
#include <iostream>
#include <iomanip>
#include <stdio.h>
#include <sstream>
#include <string>

#include <TFile.h>
#include <TTree.h>
#include <TTreeIndex.h>
#include <TH1.h>
#include <TH2.h>
#include <TProfile.h>
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

class MiniballMbsConverter {

public:
	
	MiniballMbsConverter( std::shared_ptr<MiniballSettings> myset );
	~MiniballMbsConverter() {};
	

	int ConvertFile( std::string input_file_name,
					unsigned long start_block = 0,
					long end_block = -1);
	void MakeHists();
	void MakeTree();
	unsigned long long SortTree();

	bool ProcessCurrentBlock( int nblock );

	void ProcessBlockHeader( unsigned long nblock );

	void ProcessBlockData( unsigned long nblock );

	bool GetFebexChanID();
	int  ProcessTraceData( int pos );
	void ProcessFebexData();
	void FinishFebexData();

	void SetOutput( std::string output_file_name );
	
	inline void CloseOutput(){
		std::cout << "\n Writing data and closing the file" << std::endl;
		output_file->Write( 0, TObject::kWriteDelete );
		output_file->Close();
	};
	inline TFile* GetFile(){ return output_file; };
	inline TTree* GetTree(){ return output_tree; };
	inline TTree* GetSortedTree(){ return sorted_tree; };

	inline void AddCalibration( std::shared_ptr<MiniballCalibration> mycal ){ cal = mycal; };
	inline void SourceOnly(){ flag_source = true; };

	inline void AddProgressBar( std::shared_ptr<TGProgressBar> myprog ){
		prog = myprog;
		_prog_ = true;
	};



private:

	// Flag for source run
	bool flag_source;

	// Logs
	std::stringstream sslogs;
	
	// Set the size of the block and its components.
	static const int HEADER_SIZE = 24; // Size of header in bytes
	static const int DATA_BLOCK_SIZE = 0x10000; // Block size for FEBEX data = 64 kB?
	static const int MAIN_SIZE = DATA_BLOCK_SIZE - HEADER_SIZE;
	static const int WORD_SIZE = MAIN_SIZE / sizeof(ULong64_t);


	// Flag to identify Febex data words
	bool flag_febex_data0;
	bool flag_febex_data1;
	bool flag_febex_data2;
	bool flag_febex_data3;
	bool flag_febex_trace;
	bool flag_febex_info;


	// Interpretated variables
	unsigned long long my_tm_stp;
	unsigned long my_tm_stp_lsb;
	unsigned long my_tm_stp_msb;
	unsigned long my_tm_stp_hsb;
	unsigned short my_adc_data;
	unsigned char my_info_code;
	unsigned char my_sfp_id;
	unsigned char my_board_id;
	unsigned char my_ch_id;
	float my_energy;

	// For traces
	unsigned int nsamples;

	// Data types
	std::shared_ptr<MiniballDataPackets> data_packet = 0;
	std::shared_ptr<FebexData> febex_data;
	std::shared_ptr<InfoData> info_data;
	
	// Output stuff
	TFile *output_file;
	TTree *output_tree;
	TTree *sorted_tree;

	// Counters
	std::vector<std::vector<unsigned long>> ctr_febex_hit;		// hits on each Febex module
	std::vector<std::vector<unsigned long>> ctr_febex_pause;   	// pause acq for module
	std::vector<std::vector<unsigned long>> ctr_febex_resume;  	// resume acq for module
	unsigned long ctr_febex_ext;								// pulser timestamps

	// Histograms
	std::vector<std::vector<TProfile*>> hfebex_hit;
	std::vector<std::vector<TProfile*>> hfebex_pause;
	std::vector<std::vector<TProfile*>> hfebex_resume;
	TProfile *hfebex_ext;

	std::vector<std::vector<std::vector<TH1F*>>> hfebex;
	std::vector<std::vector<std::vector<TH1F*>>> hfebex_cal;
	std::vector<std::vector<std::vector<TH1F*>>> hfebex_mwd;

	// 	Settings file
	std::shared_ptr<MiniballSettings> set;

	// 	Calibrator
	std::shared_ptr<MiniballCalibration> cal;
	
	// Progress bar
	bool _prog_;
	std::shared_ptr<TGProgressBar> prog;

};

#endif
