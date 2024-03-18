#ifndef __DATAPACKETS_HH
#define __DATAPACKETS_HH

#include <memory>

#include "TObject.h"
#include "TVector.h"
#include "TGraph.h"

class PatternUnitData : public TObject {

public:
	
	PatternUnitData( unsigned char _id, unsigned int _val ){
		id = _id;
		val = _val;
	};
	PatternUnitData(){
		id = 255;
		val = 0;
	};
	~PatternUnitData() {};
	
	void ClearData();
	
	// Setters
	void SetPattern( unsigned char _id, unsigned int _val ){
		id = _id;
		val = _val;
	};
	
	// Getters
	unsigned char	GetID(){ return id; };
	unsigned int	GetValue(){ return val; };

protected:

	unsigned char				id; 		///< ID of the channel/pattern being read
	unsigned int				val;		///< value of the pattern

	ClassDef( PatternUnitData, 1 )

};

class ScalerUnitData : public TObject {

public:
	
	ScalerUnitData( unsigned char _id, unsigned int _val ){
		id = _id;
		val = _val;
	};
	ScalerUnitData(){
		id = 255;
		val = 0;
	};
	~ScalerUnitData() {};
	
	void ClearData();
	
	// Setters
	void SetScaler( unsigned char _id, unsigned int _val ){
		id = _id;
		val = _val;
	};
	
	// Getters
	unsigned char	GetID(){ return id; };
	unsigned int	GetValue(){ return val; };

protected:

	unsigned char				id; 		///< ID of the channel/scaler being read
	unsigned int				val;		///< value of the pattern

	ClassDef( ScalerUnitData, 1 )

};

class DgfScalerData : public TObject {

public:
	
	DgfScalerData(){};
	DgfScalerData( unsigned short n ) : fNumberOfDgfChannels(n) {
		fLiveTime.resize( n, 0 );
		fFastPeak.resize( n, 0 );
	};
	~DgfScalerData() {};
	
	void ClearData();
	
	// Setters
	void SetModule( unsigned short id ) { mod = id; };
	void SetClusterID( unsigned short id ) { clu = id; };
	void SetRealTime( long long time ){ fRealTime = time; };
	void SetRunTime( long long time ) { fRunTime = time; };
	void SetGSLTTime( long long time ) { fGSLTTime = time; };
	void SetNumberOfEvents( unsigned short n ) { fNumberOfEvents = n; };
	void SetLiveTime( unsigned short i, long long time ) {
		if( i < fNumberOfDgfChannels ) fLiveTime[i] = time;
	};
	void SetFastPeak( unsigned short i, unsigned int fp ) {
		if( i < fNumberOfDgfChannels ) fFastPeak[i] = fp;
	};
	
	// Getters
	unsigned short	GetModule(){ return mod; };
	unsigned short	GetCluster(){ return clu; };
	long long		GetRealTime(){ return fRealTime; };
	long long		GetRunTime() { return fRunTime; };
	long long		GetGSLTTime() { return fGSLTTime; };
	unsigned short	GetNumberOfEvents() { return fNumberOfEvents; };
	long long		GetLiveTime( unsigned short i ) {
		if( i < fNumberOfDgfChannels ) return fLiveTime[i];
		else return -1;
	};
	unsigned int	SetFastPeak( unsigned short i ) {
		if( i < fNumberOfDgfChannels ) return fFastPeak[i];
		else return 0;
	};

protected:

	unsigned short	mod; 		///< DGF module being read
	unsigned short	clu;		///< cluster ID
	
	long long fRealTime;
	long long fRunTime;
	long long fGSLTTime;
	unsigned short fNumberOfEvents;
	std::vector<long long> fLiveTime;
	std::vector<int> fFastPeak;
	
	unsigned short fNumberOfDgfChannels;

	ClassDef( DgfScalerData, 1 )

};

class MiniballAdcData : public TObject {

public:
	
	MiniballAdcData(){};
	~MiniballAdcData() {};
	
	void ClearData();

	// Setters
	inline void SetTime( long long t ){ time = t; };
	inline void SetQint( unsigned short q ){ Qint = q; };
	inline void SetEnergy( float e ){ energy = e; };
	inline void SetModule( unsigned char m ){ mod = m; };
	inline void SetChannel( unsigned char c ){ ch = c; };
	inline void SetThreshold( bool t ){ thres = t; };

	// Getters
	long long							GetTime(){ return time; };
	unsigned char						GetModule(){ return mod; };
	unsigned char						GetChannel(){ return ch; };
	inline unsigned long long int		GetEventID() { return eventid; };
	inline unsigned int					GetQint() { return Qint; };
	inline float						GetEnergy() { return energy; };
	inline bool							IsOverThreshold() { return thres; };

protected:

	unsigned long long int		eventid;
	long long 					time;
	float						energy;
	unsigned short				Qint;
	unsigned char				mod; 		///< module number of ADC
	unsigned char				ch; 		///< channel number of ADC
	bool						thres;		///< is the energy over threshold?

	ClassDef( MiniballAdcData, 1 )

};

class MesytecAdcData : public MiniballAdcData {

public:
	
	MesytecAdcData(){};
	~MesytecAdcData() {};
	
protected:

	ClassDef( MesytecAdcData, 1 )

};

class CaenAdcData : public MiniballAdcData {
	
public:
	
	CaenAdcData(){};
	~CaenAdcData() {};
	
protected:


	ClassDef( CaenAdcData, 1 )

};

class DgfData : public TObject {
	
public:
	
	DgfData(){};
	~DgfData() {};
	
	// Reset
	void ClearData();
	
	// Setters
	inline void SetHitPattern( unsigned short hp ) {
		HitPattern = hp;
	};
	inline void SetEventTime( unsigned short RunTime, unsigned short EventTimeHigh, unsigned short EventTimeLow ){
		EventTime = EventTimeLow + 65536ll*EventTimeHigh + 65536ll*65536ll*RunTime;
	};
	inline void SetFastTriggerTime( unsigned short t ){ FastTriggerTime = t; };
	inline void SetQint( unsigned short q ){ Qint = q; };
	inline void SetEnergy( float e ){ energy = e; };
	inline void SetLongFastTriggerTime( unsigned short RunTime, unsigned short EventTimeHigh, unsigned short EventTimeLow ) {
		if( FastTriggerTime > EventTimeLow) // check for overflow
			LongFastTriggerTime = FastTriggerTime + 65536ll*EventTimeHigh + 65536ll*65536ll*RunTime;
		else
			LongFastTriggerTime = FastTriggerTime + 65536ll*EventTimeHigh + 65536ll + 65536ll*65536ll*RunTime;
	};
	inline void SetUserValues( std::vector<unsigned short> q ) { UserValues = q; };
	inline void SetModule( unsigned char m ){ mod = m; };
	inline void SetChannel( unsigned char c ){ ch = c; };
	inline void SetThreshold( bool t ){ thres = t; };

	// Getters
	long long							GetTime(){ return EventTime + LongFastTriggerTime; };
	unsigned char						GetModule(){ return mod; };
	unsigned char						GetChannel(){ return ch; };
	inline unsigned long long int		GetEventID() { return eventid; };
	inline unsigned int					GetQint() { return Qint; };
	inline float						GetEnergy() { return energy; };
	inline unsigned short				GetHitPattern(){ return HitPattern; };
	inline bool							IsOverThreshold() { return thres; };


	
protected:

	//long long int 				time
	unsigned long long int		eventid;
	long long 					EventTime;
	float						energy;
	unsigned short				Qint;
	unsigned short 				HitPattern;
	unsigned short				FastTriggerTime;
	long long					LongFastTriggerTime;
	unsigned char				mod;	///< module number of DGF
	unsigned char				ch; 	///< channel number of DGF
	std::vector<unsigned short>	UserValues;
	bool						thres;		///< is the energy over threshold?


	ClassDef( DgfData, 1 )

};

class FebexData : public TObject {
	
public:

	FebexData() {
		time = 0;
		eventid = 0;
		energy = 0;
		Qint = 0;
		Qshort = 0;
		sfp = 0;
		board = 0;
		ch = 0;
	};
	FebexData( long long int t, unsigned long long int id,
			  unsigned int qi, unsigned short qs,
			  std::vector<unsigned short> tr,
			  unsigned char s, unsigned char b, unsigned char c,
			  bool th, bool p, bool cl, bool f );
	~FebexData() {};

	inline long long int				GetTime() { return time; };
	inline unsigned long long int		GetEventID() { return eventid; };
	inline unsigned short				GetTraceLength() { return trace.size(); };
	inline unsigned short				GetQshort() { return Qshort; };
	inline unsigned int					GetQint() { return Qint; };
	inline unsigned char				GetSfp() { return sfp; };
	inline unsigned char				GetBoard() { return board; };
	inline unsigned char				GetChannel() { return ch; };
	inline float						GetEnergy() { return energy; };
	inline bool							IsOverThreshold() { return thres; };
	inline bool							IsPileUp() { return pileup; };
	inline bool							IsClipped() { return clipped; };
	inline bool							HasFlag() { return flagbit; };
	inline std::vector<unsigned short>	GetTrace() { return trace; };
	inline TGraph* GetTraceGraph() {
		std::vector<int> x, y;
		std::string title = "Trace for SFP " + std::to_string( GetSfp() );
		title += ", Board " + std::to_string( GetBoard() ) + ", Channel ";
		title += std::to_string( GetChannel() ) + ";time [ns];signal";
		for( unsigned short i = 0; i < GetTraceLength(); ++i ){
			x.push_back( i * 10 );
			y.push_back( GetSample(i) );
		}
		std::unique_ptr<TGraph> g = std::make_unique<TGraph>(
                            GetTraceLength(), x.data(), y.data() );
        g.get()->SetTitle( title.data() );
		return (TGraph*)g.get()->Clone();
	};
	inline unsigned short				GetSample( unsigned int i = 0 ) {
		if( i >= trace.size() ) return 0;
		return trace.at(i);
	};
	
	inline void	SetTime( long long int t ) { time = t; };
	inline void	SetEventID( unsigned long long int id ) { eventid = id; };
	inline void	SetTrace( std::vector<unsigned short> t ) { trace = t; };
	inline void AddSample( unsigned short s ) { trace.push_back(s); };
	inline void	SetQshort( unsigned short q ) { Qshort = q; };
	inline void	SetQint( unsigned int q ) { Qint = q; };
	inline void SetSfp( unsigned char s ){ sfp = s; };
	inline void SetBoard( unsigned char b ){ board = b; };
	inline void	SetChannel( unsigned char c ) { ch = c; };
	inline void SetEnergy( float e ){ energy = e; };
	inline void SetThreshold( bool t ){ thres = t; };
	inline void SetPileUp( bool p ){ pileup = p; };
	inline void SetClipped( bool cl ){ clipped = cl; };
	inline void SetFlag( bool f ){ flagbit = f; };

	inline void ClearTrace() { trace.clear(); };
	void ClearData();

protected:
	
	long long int					time;
	unsigned long long int			eventid;
	float							energy;
	unsigned int					Qint;		///< Charge from firmware as 32-bit integer
	unsigned short					Qshort;		///< Charge from firmware as 16-bit integer
	std::vector<unsigned short>		trace;
	unsigned char					sfp;		///< SFP ID of the event
	unsigned char					board;		///< board ID of the event
	unsigned char					ch;			///< channel ID of the event
	bool							thres;		///< is the energy over threshold?
	bool							pileup;		///< pileup flag from data stream
	bool							clipped;	///< clipped pulse flag from data stream
	bool							flagbit;	///< additional flag bit from data stream

	
	ClassDef( FebexData, 7 )
	
};

class InfoData : public TObject {
	
public:

	InfoData() {
		time = 0;
		eventid = 0;
		code = 0;
		sfp = 0;
		board = 0;
	};
	InfoData( long long int t, unsigned long long int id, unsigned char s, unsigned char b, unsigned char m );
	~InfoData() {};
	
	inline long long int			GetTime(){ return time; };
	inline unsigned long long int	GetEventID(){ return eventid; };
	inline unsigned char 			GetCode(){ return code; };
	inline unsigned char			GetSfp(){ return sfp; };
	inline unsigned char			GetBoard(){ return board; };

	inline void SetTime( long long int t ){ time = t; };
	inline void SetEventID( unsigned long long int id ){ eventid = id; };
	inline void SetCode( unsigned char c ){ code = c; };
	inline void SetSfp( unsigned char s ){ sfp = s; };
	inline void SetBoard( unsigned char b ){ board = b; };

	void ClearData();

protected:
	
	long long int			time;		///< timestamp of info event
	unsigned long long int	eventid;	///< timestamp of info event
	unsigned char			code;		///< code here represents which information timestamp we have
	unsigned char			sfp;		///< SFP ID of the event
	unsigned char			board;		///< board ID of the event
	/// code = 20 is external pulser event for whatever reason
	/// code = 21 is EBIS proton timestamp
	/// code = 22 is T1 timestamp

	
	ClassDef( InfoData, 12 )
	
};

class MiniballDataPackets : public TObject {
	
public:
	
	MiniballDataPackets() {};
	~MiniballDataPackets() {};

	inline bool	IsDgf()			{ return dgf_packets.size(); };
	inline bool	IsCaenAdc() 	{ return caen_packets.size(); };
	inline bool	IsMesytecAdc()	{ return madc_packets.size(); };
	inline bool	IsFebex()		{ return febex_packets.size(); };
	inline bool	IsInfo()		{ return info_packets.size(); };
	
	void SetData( std::shared_ptr<FebexData> data );
	void SetData( std::shared_ptr<InfoData> data );

	// These methods are not very safe for access
	inline std::shared_ptr<DgfData> GetDgfData() { return std::make_shared<DgfData>( dgf_packets.at(0) ); };
	inline std::shared_ptr<CaenAdcData> GetCaenAdcData() { return std::make_shared<CaenAdcData>( caen_packets.at(0) ); };
	inline std::shared_ptr<MesytecAdcData> GetMesytecAdcData() { return std::make_shared<MesytecAdcData>( madc_packets.at(0) ); };
	inline std::shared_ptr<FebexData> GetFebexData() { return std::make_shared<FebexData>( febex_packets.at(0) ); };
	inline std::shared_ptr<InfoData> GetInfoData() { return std::make_shared<InfoData>( info_packets.at(0) ); };
	
	// Complicated way to get the time...
	unsigned long long int GetEventID();
	long long int GetTime();
	UInt_t GetTimeMSB();
	UInt_t GetTimeLSB();
	unsigned char GetSfp();
	unsigned char GetBoard();
	unsigned char GetChannel();

	void ClearData();

protected:
	
	std::vector<DgfData>		dgf_packets;
	std::vector<CaenAdcData>	caen_packets;
	std::vector<MesytecAdcData>	madc_packets;
	std::vector<FebexData>		febex_packets;
	std::vector<InfoData>		info_packets;

	ClassDef( MiniballDataPackets, 3 )

};


class MBSInfoPackets : public TObject {
	
public:
	
	MBSInfoPackets() {
		time = 0;
		eventid = 0;
		std::vector<PatternUnitData>().swap(patterns);
		std::vector<ScalerUnitData>().swap(scalers);
	};
	~MBSInfoPackets() {};
	
	void ClearData();
	
	inline long long int			GetTime(){ return time; };
	inline unsigned long long int	GetEventID(){ return eventid; };

	inline void SetTime( long long int t ){ time = t; };
	inline void SetEventID( unsigned long long int id ){ eventid = id; };

	// Patterns
	inline void AddPattern( PatternUnitData p ){
		patterns.push_back(p);
	};
	inline void AddPattern( unsigned char _id, unsigned int _val ){
		PatternUnitData tmp(_id,_val);
		AddPattern(tmp);
	};
	inline unsigned int GetPatternValue( unsigned char id ){
		unsigned int tmp = 0;
		for( unsigned int i = 0; i < patterns.size(); i++ ){
			if( patterns[i].GetID() == id ) {
				tmp = patterns[i].GetValue();
				break;
			}
		}
		return tmp;
	};
	inline PatternUnitData GetPattern( unsigned char id ){
		PatternUnitData tmp;
		for( unsigned int i = 0; i < patterns.size(); i++ ){
			if( patterns[i].GetID() == id ) {
				tmp = patterns[i];
				break;
			}
		}
		return tmp;
	};

	// Scalers
	inline void AddScaler( ScalerUnitData s ){
		scalers.push_back(s);
	};
	inline void AddScaler( unsigned char _id, unsigned int _val ){
		ScalerUnitData tmp(_id,_val);
		AddScaler(tmp);
	};
	inline unsigned int GetScalerValue( unsigned char id ){
		unsigned int tmp = 0;
		for( unsigned int i = 0; i < scalers.size(); i++ ){
			if( scalers[i].GetID() == id ) {
				tmp = scalers[i].GetValue();
				break;
			}
		}
		return tmp;
	};
	inline ScalerUnitData GetScaler( unsigned char id ){
		ScalerUnitData tmp;
		for( unsigned int i = 0; i < scalers.size(); i++ ){
			if( scalers[i].GetID() == id ) {
				tmp = scalers[i];
				break;
			}
		}
		return tmp;
	};

	// DGF Scalers
	inline void AddDgfScaler( DgfScalerData s ){
		dgfscalers.push_back(s);
	};
	DgfScalerData GetDgfScaler( unsigned char mod ){
		DgfScalerData tmp;
		for( unsigned int i = 0; i < dgfscalers.size(); i++ ){
			if( dgfscalers[i].GetModule() == mod ) {
				tmp = dgfscalers[i];
				break;
			}
		}
		return tmp;
	};


	protected:
	
	long long int					time;		///< timestamp of info event
	unsigned long long int			eventid;	///< timestamp of info event
	std::vector<PatternUnitData> 	patterns;	///< pattern unit data
	std::vector<ScalerUnitData> 	scalers;	///< scaler unit data
	std::vector<DgfScalerData> 		dgfscalers;	///<DGF  scaler data

	ClassDef( MBSInfoPackets, 3 )
	
};


#endif
