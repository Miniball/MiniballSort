#include <fstream>
#include <iostream>
#include "TH1.h"
#include "TFile.h"
#include <string>
#include <sstream>
using namespace std;

double solid_angle_func( double *x, double *par ) {
	
	double strip = x[0];
	double dist = par[1];
	
	double Rb = 9.05 + ( 15. - x[0] ) * 2.;
	double Ra = Rb + 1.9;
	
	double Rda = Ra*Ra + dist*dist;
	double Rdb = Rb*Rb + dist*dist;
	
	double phi_coverage = -0.0044 * TMath::Power(strip,3);
	phi_coverage +=  0.0451 * TMath::Power(strip,2);
	phi_coverage += -0.3646 * strip;
	phi_coverage += 78.2188; // parametrization from Konstantin Stoychev
	phi_coverage /= 360.0;
	
	double solid_angle =  ( Rda - dist*TMath::Power( Rda, 0.5 ) ) / Rda;
	solid_angle -= ( Rdb - dist*TMath::Power( Rdb, 0.5 ) ) / Rdb;
	solid_angle *= 2. * TMath::Pi();
	solid_angle *= phi_coverage;
	
	return solid_angle;
	
}

double intensity_func( double *x, double *par ) {
	
	return par[0] * solid_angle_func(x,par);
	
}

void fit_CD_distance( std::string filename ) {
	
	TFile *fin = new TFile( filename.data() );
	TH1F *hist;
	TGraphErrors *data_points[4];
	TF1 *alpha_dist[4];
	TF1 *alpha_dist_upp_err[4];
	TF1 *alpha_dist_low_err[4];
	TF1 *alpha_dist_chi_square[4];
	TGraphErrors *chi_plot[4];
	//  double sum
	char cname[200];
	TCanvas *c1 = new TCanvas("c1","CD distance using solid angle coverage and the ^{226}Ra alpha source",1200,900);
	c1->cd();
	TPad* ptop = new TPad("ptop", "ptop", 0, 0.3, 1.0, 1);
	ptop->Draw();
	ptop->cd();
	ptop->Divide(4, 2);
	vector<double> xasym;
	vector<double> yasym;
	vector<double> duxasym;
	vector<double> duyasym;
	vector<double> dlxasym;
	vector<double> dlyasym;
	
	// Loop over quadrants
	for( unsigned int i = 0; i < 4; i++ ){
		
		xasym.push_back(i);
		duxasym.push_back(0);
		dlxasym.push_back(0);
		
		vector<double> x1, y1, xerr1, yerr1;
		
		// Loop over strips
		for( unsigned int j = 0; j < 16; j++ ){
			
			std::string histname = "/sfp_1/board_" + std::to_string(i*2);
			histname += "/febex_1_" + std::to_string(i*2) + "_" + std::to_string(j);
			histname += "_cal";
			hist = (TH1F*)fin->Get( histname.data() );
			
			int xbinlow = hist->FindBin(2000);
			int xbinupp = hist->FindBin(8000);
			double integral = hist->Integral( xbinlow, xbinupp );
			if( integral < 10 ) continue;
			
			x1.push_back( j );
			y1.push_back( integral );
			xerr1.push_back( 0 );
			yerr1.push_back( TMath::Sqrt( integral ) );
			
		}
		
		data_points[i] = new TGraphErrors(x1.size(),x1.data(),y1.data(),xerr1.data(),yerr1.data());
		sprintf(cname, "alpha_dist_Q%d", i);
		alpha_dist[i] = new TF1( cname, intensity_func, -0.5, 15.5, 2 );
		sprintf(cname, "alpha_dist_upp_err_Q%d", i);
		alpha_dist_upp_err[i] = new TF1( cname, intensity_func, -0.5, 15.5, 2 );
		sprintf(cname, "alpha_dist_low_err_Q%d", i);
		alpha_dist_low_err[i] = new TF1( cname, intensity_func, -0.5, 15.5, 2 );
		alpha_dist[i]->SetParameters( 2000, 30.5 );
		data_points[i]->Fit( alpha_dist[i] );
		alpha_dist_upp_err[i]->SetParameter( 0, alpha_dist[i]->GetParameter(0));
		alpha_dist_upp_err[i]->FixParameter( 1, alpha_dist[i]->GetParameter(1) + 0.5);
		alpha_dist_low_err[i]->SetParameter( 0, alpha_dist[i]->GetParameter(0));
		alpha_dist_low_err[i]->FixParameter( 1, alpha_dist[i]->GetParameter(1) - 0.5);
		
		ptop->cd(i+1);
		data_points[i]->Draw("AP");
		alpha_dist[i]->Draw("same");
		alpha_dist_upp_err[i]->SetLineColor(6);
		alpha_dist_upp_err[i]->Draw("same");
		alpha_dist_low_err[i]->SetLineColor(kGreen);
		alpha_dist_low_err[i]->Draw("same");
		
		ptop->cd(i+5);
		//chi-square plot
		int n_points = x1.size();
		double chi_2_nu;
		double distance;
		double chi2plus1dist[2]; // left and right distances at which the reduced chi2 is +1 from the minimum
		int index = 0;
		sprintf(cname, "alpha_dist_chi_square_Q%d", i);
		alpha_dist_chi_square[i] = new TF1( cname , intensity_func, -0.5, 15.5, 2 );
		
		alpha_dist_chi_square[i]->SetParameter( 0 , alpha_dist[i]->GetParameter(0) );
		chi_plot[i] = new TGraphErrors( n_points );
		double chi_sq = alpha_dist[i]->GetChisquare();
		bool found[2] = {false, false};
		
		for( unsigned int j = 0; j < 10001; j++ ){
		
			distance = 5 + 0.01*j ;
			alpha_dist_chi_square[i]->FixParameter( 1 , distance );
			data_points[i]->Fit( alpha_dist_chi_square[i], "NQ" );
			chi_2_nu = alpha_dist_chi_square[i]->GetChisquare(); // 15 free parameters
			if (TMath::Abs(chi_2_nu-chi_sq-1) < 0.1 && !found[index] ){
				if( index == 1 && distance < alpha_dist[i]->GetParameter(1) ) continue;
				chi2plus1dist[index] = distance;
				found[index] = true;
				index++;
			}
		
			chi_plot[i]->SetPoint( chi_plot[i]->GetN(), distance, chi_2_nu );
		
		}
		
		chi_plot[i]->Draw();
		
		chi_plot[i]->SetTitle("");
		chi_plot[i]->GetXaxis()->SetTitle("distance mm");
		chi_plot[i]->GetYaxis()->SetTitle("#chi^{2}_{#nu}");
		chi_plot[i]->GetXaxis()->CenterTitle();
		chi_plot[i]->GetYaxis()->CenterTitle();
		chi_plot[i]->GetXaxis()->SetTitleSize(0.07);
		chi_plot[i]->GetYaxis()->SetTitleSize(0.07);
		chi_plot[i]->GetXaxis()->SetTitleOffset(0.5);
		chi_plot[i]->GetYaxis()->SetTitleOffset(0.5);
		chi_plot[i]->GetYaxis()->SetRangeUser(chi_sq*0.5, chi_sq*2);
		data_points[i]->SetTitle("");
		data_points[i]->GetXaxis()->SetTitle("CD ring segment");
		data_points[i]->GetYaxis()->SetTitle("Intensity");
		data_points[i]->GetXaxis()->CenterTitle();
		data_points[i]->GetYaxis()->CenterTitle();
		data_points[i]->GetXaxis()->SetTitleSize(0.07);
		data_points[i]->GetYaxis()->SetTitleSize(0.07);
		data_points[i]->GetYaxis()->SetTitleOffset(0.5);
		data_points[i]->GetXaxis()->SetTitleOffset(0.6);
		
		double dist = alpha_dist[i]->GetParameter(1);
		double d_dist = alpha_dist[i]->GetParError(1);
		chi_plot[i]->GetXaxis()->SetLimits(dist-2, dist+2);
		
		TArrow* arrow = new TArrow(dist, chi_sq+2, dist, chi_sq, 0.01, "|>");
		arrow->Draw();
		TLine* lineLeft = new TLine(chi2plus1dist[0], 0, chi2plus1dist[0], chi_sq+1);
		lineLeft->Draw();
		TLine* lineRight = new TLine(chi2plus1dist[1], 0, chi2plus1dist[1], chi_sq+1);
		lineRight->Draw();
		TArrow* arrowAcross = new TArrow(chi2plus1dist[0], chi_sq+1, chi2plus1dist[1], chi_sq+1, 0.01, "<|>");
		arrowAcross->Draw();
		
		// sprintf(cname, "%.2f #pm %.2f mm", dist, d_dist);
		// TLatex* text = new TLatex(dist, chi_sq/15+3, cname);
		// text->SetTextSize(0.06);
		// text->SetTextAlign(22);
		// text->Draw();
		sprintf(cname, "%.2f^{#plus%.2f}_{#minus%.2f} mm", dist, chi2plus1dist[1]-dist, dist-chi2plus1dist[0]);
		TLatex* text = new TLatex(dist, chi_sq*1.2, cname);
		text->SetTextSize(0.06);
		text->SetTextAlign(22);
		text->Draw();
		cout << "par0\t" << alpha_dist[i]->GetParameter(0) << "\n" << "distance in mm\t" << dist << "\terror of distance\t" <<  d_dist << "\n" << "reduced chi square\t" << chi_sq/15. << endl;
		yasym.push_back( dist );
		duyasym.push_back( chi2plus1dist[1]-dist );
		dlyasym.push_back( dist-chi2plus1dist[0] );
		
	}
	
	c1->cd();
	TPad* pbottom = new TPad("pbottom", "pbottom", 0, 0., 1.0, 0.3);
	pbottom->Draw();
	pbottom->cd();
	TGraphAsymmErrors* gasym = new TGraphAsymmErrors(4, xasym.data(), yasym.data(), dlxasym.data(), duxasym.data(), dlyasym.data(), duyasym.data());
	gasym->GetXaxis()->SetTitle("Quadrant");
	gasym->GetXaxis()->CenterTitle();
	gasym->GetYaxis()->SetTitle("CD distance (mm)");
	gasym->GetYaxis()->CenterTitle();
	gasym->SetMarkerStyle(20);
	gasym->Draw("ap");
	gasym->SetTitle("");
	TF1* f0 = new TF1("f0", "[0]", 0, 5);
	gasym->Fit(f0);
	sprintf(cname, "Average distance = %.2f #pm %.2f mm", f0->GetParameter(0), f0->GetParError(0));
	TLatex* tdist_avg = new TLatex(2.5, f0->GetParameter(0)+0.5, cname);
	tdist_avg->SetTextSize(0.06);
	tdist_avg->SetTextAlign(22);
	tdist_avg->Draw();
	//cout<<  <<endl;
	
	return;
	
}
