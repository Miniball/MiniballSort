#include "MbsFormat.hh"

MBS::MBS() {
	
	fp = nullptr;
	ptr = nullptr;
	len = 0;
	current = -1;
	bufsize = 0x8000; // default buffer size
	
}

// Open the file
void MBS::OpenFile( std::string _filename ){
	
	// Close file if already open
	if(fp) CloseFile();
	
	// Open file
	fp = fopen( _filename.data(), "rb");
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


// Get the next event
const MBSEvent* MBS::GetNextEvent() {
	
	// Return nullptr if we've reached the end of the file
	if( pos + bufsize >= len ) return(nullptr);
	
	// Check if we need another buffer
	if( pos >= current_buffer * bufsize + used )
		if( !GetNextBuffer() ) return(nullptr);
	
	// Clear old data
	evt.Clear();
	
	// Event header (16 bytes)
	UInt_t *val32 = (UInt_t *)(ptr + pos);
	UInt_t elen = val32[0]; // l_dlen of event header
	evt.SetEventID( val32[3] ); // l_count of event header
	//eh = (s_vehe *)(ptr + pos);
	//UInt_t elen = eh->l_dlen;
	pos += sizeof(s_evhe); // Advance to trigger/counter
	
	// Subevent header (8 bytes)
	UInt_t slen = val32[4];  // l_dlen of subevent header
	//sh = (s_evhe *)(ptr + pos);
	//UInt_t slen = sh->l_dlen;
	
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

// Get the nth buffer
const UChar_t* MBS::GetBuffer( UInt_t i ) {
	
	current_buffer = i;
	pos = current_buffer * bufsize;
	if( pos + bufsize > len ) return(nullptr);
	
	bh = (s_bufhe *)(ptr + pos);
	used = bh->i_used * 2 + sizeof(s_bufhe);
	pos += sizeof(s_bufhe);
	
	return( ptr + pos - sizeof(s_bufhe) );
	
};
