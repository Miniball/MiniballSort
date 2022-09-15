#include "MiniballEvts.hh"

ClassImp(GammaRayEvt)
ClassImp(GammaRayAddbackEvt)
ClassImp(ParticleEvt)
ClassImp(BeamDumpEvt)
ClassImp(SpedeEvt)
ClassImp(MiniballEvts)


// --------------- //
// Miniball events //
// --------------- //
void MiniballEvts::ClearEvt() {
	
	gamma_event.clear();
	gamma_ab_event.clear();
	particle_event.clear();
	bd_event.clear();
	spede_event.clear();

	std::vector<GammaRayEvt>().swap(gamma_event);
	std::vector<GammaRayAddbackEvt>().swap(gamma_ab_event);
	std::vector<ParticleEvt>().swap(particle_event);
	std::vector<BeamDumpEvt>().swap(bd_event);
	std::vector<SpedeEvt>().swap(spede_event);

	ebis = 0;
	t1 = 0;
	sc = 0;

	return;

}

void MiniballEvts::AddEvt( std::shared_ptr<GammaRayEvt> event ) {
	
	// Make a copy of the event and push it back
	GammaRayEvt fill_evt;
	fill_evt.SetEnergy( event->GetEnergy() );
	fill_evt.SetSegmentEnergy( event->GetEnergy() );
	fill_evt.SetTime( event->GetTime() );
	fill_evt.SetCluster( event->GetCluster() );
	fill_evt.SetCrystal( event->GetCrystal() );
	fill_evt.SetSegment( event->GetSegment() );

	gamma_event.push_back( fill_evt );
	
}

void MiniballEvts::AddEvt( std::shared_ptr<GammaRayAddbackEvt> event ) {
	
	// Make a copy of the event and push it back
	GammaRayAddbackEvt fill_evt;
	fill_evt.SetEnergy( event->GetEnergy() );
	fill_evt.SetSegmentEnergy( event->GetEnergy() );
	fill_evt.SetTime( event->GetTime() );
	fill_evt.SetCluster( event->GetCluster() );
	fill_evt.SetCrystal( event->GetCrystal() );
	fill_evt.SetSegment( event->GetSegment() );

	gamma_ab_event.push_back( fill_evt );
	
}

void MiniballEvts::AddEvt( std::shared_ptr<ParticleEvt> event ) {
	
	// Make a copy of the event and push it back
	ParticleEvt fill_evt;
	fill_evt.SetEnergyP( event->GetEnergyP() );
	fill_evt.SetEnergyN( event->GetEnergyN() );
	fill_evt.SetTimeP( event->GetTimeP() );
	fill_evt.SetTimeN( event->GetTimeN() );
	fill_evt.SetDetector( event->GetDetector() );
	fill_evt.SetSector( event->GetSector() );
	fill_evt.SetStripP( event->GetStripP() );
	fill_evt.SetStripN( event->GetStripN() );

	particle_event.push_back( fill_evt );
	
}

void MiniballEvts::AddEvt( std::shared_ptr<BeamDumpEvt> event ) {
	
	// Make a copy of the event and push it back
	BeamDumpEvt fill_evt;
	fill_evt.SetEnergy( event->GetEnergy() );
	fill_evt.SetTime( event->GetTime() );
	fill_evt.SetDetector( event->GetDetector() );

	bd_event.push_back( fill_evt );
	
}

void MiniballEvts::AddEvt( std::shared_ptr<SpedeEvt> event ) {
	
	// Make a copy of the event and push it back
	SpedeEvt fill_evt;
	fill_evt.SetEnergy( event->GetEnergy() );
	fill_evt.SetTime( event->GetTime() );
	fill_evt.SetSegment( event->GetSegment() );

	spede_event.push_back( fill_evt );
	
}

