#include "mb_sort.hh"
#include <iostream>

int MonitorReset(){
	reset_hists();
	std::cout << "Reset histograms" << std::endl;
	return 0;
}

int StopMonitor(){
	stop_monitor();
	std::cout << "Stop monitoring" << std::endl;
	return 0;
}

int StartMonitor(){
	start_monitor();
	std::cout << "Start monitoring" << std::endl;
	return 0;
}

