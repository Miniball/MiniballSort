#include "Settings.hh"

ClassImp(MiniballSettings)

MiniballSettings::MiniballSettings() {
	
	SetFile( "dummy" );
	ReadSettings();
	
}

MiniballSettings::MiniballSettings( std::string filename ) {
	
	SetFile( filename );
	ReadSettings();
	TestSettings();
	
}

void MiniballSettings::TestSettings() {
	
	// Loop over all channels and see what we find
	for( unsigned int i = 0; i < n_febex_sfp; ++i ){
		
		for( unsigned int j = 0; j < n_febex_board; ++j ){
			
			for( unsigned int k = 0; k < n_febex_ch; ++k ){
		
				std::vector<std::string> active_chs;
				std::string detname;
				
				// Is it a Miniball?
				if( IsMiniball(i,j,k) ){
					
					detname = "Miniball_";
					detname += std::to_string( GetMiniballCluster(i,j,k) ) + "_";
					detname += std::to_string( GetMiniballCrystal(i,j,k) ) + "_";
					detname += std::to_string( GetMiniballSegment(i,j,k) ) + "\n";
					active_chs.push_back( detname );
					
				}
					
				// Is it a CD?
				if( IsCD(i,j,k) ){
					
					detname = "CD_";
					detname += std::to_string( GetCDDetector(i,j,k) ) + "_";
					detname += std::to_string( GetCDSector(i,j,k) ) + "_";
					detname += std::to_string( GetCDStrip(i,j,k) ) + ".";
					detname += std::to_string( GetCDSide(i,j,k) ) + "\n";
					active_chs.push_back( detname );
					
				}
				
				// Is it a Pad?
				if( IsPad(i,j,k) ){
					
					detname = "Pad_";
					detname += std::to_string( GetPadDetector(i,j,k) ) + "_";
					detname += std::to_string( GetPadSector(i,j,k) ) + "\n";
					active_chs.push_back( detname );
					
				}
				
				// Is it a Spede?
				if( IsSpede(i,j,k) ){
					
					detname = "Spede_";
					detname += std::to_string( GetSpedeSegment(i,j,k) ) + "\n";
					active_chs.push_back( detname );
					
				}
				
				// Is it a beam dump?
				if( IsBeamDump(i,j,k) ){
					
					detname = "BeamDump_";
					detname += std::to_string( GetBeamDumpDetector(i,j,k) ) + "\n";
					active_chs.push_back( detname );
					
				}
				
				// Is it an IonChamber?
				if( IsIonChamber(i,j,k) ){
					
					detname = "IonChamber_";
					detname += std::to_string( GetIonChamberLayer(i,j,k) ) + "\n";
					active_chs.push_back( detname );
					
				}

				// Is it a pulser?
				if( IsPulser(i,j,k) ){
					
					detname = "Pulser_";
					detname += std::to_string( GetPulser(i,j,k) ) + "\n";
					active_chs.push_back( detname );
					
				}

				// Is it EBIS?
				if( GetEBISSfp() == i && GetEBISBoard() == j && GetEBISChannel() == k )
					active_chs.push_back( "EBIS\n" );
				
				// Is it T1?
				if( GetT1Sfp() == i && GetT1Board() == j && GetT1Channel() == k )
					active_chs.push_back( "T1\n" );
				
				// Is it SC?
				if( GetSCSfp() == i && GetSCBoard() == j && GetSCChannel() == k )
					active_chs.push_back( "SC\n" );
				
				// Is it RILIS?
				if( GetRILISSfp() == i && GetRILISBoard() == j && GetRILISChannel() == k )
					active_chs.push_back( "RILIS\n" );
				
				
				// Check now if we have multiple detectors in this channel
				if( active_chs.size() > 1 ) {
					
					std::cout << "Too many detectors in SFP " << i;
					std::cout << ", board " << j << ", channel " << k << ":\n";
					for( unsigned int q = 0; q < active_chs.size(); ++q )
						std::cout << active_chs.at(q);
					
				} // more than one detector
				
			} // k = ch

		} // j = board
		
	} // i = sfp
	
}

void MiniballSettings::ReadSettings() {
	
	TEnv *config = new TEnv( fInputFile.data() );
	
	// FEBEX initialisation
	n_febex_sfp		= config->GetValue( "NumberOfFebexSfps", 2 );
	n_febex_board	= config->GetValue( "NumberOfFebexBoards", 16 );
	n_febex_ch		= config->GetValue( "NumberOfFebexChannels", 16 );

	// DGF initialisation
	n_dgf_mod			= config->GetValue( "NumberOfDgfModules", 0 );
	n_dgf_ts_mod		= config->GetValue( "NumberOfDgfTimestampModules", 5 );
	n_dgf_ch			= config->GetValue( "NumberOfDgfChannels", 4 );
	dgf_mod_offset		= config->GetValue( "DgfModuleOffset", 1 );
	dgf_ts_mod_offset	= config->GetValue( "DgfTimestampModuleOffset", 49 );
	dgf_ts_delay		= config->GetValue( "DgfTimestampDelay", 65527 );
	dgf_ts_units		= config->GetValue( "DgfTimestampUnits", 25. );

	// CAEN ADC initialisation
	n_caen_mod		= config->GetValue( "NumberOfCaenAdcModules", 0 );
	n_caen_ch		= config->GetValue( "NumberOfCaenAdcChannels", 4 );
	caen_mod_offset	= config->GetValue( "CaenAdcModuleOffset", 60 );
	caen_ts_units	= config->GetValue( "CaenAdcTimestampUnits", 25. );

	// Mesytec ADC initialisation
	n_madc_mod		= config->GetValue( "NumberOfMesytecAdcModules", 0 );
	n_madc_ch		= config->GetValue( "NumberOfMesytecAdcChannels", 4 );
	madc_mod_offset	= config->GetValue( "MesytecAdcModuleOffset", 55 );
	madc_ts_units	= config->GetValue( "MesytecAdcTimestampUnits", 25. );

	// Scaler unit initialisation
	n_scaler_unit		= config->GetValue( "NumberOfScalerUnits", 1 );
	scaler_unit_offset	= config->GetValue( "ScalerUnitOffset", 0 );
	
	// Pattern unit initialisation
	n_pattern_unit		= config->GetValue( "NumberOfPatternUnits", 1 );
	pattern_unit_offset	= config->GetValue( "PatternUnitOffset", 67 );

	// VME ID initialisation
	dgf_vme_first		= config->GetValue( "Dgf.VME.First", 1 );
	dgf_vme_last		= config->GetValue( "Dgf.VME.Last", 10 );
	adc_vme_first		= config->GetValue( "Adc.VME.First", 11 );
	adc_vme_last		= config->GetValue( "Adc.VME.Last", 19 );
	scaler_vme_first	= config->GetValue( "Scaler.VME.First", 20 );
	scaler_vme_last		= config->GetValue( "Scaler.VME.Last", 20 );
	pattern_vme_first	= config->GetValue( "Pattern.VME.First", 21 );
	pattern_vme_last	= config->GetValue( "Pattern.VME.Last", 21 );
	dgfscaler_vme_first	= config->GetValue( "DgfScaler.VME.First", 22 );
	dgfscaler_vme_last	= config->GetValue( "DgfScaler.VME.Last", 24 );

	// Miniball array initialisation
	n_mb_cluster	= config->GetValue( "NumberOfMiniballClusters", 8 );
	n_mb_crystal	= config->GetValue( "NumberOfMiniballCrystals", 3 );
	n_mb_segment	= config->GetValue( "NumberOfMiniballSegments", 7 );
	
	// CD detector initialisation
	n_cd_det		= config->GetValue( "NumberOfCDDetectors", 1 );
	n_cd_sector		= config->GetValue( "NumberOfCDSectors", 4 );
	n_cd_side		= 2; // always 2, you cannot change this!
	n_cd_pstrip		= config->GetValue( "NumberOfCDStrips.P", 16 );
	n_cd_nstrip		= config->GetValue( "NumberOfCDStrips.N", 12 );

	// Beam dump initialisation
	n_bd_det		= config->GetValue( "NumberOfBeamDumpDetectors", 1 );
	
	// SPEDE initialisation
	//n_spede_seg		= config->GetValue( "NumberOfSpedeSegments", 24 );
	n_spede_seg		= config->GetValue( "NumberOfSpedeSegments", 0 );
	
	// IonChamber initialisation
	n_ic_layer		= config->GetValue( "NumberOfIonChamberLayers", 2 );
	
	// Pulser events
	n_pulsers		= config->GetValue( "NumberOfPulsers", 2 );
	if( n_pulsers > 10 ) {
		
		std::cout << "Number of pulsers limited to 10" << std::endl;
		n_pulsers = 10;
		
	}
	
	// Info code initialisation
	pause_code		= 2;
	resume_code		= 3;
	sync_code		= 7;
	thsb_code		= 8;
	pulser_code		= 30;	// code for first channel, others are pulser_code+id
	ebis_sfp		= config->GetValue( "EBIS.Sfp", 1 );
	ebis_board		= config->GetValue( "EBIS.Board", 10 );
	ebis_ch			= config->GetValue( "EBIS.Channel", 0 ); // ch 0 if in DGF too!
	ebis_dgf		= config->GetValue( "EBIS.Dgf", 53 );
	ebis_code		= 21;
	t1_sfp			= config->GetValue( "T1.Sfp", 1 );
	t1_board		= config->GetValue( "T1.Board", 10 );
	t1_ch			= config->GetValue( "T1.Channel", 2 );
	t1_dgf			= config->GetValue( "T1.Dgf", 53 );
	t1_code			= 22;
	sc_sfp			= config->GetValue( "SC.Sfp", 1 );
	sc_board		= config->GetValue( "SC.Board", 10 );
	sc_ch			= config->GetValue( "SC.Channel", 4 );
	sc_dgf			= config->GetValue( "SC.Dgf", 53 );
	sc_code			= 23;
	laser_sfp		= config->GetValue( "RILIS.Sfp", 1 );
	laser_board		= config->GetValue( "RILIS.Board", 10 );
	laser_ch		= config->GetValue( "RILIS.Channel", 6 );
	laser_pattern	= config->GetValue( "RILIS.Pattern", 8 ); // channel for laser status
	laser_code		= 24;

	
	// Event builder
	event_window	= config->GetValue( "EventWindow", 3e3 );
	mbs_event_sort	= config->GetValue( "MbsEventSort", true );

	// Hit windows for complex events
	mb_hit_window	= config->GetValue( "MiniballCrystalHitWindow", 400. );
	ab_hit_window	= config->GetValue( "MiniballAddbackHitWindow", 400. );
	cd_hit_window	= config->GetValue( "CDHitWindow", 150. );
	pad_hit_window	= config->GetValue( "PadHitWindow", 150. );
	ic_hit_window	= config->GetValue( "IonChamberHitWindow", 500. );

	
	// Data things
	block_size			= config->GetValue( "DataBlockSize", 0x10000 );
	flag_febex_only		= config->GetValue( "FebexOnlyData", true );

	
	// Pileup rejection
	pileup_reject		= config->GetValue( "PileUpRejection", false );
	
	
	// Buffer full rejection
	bufferfull_reject	= config->GetValue( "BufferFullRejection", false );
	bufferpart_reject	= config->GetValue( "BufferPartRejection", false );

	
	// Electronics mapping
	// Will depend on whether we have FEBEX or DGF/ADC combo
	unsigned int i_size		= n_febex_sfp;
	unsigned int j_size_dgf	= n_febex_board;
	unsigned int j_size_adc	= n_febex_board;
	unsigned int k_size_dgf	= n_febex_ch;
	unsigned int k_size_adc	= n_febex_ch;

	// We don't have FEBEX if any of the SFP/board/ch is = 0
	// but for confidence, check that we have some ADCs or DGFs
	if( ( !n_febex_sfp || !n_febex_board || !n_febex_ch ) &&
		( n_madc_mod || n_caen_mod || n_dgf_mod ) ) {
		
		i_size		= 1;
		j_size_dgf	= GetNumberOfDgfModules();
		j_size_adc	= GetNumberOfAdcModules();
		k_size_dgf	= GetNumberOfDgfChannels();
		k_size_adc	= GetMaximumNumberOfAdcChannels();

	}
		
	// Resize it all
	mb_cluster.resize( i_size );
	mb_crystal.resize( i_size );
	mb_segment.resize( i_size );
	cd_det.resize( i_size );
	cd_sector.resize( i_size );
	cd_side.resize( i_size );
	cd_strip.resize( i_size );
	pad_det.resize( i_size );
	pad_sector.resize( i_size );
	bd_det.resize( i_size );
	spede_seg.resize( i_size );
	ic_layer.resize( i_size );
	pulser.resize( i_size );

	for( unsigned int i = 0; i < i_size; ++i ){

		mb_cluster[i].resize( j_size_dgf );
		mb_crystal[i].resize( j_size_dgf );
		mb_segment[i].resize( j_size_dgf );
		bd_det[i].resize( j_size_dgf );
		spede_seg[i].resize( j_size_dgf );
		pulser[i].resize( j_size_dgf );

		// DGFs or FEBEX
		for( unsigned int j = 0; j < j_size_dgf; ++j ){

			mb_cluster[i][j].resize( k_size_dgf );
			mb_crystal[i][j].resize( k_size_dgf );
			mb_segment[i][j].resize( k_size_dgf );
			bd_det[i][j].resize( k_size_dgf );
			spede_seg[i][j].resize( k_size_dgf );
			pulser[i][j].resize( k_size_dgf );

			for( unsigned int k = 0; k < k_size_dgf; ++k ){

				mb_cluster[i][j][k]	= -1;
				mb_crystal[i][j][k]	= -1;
				mb_segment[i][j][k]	= -1;
				bd_det[i][j][k]		= -1;
				spede_seg[i][j][k]	= -1;
				pulser[i][j][k]		= -1;

			} // k: febex/dgf ch
			
		} // j: febex/dgf board
		
		cd_det[i].resize( j_size_adc );
		cd_sector[i].resize( j_size_adc );
		cd_side[i].resize( j_size_adc );
		cd_strip[i].resize( j_size_adc );
		pad_det[i].resize( j_size_adc );
		pad_sector[i].resize( j_size_adc );
		ic_layer[i].resize( j_size_adc );

		// ADCs or FEBEX
		for( unsigned int j = 0; j < j_size_adc; ++j ){

			cd_det[i][j].resize( k_size_adc );
			cd_sector[i][j].resize( k_size_adc );
			cd_side[i][j].resize( k_size_adc );
			cd_strip[i][j].resize( k_size_adc );
			pad_det[i][j].resize( k_size_adc );
			pad_sector[i][j].resize( k_size_adc );
			ic_layer[i][j].resize( k_size_adc );

			for( unsigned int k = 0; k < k_size_adc; ++k ){

				cd_det[i][j][k]		= -1;
				cd_sector[i][j][k]	= -1;
				cd_side[i][j][k]	= -1;
				cd_strip[i][j][k]	= -1;
				pad_det[i][j][k]	= -1;
				pad_sector[i][j][k]	= -1;
				ic_layer[i][j][k]	= -1;

			} // k: febex/adc ch
			
		} // j: febex/adc board

	} // i: febex sfp
	
	
	// Miniball array electronics mapping
	int d, s, b, c, g;
	mb_sfp.resize( n_mb_cluster );
	mb_board.resize( n_mb_cluster );
	mb_dgf.resize( n_mb_cluster );
	mb_ch.resize( n_mb_cluster );
	
	for( unsigned int i = 0; i < n_mb_cluster; ++i ){

		mb_sfp[i].resize( n_mb_crystal );
		mb_board[i].resize( n_mb_crystal );
		mb_dgf[i].resize( n_mb_crystal );
		mb_ch[i].resize( n_mb_crystal );

		for( unsigned int j = 0; j < n_mb_crystal; ++j ){

			mb_sfp[i][j].resize( n_mb_segment );
			mb_board[i][j].resize( n_mb_segment );
			mb_dgf[i][j].resize( n_mb_segment );
			mb_ch[i][j].resize( n_mb_segment );

			for( unsigned int k = 0; k < n_mb_segment; ++k ){

				d = i*3 + j;			// Crystal ordering: 0-23
				s = 0;					// spread 24 crystals over 1 SFPs
				b = d/2;				// 2 crystals per board
				c = k + 9*(d&0x1);		// odd crystals starts at ch9
				g = d*2;				// dgf number is 2 * detector number
				if( k > 2 ) g++;		// but add one for segments 3-6
				mb_sfp[i][j][k]		= config->GetValue( Form( "Miniball_%d_%d_%d.Sfp", i, j, k ), s );
				mb_board[i][j][k]	= config->GetValue( Form( "Miniball_%d_%d_%d.Board", i, j, k ), b );
				mb_dgf[i][j][k]		= config->GetValue( Form( "Miniball_%d_%d_%d.Dgf", i, j, k ), g );
				mb_ch[i][j][k]		= config->GetValue( Form( "Miniball_%d_%d_%d.Channel", i, j, k ), c );

				// Check FEBEX inputs aren't overlapped
				if( mb_sfp[i][j][k] < n_febex_sfp && n_febex_sfp > 0 &&
					mb_board[i][j][k] < n_febex_board && n_febex_board > 0 &&
					mb_ch[i][j][k] < n_febex_ch && n_febex_ch > 0 ){
					
					if( mb_cluster[mb_sfp[i][j][k]][mb_board[i][j][k]][mb_ch[i][j][k]] >= 0 ||
					    mb_crystal[mb_sfp[i][j][k]][mb_board[i][j][k]][mb_ch[i][j][k]] >= 0 ||
					    mb_segment[mb_sfp[i][j][k]][mb_board[i][j][k]][mb_ch[i][j][k]] >= 0 ) {
						
						std::cout << "You have defined two Miniball detectors in the same channel:" << std::endl;
						std::cout << "\tMiniball_" << mb_cluster[mb_sfp[i][j][k]][mb_board[i][j][k]][mb_ch[i][j][k]] << "_";
						std::cout << mb_crystal[mb_sfp[i][j][k]][mb_board[i][j][k]][mb_ch[i][j][k]] << "_";
						std::cout << mb_segment[mb_sfp[i][j][k]][mb_board[i][j][k]][mb_ch[i][j][k]] << " and ";
						std::cout << "Miniball_" << i << "_" << j << "_" << k << std::endl;
						
					}
					
					else {
						
						mb_cluster[mb_sfp[i][j][k]][mb_board[i][j][k]][mb_ch[i][j][k]] = i;
						mb_crystal[mb_sfp[i][j][k]][mb_board[i][j][k]][mb_ch[i][j][k]] = j;
						mb_segment[mb_sfp[i][j][k]][mb_board[i][j][k]][mb_ch[i][j][k]] = k;
						
					}
					
				}
				
				// Check DGF inputs aren't overlapped
				else if( mb_dgf[i][j][k] < n_dgf_mod && n_dgf_mod > 0 &&
						 mb_ch[i][j][k] < n_dgf_ch && n_dgf_ch > 0 ){
					
					if( mb_cluster[0][mb_dgf[i][j][k]][mb_ch[i][j][k]] >= 0 ||
					    mb_crystal[0][mb_dgf[i][j][k]][mb_ch[i][j][k]] >= 0 ||
					    mb_segment[0][mb_dgf[i][j][k]][mb_ch[i][j][k]] >= 0 ) {
						
						std::cout << "You have defined two Miniball detectors in the same channel:" << std::endl;
						std::cout << "\tMiniball_" << mb_cluster[0][mb_dgf[i][j][k]][mb_ch[i][j][k]] << "_";
						std::cout << mb_crystal[0][mb_dgf[i][j][k]][mb_ch[i][j][k]] << "_";
						std::cout << mb_segment[0][mb_dgf[i][j][k]][mb_ch[i][j][k]] << " and ";
						std::cout << "Miniball_" << i << "_" << j << "_" << k << std::endl;
						
					}
					
					else {
						
						mb_cluster[0][mb_dgf[i][j][k]][mb_ch[i][j][k]] = i;
						mb_crystal[0][mb_dgf[i][j][k]][mb_ch[i][j][k]] = j;
						mb_segment[0][mb_dgf[i][j][k]][mb_ch[i][j][k]] = k;
						
					}

					
				}
				
				else {
					
					std::cerr << "Dodgy Miniball settings: sfp = " << mb_sfp[i][j][k];
					if( n_febex_board ) std::cerr << ", board = " << mb_board[i][j][k];
					if( n_dgf_mod ) std::cerr << ", dgf = " << mb_dgf[i][j][k];
					std::cerr << ", channel = " << mb_ch[i][j][k] << std::endl;

				}

			} // k: mb segment

		} // j: mb crystal

	} // i: mb cluster
	

	// CD detector electronics mapping
	unsigned int side_size;
	std::string side_str;
	cd_sfp.resize( n_cd_det );
	cd_board.resize( n_cd_det );
	cd_adc.resize( n_cd_det );
	cd_ch.resize( n_cd_det );
	
	for( unsigned int i = 0; i < n_cd_det; ++i ){
		
		cd_sfp[i].resize( n_cd_sector );
		cd_board[i].resize( n_cd_sector );
		cd_adc[i].resize( n_cd_sector );
		cd_ch[i].resize( n_cd_sector );
		
		for( unsigned int j = 0; j < n_cd_sector; ++j ){
			
			cd_sfp[i][j].resize( n_cd_side );
			cd_board[i][j].resize( n_cd_side );
			cd_adc[i][j].resize( n_cd_side );
			cd_ch[i][j].resize( n_cd_side );
			
			for( unsigned int k = 0; k < n_cd_side; ++k ){
				
				// p or n side?
				if( k == 0 ) side_str = "P";
				else side_str = "N";

				// p or n side?
				if( k == 0 ) side_size = n_cd_pstrip;
				else side_size = n_cd_nstrip;

				cd_sfp[i][j][k].resize( side_size );
				cd_board[i][j][k].resize( side_size );
				cd_adc[i][j][k].resize( side_size );
				cd_ch[i][j][k].resize( side_size );

				for( unsigned int l = 0; l < side_size; ++l ){
					
					s = 1;			// sfp number - all in SFP 1
					b = j*2 + k;	// boards go 0-7
					c = l;
					g = i;			// adc number is same as segment by default
					cd_sfp[i][j][k][l]		= config->GetValue( Form( "CD_%d_%d_%d.%s.Sfp", i, j, l, side_str.data() ), s );
					cd_board[i][j][k][l]	= config->GetValue( Form( "CD_%d_%d_%d.%s.Board", i, j, l, side_str.data() ), b );
					cd_adc[i][j][k][l]		= config->GetValue( Form( "CD_%d_%d_%d.%s.Adc", i, j, l, side_str.data() ), g );
					cd_ch[i][j][k][l]		= config->GetValue( Form( "CD_%d_%d_%d.%s.Channel", i, j, l, side_str.data() ), c );
					
					if( cd_sfp[i][j][k][l] < n_febex_sfp && n_febex_sfp > 0 &&
					    cd_board[i][j][k][l] < n_febex_board && n_febex_board > 0 &&
					    cd_ch[i][j][k][l] < n_febex_ch && n_febex_ch > 0 ){
						
						if( cd_det[cd_sfp[i][j][k][l]][cd_board[i][j][k][l]][cd_ch[i][j][k][l]] >= 0 ||
						    cd_sector[cd_sfp[i][j][k][l]][cd_board[i][j][k][l]][cd_ch[i][j][k][l]] >= 0 ||
						    cd_side[cd_sfp[i][j][k][l]][cd_board[i][j][k][l]][cd_ch[i][j][k][l]] >= 0 ||
						    cd_strip[cd_sfp[i][j][k][l]][cd_board[i][j][k][l]][cd_ch[i][j][k][l]] >= 0 ) {
							
							std::cout << "You have defined two CD detector strips in the same channel:" << std::endl;
							std::cout << "\tCD_" << cd_det[cd_sfp[i][j][k][l]][cd_board[i][j][k][l]][cd_ch[i][j][k][l]] << "_";
							std::cout << cd_sector[cd_sfp[i][j][k][l]][cd_board[i][j][k][l]][cd_ch[i][j][k][l]] << "_";
							std::cout << cd_strip[cd_sfp[i][j][k][l]][cd_board[i][j][k][l]][cd_ch[i][j][k][l]];
							if( k == 0 ) std::cout << ".P and" << std::endl;
							else std::cout << ".N and CD_" << std::endl;
							std::cout << i << "_" << j << "_" << l;
							if( k == 0 ) std::cout << ".P" << std::endl;
							else std::cout << ".N" << std::endl;
									
						}
						
						else {

							cd_det[cd_sfp[i][j][k][l]][cd_board[i][j][k][l]][cd_ch[i][j][k][l]] = i;
							cd_sector[cd_sfp[i][j][k][l]][cd_board[i][j][k][l]][cd_ch[i][j][k][l]] = j;
							cd_side[cd_sfp[i][j][k][l]][cd_board[i][j][k][l]][cd_ch[i][j][k][l]] = k;
							cd_strip[cd_sfp[i][j][k][l]][cd_board[i][j][k][l]][cd_ch[i][j][k][l]] = l;
							
						}
						
					}
					
					else if( cd_adc[i][j][k][l] < GetNumberOfAdcModules() && GetNumberOfAdcModules() > 0 &&
							 cd_ch[i][j][k][l] < GetMaximumNumberOfAdcChannels() && GetMaximumNumberOfAdcChannels() > 0 ) {
						
						
						if( cd_det[0][cd_adc[i][j][k][l]][cd_ch[i][j][k][l]] >= 0 ||
						    cd_sector[0][cd_adc[i][j][k][l]][cd_ch[i][j][k][l]] >= 0 ||
						    cd_side[0][cd_adc[i][j][k][l]][cd_ch[i][j][k][l]] >= 0 ||
						    cd_strip[0][cd_adc[i][j][k][l]][cd_ch[i][j][k][l]] >= 0 ) {
							
							std::cout << "You have defined two CD detector strips in the same channel:" << std::endl;
							std::cout << "\tCD_" << cd_det[0][cd_adc[i][j][k][l]][cd_ch[i][j][k][l]] << "_";
							std::cout << cd_sector[0][cd_adc[i][j][k][l]][cd_ch[i][j][k][l]] << "_";
							std::cout << cd_strip[0][cd_adc[i][j][k][l]][cd_ch[i][j][k][l]];
							if( k == 0 ) std::cout << ".P and" << std::endl;
							else std::cout << ".N and CD_" << std::endl;
							std::cout << i << "_" << j << "_" << l;
							if( k == 0 ) std::cout << ".P" << std::endl;
							else std::cout << ".N" << std::endl;
									
						}
						
						else {

							cd_det[0][cd_adc[i][j][k][l]][cd_ch[i][j][k][l]] = i;
							cd_sector[0][cd_adc[i][j][k][l]][cd_ch[i][j][k][l]] = j;
							cd_side[0][cd_adc[i][j][k][l]][cd_ch[i][j][k][l]] = k;
							cd_strip[0][cd_adc[i][j][k][l]][cd_ch[i][j][k][l]] = l;
							
						}

					}
					
					else {
						
						std::cerr << "Dodgy CD settings: sfp = " << cd_sfp[i][j][k][l];
						if( n_febex_board > 0 ) std::cerr << ", board = " << cd_board[i][j][k][l];
						if( GetNumberOfAdcModules() > 0 ) std::cerr << ", board = " << cd_adc[i][j][k][l];
						std::cerr << ", channel = " << cd_ch[i][j][k][l] << std::endl;
						
					}
					
				} // l: cd strips
				
			} // k: cd side
			
		} // j: cd sector
		
	} // i: cd detector
	
	
	// PAD detector electronics mapping
	pad_sfp.resize( n_cd_det );
	pad_board.resize( n_cd_det );
	pad_adc.resize( n_cd_det );
	pad_ch.resize( n_cd_det );
	
	for( unsigned int i = 0; i < n_cd_det; ++i ){

		pad_sfp[i].resize( n_cd_sector );
		pad_board[i].resize( n_cd_sector );
		pad_adc[i].resize( n_cd_sector );
		pad_ch[i].resize( n_cd_sector );
		
		for( unsigned int j = 0; j < n_cd_sector; ++j ){
			
			s = 1;
			b = 8;
			c = i*n_cd_sector+j;
			g = i;
			pad_sfp[i][j]	= config->GetValue( Form( "Pad_%d_%d.Sfp", i, j ), s );
			pad_board[i][j]	= config->GetValue( Form( "Pad_%d_%d.Board", i, j ), b );
			pad_adc[i][j]	= config->GetValue( Form( "Pad_%d_%d.Adc", i, j ), g );
			pad_ch[i][j]	= config->GetValue( Form( "Pad_%d_%d.Channel", i, j ), c );
			
			if( pad_sfp[i][j] < n_febex_sfp && n_febex_sfp > 0 &&
			    pad_board[i][j] < n_febex_board && n_febex_board > 0 &&
			    pad_ch[i][j] < n_febex_ch && n_febex_ch > 0 ){
				
				if( pad_det[pad_sfp[i][j]][pad_board[i][j]][pad_ch[i][j]] >= 0 ||
				    pad_sector[pad_sfp[i][j]][pad_board[i][j]][pad_ch[i][j]] >= 0 ) {
					
					std::cout << "You have defined two Pad detectors in the same channel:" << std::endl;
					std::cout << "\tPad_" << pad_det[pad_sfp[i][j]][pad_board[i][j]][pad_ch[i][j]] << "_";
					std::cout << pad_sector[pad_sfp[i][j]][pad_board[i][j]][pad_ch[i][j]] << "and";
					std::cout << "Pad_" << i << "_" << j << std::endl;
							
				}
				
				else {
					
					pad_det[pad_sfp[i][j]][pad_board[i][j]][pad_ch[i][j]] = i;
					pad_sector[pad_sfp[i][j]][pad_board[i][j]][pad_ch[i][j]] = j;
					
				}
				
			}
			
			else if( pad_adc[i][j] < GetNumberOfAdcModules() && GetNumberOfAdcModules() > 0 &&
					 pad_ch[i][j] < GetMaximumNumberOfAdcChannels() && GetMaximumNumberOfAdcChannels() > 0 ){
				
				if( pad_det[0][pad_adc[i][j]][pad_ch[i][j]] >= 0 ||
				    pad_sector[0][pad_adc[i][j]][pad_ch[i][j]] >= 0 ) {
					
					std::cout << "You have defined two Pad detectors in the same channel:" << std::endl;
					std::cout << "\tPad_" << pad_det[0][pad_adc[i][j]][pad_ch[i][j]] << "_";
					std::cout << pad_sector[0][pad_adc[i][j]][pad_ch[i][j]] << "and";
					std::cout << "Pad_" << i << "_" << j << std::endl;
							
				}
				
				else {
					
					pad_det[0][pad_adc[i][j]][pad_ch[i][j]] = i;
					pad_sector[0][pad_adc[i][j]][pad_ch[i][j]] = j;
					
				}
				
			}
			
			else {
				
				std::cerr << "Dodgy PAD settings: sfp = " << pad_sfp[i][j];
				if( n_febex_board > 0 ) std::cerr << ", board = " << pad_board[i][j];
				if( GetNumberOfAdcModules() > 0 ) std::cerr << ", adc = " << pad_adc[i][j];
				std::cerr << ", channel = " << pad_ch[i][j] << std::endl;

			}
			
		} // j: cd sector
	
	} // i: cd detector

	
	// Beam dump detector mapping
	bd_sfp.resize( n_bd_det );
	bd_board.resize( n_bd_det );
	bd_dgf.resize( n_bd_det );
	bd_ch.resize( n_bd_det );
	
	for( unsigned int i = 0; i < n_bd_det; ++i ){
		
		bd_sfp[i]		= config->GetValue( Form( "BeamDump_%d.Sfp", i ), 1 );
		bd_board[i]		= config->GetValue( Form( "BeamDump_%d.Board", i ), 10 );
		bd_dgf[i]		= config->GetValue( Form( "BeamDump_%d.Dgf", i ), 54 );
		bd_ch[i]		= config->GetValue( Form( "BeamDump_%d.Channel", i ), (int)(i*2+1) );
		
		if( bd_sfp[i] < n_febex_sfp && n_febex_sfp > 0 &&
		    bd_board[i] < n_febex_board && n_febex_board > 0 &&
		    bd_ch[i] < n_febex_ch && n_febex_ch > 0 ){
			
			if( bd_det[bd_sfp[i]][bd_board[i]][bd_ch[i]] >= 0 ){
				
				std::cout << "You have defined two beam-dump detectors in the same channel:" << std::endl;
				std::cout << "\tBeamDump_" << bd_det[bd_sfp[i]][bd_board[i]][bd_ch[i]] << " and ";
				std::cout << "BeamDump_" << i << std::endl;
				
			}
			
			else {
			
				bd_det[bd_sfp[i]][bd_board[i]][bd_ch[i]] = i;
			
			}
			
		}
		
		else if( bd_dgf[i] < n_dgf_mod && n_dgf_mod > 0 &&
				 bd_ch[i] < n_dgf_ch && n_dgf_ch > 0 ){
			
			if( bd_det[0][bd_dgf[i]][bd_ch[i]] >= 0 ){
				
				std::cout << "You have defined two beam-dump detectors in the same channel:" << std::endl;
				std::cout << "\tBeamDump_" << bd_det[0][bd_dgf[i]][bd_ch[i]] << " and ";
				std::cout << "BeamDump_" << i << std::endl;
				
			}
			
			else {
			
				bd_det[0][bd_dgf[i]][bd_ch[i]] = i;
			
			}
			
		}
		
		else {
			
			std::cerr << "Dodgy beam-dump settings: sfp = " << bd_sfp[i];
			if( n_febex_board > 0 ) std::cerr << ", board = " << bd_board[i];
			if( n_dgf_mod > 0 )std::cerr << ", dgf = " << bd_dgf[i];
			std::cerr << ", channel = " << bd_ch[i] << std::endl;
			
		}

	} // i: beam dump detector
	
	
	// SPEDE detector mapping
	spede_sfp.resize( n_spede_seg );
	spede_board.resize( n_spede_seg );
	spede_ch.resize( n_spede_seg );
	
	for( unsigned int i = 0; i < n_spede_seg; ++i ){
		
		s = 1;
		if( i < 16 ){
			b = 8;
			c = i;
		}
		else {
			b = 9;
			c = i-16;
		}
		
		spede_sfp[i]	= config->GetValue( Form( "Spede_%d.Sfp", i ), s );
		spede_board[i]	= config->GetValue( Form( "Spede_%d.Board", i ), b );
		spede_ch[i]		= config->GetValue( Form( "Spede_%d.Channel", i ), c );
		
		if( spede_sfp[i] < n_febex_sfp &&
		    spede_board[i] < n_febex_board &&
		    spede_ch[i] < n_febex_ch ){
			
			if( spede_seg[spede_sfp[i]][spede_board[i]][spede_ch[i]] >= 0 ){
				
				std::cout << "You have defined two SPEDE segments in the same channel:" << std::endl;
				std::cout << "\tSpede_" << spede_seg[spede_sfp[i]][spede_board[i]][spede_ch[i]] << " and ";
				std::cout << "Spede_" << i << std::endl;
				
			}
			
			else {
			
				spede_seg[spede_sfp[i]][spede_board[i]][spede_ch[i]] = i;
			
			}
			
		}
		
		else {
			
			std::cerr << "Dodgy SPEDE settings: sfp = " << spede_sfp[i];
			std::cerr << ", board = " << spede_board[i];
			std::cerr << ", channel = " << spede_ch[i] << std::endl;
			
		}
		
		
	} // i: SPEDE detector
	
	// IonChamber detector mapping
	ic_sfp.resize( n_ic_layer );
	ic_board.resize( n_ic_layer );
	ic_adc.resize( n_ic_layer );
	ic_ch.resize( n_ic_layer );
	
	for( unsigned int i = 0; i < n_ic_layer; ++i ){
		
		ic_sfp[i]		= config->GetValue( Form( "IonChamber_%d.Sfp", i ), 1 );
		ic_board[i]		= config->GetValue( Form( "IonChamber_%d.Board", i ), 10 );
		ic_adc[i]		= config->GetValue( Form( "IonChamber_%d.Adc", i ), 4 );
		ic_ch[i]		= config->GetValue( Form( "IonChamber_%d.Channel", i ), (int)((i+n_bd_det)*2+1) );
		
		if( ic_sfp[i] < n_febex_sfp && n_febex_sfp > 0 &&
		    ic_board[i] < n_febex_board && n_febex_board > 0 &&
		    ic_ch[i] < n_febex_ch && n_febex_ch > 0 ){
			
			if( ic_layer[ic_sfp[i]][ic_board[i]][ic_ch[i]] >= 0 ){
				
				std::cout << "You have defined two IonChamber elements in the same channel:" << std::endl;
				std::cout << "\tIonChamber_" << ic_layer[ic_sfp[i]][ic_board[i]][ic_ch[i]] << " and ";
				std::cout << "IonChamber_" << i << std::endl;
				
			}
			
			else {
				
				ic_layer[ic_sfp[i]][ic_board[i]][ic_ch[i]] = i;
				
			}
			
		}
		
		else if( ic_adc[i] < GetNumberOfAdcModules() && GetNumberOfAdcModules() > 0 &&
				 ic_ch[i] < GetMaximumNumberOfAdcChannels() && GetMaximumNumberOfAdcChannels() > 0 ){
			
			if( ic_layer[0][ic_adc[i]][ic_ch[i]] >= 0 ){
				
				std::cout << "You have defined two IonChamber elements in the same channel:" << std::endl;
				std::cout << "\tIonChamber_" << ic_layer[0][ic_adc[i]][ic_ch[i]] << " and ";
				std::cout << "IonChamber_" << i << std::endl;
				
			}
			
			else {
				
				ic_layer[0][ic_adc[i]][ic_ch[i]] = i;
				
			}
			
		}
		
		else {
			
			std::cerr << "Dodgy IonChamber settings: sfp = " << ic_sfp[i];
			if( n_febex_board > 0 ) std::cerr << ", board = " << ic_board[i];
			if( GetNumberOfAdcModules() > 0 ) std::cerr << ", adc = " << ic_adc[i];
			std::cerr << ", channel = " << ic_ch[i] << std::endl;
			
		}
		
		
	} // i: IonChamber detector
	
	
	// Pulser input mapping
	pulser_sfp.resize( n_pulsers );
	pulser_board.resize( n_pulsers );
	pulser_ch.resize( n_pulsers );
	
	for( unsigned int i = 0; i < n_pulsers; ++i ){
		
		pulser_sfp[i]		= config->GetValue( Form( "Pulser_%d.Sfp", i ), (int)i );
		pulser_board[i]		= config->GetValue( Form( "Pulser_%d.Board", i ), 15 );
		pulser_ch[i]		= config->GetValue( Form( "Pulser_%d.Channel", i ), 15 );
		
		if( pulser_sfp[i] < n_febex_sfp &&
		    pulser_board[i] < n_febex_board &&
		    pulser_ch[i] < n_febex_ch ){
			
			if( pulser[pulser_sfp[i]][pulser_board[i]][pulser_ch[i]] >= 0 ){
				
				std::cout << "You have defined two Sync Pulsers in the same channel:" << std::endl;
				std::cout << "\tPulser_" << pulser[pulser_sfp[i]][pulser_board[i]][pulser_ch[i]] << " and ";
				std::cout << "Pulser_" << i << std::endl;
				
			}
			
			else {
				
				pulser[pulser_sfp[i]][pulser_board[i]][pulser_ch[i]] = i;

			}
			
		}
		
		else {
			
			std::cerr << "Dodgy pulser settings: sfp = " << pulser_sfp[i];
			std::cerr << ", board = " << pulser_board[i];
			std::cerr << ", channel = " << pulser_ch[i] << std::endl;
			
		}
		
	} // i: pulser inputs

	// Finished
	delete config;
	
}


bool MiniballSettings::IsMiniball( unsigned int dgf, unsigned int ch ) {
	
	/// Return true if this is a Miniball event
	if( dgf < n_dgf_mod && ch < n_dgf_ch ) {
		
		if( mb_cluster[0][dgf][ch] >= 0 ) return true;
		else return false;
	}
	
	else {
		
		std::cerr << "Bad Miniball event: dgf = " << dgf;
		std::cerr << ", channel = " << ch << std::endl;
		return false;
		
	}

}

bool MiniballSettings::IsMiniball( unsigned int sfp, unsigned int board, unsigned int ch ) {
	
	/// Return true if this is a Miniball event
	if( sfp < n_febex_sfp && board < n_febex_board && ch < n_febex_ch ) {
		
		if( mb_cluster[sfp][board][ch] >= 0 ) return true;
		else return false;
	}
	
	else {
		
		std::cerr << "Bad Miniball event: sfp = " << sfp;
		std::cerr << ", board = " << board;
		std::cerr << ", channel = " << ch << std::endl;
		return false;
		
	}

}

int MiniballSettings::GetMiniballID( unsigned int dgf, unsigned int ch,
							 std::vector<std::vector<std::vector<int>>> vector ) {
	
	/// Return the Miniball ID by the DGF module number and Channel number
	if( dgf < n_dgf_mod && ch < n_dgf_ch )
		return vector[0][dgf][ch];
	
	else {
		
		std::cerr << "Bad Miniball event: dgf = " << dgf;
		std::cerr << ", channel = " << ch << std::endl;
		return 0;
		
	}
	
}

int MiniballSettings::GetMiniballID( unsigned int sfp, unsigned int board, unsigned int ch,
							 std::vector<std::vector<std::vector<int>>> vector ) {
	
	/// Return the Miniball ID by the FEBEX SFP, Board number and Channel number
	if( sfp < n_febex_sfp && board < n_febex_board && ch < n_febex_ch )
		return vector[sfp][board][ch];
	
	else {
		
		std::cerr << "Bad Miniball event: sfp = " << sfp;
		std::cerr << ", board = " << board;
		std::cerr << ", channel = " << ch << std::endl;
		return 0;
		
	}
	
}


bool MiniballSettings::IsCD( unsigned int adc, unsigned int ch ) {
	
	/// Return true if this is a CD event
	if( adc < GetNumberOfAdcModules() && ch < GetMaximumNumberOfAdcChannels() ) {

		if( cd_det[0][adc][ch] >= 0 ) return true;
		else return false;
		
	}

	else {
		
		std::cerr << "Bad CD event: adc = " << adc;
		std::cerr << ", channel = " << ch << std::endl;
		return false;
		
	}

}

bool MiniballSettings::IsCD( unsigned int sfp, unsigned int board, unsigned int ch ) {
	
	/// Return true if this is a CD event
	if( sfp < n_febex_sfp && board < n_febex_board && ch < n_febex_ch ) {
		
		if( cd_det[sfp][board][ch] >= 0 ) return true;
		else return false;
		
	}

	else {
		
		std::cerr << "Bad CD event: sfp = " << sfp;
		std::cerr << ", board = " << board;
		std::cerr << ", channel = " << ch << std::endl;
		return false;
		
	}

}

int MiniballSettings::GetCDID( unsigned int adc, unsigned int ch,
					  std::vector<std::vector<std::vector<int>>> vector ) {
	
	/// Return the CD ID by the FEBEX SFP, Board number and Channel number
	if( adc < GetNumberOfAdcModules() && ch < GetMaximumNumberOfAdcChannels() )
		return vector[0][adc][ch];
	
	else {
		
		std::cerr << "Bad CD event: sfp = " << adc;
		std::cerr << ", channel = " << ch << std::endl;
		return 0;
		
	}
	
}

int MiniballSettings::GetCDID( unsigned int sfp, unsigned int board, unsigned int ch,
					  std::vector<std::vector<std::vector<int>>> vector ) {
	
	/// Return the CD ID by the FEBEX SFP, Board number and Channel number
	if( sfp < n_febex_sfp && board < n_febex_board && ch < n_febex_ch )
		return vector[sfp][board][ch];
	
	else {
		
		std::cerr << "Bad CD event: sfp = " << sfp;
		std::cerr << ", board = " << board;
		std::cerr << ", channel = " << ch << std::endl;
		return 0;
		
	}
	
}

bool MiniballSettings::IsPad( unsigned int adc, unsigned int ch ) {
	
	/// Return true if this is a Pad event
	if( adc < GetNumberOfAdcModules() && ch < GetMaximumNumberOfAdcChannels() ) {
		
		if( pad_det[0][adc][ch] >= 0 ) return true;
		else return false;
		
	}
	
	else {
		
		std::cerr << "Bad Pad event: adc = " << adc;
		std::cerr << ", channel = " << ch << std::endl;
		return false;
		
	}
	
}

bool MiniballSettings::IsPad( unsigned int sfp, unsigned int board, unsigned int ch ) {
	
	/// Return true if this is a Pad event
	if( sfp < n_febex_sfp && board < n_febex_board && ch < n_febex_ch ) {
		
		if( pad_det[sfp][board][ch] >= 0 ) return true;
		else return false;
		
	}
	
	else {
		
		std::cerr << "Bad Pad event: sfp = " << sfp;
		std::cerr << ", board = " << board;
		std::cerr << ", channel = " << ch << std::endl;
		return false;
		
	}
	
}

int MiniballSettings::GetPadDetector( unsigned int adc, unsigned int ch ) {
	
	/// Return the Pad detector by the ADC module  number and Channel number
	if( adc < GetNumberOfAdcModules() && ch < GetMaximumNumberOfAdcChannels() )
		return pad_det[0][adc][ch];
	
	else {
		
		std::cerr << "Bad Pad event: adc = " << adc;
		std::cerr << ", channel = " << ch << std::endl;
		return false;
		
	}
	
}

int MiniballSettings::GetPadDetector( unsigned int sfp, unsigned int board, unsigned int ch ) {
	
	/// Return the Pad detector by the FEBEX SFP, Board number and Channel number
	if( sfp < n_febex_sfp && board < n_febex_board && ch < n_febex_ch )
		return pad_det[sfp][board][ch];
	
	else {
		
		std::cerr << "Bad Pad event: sfp = " << sfp;
		std::cerr << ", board = " << board;
		std::cerr << ", channel = " << ch << std::endl;
		return false;
		
	}
	
}

int MiniballSettings::GetPadSector( unsigned int adc, unsigned int ch ) {
	
	/// Return the Pad sector by the ADC module number and Channel number
	if( adc < GetNumberOfAdcModules() && ch < GetMaximumNumberOfAdcChannels() )
		return pad_sector[0][adc][ch];
	
	else {
		
		std::cerr << "Bad Pad event: adc = " << adc;
		std::cerr << ", channel = " << ch << std::endl;
		return 0;
		
	}
	
}

int MiniballSettings::GetPadSector( unsigned int sfp, unsigned int board, unsigned int ch ) {
	
	/// Return the Pad sector by the FEBEX SFP, Board number and Channel number
	if( sfp < n_febex_sfp && board < n_febex_board && ch < n_febex_ch )
		return pad_sector[sfp][board][ch];
	
	else {
		
		std::cerr << "Bad Pad event: sfp = " << sfp;
		std::cerr << ", board = " << board;
		std::cerr << ", channel = " << ch << std::endl;
		return 0;
		
	}
	
}

bool MiniballSettings::IsBeamDump( unsigned int dgf, unsigned int ch ) {
	
	/// Return true if this is a beam dump event
	if( GetBeamDumpDetector(dgf,ch) >= 0 ) return true;
	else return false;
	
}

bool MiniballSettings::IsBeamDump( unsigned int sfp, unsigned int board, unsigned int ch ) {
	
	/// Return true if this is a beam dump event
	if( GetBeamDumpDetector(sfp,board,ch) >= 0 ) return true;
	else return false;
	
}

int MiniballSettings::GetBeamDumpDetector( unsigned int dgf, unsigned int ch ) {
	
	/// Return the beam dump detector ID by the DGF module number and Channel number
	if( dgf < n_dgf_mod && ch < n_dgf_ch )
		return bd_det[0][dgf][ch];
	
	else {
		
		std::cerr << "Bad beam dump event: dgf = " << dgf;
		std::cerr << ", channel = " << ch << std::endl;
		return 0;
		
	}
	
}

int MiniballSettings::GetBeamDumpDetector( unsigned int sfp, unsigned int board, unsigned int ch ) {
	
	/// Return the beam dump detector ID by the FEBEX SFP, Board number and Channel number
	if( sfp < n_febex_sfp && board < n_febex_board && ch < n_febex_ch )
		return bd_det[sfp][board][ch];
	
	else {
		
		std::cerr << "Bad beam dump event: sfp = " << sfp;
		std::cerr << ", board = " << board;
		std::cerr << ", channel = " << ch << std::endl;
		return 0;
		
	}
	
}

bool MiniballSettings::IsSpede( unsigned int sfp, unsigned int board, unsigned int ch ) {
	
	/// Return true if this is a SPEDE event
	if( GetSpedeSegment(sfp,board,ch) >= 0 ) return true;
	else return false;
	
}

int MiniballSettings::GetSpedeSegment( unsigned int sfp, unsigned int board, unsigned int ch ) {
	
	/// Return the SPEDE segment ID by the FEBEX SFP, Board number and Channel number
	if( sfp < n_febex_sfp && board < n_febex_board && ch < n_febex_ch )
		return spede_seg[sfp][board][ch];
	
	else {
		
		std::cerr << "Bad SPEDE event: sfp = " << sfp;
		std::cerr << ", board = " << board;
		std::cerr << ", channel = " << ch << std::endl;
		return 0;
		
	}
	
}

bool MiniballSettings::IsIonChamber( unsigned int adc, unsigned int ch ) {
	
	/// Return true if this is a IonChamber event
	if( GetIonChamberLayer(adc,ch) >= 0 ) return true;
	else return false;
	
}

bool MiniballSettings::IsIonChamber( unsigned int sfp, unsigned int board, unsigned int ch ) {
	
	/// Return true if this is a IonChamber event
	if( GetIonChamberLayer(sfp,board,ch) >= 0 ) return true;
	else return false;
	
}

int MiniballSettings::GetIonChamberLayer( unsigned int adc, unsigned int ch ) {
	
	/// Return the IonChamber layer ID by the ADC module number and Channel number
	if( adc < GetNumberOfAdcModules() && ch < GetMaximumNumberOfAdcChannels() )
		return ic_layer[0][adc][ch];
	
	else {
		
		std::cerr << "Bad IonChamber event: adc = " << adc;
		std::cerr << ", channel = " << ch << std::endl;
		return 0;
		
	}
	
}

int MiniballSettings::GetIonChamberLayer( unsigned int sfp, unsigned int board, unsigned int ch ) {
	
	/// Return the IonChamber layer ID by the FEBEX SFP, Board number and Channel number
	if( sfp < n_febex_sfp && board < n_febex_board && ch < n_febex_ch )
		return ic_layer[sfp][board][ch];
	
	else {
		
		std::cerr << "Bad IonChamber event: sfp = " << sfp;
		std::cerr << ", board = " << board;
		std::cerr << ", channel = " << ch << std::endl;
		return 0;
		
	}
	
}

bool MiniballSettings::IsPulser( unsigned int sfp, unsigned int board, unsigned int ch ) {
	
	/// Return true if this is a pulser event
	if( GetPulser(sfp,board,ch) >= 0 ) return true;
	else return false;
	
}

int MiniballSettings::GetPulser( unsigned int sfp, unsigned int board, unsigned int ch ) {
	
	/// Return the pulser ID
	if( sfp < n_febex_sfp && board < n_febex_board && ch < n_febex_ch )
		return pulser[sfp][board][ch];
	
	else {
		
		std::cerr << "Bad pulser event: sfp = " << sfp;
		std::cerr << ", board = " << board;
		std::cerr << ", channel = " << ch << std::endl;
		return 0;
		
	}
	
}
