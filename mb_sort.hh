#ifndef __MBSORT_HH
# define __MBSORT_HH

// ROOT include.
#include <TTree.h>
#include <TFile.h>
#include <THttpServer.h>
#include <TThread.h>
#include <TGClient.h>
#include <TApplication.h>

// C++ include.
#include <iostream>
#include <stdio.h>
#include <string>
#include <vector>
#include <sstream>
#include <memory>

// Some compiler things
#ifndef CURDIR
# define CURDIR "./"
#endif
#ifndef AMEDIR
# define AMEDIR "./data"
#endif
#ifndef SRIMDIR
# define SRIMDIR "./srim"
#endif

Bool_t bRunMon = kTRUE;
Bool_t bFirstRun = kTRUE;
std::string curFileMon;


void reset_conv_hists();
void reset_evnt_hists();
void reset_phys_hists();
void stop_monitor();
void start_monitor();

#endif
