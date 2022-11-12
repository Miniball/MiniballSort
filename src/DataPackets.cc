#include "DataPackets.hh"

ClassImp(FebexData)
ClassImp(InfoData)
ClassImp(MiniballDataPackets)
ClassImp(MBSInfoPackets)

FebexData::FebexData( long long t, unsigned long long id,
					unsigned int qi, Float16_t qh, unsigned short qs,
				    std::vector<unsigned short> tr,
					unsigned char s, unsigned char b, unsigned char c,
				    bool th, bool v, bool f, bool p ) :
					time(t), eventid(id), Qint(qi), Qhalf(qh), Qshort(qs), trace(tr),
					sfp(s), board(b), ch(c), thres(th), veto(v), fail(f), pileup(p) {}

InfoData::InfoData( long long t, unsigned long long id, unsigned char c,
				    unsigned char s, unsigned char b ) :
					time(t), eventid(id), code(c), sfp(s), board(b) {}

void MiniballDataPackets::SetData( std::shared_ptr<FebexData> data ){
	
	// Reset the vector to size = 0
	// We only want to have one element per Tree entry
	ClearData();
	
	// Make a copy of the input data and push it back
	FebexData fill_data;
	
	fill_data.SetTime( data->GetTime() );
	fill_data.SetEventID( data->GetEventID() );
	fill_data.SetTrace( data->GetTrace() );
	fill_data.SetQint( data->GetQint() );
	fill_data.SetQhalf( data->GetQhalf() );
	fill_data.SetQshort( data->GetQshort() );
	fill_data.SetSfp( data->GetSfp() );
	fill_data.SetBoard( data->GetBoard() );
	fill_data.SetChannel( data->GetChannel() );
	fill_data.SetEnergy( data->GetEnergy() );
	fill_data.SetThreshold( data->IsOverThreshold() );
	fill_data.SetVeto( data->IsVeto() );
	fill_data.SetFail( data->IsFail() );
	fill_data.SetPileUp( data->IsPileUp() );

	febex_packets.push_back( fill_data );

}

void MiniballDataPackets::SetData( std::shared_ptr<InfoData> data ){
	
	// Reset the vector to size = 0
	// We only want to have one element per Tree entry
	ClearData();
	
	// Make a copy of the input data and push it back
	InfoData fill_data;
	fill_data.SetTime( data->GetTime() );
	fill_data.SetEventID( data->GetEventID() );
	fill_data.SetCode( data->GetCode() );
	fill_data.SetSfp( data->GetSfp() );
	fill_data.SetBoard( data->GetBoard() );

	info_packets.push_back( fill_data );
	
}


void MiniballDataPackets::ClearData(){
	
	febex_packets.clear();
	info_packets.clear();
	
	return;
	
}

unsigned long long MiniballDataPackets::GetEventID(){
		
	if( IsFebex() ) return GetFebexData()->GetEventID();
	if( IsInfo() ) return GetInfoData()->GetEventID();

	return 0;
	
}

long long MiniballDataPackets::GetTime(){
		
	if( IsFebex() ) return GetFebexData()->GetTime();
	if( IsInfo() ) return GetInfoData()->GetTime();

	return 0;
	
}

UInt_t MiniballDataPackets::GetTimeMSB(){
	
	return ( (this->GetTime() >> 32) & 0x0000FFFF );
	
}

UInt_t MiniballDataPackets::GetTimeLSB(){
	
	return (UInt_t)this->GetTime();
	
}

void FebexData::ClearData(){
	
	time = 0;
	eventid = 0;
	trace.clear();
	std::vector<unsigned short>().swap(trace);
	Qint = 0;
	Qhalf = 0.;
	Qshort = 0;
	sfp = 255;
	board = 255;
	ch = 255;
	energy = -999.;
	veto = false;
	fail = false;
	thres = true;
	pileup = false;

	return;
	
}

void InfoData::ClearData(){
	
	time = 0;
	code = 0;
	sfp = 255;
	board = 255;

	return;
	
}
