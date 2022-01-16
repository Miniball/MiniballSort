#include "MiniballEvts.hh"

ClassImp(GammaRayEvt)
ClassImp(GammaRayAddbackEvt)
ClassImp(MiniballEvts)


// --------------- //
// Miniball events //
// --------------- //
void MiniballEvts::ClearEvt() {
	
	gamma_event.clear();
	gamma_ab_event.clear();
	
	std::vector<GammaRayEvt>().swap(gamma_event);
	std::vector<GammaRayAddbackEvt>().swap(gamma_ab_event);
	
	ebis = -999;
	t1 = -999;

	return;

}

void MiniballEvts::AddEvt( GammaRayEvt *event ) {
	
	// Make a copy of the event and push it back
	GammaRayEvt fill_evt;
	fill_evt.SetEnergy( event->GetEnergy() );
	fill_evt.SetTime( event->GetTime() );
	fill_evt.SetClu( event->GetClu() );
	fill_evt.SetCry( event->GetCry() );
	fill_evt.SetSeg( event->GetSeg() );

	gamma_event.push_back( fill_evt );
	
}

void MiniballEvts::AddEvt( GammaRayAddbackEvt *event ) {
	
	// Make a copy of the event and push it back
	GammaRayAddbackEvt fill_evt;
	fill_evt.SetEnergy( event->GetEnergy() );
	fill_evt.SetTime( event->GetTime() );
	fill_evt.SetClu( event->GetClu() );
	fill_evt.SetCry( event->GetCry() );
	fill_evt.SetSeg( event->GetSeg() );

	gamma_ab_event.push_back( fill_evt );
	
}
