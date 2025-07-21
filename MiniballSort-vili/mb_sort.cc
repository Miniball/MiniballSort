// My code include.
#include "mb_sort.hh"

// GUI Header
#ifndef __MINIBALLGUI_HH
#include "MiniballGUI.hh"
#endif

// Settings header
#ifndef __SETTINGS_HH
# include "Settings.hh"
#endif

// Calibration header
#ifndef __CALIBRATION_HH
# include "Calibration.hh"
#endif

// Reaction header
#ifndef __REACTION_HH
# include "Reaction.hh"
#endif

// Converter headers
#ifndef __MIDASCONVERTER_HH
# include "MidasConverter.hh"
#endif
#ifndef __MBSCONVERTER_HH
# include "MbsConverter.hh"
#endif
#ifndef __MEDCONVERTER_HH
# include "MedConverter.hh"
#endif

// EventBuilder header
#ifndef __EVENTBUILDER_HH
# include "EventBuilder.hh"
#endif

// Histogrammer header
#ifndef __HISTOGRAMMER_HH
# include "Histogrammer.hh"
#endif

// DataSpy header
#ifndef __DATASPY_HH
# include "DataSpy.hh"
#endif

// MiniballGUI header
#ifndef __MINIBALLGUI_HH
# include "MiniballGUI.hh"
#endif

// MiniballAngleFit header
#ifndef __MINIBALLANGLEFITTER_HH
# include "MiniballAngleFitter.hh"
#endif

// Command line interface
#ifndef __COMMAND_LINE_INTERFACE_HH
# include "CommandLineInterface.hh"
#endif



// Default parameters and name
std::string output_name;
std::string datadir_name;
std::string name_set_file;
std::string name_cal_file;
std::string name_react_file;
std::string name_angle_file = "";
std::vector<std::string> input_names;

// a flag at the input to force the conversion
bool flag_convert = false;
bool flag_events = false;
bool flag_source = false;
bool flag_ebis = false;

// select what steps of the analysis to be forced
std::vector<bool> force_convert;
bool force_sort = false;
bool force_events = false;

// Flag for somebody needing help on command line
bool help_flag = false;

// Flag if we want to launch the GUI for sorting
bool gui_flag = false;

// Input data type
bool flag_midas = false;
bool flag_mbs = false;
bool flag_med = false;

// Do we want to fit the 22Ne angle data?
bool flag_angle_fit = false;

// DataSpy
bool flag_spy = false;
int open_spy_data = -1;

// Monitoring input file
bool flag_monitor = false;
int mon_time = -1; // update time in seconds

// Settings file
std::shared_ptr<MiniballSettings> myset;

// Calibration file
std::shared_ptr<MiniballCalibration> mycal;
bool overwrite_cal = false;

// Reaction file
std::shared_ptr<MiniballReaction> myreact;

// Struct for passing to the thread
typedef struct thptr {
	
	std::shared_ptr<MiniballCalibration> mycal;
	std::shared_ptr<MiniballSettings> myset;
	std::shared_ptr<MiniballReaction> myreact;
	
} thread_data;

// Server and controls for the GUI
std::unique_ptr<THttpServer> serv;
int port_num = 8030;

// Pointers to the thread events TODO: sort out inhereted class stuff
std::shared_ptr<MiniballConverter> conv_mon;
std::shared_ptr<MiniballMbsConverter> conv_mbs_mon;
std::shared_ptr<MiniballMidasConverter> conv_midas_mon;
std::shared_ptr<MiniballEventBuilder> eb_mon;
std::shared_ptr<MiniballHistogrammer> hist_mon;

void reset_conv_hists(){
	conv_mon->ResetHists();
}

void reset_evnt_hists(){
	eb_mon->ResetHists();
}

void reset_phys_hists(){
	hist_mon->ResetHists();
}

void stop_monitor(){
	bRunMon = kFALSE;
}

void start_monitor(){
	bRunMon = kTRUE;
}

// Function to call the monitoring loop
void* monitor_run( void* ptr ){
	
	// This doesn't make sense for MED data which is historical
	if( flag_med ) return 0;
	
	// Get the settings, file etc.
	thptr *calfiles = (thptr*)ptr;
	
	// Load macros in thread
	std::string rootline = ".L " + std::string(CUR_DIR) + "include/MonitorMacros.hh";
	gROOT->ProcessLine( rootline.data() );

	// This function is called to run when monitoring
	if( flag_mbs ){
		conv_mbs_mon = std::make_shared<MiniballMbsConverter>( calfiles->myset );
		conv_mon.reset( conv_mbs_mon.get() );
	}
	else if( flag_midas ) {
		conv_midas_mon = std::make_shared<MiniballMidasConverter>( calfiles->myset );
		conv_mon.reset( conv_midas_mon.get() );
	}
	eb_mon = std::make_shared<MiniballEventBuilder>( calfiles->myset );
	hist_mon = std::make_shared<MiniballHistogrammer>( calfiles->myreact, calfiles->myset );

	
	// Data blocks for Data spy
	if( flag_spy && ( myset->GetBlockSize() != 0x10000 && flag_midas ) ) {
	
		// only 64 kB supported atm
		std::cerr << "Currently only supporting 64 kB block size" << std::endl;
		exit(1);
	
	}
	
	// Daresbury MIDAS DataSpy
	DataSpy myspy;
	long long buffer[8*1024];
	int file_id = 0; ///> TapeServer volume = /dev/file/<id> ... <id> = 0 on issdaqpc2
	if( flag_spy && flag_midas ) myspy.Open( file_id ); /// open the data spy
	int spy_length = 0;
	
	// GSI MBS EventServer
	MBS mbs;
	if( flag_spy && flag_mbs ) mbs.OpenEventServer( "localhost", 8030 );

	// Data/Event counters
	int start_block = 0, start_subevt = 0;
	int nblocks = 0, nsubevts = 0;
	unsigned long nbuild = 0;

	// Converter setup
	if( !flag_spy ) curFileMon = input_names.at(0); // maybe change in GUI later?
	if( flag_source ) conv_mon->SourceOnly();
	if( flag_ebis ) conv_mon->EBISOnly();
	conv_mon->AddCalibration( calfiles->mycal );
	conv_mon->SetOutput( "monitor_singles.root" );
	conv_mon->MakeTree();
	conv_mon->MakeHists();

	// Update server settings
	// title of web page
	std::string toptitle;
	if( !flag_spy ) toptitle = curFileMon.substr( curFileMon.find_last_of("/")+1,
							curFileMon.length()-curFileMon.find_last_of("/")-1 );
	else toptitle = "DataSpy ";
	toptitle += " (" + std::to_string( mon_time ) + " s)";
	serv->SetItemField("/", "_toptitle", toptitle.data() );

	// While the sort is running
	while( true ) {
		
		// bRunMon can be set by the GUI
		while( bRunMon ) {
			
			// Convert - from MIDAS file
			if( !flag_spy && flag_midas ) {
				
				nblocks = conv_midas_mon->ConvertFile( curFileMon, start_block );
				start_block = nblocks;

			}

			// Convert - from MBS file
			else if( !flag_spy && flag_mbs ) {
				
				nsubevts = conv_mbs_mon->ConvertFile( curFileMon, start_subevt );
				start_subevt = nsubevts;
				
			}
			
			// Convert - from MIDAS shared memory
			else if( flag_spy && flag_midas ){
			
				// Clean up the trees before we start
				conv_midas_mon->GetSortedTree()->Reset();
				conv_midas_mon->GetMbsInfo()->Reset();

				// Empty the previous data vector and reset counters
				conv_midas_mon->StartFile();

				// First check if we have data
				std::cout << "Looking for data from DataSpy" << std::endl;
				spy_length = myspy.Read( file_id, (char*)buffer, calfiles->myset->GetBlockSize() );
				if( spy_length == 0 && bFirstRun ) {
					  std::cout << "No data yet on first pass" << std::endl;
					  gSystem->Sleep( 2e3 );
					  continue;
				}

				// Keep reading until we have all the data
				// This could be multi-threaded to process data and go back to read more
				int block_ctr = 0;
				long byte_ctr = 0;
				int poll_ctr = 0;
				while( block_ctr < 200 && poll_ctr < 1000 ){
				
					//std::cout << "Got some data from DataSpy" << std::endl;
					if( spy_length > 0 ) {
						nblocks = conv_midas_mon->ConvertBlock( (char*)buffer, 0 );
						block_ctr += nblocks;
					}

					// Read a new block
					gSystem->Sleep( 1 ); // wait 1 ms between each read
					spy_length = myspy.Read( file_id, (char*)buffer, calfiles->myset->GetBlockSize() );
					
					byte_ctr += spy_length;
					poll_ctr++;

				}
				
				std::cout << "Got " << byte_ctr << " bytes of data from DataSpy" << std::endl;

				// Sort the packets we just got, then do the rest of the analysis
				conv_midas_mon->SortTree();
				conv_midas_mon->PurgeOutput();

			}
											 
			// Convert - from MBS event server
			else if( flag_spy && flag_mbs ){
				
				// Empty the previous data vector and reset counters
				conv_mbs_mon->StartFile();

				// First check if we have data
				std::cout << "Looking for data from MBSEventServer" << std::endl;
				conv_mbs_mon->SetMBSEvent( mbs.GetNextEventFromStream() );
				conv_mbs_mon->ProcessBlock(0);
				conv_mbs_mon->SortTree();
				conv_mbs_mon->PurgeOutput();

			}


			// Only do the rest if it is not a source run
			if( !flag_source ) {
			
				// Event builder
				if( bFirstRun ) {
					eb_mon->SetOutput( "monitor_events.root" );
					eb_mon->StartFile();

				}
				// TODO: This could be done better with smart pointers
				TTree *sorted_tree = conv_mon->GetSortedTree()->CloneTree();
				TTree *mbsinfo_tree = conv_mon->GetMbsInfo()->CloneTree();
				eb_mon->SetInputTree( sorted_tree );
				eb_mon->SetMBSInfoTree( mbsinfo_tree );
				eb_mon->GetTree()->Reset();
				nbuild = eb_mon->BuildEvents();
				eb_mon->PurgeOutput();
				delete sorted_tree;
				delete mbsinfo_tree;

				// Histogrammer
				if( bFirstRun ) {
					hist_mon->SetOutput( "monitor_hists.root" );
				}
				if( nbuild ) {
					// TODO: This could be done better with smart pointers
					TTree *evt_tree = eb_mon->GetTree()->CloneTree();
					hist_mon->SetInputTree( evt_tree );
					hist_mon->FillHists();
					hist_mon->PurgeOutput();
					delete evt_tree;
				}
				
				// If this was the first time we ran, do stuff?
				if( bFirstRun ) {
					
					bFirstRun = kFALSE;
					
				}
			
			}
			
			// This makes things unresponsive!
			// Unless we are threading?
			gSystem->Sleep( mon_time * 1e3 );

		} // bRunMon
		
	} // always running
	
	// Close the dataSpy before exiting (no point really)
	if( flag_spy && flag_midas ) myspy.Close( file_id );
	if( flag_spy && flag_mbs ) mbs.CloseEventServer();

	// Close all outputs (we never reach here anyway)
	conv_mon->CloseOutput();
	eb_mon->CloseOutput();
	hist_mon->CloseOutput();

	return 0;
	
}

//void* start_http( void* ptr ){
void start_http(){

	// Server for JSROOT
	std::string server_name = "http:" + std::to_string(port_num) + "?top=MiniballDAQMonitoring";
	serv = std::make_unique<THttpServer>( server_name.data() );
	serv->SetReadOnly(kFALSE);

	// enable monitoring and
	// specify items to draw when page is opened
	serv->SetItemField("/","_monitoring","5000");
	//serv->SetItemField("/","_layout","grid2x2");
	//serv->SetItemField("/","_drawitem","[hpxpy,hpx,Debug]");
	serv->SetItemField("/","drawopt","[colz,hist]");
	
	// register simple start/stop commands
	serv->RegisterCommand("/Start", "StartMonitor()");
	serv->RegisterCommand("/Stop", "StopMonitor()");
	serv->RegisterCommand("/ResetSingles", "ResetConv()");
	serv->RegisterCommand("/ResetEvents", "ResetEvnt()");
	serv->RegisterCommand("/ResetHists", "ResetHist()");

	// hide commands so the only show as buttons
	//serv->Hide("/Start");
	//serv->Hide("/Stop");
	//serv->Hide("/Reset");

	return;
	
}

void do_convert() {
	
	//------------------------//
	// Run conversion to ROOT //
	//------------------------//
	// TODO: Find a better way to have a converter object without creating everything thrice
	MiniballMidasConverter conv_midas( myset );
	MiniballMbsConverter conv_mbs( myset );
	MiniballMedConverter conv_med( myset );
	std::cout << "\n +++ Miniball Analysis:: processing MiniballConverter +++" << std::endl;

	TFile *rtest;
	std::ifstream ftest;
	std::string name_input_file;
	std::string name_output_file;

	// Check each file
	for( unsigned int i = 0; i < input_names.size(); i++ ){

		name_input_file = input_names.at(i).substr( input_names.at(i).find_last_of("/")+1,
								input_names.at(i).length() - input_names.at(i).find_last_of("/")-1 );
		name_input_file = name_input_file.substr( 0, name_input_file.find_last_of(".") );

		if( flag_source ) name_output_file = name_input_file + "_source.root";
		else name_output_file = name_input_file + ".root";
		
		name_output_file = datadir_name + "/" + name_output_file;
		name_input_file = input_names.at(i);

		force_convert.push_back( false );

		// If input doesn't exist, skip it
		ftest.open( name_input_file.data() );
		if( !ftest.is_open() ) {

			std::cerr << name_input_file << " does not exist" << std::endl;
			continue;

		}
		else ftest.close();

		// If output doesn't exist, we have to convert it anyway
		// The convert flag will force it to be converted
		ftest.open( name_output_file.data() );
		if( !ftest.is_open() ) force_convert.at(i) = true;
		else {

			ftest.close();
			rtest = new TFile( name_output_file.data() );
			if( rtest->IsZombie() ) force_convert.at(i) = true;
			if( rtest->TestBit(TFile::kRecovered) ){
				std::cout << name_output_file << " possibly corrupted, reconverting" << std::endl;
				force_convert.at(i) = true;
			}
			if( !flag_convert && !force_convert.at(i) )
				std::cout << name_output_file << " already converted" << std::endl;
			rtest->Close();

		}

		if( flag_convert || force_convert.at(i) ) {
			
			std::cout << name_input_file << " --> ";
			std::cout << name_output_file << std::endl;
			
			if( flag_mbs ) {

				if( flag_source ) conv_mbs.SourceOnly();
				if( flag_ebis ) conv_mbs.EBISOnly();
				conv_mbs.SetOutput( name_output_file );
				conv_mbs.AddCalibration( mycal );
				conv_mbs.MakeTree();
				conv_mbs.MakeHists();
				conv_mbs.ConvertFile( name_input_file );

				// Sort the tree before writing and closing
				if( !flag_source ){
					conv_mbs.BuildMbsIndex();
					if( myset->GetMbsEventMode() )
						conv_mbs.SortTree(false);
					else conv_mbs.SortTree();
				}
				conv_mbs.CloseOutput();
				
			}
			
			else if( flag_midas ) {
				
				if( flag_source ) conv_midas.SourceOnly();
				if( flag_ebis ) conv_midas.EBISOnly();
				conv_midas.SetOutput( name_output_file );
				conv_midas.AddCalibration( mycal );
				conv_midas.MakeTree();
				conv_midas.MakeHists();
				conv_midas.ConvertFile( name_input_file );

				// Sort the tree before writing and closing
				if( !flag_source ) conv_midas.SortTree();
				conv_midas.CloseOutput();
				
			}
			
			else if( flag_med ){
				
				if( flag_source ) conv_med.SourceOnly();
				if( flag_ebis ) conv_med.EBISOnly();
				conv_med.SetOutput( name_output_file );
				conv_med.AddCalibration( mycal );
				conv_med.MakeTree();
				conv_med.MakeHists();
				conv_med.ConvertFile( name_input_file );

				// Sort the tree before writing and closing
				if( !flag_source ){
					conv_med.BuildMbsIndex();
					if( myset->GetMbsEventMode() )
						conv_med.SortTree(false);
					else conv_med.SortTree();
				}
				conv_med.CloseOutput();

			}

		}

	}

	return;

}

bool do_build() {
	
	//-----------------------//
	// Physics event builder //
	//-----------------------//
	MiniballEventBuilder eb( myset );
	std::cout << "\n +++ Miniball Analysis:: processing MiniballEventBuilder +++" << std::endl;

	TFile *rtest;
	std::ifstream ftest;
	std::string name_input_file;
	std::string name_output_file;
	bool return_flag = false;

	// Update calibration file if given
	if( overwrite_cal ) eb.AddCalibration( mycal );

	// Do event builder for each file individually
	for( unsigned int i = 0; i < input_names.size(); i++ ){

		name_input_file = input_names.at(i).substr( input_names.at(i).find_last_of("/")+1,
												   input_names.at(i).length() - input_names.at(i).find_last_of("/")-1 );
		name_input_file = name_input_file.substr( 0, name_input_file.find_last_of(".") );
		
		name_output_file = datadir_name + "/" + name_input_file + "_events.root";
		name_input_file = datadir_name + "/" + name_input_file + ".root";

		// If input doesn't exist, skip it
		ftest.open( name_input_file.data() );
		if( !ftest.is_open() ) {

			std::cerr << name_input_file << " does not exist" << std::endl;
			continue;

		}
		else {
			
			ftest.close();
			return_flag = true;
			
		}
		
		// We need to do event builder if we just converted it
		// specific request to do new event build with -e
		// this is useful if you need to add a new calibration
		if( flag_convert || force_convert.at(i) || flag_events )
			force_events = true;

		// If it doesn't exist, we have to sort it anyway
		else {

			ftest.open( name_output_file.data() );
			if( !ftest.is_open() ) force_events = true;
			else {

				ftest.close();
				rtest = new TFile( name_output_file.data() );
				if( rtest->IsZombie() ) force_events = true;
				if( rtest->TestBit(TFile::kRecovered) ){
					std::cout << name_output_file << " possibly corrupted, rebuilding" << std::endl;
					force_events = true;
				}
				if( !force_events )
					std::cout << name_output_file << " already built" << std::endl;
				rtest->Close();

			}

		}

		if( force_events ) {

			std::cout << name_input_file << " --> ";
			std::cout << name_output_file << std::endl;

			eb.SetInputFile( name_input_file );
			eb.SetOutput( name_output_file );
			eb.BuildEvents();
			eb.CloseOutput();

			force_events = false;

		}

	}

	return return_flag;
	
}

void do_hist() {
	
	//------------------------------//
	// Finally make some histograms //
	//------------------------------//
	MiniballHistogrammer hist( myreact, myset );
	std::cout << "\n +++ Miniball Analysis:: processing MiniballHistogrammer +++" << std::endl;

	std::ifstream ftest;
	std::string name_input_file;

	std::vector<std::string> name_hist_files;

	// We are going to chain all the event files now
	for( unsigned int i = 0; i < input_names.size(); i++ ){

		name_input_file = input_names.at(i).substr( input_names.at(i).find_last_of("/")+1,
												   input_names.at(i).length() - input_names.at(i).find_last_of("/")-1 );
		name_input_file = name_input_file.substr( 0,
												 name_input_file.find_last_of(".") );
		name_input_file = datadir_name + "/" + name_input_file + "_events.root";

		ftest.open( name_input_file.data() );
		if( !ftest.is_open() ) {
			
			std::cerr << name_input_file << " does not exist" << std::endl;
			continue;
			
		}
		else ftest.close();

		name_hist_files.push_back( name_input_file );

	}

	// Only do something if there are valid files
	if( name_hist_files.size() ) {
		
		hist.SetOutput( output_name );
		hist.SetInputFile( name_hist_files );
		hist.FillHists();
		hist.CloseOutput();
	
	}
	
	return;
	
}

void do_angle_fit(){

	//------------------------------------------//
	// Run angle fitting routine with 22Ne data //
	//------------------------------------------//
	MiniballAngleFitter angle_fit( myset, myreact );
	std::cout << "\n +++ Miniball Analysis:: processing MiniballAngleFitter +++" << std::endl;

	TFile *rtest;
	std::ifstream ftest;
	std::string name_input_file;
	std::string name_output_file = "22Ne_angle_fit.root";
	std::string hadd_file_list = "";
	std::string name_results_file = "22Ne_angle_fit.cal";
	
	// Check each file
	for( unsigned int i = 0; i < input_names.size(); i++ ){
	
		name_input_file = input_names.at(i).substr( input_names.at(i).find_last_of("/")+1,
												   input_names.at(i).length() - input_names.at(i).find_last_of("/")-1 );
		name_input_file = name_input_file.substr( 0,
												 name_input_file.find_last_of(".") );
		name_input_file = datadir_name + "/" + name_input_file + "_events.root";
	
		// Add to list if the converted file exists
		ftest.open( name_input_file.data() );
		if( ftest.is_open() ) {
	
			ftest.close();
			rtest = new TFile( name_input_file.data() );
			if( !rtest->IsZombie() ) {
				hadd_file_list += " " + name_input_file;
			}
			else {
				std::cout << "Skipping " << name_input_file;
				std::cout << ", it's broken" << std::endl;
			}
			rtest->Close();
	
		}
	
		else {
	
			std::cout << "Skipping " << name_input_file;
			std::cout << ", file does not exist" << std::endl;
	
		}
	
	}
	
	// If we have some ROOT files, add them and pass to the fitter
	if( input_names.size() ){

		// Perform the hadd (doesn't work on Windows)
		gErrorIgnoreLevel = kError;
		std::string cmd = "hadd -k -T -v 0 -f ";
		cmd += name_output_file;
		cmd += hadd_file_list;
		gSystem->Exec( cmd.data() );
		gErrorIgnoreLevel = kInfo;

		// Give this file to the angle fitter
		if( !angle_fit.SetInputROOTFile( name_output_file ) ) return;
		
	}
	
	// Otherwise we have to take the energies from the file
	else if( !angle_fit.SetInputEnergiesFile( name_angle_file ) ) return;
	
	// Perform the fitting
	angle_fit.DoFit();

	// Save the experimental energies and angles to a file
	if( input_names.size() )
		angle_fit.SaveExpEnergies( "22Ne_fitted_energies.dat" );
	angle_fit.SaveReactionFile( name_results_file );
	
	// Close the ROOT file
	angle_fit.CloseROOTFile();
	
}

int main( int argc, char *argv[] ){
	
	// Command line interface, stolen from MiniballCoulexSort
	std::unique_ptr<CommandLineInterface> interface = std::make_unique<CommandLineInterface>();

	interface->Add("-i", "List of input files", &input_names );
	interface->Add("-o", "Output file for histogram file", &output_name );
	interface->Add("-s", "Settings file", &name_set_file );
	interface->Add("-c", "Calibration file", &name_cal_file );
	interface->Add("-r", "Reaction file", &name_react_file );
	interface->Add("-f", "Flag to force new ROOT conversion", &flag_convert );
	interface->Add("-e", "Flag to force new event builder (new calibration)", &flag_events );
	interface->Add("-source", "Flag to define an source only run", &flag_source );
	interface->Add("-ebis", "Flag to define an EBIS only run, discarding data >4ms after an EBIS event", &flag_ebis );
	interface->Add("-midas", "Flag to define input as MIDAS data type (FEBEX with Daresbury firmware - default)", &flag_midas );
	interface->Add("-mbs", "Flag to define input as MBS data type (FEBEX with GSI firmware)", &flag_mbs );
	interface->Add("-med", "Flag to define input as MED data type (DGF and MADC)", &flag_med );
	interface->Add("-anglefit", "Flag to run the angle fit", &flag_angle_fit );
	interface->Add("-angledata", "File containing 22Ne segment energies", &name_angle_file );
	interface->Add("-spy", "Flag to run the DataSpy", &flag_spy );
	interface->Add("-m", "Monitor input file every X seconds", &mon_time );
	interface->Add("-p", "Port number for web server (default 8030)", &port_num );
	interface->Add("-d", "Directory to put the sorted data default is /path/to/data/sorted", &datadir_name );
	interface->Add("-g", "Launch the GUI", &gui_flag );
	interface->Add("-h", "Print this help", &help_flag );

	interface->CheckFlags( argc, argv );
	if( help_flag ) {
		
		interface->CheckFlags( 1, argv );
		return 0;
		
	}
	
	// If we are launching the GUI
	if( gui_flag || argc == 1 ) {
		
		TApplication theApp( "App", &argc, argv );
		new MiniballGUI();
		theApp.Run();
		
		return 0;

	}

	// Check if we are doing the angle fit
	if( flag_angle_fit ) {
		
		if( input_names.size() == 0 && name_angle_file.length() > 0 )
			std::cout << "Angle fitting using energies from a file" << std::endl;
			
		else if( input_names.size() > 0 && name_angle_file.length() == 0 )
			std::cout << "Angle fitting using 22Ne data files, with automatic peak fitting" << std::endl;

		else {
			
			std::cout << "When fitting the 22Ne angle data, you must give segments energy file as input" << std::endl;
			std::cout << "using the -angledata flag. Alternatively, you can give the raw data files using" << std::endl;
			std::cout << "the -i flag and the peaks will be automatically fitted from the events file." << std::endl;
			return 1;
			
		}
		
	}
	
	// Check we have data files
	else if( !input_names.size() && !flag_spy ) {
			
		std::cout << "You have to provide at least one input file unless you are in DataSpy mode!" << std::endl;
		return 1;
			
	}
	
	// Check if it should be MIDAS, MBS or MED format
	if( !flag_midas && !flag_mbs && !flag_med && !flag_spy && !name_angle_file.length() ){

		std::string extension = input_names.at(0).substr( input_names.at(0).find_last_of(".")+1,
														 input_names.at(0).length()-input_names.at(0).find_last_of(".")-1 );
		
		if( extension == "lmd" ) {
			
			flag_mbs = true;
			std::cout << "Assuming we have MBS data because of the .lmd extension" << std::endl;
			std::cout << "Forcing the data block size to 32 kB" << std::endl;
			
		}
		
		else if( extension == "med" ) {
			
			flag_med = true;
			std::cout << "Assuming we have MED data because of the .med extension" << std::endl;
			std::cout << "Forcing the data block size to 32 kB" << std::endl;
			
		}
		
		else flag_midas = true;
		
	}
	
	// Check if we should be monitoring the input
	if( flag_spy ) {
		
		flag_monitor = true;
		if( mon_time < 0 ) mon_time = 30;
		std::cout << "Getting data from shared memory every " << mon_time;
		std::cout << " seconds using DataSpy" << std::endl;
		
	}
	
	else if( mon_time >= 0 && input_names.size() == 1 && !flag_angle_fit ) {
		
		flag_monitor = true;
		std::cout << "Running sort in a loop every " << mon_time;
		std::cout << " seconds\nMonitoring " << input_names.at(0) << std::endl;
		
	}
	
	else if( mon_time >= 0 && input_names.size() != 1 ) {
		
		flag_monitor = false;
		std::cout << "Cannot monitor multiple input files, switching to normal mode" << std::endl;
		
	}
	
	// Check data type for spy
	if( flag_spy && !flag_midas && !flag_mbs && !flag_med ){
		
		std::cout << "Assuming MIDAS data for spy" << std::endl;
		flag_midas = true;
		
	}
	
	// Check the directory we are writing to
	if( datadir_name.length() == 0 ) {
		
		if( bool( input_names.size() ) ) {
			
			// Probably in the current working directory
			if( input_names.at(0).find("/") == std::string::npos )
				datadir_name = "./sorted";
			
			// Called from a different directory
			else {
				
				datadir_name = input_names.at(0).substr( 0,
														input_names.at(0).find_last_of("/") );
				datadir_name += "/sorted";
				
			}
			
		}
		
		else if( flag_spy ) datadir_name = "dataspy";
		else if( flag_angle_fit ) datadir_name = "positions";
		else datadir_name = "mb_sort_outputs";
				
	}
	
	// Create the directory if it doesn't exist (not Windows compliant)
	std::string cmd = "mkdir -p " + datadir_name;
	gSystem->Exec( cmd.data() );	
	std::cout << "Sorted data files being saved to " << datadir_name << std::endl;

	
	// Check the ouput file name
	if( output_name.length() == 0 ) {

		if( bool( input_names.size() ) ) {

			std::string name_input_file = input_names.at(0).substr( input_names.at(0).find_last_of("/")+1,
													   input_names.at(0).length() - input_names.at(0).find_last_of("/")-1 );
			name_input_file = name_input_file.substr( 0,
													 name_input_file.find_last_of(".") );
			
			if( flag_angle_fit ) {
				
				output_name = datadir_name + "/" + name_input_file + "_results.root";

			}
			
			else if( input_names.size() > 1 ) {
				
				output_name = datadir_name + "/" + name_input_file + "_hists_";
				output_name += std::to_string(input_names.size()) + "_subruns.root";
			
			}
			
			else
				output_name = datadir_name + "/" + name_input_file + "_hists.root";
				
		}
		
		else output_name = datadir_name + "/monitor_hists.root";

	}

	// Check we have a Settings file
	if( name_set_file.length() > 0 ) {
		
		// Test if the file exists
		std::ifstream ftest;
		ftest.open( name_set_file.data() );
		if( !ftest.is_open() ) {
			
			std::cout << name_set_file << " does not exist.";
			std::cout << " Using defaults" << std::endl;
			name_set_file = "dummy";

		}
		
		else {
		
			ftest.close();
			std::cout << "Settings file: " << name_set_file << std::endl;
		
		}

	}
	else {
		
		std::cout << "No settings file provided. Using defaults." << std::endl;
		name_set_file = "dummy";

	}
	
	// Check we have a calibration file
	if( name_cal_file.length() > 0 ) {
		
		// Test if the file exists
		std::ifstream ftest;
		ftest.open( name_cal_file.data() );
		if( !ftest.is_open() ) {
			
			std::cout << name_cal_file << " does not exist.";
			std::cout << " Using defaults" << std::endl;
			name_cal_file = "dummy";

		}
		
		else {
			
			ftest.close();
			std::cout << "Calibration file: " << name_cal_file << std::endl;
			overwrite_cal = true;
			
		}

	}
	else {
		
		std::cout << "No calibration file provided. Using defaults." << std::endl;
		name_cal_file = "dummy";

	}
	
	// Check we have a reaction file
	if( name_react_file.length() > 0 ) {
		
		// Test if the file exists
		std::ifstream ftest;
		ftest.open( name_react_file.data() );
		if( !ftest.is_open() ) {
			
			std::cout << name_react_file << " does not exist.";
			std::cout << " Using defaults" << std::endl;
			name_react_file = "dummy";

		}
		
		else {
		
			ftest.close();
			std::cout << "Reaction file: " << name_react_file << std::endl;

		}

	}
	else {
		
		std::cout << "No reaction file provided. Using defaults." << std::endl;
		name_react_file = "dummy";

	}
	
	myset = std::make_shared<MiniballSettings>( name_set_file );
	mycal = std::make_shared<MiniballCalibration>( name_cal_file, myset );
	if( flag_mbs || flag_med ) mycal->SetDefaultQint();
	mycal->ReadCalibration();
	myreact = std::make_shared<MiniballReaction>( name_react_file, myset );
	
	// Force data block size for MBS and MED data
	if( flag_mbs ) myset->SetBlockSize( 0x8000 );
	else if( flag_med ) myset->SetBlockSize( 0x4000 );
	
	// Turn of MBS event sorting for MIDAS and MED files
	if( flag_midas || flag_med ) myset->SetMbsEventMode(false);


	//-------------------//
	// Online monitoring //
	//-------------------//
	if( flag_monitor || flag_spy ) {
		
		// Don't support MBS data spy
		if( flag_mbs && flag_spy ){
			
			std::cout << "MBS data spy not yet supported" << std::endl;
			return 0;
			
		}
		
		// Don't support MED data spy (historical data)
		if( flag_med && flag_spy ){
			
			std::cout << "MED data spy not supported because data is historical" << std::endl;
			return 0;
			
		}
		
		// Make some data for the thread
		thread_data data;
		data.mycal = mycal;
		data.myset = myset;
		data.myreact = myreact;

		// Start the HTTP server from the main thread (should usually do this)
		start_http();
		gSystem->ProcessEvents();

		// Thread for the monitor process
		TThread *th0 = new TThread( "monitor", monitor_run, &data );
		th0->Run();

		// wait until we finish
		while( true ){
			
			gSystem->Sleep(10);
			gSystem->ProcessEvents();
			
		}
		
		return 0;
		
	}


	//------------------//
	// Run the analysis //
	//------------------//
	do_convert();
	if( flag_angle_fit ){
		do_build();
		do_angle_fit();
	}
	else if( !flag_source ) {
		if( do_build() )
			do_hist();
	}

	std::cout << "\n\nFinished!\n";
			
	return 0;
	
}
