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
		triggers[i].SetType( MBS_L_NAME, trigname, sizeof(s_vehe), sizeof(unsigned short) );
	}
	
	// Buffer types
	buffer_types.push_back( MBSBufferElem( MBS_BTYPE_FHEADER, "File header", sizeof(s_filhe), sizeof(unsigned short) ) );
	buffer_types.push_back( MBSBufferElem( MBS_BTYPE_VME, "VME buffer", sizeof(s_bufhe), sizeof(unsigned short) ) );
	buffer_types.push_back( MBSBufferElem( MBS_BTYPE_ERROR, "Illegal buffer type", 0, 0 ) );
	buffer_types.push_back( MBSBufferElem( 0, "", 0, 0 ) );
	
	// Event types
	event_types.push_back( MBSBufferElem( MBS_ETYPE_VME, "Standard MBS event", sizeof(s_vehe), sizeof(unsigned short) ) );
	event_types.push_back( MBSBufferElem( 0, "", 0, 0 ) );
	
	// Subevent types
	sevent_types.push_back( MBSBufferElem( MBS_STYPE_CAMAC_1, "Chn + Data", sizeof(s_veshe), sizeof(unsigned int)*10 ) );
	sevent_types.push_back( MBSBufferElem( MBS_STYPE_CAMAC_WO_ID_1, "Data w/o Chn", sizeof(s_veshe), sizeof(unsigned short) ) );
	sevent_types.push_back( MBSBufferElem( MBS_STYPE_CAMAC_2, "Chn + Data", sizeof(s_veshe), sizeof(unsigned int)*10 ) );
	sevent_types.push_back( MBSBufferElem( MBS_STYPE_CAMAC_WO_ID_2, "Data w/o Chn", sizeof(s_veshe), sizeof(unsigned short) ) );
	sevent_types.push_back( MBSBufferElem( MBS_STYPE_TIME_STAMP, "Time stamp", sizeof(s_veshe), sizeof(unsigned int) ) );
	sevent_types.push_back( MBSBufferElem( MBS_STYPE_DEAD_TIME, "Dead time", sizeof(s_veshe), sizeof(unsigned int) ) );
	sevent_types.push_back( MBSBufferElem( MBS_STYPE_CAMAC_DGF_1, "XIA DGF-4C (1)", sizeof(s_veshe), sizeof(unsigned short) ) );
	sevent_types.push_back( MBSBufferElem( MBS_STYPE_CAMAC_DGF_2, "XIA DGF-4C (2)", sizeof(s_veshe), sizeof(unsigned short) ) );
	sevent_types.push_back( MBSBufferElem( MBS_STYPE_CAMAC_DGF_3, "XIA DGF-4C (3)", sizeof(s_veshe), sizeof(unsigned short) ) );
	sevent_types.push_back( MBSBufferElem( MBS_STYPE_CAMAC_SILENA_1, "Silena 4418 (1, zero suppr)", sizeof(s_veshe), sizeof(unsigned short) ) );
	sevent_types.push_back( MBSBufferElem( MBS_STYPE_CAMAC_SILENA_2, "Silena 4418 (2, zero suppr)", sizeof(s_veshe), sizeof(unsigned short) ) );
	sevent_types.push_back( MBSBufferElem( MBS_STYPE_VME_CAEN_V556_1, "Caen VME ADCs V556 (1)", sizeof(s_veshe), sizeof(unsigned short) ) );
	sevent_types.push_back( MBSBufferElem( MBS_STYPE_VME_CAEN_V556_2, "Caen VME ADCs V556 (2)", sizeof(s_veshe), sizeof(unsigned short) ) );
	sevent_types.push_back( MBSBufferElem( MBS_STYPE_VME_CAEN_V556_3, "Caen VME ADCs V556 (3)", sizeof(s_veshe), sizeof(unsigned short) ) );
	sevent_types.push_back( MBSBufferElem( MBS_STYPE_VME_CAEN_1, "Caen VME ADCs/TDCs (1)", sizeof(s_veshe), sizeof(unsigned short) ) );
	sevent_types.push_back( MBSBufferElem( MBS_STYPE_VME_CAEN_2, "Caen VME ADCs/TDCs (2)", sizeof(s_veshe), sizeof(unsigned short) ) );
	sevent_types.push_back( MBSBufferElem( MBS_STYPE_VME_CAEN_3, "Caen VME ADCs/TDCs (3)", sizeof(s_veshe), sizeof(unsigned short) ) );
	sevent_types.push_back( MBSBufferElem( MBS_STYPE_VME_CAEN_V1X90_1, "Caen VME TDCs V1X90 (1)", sizeof(s_veshe), sizeof(unsigned short) ) );
	sevent_types.push_back( MBSBufferElem( MBS_STYPE_VME_CAEN_V1X90_2, "Caen VME TDCs V1X90 (2)", sizeof(s_veshe), sizeof(unsigned short) ) );
	sevent_types.push_back( MBSBufferElem( MBS_STYPE_VME_CAEN_V1X90_3, "Caen VME TDCs V1X90 (3)", sizeof(s_veshe), sizeof(unsigned short) ) );
	sevent_types.push_back( MBSBufferElem( MBS_STYPE_VME_SIS_1, "SIS VME modules (1)", sizeof(s_veshe), sizeof(unsigned short) ) );
	sevent_types.push_back( MBSBufferElem( MBS_STYPE_VME_SIS_2, "SIS VME modules (2)", sizeof(s_veshe), sizeof(unsigned short) ) );
	sevent_types.push_back( MBSBufferElem( MBS_STYPE_VME_SIS_3, "SIS VME modules (3)", sizeof(s_veshe), sizeof(unsigned short) ) );
	sevent_types.push_back( MBSBufferElem( MBS_STYPE_VME_SIS_3300, "SIS 3300 VME modules", sizeof(s_veshe), sizeof(unsigned short) ) );
	sevent_types.push_back( MBSBufferElem( MBS_STYPE_VME_SIS_3302_1, "SIS 3302 VME modules (1)", sizeof(s_veshe), sizeof(unsigned long) ) );
	sevent_types.push_back( MBSBufferElem( MBS_STYPE_VME_SIS_3302_2, "SIS 3302 VME modules (2)", sizeof(s_veshe), sizeof(unsigned long) ) );
	sevent_types.push_back( MBSBufferElem( MBS_STYPE_VME_SIS_3302_3, "SIS 3302 VME modules (3)", sizeof(s_veshe), sizeof(unsigned long) ) );
	sevent_types.push_back( MBSBufferElem( MBS_STYPE_HITBUF_1, "Hitbuffer (1)", sizeof(s_veshe), sizeof(unsigned short) ) );
	sevent_types.push_back( MBSBufferElem( MBS_STYPE_VME_MADC_1, "Mesytec MADC modules (1)", sizeof(s_veshe), sizeof(unsigned short) ) );
	sevent_types.push_back( MBSBufferElem( MBS_STYPE_VME_MADC_2, "Mesytec MADC modules (2)", sizeof(s_veshe), sizeof(unsigned short) ) );
	sevent_types.push_back( MBSBufferElem( MBS_STYPE_VME_MADC_3, "Mesytec MADC modules (3)", sizeof(s_veshe), sizeof(unsigned short) ) );
	sevent_types.push_back( MBSBufferElem( MBS_STYPE_DATA_SHORT, "Plain data (16 bit)", sizeof(s_veshe), sizeof(unsigned short) ) );
	sevent_types.push_back( MBSBufferElem( MBS_STYPE_DATA_INT, "Plain data (32 bit)", sizeof(s_veshe), sizeof(unsigned short) ) );
	sevent_types.push_back( MBSBufferElem( MBS_STYPE_DUMMY, "MBS dummy", sizeof(s_veshe), sizeof(unsigned int)*10 ) );
	sevent_types.push_back( MBSBufferElem( MBS_STYPE_RAW, "Raw data", sizeof(s_veshe), sizeof(unsigned short) ) );
	sevent_types.push_back( MBSBufferElem( 0, "", 0, 0 ) );
	
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
	
	// MED file byte order is fixed
	byteorder = BYTE_ORDER_REV;
	bufsize = 0x4000; // fix byte order for now
	
	// Store filename
	filename = _filename;
	
	// Initialse number of hits
	ResetHitCount();
	
	// Set position to the start of the file
	pos = 0;
	
	
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
	
	// Clear old data
	evt.Clear();
	
	// Update buffer count and used bytes (bodge)
	current_buffer = (int)pos / (int)len;
	used = pos;

	// Check length
	if( pos >= len - sizeof(s_evhe) ){
		std::cerr << "End of file reached whilst trying to get next event" << std::endl;
		running = false;
		eof = true;
		pos = len;
		return nullptr;
	}
	
	// Event header (8 bytes)
	eh = (s_evhe *)(ptr + pos);
	ConvertEventHeader();
	evtsiz = elen * sizeof(UShort_t) + sizeof(s_evhe);

	// Check the event type
	for( unsigned int i = 0; i < event_types.size(); i++ ){
		if( event_types[i].GetType() == etype ){
			current_etype = &event_types[i];
			current_etype->IncrementHit();
			break;
		}
	}
	
	// Error checking for events
	if( etype == MBS_ETYPE_ERROR || etype == MBS_ETYPE_ABORT ){
		std::cerr << "Bad event type = " << etype << std::endl;
		pos += evtsiz;
		return(nullptr);
	}

	// Checking for a start or stop event
	if( etype == MBS_ETYPE_VME ) {
		
		// Get VME sub-event header and trigger type
		vh = (s_vehe *)(ptr + pos);
		ConvertVmeHeader();
		
		// Check we have a valid trigger
		if( trigger_id < MBS_N_TRIGGERS ){
			
			current_trigger = &triggers[trigger_id];
			current_trigger->IncrementHit();
			if( trigger_id == 14 ){
				std::cout << "Start trigger #14" << std::endl;
				running = true;
				pos += evtsiz;
				return(nullptr);
			}
			else if( trigger_id == 15 ) {
				std::cout << "Stop trigger #15" << std::endl;
				running = false;
				eof = true;
				pos += evtsiz;
				return(nullptr);
			}
		}
		
		// Bad trigger
		else {
			std::cerr << "Bad trigger: " << trigger_id << std::endl;
			pos += evtsiz;
			return(nullptr);
		}
		
		// Advance to sub event
		pos += sizeof(s_vehe);
		
	} // VME event
	
	// Has to be VME event if not an error, but just in case
	else {
		pos += evtsiz;
		return(nullptr);
	}
	
	// Loop over all sub-events
	current_subevt = 0;
	while( (int)pos < (int)used + evtsiz ){
		
		// Check sub-event type
		sh = (s_evhe *)(ptr + pos);
		CheckSubEventType();
		if( stype == MBS_STYPE_EOE ){
			pos += slen*sizeof(unsigned short) + sizeof(s_evhe);
			break;
		}
		
		// VME sub-event header
		vsh = (s_veshe *)(ptr + pos);
		ConvertSubEventHeader();
		UInt_t wc = slen - (sizeof(s_veshe) - sizeof(s_evhe)) / sizeof(unsigned short);
		pos += sizeof(s_veshe);

		// Now we start the sub events
		MBSSubEvent mbs_subevt;
		mbs_subevt.SetSubEventElement( *current_stype );
		mbs_subevt.SetSubEventID( current_subevt );
		mbs_subevt.SetDataLength( slen );
		mbs_subevt.SetProcessorType( control );
		mbs_subevt.SetCrateID( crateid );
		mbs_subevt.SetModuleID( procid );
		
		// Short data types
		if( current_stype->GetDataSize() == sizeof(unsigned short) ){
		
			// Sub-event data
			UChar_t *vartmp = (UChar_t *)(ptr + pos);
			std::vector<short> tmp_val = GetByteSwapShort( (char*)&vartmp, wc, byteorder );
			for( UInt_t i = 0; i < tmp_val.size(); i++ ){
				mbs_subevt.AddData( tmp_val[i] );
			}
		
		}
		
		// Integer data types
		else if( current_stype->GetDataSize() == sizeof(unsigned int) ){
		
			// Sub-event data
			UChar_t *vartmp = (UChar_t *)(ptr + pos);
			std::vector<int> tmp_val = GetByteSwapInt( (char*)&vartmp, wc, byteorder );
			for( UInt_t i = 0; i < tmp_val.size(); i++ ){
				mbs_subevt.AddData32( tmp_val[i] );
			}
		
		}
		
		// Integer data types, but read as short
		else if( current_stype->GetDataSize() == sizeof(unsigned int)*10 ){
		
			// Sub-event data
			wc *= 2;
			UChar_t *vartmp = (UChar_t *)(ptr + pos);
			std::vector<short> tmp_val = GetByteSwapShort( (char*)&vartmp, wc, byteorder );
			for( UInt_t i = 0; i < tmp_val.size(); i++ ){
				mbs_subevt.AddData( tmp_val[i] );
			}
		
		}
		
		// Long data types
		else if( current_stype->GetDataSize() == sizeof(unsigned long) ){
		
			// Sub-event data
			wc /= 2;
			UChar_t *vartmp = (UChar_t *)(ptr + pos);
			std::vector<int> tmp_val = GetByteSwapInt( (char*)&vartmp, wc, byteorder );
			for( UInt_t i = 0; i < tmp_val.size(); i++ ){
				mbs_subevt.AddData32( tmp_val[i] );
			}
		
		
		}
		
		// Store this sub event
		if( wc > 0 )
			evt.StoreSubEvent( mbs_subevt );
		
		
		// Move the pointer on
		int nextevt = slen * sizeof(unsigned short);
		nextevt += sizeof(s_evhe) - sizeof(s_veshe);
		pos += nextevt;

		// Check if we finished the file
		if( pos >= len ) {
			std::cerr << "Reached the end of the file already" << std::endl;
			break;
		}
			
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

// Convert event header
void MBS::ConvertEventHeader(){
	
	elen = GetByteSwapInt( (char *)&eh->l_dlen, byteorder );
	std::vector<short> type_tmp = GetByteSwapShort( (char *)&eh->i_type, 2, byteorder );
	
	etype = (type_tmp[0] << 16) | type_tmp[1];
	
}

// Check sub event header
void MBS::CheckSubEventType(){
	
	std::vector<short> type_tmp = GetByteSwapShort( (char *)&sh->i_type, 2, byteorder );
	
	stype = (type_tmp[0] << 16) | type_tmp[1];
	
	for( unsigned int i = 0; i < sevent_types.size(); i++ ){
		if( sevent_types[i].GetType() == stype ){
			current_stype = &sevent_types[i];
			current_stype->IncrementHit();
			break;
		}
	}
	
}
// Convert sub event header
void MBS::ConvertSubEventHeader(){
	
	slen = GetByteSwapInt( (char *)&vsh->l_dlen, byteorder );
	std::vector<short> proc_tmp = GetByteSwapShort( (char *)&vsh->i_procid, 2, byteorder );
	
	procid = proc_tmp[1];
	crateid = (char)( (proc_tmp[0] >> 8 ) & 0x00ff);
	control = (char)(proc_tmp[0] & 0x00ff);
	
}

// Convert VME header
void MBS::ConvertVmeHeader(){
	
	vlen = GetByteSwapInt( (char *)&vh->l_dlen, byteorder );
	std::vector<short> type_tmp = GetByteSwapShort( (char *)&vh->i_type, 2, byteorder );
	std::vector<short> trig_tmp = GetByteSwapShort( (char *)&vh->i_dummy, 2, byteorder);
	int lcnt_tmp = GetByteSwapInt( (char *)&vh->l_count, byteorder );
	
	trigger_id = trig_tmp[0];	// just one part of trigger/dummy pair
	evt.SetEventID( lcnt_tmp ); // l_count of event header
	
	
}

// Byte order swaps
std::vector<short> MBS::GetByteSwapShort( char *in, int count, int bo ){
	
	// Returning a short depending on the byte ordering (bo)
	std::vector<short> s;
	char b[2];
	
	switch( bo ) {
			
		case BYTE_ORDER_LSW:
		case BYTE_ORDER_1_TO_1:
			for( int i = 0; i < count; i++ ) {
				b[0] = *in++;
				b[1] = *in++;
				s.push_back( (((short)b[1] << 8) & 0xff00) |
							((short)b[0] & 0x00ff) );
			}
			break;
			
		case BYTE_ORDER_REV:
		case BYTE_ORDER_BSW:
			for( int i = 0; i < count; i++ ) {
				b[1] = *in++;
				b[0] = *in++;
				s.push_back( (((short)b[1] << 8) & 0xff00) |
							((short)b[0] & 0x00ff) );
			}
			break;
			
	}
	
	return s;
	
};

std::vector<int> MBS::GetByteSwapInt( char *in, int count, int bo ){
	
	// Returning an int depending on the byte ordering (bo)
	std::vector<int> s;
	char b[4];
	
	switch( bo ) {
			
		case BYTE_ORDER_1_TO_1:
			for( int i = 0; i < count; i++ ) {
				b[0] = *in++;
				b[1] = *in++;
				b[2] = *in++;
				b[3] = *in++;
				s.push_back( (((int)b[3] << 24) & 0xff000000) |
							(((int)b[2] << 16) & 0x00ff0000) |
							(((int)b[1] << 8) & 0x0000ff00) |
							((int)b[0] & 0x000000ff) );
			}
			break;
			
		case BYTE_ORDER_BSW:
			for( int i = 0; i < count; i++ ) {
				b[1] = *in++;
				b[0] = *in++;
				b[3] = *in++;
				b[2] = *in++;
				s.push_back( (((int)b[3] << 24) & 0xff000000) |
							(((int)b[2] << 16) & 0x00ff0000) |
							(((int)b[1] << 8) & 0x0000ff00) |
							((int)b[0] & 0x000000ff) );
			}
			break;
			
		case BYTE_ORDER_LSW:
			for( int i = 0; i < count; i++ ) {
				b[2] = *in++;
				b[3] = *in++;
				b[0] = *in++;
				b[1] = *in++;
				s.push_back( (((int)b[3] << 24) & 0xff000000) |
							(((int)b[2] << 16) & 0x00ff0000) |
							(((int)b[1] << 8) & 0x0000ff00) |
							((int)b[0] & 0x000000ff) );
			}
			break;
			
			
		case BYTE_ORDER_REV:
			for( int i = 0; i < count; i++ ) {
				b[3] = *in++;
				b[2] = *in++;
				b[1] = *in++;
				b[0] = *in++;
				s.push_back( (((int)b[3] << 24) & 0xff000000) |
							(((int)b[2] << 16) & 0x00ff0000) |
							(((int)b[1] << 8) & 0x0000ff00) |
							((int)b[0] & 0x000000ff) );
			}
			break;

	}
	
	return s;
	
};

std::string MBS::GetByteSwapString( char *in, int count, int bo ){
	
	// Returning a short depending on the byte ordering (bo)
	std::string sin;
	std::string sout;
	
	if( count <= 0 ) sin = std::string( in, std::strlen(in) );
	else sin = std::string( in, count );
	
	switch( bo ) {
			
		case BYTE_ORDER_REV:
		case BYTE_ORDER_1_TO_1:
			sout = sin;
			break;
			
		case BYTE_ORDER_LSW:
		case BYTE_ORDER_BSW:
			sout = sin;
			std::reverse( sout.begin(), sout.end() );
			break;
			
	}
	
	return sout;
	
};
