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
	fill_evt.SetTheta( event->GetTheta() );
	fill_evt.SetPhi( event->GetPhi() );

	gamma_event.push_back( fill_evt );
	
}

// --------------- //
// GammaRay events //
// --------------- //

void GammaRayEvt::CopyEvent( GammaRayEvt *in ){
	
	energy 	= in->GetEnergy();
	time	= in->GetTime();
	theta	= in->GetTheta();
	phi		= in->GetPhi();

	return;
	
}

TVector3 GammaRayEvt::GetUnitVector(){
	
	// Make a unit vector and rotate to correct angle
	TVector3 pos( 1, 1, 1 );
	pos.SetTheta( GetTheta() );
	pos.SetPhi( GetPhi() );

	return pos;
	
}
