#include "MbsFormat.hh"

MBS::MBS() {
	
	fp = nullptr;
	ptr = nullptr;
	len = 0;
	current = -1;
	bufsize = 0x8000; // default buffer size
	
	// Set up the trigger types
	triggers.resize(MBS_N_TRIGGERS);
	for( unsigned int i = 0; i < MBS_N_TRIGGERS; i++ ) {
		std::string trigname = "Trig %d" + std::to_string(i);
		if( i == 14 ) trigname += " (Start)";
		if( i == 15 ) trigname += " (Stop)";
		triggers[i].SetType( MBS_L_NAME, trigname, sizeof(s_vehe), 0 );
	}
	
	// Buffer types
	MBSBufferElem tmp_buf_elem;
	tmp_buf_elem.SetType( MBS_BTYPE_FHEADER, "File header", sizeof(s_filhe), 0 );
	buffer_types.push_back( tmp_buf_elem );
	tmp_buf_elem.SetType( MBS_BTYPE_VME, "VME buffer", sizeof(s_bufhe), 0 );
	buffer_types.push_back( tmp_buf_elem );
	tmp_buf_elem.SetType( MBS_BTYPE_ERROR, "Illegal buffer type", 0, 0 );
	buffer_types.push_back( tmp_buf_elem );
	tmp_buf_elem.SetType( 0, "", 0, 0 );
	buffer_types.push_back( tmp_buf_elem );

	// Event types
	tmp_buf_elem.SetType( MBS_ETYPE_VME, "Standard MBS event", sizeof(s_vehe), 0 );
	event_types.push_back( tmp_buf_elem );
	tmp_buf_elem.SetType( 0, "", 0, 0 );
	event_types.push_back( tmp_buf_elem );

	// Subevent types
	tmp_buf_elem.SetType( MBS_STYPE_CAMAC_1, "Chn + Data" );
	sevent_types.push_back( tmp_buf_elem );
	tmp_buf_elem.SetType( MBS_STYPE_CAMAC_WO_ID_1, "Data w/o Chn" );
	sevent_types.push_back( tmp_buf_elem );
	tmp_buf_elem.SetType( MBS_STYPE_CAMAC_2, "Chn + Data" );
	sevent_types.push_back( tmp_buf_elem );
	tmp_buf_elem.SetType( MBS_STYPE_CAMAC_WO_ID_2, "Data w/o Chn" );
	sevent_types.push_back( tmp_buf_elem );
	tmp_buf_elem.SetType( MBS_STYPE_TIME_STAMP, "Time stamp" );
	sevent_types.push_back( tmp_buf_elem );
	tmp_buf_elem.SetType( MBS_STYPE_DEAD_TIME, "Dead time" );
	sevent_types.push_back( tmp_buf_elem );
	tmp_buf_elem.SetType( MBS_STYPE_CAMAC_DGF_1, "XIA DGF-4C (1)" );
	sevent_types.push_back( tmp_buf_elem );
	tmp_buf_elem.SetType( MBS_STYPE_CAMAC_DGF_2, "XIA DGF-4C (2)" );
	sevent_types.push_back( tmp_buf_elem );
	tmp_buf_elem.SetType( MBS_STYPE_CAMAC_DGF_3, "XIA DGF-4C (3)" );
	sevent_types.push_back( tmp_buf_elem );
	tmp_buf_elem.SetType( MBS_STYPE_CAMAC_SILENA_1, "Silena 4418 (1, zero suppr)" );
	sevent_types.push_back( tmp_buf_elem );
	tmp_buf_elem.SetType( MBS_STYPE_CAMAC_SILENA_2, "Silena 4418 (2, zero suppr)" );
	sevent_types.push_back( tmp_buf_elem );
	tmp_buf_elem.SetType( MBS_STYPE_VME_CAEN_V556_1, "Caen VME ADCs V556 (1)" );
	sevent_types.push_back( tmp_buf_elem );
	tmp_buf_elem.SetType( MBS_STYPE_VME_CAEN_V556_2, "Caen VME ADCs V556 (2)" );
	sevent_types.push_back( tmp_buf_elem );
	tmp_buf_elem.SetType( MBS_STYPE_VME_CAEN_V556_3, "Caen VME ADCs V556 (3)" );
	sevent_types.push_back( tmp_buf_elem );
	tmp_buf_elem.SetType( MBS_STYPE_VME_CAEN_1, "Caen VME ADCs/TDCs (1)" );
	sevent_types.push_back( tmp_buf_elem );
	tmp_buf_elem.SetType( MBS_STYPE_VME_CAEN_2, "Caen VME ADCs/TDCs (2)" );
	sevent_types.push_back( tmp_buf_elem );
	tmp_buf_elem.SetType( MBS_STYPE_VME_CAEN_3, "Caen VME ADCs/TDCs (3)" );
	sevent_types.push_back( tmp_buf_elem );
	tmp_buf_elem.SetType( MBS_STYPE_VME_CAEN_V1X90_1, "Caen VME TDCs V1X90 (1)" );
	sevent_types.push_back( tmp_buf_elem );
	tmp_buf_elem.SetType( MBS_STYPE_VME_CAEN_V1X90_2, "Caen VME TDCs V1X90 (2)" );
	sevent_types.push_back( tmp_buf_elem );
	tmp_buf_elem.SetType( MBS_STYPE_VME_CAEN_V1X90_3, "Caen VME TDCs V1X90 (3)" );
	sevent_types.push_back( tmp_buf_elem );
	tmp_buf_elem.SetType( MBS_STYPE_VME_SIS_1, "SIS VME modules (1)" );
	sevent_types.push_back( tmp_buf_elem );
	tmp_buf_elem.SetType( MBS_STYPE_VME_SIS_2, "SIS VME modules (2)" );
	sevent_types.push_back( tmp_buf_elem );
	tmp_buf_elem.SetType( MBS_STYPE_VME_SIS_3, "SIS VME modules (3)" );
	sevent_types.push_back( tmp_buf_elem );
	tmp_buf_elem.SetType( MBS_STYPE_VME_SIS_3300, "SIS 3300 VME modules" );
	sevent_types.push_back( tmp_buf_elem );
	tmp_buf_elem.SetType( MBS_STYPE_VME_SIS_3302_1, "SIS 3302 VME modules (1)" );
	sevent_types.push_back( tmp_buf_elem );
	tmp_buf_elem.SetType( MBS_STYPE_VME_SIS_3302_2, "SIS 3302 VME modules (2)" );
	sevent_types.push_back( tmp_buf_elem );
	tmp_buf_elem.SetType( MBS_STYPE_VME_SIS_3302_3, "SIS 3302 VME modules (3)" );
	sevent_types.push_back( tmp_buf_elem );
	tmp_buf_elem.SetType( MBS_STYPE_HITBUF_1, "Hitbuffer (1)" );
	sevent_types.push_back( tmp_buf_elem );
	tmp_buf_elem.SetType( MBS_STYPE_VME_MADC_1, "Mesytec MADC modules (1)" );
	sevent_types.push_back( tmp_buf_elem );
	tmp_buf_elem.SetType( MBS_STYPE_VME_MADC_2, "Mesytec MADC modules (2)" );
	sevent_types.push_back( tmp_buf_elem );
	tmp_buf_elem.SetType( MBS_STYPE_VME_MADC_3, "Mesytec MADC modules (3)" );
	sevent_types.push_back( tmp_buf_elem );
	tmp_buf_elem.SetType( MBS_STYPE_DATA_SHORT, "Plain data (16 bit)" );
	sevent_types.push_back( tmp_buf_elem );
	tmp_buf_elem.SetType( MBS_STYPE_DATA_INT, "Plain data (32 bit)" );
	sevent_types.push_back( tmp_buf_elem );
	tmp_buf_elem.SetType( MBS_STYPE_DUMMY, "MBS dummy" );
	sevent_types.push_back( tmp_buf_elem );
	tmp_buf_elem.SetType( MBS_STYPE_RAW, "Raw data" );
	sevent_types.push_back( tmp_buf_elem );
	tmp_buf_elem.SetType( 0, "", 0, 0 );
	sevent_types.push_back( tmp_buf_elem );

	
}

// Open an LMD file
void MBS::OpenLmdFile( std::string _filename ){
	
	// Close file if already open
	if(fp) CloseFile();
	
	// Open file
	fp = fopen( _filename.data(), "rb" );
	if( !fp ) {
		std::cerr << "Unable to open " << _filename << std::endl;
		return;
	}
	
	// Get length of file
	fseek( fp, 0, SEEK_END );
	len = ftell(fp);
	
	// Map into virtual memory
	ptr = (const UChar_t *)mmap(nullptr, len, PROT_READ, MAP_SHARED, fileno(fp), 0);
	if( ptr == MAP_FAILED ) {
		
		std::cerr << __FUNCTION__ << ": Error mapping MBS file " << _filename << std::endl;
		fclose(fp);
		fp = nullptr;
		ptr = nullptr;
		return;
		
	}

	// File header
	fh = (s_filhe *)ptr;
	fh->Show();
	
	// Set to first buffer with real data (i.e. skipping file header)
	GetBuffer(1);
	
	// Store filename
	filename = _filename;
	
}

// Open a MED file
void MBS::OpenMedFile( std::string _filename ){
	
	// Close file if already open
	if(fp) CloseFile();
	
	// Event type initialisation
	if( sevent_type_raw == NULL ) {
		
		for( unsigned int i = 0; i < sevent_types.size(); i++ ) {
		
			if( sevent_types[i].GetType() == MBS_STYPE_RAW ) {
				sevent_type_raw = &sevent_types[i];
				break;
			}
			
		}
			
	}
	
	// Open file
	fp = fopen( _filename.data(), "rb" );
	if( !fp ) {
		std::cerr << "Unable to open " << _filename << std::endl;
		return;
	}
	
	// Get length of file
	fseek( fp, 0, SEEK_END );
	len = ftell(fp);
	
	// Map into virtual memory
	ptr = (const UChar_t *)mmap(nullptr, len, PROT_READ, MAP_SHARED, fileno(fp), 0);
	if( ptr == MAP_FAILED ) {
		
		std::cerr << __FUNCTION__ << ": Error mapping MED file " << _filename << std::endl;
		fclose(fp);
		fp = nullptr;
		ptr = nullptr;
		return;
		
	}
	
	// File header
	fh = (s_filhe *)ptr;
	fh->Show();
	
	// Set to first buffer with real data (i.e. skipping file header)
	GetBuffer(1);
	
	// Store filename
	filename = _filename;
	
	// Initialse number of hits
	ResetHitCount();

	
}

// Close the file
void MBS::CloseFile() {
	
	if(!fp) return;
	fclose(fp);
	fp = nullptr;
	
}

// Open a stream
int MBS::OpenEventServer( std::string _server, unsigned short _port ){
	
	// Get the server and port number
	server = _server;
	port = _port;
	
	// Create to the socket
	struct sockaddr_in serv_addr;
	if( !(socket_id = socket( AF_INET, SOCK_STREAM, 0 )) ){
		
		std::cerr << "Socket creation failed" << std::endl;
		return -1;
		
	}
	
	// Make the address of the server
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons( port );
	if( inet_pton( AF_INET, server.data(), &serv_addr.sin_addr ) <= 0 ) {
		
		std::cerr << "Invalid server address " << server << std::endl;
		return -1;
		
	}
	
	// Connect to the server
	if( (server_id = connect( socket_id, (struct sockaddr*)&serv_addr,
							 sizeof(serv_addr) )) < 0 ) {
		
		std::cerr << "Failed to connect to " << server << std::endl;
		return -1;
		
	}
	
	return 0;
	
}

void MBS::CloseEventServer() {
	
	close(server_id);
	
}

const UChar_t* MBS::GetBufferFromStream(){
	
	// TODO: This is not correct, probably!!
	read( socket_id, &ptr, bufsize );
	
	return ptr;
	
}

// Get the next event from an LMD file
const MBSEvent* MBS::GetNextLmdEvent() {
	
	// Return nullptr if we've reached the end of the file
	if( pos + bufsize >= len ) return(nullptr);
	
	// Check if we need another buffer
	if( pos >= current_buffer * bufsize + used )
		if( !GetNextBuffer() ) return(nullptr);
	
	// Clear old data
	evt.Clear();
	
	// Event header (16 bytes, assuming VME)
	UInt_t *val32 = (UInt_t *)(ptr + pos);
	UInt_t elen = val32[0]; // l_dlen of event header
	evtsiz = elen * sizeof(UShort_t) + sizeof(s_evhe);
	evt.SetEventID( val32[3] ); // l_count of event header
	vh = (s_vehe *)(ptr + pos);
	//UInt_t elen = eh->l_dlen;
	pos += sizeof(s_evhe); // Advance to trigger/counter
	
	// Subevent header (8 bytes)
	UInt_t slen = val32[4];  // l_dlen of subevent header
	//UInt_t slen = vh->l_dlen;
	
	// Handle the special case, where the subevent header is in the
	// next buffer
	if( elen <= 4 ) {
		
		for( UInt_t i = 2; i < 4; i++ )
			evt.Store(val32[i]);
		
		// Next buffer
		if( !GetNextBuffer() ) return(nullptr);
		val32 = (UInt_t *)(ptr + pos);
		elen = val32[0];
		slen = val32[2];
		pos += 8;
		
	}
	
	
	// Copy payload of event (without event header)
	for( UInt_t i = 2; i < elen / 2 + 2; i++ )
		evt.Store(val32[i]);
	pos += elen * 2; // Advance past this data
	
	// Check if there's more data in the next buffer
	while( evt.GetNData() < slen / 2 + 2 ) { // Yes, there's more data
		
		// Next buffer
		if( !GetNextBuffer() ) return(nullptr);
		val32 = (UInt_t *)(ptr + pos);
		elen = val32[0];
		for( UInt_t i = 2; i < elen / 2 + 2; i++ )
			evt.Store(val32[i]);
		pos += elen * 2 + 8; // Advance past this data
		
	}
	
	return(&evt);
	
};

// Get the next event from a MED file
const MBSEvent* MBS::GetNextMedEvent() {
	
	// Return nullptr if we've reached the end of the file
	if( pos + bufsize >= len ) return(nullptr);
	
	// Check if we need another buffer
	if( pos >= current_buffer * bufsize + used )
		if( !GetNextBuffer() ) return(nullptr);
	
	// Clear old data
	evt.Clear();
	
	// Event header (8 bytes)
	eh = (s_evhe *)(ptr + pos);
	unsigned int etype = (eh->i_subtype << 16) | eh->i_type;
	UInt_t elen = eh->l_dlen; // l_dlen of event header
	evtsiz = elen * sizeof(UShort_t) + sizeof(s_evhe);
	
	// Handle the special case, where the subevent header is in the
	if( elen <= 4 ) {
		
		pos += sizeof(s_evhe);
		if( !GetNextBuffer() ) return(nullptr);
		
	}

	
	// Check the event type
	for( unsigned int i = 0; i < event_types.size(); i++ ){
		if( event_types[i].GetType() == etype ){
			current_etype = &event_types[i];
			current_etype->IncrementHit();
			break;
		}
	}
	
	// Error checking for events
	if( current_etype->GetType() == MBS_ETYPE_ERROR ||
	    current_etype->GetType() == MBS_ETYPE_ABORT ){
		std::cerr << "Bad event type = " << MBS_ETYPE_ERROR << std::endl;
		pos += elen;
		return(nullptr);
	}
	
	// Checking for a start or stop event
	unsigned int trigger_id;
	if( etype == MBS_ETYPE_VME ) {
		
		// Get VME sub-event header and trigger type
		vh = (s_vehe *)(ptr + pos);
		trigger_id = vh->i_trigger;
		evt.SetEventID( vh->l_count ); // l_count of event header

		// Advance to sub event
		pos += sizeof(s_vehe);

		// Check we havea valid trigger
		if( trigger_id < MBS_N_TRIGGERS ){

			current_trigger = &triggers[trigger_id];
			current_trigger->IncrementHit();
			if( trigger_id == 14 ){
				std::cout << "Start trigger #14" << std::endl;
				running = true;
				return(nullptr);
			}
			else if( trigger_id == 15 ) {
				std::cout << "Start trigger #14" << std::endl;
				running = false;
				return(nullptr);
			}
		}
		
		// Bad trigger
		else {
			std::cerr << "Bad trigger: " << trigger_id << std::endl;
			return(nullptr);
		}
		
	} // VME event
	
	// Has to be VME event if not an error, but just in case
	else return(nullptr);
	
	// Loop over all sub-events
	current_subevt = 0;
	while( pos < current_buffer * bufsize + used + evtsiz ){

		// Check if we need a new buffer
		if( pos >= current_buffer * bufsize + used )
			if( !GetNextBuffer() ) return(nullptr);

		
		// Check sub-event type
		sh = (s_evhe *)(ptr + pos);
		unsigned int stype = (sh->i_subtype << 16) | sh->i_type;
		for( unsigned int i = 0; i < sevent_types.size(); i++ ){
			if( sevent_types[i].GetType() == stype ){
				current_stype = &sevent_types[i];
				current_stype->IncrementHit();
				break;
			}
		}
		pos += sizeof(s_evhe);

		// Sub-event header
		vsh = (s_veshe *)(ptr + pos);
		UInt_t slen = vsh->l_dlen - 2; // length of sub-event data in 16-bit words
		UInt_t wc = slen - (sizeof(s_veshe) - sizeof(s_evhe)) / sizeof(unsigned short);
		pos += sizeof(s_veshe);
		
		// Now we start the sub events
		MBSSubEvent mbs_sevt;
		mbs_sevt.SetSubEventElement( *current_stype );
		mbs_sevt.SetSubEventID( current_subevt );
		mbs_sevt.SetDataLength( slen );
		mbs_sevt.SetProcessorType( vsh->h_control );
		mbs_sevt.SetCrateID( vsh->h_subcrate );
		mbs_sevt.SetModuleID( vsh->i_procid );

		// Sub-event data
		UChar_t *var16 = (UChar_t *)(ptr + pos);
		for( UInt_t i = 0; i < wc; i++ ){
			mbs_sevt.AddData( var16[i] );
		}
		
		// Move the pointer on
		pos += wc * sizeof(UChar_t);
			
		// Increment the subevent counter
		current_subevt++;
	
	} // finish sub events
	
	
	return(&evt);
	
};

// Get the next event
const MBSEvent* MBS::GetNextEventFromStream() {

	// Clear old data
	evt.Clear();

	// Need to do stuff here
	
	return(&evt);

	
}

// Get the nth buffer
const UChar_t* MBS::GetBuffer( UInt_t i ) {
	
	current_buffer = i;
	pos = current_buffer * bufsize;
	if( pos + bufsize > len ) return(nullptr);
	
	bh = (s_bufhe *)(ptr + pos);
	used = bh->i_used * 2 + sizeof(s_bufhe);
	pos += sizeof(s_bufhe);
	
	// Check buffer type for med files
	for( unsigned int i = 0; i < buffer_types.size(); i++ ){
		if( buffer_types[i].GetType() == bh->i_subtype ){
			current_btype = &buffer_types[i];
			current_btype->IncrementHit();
			break;
		}
	}
	
	// Is it a VME buffer?
	if( current_btype->GetType() != MBS_BTYPE_VME ){
		std::cerr << "Current buffer type incorrect. Expecting ";
		std::cerr << MBS_BTYPE_VME << " but got " << current_btype->GetType();
		std::cerr << std::endl;
	}
	
	// Store the current buffer timestamp
	buf_ts = ((long long) bh->l_time[0]) << 32 | ((long long) bh->l_time[1]);
	if( start_ts == 0 ) start_ts = buf_ts;
	
	return( ptr + pos - sizeof(s_bufhe) );
	
};
