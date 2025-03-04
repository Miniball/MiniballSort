#ifndef __REACTION_HH
#define __REACTION_HH

#include <iostream>
#include <vector>
#include <algorithm>
#include <memory>
#include <string>
#include <sstream>
#include <map>

#include "TSystem.h"
#include "TEnv.h"
#include "TMath.h"
#include "TObject.h"
#include "TString.h"
#include "TFile.h"
#include "TCutG.h"
#include "TVector3.h"
#include "TRandom.h"
#include "TSpline.h"
#include "TCanvas.h"
#include "TGraph.h"

// Settings header
#ifndef __SETTINGS_HH
# include "Settings.hh"
#endif

// Miniball Events tree
#ifndef __MINIBALLEVTS_HH
# include "MiniballEvts.hh"
#endif

// Header for geometry calibration
#ifndef __MINIBALLGEOMETRY_HH
# include "MiniballGeometry.hh"
#endif

// Make sure that the data and srim file are defined
#ifndef AME_FILE
# define AME_FILE "./data/mass_1.mas20"
#endif
#ifndef SRIM_DIR
# define SRIM_DIR "./srim/"
#endif

const double p_mass = 938272.08816;	///< mass of the proton in keV/c^2
const double n_mass = 939565.42052;	///< mass of the neutron in keV/c^2
const double u_mass = 931494.10242;	///< atomic mass unit in keV/c^2
const double e_mass = 510.99895026;	///< mass of the electron in keV/c^2

// Element names
const std::vector<std::string> gElName = {
	"n","H","He","Li","Be","B","C","N","O","F","Ne","Na","Mg",
	"Al","Si","P","S","Cl","Ar","K","Ca","Sc","Ti","V","Cr",
	"Mn","Fe","Co","Ni","Cu","Zn","Ga","Ge","As","Se","Br","Kr",
	"Rb","Sr","Y","Zr","Nb","Mo","Tc","Ru","Rh","Pd","Ag","Cd",
	"In","Sn","Sb","Te","I","Xe","Cs","Ba","La","Ce","Pr","Nd",
	"Pm","Sm","Eu","Gd","Tb","Dy","Ho","Er","Tm","Yb","Lu","Hf",
	"Ta","W","Re","Os","Ir","Pt","Au","Hg","Tl","Pb","Bi","Po",
	"At","Rn","Fr","Ra","Ac","Th","Pa","U","Np","Pu","Am","Cm",
	"Bk","Cf","Es","Fm","Md","No","Lr","Rf","Db","Sg","Bh","Hs",
	"Mt","Ds","Rg","Cn","Nh","Fl","Ms","Lv","Ts","Og","Uue","Ubn"
};


/// A class to read in the reaction file in ROOT's TConfig format.
/// And also to do the physics stuff for the reaction

class MiniballParticle : public TObject {
	
public:
	
	// setup functions
	MiniballParticle() {};
	~MiniballParticle() {};
	
	// Get properties
	inline double		GetMass_u(){
		return GetMass() / u_mass;
	};			// returns mass in u
	inline double		GetMass(){
		double mass = (double)GetN() * n_mass;
		mass += (double)GetZ() * p_mass;
		mass -= (double)GetA() * bindingE;
		mass += GetEx();
		return mass;
	};		// returns mass in keV/c^2
	inline int			GetA(){ return A; };	// returns mass number
	inline int			GetZ(){ return Z; };
	inline int			GetN(){ return A-Z; };
	inline std::string	GetIsotope(){
		return std::to_string( GetA() ) + gElName.at( GetZ() );
	};
	inline double		GetBindingEnergy(){ return bindingE; };
	inline double		GetEnergyTot(){ return GetEnergy() + GetMass(); };
	inline double		GetBeta(){
		double beta2 = 0.25 * GetMass() + 1.5 * GetEnergy();
		beta2  = TMath::Sqrt( beta2 * GetMass() );
		beta2 -= 0.5 * GetMass();
		return TMath::Sqrt( beta2 / ( 0.75 * GetMass() ) );
	};
	inline double		GetGamma(){
		return 1.0 / TMath::Sqrt( 1.0 - TMath::Power( GetBeta(), 2.0 ) );
	};
	inline double		GetEnergy(){ return Elab; };
	inline double		GetEx(){ return Ex; };
	inline double		GetTheta(){ return Theta; };
	inline double		GetThetaCoM(){ return ThetaCoM; };
	inline double		GetPhi(){ return Phi; };
	inline TVector3		GetVector(){
		TVector3 vec( 1, 0, 0 );
		vec.SetTheta( GetTheta() );
		vec.SetPhi( GetPhi() );
		return vec;
	};


	// Set properties
	inline void		SetA( int myA ){ A = myA; };
	inline void		SetZ( int myZ ){ Z = myZ; };
	inline void		SetBindingEnergy( double myBE ){ bindingE = myBE; };
	inline void		SetEnergy( double myElab ){ Elab = myElab; };
	inline void		SetEx( double myEx ){ Ex = myEx; };
	inline void		SetTheta( double mytheta ){ Theta = mytheta; };
	inline void		SetThetaCoM( double mytheta ){ ThetaCoM = mytheta; };
	inline void		SetPhi( double myphi ){ Phi = myphi; };


private:
	
	// Properties of reaction particles
	int		A;			///< mass number, A of the particle, obviously
	int		Z; 			///< The Z of the particle, obviously
	double	bindingE;	///< binding energy per nucleon in keV/c^2
	double	Elab;		///< energy in the laboratory system
	double	Ex;			///< excitation energy of the nucleus
	double	Theta;		///< theta in the laboratory system in radians
	double	ThetaCoM;	///< theta in the centre-of-mass system in radians
	double	Phi;		///< phi in the laboratory system in radians

	
	ClassDef( MiniballParticle, 1 )
	
};

class MiniballReaction : public TObject {
	
public:
	
	// setup functions
	MiniballReaction( std::string filename, std::shared_ptr<MiniballSettings> myset );
	~MiniballReaction() {};
	
	// Print the reaction data to a file
	void PrintReaction( std::ostream &stream, std::string opt );
	
	// Main functions
	void AddBindingEnergy( short Ai, short Zi, TString ame_be_str );
	void ReadMassTables();
	void ReadReaction();
	void SetFile( std::string filename ){
		fInputFile = filename;
	}
	const std::string InputFile(){
		return fInputFile;
	}
	
	// Get Doppler mode for calculation the velocity in the Doppler correction
	inline unsigned char GetDopplerMode(){ return doppler_mode; };
	
	// Get laser mode for histogramming
	inline unsigned char GetLaserMode(){ return laser_mode; };
	
	// Get values for geometry
	inline double			GetCDDistance( unsigned char det ){
		if( det < cd_dist.size() ) return cd_dist.at(det);
		else return 0.0;
	};
	inline double			GetCDPhiOffset( unsigned char det ){
		if( det < cd_offset.size() ) return cd_offset.at(det);
		else return 0.0;
	};
	inline double			GetCDDeadLayer( unsigned char det ){
		if( det < dead_layer.size() ) return dead_layer.at(det);
		else return 0.0;
	};
	inline unsigned int		GetNumberOfParticleThetas() {
		return set->GetNumberOfCDPStrips() * set->GetNumberOfCDDetectors();
	};
	inline std::vector<double>	GetParticleThetas(){
		std::vector<double> cd_angles;
		for( unsigned char i = 0; i < set->GetNumberOfCDDetectors(); i++ )
			for( unsigned char j = 0; j <= set->GetNumberOfCDPStrips(); j++ )
				cd_angles.push_back( GetCDVector(i,0,(float)j-0.5,(float)0).Theta() * TMath::RadToDeg() );
		std::sort( cd_angles.begin(), cd_angles.end() );
		return cd_angles;
	};
	TVector3		GetCDVector( unsigned char det, unsigned char sec, float pid, float nid );
	inline TVector3	GetCDVector( unsigned char det, unsigned char sec, unsigned char pid, unsigned char nid ){
		return GetCDVector( det, sec, (float)pid, (float)nid );
	};
	TVector3		GetParticleVector( unsigned char det, unsigned char sec, unsigned char pid, unsigned char nid );
	inline double	GetParticleTheta( unsigned char det, unsigned char sec, unsigned char pid, unsigned char nid ){
		return GetParticleVector( det, sec, pid, nid ).Theta();
	};
	inline double	GetParticlePhi( unsigned char det, unsigned char sec, unsigned char pid, unsigned char nid ){
		return GetParticleVector( det, sec, pid, nid ).Phi();
	};
	inline double	GetParticleX( unsigned char det, unsigned char sec, unsigned char pid, unsigned char nid ){
		return GetParticleVector( det, sec, pid, nid ).X();
	};
	inline double	GetParticleY( unsigned char det, unsigned char sec, unsigned char pid, unsigned char nid ){
		return GetParticleVector( det, sec, pid, nid ).Y();
	};
	inline double	GetParticleZ( unsigned char det, unsigned char sec, unsigned char pid, unsigned char nid ){
		return GetParticleVector( det, sec, pid, nid ).Z();
	};
	inline TVector3	GetCDVector( std::shared_ptr<ParticleEvt> p ){
		return GetCDVector( p->GetDetector(), p->GetSector(), p->GetStripP(), p->GetStripN() );
	};
	inline TVector3	GetParticleVector( std::shared_ptr<ParticleEvt> p ){
		return GetParticleVector( p->GetDetector(), p->GetSector(), p->GetStripP(), p->GetStripN() );
	};
	inline double	GetParticleTheta( std::shared_ptr<ParticleEvt> p ){
		return GetParticleTheta( p->GetDetector(), p->GetSector(), p->GetStripP(), p->GetStripN() );
	};
	inline double	GetParticlePhi( std::shared_ptr<ParticleEvt> p ){
		return GetParticlePhi( p->GetDetector(), p->GetSector(), p->GetStripP(), p->GetStripN() );
	};
	inline double	GetParticleX( std::shared_ptr<ParticleEvt> p ){
		return GetParticleX( p->GetDetector(), p->GetSector(), p->GetStripP(), p->GetStripN() );
	};
	inline double	GetParticleY( std::shared_ptr<ParticleEvt> p ){
		return GetParticleY( p->GetDetector(), p->GetSector(), p->GetStripP(), p->GetStripN() );
	};
	inline double	GetParticleZ( std::shared_ptr<ParticleEvt> p ){
		return GetParticleZ( p->GetDetector(), p->GetSector(), p->GetStripP(), p->GetStripN() );
	};

	// Miniball geometry functions
	inline void   SetupCluster( unsigned char clu, double user_theta, double user_phi, double user_alpha, double user_r, double user_z) {
		mb_geo[clu].SetupCluster(user_theta, user_phi, user_alpha, user_r, user_z);
	}
	inline double	GetMiniballTheta( unsigned char clu ){
		return mb_geo[clu].GetCluTheta();
	};
	inline double	GetMiniballPhi( unsigned char clu ){
		return mb_geo[clu].GetCluPhi();
	};
	inline double	GetMiniballAlpha( unsigned char clu ){
		return mb_geo[clu].GetCluAlpha();
	};
	inline double	GetMiniballR( unsigned char clu ){
		return mb_geo[clu].GetCluR();
	};
	inline double	GetGammaTheta( unsigned char clu, unsigned char cry, unsigned char seg ){
		return mb_geo[clu].GetSegTheta( cry, seg );
	};
	inline double	GetGammaPhi( unsigned char clu, unsigned char cry, unsigned char seg ){
		return mb_geo[clu].GetSegPhi( cry, seg );
	};
	inline double	GetGammaX( unsigned char clu, unsigned char cry, unsigned char seg ){
		return mb_geo[clu].GetSegX( cry, seg );
	};
	inline double	GetGammaY( unsigned char clu, unsigned char cry, unsigned char seg ){
		return mb_geo[clu].GetSegY( cry, seg );
	};
	inline double	GetGammaZ( unsigned char clu, unsigned char cry, unsigned char seg ){
		return mb_geo[clu].GetSegZ( cry, seg );
	};
	inline double	GetGammaTheta( std::shared_ptr<GammaRayEvt> g ){
		return GetGammaTheta( g->GetCluster(), g->GetCrystal(), g->GetSegment() );
	};
	inline double	GetGammaPhi( std::shared_ptr<GammaRayEvt> g ){
		return GetGammaPhi( g->GetCluster(), g->GetCrystal(), g->GetSegment() );
	};
	inline double	GetGammaX( std::shared_ptr<GammaRayEvt> g ){
		return GetGammaX( g->GetCluster(), g->GetCrystal(), g->GetSegment() );
	};
	inline double	GetGammaY( std::shared_ptr<GammaRayEvt> g ){
		return GetGammaY( g->GetCluster(), g->GetCrystal(), g->GetSegment() );
	};
	inline double	GetGammaZ( std::shared_ptr<GammaRayEvt> g ){
		return GetGammaZ( g->GetCluster(), g->GetCrystal(), g->GetSegment() );
	};

	// SPEDE and electron geometry
	inline double	GetSpedeDistance(){ return spede_dist; };
	inline double	GetSpedePhiOffset(){ return spede_offset; };
	TVector3		GetSpedeVector( unsigned char seg, bool random = false );
	TVector3		GetElectronVector( unsigned char seg );
	inline double	GetElectronTheta( unsigned char seg ){
		return GetElectronVector(seg).Theta();
	};
	inline double	GetElectronTheta( SpedeEvt *s ){
		return GetElectronTheta( s->GetSegment() );
	};
	inline double	GetElectronPhi( unsigned char seg ){
		return GetElectronVector(seg).Phi();
	};
	inline double	GetElectronPhi( SpedeEvt *s ){
		return GetElectronPhi( s->GetSegment() );
	};

	
	// Identify the ejectile and recoil and calculate in Coulex
	void	IdentifyEjectile( std::shared_ptr<ParticleEvt> p, bool kinflag = false );
	void	IdentifyRecoil( std::shared_ptr<ParticleEvt> p, bool kinflag = false );
	void	CalculateEjectile();
	void	CalculateRecoil();

	// Identify the light ion recoil in transfer
	void	TransferProduct( std::shared_ptr<ParticleEvt> p, bool kinflag = false );


	// Reaction calculations
	inline double GetQvalue(){
		return Beam.GetMass() + Target.GetMass() -
			Ejectile.GetMass() - Recoil.GetMass();
	};
	inline double GetEnergyTotLab(){
		return Beam.GetEnergyTot() + Target.GetEnergyTot();
	};
	inline double GetEnergyTotCM(){
		double etot = TMath::Power( Beam.GetMass(), 2.0 );
		etot += TMath::Power( Target.GetMass(), 2.0 );
		etot += 2.0 * Beam.GetEnergyTot() * Target.GetMass();
		etot = TMath::Sqrt( etot );
		return etot;
	};
	inline double GetBeta(){
		double beta2 = 0.25 * Beam.GetMass() + 1.5 * Beam.GetEnergy();
		beta2  = TMath::Sqrt( beta2 * Beam.GetMass() );
		beta2 -= 0.5 * Beam.GetMass();
		return TMath::Sqrt( beta2 / ( 0.75 * Beam.GetMass() ) );
		//return TMath::Sqrt( 2.0 * Beam.GetEnergy() / Beam.GetMass() );
	};
	inline double GetGamma(){
		return 1.0 / TMath::Sqrt( 1.0 - TMath::Power( GetBeta(), 2.0 ) );
	};
	inline double GetTau(){
		return Beam.GetMass() / Target.GetMass();
	};
	inline double GetEnergyPrime(){
		return Beam.GetEnergy() - ( Ejectile.GetEx() + Recoil.GetEx() ) * ( 1.0 + GetTau() );
	};
	inline double GetEpsilon(){
		return TMath::Sqrt( Beam.GetEnergy() / GetEnergyPrime() );
	};

	
	// Doppler correction
	double DopplerShift( double gen, double pbeta, double costheta );
	double DopplerCorrection( double gen, double gth, double gph, double pbeta, double ptheta, double pphi );
	double DopplerCorrection( double gen, double gth, double gph, bool ejectile );
	double DopplerCorrection( std::shared_ptr<GammaRayEvt> g, double pbeta, double ptheta, double pphi );
	double DopplerCorrection( std::shared_ptr<GammaRayEvt> g, bool ejectile );
	double DopplerCorrection( std::shared_ptr<SpedeEvt> s, bool ejectile );
	double CosTheta( std::shared_ptr<GammaRayEvt> g, bool ejectile );
	double CosTheta( std::shared_ptr<SpedeEvt> s, bool ejectile );


	// Get EBIS times
	inline double GetEBISOnTime(){ return EBIS_On; };
	inline double GetEBISOffTime(){ return EBIS_Off; };
	inline double GetEBISTimeRatio(){ return EBIS_On / ( EBIS_Off - EBIS_On ); };
	inline double GetEBISFillRatio(){ return EBIS_ratio; };
	
	
	// Get T1 times
	inline bool GetT1Cut(){ return t1_cut; };
	inline double GetT1MinTime(){ return t1_time[0]; };
	inline double GetT1MaxTime(){ return t1_time[1]; };

	// Get particle gamma coincidence times
	inline double GetParticleGammaPromptTime( unsigned char i ){
		// i = 0 for lower limit and i = 1 for upper limit
		if( i < 2 ) return pg_prompt[i];
		else return 0;
	};
	inline double GetParticleGammaRandomTime( unsigned char i ){
		// i = 0 for lower limit and i = 1 for upper limit
		if( i < 2 ) return pg_random[i];
		else return 0;
	};
	inline double GetParticleGammaTimeRatio(){
		return ( pg_prompt[1] - pg_prompt[0] ) / ( pg_random[1] - pg_random[0] );
	};
	inline double GetParticleGammaFillRatio(){
		return pg_ratio;
	};
	inline double GetGammaGammaPromptTime( unsigned char i ){
		// i = 0 for lower limit and i = 1 for upper limit
		if( i < 2 ) return gg_prompt[i];
		else return 0;
	};
	inline double GetGammaGammaRandomTime( unsigned char i ){
		// i = 0 for lower limit and i = 1 for upper limit
		if( i < 2 ) return gg_random[i];
		else return 0;
	};
	inline double GetGammaGammaTimeRatio(){
		return ( gg_prompt[1] - gg_prompt[0] ) / ( gg_random[1] - gg_random[0] );
	};
	inline double GetGammaGammaFillRatio(){
		return gg_ratio;
	};
	inline double GetParticleParticlePromptTime( unsigned char i ){
		// i = 0 for lower limit and i = 1 for upper limit
		if( i < 2 ) return pp_prompt[i];
		else return 0;
	};
	inline double GetParticleParticleRandomTime( unsigned char i ){
		// i = 0 for lower limit and i = 1 for upper limit
		if( i < 2 ) return pp_random[i];
		else return 0;
	};
	inline double GetParticleParticleTimeRatio(){
		return ( pp_prompt[1] - pp_prompt[0] ) / ( pp_random[1] - pp_random[0] );
	};
	inline double GetParticleParticleFillRatio(){
		return pp_ratio;
	};
	inline double GetGammaElectronPromptTime( unsigned char i ){
		// i = 0 for lower limit and i = 1 for upper limit
		if( i < 2 ) return ge_prompt[i];
		else return 0;
	};
	inline double GetGammaElectronRandomTime( unsigned char i ){
		// i = 0 for lower limit and i = 1 for upper limit
		if( i < 2 ) return ge_random[i];
		else return 0;
	};
	inline double GetGammaElectronTimeRatio(){
		return ( ge_prompt[1] - ge_prompt[0] ) / ( ge_random[1] - ge_random[0] );
	};
	inline double GetGammaElectronFillRatio(){
		return ge_ratio;
	};
	inline double GetElectronElectronPromptTime( unsigned char i ){
		// i = 0 for lower limit and i = 1 for upper limit
		if( i < 2 ) return ee_prompt[i];
		else return 0;
	};
	inline double GetElectronElectronRandomTime( unsigned char i ){
		// i = 0 for lower limit and i = 1 for upper limit
		if( i < 2 ) return ee_random[i];
		else return 0;
	};
	inline double GetElectronElectronTimeRatio(){
		return ( ee_prompt[1] - ee_prompt[0] ) / ( ee_random[1] - ee_random[0] );
	};
	inline double GetElectronElectronFillRatio(){
		return ee_ratio;
	};
	inline double GetParticleElectronPromptTime( unsigned char i ){
		// i = 0 for lower limit and i = 1 for upper limit
		if( i < 2 ) return pe_prompt[i];
		else return 0;
	};
	inline double GetParticleElectronRandomTime( unsigned char i ){
		// i = 0 for lower limit and i = 1 for upper limit
		if( i < 2 ) return pe_random[i];
		else return 0;
	};
	inline double GetParticleElectronTimeRatio(){
		return ( pe_prompt[1] - pe_prompt[0] ) / ( pe_random[1] - pe_random[0] );
	};
	inline double GetParticleElectronFillRatio(){
		return pe_ratio;
	};

	// Energy loss and stopping powers
	double GetEnergyLoss( double Ei, double dist, std::unique_ptr<TGraph> &g );
	bool ReadStoppingPowers( std::string isotope1, std::string isotope2, std::unique_ptr<TGraph> &g );

	
	// Getter for target offsets
	inline double GetOffsetX(){ return x_offset; };
	inline double GetOffsetY(){ return y_offset; };
	inline double GetOffsetZ(){ return z_offset; };

	// Get cuts
	inline TCutG* GetEjectileCut(){ return ejectile_cut; };
	inline TCutG* GetRecoilCut(){ return recoil_cut; };
	inline TCutG* GetTransferCut(){ return transfer_cut; };
	inline std::string GetTransferX(){ return transfercut_x; };
	inline std::string GetTransferY(){ return transfercut_y; };

	// Get particles
	inline MiniballParticle* GetBeam(){ return &Beam; };
	inline MiniballParticle* GetTarget(){ return &Target; };
	inline MiniballParticle* GetEjectile(){ return &Ejectile; };
	inline MiniballParticle* GetRecoil(){ return &Recoil; };
	
	// Timing for coincidence
	inline unsigned long long GetParticleTime(){ return particle_time; };
	inline void ResetParticles(){
		ejectile_detected = false;
		recoil_detected = false;
		transfer_detected = false;
	};
	inline void SetParticleTime( unsigned long long t ){ particle_time = t; };
	inline bool IsEjectileDetected(){ return ejectile_detected; };
	inline bool IsRecoilDetected(){ return recoil_detected; };
	inline bool IsTransferDetected(){ return transfer_detected; };

	// Events tree options
	inline bool EventsParticleGammaOnly(){ return events_particle_gamma; };
	
	// Histogram options
	inline bool HistSegmentPhi(){ return hist_segment_phi; };
	inline bool HistByCrystal(){ return hist_by_crystal; };
	inline bool HistByMultiplicity(){ return hist_by_pmult; };
	inline bool HistBySector(){ return hist_by_sector; };
	inline bool HistByT1(){ return hist_by_t1; };
	inline bool HistGammaGamma(){ return hist_gamma_gamma; };
	inline bool HistElectron(){ return hist_electron; };
	inline bool HistElectronGamma(){ return hist_electron_gamma; };
	inline bool HistBeamDump(){ return hist_beam_dump; };
	inline bool HistIonChamber(){ return hist_ion_chamb; };
	
	// Histogram ranges
	inline unsigned int HistGammaBins(){ return gamma_bins; }
	inline double HistGammaMin(){ return gamma_range[0]; }
	inline double HistGammaMax(){ return gamma_range[1]; }
	inline unsigned int HistElectronBins(){ return electron_bins; }
	inline double HistElectronMin(){ return electron_range[0]; }
	inline double HistElectronMax(){ return electron_range[1]; }
	inline unsigned int HistParticleBins(){ return particle_bins; }
	inline double HistParticleMin(){ return particle_range[0]; }
	inline double HistParticleMax(){ return particle_range[1]; }

	ClassDef( MiniballReaction, 3 )

private:

	std::string fInputFile;
	
	// Settings file
	std::shared_ptr<MiniballSettings> set;
	
	// Mass tables
	std::map< std::string, double > ame_be; ///< List of biniding energies from  AME2021

	// Reaction partners
	MiniballParticle Beam, Target, Ejectile, Recoil;
	
	// Reaction times and flags for coincidences
	unsigned long long particle_time;
	bool ejectile_detected, recoil_detected, transfer_detected;

	// Initial properties from file
	double Eb;		///< laboratory beam energy in keV/u

	// EBIS time windows
	double EBIS_On;		///< beam on max time in ns
	double EBIS_Off;	///< beam off max time in ns
	double EBIS_ratio;	///< ratio of ebis on/off as measured

	// T1 cut times
	bool t1_cut;		///< enable/disable T1 cuts on data
	double t1_time[2];	///< event time - T1 cut window
	
	// Particle and Gamma coincidences windows
	int pg_prompt[2];	///< particle-gamma prompt
	int pg_random[2];	///< particle-gamma random
	int gg_prompt[2];	///< gamma-gamma prompt
	int gg_random[2];	///< gamma-gamma random
	int pp_prompt[2];	///< particle-particle prompt
	int pp_random[2];	///< particle-particle random
	int ge_prompt[2];	///< gamma-electron prompt
	int ge_random[2];	///< gamma-electron random
	int ee_prompt[2];	///< electron-electron prompt
	int ee_random[2];	///< electron-electron random
	int pe_prompt[2];	///< particle-electron prompt
	int pe_random[2];	///< particle-electron random
	double pg_ratio, gg_ratio, pp_ratio; ///< fill ratios
	double pe_ratio, ge_ratio, ee_ratio; ///< fill ratios

	// Target thickness and offsets
	double target_thickness;	///< target thickness in units of mg/cm^2
	double x_offset;			///< horizontal offset of the target/beam position, with respect to the CD and Miniball in mm
	double y_offset;			///< vertical offset of the target/beam position, with respect to the CD and Miniball in mm
	double z_offset;			///< lateral offset of the target/beam position, with respect to the only Miniball in mm (cd_dist is independent)

	// CD detector things
	std::vector<double> cd_dist;		///< distance from target to CD detector in mm
	std::vector<double> cd_offset;	///< phi rotation of the CD in degrees
	std::vector<double> dead_layer;	///< dead layer thickness in mm

	// Miniball detector things
	std::vector<MiniballGeometry> mb_geo;
	std::vector<double> mb_theta, mb_phi, mb_alpha, mb_r;
	unsigned char mb_type;

	// SPEDE things
	double spede_dist;	///< distance from target to SPEDE detector
	double spede_offset;	///< phi rotation of the SPEDE detector
	
	unsigned char doppler_mode; 	///< Doppler mode, calculating the velocity for Doppler correction
									///< 0 = use angles and two-body kinematics at centre of the target
									///< 1 = like 0, but corrected for energy loss through the back of the target
									///< 2 = use energy of particle in the CD detector
									///< 3 = like 2, but corrected for energy loss in dead-layer

	
	unsigned char laser_mode;		///< Laser status mode:
									///< 0 = OFF, 1 = ON, 2 = OFF or ON
	
	// Events tree options
	bool events_particle_gamma;
	
	// Histogram options
	bool hist_segment_phi;
	bool hist_by_crystal;
	bool hist_by_pmult;
	bool hist_by_sector;
	bool hist_by_t1;
	bool hist_gamma_gamma;
	bool hist_electron;
	bool hist_electron_gamma;
	bool hist_beam_dump;
	bool hist_ion_chamb;
	
	// Histogram ranges
	unsigned int gamma_bins, electron_bins, particle_bins;
	double gamma_range[2], electron_range[2], particle_range[2];

	// Random numbers
	TRandom rand;

	// Cuts
	std::string ejectilecutfile, ejectilecutname;
	std::string recoilcutfile, recoilcutname;
	std::string transfercutfile, transfercutname;
	std::string transfercut_x, transfercut_y;
	TFile *cut_file;
	TCutG *ejectile_cut, *recoil_cut, *transfer_cut;
	
	// Stopping powers
	std::vector<std::unique_ptr<TGraph>> gStopping;
	bool stopping;
	
};

#endif
