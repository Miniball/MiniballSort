#ifndef __MINIBALLEVTS_HH
#define __MINIBALLEVTS_HH

#include <iostream>
#include <vector>
#include <string>

#include "TVector2.h"
#include "TVector3.h"
#include "TObject.h"

class GammaRayEvt : public TObject {

public:
	
	// setup functions
	GammaRayEvt() {};
	~GammaRayEvt() {};

	// Event set functions
	inline void SetEnergy( float e ){ energy = e; };
	inline void SetTime( unsigned long long t ){ time = t; };
	inline void SetCluster( unsigned char c ){ clu = c; };
	inline void SetCrystal( unsigned char c ){ cry = c; };
	inline void SetSegment( unsigned char s ){ seg = s; };
	
	// Return functions
	inline float 				GetEnergy(){ return energy; };
	inline unsigned long long	GetTime(){ return time; };
	inline unsigned char		GetCluster(){ return clu; };
	inline unsigned char		GetCrystal(){ return cry; };
	inline unsigned char		GetSegment(){ return seg; };

	// Geometry functions
	//inline float				GetTheta(){ return theta; };
	//inline float				GetPhi(){ return phi; };
	//TVector3					GetUnitVector(){
	//	TVector3 pos( 1, 1, 1 );
	//	pos.SetTheta( GetTheta() );
	//	pos.SetPhi( GetPhi() );
	//	return pos;
	//};

private:

	// variables for gamma-ray event
	float				energy;		///< energy in keV
	unsigned long long	time;		///< timestamp of event
	unsigned char		clu;		///< cluster ID
	unsigned char		cry;		///< crystal ID
	unsigned char		seg;		///< segment ID


	ClassDef( GammaRayEvt, 1 )

};

class GammaRayAddbackEvt : public GammaRayEvt {

public:
	
	// setup functions
	GammaRayAddbackEvt() {};
	~GammaRayAddbackEvt() {};

private:

	ClassDef( GammaRayAddbackEvt, 1 )

};

class ParticleEvt : public TObject {

public:
	
	// setup functions
	ParticleEvt() {};
	~ParticleEvt() {};

	// Event set functions
	inline void SetEnergyP( float e ){ penergy = e; };
	inline void SetEnergyN( float e ){ nenergy = e; };
	inline void SetTimeP( unsigned long long t ){ ptime = t; };
	inline void SetTimeN( unsigned long long t ){ ntime = t; };
	inline void SetDetector( unsigned char d ){ det = d; };
	inline void SetSector( unsigned char s ){ sec = s; };
	inline void SetStripP( unsigned char s ){ pstrip = s; };
	inline void SetStripN( unsigned char s ){ nstrip = s; };

	// Return functions
	inline float 				GetEnergy(){ return GetEnergyP(); };
	inline unsigned long long	GetTime(){ return GetTimeP(); };
	inline float 				GetEnergyP(){ return penergy; };
	inline float 				GetEnergyN(){ return nenergy; };
	inline unsigned long long	GetTimeP(){ return ptime; };
	inline unsigned long long	GetTimeN(){ return ntime; };
	inline unsigned char		GetDetector(){ return det; };
	inline unsigned char		GetSector(){ return sec; };
	inline unsigned char		GetStripP(){ return pstrip; };
	inline unsigned char		GetStripN(){ return nstrip; };


private:

	// variables for particle event
	float				penergy;		///< p-side energy in keV
	float				nenergy;		///< n-side energy in keV
	unsigned long long	ptime;			///< p-side timestamp of event
	unsigned long long	ntime;			///< n-side timestamp of event
	unsigned char		det;			///< detector ID (=0 for forward CD)
	unsigned char		sec;			///< sector ID (0-3 for quadrants)
	unsigned char		pstrip;			///< p-side strip ID
	unsigned char		nstrip;			///< n-side strip ID


	ClassDef( ParticleEvt, 1 )

};

class MiniballEvts : public TObject {

public:
	
	// setup functions
	MiniballEvts() {};
	~MiniballEvts() {};
	
	void AddEvt( GammaRayEvt *event );
	void AddEvt( GammaRayAddbackEvt *event );
	void AddEvt( ParticleEvt *event );

	inline unsigned int GetGammaRayMultiplicity(){ return gamma_event.size(); };
	inline unsigned int GetGammaRayAddbackMultiplicity(){ return gamma_ab_event.size(); };
	inline unsigned int GetParticleMultiplicity(){ return particle_event.size(); };

	inline GammaRayEvt* GetGammaRayEvt( unsigned int i ){
		if( i < gamma_event.size() ) return &gamma_event.at(i);
		else return nullptr;
	};
	inline GammaRayAddbackEvt* GetGammaRayAddbackEvt( unsigned int i ){
		if( i < gamma_ab_event.size() ) return &gamma_ab_event.at(i);
		else return nullptr;
	};
	inline ParticleEvt* GetParticleEvt( unsigned int i ){
		if( i < particle_event.size() ) return &particle_event.at(i);
		else return nullptr;
	};

	void ClearEvt();
	
	// ISOLDE timestamping
	inline void SetEBIS( unsigned long t ){ ebis = t; return; };
	inline void SetT1( unsigned long t ){ t1 = t; return; };
	
	inline unsigned long GetEBIS(){ return ebis; };
	inline unsigned long GetT1(){ return t1; };

	
private:
	
	// variables for timestamping
	unsigned long ebis;		///< absolute EBIS pulse time
	unsigned long t1;		///< absolute proton pulse time

	std::vector<GammaRayEvt> gamma_event;
	std::vector<GammaRayAddbackEvt> gamma_ab_event;
	std::vector<ParticleEvt> particle_event;

	ClassDef( MiniballEvts, 1 )
	
};

#endif

