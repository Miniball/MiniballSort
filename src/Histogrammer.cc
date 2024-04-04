#include "Histogrammer.hh"

MiniballHistogrammer::MiniballHistogrammer( std::shared_ptr<MiniballReaction> myreact, std::shared_ptr<MiniballSettings> myset ){
	
	react = myreact;
	set = myset;

	// Progress bar starts as false
	_prog_ = false;
	
	// Calculate histogram limits
	int window_ticks = ( set->GetEventWindow() + 10 ) / 10;
	TMAX = (float)window_ticks * 10.0 + 5.0;
	TMIN = TMAX * -1.0;
	TBIN = window_ticks * 2 + 1;
	
	// Check if we're doing transfer reactions and change particle energy range
	if( react->GetBeam()->GetIsotope() != react->GetEjectile()->GetIsotope() ) {
		
		if( react->GetRecoil()->GetA() <= 12 ) PMAX = 200e3;
		else if( react->GetBeam()->GetEnergy() < 100e3 ) PMAX = 200e3;
		else if( react->GetBeam()->GetEnergy() < 200e3 ) PMAX = 400e3;
		else if( react->GetBeam()->GetEnergy() < 400e3 ) PMAX = 800e3;
		else if( react->GetBeam()->GetEnergy() < 800e3 ) PMAX = 1600e3;

	}
	
	// Check if we're doing low-energy or light Coulex
	else {

		if( react->GetBeam()->GetEnergy() < 100e3 ) PMAX = 200e3;
		else if( react->GetBeam()->GetEnergy() < 200e3 ) PMAX = 400e3;
		else if( react->GetBeam()->GetEnergy() < 400e3 ) PMAX = 800e3;
		else if( react->GetBeam()->GetEnergy() < 800e3 ) PMAX = 1600e3;
		else if( react->GetBeam()->GetEnergy() > 2000e3 ) PMAX = 4000e3;

	}
	
}

void MiniballHistogrammer::MakeHists() {
	
	std::string hname, htitle;
	std::string dirname;
	
	// Time difference plots
	dirname = "Timing";
	output_file->mkdir( dirname.data() );
	output_file->cd( dirname.data() );
	
	ebis_td_gamma = new TH1F( "ebis_td_gamma", "Gamma-ray time with respect to EBIS;#Deltat;Counts per 20 #mus", 5.5e3, -0.1e8, 1e8  );
	ebis_td_particle = new TH1F( "ebis_td_particle", "Particle time with respect to EBIS;#Deltat;Counts per 20 #mus", 5.5e3, -0.1e8, 1e8  );
	
	hname = "gamma_particle_td";
	htitle = "Gamma-ray - Particle time difference;#Deltat;Counts";
	gamma_particle_td = new TH1F( hname.data(), htitle.data(),
								 TBIN, TMIN, TMAX );

	hname = "gamma_particle_E_vs_td";
	htitle = "Gamma-ray - Particle time difference versus gamma-ray energy;#Deltat;Gamma-ray energy (keV);Counts";
	gamma_particle_E_vs_td = new TH2F( hname.data(), htitle.data(),
								 TBIN, TMIN, TMAX, GBIN/4., 0., 2000. );

	hname = "gamma_gamma_td";
	htitle = "Gamma-ray - Gamma-ray time difference;#Deltat [ns];Counts";
	gamma_gamma_td = new TH1F( hname.data(), htitle.data(),
							  TBIN, TMIN, TMAX );
	
	hname = "gamma_electron_td";
	htitle = "Gamma-ray - Electron time difference;#Deltat [ns];Counts per 10 ns";
	gamma_electron_td = new TH1F( hname.data(), htitle.data(),
							  TBIN, TMIN, TMAX );
	
	hname = "electron_electron_td";
	htitle = "Electron - Electron time difference;#Deltat [ns];Counts per 10 ns";
	electron_electron_td = new TH1F( hname.data(), htitle.data(),
							  TBIN, TMIN, TMAX );
	
	hname = "electron_particle_td";
	htitle = "Electron - Particle time difference;#Deltat [ns];Counts per 10 ns";
	electron_particle_td = new TH1F( hname.data(), htitle.data(),
							  TBIN, TMIN, TMAX );
	
	hname = "particle_particle_td";
	htitle = "Particle - Particle time difference;#Deltat [ns];Counts per 10 ns";
	particle_particle_td = new TH1F( hname.data(), htitle.data(),
								 TBIN, TMIN, TMAX );

	// Gamma-ray singles histograms
	dirname = "GammaRaySingles";
	output_file->mkdir( dirname.data() );
	output_file->cd( dirname.data() );
	
	hname = "gE_singles";
	htitle = "Gamma-ray energy singles;Energy [keV];Counts per 0.5 keV";
	gE_singles = new TH1F( hname.data(), htitle.data(), GBIN, GMIN, GMAX );
	
	hname = "gE_singles_ebis";
	htitle = "Gamma-ray energy singles EBIS on-off;Energy [keV];Counts per 0.5 keV";
	gE_singles_ebis = new TH1F( hname.data(), htitle.data(), GBIN, GMIN, GMAX );
	
	hname = "gE_singles_ebis_on";
	htitle = "Gamma-ray energy singles EBIS on;Energy [keV];Counts per 0.5 keV";
	gE_singles_ebis_on = new TH1F( hname.data(), htitle.data(), GBIN, GMIN, GMAX );
	
	hname = "gE_singles_ebis_off";
	htitle = "Gamma-ray energy singles EBIS off;Energy [keV];Counts per 0.5 keV";
	gE_singles_ebis_off = new TH1F( hname.data(), htitle.data(), GBIN, GMIN, GMAX );
	
	hname = "gE_singles_dc";
	htitle = "Gamma-ray energy singles, Doppler corrected for unscattered beam;Energy [keV];Counts per 0.5 keV";
	gE_singles_dc = new TH1F( hname.data(), htitle.data(), GBIN, GMIN, GMAX );
	
	hname = "gE_singles_dc_ebis";
	htitle = "Gamma-ray energy singles, Doppler corrected for unscattered beam, EBIS on-off;Energy [keV];Counts per 0.5 keV";
	gE_singles_dc_ebis = new TH1F( hname.data(), htitle.data(), GBIN, GMIN, GMAX );
	
	hname = "aE_singles";
	htitle = "Gamma-ray energy with addback singles;Energy [keV];Counts per 0.5 keV";
	aE_singles = new TH1F( hname.data(), htitle.data(), GBIN, GMIN, GMAX );
	
	hname = "aE_singles_ebis";
	htitle = "Gamma-ray energy with addback singles EBIS on-off;Energy [keV];Counts per 0.5 keV";
	aE_singles_ebis = new TH1F( hname.data(), htitle.data(), GBIN, GMIN, GMAX );
	
	hname = "aE_singles_ebis_on";
	htitle = "Gamma-ray energy with addback singles EBIS on;Energy [keV];Counts per 0.5 keV";
	aE_singles_ebis_on = new TH1F( hname.data(), htitle.data(), GBIN, GMIN, GMAX );
	
	hname = "aE_singles_ebis_off";
	htitle = "Gamma-ray energy with addback singles EBIS off;Energy [keV];Counts per 0.5 keV";
	aE_singles_ebis_off = new TH1F( hname.data(), htitle.data(), GBIN, GMIN, GMAX );

	hname = "aE_singles_dc";
	htitle = "Gamma-ray energy with addback singles, Doppler corrected for unscattered beam;Energy [keV];Counts per 0.5 keV";
	aE_singles_dc = new TH1F( hname.data(), htitle.data(), GBIN, GMIN, GMAX );
	
	hname = "aE_singles_dc_ebis";
	htitle = "Gamma-ray energy with addback singles, Doppler corrected for unscattered beam, EBIS on-off;Energy [keV];Counts per 0.5 keV";
	aE_singles_dc_ebis = new TH1F( hname.data(), htitle.data(), GBIN, GMIN, GMAX );
	
	hname = "gamma_xy_map_forward";
	htitle = "Gamma-ray X-Y hit map (forward: z > 0);y (horizontal) [mm];x (vertical) [mm];Counts";
	gamma_xy_map_forward = new TH2F( hname.data(), htitle.data(), 201, -201., 201., 201, -201., 201. );

	hname = "gamma_xy_map_backward";
	htitle = "Gamma-ray X-Y hit map (backwards: z < 0);y (horizontal) [mm];x (vertical) [mm];Counts";
	gamma_xy_map_backward = new TH2F( hname.data(), htitle.data(), 201, -201., 201., 201, -201., 201. );

	hname = "gamma_xz_map_left";
	htitle = "Gamma-ray X-Z hit map (left: y < 0);z (horizontal) [mm];x (vertical) [mm];Counts";
	gamma_xz_map_left = new TH2F( hname.data(), htitle.data(), 201, -201., 201., 201, -201., 201. );
	
	hname = "gamma_xz_map_right";
	htitle = "Gamma-ray X-Z hit map (right: y > 0);z (horizontal) [mm];x (vertical) [mm];Counts";
	gamma_xz_map_right = new TH2F( hname.data(), htitle.data(), 201, -201., 201., 201, -201., 201. );
	
	hname = "gamma_theta_phi_map";
	htitle = "Gamma-ray #theta-#phi hit map;#theta [degrees];#phi [degrees];Counts";
	gamma_theta_phi_map = new TH2F( hname.data(), htitle.data(), 180, 0., 180., 360, 0., 360. );

	// Gamma-ray coincidence histograms
	dirname = "CoincidenceMatrices";
	output_file->mkdir( dirname.data() );
	output_file->cd( dirname.data() );
	
	// If gamma-gamma histograms are turned on
	if( react->HistGammaGamma() ) {
		
		hname = "gE_gE";
		htitle = "Gamma-ray coincidence matrix;Energy [keV];Energy [keV];Counts per 0.5 keV";
		gE_gE = new TH2F( hname.data(), htitle.data(), GBIN, GMIN, GMAX, GBIN, GMIN, GMAX );
		
		hname = "gE_gE_ebis_on";
		htitle = "Gamma-ray coincidence matrix EBIS on;Energy [keV];Energy [keV];Counts per 0.5 keV";
		gE_gE_ebis_on = new TH2F( hname.data(), htitle.data(), GBIN, GMIN, GMAX, GBIN, GMIN, GMAX );
		
		hname = "aE_aE";
		htitle = "Gamma-ray addback coincidence matrix;Energy [keV];Energy [keV];Counts per 0.5 keV";
		aE_aE = new TH2F( hname.data(), htitle.data(), GBIN, GMIN, GMAX, GBIN, GMIN, GMAX );
		
		hname = "aE_aE_ebis_on";
		htitle = "Gamma-ray addback coincidence matrix EBIS on;Energy [keV];Energy [keV];Counts per 0.5 keV";
		aE_aE_ebis_on = new TH2F( hname.data(), htitle.data(), GBIN, GMIN, GMAX, GBIN, GMIN, GMAX );
		
	} // gamma-gamma on
	
	// If electron histograms are turned on
	if( react->HistElectron() ) {
		
		hname = "eE_eE";
		htitle = "Electron coincidence matrix;Energy [keV];Energy [keV];Counts per keV";
		eE_eE = new TH2F( hname.data(), htitle.data(), EBIN, EMIN, EMAX, EBIN, EMIN, EMAX );
		
		hname = "eE_eE_ebis_on";
		htitle = "Electron coincidence matrix EBIS on;Energy [keV];Energy [keV];Counts per keV";
		eE_eE_ebis_on = new TH2F( hname.data(), htitle.data(), EBIN, EMIN, EMAX, EBIN, EMIN, EMAX );
		
		// If electron-gamma histograms are turned on
		if( react->HistElectronGamma() ) {
			
			hname = "gE_eE";
			htitle = "Gamma-ray and electron coincidence matrix;#gamma-ray energy [keV];e^{-} energy [keV];Counts per 0.5 keV";
			gE_eE = new TH2F( hname.data(), htitle.data(), GBIN, GMIN, GMAX, EBIN, EMIN, EMAX );
			
			hname = "gE_eE_ebis_on";
			htitle = "Gamma-ray and electron coincidence matrix EBIS on;#gamma-ray energy [keV];e^{-} energy [keV];Counts per 0.5 keV";
			gE_eE_ebis_on = new TH2F( hname.data(), htitle.data(), GBIN, GMIN, GMAX, EBIN, EMIN, EMAX );
			
			hname = "aE_eE";
			htitle = "Gamma-ray addback and electron coincidence matrix;#gamma-ray energy [keV];e^{-} energy [keV];Counts per 0.5 keV";
			aE_eE = new TH2F( hname.data(), htitle.data(), GBIN, GMIN, GMAX, EBIN, EMIN, EMAX );
			
			hname = "aE_eE_ebis_on";
			htitle = "Gamma-ray addback and electron coincidence matrix EBIS on;#gamma-ray energy [keV];e^{-} energy [keV];Counts per 0.5 keV";
			aE_eE_ebis_on = new TH2F( hname.data(), htitle.data(), GBIN, GMIN, GMAX, EBIN, EMIN, EMAX );
			
		} // electron-gamma on
		
		// Electron singles histograms
		dirname = "ElectronSingles";
		output_file->mkdir( dirname.data() );
		output_file->cd( dirname.data() );
		
		hname = "eE_singles";
		htitle = "Electron energy singles;Energy [keV];Counts keV";
		eE_singles = new TH1F( hname.data(), htitle.data(), EBIN, EMIN, EMAX );
		
		hname = "eE_singles_ebis";
		htitle = "Electron energy singles EBIS on-off;Energy [keV];Counts keV";
		eE_singles_ebis = new TH1F( hname.data(), htitle.data(), EBIN, EMIN, EMAX );
		
		hname = "eE_singles_ebis_on";
		htitle = "Electron energy singles EBIS on;Energy [keV];Counts keV";
		eE_singles_ebis_on = new TH1F( hname.data(), htitle.data(), EBIN, EMIN, EMAX );
		
		hname = "eE_singles_ebis_off";
		htitle = "Electron energy singles EBIS off;Energy [keV];Counts keV";
		eE_singles_ebis_off = new TH1F( hname.data(), htitle.data(), EBIN, EMIN, EMAX );
		
		hname = "electron_xy_map";
		htitle = "Electron X-Y hit map (#theta < 90);y (horizontal) [mm];x (vertical) [mm];Counts per mm^2";
		electron_xy_map = new TH2F( hname.data(), htitle.data(), 361, -45.125, 45.125, 361, -45.125, 45.125 );
		
	} // electrons on
	
	// CD singles histograms
	dirname = "ParticleSpectra";
	output_file->mkdir( dirname.data() );
	output_file->cd( dirname.data() );
	
	hname = "pE_theta";
	htitle = "Particle energy singles;Angle [deg];Energy [keV];Counts";
	pE_theta = new TH2F( hname.data(), htitle.data(), react->GetNumberOfParticleThetas(), react->GetParticleThetas().data(), PBIN, PMIN, PMAX );
	
	hname = "pE_theta_coinc";
	htitle = "Particle energy in coincidence with a gamma ray;Angle [deg];Energy [keV];Counts";
	pE_theta_coinc = new TH2F( hname.data(), htitle.data(), react->GetNumberOfParticleThetas(), react->GetParticleThetas().data(), PBIN, PMIN, PMAX );
	
	hname = "pE_theta_ejectile";
	htitle = "Particle energy singles, gated on ejectile;Angle [deg];Energy [keV];Counts";
	pE_theta_ejectile = new TH2F( hname.data(), htitle.data(), react->GetNumberOfParticleThetas(), react->GetParticleThetas().data(), PBIN, PMIN, PMAX );
	
	hname = "pE_theta_recoil";
	htitle = "Particle energy singles, gated on recoil;Angle [deg];Energy [keV];Counts";
	pE_theta_recoil = new TH2F( hname.data(), htitle.data(), react->GetNumberOfParticleThetas(), react->GetParticleThetas().data(), PBIN, PMIN, PMAX );
	
	pE_dE.resize( set->GetNumberOfCDDetectors() );
	pE_dE_coinc.resize( set->GetNumberOfCDDetectors() );
	pE_dE_cut.resize( set->GetNumberOfCDDetectors() );
	for( unsigned int i = 0; i < set->GetNumberOfCDDetectors(); ++i ) {

		pE_dE[i].resize( set->GetNumberOfCDSectors() );
		pE_dE_coinc[i].resize( set->GetNumberOfCDSectors() );
		pE_dE_cut[i].resize( set->GetNumberOfCDSectors() );

		for( unsigned int j = 0; j < set->GetNumberOfCDSectors(); ++j ) {

			hname = "pE_dE_" + std::to_string(i) + "_" + std::to_string(j);
			htitle = "Particle energy total versus energy loss for CD " + std::to_string(i);
			htitle += ", sector " + std::to_string(j);
			htitle += ";Energy total [keV];Energy loss [keV];Counts";
			pE_dE[i][j] = new TH2F( hname.data(), htitle.data(), PBIN, PMIN, PMAX, PBIN, PMIN, PMAX );

			hname = "pE_dE_" + std::to_string(i) + "_" + std::to_string(j) + "_coinc";
			htitle = "Particle energy total versus energy loss for CD " + std::to_string(i);
			htitle += ", sector " + std::to_string(j) + ", coincident with a gamma-ray";
			htitle += ";Energy total [keV];Energy loss [keV];Counts";
			pE_dE_coinc[i][j] = new TH2F( hname.data(), htitle.data(), PBIN, PMIN, PMAX, PBIN, PMIN, PMAX );

			hname = "pE_dE_" + std::to_string(i) + "_" + std::to_string(j) + "_cut";
			htitle = "Particle energy total versus energy loss for CD " + std::to_string(i);
			htitle += ", sector " + std::to_string(j) + ", after transfer cut applied";
			htitle += ";Energy total [keV];Energy loss [keV];Counts";
			pE_dE_cut[i][j] = new TH2F( hname.data(), htitle.data(), PBIN, PMIN, PMAX, PBIN, PMIN, PMAX );

		}

	}
	
	hname = "pBeta_theta_ejectile";
	htitle = "Reconstructed ejectile velocity;Angle [deg];#beta [c];Counts";
	pBeta_theta_ejectile = new TProfile( hname.data(), htitle.data(), react->GetNumberOfParticleThetas(), react->GetParticleThetas().data() );
	
	hname = "pBeta_theta_recoil";
	htitle = "Reconstructed recoil velocity;Angle [deg];#beta [c];Counts";
	pBeta_theta_recoil = new TProfile( hname.data(), htitle.data(), react->GetNumberOfParticleThetas(), react->GetParticleThetas().data() );
	
	hname = "particle_xy_map_forward";
	htitle = "Particle X-Y hit map (#theta < 90);y (horizontal) [mm];x (vertical) [mm];Counts per mm^2";
	particle_xy_map_forward = new TH2F( hname.data(), htitle.data(), 361, -45.125, 45.125, 361, -45.125, 45.125 );

	hname = "particle_xy_map_backward";
	htitle = "Particle X-Y hit map (#theta > 90);y (horizontal) [mm];x (vertical);Counts per mm^2";
	particle_xy_map_backward = new TH2F( hname.data(), htitle.data(), 361, -45.125, 45.125, 361, -45.125, 45.125 );

	hname = "particle_theta_phi_map";
	htitle = "Particle #theta-#phi hit map;#theta [mm];#phi [mm];Counts";
	particle_theta_phi_map = new TH2F( hname.data(), htitle.data(), react->GetNumberOfParticleThetas(), react->GetParticleThetas().data(), 180, -181, 181 );

	// Gamma-particle coincidences without addback
	dirname = "GammaRayParticleCoincidences";
	output_file->mkdir( dirname.data() );
	output_file->cd( dirname.data() );
	
	hname = "gE_prompt";
	htitle = "Gamma-ray energy in prompt coincide with any particle;Energy [keV];Counts per 0.5 keV";
	gE_prompt = new TH1F( hname.data(), htitle.data(), GBIN, GMIN, GMAX );

	hname = "gE_prompt_1p";
	htitle = "Gamma-ray energy in prompt coincide with just 1 particle;Energy [keV];Counts per 0.5 keV";
	gE_prompt_1p = new TH1F( hname.data(), htitle.data(), GBIN, GMIN, GMAX );

	hname = "gE_prompt_2p";
	htitle = "Gamma-ray energy in prompt coincide with 2 particles;Energy [keV];Counts per 0.5 keV";
	gE_prompt_2p = new TH1F( hname.data(), htitle.data(), GBIN, GMIN, GMAX );

	hname = "gE_random";
	htitle = "Gamma-ray energy in random coincide with any particle;Energy [keV];Counts per 0.5 keV";
	gE_random = new TH1F( hname.data(), htitle.data(), GBIN, GMIN, GMAX );

	hname = "gE_random_1p";
	htitle = "Gamma-ray energy in random coincide with just 1 particle;Energy [keV];Counts per 0.5 keV";
	gE_random_1p = new TH1F( hname.data(), htitle.data(), GBIN, GMIN, GMAX );

	hname = "gE_random_2p";
	htitle = "Gamma-ray energy in random coincide with 2 particles;Energy [keV];Counts per 0.5 keV";
	gE_random_2p = new TH1F( hname.data(), htitle.data(), GBIN, GMIN, GMAX );

	hname = "gE_ejectile_dc_none";
	htitle = "Gamma-ray energy, gated on the ejectile with random subtraction;";
	htitle += "Energy [keV];Counts per 0.5 keV";
	gE_ejectile_dc_none = new TH1F( hname.data(), htitle.data(), GBIN, GMIN, GMAX );

	hname = "gE_ejectile_dc_ejectile";
	htitle = "Gamma-ray energy, gated on the ejectile, Doppler corrected for the ejectile with random subtraction;";
	htitle += "Energy [keV];Counts per 0.5 keV";
	gE_ejectile_dc_ejectile = new TH1F( hname.data(), htitle.data(), GBIN, GMIN, GMAX );

	hname = "gE_ejectile_dc_recoil";
	htitle = "Gamma-ray energy, gated on the ejectile, Doppler corrected for the recoil with random subtraction;";
	htitle += "Energy [keV];Counts per 0.5 keV";
	gE_ejectile_dc_recoil = new TH1F( hname.data(), htitle.data(), GBIN, GMIN, GMAX );

	hname = "gE_recoil_dc_none";
	htitle = "Gamma-ray energy, gated on the recoil with random subtraction;";
	htitle += "Energy [keV];Counts per 0.5 keV";
	gE_recoil_dc_none = new TH1F( hname.data(), htitle.data(), GBIN, GMIN, GMAX );

	hname = "gE_recoil_dc_ejectile";
	htitle = "Gamma-ray energy, gated on the recoil, Doppler corrected for the ejectile with random subtraction;";
	htitle += "Energy [keV];Counts per 0.5 keV";
	gE_recoil_dc_ejectile = new TH1F( hname.data(), htitle.data(), GBIN, GMIN, GMAX );

	hname = "gE_recoil_dc_recoil";
	htitle = "Gamma-ray energy, gated on the recoil, Doppler corrected for the recoil with random subtraction;";
	htitle += "Energy [keV];Counts per 0.5 keV";
	gE_recoil_dc_recoil = new TH1F( hname.data(), htitle.data(), GBIN, GMIN, GMAX );

	hname = "gE_2p_dc_none";
	htitle = "Gamma-ray energy, in coincidence with ejectile and recoil with random subtraction;";
	htitle += "Energy [keV];Counts per 0.5 keV";
	gE_2p_dc_none = new TH1F( hname.data(), htitle.data(), GBIN, GMIN, GMAX );

	hname = "gE_2p_dc_ejectile";
	htitle = "Gamma-ray energy, in coincidence with ejectile and recoil, Doppler corrected for the ejectile with random subtraction;";
	htitle += "Energy [keV];Counts per 0.5 keV";
	gE_2p_dc_ejectile = new TH1F( hname.data(), htitle.data(), GBIN, GMIN, GMAX );

	hname = "gE_2p_dc_recoil";
	htitle = "Gamma-ray energy, in coincidence with ejectile and recoil, Doppler corrected for the recoil with random subtraction;";
	htitle += "Energy [keV];Counts per 0.5 keV";
	gE_2p_dc_recoil = new TH1F( hname.data(), htitle.data(), GBIN, GMIN, GMAX );

	hname = "gE_costheta_ejectile";
	htitle = "Gamma-ray energy versus cos(#theta) of angle between ejectile and gamma-ray;Energy [keV];cos(#theta_p#gamma)";
	gE_costheta_ejectile = new TH2F( hname.data(), htitle.data(), GBIN, GMIN, GMAX, 100, -1.0, 1.0 );

	hname = "gE_costheta_recoil";
	htitle = "Gamma-ray energy versus cos(#theta) of angle between recoil and gamma-ray;Energy [keV];cos(#theta_p#gamma)";
	gE_costheta_recoil = new TH2F( hname.data(), htitle.data(), GBIN, GMIN, GMAX, 100, -1.0, 1.0 );

	hname = "gE_vs_theta_ejectile_dc_none";
	htitle = "Gamma-ray energy, gated on the ejectile with random subtraction;";
	htitle += "Theta [deg];Energy [keV];Counts per 0.5 keV per strip";
	gE_vs_theta_ejectile_dc_none = new TH2F( hname.data(), htitle.data(), react->GetNumberOfParticleThetas(), react->GetParticleThetas().data(), GBIN, GMIN, GMAX );

	hname = "gE_vs_theta_ejectile_dc_ejectile";
	htitle = "Gamma-ray energy, gated on the ejectile, Doppler corrected for the ejectile with random subtraction;";
	htitle += "Theta [deg];Energy [keV];Counts per 0.5 keV per strip";
	gE_vs_theta_ejectile_dc_ejectile = new TH2F( hname.data(), htitle.data(), react->GetNumberOfParticleThetas(), react->GetParticleThetas().data(), GBIN, GMIN, GMAX );

	hname = "gE_vs_theta_ejectile_dc_recoil";
	htitle = "Gamma-ray energy, gated on the ejectile, Doppler corrected for the recoil with random subtraction;";
	htitle += "Theta [deg];Energy [keV];Counts per 0.5 keV per strip";
	gE_vs_theta_ejectile_dc_recoil = new TH2F( hname.data(), htitle.data(), react->GetNumberOfParticleThetas(), react->GetParticleThetas().data(), GBIN, GMIN, GMAX );

	hname = "gE_vs_theta_recoil_dc_none";
	htitle = "Gamma-ray energy, gated on the recoil with random subtraction;";
	htitle += "Theta [deg];Energy [keV];Counts per 0.5 keV per strip";
	gE_vs_theta_recoil_dc_none = new TH2F( hname.data(), htitle.data(), react->GetNumberOfParticleThetas(), react->GetParticleThetas().data(), GBIN, GMIN, GMAX );

	hname = "gE_vs_theta_recoil_dc_ejectile";
	htitle = "Gamma-ray energy, gated on the recoil, Doppler corrected for the ejectile with random subtraction;";
	htitle += "Theta [deg];Energy [keV];Counts per 0.5 keV per strip";
	gE_vs_theta_recoil_dc_ejectile = new TH2F( hname.data(), htitle.data(), react->GetNumberOfParticleThetas(), react->GetParticleThetas().data(), GBIN, GMIN, GMAX );

	hname = "gE_vs_theta_recoil_dc_recoil";
	htitle = "Gamma-ray energy, gated on the recoil, Doppler corrected for the recoil with random subtraction;";
	htitle += "Theta [deg];Energy [keV];Counts per 0.5 keV per strip";
	gE_vs_theta_recoil_dc_recoil = new TH2F( hname.data(), htitle.data(), react->GetNumberOfParticleThetas(), react->GetParticleThetas().data(), GBIN, GMIN, GMAX );

	hname = "gE_vs_theta_2p_dc_none";
	htitle = "Gamma-ray energy, in coincidence with ejectile and recoil with random subtraction;";
	htitle += "Theta [deg];Energy [keV];Counts per 0.5 keV per strip";
	gE_vs_theta_2p_dc_none = new TH2F( hname.data(), htitle.data(), react->GetNumberOfParticleThetas(), react->GetParticleThetas().data(), GBIN, GMIN, GMAX );

	hname = "gE_vs_theta_2p_dc_ejectile";
	htitle = "Gamma-ray energy, in coincidence with ejectile and recoil, Doppler corrected for the ejectile with random subtraction;";
	htitle += "Theta [deg];Energy [keV];Counts per 0.5 keV per strip";
	gE_vs_theta_2p_dc_ejectile = new TH2F( hname.data(), htitle.data(), react->GetNumberOfParticleThetas(), react->GetParticleThetas().data(), GBIN, GMIN, GMAX );

	hname = "gE_vs_theta_2p_dc_recoil";
	htitle = "Gamma-ray energy, in coincidence with ejectile and recoil, Doppler corrected for the recoil with random subtraction;";
	htitle += "Theta [deg];Energy [keV];Counts per 0.5 keV per strip";
	gE_vs_theta_2p_dc_recoil = new TH2F( hname.data(), htitle.data(), react->GetNumberOfParticleThetas(), react->GetParticleThetas().data(), GBIN, GMIN, GMAX );

	hname = "ggE_ejectile_dc_none";
	htitle = "Gamma-gamma matrix, gated on the ejectile with random subtraction;";
	htitle += "Gamma-ray Energy [keV];Gamma-ray Energy [keV];Counts per 0.5 keV";
	ggE_ejectile_dc_none = new TH2F( hname.data(), htitle.data(), GBIN, GMIN, GMAX, GBIN, GMIN, GMAX );

	hname = "ggE_ejectile_dc_ejectile";
	htitle = "Gamma-gamma matrix, gated on the ejectile, Doppler corrected for the ejectile with random subtraction;";
	htitle += "Gamma-ray Energy [keV];Gamma-ray Energy [keV];Counts per 0.5 keV";
	ggE_ejectile_dc_ejectile = new TH2F( hname.data(), htitle.data(), GBIN, GMIN, GMAX, GBIN, GMIN, GMAX );

	hname = "ggE_ejectile_dc_recoil";
	htitle = "Gamma-gamma matrix, gated on the ejectile, Doppler corrected for the recoil with random subtraction;";
	htitle += "Gamma-ray Energy [keV];Gamma-ray Energy [keV];Counts per 0.5 keV";
	ggE_ejectile_dc_recoil = new TH2F( hname.data(), htitle.data(), GBIN, GMIN, GMAX, GBIN, GMIN, GMAX );

	hname = "ggE_recoil_dc_none";
	htitle = "Gamma-gamma matrix, gated on the recoil with random subtraction;";
	htitle += "Gamma-ray Energy [keV];Gamma-ray Energy [keV];Counts per 0.5 keV";
	ggE_recoil_dc_none = new TH2F( hname.data(), htitle.data(), GBIN, GMIN, GMAX, GBIN, GMIN, GMAX );

	hname = "ggE_recoil_dc_ejectile";
	htitle = "Gamma-gamma matrix, gated on the recoil, Doppler corrected for the ejectile with random subtraction;";
	htitle += "Gamma-ray Energy [keV];Gamma-ray Energy [keV];Counts per 0.5 keV";
	ggE_recoil_dc_ejectile = new TH2F( hname.data(), htitle.data(), GBIN, GMIN, GMAX, GBIN, GMIN, GMAX );

	hname = "ggE_recoil_dc_recoil";
	htitle = "Gamma-gamma matrix, gated on the recoil, Doppler corrected for the recoil with random subtraction;";
	htitle += "Gamma-ray Energy [keV];Gamma-ray Energy [keV];Counts per 0.5 keV";
	ggE_recoil_dc_recoil = new TH2F( hname.data(), htitle.data(), GBIN, GMIN, GMAX, GBIN, GMIN, GMAX );

	// Gamma-particle coincidences with addback
	dirname = "GammaRayAddbackParticleCoincidences";
	output_file->mkdir( dirname.data() );
	output_file->cd( dirname.data() );
	
	hname = "aE_prompt";
	htitle = "Gamma-ray energy with addback in prompt coincide with any particle;Energy [keV];Counts per 0.5 keV";
	aE_prompt = new TH1F( hname.data(), htitle.data(), GBIN, GMIN, GMAX );

	hname = "aE_prompt_1p";
	htitle = "Gamma-ray energy with addback in prompt coincide with just 1 particle;Energy [keV];Counts per 0.5 keV";
	aE_prompt_1p = new TH1F( hname.data(), htitle.data(), GBIN, GMIN, GMAX );

	hname = "aE_prompt_2p";
	htitle = "Gamma-ray energy with addback in prompt coincide with 2 particles;Energy [keV];Counts per 0.5 keV";
	aE_prompt_2p = new TH1F( hname.data(), htitle.data(), GBIN, GMIN, GMAX );

	hname = "aE_random";
	htitle = "Gamma-ray energy with addback in random coincide with any particle;Energy [keV];Counts per 0.5 keV";
	aE_random = new TH1F( hname.data(), htitle.data(), GBIN, GMIN, GMAX );

	hname = "aE_random_1p";
	htitle = "Gamma-ray energy with addback in random coincide with just 1 particle;Energy [keV];Counts per 0.5 keV";
	aE_random_1p = new TH1F( hname.data(), htitle.data(), GBIN, GMIN, GMAX );

	hname = "aE_random_2p";
	htitle = "Gamma-ray energy with addback in random coincide with 2 particles;Energy [keV];Counts per 0.5 keV";
	aE_random_2p = new TH1F( hname.data(), htitle.data(), GBIN, GMIN, GMAX );

	hname = "aE_ejectile_dc_none";
	htitle = "Gamma-ray energy with addback, gated on the ejectile with random subtraction;";
	htitle += "Energy [keV];Counts per 0.5 keV";
	aE_ejectile_dc_none = new TH1F( hname.data(), htitle.data(), GBIN, GMIN, GMAX );

	hname = "aE_ejectile_dc_ejectile";
	htitle = "Gamma-ray energy with addback, gated on the ejectile, Doppler corrected for the ejectile with random subtraction;";
	htitle += "Energy [keV];Counts per 0.5 keV";
	aE_ejectile_dc_ejectile = new TH1F( hname.data(), htitle.data(), GBIN, GMIN, GMAX );

	hname = "aE_ejectile_dc_recoil";
	htitle = "Gamma-ray energy with addback, gated on the ejectile, Doppler corrected for the recoil with random subtraction;";
	htitle += "Energy [keV];Counts per 0.5 keV";
	aE_ejectile_dc_recoil = new TH1F( hname.data(), htitle.data(), GBIN, GMIN, GMAX );

	hname = "aE_recoil_dc_none";
	htitle = "Gamma-ray energy with addback, gated on the recoil with random subtraction;";
	htitle += "Energy [keV];Counts per 0.5 keV";
	aE_recoil_dc_none = new TH1F( hname.data(), htitle.data(), GBIN, GMIN, GMAX );

	hname = "aE_recoil_dc_ejectile";
	htitle = "Gamma-ray energy with addback, gated on the recoil, Doppler corrected for the ejectile with random subtraction;";
	htitle += "Energy [keV];Counts per 0.5 keV";
	aE_recoil_dc_ejectile = new TH1F( hname.data(), htitle.data(), GBIN, GMIN, GMAX );

	hname = "aE_recoil_dc_recoil";
	htitle = "Gamma-ray energy with addback, gated on the recoil, Doppler corrected for the recoil with random subtraction;";
	htitle += "Energy [keV];Counts per 0.5 keV";
	aE_recoil_dc_recoil = new TH1F( hname.data(), htitle.data(), GBIN, GMIN, GMAX );

	hname = "aE_2p_dc_none";
	htitle = "Gamma-ray energy with addback, in coincidence with ejectile and recoil with random subtraction;";
	htitle += "Energy [keV];Counts per 0.5 keV";
	aE_2p_dc_none = new TH1F( hname.data(), htitle.data(), GBIN, GMIN, GMAX );

	hname = "aE_2p_dc_ejectile";
	htitle = "Gamma-ray energy with addback, in coincidence with ejectile and recoil, Doppler corrected for the ejectile with random subtraction;";
	htitle += "Energy [keV];Counts per 0.5 keV";
	aE_2p_dc_ejectile = new TH1F( hname.data(), htitle.data(), GBIN, GMIN, GMAX );

	hname = "aE_2p_dc_recoil";
	htitle = "Gamma-ray energy with addback, in coincidence with ejectile and recoil, Doppler corrected for the recoil with random subtraction;";
	htitle += "Energy [keV];Counts per 0.5 keV";
	aE_2p_dc_recoil = new TH1F( hname.data(), htitle.data(), GBIN, GMIN, GMAX );

	hname = "aE_costheta_ejectile";
	htitle = "Gamma-ray energy with addback versus cos(#theta) of angle between ejectile and gamma-ray;";
	htitle += ";Energy [keV];cos(#theta_p#gamma)";
	aE_costheta_ejectile = new TH2F( hname.data(), htitle.data(), GBIN, GMIN, GMAX, 100, -1.0, 1.0 );

	hname = "aE_costheta_recoil";
	htitle = "Gamma-ray energy with addback versus cos(#theta) of angle between recoil and gamma-ray;";
	htitle += ";Energy [keV];cos(#theta_p#gamma)";
	aE_costheta_recoil = new TH2F( hname.data(), htitle.data(), GBIN, GMIN, GMAX, 100, -1.0, 1.0 );

	hname = "aE_vs_theta_ejectile_dc_none";
	htitle = "Gamma-ray energy with addback, gated on the ejectile with random subtraction;";
	htitle += "Theta [deg];Energy [keV];Counts per 0.5 keV per strip";
	aE_vs_theta_ejectile_dc_none = new TH2F( hname.data(), htitle.data(), react->GetNumberOfParticleThetas(), react->GetParticleThetas().data(), GBIN, GMIN, GMAX );

	hname = "aE_vs_theta_ejectile_dc_ejectile";
	htitle = "Gamma-ray energy with addback, gated on the ejectile, Doppler corrected for the ejectile with random subtraction;";
	htitle += "Theta [deg];Energy [keV];Counts per 0.5 keV per strip";
	aE_vs_theta_ejectile_dc_ejectile = new TH2F( hname.data(), htitle.data(), react->GetNumberOfParticleThetas(), react->GetParticleThetas().data(), GBIN, GMIN, GMAX );

	hname = "aE_vs_theta_ejectile_dc_recoil";
	htitle = "Gamma-ray energy with addback, gated on the ejectile, Doppler corrected for the recoil with random subtraction;";
	htitle += "Theta [deg];Energy [keV];Counts per 0.5 keV per strip";
	aE_vs_theta_ejectile_dc_recoil = new TH2F( hname.data(), htitle.data(), react->GetNumberOfParticleThetas(), react->GetParticleThetas().data(), GBIN, GMIN, GMAX );

	hname = "aE_vs_theta_recoil_dc_none";
	htitle = "Gamma-ray energy with addback, gated on the recoil with random subtraction;";
	htitle += "Theta [deg];Energy [keV];Counts per 0.5 keV per strip";
	aE_vs_theta_recoil_dc_none = new TH2F( hname.data(), htitle.data(), react->GetNumberOfParticleThetas(), react->GetParticleThetas().data(), GBIN, GMIN, GMAX );

	hname = "aE_vs_theta_recoil_dc_ejectile";
	htitle = "Gamma-ray energy with addback, gated on the recoil, Doppler corrected for the ejectile with random subtraction;";
	htitle += "Theta [deg];Energy [keV];Counts per 0.5 keV per strip";
	aE_vs_theta_recoil_dc_ejectile = new TH2F( hname.data(), htitle.data(), react->GetNumberOfParticleThetas(), react->GetParticleThetas().data(), GBIN, GMIN, GMAX );

	hname = "aE_vs_theta_recoil_dc_recoil";
	htitle = "Gamma-ray energy with addback, gated on the recoil, Doppler corrected for the recoil with random subtraction;";
	htitle += "Theta [deg];Energy [keV];Counts per 0.5 keV per strip";
	aE_vs_theta_recoil_dc_recoil = new TH2F( hname.data(), htitle.data(), react->GetNumberOfParticleThetas(), react->GetParticleThetas().data(), GBIN, GMIN, GMAX );

	hname = "aE_vs_theta_2p_dc_none";
	htitle = "Gamma-ray energy with addback, in coincidence with ejectile and recoil with random subtraction;";
	htitle += "Theta [deg];Energy [keV];Counts per 0.5 keV per strip";
	aE_vs_theta_2p_dc_none = new TH2F( hname.data(), htitle.data(), react->GetNumberOfParticleThetas(), react->GetParticleThetas().data(), GBIN, GMIN, GMAX );

	hname = "aE_vs_theta_2p_dc_ejectile";
	htitle = "Gamma-ray energy with addback, in coincidence with ejectile and recoil, Doppler corrected for the ejectile with random subtraction;";
	htitle += "Theta [deg];Energy [keV];Counts per 0.5 keV per strip";
	aE_vs_theta_2p_dc_ejectile = new TH2F( hname.data(), htitle.data(), react->GetNumberOfParticleThetas(), react->GetParticleThetas().data(), GBIN, GMIN, GMAX );

	hname = "aE_vs_theta_2p_dc_recoil";
	htitle = "Gamma-ray energy with addback, in coincidence with ejectile and recoil, Doppler corrected for the recoil with random subtraction;";
	htitle += "Theta [deg];Energy [keV];Counts per 0.5 keV per strip";
	aE_vs_theta_2p_dc_recoil = new TH2F( hname.data(), htitle.data(), react->GetNumberOfParticleThetas(), react->GetParticleThetas().data(), GBIN, GMIN, GMAX );

	hname = "aaE_ejectile_dc_none";
	htitle = "Gamma-gamma matrix with addback, gated on the ejectile with random subtraction;";
	htitle += "Gamma-ray Energy [keV];Gamma-ray Energy [keV];Counts per 0.5 keV";
	aaE_ejectile_dc_none = new TH2F( hname.data(), htitle.data(), GBIN, GMIN, GMAX, GBIN, GMIN, GMAX );

	hname = "aaE_ejectile_dc_ejectile";
	htitle = "Gamma-gamma matrix with addback, gated on the ejectile, Doppler corrected for the ejectile with random subtraction;";
	htitle += "Gamma-ray Energy [keV];Gamma-ray Energy [keV];Counts per 0.5 keV";
	aaE_ejectile_dc_ejectile = new TH2F( hname.data(), htitle.data(), GBIN, GMIN, GMAX, GBIN, GMIN, GMAX );

	hname = "aaE_ejectile_dc_recoil";
	htitle = "Gamma-gamma matrix with addback, gated on the ejectile, Doppler corrected for the recoil with random subtraction;";
	htitle += "Gamma-ray Energy [keV];Gamma-ray Energy [keV];Counts per 0.5 keV";
	aaE_ejectile_dc_recoil = new TH2F( hname.data(), htitle.data(), GBIN, GMIN, GMAX, GBIN, GMIN, GMAX );

	hname = "aaE_recoil_dc_none";
	htitle = "Gamma-gamma matrix with addback, gated on the recoil with random subtraction;";
	htitle += "Gamma-ray Energy [keV];Gamma-ray Energy [keV];Counts per 0.5 keV";
	aaE_recoil_dc_none = new TH2F( hname.data(), htitle.data(), GBIN, GMIN, GMAX, GBIN, GMIN, GMAX );

	hname = "aaE_recoil_dc_ejectile";
	htitle = "Gamma-gamma matrix with addback, gated on the recoil, Doppler corrected for the ejectile with random subtraction;";
	htitle += "Gamma-ray Energy [keV];Gamma-ray Energy [keV];Counts per 0.5 keV";
	aaE_recoil_dc_ejectile = new TH2F( hname.data(), htitle.data(), GBIN, GMIN, GMAX, GBIN, GMIN, GMAX );

	hname = "aaE_recoil_dc_recoil";
	htitle = "Gamma-gamma matrix with addback, gated on the recoil, Doppler corrected for the recoil with random subtraction;";
	htitle += "Gamma-ray Energy [keV];Gamma-ray Energy [keV];Counts per 0.5 keV";
	aaE_recoil_dc_recoil = new TH2F( hname.data(), htitle.data(), GBIN, GMIN, GMAX, GBIN, GMIN, GMAX );

	// Segment phi determination
	if( react->HistSegmentPhi() ) {
	
		dirname = "SegmentPhiDetermination";
		output_file->mkdir( dirname.data() );
		output_file->cd( dirname.data() );

		unsigned int nsegs = set->GetNumberOfMiniballClusters();
		nsegs *= set->GetNumberOfMiniballCrystals();
		nsegs *= set->GetNumberOfMiniballSegments();
		
		gE_vs_phi_dc_ejectile.resize( nsegs );
		gE_vs_phi_dc_recoil.resize( nsegs );

		for ( unsigned int i = 0; i < nsegs; i++ ) {
			
			hname = "gE_vs_phi_dc_ejectile_seg";
			hname += std::to_string(i);
			htitle = "Gamma-ray energy versus segment phi angle, Doppler corrected for the ejectile with random subtraction;";
			htitle += "Phi angle of segment " + std::to_string(i) + "[deg];Gamma-ray Energy [keV];Counts";
			gE_vs_phi_dc_ejectile[i] = new TH2F( hname.data(), htitle.data(), nsegs, -0.5, nsegs-0.5, GBIN, GMIN, GMAX );
			
			hname = "gE_vs_phi_dc_recoil_seg";
			hname += std::to_string(i);
			htitle = "Gamma-ray energy versus segment phi angle, Doppler corrected for the recoil with random subtraction;";
			htitle += "Phi angle of segment " + std::to_string(i) + "[deg];Gamma-ray Energy [keV];Counts";
			gE_vs_phi_dc_recoil[i] = new TH2F( hname.data(), htitle.data(), nsegs, -0.5, nsegs-0.5, GBIN, GMIN, GMAX );
			
		}
		
	}
	
	//  Electron-particle coincidences
	if( react->HistElectron() ) {
		
		dirname = "ElectronParticleCoincidences";
		output_file->mkdir( dirname.data() );
		output_file->cd( dirname.data() );
		
		hname = "eE_prompt";
		htitle = "Electron energy in prompt coincide with any particle;Energy [keV];Counts per keV";
		eE_prompt = new TH1F( hname.data(), htitle.data(), EBIN, EMIN, EMAX );
		
		hname = "eE_prompt_1p";
		htitle = "Electron energy in prompt coincide with just 1 particle;Energy [keV];Counts per keV";
		eE_prompt_1p = new TH1F( hname.data(), htitle.data(), EBIN, EMIN, EMAX );
		
		hname = "eE_prompt_2p";
		htitle = "Electron energy in prompt coincide with 2 particles;Energy [keV];Counts per keV";
		eE_prompt_2p = new TH1F( hname.data(), htitle.data(), EBIN, EMIN, EMAX );
		
		hname = "eE_random";
		htitle = "Electron energy in random coincide with any particle;Energy [keV];Counts per keV";
		eE_random = new TH1F( hname.data(), htitle.data(), EBIN, EMIN, EMAX );
		
		hname = "eE_random_1p";
		htitle = "Electron energy in random coincide with just 1 particle;Energy [keV];Counts per keV";
		eE_random_1p = new TH1F( hname.data(), htitle.data(), EBIN, EMIN, EMAX );
		
		hname = "eE_random_2p";
		htitle = "Electron energy in random coincide with 2 particles;Energy [keV];Counts per keV";
		eE_random_2p = new TH1F( hname.data(), htitle.data(), EBIN, EMIN, EMAX );
		
		hname = "eE_ejectile_dc_none";
		htitle = "Electron energy, gated on the ejectile with random subtraction;";
		htitle += "Energy [keV];Counts per keV";
		eE_ejectile_dc_none = new TH1F( hname.data(), htitle.data(), EBIN, EMIN, EMAX );
		
		hname = "eE_ejectile_dc_ejectile";
		htitle = "Electron energy, gated on the ejectile, Doppler corrected for the ejectile with random subtraction;";
		htitle += "Energy [keV];Counts per keV";
		eE_ejectile_dc_ejectile = new TH1F( hname.data(), htitle.data(), EBIN, EMIN, EMAX );
		
		hname = "eE_ejectile_dc_recoil";
		htitle = "Electron energy, gated on the ejectile, Doppler corrected for the recoil with random subtraction;";
		htitle += "Energy [keV];Counts per keV";
		eE_ejectile_dc_recoil = new TH1F( hname.data(), htitle.data(), EBIN, EMIN, EMAX );
		
		hname = "eE_recoil_dc_none";
		htitle = "Electron energy, gated on the recoil with random subtraction;";
		htitle += "Energy [keV];Counts per keV";
		eE_recoil_dc_none = new TH1F( hname.data(), htitle.data(), EBIN, EMIN, EMAX );
		
		hname = "eE_recoil_dc_ejectile";
		htitle = "Electron energy, gated on the recoil, Doppler corrected for the ejectile with random subtraction;";
		htitle += "Energy [keV];Counts per keV";
		eE_recoil_dc_ejectile = new TH1F( hname.data(), htitle.data(), EBIN, EMIN, EMAX );
		
		hname = "eE_recoil_dc_recoil";
		htitle = "Electron energy, gated on the recoil, Doppler corrected for the recoil with random subtraction;";
		htitle += "Energy [keV];Counts per keV";
		eE_recoil_dc_recoil = new TH1F( hname.data(), htitle.data(), EBIN, EMIN, EMAX );
		
		hname = "eE_2p_dc_none";
		htitle = "Electron energy, in coincidence with ejectile and recoil with random subtraction;";
		htitle += "Energy [keV];Counts per keV";
		eE_2p_dc_none = new TH1F( hname.data(), htitle.data(), EBIN, EMIN, EMAX );
		
		hname = "eE_2p_dc_ejectile";
		htitle = "Electron energy, in coincidence with ejectile and recoil, Doppler corrected for the ejectile with random subtraction;";
		htitle += "Energy [keV];Counts per keV";
		eE_2p_dc_ejectile = new TH1F( hname.data(), htitle.data(), EBIN, EMIN, EMAX );
		
		hname = "eE_2p_dc_recoil";
		htitle = "Electron energy, in coincidence with ejectile and recoil, Doppler corrected for the recoil with random subtraction;";
		htitle += "Energy [keV];Counts per keV";
		eE_2p_dc_recoil = new TH1F( hname.data(), htitle.data(), EBIN, EMIN, EMAX );
		
		hname = "eE_vs_theta_ejectile_dc_none";
		htitle = "Electron energy, gated on the ejectile with random subtraction;";
		htitle += "Theta [deg];Energy [keV];Counts per keV per strip";
		eE_vs_theta_ejectile_dc_none = new TH2F( hname.data(), htitle.data(), react->GetNumberOfParticleThetas(), react->GetParticleThetas().data(), EBIN, EMIN, EMAX );
		
		hname = "eE_vs_theta_ejectile_dc_ejectile";
		htitle = "Electron energy, gated on the ejectile, Doppler corrected for the ejectile with random subtraction;";
		htitle += "Theta [deg];Energy [keV];Counts per keV per strip";
		eE_vs_theta_ejectile_dc_ejectile = new TH2F( hname.data(), htitle.data(), react->GetNumberOfParticleThetas(), react->GetParticleThetas().data(), EBIN, EMIN, EMAX );
		
		hname = "eE_vs_theta_ejectile_dc_recoil";
		htitle = "Electron energy, gated on the ejectile, Doppler corrected for the recoil with random subtraction;";
		htitle += "Theta [deg];Energy [keV];Counts per keV per strip";
		eE_vs_theta_ejectile_dc_recoil = new TH2F( hname.data(), htitle.data(), react->GetNumberOfParticleThetas(), react->GetParticleThetas().data(), EBIN, EMIN, EMAX );
		
		hname = "eE_vs_theta_recoil_dc_none";
		htitle = "Electron energy, gated on the recoil with random subtraction;";
		htitle += "Theta [deg];Energy [keV];Counts per keV per strip";
		eE_vs_theta_recoil_dc_none = new TH2F( hname.data(), htitle.data(), react->GetNumberOfParticleThetas(), react->GetParticleThetas().data(), EBIN, EMIN, EMAX );
		
		hname = "eE_vs_theta_recoil_dc_ejectile";
		htitle = "Electron energy, gated on the recoil, Doppler corrected for the ejectile with random subtraction;";
		htitle += "Theta [deg];Energy [keV];Counts per keV per strip";
		eE_vs_theta_recoil_dc_ejectile = new TH2F( hname.data(), htitle.data(), react->GetNumberOfParticleThetas(), react->GetParticleThetas().data(), EBIN, EMIN, EMAX );
		
		hname = "eE_vs_theta_recoil_dc_recoil";
		htitle = "Electron energy, gated on the recoil, Doppler corrected for the recoil with random subtraction;";
		htitle += "Theta [deg];Energy [keV];Counts per keV per strip";
		eE_vs_theta_recoil_dc_recoil = new TH2F( hname.data(), htitle.data(), react->GetNumberOfParticleThetas(), react->GetParticleThetas().data(), EBIN, EMIN, EMAX );
		
		hname = "eE_vs_theta_2p_dc_none";
		htitle = "Electron energy, in coincidence with ejectile and recoil with random subtraction;";
		htitle += "Theta [deg];Energy [keV];Counts per keV per strip";
		eE_vs_theta_2p_dc_none = new TH2F( hname.data(), htitle.data(), react->GetNumberOfParticleThetas(), react->GetParticleThetas().data(), EBIN, EMIN, EMAX );
		
		hname = "eE_vs_theta_2p_dc_ejectile";
		htitle = "Electron energy, in coincidence with ejectile and recoil, Doppler corrected for the ejectile with random subtraction;";
		htitle += "Theta [deg];Energy [keV];Counts per keV per strip";
		eE_vs_theta_2p_dc_ejectile = new TH2F( hname.data(), htitle.data(), react->GetNumberOfParticleThetas(), react->GetParticleThetas().data(), EBIN, EMIN, EMAX );
		
		hname = "eE_vs_theta_2p_dc_recoil";
		htitle = "Electron energy, in coincidence with ejectile and recoil, Doppler corrected for the recoil with random subtraction;";
		htitle += "Theta [deg];Energy [keV];Counts per keV per strip";
		eE_vs_theta_2p_dc_recoil = new TH2F( hname.data(), htitle.data(), react->GetNumberOfParticleThetas(), react->GetParticleThetas().data(), EBIN, EMIN, EMAX );
		
		hname = "eE_costheta_ejectile";
		htitle = "Electron energy versus cos(#theta) of angle between ejectile and electron;";
		htitle += "Energy [keV];cos(#theta_pe)";
		eE_costheta_ejectile = new TH2F( hname.data(), htitle.data(), EBIN, EMIN, EMAX, 100, -1.0, 1.0 );
		
		hname = "eE_costheta_recoil";
		htitle = "Electron energy versus cos(#theta) of angle between recoil and electron;";
		htitle += "Energy [keV];cos(#theta_pe)";
		eE_costheta_recoil = new TH2F( hname.data(), htitle.data(), EBIN, EMIN, EMAX, 100, -1.0, 1.0 );
		
		hname = "eE_vs_ejectile_dc_none_segment";
		htitle = "Electron energy, gated on the ejectile with random subtraction;";
		htitle += "Energy [keV];Ring;Counts per keV per segment";
		eE_vs_ejectile_dc_none_segment = new TH2F( hname.data(), htitle.data(), EBIN, EMIN, EMAX, 24, -0.5, 23.5 );
		
		hname = "eE_vs_ejectile_dc_ejectile_segment";
		htitle = "Electron energy, gated on the ejectile, Doppler corrected for the ejectile with random subtraction;";
		htitle += "Energy [keV];Ring;Counts per keV per ring";
		eE_vs_ejectile_dc_ejectile_segment = new TH2F( hname.data(), htitle.data(), EBIN, EMIN, EMAX, 24, -0.5, 23.5 );
		
		hname = "eE_vs_ejectile_dc_recoil_segment";
		htitle = "Electron energy, gated on the ejectile, Doppler corrected for the recoil with random subtraction;";
		htitle += "Energy [keV];Ring;Counts per keV per ring";
		eE_vs_ejectile_dc_recoil_segment = new TH2F( hname.data(), htitle.data(), EBIN, EMIN, EMAX, 24, -0.5, 23.5 );
		
		hname = "eE_vs_recoil_dc_none_segment";
		htitle = "Electron energy, gated on the recoil with random subtraction;";
		htitle += "Energy [keV];Ring;Counts per keV per ring";
		eE_vs_recoil_dc_none_segment = new TH2F( hname.data(), htitle.data(), EBIN, EMIN, EMAX, 24, -0.5, 23.5 );
		
		hname = "eE_vs_recoil_dc_ejectile_segment";
		htitle = "Electron energy, gated on the recoil, Doppler corrected for the ejectile with random subtraction;";
		htitle += "Energy [keV];Ring;Counts per keV per ring";
		eE_vs_recoil_dc_ejectile_segment = new TH2F( hname.data(), htitle.data(), EBIN, EMIN, EMAX, 24, -0.5, 23.5 );
		
		hname = "eE_vs_recoil_dc_recoil_segment";
		htitle = "Electron energy, gated on the recoil, Doppler corrected for the recoil with random subtraction;";
		htitle += "Energy [keV];Ring;Counts per keV per ring";
		eE_vs_recoil_dc_recoil_segment = new TH2F( hname.data(), htitle.data(), EBIN, EMIN, EMAX, 24, -0.5, 23.5 );
		
		hname = "egE_ejectile_dc_none";
		htitle = "Electron-gamma matrix without addback, gated on the ejectile with random subtraction;";
		htitle += "Electron Energy [keV];Gamma-ray Energy [keV];Counts per 0.5 keV";
		egE_ejectile_dc_none = new TH2F( hname.data(), htitle.data(), EBIN, EMIN, EMAX, GBIN, GMIN, GMAX );
		
		hname = "egE_ejectile_dc_ejectile";
		htitle = "Electron-gamma matrix without addback, gated on the ejectile, Doppler corrected for the ejectile with random subtraction;";
		htitle += "Electron Energy [keV];Gamma-ray Energy [keV];Counts per 0.5 keV";
		egE_ejectile_dc_ejectile = new TH2F( hname.data(), htitle.data(), EBIN, EMIN, EMAX, GBIN, GMIN, GMAX );
		
		hname = "egE_ejectile_dc_recoil";
		htitle = "Electron-gamma matrix without addback, gated on the ejectile, Doppler corrected for the recoil with random subtraction;";
		htitle += "Electron Energy [keV];Gamma-ray Energy [keV];Counts per 0.5 keV";
		egE_ejectile_dc_recoil = new TH2F( hname.data(), htitle.data(), EBIN, EMIN, EMAX, GBIN, GMIN, GMAX );
		
		hname = "egE_recoil_dc_none";
		htitle = "Electron-gamma matrix without addback, gated on the recoil with random subtraction;";
		htitle += "Electron Energy [keV];Gamma-ray Energy [keV];Counts per 0.5 keV";
		egE_recoil_dc_none = new TH2F( hname.data(), htitle.data(), EBIN, EMIN, EMAX, GBIN, GMIN, GMAX );
		
		hname = "egE_recoil_dc_ejectile";
		htitle = "Electron-gamma matrix without addback, gated on the recoil, Doppler corrected for the ejectile with random subtraction;";
		htitle += "Electron Energy [keV];Gamma-ray Energy [keV];Counts per 0.5 keV";
		egE_recoil_dc_ejectile = new TH2F( hname.data(), htitle.data(), EBIN, EMIN, EMAX, GBIN, GMIN, GMAX );
		
		hname = "egE_recoil_dc_recoil";
		htitle = "Electron-gamma matrix without addback, gated on the recoil, Doppler corrected for the recoil with random subtraction;";
		htitle += "Electron Energy [keV];Gamma-ray Energy [keV];Counts per 0.5 keV";
		egE_recoil_dc_recoil = new TH2F( hname.data(), htitle.data(), EBIN, EMIN, EMAX, GBIN, GMIN, GMAX );
		
		hname = "eaE_ejectile_dc_none";
		htitle = "Electron-gamma matrix with addback, gated on the ejectile with random subtraction;";
		htitle += "Electron Energy [keV];Gamma-ray Energy [keV];Counts per 0.5 keV";
		eaE_ejectile_dc_none = new TH2F( hname.data(), htitle.data(), EBIN, EMIN, EMAX, GBIN, GMIN, GMAX );
		
		hname = "eaE_ejectile_dc_ejectile";
		htitle = "Electron-gamma matrix with addback, gated on the ejectile, Doppler corrected for the ejectile with random subtraction;";
		htitle += "Electron Energy [keV];Gamma-ray Energy [keV];Counts per 0.5 keV";
		eaE_ejectile_dc_ejectile = new TH2F( hname.data(), htitle.data(), EBIN, EMIN, EMAX, GBIN, GMIN, GMAX );
		
		hname = "eaE_ejectile_dc_recoil";
		htitle = "Electron-gamma matrix with addback, gated on the ejectile, Doppler corrected for the recoil with random subtraction;";
		htitle += "Electron Energy [keV];Gamma-ray Energy [keV];Counts per 0.5 keV";
		eaE_ejectile_dc_recoil = new TH2F( hname.data(), htitle.data(), EBIN, EMIN, EMAX, GBIN, GMIN, GMAX );
		
		hname = "eaE_recoil_dc_none";
		htitle = "Electron-gamma matrix with addback, gated on the recoil with random subtraction;";
		htitle += "Electron Energy [keV];Gamma-ray Energy [keV];Counts per 0.5 keV";
		eaE_recoil_dc_none = new TH2F( hname.data(), htitle.data(), EBIN, EMIN, EMAX, GBIN, GMIN, GMAX );
		
		hname = "eaE_recoil_dc_ejectile";
		htitle = "Electron-gamma matrix with addback, gated on the recoil, Doppler corrected for the ejectile with random subtraction;";
		htitle += "Electron Energy [keV];Gamma-ray Energy [keV];Counts per 0.5 keV";
		eaE_recoil_dc_ejectile = new TH2F( hname.data(), htitle.data(), EBIN, EMIN, EMAX, GBIN, GMIN, GMAX );
		
		hname = "eaE_recoil_dc_recoil";
		htitle = "Electron-gamma matrix with addback, gated on the recoil, Doppler corrected for the recoil with random subtraction;";
		htitle += "Electron Energy [keV];Gamma-ray Energy [keV];Counts per 0.5 keV";
		eaE_recoil_dc_recoil = new TH2F( hname.data(), htitle.data(), EBIN, EMIN, EMAX, GBIN, GMIN, GMAX );
		
	} // electrons on

	// Beam dump histograms
	if( react->HistBeamDump() ) {
		
		dirname = "BeamDump";
		output_file->mkdir( dirname.data() );
		output_file->cd( dirname.data() );
		
		hname = "bdE_singles";
		htitle = "Beam-dump gamma-ray energy singles;Energy [keV];Counts per 0.5 keV";
		bdE_singles = new TH1F( hname.data(), htitle.data(), GBIN, GMIN, GMAX );
		
		hname = "bd_bd_td";
		htitle = "Beam-dump - Beam-dump time difference;#Deltat [ns];Counts per 10 ns";
		bd_bd_td = new TH1F( hname.data(), htitle.data(),
							TBIN, TMIN, TMAX );
		
		hname = "bdE_bdE";
		htitle = "Beam-dump gamma-ray coincidence matrix;Energy [keV];Energy [keV];Counts per 0.5 keV";
		bdE_bdE = new TH2F( hname.data(), htitle.data(), GBIN, GMIN, GMAX, GBIN, GMIN, GMAX );
		
		bdE_singles_det.resize( set->GetNumberOfBeamDumpDetectors() );
		for( unsigned int i = 0; i < set->GetNumberOfBeamDumpDetectors(); ++i ){
			
			hname = "bdE_singles_det" + std::to_string(i);
			htitle  = "Beam-dump gamma-ray energy singles in detector ";
			htitle += std::to_string(i);
			htitle += ";Energy [keV];Counts per 0.5 keV";
			bdE_singles_det[i] = new TH1F( hname.data(), htitle.data(), GBIN, GMIN, GMAX );
			
		}
		
	} // beam dump on
	
	// Ionisation chamber histograms
	if( react->HistIonChamber() ) {
		
		dirname = "IonChamber";
		output_file->mkdir( dirname.data() );
		output_file->cd( dirname.data() );
		
		hname = "ic_dE";
		htitle = "Ionisation chamber;Energy loss in dE layers (Gas) (arb. units);Counts";
		ic_dE = new TH1F( hname.data(), htitle.data(), 4096, 0, 10000 );
		
		hname = "ic_E";
		htitle = "Ionisation chamber;Energy loss in rest of the layers (Si) (Gas) (arb. units);Counts";
		ic_E = new TH1F( hname.data(), htitle.data(), 4096, 0, 10000 );
		
		hname = "ic_dE_E";
		htitle = "Ionisation chamber;Energy loss in dE layers (Gas) (arb. units);Energy loss in rest of the layers (Si) (arb. units);Counts";
		ic_dE_E = new TH2F( hname.data(), htitle.data(), 4096, 0, 10000, 4096, 0, 10000 );
		
	} // ion-chamber on
	
	return;
	
}

// Reset histograms in the DataSpy
void MiniballHistogrammer::ResetHists() {
	
	
	ebis_td_gamma->Reset("ICESM");
	ebis_td_particle->Reset("ICESM");
	gamma_particle_td->Reset("ICESM");
	gamma_particle_E_vs_td->Reset("ICESM");
	gamma_gamma_td->Reset("ICESM");
	gamma_electron_td->Reset("ICESM");
	electron_electron_td->Reset("ICESM");
	electron_particle_td->Reset("ICESM");
	particle_particle_td->Reset("ICESM");
	gE_singles->Reset("ICESM");
	gE_singles_ebis->Reset("ICESM");
	gE_singles_ebis_on->Reset("ICESM");
	gE_singles_ebis_off->Reset("ICESM");
	aE_singles->Reset("ICESM");
	aE_singles_ebis->Reset("ICESM");
	aE_singles_ebis_on->Reset("ICESM");
	aE_singles_ebis_off->Reset("ICESM");
	gamma_xy_map_forward->Reset("ICESM");
	gamma_xy_map_backward->Reset("ICESM");
	gamma_xz_map_left->Reset("ICESM");
	gamma_xz_map_right->Reset("ICESM");
	gamma_theta_phi_map->Reset("ICESM");
	pE_theta->Reset("ICESM");
	pE_theta_coinc->Reset("ICESM");
	pE_theta_ejectile->Reset("ICESM");
	pE_theta_recoil->Reset("ICESM");
	pBeta_theta_ejectile->Reset("ICESM");
	pBeta_theta_recoil->Reset("ICESM");
	particle_xy_map_forward->Reset("ICESM");
	particle_xy_map_backward->Reset("ICESM");
	particle_theta_phi_map->Reset("ICESM");
	gE_prompt->Reset("ICESM");
	gE_prompt_1p->Reset("ICESM");
	gE_prompt_2p->Reset("ICESM");
	gE_random->Reset("ICESM");
	gE_random_1p->Reset("ICESM");
	gE_random_2p->Reset("ICESM");
	gE_ejectile_dc_none->Reset("ICESM");
	gE_ejectile_dc_ejectile->Reset("ICESM");
	gE_ejectile_dc_recoil->Reset("ICESM");
	gE_recoil_dc_none->Reset("ICESM");
	gE_recoil_dc_ejectile->Reset("ICESM");
	gE_recoil_dc_recoil->Reset("ICESM");
	gE_2p_dc_none->Reset("ICESM");
	gE_2p_dc_ejectile->Reset("ICESM");
	gE_2p_dc_recoil->Reset("ICESM");
	gE_costheta_ejectile->Reset("ICESM");
	gE_costheta_recoil->Reset("ICESM");
	gE_vs_theta_ejectile_dc_none->Reset("ICESM");
	gE_vs_theta_ejectile_dc_ejectile->Reset("ICESM");
	gE_vs_theta_ejectile_dc_recoil->Reset("ICESM");
	gE_vs_theta_recoil_dc_none->Reset("ICESM");
	gE_vs_theta_recoil_dc_ejectile->Reset("ICESM");
	gE_vs_theta_recoil_dc_recoil->Reset("ICESM");
	gE_vs_theta_2p_dc_none->Reset("ICESM");
	gE_vs_theta_2p_dc_ejectile->Reset("ICESM");
	gE_vs_theta_2p_dc_recoil->Reset("ICESM");
	aE_prompt->Reset("ICESM");
	aE_prompt_1p->Reset("ICESM");
	aE_prompt_2p->Reset("ICESM");
	aE_random->Reset("ICESM");
	aE_random_1p->Reset("ICESM");
	aE_random_2p->Reset("ICESM");
	aE_ejectile_dc_none->Reset("ICESM");
	aE_ejectile_dc_ejectile->Reset("ICESM");
	aE_ejectile_dc_recoil->Reset("ICESM");
	aE_recoil_dc_none->Reset("ICESM");
	aE_recoil_dc_ejectile->Reset("ICESM");
	aE_recoil_dc_recoil->Reset("ICESM");
	aE_2p_dc_none->Reset("ICESM");
	aE_2p_dc_ejectile->Reset("ICESM");
	aE_2p_dc_recoil->Reset("ICESM");
	aE_costheta_ejectile->Reset("ICESM");
	aE_costheta_recoil->Reset("ICESM");
	aE_vs_theta_ejectile_dc_none->Reset("ICESM");
	aE_vs_theta_ejectile_dc_ejectile->Reset("ICESM");
	aE_vs_theta_ejectile_dc_recoil->Reset("ICESM");
	aE_vs_theta_recoil_dc_none->Reset("ICESM");
	aE_vs_theta_recoil_dc_ejectile->Reset("ICESM");
	aE_vs_theta_recoil_dc_recoil->Reset("ICESM");
	aE_vs_theta_2p_dc_none->Reset("ICESM");
	aE_vs_theta_2p_dc_ejectile->Reset("ICESM");
	aE_vs_theta_2p_dc_recoil->Reset("ICESM");
	
	// Gamma-gamma hists
	if( react->HistGammaGamma() ) {
		
		gE_gE->Reset("ICESM");
		gE_gE_ebis_on->Reset("ICESM");
		aE_aE->Reset("ICESM");
		aE_aE_ebis_on->Reset("ICESM");
		ggE_ejectile_dc_none->Reset("ICESM");
		ggE_ejectile_dc_ejectile->Reset("ICESM");
		ggE_ejectile_dc_recoil->Reset("ICESM");
		ggE_recoil_dc_none->Reset("ICESM");
		ggE_recoil_dc_ejectile->Reset("ICESM");
		ggE_recoil_dc_recoil->Reset("ICESM");
		aaE_ejectile_dc_none->Reset("ICESM");
		aaE_ejectile_dc_ejectile->Reset("ICESM");
		aaE_ejectile_dc_recoil->Reset("ICESM");
		aaE_recoil_dc_none->Reset("ICESM");
		aaE_recoil_dc_ejectile->Reset("ICESM");
		aaE_recoil_dc_recoil->Reset("ICESM");

	} // gamma-gamma
	
	// Segment phi determination
	if( react->HistSegmentPhi() ) {
		
		for( unsigned int i = 0; i < gE_vs_phi_dc_ejectile.size(); i++ )
			gE_vs_phi_dc_ejectile[i]->Reset("ICESM");
		
		for( unsigned int i = 0; i < gE_vs_phi_dc_recoil.size(); i++ )
			gE_vs_phi_dc_recoil[i]->Reset("ICESM");
		
	} // segment phi

	//  Electron-particle singles and coincidences
	if( react->HistElectron() ) {
		
		eE_singles->Reset("ICESM");
		eE_singles_ebis->Reset("ICESM");
		eE_singles_ebis_on->Reset("ICESM");
		eE_singles_ebis_off->Reset("ICESM");
		electron_xy_map->Reset("ICESM");

		eE_prompt->Reset("ICESM");
		eE_prompt_1p->Reset("ICESM");
		eE_prompt_2p->Reset("ICESM");
		eE_random->Reset("ICESM");
		eE_random_1p->Reset("ICESM");
		eE_random_2p->Reset("ICESM");
		eE_ejectile_dc_none->Reset("ICESM");
		eE_ejectile_dc_ejectile->Reset("ICESM");
		eE_ejectile_dc_recoil->Reset("ICESM");
		eE_recoil_dc_none->Reset("ICESM");
		eE_recoil_dc_ejectile->Reset("ICESM");
		eE_recoil_dc_recoil->Reset("ICESM");
		eE_2p_dc_none->Reset("ICESM");
		eE_2p_dc_ejectile->Reset("ICESM");
		eE_2p_dc_recoil->Reset("ICESM");
		eE_vs_theta_ejectile_dc_none->Reset("ICESM");
		eE_vs_theta_ejectile_dc_ejectile->Reset("ICESM");
		eE_vs_theta_ejectile_dc_recoil->Reset("ICESM");
		eE_vs_theta_recoil_dc_none->Reset("ICESM");
		eE_vs_theta_recoil_dc_ejectile->Reset("ICESM");
		eE_vs_theta_recoil_dc_recoil->Reset("ICESM");
		eE_vs_theta_2p_dc_none->Reset("ICESM");
		eE_vs_theta_2p_dc_ejectile->Reset("ICESM");
		eE_vs_theta_2p_dc_recoil->Reset("ICESM");
		eE_costheta_ejectile->Reset("ICESM");
		eE_costheta_recoil->Reset("ICESM");
		eE_vs_ejectile_dc_none_segment->Reset("ICESM");
		eE_vs_ejectile_dc_ejectile_segment->Reset("ICESM");
		eE_vs_ejectile_dc_recoil_segment->Reset("ICESM");
		eE_vs_recoil_dc_none_segment->Reset("ICESM");
		eE_vs_recoil_dc_ejectile_segment->Reset("ICESM");
		eE_vs_recoil_dc_recoil_segment->Reset("ICESM");

		eE_eE->Reset("ICESM");
		eE_eE_ebis_on->Reset("ICESM");
	
		if( react->HistElectron() ) {
			
			gE_eE->Reset("ICESM");
			gE_eE_ebis_on->Reset("ICESM");
			aE_eE->Reset("ICESM");
			aE_eE_ebis_on->Reset("ICESM");
			egE_ejectile_dc_none->Reset("ICESM");
			egE_ejectile_dc_ejectile->Reset("ICESM");
			egE_ejectile_dc_recoil->Reset("ICESM");
			egE_recoil_dc_none->Reset("ICESM");
			egE_recoil_dc_ejectile->Reset("ICESM");
			egE_recoil_dc_recoil->Reset("ICESM");
			eaE_ejectile_dc_none->Reset("ICESM");
			eaE_ejectile_dc_ejectile->Reset("ICESM");
			eaE_ejectile_dc_recoil->Reset("ICESM");
			eaE_recoil_dc_none->Reset("ICESM");
			eaE_recoil_dc_ejectile->Reset("ICESM");
			eaE_recoil_dc_recoil->Reset("ICESM");
			
		}
		
	} // electron hists

	// Ionisation chamber
	if( react->HistIonChamber() ) {
		
		ic_dE->Reset("ICESM");
		ic_E->Reset("ICESM");
		ic_dE_E->Reset("ICESM");
	
	} // ion chamber
	
	// Beam dump
	if( react->HistBeamDump() ) {
		
		bdE_singles->Reset("ICESM");
		bd_bd_td->Reset("ICESM");
		bdE_bdE->Reset("ICESM");
		
		for( unsigned int i = 0; i < set->GetNumberOfBeamDumpDetectors(); ++i )
			bdE_singles_det[i]->Reset("ICESM");
		
	} // beam dump
	
	for( unsigned int i = 0; i < set->GetNumberOfCDDetectors(); ++i ){
		for( unsigned int j = 0; j < set->GetNumberOfCDSectors(); ++j ){
			pE_dE[i][j]->Reset("ICESM");
			pE_dE_coinc[i][j]->Reset("ICESM");
			pE_dE_cut[i][j]->Reset("ICESM");
		}
	}
	
	
	return;
	
}
// Particle-Gamma coincidences without addback
void MiniballHistogrammer::FillParticleGammaHists( std::shared_ptr<GammaRayEvt> g ) {

	// Work out the weight if it's prompt or random
	bool prompt = false;
	float weight;
	if( PromptCoincidence( g, react->GetParticleTime() ) ) {
		prompt = true;
		weight = 1.0;
	}
	else if( RandomCoincidence( g, react->GetParticleTime() ) ){
		weight = -1.0 * react->GetParticleGammaFillRatio();
	}
	else return; // outside of either window, quit now
	
	// Plot the prompt and random gamma spectra
	if( prompt ) gE_prompt->Fill( g->GetEnergy() );
	else gE_random->Fill( g->GetEnergy() );
	
	// Same again but explicitly 1 particle events
	if( prompt && ( react->IsEjectileDetected() != react->IsRecoilDetected() ) )
		gE_prompt_1p->Fill( g->GetEnergy() );
	else if( react->IsEjectileDetected() != react->IsRecoilDetected() )
		gE_random_1p->Fill( g->GetEnergy() );

	// Ejectile-gated spectra
	if( react->IsEjectileDetected() ) {
		
		gE_costheta_ejectile->Fill( g->GetEnergy(), react->CosTheta( g, true ), weight );

		gE_ejectile_dc_none->Fill( g->GetEnergy(), weight );
		gE_ejectile_dc_ejectile->Fill( react->DopplerCorrection( g, true ), weight );
		gE_ejectile_dc_recoil->Fill( react->DopplerCorrection( g, false ), weight );

		gE_vs_theta_ejectile_dc_none->Fill( react->GetEjectile()->GetTheta() * TMath::RadToDeg(), g->GetEnergy(), weight );
		gE_vs_theta_ejectile_dc_ejectile->Fill( react->GetEjectile()->GetTheta() * TMath::RadToDeg(), react->DopplerCorrection( g, true ), weight );
		gE_vs_theta_ejectile_dc_recoil->Fill( react->GetEjectile()->GetTheta() * TMath::RadToDeg(), react->DopplerCorrection( g, false ), weight );

	}

	// Recoil-gated spectra
	if( react->IsRecoilDetected() || react->IsTransferDetected() ) {
		
		gE_costheta_recoil->Fill( g->GetEnergy(), react->CosTheta( g, false ), weight );

		gE_recoil_dc_none->Fill( g->GetEnergy(), weight );
		gE_recoil_dc_ejectile->Fill( react->DopplerCorrection( g, true ), weight );
		gE_recoil_dc_recoil->Fill( react->DopplerCorrection( g, false ), weight );

		gE_vs_theta_recoil_dc_none->Fill( react->GetRecoil()->GetTheta() * TMath::RadToDeg(), g->GetEnergy(), weight );
		gE_vs_theta_recoil_dc_ejectile->Fill( react->GetRecoil()->GetTheta() * TMath::RadToDeg(), react->DopplerCorrection( g, true ), weight );
		gE_vs_theta_recoil_dc_recoil->Fill( react->GetRecoil()->GetTheta() * TMath::RadToDeg(), react->DopplerCorrection( g, false ), weight );

	}
	
	// Two-particle spectra
	if( react->IsEjectileDetected() && react->IsRecoilDetected() ){
		
		// Prompt and random spectra
		if( prompt ) gE_prompt_2p->Fill( g->GetEnergy() );
		else gE_random_2p->Fill( g->GetEnergy() );

		gE_2p_dc_none->Fill( g->GetEnergy(), weight );
		gE_2p_dc_ejectile->Fill( react->DopplerCorrection( g, true ), weight );
		gE_2p_dc_recoil->Fill( react->DopplerCorrection( g, false ), weight );

		gE_vs_theta_2p_dc_none->Fill( react->GetRecoil()->GetTheta() * TMath::RadToDeg(), g->GetEnergy(), weight );
		gE_vs_theta_2p_dc_ejectile->Fill( react->GetRecoil()->GetTheta() * TMath::RadToDeg(), react->DopplerCorrection( g, true ), weight );
		gE_vs_theta_2p_dc_recoil->Fill( react->GetRecoil()->GetTheta() * TMath::RadToDeg(), react->DopplerCorrection( g, false ), weight );
		
	}
	
	// Segment-phi determination
	if( react->HistSegmentPhi() ) {
		
		for( unsigned int i = 0; i < 360; i ++ ) {
			
			double gphi_deg = (double)i;
			double gphi = gphi_deg * TMath::DegToRad();
			double gtheta = react->GetGammaTheta(g);
			
			unsigned short segID = g->GetCluster();
			segID *= set->GetNumberOfMiniballCrystals() * set->GetNumberOfMiniballSegments();
			segID += set->GetNumberOfMiniballSegments() * g->GetCrystal();
			segID += g->GetSegment();
	
			// Ejectile DC
			double dc_gen = react->DopplerCorrection( g->GetEnergy(), gtheta, gphi, true );
			gE_vs_phi_dc_ejectile[segID]->Fill( gphi_deg, dc_gen, weight );

			// Recoil DC
			dc_gen = react->DopplerCorrection( g->GetEnergy(), gtheta, gphi, false );
			gE_vs_phi_dc_recoil[segID]->Fill( gphi_deg, dc_gen, weight );

		}
		
	}
	
	return;

}

// Particle-Gamma coincidences with addback
void MiniballHistogrammer::FillParticleGammaHists( std::shared_ptr<GammaRayAddbackEvt> g ) {

	// Work out the weight if it's prompt or random
	bool prompt = false;
	float weight;
	if( PromptCoincidence( g, react->GetParticleTime() ) ) {
		prompt = true;
		weight = 1.0;
	}
	else if( RandomCoincidence( g, react->GetParticleTime() ) ){
		weight = -1.0 * react->GetParticleGammaFillRatio();
	}
	else return; // outside of either window, quit now
	
	// Plot the prompt and random gamma spectra
	if( prompt ) aE_prompt->Fill( g->GetEnergy() );
	else aE_random->Fill( g->GetEnergy() );
	
	// Same again but explicitly 1 particle events
	if( prompt && ( react->IsEjectileDetected() != react->IsRecoilDetected() ) )
		aE_prompt_1p->Fill( g->GetEnergy() );
	else if( react->IsEjectileDetected() != react->IsRecoilDetected() )
		aE_random_1p->Fill( g->GetEnergy() );
	
	// Ejectile-gated spectra
	if( react->IsEjectileDetected() ) {
		
		aE_costheta_ejectile->Fill( g->GetEnergy(), react->CosTheta( g, true ), weight );

		aE_ejectile_dc_none->Fill( g->GetEnergy(), weight );
		aE_ejectile_dc_ejectile->Fill( react->DopplerCorrection( g, true ), weight );
		aE_ejectile_dc_recoil->Fill( react->DopplerCorrection( g, false ), weight );

		aE_vs_theta_ejectile_dc_none->Fill( react->GetEjectile()->GetTheta() * TMath::RadToDeg(), g->GetEnergy(), weight );
		aE_vs_theta_ejectile_dc_ejectile->Fill( react->GetEjectile()->GetTheta() * TMath::RadToDeg(), react->DopplerCorrection( g, true ), weight );
		aE_vs_theta_ejectile_dc_recoil->Fill( react->GetEjectile()->GetTheta() * TMath::RadToDeg(), react->DopplerCorrection( g, false ), weight );

	}

	// Recoil-gated spectra
	if( react->IsRecoilDetected() || react->IsTransferDetected() ) {
		
		aE_costheta_recoil->Fill( g->GetEnergy(), react->CosTheta( g, false ), weight );

		aE_recoil_dc_none->Fill( g->GetEnergy(), weight );
		aE_recoil_dc_ejectile->Fill( react->DopplerCorrection( g, true ), weight );
		aE_recoil_dc_recoil->Fill( react->DopplerCorrection( g, false ), weight );

		aE_vs_theta_recoil_dc_none->Fill( react->GetRecoil()->GetTheta() * TMath::RadToDeg(), g->GetEnergy(), weight );
		aE_vs_theta_recoil_dc_ejectile->Fill( react->GetRecoil()->GetTheta() * TMath::RadToDeg(), react->DopplerCorrection( g, true ), weight );
		aE_vs_theta_recoil_dc_recoil->Fill( react->GetRecoil()->GetTheta() * TMath::RadToDeg(), react->DopplerCorrection( g, false ), weight );

	}
	
	// Two-particle spectra
	if( react->IsEjectileDetected() && react->IsRecoilDetected() ){
		
		// Prompt and random spectra
		if( prompt ) aE_prompt_2p->Fill( g->GetEnergy() );
		else aE_random_2p->Fill( g->GetEnergy() );

		aE_2p_dc_none->Fill( g->GetEnergy(), weight );
		aE_2p_dc_ejectile->Fill( react->DopplerCorrection( g, true ), weight );
		aE_2p_dc_recoil->Fill( react->DopplerCorrection( g, false ), weight );

		aE_vs_theta_2p_dc_none->Fill( react->GetRecoil()->GetTheta() * TMath::RadToDeg(), g->GetEnergy(), weight );
		aE_vs_theta_2p_dc_ejectile->Fill( react->GetRecoil()->GetTheta() * TMath::RadToDeg(), react->DopplerCorrection( g, true ), weight );
		aE_vs_theta_2p_dc_recoil->Fill( react->GetRecoil()->GetTheta() * TMath::RadToDeg(), react->DopplerCorrection( g, false ), weight );
		
	}
	
	return;

}

// Particle-Electron coincidences with addback
void MiniballHistogrammer::FillParticleElectronHists( std::shared_ptr<SpedeEvt> e ) {

	// Work out the weight if it's prompt or random
	bool prompt = false;
	float weight;
	if( PromptCoincidence( e, react->GetParticleTime() ) ) {
		prompt = true;
		weight = 1.0;
	}
	else if( RandomCoincidence( e, react->GetParticleTime() ) ){
		weight = -1.0 * react->GetParticleElectronFillRatio();
	}
	else return; // outside of either window, quit now
	
	// Plot the prompt and random gamma spectra
	if( prompt ) eE_prompt->Fill( e->GetEnergy() );
	else eE_random->Fill( e->GetEnergy() );
	
	// Same again but explicitly 1 particle events
	if( prompt && ( react->IsEjectileDetected() != react->IsRecoilDetected() ) )
		eE_prompt_1p->Fill( e->GetEnergy() );
	else if( react->IsEjectileDetected() != react->IsRecoilDetected() )
		eE_random_1p->Fill( e->GetEnergy() );

	// Ejectile-gated spectra
	if( react->IsEjectileDetected() ) {
		
		eE_costheta_ejectile->Fill( e->GetEnergy(), react->CosTheta( e, true ), weight );
		
		eE_ejectile_dc_none->Fill( e->GetEnergy(), weight );
		eE_ejectile_dc_ejectile->Fill( react->DopplerCorrection( e, true ), weight );
		eE_ejectile_dc_recoil->Fill( react->DopplerCorrection( e, false ), weight );

		eE_vs_theta_ejectile_dc_none->Fill( react->GetEjectile()->GetTheta() * TMath::RadToDeg(), e->GetEnergy(), weight );
		eE_vs_theta_ejectile_dc_ejectile->Fill( react->GetEjectile()->GetTheta() * TMath::RadToDeg(), react->DopplerCorrection( e, true ), weight );
		eE_vs_theta_ejectile_dc_recoil->Fill( react->GetEjectile()->GetTheta() * TMath::RadToDeg(), react->DopplerCorrection( e, false ), weight );
    
		eE_vs_ejectile_dc_none_segment->Fill( e->GetEnergy(), e->GetSegment(), weight );
		eE_vs_ejectile_dc_ejectile_segment->Fill( react->DopplerCorrection( e, true ), e->GetSegment(), weight );
		eE_vs_ejectile_dc_recoil_segment->Fill( react->DopplerCorrection( e, false ), e->GetSegment(), weight );

	}

	// Recoil-gated spectra
	if( react->IsRecoilDetected() || react->IsTransferDetected() ) {
		
		eE_costheta_recoil->Fill( e->GetEnergy(), react->CosTheta( e, false ), weight );
		
		eE_recoil_dc_none->Fill( e->GetEnergy(), weight );
		eE_recoil_dc_ejectile->Fill( react->DopplerCorrection( e, true ), weight );
		eE_recoil_dc_recoil->Fill( react->DopplerCorrection( e, false ), weight );

		eE_vs_theta_recoil_dc_none->Fill( react->GetRecoil()->GetTheta() * TMath::RadToDeg(), e->GetEnergy(), weight );
		eE_vs_theta_recoil_dc_ejectile->Fill( react->GetRecoil()->GetTheta() * TMath::RadToDeg(), react->DopplerCorrection( e, true ), weight );
		eE_vs_theta_recoil_dc_recoil->Fill( react->GetRecoil()->GetTheta() * TMath::RadToDeg(), react->DopplerCorrection( e, false ), weight );
    
		eE_vs_recoil_dc_none_segment->Fill( e->GetEnergy(), e->GetSegment(), weight );
		eE_vs_recoil_dc_ejectile_segment->Fill( react->DopplerCorrection( e, true ), e->GetSegment(), weight );
		eE_vs_recoil_dc_recoil_segment->Fill( react->DopplerCorrection( e, false ), e->GetSegment(), weight );
 
	}
	
	// Two-particle spectra
	if( react->IsEjectileDetected() && react->IsRecoilDetected() ){
		
		// Prompt and random spectra
		if( prompt ) eE_prompt_2p->Fill( e->GetEnergy() );
		else eE_random_2p->Fill( e->GetEnergy() );

		eE_2p_dc_none->Fill( e->GetEnergy(), weight );
		eE_2p_dc_ejectile->Fill( react->DopplerCorrection( e, true ), weight );
		eE_2p_dc_recoil->Fill( react->DopplerCorrection( e, false ), weight );

		eE_vs_theta_2p_dc_none->Fill( react->GetRecoil()->GetTheta() * TMath::RadToDeg(), e->GetEnergy(), weight );
		eE_vs_theta_2p_dc_ejectile->Fill( react->GetRecoil()->GetTheta() * TMath::RadToDeg(), react->DopplerCorrection( e, true ), weight );
		eE_vs_theta_2p_dc_recoil->Fill( react->GetRecoil()->GetTheta() * TMath::RadToDeg(), react->DopplerCorrection( e, false ), weight );
		
	}
	
	return;

}

// Particle-Gamma-Gamma coincidences without addback
void MiniballHistogrammer::FillParticleGammaGammaHists( std::shared_ptr<GammaRayEvt> g1, std::shared_ptr<GammaRayEvt> g2 ) {

	// Work out the weight if it's prompt or random
	float weight;
	if( PromptCoincidence( g1, react->GetParticleTime() ) ) {
		weight = 1.0;
	}
	else if( RandomCoincidence( g1, react->GetParticleTime() ) ){
		weight = -1.0 * react->GetParticleGammaFillRatio();
	}
	else return; // outside of either window, quit now

	// Ejectile-gated spectra
	if( react->IsEjectileDetected() ) {
		
		// Gamma-gamma
		ggE_ejectile_dc_none->Fill( g1->GetEnergy(), g2->GetEnergy(), weight );
		ggE_ejectile_dc_ejectile->Fill( react->DopplerCorrection( g1, true ), react->DopplerCorrection( g2, true ), weight );
		ggE_ejectile_dc_recoil->Fill( react->DopplerCorrection( g1, false ), react->DopplerCorrection( g2, false ), weight );
		
	}
	
	// Recoil-gated spectra
	if( react->IsRecoilDetected() || react->IsTransferDetected() ) {
		
		// Gamma-gamma
		ggE_recoil_dc_none->Fill( g1->GetEnergy(), g2->GetEnergy(), weight );
		ggE_recoil_dc_ejectile->Fill( react->DopplerCorrection( g1, true ), react->DopplerCorrection( g2, true ), weight );
		ggE_recoil_dc_recoil->Fill( react->DopplerCorrection( g1, false ), react->DopplerCorrection( g2, false ), weight );

	}
	
	return;
	
}

// Particle-Gamma-Gamma coincidences with addback
void MiniballHistogrammer::FillParticleGammaGammaHists( std::shared_ptr<GammaRayAddbackEvt> g1, std::shared_ptr<GammaRayAddbackEvt> g2 ) {

	// Work out the weight if it's prompt or random
	float weight;
	if( PromptCoincidence( g1, react->GetParticleTime() ) ) {
		weight = 1.0;
	}
	else if( RandomCoincidence( g1, react->GetParticleTime() ) ){
		weight = -1.0 * react->GetParticleGammaFillRatio();
	}
	else return; // outside of either window, quit now

	// Ejectile-gated spectra
	if( react->IsEjectileDetected() ) {
		
		// Gamma-gamma
		aaE_ejectile_dc_none->Fill( g1->GetEnergy(), g2->GetEnergy(), weight );
		aaE_ejectile_dc_ejectile->Fill( react->DopplerCorrection( g1, true ), react->DopplerCorrection( g2, true ), weight );
		aaE_ejectile_dc_recoil->Fill( react->DopplerCorrection( g1, false ), react->DopplerCorrection( g2, false ), weight );

	}
	
	// Recoil-gated spectra
	if( react->IsRecoilDetected() || react->IsTransferDetected() ) {
		
		// Gamma-gamma
		aaE_recoil_dc_none->Fill( g1->GetEnergy(), g2->GetEnergy(), weight );
		aaE_recoil_dc_ejectile->Fill( react->DopplerCorrection( g1, true ), react->DopplerCorrection( g2, true ), weight );
		aaE_recoil_dc_recoil->Fill( react->DopplerCorrection( g1, false ), react->DopplerCorrection( g2, false ), weight );

	}

	return;
	
}

// Particle-Electron-Gamma coincidences without addback
void MiniballHistogrammer::FillParticleElectronGammaHists( std::shared_ptr<SpedeEvt> e, std::shared_ptr<GammaRayEvt> g ) {

	// Work out the weight if it's prompt or random
	float weight;
	if( PromptCoincidence( e, react->GetParticleTime() ) ) {
		weight = 1.0;
	}
	else if( RandomCoincidence( e, react->GetParticleTime() ) ){
		weight = -1.0 * react->GetParticleElectronFillRatio();
	}
	else return; // outside of either window, quit now

	// Ejectile-gated spectra
	if( react->IsEjectileDetected() ) {
		
		// Electon-gamma
		egE_ejectile_dc_none->Fill( e->GetEnergy(), g->GetEnergy(), weight );
		egE_ejectile_dc_ejectile->Fill( react->DopplerCorrection( e, true ), react->DopplerCorrection( g, true ), weight );
		egE_ejectile_dc_recoil->Fill( react->DopplerCorrection( e, false ), react->DopplerCorrection( g, false ), weight );

	}
	
	// Recoil-gated spectra
	if( react->IsRecoilDetected() || react->IsTransferDetected() ) {
		
		// Electon-gamma
		egE_recoil_dc_none->Fill( e->GetEnergy(), g->GetEnergy(), weight );
		egE_recoil_dc_ejectile->Fill( react->DopplerCorrection( e, true ), react->DopplerCorrection( g, true ), weight );
		egE_recoil_dc_recoil->Fill( react->DopplerCorrection( e, false ), react->DopplerCorrection( g, false ), weight );

	}

	return;
	
}

// Particle-Electron-Gamma coincidences with addback
void MiniballHistogrammer::FillParticleElectronGammaHists( std::shared_ptr<SpedeEvt> e, std::shared_ptr<GammaRayAddbackEvt> g ) {

	// Work out the weight if it's prompt or random
	float weight;
	if( PromptCoincidence( e, react->GetParticleTime() ) ) {
		weight = 1.0;
	}
	else if( RandomCoincidence( e, react->GetParticleTime() ) ){
		weight = -1.0 * react->GetParticleElectronFillRatio();
	}
	else return; // outside of either window, quit now

	// Ejectile-gated spectra
	if( react->IsEjectileDetected() ) {
		
		// Electon-gamma
		eaE_ejectile_dc_none->Fill( e->GetEnergy(), g->GetEnergy(), weight );
		eaE_ejectile_dc_ejectile->Fill( react->DopplerCorrection( e, true ), react->DopplerCorrection( g, true ), weight );
		eaE_ejectile_dc_recoil->Fill( react->DopplerCorrection( e, false ), react->DopplerCorrection( g, false ), weight );
		
	}
	
	// Recoil-gated spectra
	if( react->IsRecoilDetected() || react->IsTransferDetected() ) {
		
		// Electon-gamma
		eaE_recoil_dc_none->Fill( e->GetEnergy(), g->GetEnergy(), weight );
		eaE_recoil_dc_ejectile->Fill( react->DopplerCorrection( e, true ), react->DopplerCorrection( g, true ), weight );
		eaE_recoil_dc_recoil->Fill( react->DopplerCorrection( e, false ), react->DopplerCorrection( g, false ), weight );
		
	}
	
	return;
	
}

unsigned long MiniballHistogrammer::FillHists() {
	
	/// Main function to fill the histograms
	n_entries = input_tree->GetEntries();
	
	std::cout << " MiniballHistogrammer: number of entries in event tree = ";
	std::cout << n_entries << std::endl;
	
	if( n_entries == 0 ){
		
		std::cout << " MiniballHistogrammer: Nothing to do..." << std::endl;
		output_file->Write( nullptr, TObject::kOverwrite );
		return n_entries;
		
	}
	else {
		
		std::cout << " MiniballHistogrammer: Start filling histograms" << std::endl;
		
	}

	// ------------------------------------------------------------------------ //
	// Main loop over TTree to find events
	// ------------------------------------------------------------------------ //
	for( unsigned int i = 0; i < n_entries; ++i ){
		
		// Current event data
		input_tree->GetEntry(i);
		
		// Get laser status
		unsigned char laser_status = read_evts->GetLaserStatus();
		
		// Check laser mode
		unsigned char laser_mode = react->GetLaserMode();
		
		// Test if we want to plot this event or not
		if( laser_status != laser_mode && laser_mode != 2 ) continue;
		
		
		// ------------------------- //
		// Loop over particle events //
		// ------------------------- //
		react->ResetParticles();
		for( unsigned int j = 0; j < read_evts->GetParticleMultiplicity(); ++j ){
			
			// Get particle event
			particle_evt = read_evts->GetParticleEvt(j);
			
			// EBIS time
			ebis_td_particle->Fill( (double)particle_evt->GetTime() - (double)read_evts->GetEBIS() );
			
			// Energy vs Angle plot no gates
			float pid = particle_evt->GetStripP() + rand.Rndm() - 0.5; // randomise strip number
			float nid = particle_evt->GetStripN() + rand.Rndm() - 0.5; // randomise strip number
			TVector3 pvec = react->GetCDVector( particle_evt->GetDetector(), particle_evt->GetSector(), pid, nid );
			pE_theta->Fill( react->GetParticleTheta( particle_evt ) * TMath::RadToDeg(), particle_evt->GetDeltaEnergy() );
			particle_theta_phi_map->Fill( react->GetParticleTheta( particle_evt ) * TMath::RadToDeg(),
										  react->GetParticlePhi( particle_evt ) * TMath::RadToDeg() );
			if( react->GetParticleTheta( particle_evt ) < TMath::PiOver2() )
				particle_xy_map_forward->Fill( pvec.Y(), pvec.X() );
			else
				particle_xy_map_backward->Fill( pvec.Y(), pvec.X() );

			// Energy total versus energy loss, i.e. CD+PAD vs. CD
			pE_dE[particle_evt->GetDetector()][particle_evt->GetSector()]->Fill( particle_evt->GetEnergy(), particle_evt->GetDeltaEnergy() );
			

			// Check for prompt coincidence with a gamma-ray
			for( unsigned int k = 0; k < read_evts->GetGammaRayMultiplicity(); ++k ){
				
				// Get gamma-ray event
				gamma_evt = read_evts->GetGammaRayEvt(k);
				
				// Time differences
				gamma_particle_td->Fill( (double)particle_evt->GetTime() - (double)gamma_evt->GetTime() );
				gamma_particle_E_vs_td->Fill( (double)particle_evt->GetTime() - (double)gamma_evt->GetTime(), gamma_evt->GetEnergy() );

				// Check for prompt coincidence
				if( PromptCoincidence( gamma_evt, particle_evt ) ){
					
					// Energy vs Angle plot with gamma-ray coincidence
					pE_theta_coinc->Fill( react->GetParticleTheta( particle_evt ) * TMath::RadToDeg(), particle_evt->GetDeltaEnergy() );
					pE_dE_coinc[particle_evt->GetDetector()][particle_evt->GetSector()]->Fill( particle_evt->GetEnergy(), particle_evt->GetDeltaEnergy() );

				} // if prompt
				
			} // k: gammas
			
			// Check for prompt coincidence with an electron
			for( unsigned int k = 0; k < read_evts->GetSpedeMultiplicity(); ++k ){
				
				// Get SPEDE event
				spede_evt = read_evts->GetSpedeEvt(k);
				
				// Time differences
				electron_particle_td->Fill( (double)particle_evt->GetTime() - (double)spede_evt->GetTime() );
								
			} // k: eleectrons

			// This flag is used so that we don't double count
			// TODO: It needs to be improved to allow multiple particles in transfer
			bool event_used = false;

			// See if we are doing transfer reactions
			if( react->GetBeam()->GetIsotope() != react->GetEjectile()->GetIsotope() &&
				TransferCut( particle_evt ) ) {
				
				react->TransferProduct( particle_evt );
				react->SetParticleTime( particle_evt->GetTime() );
				event_used = true;
				
				pE_dE_cut[particle_evt->GetDetector()][particle_evt->GetSector()]->Fill( particle_evt->GetEnergy(), particle_evt->GetDeltaEnergy() );

			} // transfer event
			
			// Check for prompt coincidence with another particle
			for( unsigned int k = j+1; k < read_evts->GetParticleMultiplicity(); ++k ){
				
				// Get second particle event
				particle_evt2 = read_evts->GetParticleEvt(k);

				// Time differences and fill symmetrically
				particle_particle_td->Fill( (double)particle_evt->GetTime() - (double)particle_evt2->GetTime() );
				particle_particle_td->Fill( (double)particle_evt2->GetTime() - (double)particle_evt->GetTime() );
				
				// Don't try to make more particle events
				// if we already got one?
				if( event_used ) continue;
				
				// Do a two-particle cut and check that they are coincident
				// particle_evt (j) is beam and particle_evt2 (k) is target
				else if( TwoParticleCut( particle_evt, particle_evt2 ) ){
					
					react->IdentifyEjectile( particle_evt );
					react->IdentifyRecoil( particle_evt2 );
					if( particle_evt->GetTime() < particle_evt2->GetTime() )
						react->SetParticleTime( particle_evt->GetTime() );
					else react->SetParticleTime( particle_evt2->GetTime() );
					event_used = true;
					
					pE_theta_ejectile->Fill( react->GetParticleTheta( particle_evt ) * TMath::RadToDeg(), particle_evt->GetDeltaEnergy() );
					pE_theta_recoil->Fill( react->GetParticleTheta( particle_evt2 ) * TMath::RadToDeg(), particle_evt2->GetDeltaEnergy() );
					pBeta_theta_ejectile->Fill( react->GetParticleTheta( particle_evt ) * TMath::RadToDeg(), react->GetEjectile()->GetBeta() );
					pBeta_theta_recoil->Fill( react->GetParticleTheta( particle_evt2 ) * TMath::RadToDeg(), react->GetRecoil()->GetBeta() );


				} // 2-particle check
				
				// particle_evt2 (k) is beam and particle_evt (j) is target
				else if( TwoParticleCut( particle_evt2, particle_evt ) ){
					
					react->IdentifyEjectile( particle_evt2 );
					react->IdentifyRecoil( particle_evt );
					if( particle_evt->GetTime() < particle_evt2->GetTime() )
						react->SetParticleTime( particle_evt->GetTime() );
					else react->SetParticleTime( particle_evt2->GetTime() );
					event_used = true;
					
					pE_theta_ejectile->Fill( react->GetParticleTheta( particle_evt2 ) * TMath::RadToDeg(), particle_evt2->GetDeltaEnergy() );
					pE_theta_recoil->Fill( react->GetParticleTheta( particle_evt ) * TMath::RadToDeg(), particle_evt->GetDeltaEnergy() );
					pBeta_theta_ejectile->Fill( react->GetParticleTheta( particle_evt2 ) * TMath::RadToDeg(), react->GetEjectile()->GetBeta() );
					pBeta_theta_recoil->Fill( react->GetParticleTheta( particle_evt ) * TMath::RadToDeg(), react->GetRecoil()->GetBeta() );

					
				} // 2-particle check

			} // k: second particle
			
			// If we found a particle and used it, then we need to
			// stop so we don't fill the gamma-spectra more than once
			if( event_used ) break;

			// Otherwise we can build a one particle event
			else if( EjectileCut( particle_evt ) ) {
				
				react->IdentifyEjectile( particle_evt );
				react->CalculateRecoil();
				react->SetParticleTime( particle_evt->GetTime() );

				pE_theta_ejectile->Fill( react->GetParticleTheta( particle_evt ) * TMath::RadToDeg(), particle_evt->GetDeltaEnergy() );
				pBeta_theta_ejectile->Fill( react->GetParticleTheta( particle_evt ) * TMath::RadToDeg(), react->GetEjectile()->GetBeta() );

			} // ejectile event

			else if( RecoilCut( particle_evt ) ) {
				
				react->IdentifyRecoil( particle_evt );
				react->CalculateEjectile();
				react->SetParticleTime( particle_evt->GetTime() );

				pE_theta_recoil->Fill( react->GetParticleTheta( particle_evt ) * TMath::RadToDeg(), particle_evt->GetDeltaEnergy() );
				pBeta_theta_recoil->Fill( react->GetParticleTheta( particle_evt ) * TMath::RadToDeg(), react->GetRecoil()->GetBeta() );

			} // recoil event

		} // j: particles

		
		
		// ------------------------------------------ //
		// Loop over gamma-ray events without addback //
		// ------------------------------------------ //
		for( unsigned int j = 0; j < read_evts->GetGammaRayMultiplicity(); ++j ){
						
			// Get gamma-ray event
			gamma_evt = read_evts->GetGammaRayEvt(j);
			
			// Singles
			gE_singles->Fill( gamma_evt->GetEnergy() );
			
			// Singles - Doppler corrected
			gE_singles_dc->Fill( react->DopplerCorrection( gamma_evt, react->GetBeam()->GetBeta(), 0, 0 ) );
			
			// EBIS time
			ebis_td_gamma->Fill( (double)gamma_evt->GetTime() - (double)read_evts->GetEBIS() );
			
			// Check for events in the EBIS on-beam window
			if( OnBeam( gamma_evt ) ){
				
				gE_singles_ebis->Fill( gamma_evt->GetEnergy() );
				gE_singles_ebis_on->Fill( gamma_evt->GetEnergy() );
				gE_singles_dc_ebis->Fill( react->DopplerCorrection( gamma_evt, react->GetBeam()->GetBeta(), 0, 0 ) );

			} // ebis on
			
			else if( OffBeam( gamma_evt ) ){
				
				gE_singles_ebis->Fill( gamma_evt->GetEnergy(), -1.0 * react->GetEBISFillRatio() );
				gE_singles_ebis_off->Fill( gamma_evt->GetEnergy() );
				gE_singles_dc_ebis->Fill( react->DopplerCorrection( gamma_evt, react->GetBeam()->GetBeta(), 0, 0 ), -1.0 * react->GetEBISFillRatio() );

			} // ebis off
			
			// Gamma-ray X-Y hit map
			if( react->GetGammaZ( gamma_evt ) > 0 )
				gamma_xy_map_forward->Fill( react->GetGammaY( gamma_evt ), react->GetGammaX( gamma_evt ) );
			else
				gamma_xy_map_backward->Fill( react->GetGammaY( gamma_evt ), react->GetGammaX( gamma_evt ) );

			// Gamma-ray X-Z hit map
			if( react->GetGammaY( gamma_evt ) > 0 )
				gamma_xz_map_right->Fill( react->GetGammaZ( gamma_evt ), react->GetGammaX( gamma_evt ) );
			else
				gamma_xz_map_left->Fill( react->GetGammaZ( gamma_evt ), react->GetGammaX( gamma_evt ) );

			// Gamma-ray theta-phi map
			double theta = react->GetGammaTheta( gamma_evt );
			double phi = react->GetGammaPhi( gamma_evt );
			if( theta < 0 ) theta += TMath::Pi();
			if( phi < 0 ) phi += TMath::TwoPi();
			gamma_theta_phi_map->Fill( theta*TMath::RadToDeg(), phi*TMath::RadToDeg() );
			
			// Particle-gamma coincidence spectra
			FillParticleGammaHists( gamma_evt );

			// Loop over other gamma events
			for( unsigned int k = j+1; k < read_evts->GetGammaRayMultiplicity(); ++k ){
				
				// Get gamma-ray event
				gamma_evt2 = read_evts->GetGammaRayEvt(k);
				
				// Time differences - symmetrise
				gamma_gamma_td->Fill( (double)gamma_evt->GetTime() - (double)gamma_evt2->GetTime() );
				gamma_gamma_td->Fill( (double)gamma_evt2->GetTime() - (double)gamma_evt->GetTime() );
				
				// Particle-gamma-gamma coincidence spectra
				if( react->HistGammaGamma() ) {
					
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
						
						FillParticleGammaGammaHists( gamma_evt, gamma_evt2 );
						FillParticleGammaGammaHists( gamma_evt2, gamma_evt );
						
					} // if prompt
					
				} // gamma-gamma user option on
				
			} // k: second gamma-ray
			
		} // j: gamma ray
		
		
		// --------------------------------------- //
		// Loop over gamma-ray events with addback //
		// --------------------------------------- //
		for( unsigned int j = 0; j < read_evts->GetGammaRayAddbackMultiplicity(); ++j ){
			
			// Get gamma-ray event
			gamma_ab_evt = read_evts->GetGammaRayAddbackEvt(j);
			
			// Singles
			aE_singles->Fill( gamma_ab_evt->GetEnergy() );
			
			// Singles - Doppler corrected
			aE_singles_dc->Fill( react->DopplerCorrection( gamma_ab_evt, react->GetBeam()->GetBeta(), 0, 0 ) );
			
			// Check for events in the EBIS on-beam window
			if( OnBeam( gamma_ab_evt ) ){
				
				aE_singles_ebis->Fill( gamma_ab_evt->GetEnergy() );
				aE_singles_ebis_on->Fill( gamma_ab_evt->GetEnergy() );
				aE_singles_dc_ebis->Fill( react->DopplerCorrection( gamma_ab_evt, react->GetBeam()->GetBeta(), 0, 0 ) );

			} // ebis on
			
			else if( OffBeam( gamma_ab_evt ) ){
				
				aE_singles_ebis->Fill( gamma_ab_evt->GetEnergy(), -1.0 * react->GetEBISFillRatio() );
				aE_singles_ebis_off->Fill( gamma_ab_evt->GetEnergy() );
				aE_singles_dc_ebis->Fill( react->DopplerCorrection( gamma_ab_evt, react->GetBeam()->GetBeta(), 0, 0 ), -1.0 * react->GetEBISFillRatio() );

			} // ebis off
			
			// Particle-gamma coincidence spectra
			FillParticleGammaHists( gamma_ab_evt );
			
			// If gamma-gamma histograms are turned on
			if( react->HistGammaGamma() ) {
				
				// Loop over other gamma events
				for( unsigned int k = j+1; k < read_evts->GetGammaRayAddbackMultiplicity(); ++k ){
					
					// Get gamma-ray event
					gamma_ab_evt2 = read_evts->GetGammaRayAddbackEvt(k);
					
					// Check for prompt gamma-gamma coincidences
					if( PromptCoincidence( gamma_ab_evt, gamma_ab_evt2 ) ) {
						
						// Fill and symmetrise
						aE_aE->Fill( gamma_ab_evt->GetEnergy(), gamma_ab_evt2->GetEnergy() );
						aE_aE->Fill( gamma_ab_evt2->GetEnergy(), gamma_ab_evt->GetEnergy() );
						
						// Apply EBIS condition
						if( OnBeam( gamma_ab_evt ) && OnBeam( gamma_ab_evt2 ) ) {
							
							// Fill and symmetrise
							aE_aE_ebis_on->Fill( gamma_ab_evt->GetEnergy(), gamma_ab_evt2->GetEnergy() );
							aE_aE_ebis_on->Fill( gamma_ab_evt2->GetEnergy(), gamma_ab_evt->GetEnergy() );
							
						} // On Beam
						
						// Particle-gamma-gamma coincidence spectra
						FillParticleGammaGammaHists( gamma_ab_evt, gamma_ab_evt2 );
						FillParticleGammaGammaHists( gamma_ab_evt2, gamma_ab_evt );
						
					} // if prompt
					
				} // k: second gamma-ray
				
			} // gamma-gamma user option on
			
		} // j: gamma ray
		

		// ---------------------------------- //
		// Loop over electron events in SPEDE //
		// ---------------------------------- //
		// If the electron histograms are turned on
		if( react->HistElectron() ) {
			
			// Get the first SPEDE event
			for( unsigned int j = 0; j < read_evts->GetSpedeMultiplicity(); ++j ){
				
				// Get SPEDE event
				spede_evt = read_evts->GetSpedeEvt(j);
				
				// Singles
				eE_singles->Fill( spede_evt->GetEnergy() );
				
				// Check for events in the EBIS on-beam window
				if( OnBeam( spede_evt ) ){
					
					eE_singles_ebis->Fill( spede_evt->GetEnergy() );
					eE_singles_ebis_on->Fill( spede_evt->GetEnergy() );
					
				} // ebis on
				
				else if( OffBeam( spede_evt ) ){
					
					eE_singles_ebis->Fill( spede_evt->GetEnergy(), -1.0 * react->GetEBISFillRatio() );
					eE_singles_ebis_off->Fill( spede_evt->GetEnergy() );
					
				} // ebis off
				
				// Particle-electron coincidence spectra
				FillParticleElectronHists( spede_evt );
				
				// SPEDE hitmap
				TVector3 evec = react->GetSpedeVector( spede_evt->GetSegment(), true );
				electron_xy_map->Fill( evec.Y(), evec.X() );
				
				// Loop over other SPEDE events
				for( unsigned int k = j+1; k < read_evts->GetSpedeMultiplicity(); ++k ){
					
					// Get second SPEDE event
					spede_evt2 = read_evts->GetSpedeEvt(k);
					
					// Time differences - symmetrise
					electron_electron_td->Fill( (double)spede_evt->GetTime() - (double)spede_evt2->GetTime() );
					electron_electron_td->Fill( (double)spede_evt2->GetTime() - (double)spede_evt->GetTime() );
					
					// Check for prompt gamma-gamma coincidences
					if( PromptCoincidence( spede_evt, spede_evt2 ) ) {
						
						// Fill and symmetrise
						eE_eE->Fill( spede_evt->GetEnergy(), spede_evt2->GetEnergy() );
						eE_eE->Fill( spede_evt2->GetEnergy(), spede_evt->GetEnergy() );
						
						// Apply EBIS condition
						if( OnBeam( spede_evt ) && OnBeam( spede_evt2 ) ) {
							
							// Fill and symmetrise
							eE_eE_ebis_on->Fill( spede_evt->GetEnergy(), spede_evt2->GetEnergy() );
							eE_eE_ebis_on->Fill( spede_evt2->GetEnergy(), spede_evt->GetEnergy() );
							
						} // On Beam
						
					} // if prompt
					
				} // k: second electron
				
				// Loop over gamma events
				for( unsigned int k = 0; k < read_evts->GetGammaRayMultiplicity(); ++k ){
					
					// Get gamma-ray event
					gamma_evt = read_evts->GetGammaRayEvt(k);
					
					// Time differences
					gamma_electron_td->Fill( (double)spede_evt->GetTime() - (double)gamma_evt->GetTime() );
					gamma_electron_td->Fill( (double)gamma_evt->GetTime() - (double)spede_evt->GetTime() );
					
					// If electron-gamma histograms are turned on
					if( react->HistElectronGamma() ) {
						
						// Check for prompt gamma-electron coincidences
						if( PromptCoincidence( gamma_evt, spede_evt ) ) {
							
							// Fill
							gE_eE->Fill( gamma_evt->GetEnergy(), spede_evt->GetEnergy() );
							
							// Apply EBIS condition
							if( OnBeam( gamma_evt ) && OnBeam( spede_evt ) ) {
								
								// Fill
								gE_eE_ebis_on->Fill( gamma_evt->GetEnergy(), spede_evt->GetEnergy() );
								
							} // On Beam
							
							// Particle-electron-gamma coincidence spectra
							FillParticleElectronGammaHists( spede_evt, gamma_evt );
							
						} // if prompt
						
					} // electron-gamma user option
					
				} // k: gamma without addback
				
				// If electron-gamma histograms are turned on
				if( react->HistElectronGamma() ) {
					
					// Loop over gamma addback events
					for( unsigned int k = 0; k < read_evts->GetGammaRayAddbackMultiplicity(); ++k ){
						
						// Get gamma-ray event
						gamma_ab_evt = read_evts->GetGammaRayAddbackEvt(k);
						
						// Check for prompt gamma-electron coincidences
						if( PromptCoincidence( gamma_ab_evt, spede_evt ) ) {
							
							// Fill
							aE_eE->Fill( gamma_ab_evt->GetEnergy(), spede_evt->GetEnergy() );
							
							// Apply EBIS condition
							if( OnBeam( gamma_ab_evt ) && OnBeam( spede_evt ) ) {
								
								// Fill
								aE_eE_ebis_on->Fill( gamma_ab_evt->GetEnergy(), spede_evt->GetEnergy() );
								
							} // On Beam
							
							// Particle-electron-gamma coincidence spectra
							FillParticleElectronGammaHists( spede_evt, gamma_ab_evt );
							
						} // if prompt
						
					} // k: gamma with addback
					
				} // electron-gamma user option
				
			} // j: SPEDE electrons
			
		} // if electron hists turned on
		
		
		// -------------------------- //
		// Loop over beam dump events //
		// -------------------------- //
		// If beam-dump histograms are turned on
		if( react->HistBeamDump() ) {
			
			// Do loop over first beam-dump event
			for( unsigned int j = 0; j < read_evts->GetBeamDumpMultiplicity(); ++j ){
				
				// Get beam-dump event
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
			
		} // beam-dump user histograms on
		
		
		// ---------------------------- //
		// Loop over ion chamber events //
		// ---------------------------- //
		// If ionisation chamber histograms are turned on
		if( react->HistIonChamber() ) {
			
			// Loop over ion chamber events
			for( unsigned int j = 0; j < read_evts->GetIonChamberMultiplicity(); ++j ){
				
				// Get ion chamber event
				ic_evt = read_evts->GetIonChamberEvt(j);
				
				// Single spectra
				ic_dE->Fill( ic_evt->GetEnergyLoss() );
				ic_E->Fill( ic_evt->GetEnergyRest() );
				
				// 2D plot
				ic_dE_E->Fill( ic_evt->GetEnergyRest(), ic_evt->GetEnergyLoss() );
				
			} // j: ion chamber
			
		} // if ion chamber hists are on
		
		// Progress bar
		bool update_progress = false;
		if( n_entries < 200 )
			update_progress = true;
		else if( i % (n_entries/100) == 0 || i+1 == n_entries )
			update_progress = true;
		
		if( update_progress ) {

			// Percent complete
			float percent = (float)(i+1)*100.0/(float)n_entries;
			
			// Progress bar in GUI
			if( _prog_ ){
				
				prog->SetPosition( percent );
				gSystem->ProcessEvents();
				
			}
		
			// Progress bar in terminal
			std::cout << " " << std::setw(6) << std::setprecision(4);
			std::cout << percent << "%    \r";
			std::cout.flush();

		}
		
		
	} // all events
	
	output_file->Write( nullptr, TObject::kOverwrite );
	output_file->Purge(2);
	
	return n_entries;
	
}

void MiniballHistogrammer::SetInputFile( std::vector<std::string> input_file_names ) {
	
	/// Overloaded function for a single file or multiple files
	input_tree = new TChain( "evt_tree" );
	for( unsigned int i = 0; i < input_file_names.size(); i++ ) {
		
		input_tree->Add( input_file_names[i].data() );
		
	}
	input_tree->SetBranchAddress( "MiniballEvts", &read_evts );
	
	return;
	
}

void MiniballHistogrammer::SetInputFile( std::string input_file_name ) {
	
	/// Overloaded function for a single file or multiple files
	input_tree = new TChain( "evt_tree" );
	input_tree->Add( input_file_name.data() );
	input_tree->SetBranchAddress( "MiniballEvts", &read_evts );
	
	return;
	
}

void MiniballHistogrammer::SetInputTree( TTree *user_tree ){
	
	// Find the tree and set branch addresses
	input_tree = (TChain*)user_tree;
	input_tree->SetBranchAddress( "MiniballEvts", &read_evts );
	
	return;
	
}
