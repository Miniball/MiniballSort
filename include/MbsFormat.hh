// A class to store and rea MBS format data
// Shamelessly stolen from Nigel Warr (IKP Koln)
// who will disapprove of this poor implmentation
//
// Additional elements for MED files stolen from Marabou
// written by Rudi Lutter

#ifndef __MBSFORMAT_HH
#define __MBSFORMAT_HH

#include <TROOT.h>
#include <iostream>
#include <string>
#include <ctime>
#include <cstdio>
#include <sys/mman.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

// MBS defines header
#ifndef __MBSDEFINES_HH
# include "MbsDefines.hh"
#endif


// String
struct cv_string {
	UShort_t string_l;
	Char_t string[78];
};

// File header
typedef struct s_filhe {
	UInt_t l_dlen;
	UShort_t i_type;
	UShort_t i_subtype;
	UShort_t i_used;
	UShort_t i_frag;
	UInt_t l_buf;
	UInt_t l_evt;
	UInt_t l_current_i;
	UInt_t l_stime[2];
	UInt_t l_free[4];
	UShort_t i_label_l;
	Char_t c_label[30];
	UShort_t i_file_l;
	Char_t c_file[86];
	UShort_t i_user_l;
	Char_t c_user[30];
	Char_t s_time[24];
	UShort_t i_run_l;
	Char_t c_run[66];
	UShort_t i_exp_l;
	Char_t c_exp[66];
	UInt_t l_lines;
	struct cv_string s_strings[30];
	void Show() const {
		printf("File: dlen=%d type=[%d,%d] used=%d frag=%d buf=%d evt=%d current_i=%d stime=%d.%d free=(%d,%d,%d,%d)\n",
			   l_dlen, i_type, i_subtype, i_used, i_frag, l_buf, l_evt,
			   l_current_i, l_stime[0], l_stime[1], l_free[0], l_free[1],
			   l_free[2], l_free[3]);
		printf("\tlabel=");
		for (UInt_t i = 0; i < i_label_l; i++) printf("%c", c_label[i]);
		printf("\n\tfile=");
		for (UInt_t i = 0; i < i_file_l; i++) printf("%c", c_file[i]);
		printf("\n\tuser=");
		for (UInt_t i = 0; i < i_user_l; i++) printf("%c", c_user[i]);
		printf("\n\ttime=%s\n\trun=", s_time);
		for (UInt_t i = 0; i < i_run_l; i++) printf("%c", c_run[i]);
		printf("\n\texp=");
		for (UInt_t i = 0; i < i_exp_l; i++) printf("%c", c_exp[i]);
		printf("\n");
	};
} s_filhe;

// Buffer header
typedef struct s_bufhe {
	UInt_t l_dlen;       ///< Length of data field in 16-bit words
	UShort_t i_type;     ///< Type
	UShort_t i_subtype;  ///< Subtype
	UShort_t i_used;     ///< Used length of data field in 16-bit words
	UChar_t h_begin;     ///< Fragment at end of buffer
	UChar_t h_end;       ///< Fragment at begin of buffer
	UInt_t l_buf;        ///< Current buffer number
	UInt_t l_evt;        ///< Number of fragments
	UInt_t l_current_i;  ///< Index
	UInt_t l_time[2];    ///< 64-bit timestamp since 1970
	UInt_t l_free[4];    ///< Free
	void Show() const {
		time_t t = (time_t)l_time[0];
		printf("Buffer: dlen=%d type=[%d,%d] used=%d begin=%d end=%d buf=%d evt=%d current_i=%d time=%d.%d free=(%d,%d,%d,%d) %s",
			   l_dlen, i_type, i_subtype, i_used, h_begin, h_end, l_buf, l_evt,
			   l_current_i, l_time[0], l_time[1], l_free[0], l_free[1],
			   l_free[2], l_free[3], ctime(&t));
	};
} s_bufhe;

// Full event header
typedef struct s_vehe {
	UInt_t l_dlen;       ///< Length of data in 16-bit words
	UShort_t i_type;     ///< Type
	UShort_t i_subtype;  ///< Subtype
	UShort_t i_dummy;    ///< Not used
	UShort_t i_trigger;  ///< Trigger
	UInt_t l_count;      ///< Event number
	void Show() const {
		printf("Event dlen=%d type=[%d,%d] dummy=%d trigger=%d count=%d\n",
			   l_dlen, i_type, i_subtype, i_dummy, i_trigger, l_count);
	};
} s_vehe;

// Main event header
typedef struct s_evhe {
	UInt_t l_dlen;       ///< Length of data in 16-bit words
	UShort_t i_type;     ///< Type
	UShort_t i_subtype;  ///< Subtype
} s_evhe;

// GSI VME event header
typedef struct s_veshe {
	UInt_t l_dlen;			///< data length + 2 in words
	UShort_t i_subtype;		///< subtype
	UShort_t i_type;		///< type
	UChar_t h_control;		///< processor type
	UChar_t h_subcrate;		///< crate number
	UShort_t i_procid; 		///< processor ID
} s_veshe;

//-----------------------------------------------------------------------------
// MBS Buffer Element class for MED files
class MBSBufferElem {

private:
	
	unsigned int type;		//!< element type
	std::string descr;		//!< description
	int hsize;				//!< header size (bytes)
	int hit;				//!< # of hits
	
public:
	
	// Default constructor
	MBSBufferElem(){
		type = 0;
		descr = "";
		hsize = 0;
		hit = 0;
	};
	
	// Setting onstructor
	MBSBufferElem( unsigned int _type, std::string _descr, int _hsize, int _hit ){
		type = _type;
		descr = _descr;
		hsize = _hsize;
		hit = _hit;
	};
	
	// Destructor
	~MBSBufferElem(){};
	
	// Setter functions
	void SetType( unsigned int _type, std::string _descr, int _hsize, int _hit ){
		type = _type;
		descr = _descr;
		hsize = _hsize;
		hit = _hit;
	};
	void SetType( unsigned int _type, std::string _descr, int _hsize ){
		type = _type;
		descr = _descr;
		hsize = _hsize;
		hit = 0;
	};
	void SetType( unsigned int _type, std::string _descr ){
		type = _type;
		descr = _descr;
		hsize = sizeof(s_veshe);
		hit = 0;
	};
	
	// Getter functions
	unsigned int GetType() const { return type; };
	int GetHeaderSize() const { return hsize; };

	// Other functions
	void IncrementHit(){ hit++; };
	void ResetHits(){ hit = 0; };

	int Unpack();		//!< function to unpack element
	int Show();			//!< function to to show element
	int Convert();		//!< function to convert element data

};

//-----------------------------------------------------------------------------
// MBS subevent class
class MBSSubEvent {
	
private:

	std::vector<unsigned char> data;
	unsigned int data_len;			///< length of the sub event
	unsigned long seventid;			///< subevent counter
	unsigned char proctype;			///< processor type
	unsigned char crateid;			///< crate number
	unsigned short modid; 			///< module ID
	MBSBufferElem stype;		///< type class identifier

public:
	
	// Tracking data - 16-bit words
	void AddData( UChar_t datum ){
		data.push_back(datum);
	};
	unsigned char GetData16( unsigned int i ) const {
		if( i < data.size() ) return data.at(i);
		return 0;
	};
	unsigned int	GetNumberOfData16() const { return data.size(); };

	// Return 32-bit words
	unsigned short	GetData32( unsigned int i ) const {
		if( i < data.size()/2 )
			return ( (short)data.at(2*i) | ( (short)data.at(2*i+1) << 16 ) );
		return 0;
	};
	unsigned int	GetNumberOfData32() const { return data.size()/2; };

	// Default return value is 32-bits
	unsigned short	GetData( unsigned int i ) const { return GetData32(i); };
	unsigned int	GetNumberOfData() const { return GetNumberOfData32(); };

	// Getters
	MBSBufferElem	GetSubEventElement() const { return stype; };
	unsigned int	GetSubEventType() const { return stype.GetType(); };
	unsigned long	GetSubEventID() const { return seventid; };
	unsigned int	GetDataLength() const { return data_len; };
	unsigned char	GetProcessorType() const { return proctype; };
	unsigned char	GetCrateID() const { return crateid; };
	unsigned short	GetModuleID() const { return modid; };

	// Setters
	void SetSubEventElement( MBSBufferElem _stype ){ stype = _stype; };
	void SetSubEventID( unsigned long id ){ seventid = id; };
	void SetDataLength( unsigned int len ){ data_len = len; };
	void SetProcessorType( unsigned char ptype ){ proctype = ptype; };
	void SetCrateID( unsigned char id ){ crateid = id; };
	void SetModuleID( unsigned short id ){ modid = id; };

};

//-----------------------------------------------------------------------------
// MBS event class
class MBSEvent {
	
private:

	std::vector<MBSSubEvent> sevts;
	std::vector<UInt_t> data;
	ULong_t eventid;

public:
	
	// Get the trigger
	UInt_t GetTrigger() const {
		if( data.size() < 1 ) return(0);
		return( data[0] >> 16 );
	};
	
	// Get the counter
	UInt_t GetCount() const {
		if( data.size() < 2 ) return(0);
		return( data[1] );
	};
	
	// Get the number of actual 32-bit data words
	size_t GetNData() const {
		return( data.size() < 2 ? 0 : data.size() - 2 );
	};
	
	// Get the actual data (without the trigger and counter) as 32-bit words
	const UInt_t *GetData() const {
		if( data.size() < 2 ) return(nullptr);
		return( &data[2] );
	};
	
	// Get event id
	ULong_t GetEventID() const { return eventid; };

	// set event id
	void SetEventID( unsigned long long id ){ eventid = id; };

	// Store the data - first the trigger, then the counter, then the actual
	// event data
	void Store( UInt_t datum ) {
		data.push_back(datum);
	};
	
	// Store the subevent
	void StoreSubEvent( MBSSubEvent _sevt ) {
		sevts.push_back(_sevt);
	};
	
	// Get the sub events
	unsigned int GetNumberOfSubEvents() const { return sevts.size(); };
	const MBSSubEvent *GetSubEvent( unsigned int i ) const {
		if( i < sevts.size() ) return &sevts[i];
		else return nullptr;
	};
	
	// Clear the event
	void Clear() {
		sevts.clear();
		data.clear();
	};
	
	// Show the contents of the event
	void Show( UInt_t verbose = 0 ) const {
		printf( "Event %d: trigger %d\n", GetCount(), GetTrigger() );
		if( verbose < 1 ) return;
		printf("\t");
		for( UInt_t i = 0; i < GetNData(); i++ ) {
			if( (i % 4) == 0 && i ) printf("\n\t");
			printf("%08X ", GetData()[i]);
		}
		printf("\n");
	};
};

//-----------------------------------------------------------------------------
// MBS class
class MBS {

private:
	
	std::string filename;
	std::string server;
	unsigned short port;
	FILE *fp;
	UInt_t socket_id;
	Int_t server_id;
	UInt_t current_buffer;
	UInt_t current_subevt;
	UInt_t pos;
	MBSEvent evt;
	s_filhe *fh; // file header
	s_bufhe *bh; // buffer header
	s_evhe *eh; // event header
	s_vehe *vh; // vme event header
	s_evhe *sh; // sub event header
	s_veshe *vsh; // vme sub event header
	UInt_t used; // Bytes used in buffer including header
	unsigned int evtsiz; // in bytes
	long long start_ts = 0;
	long long buf_ts;
	bool running = false;

	// For med files
	MBSBufferElem *current_btype = nullptr;
	MBSBufferElem *current_etype = nullptr;
	MBSBufferElem *current_stype = nullptr;
	MBSBufferElem *current_trigger = nullptr;
	MBSBufferElem *sevent_type_raw = nullptr;

	// Buffer elements
	std::vector<MBSBufferElem> buffer_types;
	std::vector<MBSBufferElem> event_types;
	std::vector<MBSBufferElem> sevent_types;
	std::vector<MBSBufferElem> triggers;

	const UChar_t *ptr;
	size_t len;
	Int_t current;
	UInt_t bufsize;
	
public:

	// Default constructor
	MBS();
	
	// Destructor
	~MBS(){};
	
	// Open and close functions
	void OpenLmdFile( std::string _filename );
	void OpenMedFile( std::string _filename );
	int OpenEventServer( std::string _server, unsigned short _port );
	void CloseFile();
	void CloseEventServer();

	void SetBufferSize( unsigned int size ){ bufsize = size; };
	
	// Get number of buffers
	UInt_t GetNBuffers() const {
		return( len ? len / bufsize - 1 : 0 );
	};
	
	// Get the buffer count
	UInt_t GetBufferCount(){ return current_buffer; };
	
	// Get the nth buffer
	const UChar_t* GetBuffer( UInt_t i );
	
	// Get the next buffer
	const UChar_t* GetNextBuffer() {
		return( GetBuffer(++current_buffer) );
	};
	
	// Get the next buffer from the stream
	const UChar_t* GetBufferFromStream();
	
	// Get the next event from file
	const MBSEvent* GetNextLmdEvent();
	const MBSEvent* GetNextMedEvent();
	
	// Get the next event from stream
	const MBSEvent* GetNextEventFromStream();
	
	// Show the file header
	void ShowFileHeader() {
		if(!fp) return;
		fh = (s_filhe *)ptr;
		fh->Show();
	};
	
	// Reset hit count at the start of the file
	void ResetHitCount(){
		for( unsigned int i = 0; i < buffer_types.size(); i++ )
			buffer_types[i].ResetHits();
		for( unsigned int i = 0; i < event_types.size(); i++ )
			event_types[i].ResetHits();
		for( unsigned int i = 0; i < sevent_types.size(); i++ )
			sevent_types[i].ResetHits();
		for( unsigned int i = 0; i < triggers.size(); i++ )
			triggers[i].ResetHits();
	};

};


#endif
