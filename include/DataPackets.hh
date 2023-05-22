#ifndef __DATAPACKETS_HH
#define __DATAPACKETS_HH

#include <memory>

#include "TObject.h"
#include "TVector.h"
#include "TGraph.h"

class FebexData : public TObject {
	
public:

	FebexData() {};
	FebexData( long long int t, unsigned long long int id,
			  unsigned int qi, unsigned short qs,
			  std::vector<unsigned short> tr,
			  unsigned char s, unsigned char b, unsigned char c,
			  bool th, bool p );
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
	inline std::vector<unsigned short>	GetTrace() { return trace; };
	inline TGraph* GetTraceGraph() {
		std::vector<int> x, y;
		std::string title = "Trace for SFP " + std::to_string( GetSfp() );
		title += ", Board " + std::to_string( GetBoard() ) + ", Channel ";
		title += std::to_string( GetChannel() ) + ";time [ns];signal";
		for( unsigned short i = 0; i < GetTraceLength(); ++i ){
			x.push_back( i * 20 );
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

	
	ClassDef( FebexData, 6 )
	
};

class InfoData : public TObject {
	
public:

	InfoData() {};
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

	inline bool	IsFebex() { return febex_packets.size(); };
	inline bool	IsInfo() { return info_packets.size(); };
	
	void SetData( std::shared_ptr<FebexData> data );
	void SetData( std::shared_ptr<InfoData> data );

	// These methods are not very safe for access
	inline std::shared_ptr<FebexData> GetFebexData() { return std::make_shared<FebexData>( febex_packets.at(0) ); };
	inline std::shared_ptr<InfoData> GetInfoData() { return std::make_shared<InfoData>( info_packets.at(0) ); };
	
	// Complicated way to get the time...
	unsigned long long int GetEventID();
	long long int GetTime();
	UInt_t GetTimeMSB();
	UInt_t GetTimeLSB();

	void ClearData();

protected:
	
	std::vector<FebexData>	febex_packets;
	std::vector<InfoData>	info_packets;

	ClassDef( MiniballDataPackets, 2 )

};


class MBSInfoPackets : public TObject {
	
public:
	
	MBSInfoPackets() {};
	~MBSInfoPackets() {};
	
	inline long long int			GetTime(){ return time; };
	inline unsigned long long int	GetEventID(){ return eventid; };

	inline void SetTime( long long int t ){ time = t; };
	inline void SetEventID( unsigned long long int id ){ eventid = id; };

	protected:
	
	long long int			time;		///< timestamp of info event
	unsigned long long int	eventid;	///< timestamp of info event

	ClassDef( MBSInfoPackets, 3 )
	
};


#endif
