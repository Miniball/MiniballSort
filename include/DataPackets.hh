#ifndef __DATAPACKETS_HH
#define __DATAPACKETS_HH

#include <memory>

#include "TObject.h"
#include "TVector.h"
#include "TGraph.h"

class PatternUnitData : public TObject {

public:
	
	PatternUnitData( unsigned char _mod, unsigned char _id, unsigned int _val ){
		mod = _mod;
		id = _id;
		val = _val;
	};
	PatternUnitData(){
		mod = 255;
		id = 255;
		val = 0;
	};
	~PatternUnitData() {};
	
	void ClearData();
	
	// Setters
	inline void SetPattern( unsigned char _mod, unsigned char _id, unsigned int _val ){
		mod = _mod;
		id = _id;
		val = _val;
	};
	
	// Getters
	inline unsigned char	GetModule(){ return mod; };
	inline unsigned char	GetID(){ return id; };
	inline unsigned int		GetValue(){ return val; };

protected:

	unsigned char				mod; 		///< module number of the pattern being read
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
	inline void SetScaler( unsigned char _id, unsigned int _val ){
		id = _id;
		val = _val;
	};
	
	// Getters
	inline unsigned char	GetID(){ return id; };
	inline unsigned int		GetValue(){ return val; };

protected:

	unsigned char		id; 		///< ID of the channel/scaler being read
	unsigned int		val;		///< value of the pattern

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
	inline void SetModule( unsigned short id ) { mod = id; };
	inline void SetClusterID( unsigned short id ) { clu = id; };
	inline void SetRealTime( long long time ){ fRealTime = time; };
	inline void SetRunTime( long long time ) { fRunTime = time; };
	inline void SetGSLTTime( long long time ) { fGSLTTime = time; };
	inline void SetNumberOfEvents( unsigned short n ) { fNumberOfEvents = n; };
	inline void SetLiveTime( unsigned short i, long long time ) {
		if( i < fNumberOfDgfChannels ) fLiveTime[i] = time;
	};
	inline void SetFastPeak( unsigned short i, unsigned int fp ) {
		if( i < fNumberOfDgfChannels ) fFastPeak[i] = fp;
	};
	
	// Getters
	inline unsigned short	GetModule(){ return mod; };
	inline unsigned short	GetCluster(){ return clu; };
	inline long long		GetRealTime(){ return fRealTime; };
	inline long long		GetRunTime() { return fRunTime; };
	inline long long		GetGSLTTime() { return fGSLTTime; };
	inline unsigned short	GetNumberOfEvents() { return fNumberOfEvents; };
	inline long long		GetLiveTime( unsigned short i ) {
		if( i < fNumberOfDgfChannels ) return fLiveTime[i];
		else return -1;
	};
	inline int	SetFastPeak( unsigned short i ) {
		if( i < fNumberOfDgfChannels ) return fFastPeak[i];
		else return -1;
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

class AdcData : public TObject {

public:
	
	AdcData(){};
	~AdcData() {};
	
	void ClearData();

	// Setters
	inline void	SetEventID( unsigned long long int id ) { eventid = id; };
	inline void SetTime( long long t ){ time = t; };
	inline void SetQshort( unsigned short q ){ Qshort = q; };
	inline void SetEnergy( float e ){ energy = e; };
	inline void SetModule( unsigned char m ){ mod = m; };
	inline void SetChannel( unsigned char c ){ ch = c; };
	inline void SetThreshold( bool t ){ thres = t; };
	inline void SetClipped( bool c ){ clipped = c; };

	// Getters
	inline long long				GetTime(){ return time; };
	inline unsigned char			GetModule(){ return mod; };
	inline unsigned char			GetChannel(){ return ch; };
	inline unsigned long long int	GetEventID() { return eventid; };
	inline unsigned int				GetQshort() { return Qshort; };
	inline float					GetEnergy() { return energy; };
	inline bool						IsOverThreshold() { return thres; };
	inline bool						IsClipped() { return clipped; };

protected:

	unsigned long long int		eventid;
	long long 					time;
	float						energy;
	unsigned short				Qshort;
	unsigned char				mod; 		///< module number of ADC
	unsigned char				ch; 		///< channel number of ADC
	bool						thres;		///< is the energy over threshold?
	bool						clipped;	///< is the ADC value over range?

	ClassDef( AdcData, 1 )

};

class DgfData : public TObject {
	
public:
	
	DgfData(){};
	~DgfData() {};
	
	// Reset
	void ClearData();
	
	// Setters
	inline void	SetEventID( unsigned long long int id ) { eventid = id; };
	inline void SetHitPattern( unsigned short hp ) { HitPattern = hp; };
	inline void SetEventTime( long long time ){ EventTime = time; };
	inline void SetQshort( unsigned short q ){ Qshort = q; };
	inline void SetEnergy( float e ){ energy = e; };
	inline void SetRunTime( unsigned short time ){ RunTime = time; };
	inline void SetFastTriggerTime( unsigned short time ){ FastTriggerTime = time; };
	inline void SetLongFastTriggerTime( long long time ){ LongFastTriggerTime = time; };
	inline void SetUserValues( std::vector<unsigned short> q ) { UserValues = q; };
	inline void	SetTrace( std::vector<unsigned short> t ) { trace = t; };
	inline void AddSample( unsigned short s ) { trace.push_back(s); };
	inline void SetModule( unsigned char m ){ mod = m; };
	inline void SetChannel( unsigned char c ){ ch = c; };
	inline void SetThreshold( bool t ){ thres = t; };

	// Getters
	inline long long					GetTime(){ return GetLongFastTriggerTime(); };
	inline long long					GetEventTime(){ return EventTime; };
	inline long long					GetLongFastTriggerTime(){ return LongFastTriggerTime; };
	inline unsigned short				GetFastTriggerTime(){ return FastTriggerTime; };
	inline unsigned short				GetRunTime(){ return RunTime; };
	inline unsigned char				GetModule(){ return mod; };
	inline unsigned char				GetChannel(){ return ch; };
	inline unsigned long long int		GetEventID() { return eventid; };
	inline unsigned int					GetQshort() { return Qshort; };
	inline float						GetEnergy() { return energy; };
	inline unsigned short				GetHitPattern(){ return HitPattern; };
	inline std::vector<unsigned short>	GetUserValues(){ return UserValues; };
	inline bool							IsOverThreshold() { return thres; };
	inline unsigned short				GetTraceLength() { return trace.size(); };
	inline std::vector<unsigned short>	GetTrace() { return trace; };
	inline TGraph* GetTraceGraph() {
		std::vector<int> x, y;
		std::string title = "Trace for DGF Mod " + std::to_string( GetModule() );
		title += ", Channel " + std::to_string( GetChannel() );
		title += ";time [ns];signal";
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


	
protected:

	//long long int 				time
	unsigned long long int		eventid;
	long long 					EventTime;
	unsigned short				RunTime;
	float						energy;
	unsigned short				Qshort;
	unsigned short 				HitPattern;
	unsigned short				FastTriggerTime;
	long long					LongFastTriggerTime;
	unsigned char				mod;	///< module number of DGF
	unsigned char				ch; 	///< channel number of DGF
	std::vector<unsigned short>	UserValues;
	std::vector<unsigned short>	trace;
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
	inline bool							IsPileup() { return pileup; };
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
	inline void SetPileup( bool p ){ pileup = p; };
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

	MiniballDataPackets( std::unique_ptr<MiniballDataPackets> &in ) {
		if( in->IsDgf() )	SetData( in->GetDgfData() );
		if( in->IsAdc() )	SetData( in->GetAdcData() );
		if( in->IsFebex() )	SetData( in->GetFebexData() );
		if( in->IsInfo() )	SetData( in->GetInfoData() );
	};

	inline bool	IsDgf() const { return dgf_packets.size(); };
	inline bool	IsAdc() const { return adc_packets.size(); };
	inline bool	IsFebex() const { return febex_packets.size(); };
	inline bool	IsInfo() const { return info_packets.size(); };

	void SetData( MiniballDataPackets in ){
		if( in.IsDgf() )	SetData( in.GetDgfData() );
		if( in.IsAdc() )	SetData( in.GetAdcData() );
		if( in.IsFebex() )	SetData( in.GetFebexData() );
		if( in.IsInfo() )	SetData( in.GetInfoData() );
	};
	void SetData( std::shared_ptr<DgfData> data );
	void SetData( std::shared_ptr<AdcData> data );
	void SetData( std::shared_ptr<FebexData> data );
	void SetData( std::shared_ptr<InfoData> data );

	// These methods are not very safe for access
	inline std::shared_ptr<DgfData> GetDgfData() const {
		return std::make_shared<DgfData>( dgf_packets.at(0) );
	};
	inline std::shared_ptr<AdcData> GetAdcData() const {
		return std::make_shared<AdcData>( adc_packets.at(0) );
	};
	inline std::shared_ptr<FebexData> GetFebexData() const {
		return std::make_shared<FebexData>( febex_packets.at(0) );
	};
	inline std::shared_ptr<InfoData> GetInfoData() const {
		return std::make_shared<InfoData>( info_packets.at(0) );
	};

	// Complicated way to get the time...
	unsigned long long int GetEventID() const;
	long long int GetTime() const;
	UInt_t GetTimeMSB() const;
	UInt_t GetTimeLSB() const;
	unsigned char GetSfp() const;
	unsigned char GetBoard() const;
	unsigned char GetModule() const;
	unsigned char GetChannel() const;

	// Sorting function to do time ordering
	bool operator < ( const MiniballDataPackets &data ) const {
		return( this->GetTime() < data.GetTime() );
	};

	void ClearData();

protected:
	
	std::vector<DgfData>		dgf_packets;
	std::vector<AdcData>		adc_packets;
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
		std::vector<DgfScalerData>().swap(dgfscalers);
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
	inline void AddPattern( unsigned char _mod, unsigned char _id, unsigned int _val ){
		PatternUnitData tmp(_mod,_id,_val);
		AddPattern(tmp);
	};
	inline unsigned int GetPatternValue( unsigned char mod, unsigned char id ){
		unsigned int tmp = 0;
		for( unsigned int i = 0; i < patterns.size(); i++ ){
			if( patterns[i].GetModule() == mod && patterns[i].GetID() == id ) {
				tmp = patterns[i].GetValue();
				break;
			}
		}
		return tmp;
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
	inline void AddScaler( ScalerUnitData s ){ scalers.push_back(s); };
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

	ClassDef( MBSInfoPackets, 4 )
	
};


#endif
