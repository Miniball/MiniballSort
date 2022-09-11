#include "MbsFormat.hh"

MBS::MBS() {
	
	fp = nullptr;
	ptr = nullptr;
	len = 0;
	current = -1;
	bufsize = 0x8000; // default buffer size

}

// Open the file
void MBS::Open( std::string _filename ){
	
	// Close file if already open
	if(fp) Close();
	
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
	s_filhe *fh = (s_filhe *)ptr;
	fh->Show();
	
	// Set to first buffer with real data (i.e. skipping file header)
	GetBuffer(1);
	
	// Store filename
	filename = _filename;
		
}

// Close the file
void MBS::Close() {
	
	if(!fp) return;
	fclose(fp);
	fp = nullptr;
	
}

// Get the next event
const MBSEvent* MBS::GetNextEvent() {
	
	// Return nullptr if we've reached the end of the file
	if( pos + bufsize >= len ) return(nullptr);
	
	// Event header (16 bytes)
	s_vehe *eh = (s_vehe *)(ptr + pos);
	pos += sizeof(s_evhe); // Advance to trigger/counter
	
	// Copy payload of event
	evt.Clear();
	for( UInt_t i = 0; i < eh->l_dlen * 2; i += 4 ) {
		UInt_t *val32 = (UInt_t *)(ptr + pos + i);
		evt.Store(*val32);
	}
	pos += eh->l_dlen * 2; // Advance past this data
	
	// If we are not at the end of the buffer, that's all
	if( pos != current_buffer * bufsize + used ) return(&evt);
	
	// Check if event was truncated by end of buffer
	if( bh->h_end == 1 ) {
		
		// Next buffer
		if( !GetNextBuffer() ) return(nullptr);
		
		// Event header (8 bytes)
		s_evhe *eh = (s_evhe *)(ptr + pos);
		pos += sizeof(s_evhe); // Advance past event header
		
		// Add rest of payload
		for (UInt_t i = 0; i < eh->l_dlen * 2; i += 4) {
			UInt_t *val32 = (UInt_t *)(ptr + pos + i);
			evt.Store(*val32);
		}
		pos += eh->l_dlen * 2; // Advance past this data
		if( pos != current_buffer * bufsize + used ) return(&evt);

	}
	
	// Next buffer
	if( !GetNextBuffer() ) return(nullptr);
	
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
