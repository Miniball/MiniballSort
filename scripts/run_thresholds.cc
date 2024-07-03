#include <iostream>
#include <fstream>
#include "TH1F.h"
#include "TH2F.h"
#include "TFile.h"

int channel_threshold( TH1F *h ) {
	
	int thres;
	int maxbin = h->GetMaximumBin();
	double maxval = h->GetBinContent(maxbin);
	double curval;
	
	int binlimit = 20;
	double peakfraction = 0.005;
	
	for( int i = maxbin+1; i < maxbin + binlimit; ++i ){
		
		curval = h->GetBinContent(i);
		thres = h->GetBinCenter(i);
		if( curval < maxval*peakfraction ) break;
		
	}
	
	return thres;
	
}

int run_thresholds( string filename) {
	
	TFile *infile = new TFile( filename.data() );
	
	std::string hname;
	TH1F* h1;
	
	std::ofstream outfile( "thresholds_2024.dat" );
	
	int thres;
	
	// Loop over all sfps
	for( int sfp = 0; sfp < 2; ++sfp ) {
		
		// Loop over all boards
		for( int board = 0; board < 16; ++board ) {
			
			// Skip empty boards
			if( sfp == 0 && board > 12 ) continue;
			if( sfp == 1 && board > 7 ) continue;

			// Loop over all channels
			for( int i = 0; i < 16; ++i ){
				
				// Skip empty channels
				if( sfp == 0 && ( board == 7 || board == 8 ) ) continue;
				
				// Get histogram from file
				hname  = "sfp_" + std::to_string(sfp);
				hname += "/board_" + std::to_string(board);
				hname += "/febex_" + std::to_string(sfp) + "_";
				hname += std::to_string(board) + "_";
				hname += std::to_string(i) + "_qint";
				
				h1 = (TH1F*)infile->Get( hname.data() );
				thres = channel_threshold( h1 );
				
				outfile << "febex_" << sfp << "_" << board << "_" << i;
				outfile << ".Threshold:\t" << thres << endl;
				
			}
			
		}
		
	}
	
	outfile.close();
	infile->Close();
	
	return 0;
	
}


