#ifndef _TIMESORTER_HH
#define _TIMESORTER_HH

#include <bitset>
#include <fstream>
#include <iostream>
#include <iomanip>
#include <string>
#include <vector>
#include <ctime>
#include <memory>

#include <TFile.h>
#include <TTree.h>
#include <TTreeIndex.h>
#include <TGProgressBar.h>
#include <TSystem.h>

class TimeSorter {

public:

	TimeSorter();
	~TimeSorter(){};
	
	unsigned long SortFile( unsigned long start_sort = 0 );
	
	bool	SetInputFile( std::string input_file_name );
	void	SetInputTree( TTree *user_tree );
	void	SetOutput( std::string output_file_name );
	inline void CloseOutput(){
		output_file->Close();
		input_file->Close(); // Close TFile
	};

	inline TFile* GetFile(){ return output_file; };
	inline TTree* GetTree(){ return output_tree; };

	inline void AddProgressBar( std::shared_ptr<TGProgressBar> myprog ){
		prog = myprog;
		_prog_ = true;
	};



private:
		
	TFile *input_file;
	TTree *input_tree;
	TFile *output_file;
	TTree *output_tree;
	
	TTreeIndex *att_index;

	// Counters
	unsigned long long n_entries;
	unsigned long long nb_idx, idx;
	
	// Sort time
	time_t t_start;
	
	// Progress bar
	bool _prog_;
	std::shared_ptr<TGProgressBar> prog;

};

#endif
