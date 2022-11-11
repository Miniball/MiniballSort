#include <iostream>

#include "TCanvas.h"
#include "TGraph.h"
#include "TGraphErrors.h"
#include "TF1.h"
#include "Math/Functor.h"
#include "Math/Factory.h"
#include "Math/Minimizer.h"
#include "Math/MinimizerOptions.h"

#include "Settings.hh"
#include "Calibration.hh"
#include "MbsConverter.hh"
#include "EventBuilder.hh"
#include "Reaction.hh"
#include "Histogrammer.hh"
#include "DataSpy.hh"
#include "MiniballGUI.hh"

class FitFunc {
public:
  int present[8][3][7];
  double energy[8][3][7];
  double err[8][3][7];
  int cluster[8];
  std::shared_ptr<MiniballSettings> myset;
  std::shared_ptr<MiniballCalibration> mycal;
  std::shared_ptr<MiniballReaction> myreact;
  double user_z = 0;
  double eref = 440.2;

  FitFunc() {
    for (int clu = 0; clu < 8; ++clu) {
      cluster[clu] = 0;
      for (int cry = 0; cry < 3; ++cry) {
        for (int seg = 0; seg < 7; ++seg) {
          present[clu][cry][seg] = 0;
          energy[clu][cry][seg] = 0;
          err[clu][cry][seg] = 0;
        }
      }
    }
  }

  FitFunc(std::string settings_file, std::string reaction_file) {
    for (int clu = 0; clu < 8; ++clu) {
      cluster[clu] = 0;
      for (int cry = 0; cry < 3; ++cry) {
        for (int seg = 0; seg < 7; ++seg) {
          present[clu][cry][seg] = 0;
          energy[clu][cry][seg] = 0;
          err[clu][cry][seg] = 0;
        }
      }
    }

    myset = std::make_shared<MiniballSettings>(settings_file);
    myreact = std::make_shared<MiniballReaction>(reaction_file, myset);
  }

  void LoadExpEnergies(std::string energy_file) {
    std::ifstream energyfile(energy_file);
    int cl,cr,sg;
    double en,er;
    while (energyfile >> cl >> cr >> sg >> en >> er) {
      present[cl][cr][sg] = 1;
      energy[cl][cr][sg] = en;
      err[cl][cr][sg] = er;
      cluster[cl] = 1;
    }
    energyfile.close();
  }
  
  double operator() (const double *p) {
    //change angles
    //p[0] = beta
    //p[1] = theta1, p[2] = phi1, p[3] = alpha1, p[4] = r1, 
    //p[5] = theta2, p[6] = phi2, p[7] = alpha2, p[8] = r2, etc.    
    int indx = 1;    
    for (int clu=0; clu<8; ++clu) {
      if (cluster[clu] == 0) { continue; }
      myreact->SetupCluster(clu, p[indx], p[indx+1], p[indx+2], p[indx+3], user_z);
      indx += 4;
    }
    
    double chisq = 0;

    for (int clu = 0; clu < 8; ++clu) {
      for (int cry = 0; cry < 3; ++cry) {
        for (int seg = 0; seg < 7; ++seg) {
          if (present[clu][cry][seg] == 0) { continue; }
          double theta = myreact->GetGammaTheta(clu ,cry, seg);
          double beta = p[0];
          double corr = 1. -  beta * std::cos(theta);
          corr *= 1./(std::sqrt(1.-std::pow(beta, 2.)));
          double edop = eref/corr;
          chisq += std::pow((energy[clu][cry][seg] - edop)/err[clu][cry][seg], 2);
        }
      }
    }
    return chisq;
  }
  
};


int main(int argc, const char **argv) {
  if (argc < 4) {
    std::cout << "useage: mb_angle_fit <settings_file> <reaction_file> <centroids_file>" << std::endl << std::endl;
    std::cout << "where centroids_file contains the un-doppler corrected centroids for cores gated on each segment" << std::endl;
    std::cout << "for some reference transition. The format for centroids_file is" << std::endl << std::endl;
    std::cout << "cluster   crystal   segment    energy    uncertainty" << std::endl << std::endl;
    std::cout << "beta (v/c), as well as the theta, phi, alpha, and R values for each detector present are varied." << std::endl;
    std::cout << "The reference energy is set at 440.2 keV, this can be varied but will need to be recompiled." << std::endl;
    return -1;
  }
  FitFunc ff(argv[1], argv[2]);
  ff.LoadExpEnergies(argv[3]);
  ff.eref = 440.2;

  //set up parameters, bounds
  std::vector<double> pars(1+7*4);
  std::vector<std::string> names(1+7*4);
  std::vector<double> LL(1+7*4);
  std::vector<double> UL(1+7*4);

  pars[0] = ff.myreact->GetBeta();
  names[0] = "beta";
  LL[0] = 0.01;
  UL[0] = 0.5;
  //set initial guesses
  TEnv *config = new TEnv( argv[2] );  
  int indx = 1;
  int nClusters = 0;
  for (int clu=0; clu<8; ++clu) {
    //skip clusters for which we have no data
    if (ff.cluster[clu] == 0) { continue; }
    ++nClusters;
    //get starting guesses from the reaction file
	  pars[indx] = config->GetValue( Form( "MiniballCluster_%d.Theta", clu ), 0. );
	  pars[indx+1] 	= config->GetValue( Form( "MiniballCluster_%d.Phi", clu ), 0. );    
	  pars[indx+2] = config->GetValue( Form( "MiniballCluster_%d.Alpha", clu ), 0. );
	  pars[indx+3]	 	= config->GetValue( Form( "MiniballCluster_%d.R", clu ), 0. );

    names[indx] = "theta"+std::to_string(clu);
    names[indx+1] = "phi"+std::to_string(clu);
    names[indx+2] = "alpha"+std::to_string(clu);
    names[indx+3] = "r"+std::to_string(clu);

    //these may cause issue if we need to cross over the 0/360 boundary
    LL[indx] = 0.0;
    LL[indx+1] = 0.0;
    LL[indx+2] = 0.0;
    LL[indx+3] = 10.0;

    UL[indx] = 180.0;
    UL[indx+1] = 360.0;
    UL[indx+2] = 360.0;
    UL[indx+3] = 400.0;
    
    indx += 4;
  }

  //create minimizer
  ROOT::Math::Minimizer *min =
    ROOT::Math::Factory::CreateMinimizer("Minuit2", "Migrad");

  ROOT::Math::Functor f_init(ff,4*nClusters+1);

  min->SetErrorDef(1.);
  min->SetMaxFunctionCalls(1000);
  min->SetMaxIterations(1000);
  min->SetTolerance(0.001);
  min->SetPrecision(1e-6);
  min->SetFunction(f_init);

  for (int i=0; i<pars.size(); ++i) {
    min->SetLimitedVariable(i, names.at(i), pars.at(i), 0.0001, LL.at(i), UL.at(i));
    min->SetVariableStepSize(i, 1.0);
  }
  min->SetVariableStepSize(0, 0.001);
    
  min->Minimize();

  min->PrintResults();
  //minimization finished

  //copy results into reaction object
  indx = 1;
  for (int clu = 0; clu < 8 ; ++clu) {
    if (ff.cluster[clu] == 0) { continue; }
    ff.myreact->SetupCluster(clu, min->X()[indx], min->X()[indx+1], min->X()[indx+2], min->X()[indx+3], 0.0);
    indx += 4;
  }

  //print fit + residuals to pdf
  TGraphErrors *engraph = new TGraphErrors();
  TGraph *calcgraph = new TGraph();
  TGraphErrors *resgraph = new TGraphErrors();

  engraph->GetXaxis()->SetTitle("Channel index");
  engraph->GetYaxis()->SetTitle("Energy [keV]");

  resgraph->GetXaxis()->SetTitle("Channel index");
  resgraph->GetYaxis()->SetTitle("Energy Residual [keV]");

  indx = 0;
  for (int clu = 0; clu < 8; ++clu) {
    for (int cry = 0; cry < 3; ++cry) {
      for (int seg = 0; seg < 7; ++seg) {
        ++indx;
        if (ff.present[clu][cry][seg] == 0) { continue; }
        double beta = min->X()[0];
        double theta = ff.myreact->GetGammaTheta(clu ,cry, seg);
        double corr = 1. -  beta * std::cos(theta);
        corr *= 1./(std::sqrt(1.-std::pow(beta, 2.)));
        double edop = ff.eref/corr;

        engraph->AddPoint(indx, ff.energy[clu][cry][seg]);
        engraph->SetPointError(engraph->GetN()-1, 0, ff.err[clu][cry][seg]);
        calcgraph->AddPoint(indx, edop);
        resgraph->AddPoint(indx, edop - ff.energy[clu][cry][seg]);
        resgraph->SetPointError(resgraph->GetN()-1, 0, ff.err[clu][cry][seg]);
      }
    }
  }

  TCanvas *c1 = new TCanvas();
  engraph->SetMarkerStyle(kFullCircle);
  engraph->SetMarkerSize(0.5);
  engraph->SetMarkerColor(kRed);
  engraph->SetLineColor(kRed);
  engraph->Draw("AP");
  calcgraph->SetMarkerStyle(kFullCircle);
  calcgraph->SetMarkerSize(0.5);
  calcgraph->Draw("P");

  c1->Print("position_cal.pdf(");

  resgraph->SetMarkerStyle(kFullCircle);
  resgraph->SetMarkerSize(0.5);
  resgraph->Draw("AP");

  c1->Print("position_cal.pdf)");

  //print final results to terminal
  std::cout << "fitted beta = " << min->X()[0] << std::endl;
  indx = 1;
  printf("       theta       phi     alpha         R\n");
  for (int clu = 0; clu < 8 ; ++clu) {
    if (ff.cluster[clu] == 0) { continue; }
    printf("Cl%i   %6.2f    %6.2f    %6.2f    %6.2f\n", clu, min->X()[indx], min->X()[indx+1], min->X()[indx+2], min->X()[indx+3]);
    indx += 4;
  }
  std::cout << std::endl << std::endl;
  indx = 1;
  for (int clu = 0; clu < 8 ; ++clu) {
    if (ff.cluster[clu] == 0) { continue; }
    printf("MiniballCluster_%d.Theta       %6.2f\n", clu, min->X()[indx]);
    printf("MiniballCluster_%d.Phi         %6.2f\n", clu, min->X()[indx+1]);
    printf("MiniballCluster_%d.Alpha       %6.2f\n", clu, min->X()[indx+2]);
    printf("MiniballCluster_%d.R           %6.2f\n", clu, min->X()[indx+3]);
    indx += 4;
  }
  
}

