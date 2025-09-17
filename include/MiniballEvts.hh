#ifndef __MINIBALLEVTS_HH
#define __MINIBALLEVTS_HH

#include <iostream>
#include <vector>
#include <string>
#include <memory>

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
	inline void SetSegmentSumEnergy( float e ){ seg_sum_energy = e; };
	inline void SetSegmentMaxEnergy( float e ){ seg_max_energy = e; };
	inline void SetSegmentMultiplicity( unsigned int m ){ seg_mult = m; };
	inline void SetAddbackMultiplicity( unsigned int m ){ ab_mult = m; };
	inline void SetTime( unsigned long long t ){ time = t; };
	inline void SetCluster( unsigned char c ){ clu = c; };
	inline void SetCrystal( unsigned char c ){ cry = c; };
	inline void SetSegment( unsigned char s ){ seg = s; };
	
	// Return functions
	inline float 				GetEnergy(){ return energy; };
	inline float 				GetSegmentSumEnergy(){ return seg_sum_energy; };
	inline float 				GetSegmentMaxEnergy(){ return seg_max_energy; };
	inline unsigned int			GetSegmentMultiplicity(){ return seg_mult; };
	inline unsigned int			GetAddbackMultiplicity(){ return ab_mult; };
	inline unsigned long long	GetTime(){ return time; };
	inline unsigned char		GetCluster(){ return clu; };
	inline unsigned char		GetCrystal(){ return cry; };
	inline unsigned char		GetSegment(){ return seg; };

private:

	// variables for gamma-ray event
	float				energy;			///< energy in keV
	float				seg_max_energy;	///< energy in keV for the hit segment
	float				seg_sum_energy;	///< energy in keV for the hit segment
	unsigned long long	time;			///< timestamp of event
	unsigned char		seg_mult;		///< segment multiplicity
	unsigned char		ab_mult;		///< add-back multiplicity
	unsigned char		clu;			///< cluster ID
	unsigned char		cry;			///< crystal ID
	unsigned char		seg;			///< segment ID


	ClassDef( GammaRayEvt, 2 )

};

class GammaRayAddbackEvt : public GammaRayEvt {

public:
	
	// setup functions
	GammaRayAddbackEvt() {};
	~GammaRayAddbackEvt() {};

private:

	ClassDef( GammaRayAddbackEvt, 2 )

};

class ParticleEvt : public TObject {

public:
	
	// setup functions
	ParticleEvt() {};
	~ParticleEvt() {};

	// Event set functions
	inline void SetEnergyP( float e ){ penergy = e; };
	inline void SetEnergyN( float e ){ nenergy = e; };
	inline void SetEnergyPad( float e ){ padenergy = e; };
	inline void SetTimeP( unsigned long long t ){ ptime = t; };
	inline void SetTimeN( unsigned long long t ){ ntime = t; };
	inline void SetTimePad( unsigned long long t ){ rtime = t; };
	inline void SetDetector( unsigned char d ){ det = d; };
	inline void SetSector( unsigned char s ){ sec = s; };
	inline void SetStripP( unsigned char s ){ pstrip = s; };
	inline void SetStripN( unsigned char s ){ nstrip = s; };

	// Return functions
	inline float 				GetEnergy(){ return GetEnergyP() + GetEnergyPad(); };
	inline float 				GetDeltaEnergy(){ return GetEnergyP(); };
	inline float 				GetRestEnergy(){ return GetEnergyPad(); };
	inline unsigned long long	GetTime(){ return GetTimeP(); };
	inline float 				GetEnergyP(){ return penergy; };
	inline float 				GetEnergyN(){ return nenergy; };
	inline float 				GetEnergyPad(){ return padenergy; };
	inline unsigned long long	GetTimeP(){ return ptime; };
	inline unsigned long long	GetTimeN(){ return ntime; };
	inline unsigned long long	GetTimePad(){ return rtime; };
	inline unsigned char		GetDetector(){ return det; };
	inline unsigned char		GetSector(){ return sec; };
	inline unsigned char		GetStripP(){ return pstrip; };
	inline unsigned char		GetStripN(){ return nstrip; };


private:

	// variables for particle event
	float				penergy;		///< p-side energy in keV
	float				nenergy;		///< n-side energy in keV
	float				padenergy;		///< pad energy in keV
	unsigned long long	ptime;			///< p-side timestamp of event
	unsigned long long	ntime;			///< n-side timestamp of event
	unsigned long long	rtime;			///< pad timestamp of event
	unsigned char		det;			///< detector ID (=0 for forward CD)
	unsigned char		sec;			///< sector ID (0-3 for quadrants)
	unsigned char		pstrip;			///< p-side strip ID
	unsigned char		nstrip;			///< n-side strip ID


	ClassDef( ParticleEvt, 2 )

};

class BeamDumpEvt : public TObject {

public:
	
	// setup functions
	BeamDumpEvt() {};
	~BeamDumpEvt() {};

	// Event set functions
	inline void SetEnergy( float e ){ energy = e; };
	inline void SetTime( unsigned long long t ){ time = t; };
	inline void SetDetector( unsigned char d ){ det = d; };
	
	// Return functions
	inline float 				GetEnergy(){ return energy; };
	inline unsigned long long	GetTime(){ return time; };
	inline unsigned char		GetDetector(){ return det; };

private:

	// variables for beam-dump event
	float				energy;		///< energy in keV
	unsigned long long	time;		///< timestamp of event
	unsigned char		det;		///< detector ID


	ClassDef( BeamDumpEvt, 1 )

};


class SpedeEvt : public TObject {

public:
	
	// setup functions
	SpedeEvt() {};
	~SpedeEvt() {};

	// Event set functions
	inline void SetEnergy( float e ){ energy = e; };
	inline void SetTime( unsigned long long t ){ time = t; };
	inline void SetSegment( unsigned char s ){ seg = s; };
	
	// Return functions
	inline float 				GetEnergy(){ return energy; };
	inline unsigned long long	GetTime(){ return time; };
	inline unsigned char		GetSegment(){ return seg; };

private:

	// variables for Spede event
	float				energy;		///< energy in keV
	unsigned long long	time;		///< timestamp of event
	unsigned char		seg;		///< segment ID within SPEDE


	ClassDef( SpedeEvt, 1 )

};

class IonChamberEvt : public TObject {

public:
		
	// setup functions
	IonChamberEvt() {};
	~IonChamberEvt() {};

	inline void AddIonChamber( float myenergy, unsigned char myid ){
		energy.push_back( myenergy );
		id.push_back( myid );
	};
	
	void ClearEvt();

	inline void SetdETime( unsigned long t ){ detime = t; };
	inline void SetETime( unsigned long t ){ etime = t; };
	inline void	SetEnergies( std::vector<float> x ){ energy = x; };
	inline void	SetIDs( std::vector<unsigned char> x ){ id = x; };

	inline unsigned long	GetTime(){ return detime; };
	inline unsigned long	GetdETime(){ return detime; };
	inline unsigned long	GetETime(){ return etime; };
	inline std::vector<float>			GetEnergies(){ return energy; };
	inline std::vector<unsigned char>	GetIDs(){ return id; };

	inline float GetEnergy( unsigned char i ){
		if( i < energy.size() ) return energy.at(i);
		else return 0;
	};
	
	inline float GetEnergyLoss( unsigned char start = 0, unsigned char stop = 0 ){
		float total = 0;
		for( unsigned int j = 0; j < energy.size(); ++j )
			if( GetID(j) >= start && GetID(j) <= stop )
				total += energy.at(j);
		return total;
	};

	inline float GetEnergyRest( unsigned char start = 1, unsigned char stop = 1 ){
		float total = 0;
		for( unsigned int j = 0; j < energy.size(); ++j )
			if( GetID(j) >= start && GetID(j) <= stop )
				total += energy.at(j);
		return total;
	};
	
	inline float GetEnergyTotal( unsigned char start = 0, unsigned char stop = 1 ){
		float total = 0;
		for( unsigned int j = 0; j < energy.size(); ++j )
			if( GetID(j) >= start && GetID(j) <= stop )
				total += energy.at(j);
		return total;
	};

	inline int GetID( unsigned char i ){
		if( i < id.size() ) return id.at(i);
		else return -1;
	};

private:

	std::vector<float>			energy;	///< differential energy list
	std::vector<unsigned char>	id;		///< differential id list, i.e. dE = 0, E = 1, for example
	unsigned long				detime;	///< time stamp of gas event
	unsigned long				etime;	///< time stamp of silicon event

	ClassDef( IonChamberEvt, 1 )

};


class MiniballEvts : public TObject {

public:
	
	// setup functions
	MiniballEvts() {};
	~MiniballEvts() {};
	
	void AddEvt( std::shared_ptr<GammaRayEvt> event );
	void AddEvt( std::shared_ptr<GammaRayAddbackEvt> event );
	void AddEvt( std::shared_ptr<ParticleEvt> event );
	void AddEvt( std::shared_ptr<BeamDumpEvt> event );
	void AddEvt( std::shared_ptr<SpedeEvt> event );
	void AddEvt( std::shared_ptr<IonChamberEvt> event );

	inline unsigned int GetGammaRayMultiplicity(){ return gamma_event.size(); };
	inline unsigned int GetGammaRayAddbackMultiplicity(){ return gamma_ab_event.size(); };
	inline unsigned int GetParticleMultiplicity(){ return particle_event.size(); };
	inline unsigned int GetBeamDumpMultiplicity(){ return bd_event.size(); };
	inline unsigned int GetSpedeMultiplicity(){ return spede_event.size(); };
	inline unsigned int GetIonChamberMultiplicity(){ return ic_event.size(); };

	inline std::shared_ptr<GammaRayEvt> GetGammaRayEvt( unsigned int i ){
		if( i < gamma_event.size() ) return std::make_shared<GammaRayEvt>( gamma_event.at(i) );
		else return nullptr;
	};
	inline std::shared_ptr<GammaRayAddbackEvt> GetGammaRayAddbackEvt( unsigned int i ){
		if( i < gamma_ab_event.size() ) return std::make_shared<GammaRayAddbackEvt>( gamma_ab_event.at(i) );
		else return nullptr;
	};
	inline std::shared_ptr<ParticleEvt> GetParticleEvt( unsigned int i ){
		if( i < particle_event.size() ) return std::make_shared<ParticleEvt>( particle_event.at(i) );
		else return nullptr;
	};
	inline std::shared_ptr<BeamDumpEvt> GetBeamDumpEvt( unsigned int i ){
		if( i < bd_event.size() ) return std::make_shared<BeamDumpEvt>( bd_event.at(i) );
		else return nullptr;
	};
	inline std::shared_ptr<SpedeEvt> GetSpedeEvt( unsigned int i ){
		if( i < spede_event.size() ) return std::make_shared<SpedeEvt>( spede_event.at(i) );
		else return nullptr;
	};
	inline std::shared_ptr<IonChamberEvt> GetIonChamberEvt( unsigned int i ){
		if( i < ic_event.size() ) return std::make_shared<IonChamberEvt>( ic_event.at(i) );
		else return nullptr;
	};

	void ClearEvt();
	
	// ISOLDE timestamping
	inline void SetEBIS( unsigned long t ){ ebis = t; return; };
	inline void SetT1( unsigned long t ){ t1 = t; return; };
	inline void SetSC( unsigned long t ){ sc = t; return; };
	inline void SetLaserStatus( bool l ){ laser = l; return; };
	
	inline unsigned long GetEBIS(){ return ebis; };
	inline unsigned long GetT1(){ return t1; };
	inline unsigned long GetSC(){ return sc; };
	inline bool GetLaserStatus(){ return laser; };

	
private:
	
	// variables for timestamping
	unsigned long ebis;		///< absolute EBIS pulse time
	unsigned long t1;		///< absolute proton pulse time
	unsigned long sc;		///< absolute SuperCycle time
	bool laser;				///< RILIS laser on (true) or off (false)

	std::vector<GammaRayEvt> gamma_event;
	std::vector<GammaRayAddbackEvt> gamma_ab_event;
	std::vector<ParticleEvt> particle_event;
	std::vector<BeamDumpEvt> bd_event;
	std::vector<SpedeEvt> spede_event;
	std::vector<IonChamberEvt> ic_event;

	ClassDef( MiniballEvts, 3 )
	
};

#endif

