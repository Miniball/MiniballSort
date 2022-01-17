#include "Histogrammer.hh"

Histogrammer::Histogrammer( Reaction *myreact, Settings *myset ){
	
	react = myreact;
	set = myset;
		
}

Histogrammer::~Histogrammer(){}


void Histogrammer::MakeHists() {

	std::string hname, htitle;

	// EBIS time windows
	ebis_td_gamma = new TH1F( "ebis_td_gamma", "Gamma-ray time with respect to EBIS;#Deltat;Counts per 20 #mus", 5e3, 0, 1e8  );
	ebis_td_particle = new TH1F( "ebis_td_particle", "Particle time with respect to EBIS;#Deltat;Counts per 20 #mus", 5e3, 0, 1e8  );

	// Gamma-ray singles histograms
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

	// CD singles histograms
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
	hname = "gamma_particle_td";
	htitle = "Gamma-ray - Particle time difference;#Deltat;Counts";
	gamma_particle_td = new TH1F( hname.data(), htitle.data(),
				1000, -1.0*set->GetEventWindow()-50, 1.0*set->GetEventWindow()+50 );

	
	
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

			} // k: recoils

		} // j: gamma ray
				

		// ------------------------- //
		// Loop over particle events //
		// ------------------------- //
		for( unsigned int j = 0; j < read_evts->GetParticleMultiplicity(); ++j ){

			// Get recoil event
			particle_evt = read_evts->GetParticleEvt(i);
			
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
