#ifndef __REACTION_HH
#define __REACTION_HH

#include <iostream>
#include <vector>
#include <string>
#include <map>

#include "TSystem.h"
#include "TEnv.h"
#include "TMath.h"
#include "TObject.h"
#include "TString.h"
#include "TFile.h"
#include "TCutG.h"
#include "TVector3.h"
#include "TF1.h"
#include "TError.h"

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


const double p_mass = 938272.08816;	///< mass of the proton in keV/c^2
const double n_mass = 939565.42052;	///< mass of the neutron in keV/c^2
const double u_mass = 931494.10242;	///< atomic mass unit in keV/c^2

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

class Particle : public TObject {
	
public:
	
	// setup functions
	Particle();
	~Particle();
	
	// Get properties
	inline double		GetMass_u(){
		return GetMass() / u_mass;
	};			// returns mass in u
	inline double		GetMass(){
		double mass = (double)GetN() * n_mass;
		mass += (double)GetZ() * p_mass;
		mass -= (double)GetA() * bindingE;
		return mass;
	};		// returns mass in keV/c^2
	inline int			GetA(){ return A; };	// returns mass number
	inline int			GetZ(){ return Z; };
	inline int			GetN(){ return A-Z; };
	inline std::string	GetIsotope(){
		return std::to_string( GetA() ) + gElName.at( GetZ() );
	};
	inline double		GetBindingEnergy(){ return bindingE; };
	inline double		GetEnergyLab(){ return Elab; };
	inline double		GetEnergyTotLab(){
		return GetMass() + GetEnergyLab();
	};
	inline double		GetEnergyTotCM(){ return Ecm_tot; };
	inline double		GetMomentumLab(){
		return TMath::Sqrt( TMath::Power( GetEnergyTotLab(), 2.0 ) - TMath::Power( GetMass(), 2.0 ) );
	};
	inline double		GetMomentumCM(){
		return TMath::Sqrt( TMath::Power( GetEnergyTotCM(), 2.0 ) - TMath::Power( GetMass(), 2.0 ) );
	};
	inline double		GetGamma(){
		return GetEnergyTotLab() / GetMass();
	};
	inline double		GetThetaCM(){ return ThetaCM; };
	inline double		GetThetaLab(){ return ThetaLab; };
	inline double		GetEx(){ return Ex; };

	// Set properties
	inline void		SetA( int myA ){ A = myA; };
	inline void		SetZ( int myZ ){ Z = myZ; };
	inline void		SetBindingEnergy( double myBE ){ bindingE = myBE; };
	inline void		SetEnergyLab( double myElab ){ Elab = myElab; };
	inline void		SetEnergyTotCM( double myEcm ){ Ecm_tot = myEcm; };
	inline void		SetThetaCM( double mytheta ){ ThetaCM = mytheta; };
	inline void		SetThetaLab( double mytheta ){ ThetaLab = mytheta; };
	inline void		SetEx( double myEx ){ Ex = myEx; };


private:
	
	// Properties of reaction particles
	int		A;			///< mass number, A of the particle, obviously
	int		Z; 			///< The Z of the particle, obviously
	double	bindingE;	///< binding energy per nucleon in keV/c^2
	double	Ecm_tot;	///< total  energy in the centre of mass frame
	double	Elab;		///< energy in the laboratory system
	double	ThetaCM;	///< theta in the centre of mass frame in radians
	double	ThetaLab;	///< theta in the laboratory system in radians
	double	Ex;			///< Excitation energy in keV

	
	ClassDef( Particle, 1 )
	
};

class Reaction {
	
public:
	
	// setup functions
	Reaction( std::string filename, Settings *myset );
	~Reaction() {};
	
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
	
	// Get values for geometry
	inline double			GetCDDistance(){ return cd_dist; };
	inline unsigned int		GetNumberOfParticleThetas() {
		return set->GetNumberOfCDPStrips() * set->GetNumberOfCDDetectors();
	};
	inline double*			GetParticleThetas(){
		std::vector<double> cd_angles;
		for( unsigned char i = 0; i < set->GetNumberOfCDDetectors(); i++ )
			for( unsigned char j = 0; j < set->GetNumberOfCDPStrips(); j++ )
				cd_angles.push_back( GetParticleTheta(i,0,j) * TMath::RadToDeg() );
		return cd_angles.data();
	};
	float	GetParticleTheta( unsigned char det, unsigned char sec, unsigned char strip );
	float	GetParticleTheta( ParticleEvt *p );
	float	GetParticlePhi( ParticleEvt *p );
	float	GetGammaTheta( unsigned char clu, unsigned char cry, unsigned char seg );
	float	GetGammaTheta( GammaRayEvt *g );
	float	GetGammaTheta( GammaRayAddbackEvt *g );
	float	GetGammaPhi( unsigned char clu, unsigned char cry, unsigned char seg );
	float	GetGammaPhi( GammaRayEvt *g );
	float	GetGammaPhi( GammaRayAddbackEvt *g );

	// Reaction calculations
	inline double GetQvalue(){
		return Beam.GetMass() + Target.GetMass() -
			Ejectile.GetMass() - Recoil.GetMass();
	};
	inline double GetEnergyTotLab(){
		return Beam.GetEnergyTotLab() + Target.GetEnergyTotLab();
	};
	inline double GetEnergyTotCM(){
		double etot = TMath::Power( Beam.GetMass(), 2.0 );
		etot += TMath::Power( Target.GetMass(), 2.0 );
		etot += 2.0 * Beam.GetEnergyTotLab() * Target.GetMass();
		etot = TMath::Sqrt( etot );
		return etot;
	};
	inline double GetBeta(){
		return TMath::Sqrt( 2.0 * Beam.GetEnergyLab() / Beam.GetMass() );
	};
	inline double GetGamma(){
		return 1.0 / TMath::Sqrt( 1.0 - TMath::Power( GetBeta(), 2.0 ) );
	};

	// Get EBIS times
	inline double GetEBISOnTime(){ return EBIS_On; };
	inline double GetEBISOffTime(){ return EBIS_Off; };
	inline double GetEBISRatio(){ return EBIS_On / ( EBIS_Off - EBIS_On ); };

	// Set values for CD
	inline void	SetCDDistance( double d ){ cd_dist = d; };
	inline void	SetCDPhiOffset( double o ){ cd_offset = o; };

	// Target offsets
	inline void SetOffsetX( double x ){ x_offset = x; };
	inline void SetOffsetY( double y ){ y_offset = y; };
	inline void SetOffsetZ( double z ){ z_offset = z; };

	// Get cuts
	inline TCutG* GetBeamCut(){ return beam_cut; };
	inline TCutG* GetTargetCut(){ return target_cut; };

private:

	std::string fInputFile;
	
	// Settings file
	Settings *set;
	
	// Mass tables
	std::map< std::string, double > ame_be; ///< List of biniding energies from  AME2021

	// Reaction partners
	Particle Beam, Target, Ejectile, Recoil;
	
	// Initial properties from file
	double Eb;		///< laboratory beam energy in keV/u

	// EBIS time windows
	double EBIS_On;		///< beam on max time in ns
	double EBIS_Off;	///< beam off max time in ns

	// Target offsets
	double x_offset;	///< horizontal offset of the target/beam position, with respect to the CD and Miniball in mm
	double y_offset;	///< vertical offset of the target/beam position, with respect to the CD and Miniball in mm
	double z_offset;	///< lateral offset of the target/beam position, with respect to the only Miniball in mm (cd_dist is independent)

	// CD detector things
	float cd_dist;		///< distance from target to CD detector in mm
	float cd_offset;	///< phi rotation of the CD in radians
	
	// Miniball detector things
	std::vector<MiniballGeometry> mb_geo;
	std::vector<float> mb_theta, mb_phi, mb_alpha, mb_r;

	
	// Cuts
	std::string beamcutfile, beamcutname;
	std::string targetcutfile, targetcutname;
	TFile *cut_file;
	TCutG *beam_cut, *target_cut;
	
};

#endif
