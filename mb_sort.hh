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


#include "MiniballGUI.hh"

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
