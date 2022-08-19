#include "Histogrammer.hh"

Histogrammer::Histogrammer( std::shared_ptr<Reaction> myreact, std::shared_ptr<Settings> myset ){
	
	react = myreact;
	set = myset;

	// Progress bar starts as false
	_prog_ = false;
	
}

void Histogrammer::MakeHists() {
	
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
								 1000, -1.0*set->GetEventWindow()-50, 1.0*set->GetEventWindow()+50 );

	hname = "gamma_gamma_td";
	htitle = "Gamma-ray - Gamma-ray time difference;#Deltat;Counts";
	gamma_gamma_td = new TH1F( hname.data(), htitle.data(),
							  1000, -1.0*set->GetEventWindow()-50, 1.0*set->GetEventWindow()+50 );
	
	hname = "gamma_electron_td";
	htitle = "Gamma-ray - Electron time difference;#Deltat;Counts";
	gamma_electron_td = new TH1F( hname.data(), htitle.data(),
							  1000, -1.0*set->GetEventWindow()-50, 1.0*set->GetEventWindow()+50 );
	
	hname = "electron_electron_td";
	htitle = "Electron - Electron time difference;#Deltat;Counts";
	electron_electron_td = new TH1F( hname.data(), htitle.data(),
							  1000, -1.0*set->GetEventWindow()-50, 1.0*set->GetEventWindow()+50 );
	
	hname = "electron_particle_td";
	htitle = "Electron - Particle time difference;#Deltat;Counts";
	electron_particle_td = new TH1F( hname.data(), htitle.data(),
							  1000, -1.0*set->GetEventWindow()-50, 1.0*set->GetEventWindow()+50 );
	
	hname = "particle_particle_td";
	htitle = "Particle - Particle time difference;#Deltat;Counts";
	particle_particle_td = new TH1F( hname.data(), htitle.data(),
								 1000, -1.0*set->GetEventWindow()-50, 1.0*set->GetEventWindow()+50 );

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

	// Gamma-ray coincidence histograms
	dirname = "CoincidenceMatrices";
	output_file->mkdir( dirname.data() );
	output_file->cd( dirname.data() );
	
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
	
	hname = "eE_eE";
	htitle = "Electron coincidence matrix;Energy [keV];Energy [keV];Counts per keV";
	eE_eE = new TH2F( hname.data(), htitle.data(), EBIN, EMIN, EMAX, EBIN, EMIN, EMAX );
	
	hname = "eE_eE_ebis_on";
	htitle = "Electron coincidence matrix EBIS on;Energy [keV];Energy [keV];Counts per keV";
	eE_eE_ebis_on = new TH2F( hname.data(), htitle.data(), EBIN, EMIN, EMAX, EBIN, EMIN, EMAX );

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


	// CD singles histograms
	dirname = "ParticleSpectra";
	output_file->mkdir( dirname.data() );
	output_file->cd( dirname.data() );
	
	hname = "pE_theta";
	htitle = "Particle energy singles;Angle [deg];Energy [keV];Counts per 0.5 keV";
	pE_theta = new TH2F( hname.data(), htitle.data(), react->GetNumberOfParticleThetas(), react->GetParticleThetas().data(), PBIN, PMIN, PMAX );
	
	hname = "pE_theta_coinc";
	htitle = "Particle energy in coincidence with a gamma ray;Angle [deg];Energy [keV];Counts per 0.5 keV";
	pE_theta_coinc = new TH2F( hname.data(), htitle.data(), react->GetNumberOfParticleThetas(), react->GetParticleThetas().data(), PBIN, PMIN, PMAX );
	
	hname = "pE_theta_beam";
	htitle = "Particle energy singles, gated on beam;Angle [deg];Energy [keV];Counts per 0.5 keV";
	pE_theta_beam = new TH2F( hname.data(), htitle.data(), react->GetNumberOfParticleThetas(), react->GetParticleThetas().data(), PBIN, PMIN, PMAX );
	
	hname = "pE_theta_target";
	htitle = "Particle energy singles, gated on target;Angle [deg];Energy [keV];Counts per 0.5 keV";
	pE_theta_target = new TH2F( hname.data(), htitle.data(), react->GetNumberOfParticleThetas(), react->GetParticleThetas().data(), PBIN, PMIN, PMAX );
	
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


	//  Electron-particle coincidences
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



	// Beam dump histograms
	dirname = "BeamDump";
	output_file->mkdir( dirname.data() );
	output_file->cd( dirname.data() );
	
	hname = "bdE_singles";
	htitle = "Beam-dump gamma-ray energy singles;Energy [keV];Counts per 0.5 keV";
	bdE_singles = new TH1F( hname.data(), htitle.data(), GBIN, GMIN, GMAX );
	
	hname = "bd_bd_td";
	htitle = "Beam-dump - Beam-dump time difference;#Deltat;Counts";
	bd_bd_td = new TH1F( hname.data(), htitle.data(),
						1000, -1.0*set->GetEventWindow()-50, 1.0*set->GetEventWindow()+50 );
	
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
	
	return;
	
}

// Particle-Gamma coincidences without addback
void Histogrammer::FillParticleGammaHists( std::shared_ptr<GammaRayEvt> g ) {

	// Work out the weight if it's prompt or random
	bool prompt = false;
	bool random = false;
	float weight;
	if( PromptCoincidence( g, react->GetParticleTime() ) ) {
		prompt = true;
		weight = 1.0;
	}
	else if( RandomCoincidence( g, react->GetParticleTime() ) ){
		random = true;
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
		
		gE_ejectile_dc_none->Fill( g->GetEnergy(), weight );
		gE_ejectile_dc_ejectile->Fill( react->DopplerCorrection( g, true ), weight );
		gE_ejectile_dc_recoil->Fill( react->DopplerCorrection( g, false ), weight );

		gE_vs_theta_ejectile_dc_none->Fill( react->GetEjectile()->GetTheta(), g->GetEnergy(), weight );
		gE_vs_theta_ejectile_dc_ejectile->Fill( react->GetEjectile()->GetTheta(), react->DopplerCorrection( g, true ), weight );
		gE_vs_theta_ejectile_dc_recoil->Fill( react->GetEjectile()->GetTheta(), react->DopplerCorrection( g, false ), weight );

	}

	// Recoil-gated spectra
	if( react->IsRecoilDetected() ) {
		
		gE_recoil_dc_none->Fill( g->GetEnergy(), weight );
		gE_recoil_dc_ejectile->Fill( react->DopplerCorrection( g, true ), weight );
		gE_recoil_dc_recoil->Fill( react->DopplerCorrection( g, false ), weight );

		gE_vs_theta_recoil_dc_none->Fill( react->GetRecoil()->GetTheta(), g->GetEnergy(), weight );
		gE_vs_theta_recoil_dc_ejectile->Fill( react->GetRecoil()->GetTheta(), react->DopplerCorrection( g, true ), weight );
		gE_vs_theta_recoil_dc_recoil->Fill( react->GetRecoil()->GetTheta(), react->DopplerCorrection( g, false ), weight );

	}
	
	// Two particle spectra
	if( react->IsEjectileDetected() && react->IsRecoilDetected() ){
		
		// Prompt and random spectra
		if( prompt ) gE_prompt_2p->Fill( g->GetEnergy() );
		else gE_random_2p->Fill( g->GetEnergy() );

		gE_2p_dc_none->Fill( g->GetEnergy(), weight );
		gE_2p_dc_ejectile->Fill( react->DopplerCorrection( g, true ), weight );
		gE_2p_dc_recoil->Fill( react->DopplerCorrection( g, false ), weight );

		gE_vs_theta_2p_dc_none->Fill( react->GetRecoil()->GetTheta(), g->GetEnergy(), weight );
		gE_vs_theta_2p_dc_ejectile->Fill( react->GetRecoil()->GetTheta(), react->DopplerCorrection( g, true ), weight );
		gE_vs_theta_2p_dc_recoil->Fill( react->GetRecoil()->GetTheta(), react->DopplerCorrection( g, false ), weight );
		
	}
	
	return;

}

// Particle-Gamma coincidences with addback
void Histogrammer::FillParticleGammaHists( std::shared_ptr<GammaRayAddbackEvt> g ) {

	// Work out the weight if it's prompt or random
	bool prompt = false;
	bool random = false;
	float weight;
	if( PromptCoincidence( g, react->GetParticleTime() ) ) {
		prompt = true;
		weight = 1.0;
	}
	else if( RandomCoincidence( g, react->GetParticleTime() ) ){
		random = true;
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
		
		aE_ejectile_dc_none->Fill( g->GetEnergy(), weight );
		aE_ejectile_dc_ejectile->Fill( react->DopplerCorrection( g, true ), weight );
		aE_ejectile_dc_recoil->Fill( react->DopplerCorrection( g, false ), weight );

		aE_vs_theta_ejectile_dc_none->Fill( react->GetEjectile()->GetTheta(), g->GetEnergy(), weight );
		aE_vs_theta_ejectile_dc_ejectile->Fill( react->GetEjectile()->GetTheta(), react->DopplerCorrection( g, true ), weight );
		aE_vs_theta_ejectile_dc_recoil->Fill( react->GetEjectile()->GetTheta(), react->DopplerCorrection( g, false ), weight );

	}

	// Recoil-gated spectra
	if( react->IsRecoilDetected() ) {
		
		aE_recoil_dc_none->Fill( g->GetEnergy(), weight );
		aE_recoil_dc_ejectile->Fill( react->DopplerCorrection( g, true ), weight );
		aE_recoil_dc_recoil->Fill( react->DopplerCorrection( g, false ), weight );

		aE_vs_theta_recoil_dc_none->Fill( react->GetRecoil()->GetTheta(), g->GetEnergy(), weight );
		aE_vs_theta_recoil_dc_ejectile->Fill( react->GetRecoil()->GetTheta(), react->DopplerCorrection( g, true ), weight );
		aE_vs_theta_recoil_dc_recoil->Fill( react->GetRecoil()->GetTheta(), react->DopplerCorrection( g, false ), weight );

	}
	
	// Two particle spectra
	if( react->IsEjectileDetected() && react->IsRecoilDetected() ){
		
		// Prompt and random spectra
		if( prompt ) aE_prompt_2p->Fill( g->GetEnergy() );
		else aE_random_2p->Fill( g->GetEnergy() );

		aE_2p_dc_none->Fill( g->GetEnergy(), weight );
		aE_2p_dc_ejectile->Fill( react->DopplerCorrection( g, true ), weight );
		aE_2p_dc_recoil->Fill( react->DopplerCorrection( g, false ), weight );

		aE_vs_theta_2p_dc_none->Fill( react->GetRecoil()->GetTheta(), g->GetEnergy(), weight );
		aE_vs_theta_2p_dc_ejectile->Fill( react->GetRecoil()->GetTheta(), react->DopplerCorrection( g, true ), weight );
		aE_vs_theta_2p_dc_recoil->Fill( react->GetRecoil()->GetTheta(), react->DopplerCorrection( g, false ), weight );
		
	}
	
	return;

}

// Particle-Electron coincidences with addback
void Histogrammer::FillParticleElectronHists( std::shared_ptr<SpedeEvt> e ) {

	// Work out the weight if it's prompt or random
	bool prompt = false;
	bool random = false;
	float weight;
	if( PromptCoincidence( e, react->GetParticleTime() ) ) {
		prompt = true;
		weight = 1.0;
	}
	else if( RandomCoincidence( e, react->GetParticleTime() ) ){
		random = true;
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
		
		eE_ejectile_dc_none->Fill( e->GetEnergy(), weight );
		eE_ejectile_dc_ejectile->Fill( react->DopplerCorrection( e, true ), weight );
		eE_ejectile_dc_recoil->Fill( react->DopplerCorrection( e, false ), weight );

		eE_vs_theta_ejectile_dc_none->Fill( react->GetEjectile()->GetTheta(), e->GetEnergy(), weight );
		eE_vs_theta_ejectile_dc_ejectile->Fill( react->GetEjectile()->GetTheta(), react->DopplerCorrection( e, true ), weight );
		eE_vs_theta_ejectile_dc_recoil->Fill( react->GetEjectile()->GetTheta(), react->DopplerCorrection( e, false ), weight );

	}

	// Recoil-gated spectra
	if( react->IsRecoilDetected() ) {
		
		eE_recoil_dc_none->Fill( e->GetEnergy(), weight );
		eE_recoil_dc_ejectile->Fill( react->DopplerCorrection( e, true ), weight );
		eE_recoil_dc_recoil->Fill( react->DopplerCorrection( e, false ), weight );

		eE_vs_theta_recoil_dc_none->Fill( react->GetRecoil()->GetTheta(), e->GetEnergy(), weight );
		eE_vs_theta_recoil_dc_ejectile->Fill( react->GetRecoil()->GetTheta(), react->DopplerCorrection( e, true ), weight );
		eE_vs_theta_recoil_dc_recoil->Fill( react->GetRecoil()->GetTheta(), react->DopplerCorrection( e, false ), weight );

	}
	
	// Two particle spectra
	if( react->IsEjectileDetected() && react->IsRecoilDetected() ){
		
		// Prompt and random spectra
		if( prompt ) eE_prompt_2p->Fill( e->GetEnergy() );
		else eE_random_2p->Fill( e->GetEnergy() );

		eE_2p_dc_none->Fill( e->GetEnergy(), weight );
		eE_2p_dc_ejectile->Fill( react->DopplerCorrection( e, true ), weight );
		eE_2p_dc_recoil->Fill( react->DopplerCorrection( e, false ), weight );

		eE_vs_theta_2p_dc_none->Fill( react->GetRecoil()->GetTheta(), e->GetEnergy(), weight );
		eE_vs_theta_2p_dc_ejectile->Fill( react->GetRecoil()->GetTheta(), react->DopplerCorrection( e, true ), weight );
		eE_vs_theta_2p_dc_recoil->Fill( react->GetRecoil()->GetTheta(), react->DopplerCorrection( e, false ), weight );
		
	}
	
	return;

}

unsigned long Histogrammer::FillHists() {
	
	/// Main function to fill the histograms
	n_entries = input_tree->GetEntries();
	
	std::cout << " Histogrammer: number of entries in event tree = ";
	std::cout << n_entries << std::endl;
	
	if( n_entries == 0 ){
		
		std::cout << " Histogrammer: Nothing to do..." << std::endl;
		return n_entries;
		
	}
	else {
		
		std::cout << " Histogrammer: Start filling histograms" << std::endl;
		
	}

	// ------------------------------------------------------------------------ //
	// Main loop over TTree to find events
	// ------------------------------------------------------------------------ //
	for( unsigned int i = 0; i < n_entries; ++i ){
		
		// Current event data
		input_tree->GetEntry(i);
		
		
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
			
			// Energy vs angle plot, after cuts
			if( EjectileCut( particle_evt ) )
				pE_theta_beam->Fill( react->GetParticleTheta( particle_evt ), particle_evt->GetEnergy() );
			
			if( RecoilCut( particle_evt ) )
				pE_theta_target->Fill( react->GetParticleTheta( particle_evt ), particle_evt->GetEnergy() );
			
			
			// Check for prompt coincidence with a gamma-ray
			for( unsigned int k = 0; k < read_evts->GetGammaRayMultiplicity(); ++k ){
				
				// Get gamma-ray event
				gamma_evt = read_evts->GetGammaRayEvt(k);
				
				// Time differences
				gamma_particle_td->Fill( (double)particle_evt->GetTime() - (double)gamma_evt->GetTime() );
				
				// Check for prompt coincidence
				if( PromptCoincidence( gamma_evt, particle_evt ) ){
					
					// Energy vs Angle plot with gamma-ray coincidence
					pE_theta_coinc->Fill( react->GetParticleTheta( particle_evt ), particle_evt->GetEnergy() );
					
				} // if prompt
				
			} // k: gammas
			
			// Check for prompt coincidence with an electron
			for( unsigned int k = 0; k < read_evts->GetSpedeMultiplicity(); ++k ){
				
				// Get SPEDE event
				spede_evt = read_evts->GetSpedeEvt(k);
				
				// Time differences
				electron_particle_td->Fill( (double)particle_evt->GetTime() - (double)spede_evt->GetTime() );
								
			} // k: eleectrons
			
			// Check for prompt coincidence with another particle
			bool event_used = false;
			for( unsigned int k = j+1; k < read_evts->GetParticleMultiplicity(); ++k ){
				
				// Get second particle event
				particle_evt2 = read_evts->GetParticleEvt(j);

				// Time differences and fill symmetrically
				particle_particle_td->Fill( (double)particle_evt->GetTime() - (double)particle_evt2->GetTime() );
				particle_particle_td->Fill( (double)particle_evt2->GetTime() - (double)particle_evt->GetTime() );
				
				// Don't try to make more particle events
				// if we already got one?
				if( event_used ) continue;
				
				// Do a two-particle cut and check that they are coincident
				// particle_evt (k) is beam and particle_evt2 (l) is target
				else if( TwoParticleCut( particle_evt, particle_evt2 ) ){
					
					react->IdentifyEjectile( particle_evt );
					react->IdentifyRecoil( particle_evt2 );
					if( particle_evt->GetTime() < particle_evt2->GetTime() )
						react->SetParticleTime( particle_evt->GetTime() );
					else react->SetParticleTime( particle_evt2->GetTime() );
					event_used = true;

				} // 2-particle check
				
				// particle_evt2 (l) is beam and particle_evt (k) is target
				else if( TwoParticleCut( particle_evt2, particle_evt ) ){
					
					react->IdentifyEjectile( particle_evt2 );
					react->IdentifyRecoil( particle_evt );
					if( particle_evt->GetTime() < particle_evt2->GetTime() )
						react->SetParticleTime( particle_evt->GetTime() );
					else react->SetParticleTime( particle_evt2->GetTime() );
					event_used = true;
					
				} // 2-particle check

			} // k: second particle
			
			// If we found a particle and used it, then we need to
			// stop so we don't fill the gamma-spectra more than once
			if( event_used ) continue;

			// Otherwise we can build a one particle event
			else if( EjectileCut( particle_evt ) ) {
				
				react->IdentifyEjectile( particle_evt );
				react->CalculateRecoil();
				react->SetParticleTime( particle_evt->GetTime() );
				
			} // ejectile event

			else if( RecoilCut( particle_evt ) &&
			   PromptCoincidence( gamma_evt, particle_evt ) ) {
				
				react->IdentifyRecoil( particle_evt );
				react->CalculateEjectile();
				react->SetParticleTime( particle_evt->GetTime() );

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
			
			// Particle-gamma coincidence spectra
			FillParticleGammaHists( gamma_evt );

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
					
					// TODO: Add particle gated gamma-gamma matrices
					
				} // if prompt
				
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
			
			// Check for events in the EBIS on-beam window
			if( OnBeam( gamma_ab_evt ) ){
				
				aE_singles_ebis->Fill( gamma_ab_evt->GetEnergy() );
				aE_singles_ebis_on->Fill( gamma_ab_evt->GetEnergy() );
				
			} // ebis on
			
			else if( OffBeam( gamma_ab_evt ) ){
				
				aE_singles_ebis->Fill( gamma_ab_evt->GetEnergy(), -1.0 * react->GetEBISRatio() );
				aE_singles_ebis_off->Fill( gamma_ab_evt->GetEnergy() );
				
			} // ebis off
			
			// Particle-gamma coincidence spectra
			FillParticleGammaHists( gamma_ab_evt );
			
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
					
					// TODO: Add particle gated gamma-gamma matrices

				} // if prompt
				
			} // k: second gamma-ray
			
		} // j: gamma ray
		

		// ------------------------------------------ //
		// Loop over gamma-ray events without addback //
		// ------------------------------------------ //
		for( unsigned int j = 0; j < read_evts->GetSpedeMultiplicity(); ++j ){
						
			// Get SPEDE event
			spede_evt = read_evts->GetSpedeEvt(j);

			// Singles
			eE_singles->Fill( gamma_ab_evt->GetEnergy() );
			
			// Check for events in the EBIS on-beam window
			if( OnBeam( gamma_ab_evt ) ){
				
				eE_singles_ebis->Fill( spede_evt->GetEnergy() );
				eE_singles_ebis_on->Fill( spede_evt->GetEnergy() );
				
			} // ebis on
			
			else if( OffBeam( gamma_ab_evt ) ){
				
				eE_singles_ebis->Fill( spede_evt->GetEnergy(), -1.0 * react->GetEBISRatio() );
				eE_singles_ebis_off->Fill( spede_evt->GetEnergy() );
				
			} // ebis off
			
			// Particle-electron coincidence spectra
			FillParticleElectronHists( spede_evt );
			
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

			// Loop over other gamma events
			for( unsigned int k = 0; k < read_evts->GetGammaRayMultiplicity(); ++k ){
				
				// Get gamma-ray event
				gamma_evt = read_evts->GetGammaRayEvt(k);
				
				// Time differences
				gamma_electron_td->Fill( (double)spede_evt->GetTime() - (double)gamma_evt->GetTime() );
				gamma_electron_td->Fill( (double)gamma_evt->GetTime() - (double)spede_evt->GetTime() );

				// Check for prompt gamma-electron coincidences
				if( PromptCoincidence( gamma_evt, spede_evt ) ) {
					
					// Fill
					gE_eE->Fill( gamma_ab_evt->GetEnergy(), spede_evt->GetEnergy() );
					
					// Apply EBIS condition
					if( OnBeam( gamma_evt ) && OnBeam( spede_evt ) ) {
						
						// Fill
						gE_eE_ebis_on->Fill( gamma_ab_evt->GetEnergy(), spede_evt->GetEnergy() );
						
					} // On Beam
					
					// TODO: Add particle gated gamma-electron matrices

				} // if prompt
				
			} // k: gamma without addback

			// Loop over other gamma events
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
					
					// TODO: Add particle gated gamma-electron matrices
					
				} // if prompt
				
			} // k: gamma with addback

		} // j: SPEDE electrons
		
		
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
	
	output_file->Write();
	
	return n_entries;
	
}

void Histogrammer::SetInputFile( std::vector<std::string> input_file_names ) {
	
	/// Overloaded function for a single file or multiple files
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
