#include "DataPackets.hh"

ClassImp(FebexData)
ClassImp(InfoData)
ClassImp(PatternUnitData)
ClassImp(ScalerUnitData)
ClassImp(DgfScalerData)
ClassImp(AdcData)
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

void MiniballDataPackets::SetData( std::shared_ptr<DgfData> data ){
	
	// Reset the vector to size = 0
	// We only want to have one element per Tree entry
	ClearData();
	
	// Make a copy of the input data and push it back
	DgfData fill_data;
	
	fill_data.SetRunTime( data->GetRunTime() );
	fill_data.SetFastTriggerTime( data->GetFastTriggerTime() );
	fill_data.SetLongFastTriggerTime( data->GetLongFastTriggerTime() );
	fill_data.SetRunTime( data->GetRunTime() );
	fill_data.SetEventTime( data->GetEventTime() );
	fill_data.SetEventID( data->GetEventID() );
	fill_data.SetQshort( data->GetQshort() );
	fill_data.SetModule( data->GetModule() );
	fill_data.SetChannel( data->GetChannel() );
	fill_data.SetEnergy( data->GetEnergy() );
	fill_data.SetTrace( data->GetTrace() );
	fill_data.SetThreshold( data->IsOverThreshold() );
	fill_data.SetHitPattern( data->GetHitPattern() );
	fill_data.SetUserValues( data->GetUserValues() );

	dgf_packets.push_back( fill_data );

}

void MiniballDataPackets::SetData( std::shared_ptr<AdcData> data ){
	
	// Reset the vector to size = 0
	// We only want to have one element per Tree entry
	ClearData();
	
	// Make a copy of the input data and push it back
	AdcData fill_data;
	
	fill_data.SetTime( data->GetTime() );
	fill_data.SetEventID( data->GetEventID() );
	fill_data.SetQshort( data->GetQshort() );
	fill_data.SetModule( data->GetModule() );
	fill_data.SetChannel( data->GetChannel() );
	fill_data.SetEnergy( data->GetEnergy() );
	fill_data.SetThreshold( data->IsOverThreshold() );
	fill_data.SetClipped( data->IsClipped() );

	adc_packets.push_back( fill_data );

}

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
	fill_data.SetPileup( data->IsPileup() );
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
	adc_packets.clear();
	febex_packets.clear();
	info_packets.clear();
	
	std::vector<DgfData>().swap(dgf_packets);
	std::vector<AdcData>().swap(adc_packets);
	std::vector<FebexData>().swap(febex_packets);
	std::vector<InfoData>().swap(info_packets);
	
}

unsigned long long int MiniballDataPackets::GetEventID(){
		
	if( IsDgf() )			return GetDgfData()->GetEventID();
	if( IsAdc() )			return GetAdcData()->GetEventID();
	if( IsFebex() )			return GetFebexData()->GetEventID();
	if( IsInfo() )			return GetInfoData()->GetEventID();

	return 0;
	
}

long long int MiniballDataPackets::GetTime(){
	
	if( IsDgf() )			return GetDgfData()->GetTime();
	if( IsAdc() )			return GetAdcData()->GetTime();
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
	
	if( IsDgf() )		return GetDgfData()->GetModule();
	if( IsAdc() )		return GetAdcData()->GetModule();
	if( IsFebex() )		return GetFebexData()->GetBoard();
	if( IsInfo() )		return GetInfoData()->GetBoard();
	
	return 0;
	
}

unsigned char MiniballDataPackets::GetModule(){
	
	if( IsDgf() )		return GetDgfData()->GetModule();
	if( IsAdc() )		return GetAdcData()->GetModule();
	if( IsFebex() )		return GetFebexData()->GetBoard();
	if( IsInfo() )		return GetInfoData()->GetBoard();
	
	return 0;
	
}

unsigned char MiniballDataPackets::GetChannel(){
	
	if( IsDgf() )		return GetDgfData()->GetChannel();
	if( IsAdc() )		return GetAdcData()->GetChannel();
	if( IsFebex() )		return GetFebexData()->GetChannel();
	if( IsInfo() )		return 0;
	
	return 0;
	
}

void PatternUnitData::ClearData() {
	
	id = 255;
	val = 0;
	
}

void AdcData::ClearData(){
	
	energy = -99;
	eventid = 0;
	Qshort = 0;
	time = 0;
	mod = 255;
	ch = 255;
	
}

void DgfData::ClearData(){
	
	energy = -99;
	eventid = 0;
	Qshort = 0;
	EventTime = 0;
	FastTriggerTime = 0;
	LongFastTriggerTime = 0;
	mod = 255;
	ch = 255;
	UserValues.clear();
	std::vector<unsigned short>().swap(UserValues);
	trace.clear();
	std::vector<unsigned short>().swap(trace);

}

void DgfScalerData::ClearData(){
	
	mod = 0xffff;
	clu = 0xffff;
	
	fRealTime = 0;
	fRunTime = 0;
	fGSLTTime = 0;
	fNumberOfEvents = 0;
	fLiveTime.clear();
	fFastPeak.clear();
	
};


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

void MBSInfoPackets::ClearData(){

	time = 0;
	eventid = 0;
	patterns.clear();
	scalers.clear();
	dgfscalers.clear();
	std::vector<PatternUnitData>().swap(patterns);
	std::vector<ScalerUnitData>().swap(scalers);
	std::vector<DgfScalerData>().swap(dgfscalers);

}

