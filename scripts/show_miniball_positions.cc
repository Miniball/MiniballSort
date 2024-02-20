// This code will take a reaction file and plot the positions of the Miniball segments
// in the x-y and theta-phi planes. It does not consider segment swaps.
//
// Written by Liam Gaffney (liam.gaffney@liverpool.ac.uk) - 20/02/2024

R__LOAD_LIBRARY(libmb_sort.so)

void show_miniball_positions( std::string reactionfile = "default", std::string settingsfile = "default" ) {
	
	// Settings file - needed for reaction file
	std::shared_ptr<MiniballSettings> myset = std::make_shared<MiniballSettings>( settingsfile );
	
	// Reacton file
	std::shared_ptr<MiniballReaction> myreact = std::make_shared<MiniballReaction>( reactionfile, myset );

	// Some colours and marker styles
	std::vector<int> mystyles, mycolors;
	mystyles.push_back( kFullCircle );
	mystyles.push_back( kFullSquare );
	mystyles.push_back( kFullTriangleUp );
	mystyles.push_back( kFullTriangleDown );
	mystyles.push_back( kFullCross );
	mystyles.push_back( kFullStar );
	mystyles.push_back( kFullDiamond );
	mystyles.push_back( kFullCrossX );
	mystyles.push_back( kFullFourTrianglesX );
	mystyles.push_back( kFullThreeTriangles );
	mystyles.push_back( kFullDoubleDiamond );
	mystyles.push_back( kFourSquaresX );
	mystyles.push_back( kFourSquaresPlus );
	mycolors.push_back( kRed+1 );
	mycolors.push_back( kBlue+1 );
	mycolors.push_back( kGreen+1 );

	// Canvas
	TCanvas *c1 = new TCanvas( "x-y", "x-y", 700, 1000 );
	TCanvas *c2 = new TCanvas( "theta-phi", "theta-phi", 700, 800 );
	c1->Divide(1,2);
	
	// Draw a placeholder for the axes
	double _x[2] = { -150.0, 150.0 };
	double _y[2] = { -150.0, 150.0 };
	double _th[2] = { 0.0, 180.0 };
	double _phi[2] = { -180.0, 180.0 };
	TGraph *_g_xyu = new TGraph( 2, _x, _y );
	TGraph *_g_xyd = new TGraph( 2, _x, _y );
	TGraph *_g_tp = new TGraph( 2, _th, _phi );
	auto p11 = c1->cd(1);
	p11->SetMargin( 0.15, 0.21, 0.15, 0.10 );
	_g_xyd->SetTitle( "x-y positions downstream;y (horizontal) [mm];x (vertical) [mm]" );
	_g_xyd->SetMarkerColorAlpha( kWhite, 0.0 );
	_g_xyd->Draw("A*");
	auto p12 = c1->cd(2);
	p12->SetMargin( 0.15, 0.21, 0.15, 0.10 );
	_g_xyu->SetTitle( "x-y positions upstream;y (horizontal) [mm];x (vertical) [mm]" );
	_g_xyu->SetMarkerColorAlpha( kWhite, 0.0 );
	_g_xyu->Draw("A*");
	auto p2 = c2->cd();
	p2->SetMargin( 0.15, 0.21, 0.15, 0.10 );
	_g_tp->SetTitle( "#theta-#phi positions;#theta [deg];#phi [deg]" );
	_g_tp->SetMarkerColorAlpha( kWhite, 0.0 );
	_g_tp->Draw("A*");
	
	// Make a legend
	TLegend *legend = new TLegend( 0.79, 0.15, 0.95, 0.90 );

	// Graphs for each cluster and each crystal
	std::vector<std::vector<TGraph*>> g_xy_pos, g_xy_neg, g_th_phi;
	std::string legend_label, point_label;
	
	// Loop over clusters
	g_xy_pos.resize( myset->GetNumberOfMiniballClusters() );
	g_xy_neg.resize( myset->GetNumberOfMiniballClusters() );
	g_th_phi.resize( myset->GetNumberOfMiniballClusters() );
	for( unsigned int i = 0; i < myset->GetNumberOfMiniballClusters(); i++ ){
		
		g_xy_pos[i].resize( myset->GetNumberOfMiniballCrystals() );
		g_xy_neg[i].resize( myset->GetNumberOfMiniballCrystals() );
		g_th_phi[i].resize( myset->GetNumberOfMiniballCrystals() );

		// Loop over crystals
		for( unsigned int j = 0; j < myset->GetNumberOfMiniballCrystals(); j++ ){
			
			g_xy_pos[i][j] = new TGraph();
			g_xy_neg[i][j] = new TGraph();
			g_th_phi[i][j] = new TGraph();
			
			legend_label = "MB" + std::to_string(i) + static_cast<char>(j+65);
			legend->AddEntry( g_xy_pos[i][j], legend_label.data() );

			// Loop over segments
			for( unsigned int k = 0; k < myset->GetNumberOfMiniballSegments(); k++ ){
			
				// Coordinates
				double x = myreact->GetGammaX(i,j,k);
				double y = myreact->GetGammaY(i,j,k);
				double z = myreact->GetGammaZ(i,j,k);
				
				// Theta-phi
				double th  = myreact->GetGammaTheta(i,j,k) * TMath::RadToDeg();
				double phi = myreact->GetGammaPhi(i,j,k) * TMath::RadToDeg();

				// Upstream or downstream
				if( z > 0 ) g_xy_pos[i][j]->SetPoint( g_xy_pos[i][j]->GetN(), x, y );
				else g_xy_neg[i][j]->SetPoint( g_xy_neg[i][j]->GetN(), x, y );
				
				// Theta-phi
				g_th_phi[i][j]->SetPoint( g_th_phi[i][j]->GetN(), th, phi );

				// Label the segment
				point_label = std::to_string(k);
				TText t;
				t.SetTextFont(42);
				t.SetTextSize(0.02);
				if( z > 0 ) c1->cd(1);
				else c1->cd(2);
				t.DrawText( x, y, point_label.data() );
				c2->cd();
				t.DrawText( th, phi, point_label.data() );

				
			}
			
			// Change the colours and marker styles for each crystal
			g_xy_pos[i][j]->SetMarkerStyle( mystyles.at(i) );
			g_xy_neg[i][j]->SetMarkerStyle( mystyles.at(i) );
			g_th_phi[i][j]->SetMarkerStyle( mystyles.at(i) );
			g_xy_pos[i][j]->SetMarkerColor( mycolors.at(j) );
			g_xy_neg[i][j]->SetMarkerColor( mycolors.at(j) );
			g_th_phi[i][j]->SetMarkerColor( mycolors.at(j) );
			
			// Draw the crystals
			c1->cd(1);
			g_xy_pos[i][j]->Draw("P SAME");
			c1->cd(2);
			g_xy_neg[i][j]->Draw("P SAME");
			c2->cd();
			g_th_phi[i][j]->Draw("P SAME");
			
		}
				
	}
	
	// Draw the legend
	c1->cd(1);
	legend->Draw();
	c1->cd(2);
	legend->Draw();
	c2->cd();
	legend->Draw();

	
	return;
	
}

