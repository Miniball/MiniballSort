#include "DataPackets.hh"

ClassImp(FebexData)
ClassImp(InfoData)
ClassImp(MesytecAdcData)
ClassImp(CaenAdcData)
ClassImp(DgfData)
ClassImp(MiniballDataPackets)
ClassImp(MBSInfoPackets)

FebexData::FebexData( long long int t, unsigned long long int id,
					unsigned int qi, unsigned short qs,
				    std::vector<unsigned short> tr,
					unsigned char s, unsigned char b, unsigned char c,
				    bool th, bool p, bool cl, bool f ) :
					time(t), eventid(id), Qint(qi), Qshort(qs), trace(tr), sfp(s),
					board(b), ch(c), thres(th), pileup(p), clipped(cl), flagbit(f) {}

InfoData::InfoData( long long int t, unsigned long long int id, unsigned char c,
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
	fill_data.SetQshort( data->GetQshort() );
	fill_data.SetSfp( data->GetSfp() );
	fill_data.SetBoard( data->GetBoard() );
	fill_data.SetChannel( data->GetChannel() );
	fill_data.SetEnergy( data->GetEnergy() );
	fill_data.SetThreshold( data->IsOverThreshold() );
	fill_data.SetPileUp( data->IsPileUp() );
	fill_data.SetClipped( data->IsClipped() );
	fill_data.SetFlag( data->HasFlag() );

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
	
	dgf_packets.clear();
	caen_packets.clear();
	madc_packets.clear();
	febex_packets.clear();
	info_packets.clear();
	
	return;
	
}

unsigned long long int MiniballDataPackets::GetEventID(){
		
	//if( IsDgf() )			return GetDgfData()->GetEventID();
	//if( IsCaenAdc() )		return GetCaenAdcData()->GetEventID();
	//if( IsMesytecAdc() )	return GetMesytecAdcData()->GetEventID();
	if( IsFebex() )			return GetFebexData()->GetEventID();
	if( IsInfo() )			return GetInfoData()->GetEventID();

	return 0;
	
}

long long int MiniballDataPackets::GetTime(){
	
	//if( IsDgf() )			return GetDgfData()->GetTime();
	//if( IsCaenAdc() )		return GetCaenAdcData()->GetTime();
	//if( IsMesytecAdc() )	return GetMesytecAdcData()->GetTime();
	if( IsFebex() )			return GetFebexData()->GetTime();
	if( IsInfo() )			return GetInfoData()->GetTime();
	
	return 0;
	
}

UInt_t MiniballDataPackets::GetTimeMSB(){
	
	return ( (this->GetTime() >> 32) & 0xFFFFFFFF );
	
}

UInt_t MiniballDataPackets::GetTimeLSB(){
	
	return (UInt_t)this->GetTime();
	
}

unsigned char MiniballDataPackets::GetSfp(){
	
	if( IsFebex() )		return GetFebexData()->GetSfp();
	if( IsInfo() )		return GetInfoData()->GetSfp();
	
	return 0;
	
}

unsigned char MiniballDataPackets::GetBoard(){
	
	if( IsFebex() )		return GetFebexData()->GetBoard();
	if( IsInfo() )		return GetInfoData()->GetBoard();
	
	return 0;
	
}

unsigned char MiniballDataPackets::GetChannel(){
	
	//if( IsDgf() )			return GetDgfData()->GetChannel();
	//if( IsCaenAdc() )		return GetCaenAdcData()->GetChannel();
	//if( IsMesytecAdc() )	return GetMesytecAdcData()->GetChannel();
	if( IsFebex() )			return GetFebexData()->GetChannel();
	if( IsInfo() )			return 0;
	
	return 0;
	
}


void FebexData::ClearData(){
	
	time = 0;
	eventid = 0;
	trace.clear();
	std::vector<unsigned short>().swap(trace);
	Qint = 0;
	Qshort = 0;
	sfp = 255;
	board = 255;
	ch = 255;
	energy = -999.;
	thres = true;
	pileup = false;
	clipped = false;
	flagbit = false;

	return;
	
}

void InfoData::ClearData(){
	
	time = 0;
	code = 0;
	sfp = 255;
	board = 255;

	return;
	
}
