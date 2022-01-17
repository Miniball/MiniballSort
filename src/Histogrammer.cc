#include "Histogrammer.hh"

Histogrammer::Histogrammer( Reaction *myreact, Settings *myset ){
	
	react = myreact;
	set = myset;
		
}

Histogrammer::~Histogrammer(){}


void Histogrammer::MakeHists() {

	std::string hname, htitle;
	std::string dirname;

	// EBIS time windows
	dirname = "EBIS";
	if( !output_file->GetDirectory( dirname.data() ) )
		output_file->mkdir( dirname.data() );
	output_file->cd( dirname.data() );

	ebis_td_gamma = new TH1F( "ebis_td_gamma", "Gamma-ray time with respect to EBIS;#Deltat;Counts per 20 #mus", 5e3, 0, 1e8  );
	ebis_td_particle = new TH1F( "ebis_td_particle", "Particle time with respect to EBIS;#Deltat;Counts per 20 #mus", 5e3, 0, 1e8  );

	// Gamma-ray singles histograms
	dirname = "GammaRaySingles";
	if( !output_file->GetDirectory( dirname.data() ) )
		output_file->mkdir( dirname.data() );
	output_file->cd( dirname.data() );

	hname = "gE_singles";
	htitle = "Gamma-ray energy singles;Energy [keV];Counts 0.5 keV";
	gE_singles = new TH1F( hname.data(), htitle.data(), GBIN, GMIN, GMAX );
	
	hname = "gE_singles_ebis";
	htitle = "Gamma-ray energy singles EBIS on-off;Energy [keV];Counts 0.5 keV";
	gE_singles_ebis = new TH1F( hname.data(), htitle.data(), GBIN, GMIN, GMAX );

	hname = "gE_singles_ebis_on";
	htitle = "Gamma-ray energy singles EBIS on;Energy [keV];Counts 0.5 keV";
	gE_singles_ebis_on = new TH1F( hname.data(), htitle.data(), GBIN, GMIN, GMAX );

	hname = "gE_singles_ebis_off";
	htitle = "Gamma-ray energy singles EBIS off;Energy [keV];Counts 0.5 keV";
	gE_singles_ebis_off = new TH1F( hname.data(), htitle.data(), GBIN, GMIN, GMAX );

	// Gamma-ray coincidence histograms
	dirname = "GammaRayMatrices";
	if( !output_file->GetDirectory( dirname.data() ) )
		output_file->mkdir( dirname.data() );
	output_file->cd( dirname.data() );

	hname = "gamma_gamma_td";
	htitle = "Gamma-ray - Gamma-ray time difference;#Deltat;Counts";
	gamma_gamma_td = new TH1F( hname.data(), htitle.data(),
				1000, -1.0*set->GetEventWindow()-50, 1.0*set->GetEventWindow()+50 );

	hname = "gE_gE";
	htitle = "Gamma-ray coincidence matrix;Energy [keV];Energy [keV];Counts 0.5 keV";
	gE_gE = new TH2F( hname.data(), htitle.data(), GBIN, GMIN, GMAX, GBIN, GMIN, GMAX );

	hname = "gE_gE_ebis_on";
	htitle = "Gamma-ray coincidence matrix EBIS on;Energy [keV];Energy [keV];Counts 0.5 keV";
	gE_gE_ebis_on = new TH2F( hname.data(), htitle.data(), GBIN, GMIN, GMAX, GBIN, GMIN, GMAX );

	
	// CD singles histograms
	dirname = "ParticleSingles";
	if( !output_file->GetDirectory( dirname.data() ) )
		output_file->mkdir( dirname.data() );
	output_file->cd( dirname.data() );

	hname = "pE_theta";
	htitle = "Particle energy singles;Angle [deg];Energy [keV];Counts 0.5 keV";
	pE_theta = new TH2F( hname.data(), htitle.data(), react->GetNumberOfParticleThetas(), react->GetParticleThetas(), PBIN, PMIN, PMAX );

	hname = "pE_theta_coinc";
	htitle = "Particle energy in coincidence with a gamma ray;Angle [deg];Energy [keV];Counts 0.5 keV";
	pE_theta_coinc = new TH2F( hname.data(), htitle.data(), react->GetNumberOfParticleThetas(), react->GetParticleThetas(), PBIN, PMIN, PMAX );

	hname = "pE_theta_beam";
	htitle = "Particle energy singles, gated on beam;Angle [deg];Energy [keV];Counts 0.5 keV";
	pE_theta_beam = new TH2F( hname.data(), htitle.data(), react->GetNumberOfParticleThetas(), react->GetParticleThetas(), PBIN, PMIN, PMAX );

	hname = "pE_theta_target";
	htitle = "Particle energy singles, gated on target;Angle [deg];Energy [keV];Counts 0.5 keV";
	pE_theta_target = new TH2F( hname.data(), htitle.data(), react->GetNumberOfParticleThetas(), react->GetParticleThetas(), PBIN, PMIN, PMAX );

	// Gamma-particle coincidences
	dirname = "GammaRayParticleCoincidences";
	if( !output_file->GetDirectory( dirname.data() ) )
		output_file->mkdir( dirname.data() );
	output_file->cd( dirname.data() );

	hname = "gamma_particle_td";
	htitle = "Gamma-ray - Particle time difference;#Deltat;Counts";
	gamma_particle_td = new TH1F( hname.data(), htitle.data(),
				1000, -1.0*set->GetEventWindow()-50, 1.0*set->GetEventWindow()+50 );

	
	// Beam dump histograms
	dirname = "BeamDump";
	if( !output_file->GetDirectory( dirname.data() ) )
		output_file->mkdir( dirname.data() );
	output_file->cd( dirname.data() );

	hname = "bdE_singles";
	htitle = "Beam-dump gamma-ray energy singles;Energy [keV];Counts 0.5 keV";
	bdE_singles = new TH1F( hname.data(), htitle.data(), GBIN, GMIN, GMAX );
 
	hname = "bd_bd_td";
	htitle = "Beam-dump - Beam-dump time difference;#Deltat;Counts";
	bd_bd_td = new TH1F( hname.data(), htitle.data(),
				1000, -1.0*set->GetEventWindow()-50, 1.0*set->GetEventWindow()+50 );

	hname = "bdE_bdE";
	htitle = "Beam-dump gamma-ray coincidence matrix;Energy [keV];Energy [keV];Counts 0.5 keV";
	bdE_bdE = new TH2F( hname.data(), htitle.data(), GBIN, GMIN, GMAX, GBIN, GMIN, GMAX );

	bdE_singles_det.resize( set->GetNumberOfBeamDumpDetectors() );
	for( unsigned int i = 0; i < set->GetNumberOfBeamDumpDetectors(); ++i ){
		
		hname = "bdE_singles_det" + std::to_string(i);
		htitle  = "Beam-dump gamma-ray energy singles in detector ";
		htitle += std::to_string(i);
		htitle += ";Energy [keV];Counts 0.5 keV";
		bdE_singles_det[i] = new TH1F( hname.data(), htitle.data(), GBIN, GMIN, GMAX );
		
	}
	
	return;

}

unsigned long Histogrammer::FillHists( unsigned long start_fill ) {
	
	/// Main function to fill the histograms
	n_entries = input_tree->GetEntries();

	std::cout << " Histogrammer: number of entries in event tree = ";
	std::cout << n_entries << std::endl;
	
	if( start_fill == n_entries ){
	
		std::cout << " Histogrammer: Nothing to do..." << std::endl;
		return n_entries;
	
	}
	else {
	
		std::cout << " Histogrammer: Start filling at event #";
		std::cout << std::to_string( start_fill ) << std::endl;
	
	}
	
	// ------------------------------------------------------------------------ //
	// Main loop over TTree to find events
	// ------------------------------------------------------------------------ //
	for( unsigned int i = start_fill; i < n_entries; ++i ){

		// Current event data
		input_tree->GetEntry(i);
		
		// tdiff variable
		double tdiff;
			
		// ------------------------------------------ //
		// Loop over gamma-ray events without addback //
		// ------------------------------------------ //
		for( unsigned int j = 0; j < read_evts->GetGammaRayMultiplicity(); ++j ){

			// Get gamma-ray event
			gamma_evt = read_evts->GetGammaRayEvt(j);
						
			// Singles
			gE_singles->Fill( gamma_evt->GetEnergy() );
		
			// EBIS time
			ebis_td_gamma->Fill( (double)gamma_evt->GetTime() - (double)read_evts->GetEBIS() );

			// Check for events in the EBIS on-beam window
			if( OnBeam( gamma_evt ) ){
				
				gE_singles_ebis->Fill( gamma_evt->GetEnergy() );
				gE_singles_ebis_on->Fill( gamma_evt->GetEnergy() );

			} // ebis on
			
			else if( OffBeam( gamma_evt ) ){
				
				gE_singles_ebis->Fill( gamma_evt->GetEnergy(), -1.0 * react->GetEBISRatio() );
				gE_singles_ebis_off->Fill( gamma_evt->GetEnergy() );

			} // ebis off
			
			// Loop over particle events
			for( unsigned int k = 0; k < read_evts->GetParticleMultiplicity(); ++k ){
				
				// Get particle event
				particle_evt = read_evts->GetParticleEvt(k);
				
				// Time differences
				tdiff = (double)particle_evt->GetTime() - (double)gamma_evt->GetTime();
				gamma_particle_td->Fill( tdiff );
			
				// Check for prompt particle-gamma coincidences
				if( PromptCoincidence( gamma_evt, particle_evt ) ) {
	
					// Do things... We'll probably want to check if there is
					// a second or third particle that also comes in coincidence
					// and then make some decisions based on that.
					
				} // if prompt

			} // k: particles
			
			// Loop over other gamma events
			for( unsigned int k = j+1; k < read_evts->GetGammaRayMultiplicity(); ++k ){

				// Get gamma-ray event
				gamma_evt2 = read_evts->GetGammaRayEvt(k);

				// Time differences - symmetrise
				gamma_gamma_td->Fill( (double)gamma_evt->GetTime() - (double)gamma_evt2->GetTime() );
				gamma_gamma_td->Fill( (double)gamma_evt2->GetTime() - (double)gamma_evt->GetTime() );

				// Check for prompt gamma-gamma coincidences
				if( PromptCoincidence( gamma_evt, gamma_evt2 ) ) {
					
					// Fill and symmetrise
					gE_gE->Fill( gamma_evt->GetEnergy(), gamma_evt2->GetEnergy() );
					gE_gE->Fill( gamma_evt2->GetEnergy(), gamma_evt->GetEnergy() );

					// Apply EBIS condition
					if( OnBeam( gamma_evt ) && OnBeam( gamma_evt2 ) ) {
						
						// Fill and symmetrise
						gE_gE_ebis_on->Fill( gamma_evt->GetEnergy(), gamma_evt2->GetEnergy() );
						gE_gE_ebis_on->Fill( gamma_evt2->GetEnergy(), gamma_evt->GetEnergy() );

					} // On Beam
					
				} // if prompt
				
			} // k: second gamma-ray

		} // j: gamma ray
				

		// ------------------------- //
		// Loop over particle events //
		// ------------------------- //
		for( unsigned int j = 0; j < read_evts->GetParticleMultiplicity(); ++j ){

			// Get particle event
			particle_evt = read_evts->GetParticleEvt(j);
			
			// EBIS time
			ebis_td_particle->Fill( (double)particle_evt->GetTime() - (double)read_evts->GetEBIS() );
			
			// Energy vs Angle plot no gates
			pE_theta->Fill( react->GetParticleTheta( particle_evt ), particle_evt->GetEnergy() );
			
			// Energy EdE plot, after cut
			if( BeamCut( particle_evt ) )
				pE_theta_beam->Fill( react->GetParticleTheta( particle_evt ), particle_evt->GetEnergy() );
				
			if( TargetCut( particle_evt ) )
				pE_theta_target->Fill( react->GetParticleTheta( particle_evt ), particle_evt->GetEnergy() );

			
			// Check for prompt coincidence with a gamma-ray
			for( unsigned int k = 0; k < read_evts->GetGammaRayMultiplicity(); ++k ){

				// Get gamma-ray event
				gamma_evt = read_evts->GetGammaRayEvt(k);

				// Check for prompt coincidence
				if( PromptCoincidence( gamma_evt, particle_evt ) ){
					
					// Energy vs Angle plot with gamma-ray coincidence
					pE_theta_coinc->Fill( react->GetParticleTheta( particle_evt ), particle_evt->GetEnergy() );
					
				} // if prompt
				
			} // k: gammas
			
		} // j: particles

		
		// -------------------------- //
		// Loop over beam dump events //
		// -------------------------- //
		for( unsigned int j = 0; j < read_evts->GetBeamDumpMultiplicity(); ++j ){

			// Get beam dump event
			bd_evt = read_evts->GetBeamDumpEvt(j);
			
			// Singles spectra
			bdE_singles->Fill( bd_evt->GetEnergy() );
			bdE_singles_det[bd_evt->GetDetector()]->Fill( bd_evt->GetEnergy() );

			// Check for coincidences in case we have multiple beam dump detectors
			for( unsigned int k = j+1; k < read_evts->GetBeamDumpMultiplicity(); ++k ){

				// Get second beam dump event
				bd_evt2 = read_evts->GetBeamDumpEvt(k);
				
				// Fill time differences symmetrically
				bd_bd_td->Fill( (double)bd_evt->GetTime() - (double)bd_evt2->GetTime() );
				bd_bd_td->Fill( (double)bd_evt2->GetTime() - (double)bd_evt->GetTime() );

				// Check for prompt coincidence
				if( PromptCoincidence( bd_evt, bd_evt2 ) ) {
					
					// Fill energies symmetrically
					bdE_bdE->Fill( bd_evt->GetEnergy(), bd_evt2->GetEnergy() );
					bdE_bdE->Fill( bd_evt2->GetEnergy(), bd_evt->GetEnergy() );
				
				} // if prompt
				
			} // k: second beam dump
			
		} // j: beam dump
		
		
		// Progress bar
		if( i % 10000 == 0 || i+1 == n_entries ) {
			
			std::cout << " " << std::setw(8) << std::setprecision(4);
			std::cout << (float)(i+1)*100.0/(float)n_entries << "%    \r";
			std::cout.flush();
			
		}


	} // all events
	
	output_file->Write();
	
	return n_entries;
	
}

void Histogrammer::Terminate() {
	
	// Close output file
	output_file->Close();
	
}

void Histogrammer::SetInputFile( std::vector<std::string> input_file_names ) {
	
	/// Overlaaded function for a single file or multiple files
	input_tree = new TChain( "evt_tree" );
	for( unsigned int i = 0; i < input_file_names.size(); i++ ) {
	
		input_tree->Add( input_file_names[i].data() );
		
	}
	input_tree->SetBranchAddress( "MiniballEvts", &read_evts );

	return;
	
}

void Histogrammer::SetInputFile( std::string input_file_name ) {
	
	/// Overloaded function for a single file or multiple files
	input_tree = new TChain( "evt_tree" );
	input_tree->Add( input_file_name.data() );
	input_tree->SetBranchAddress( "MiniballEvts", &read_evts );

	return;
	
}

void Histogrammer::SetInputTree( TTree *user_tree ){
	
	// Find the tree and set branch addresses
	input_tree = (TChain*)user_tree;
	input_tree->SetBranchAddress( "MiniballEvts", &read_evts );

	return;
	
}
