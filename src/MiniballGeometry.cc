
#include "MiniballGeometry.hh"

ClassImp(MiniballGeometry)

void MiniballGeometry::SetCluTheta( double user_theta ) {

	// Set the user value
	theta = user_theta;

	return;

}

void MiniballGeometry::SetCluPhi( double user_phi ) {
	
	// Set the user value
	phi = user_phi;

	return;

}

void MiniballGeometry::SetCluAlpha( double user_alpha ) {

	// Set the user value
	alpha = user_alpha;

	return;

}

void MiniballGeometry::SetCluR( double user_r ) {

	// Set the user value (mm)
	r = user_r;
	
	return;
	
}

void MiniballGeometry::SetCluZ( double user_z ) {

	// Set the user value (mm)
	z = user_z;
	
	return;
	
}

double MiniballGeometry::GetSegTheta( unsigned char cry, unsigned char seg ) {

	// New method of returning theta in true coordinate system
	// Check if user is asking for the core or a segment
	if( seg == 0 ) return TrueTheta( cry_offset[cry] );
	else return TrueTheta( seg_offset[cry][seg-1] );

}

double MiniballGeometry::GetSegPhi( unsigned char cry, unsigned char seg ) {
	
	// New method of returning phi in true coordinate system
	// Check if user is asking for the core or a segment
	if( seg == 0 ) return TruePhi( cry_offset[cry] );
	else return TruePhi( seg_offset[cry][seg-1] );
	
}

double MiniballGeometry::GetCryTheta( unsigned char cry ) {
	
	// Return theta from segment offset
	return TrueTheta( cry_offset[cry] );

}

double MiniballGeometry::GetCryPhi( unsigned char cry ) {
	
	// Return phi from segment offset
	return TruePhi( cry_offset[cry] );

}

void MiniballGeometry::SetupCluster( double user_theta, double user_phi, double user_alpha, double user_r, double user_z ) {

	// Set the user value
	theta = user_theta * TMath::DegToRad();
	phi = user_phi * TMath::DegToRad();
	alpha = user_alpha * TMath::DegToRad();
	r = user_r;
	z = user_z;

	if( phi > TMath::Pi() ) theta = TMath::Pi() - theta;

	// Resize arrays
	cry_offset.resize( ncry );
	seg_offset.resize( ncry );
	for( unsigned int i = 0; i < ncry; ++i )
		seg_offset[i].resize( nseg );

	// Set individual values
	SetupCluster();

	return;

}


void MiniballGeometry::SetupCluster() {

	// Offset of cluster
	clu_offset.SetXYZ(0, r, 0);

	// Offset from centre of cluster to centre of each detector
#define USE_EMPIRICAL 0
#if USE_EMPIRICAL // These are values determined from a Geant4 simulation
	cry_offset[0].SetXYZ(-0.0166, r + 8.6739, -33.7254);
	cry_offset[1].SetXYZ(-29.1946, r + 8.6628, 16.8867);
	cry_offset[2].SetXYZ(29.2040, r + 8.6781, 16.8364);
	
	seg_offset[0][0].SetXYZ(-13.8080,  -0.9293, -8.9018);
	seg_offset[0][1].SetXYZ(-15.1076,  0.7917, 8.1148);
	seg_offset[0][2].SetXYZ(-0.0098,  -1.5906, -16.6596);
	seg_offset[0][3].SetXYZ(13.8680,  -0.9272, -8.9303);
	seg_offset[0][4].SetXYZ(-0.0149,  1.8715, 18.2628);
	seg_offset[0][5].SetXYZ(15.0723,  0.7839, 8.1142);
	
	seg_offset[1][0].SetXYZ(-0.7397,  -0.8946, 16.4265);
	seg_offset[1][1].SetXYZ(14.5265,  0.7779, 9.0148);
	seg_offset[1][2].SetXYZ(-14.3342,  -1.5688, 8.2345);
	seg_offset[1][3].SetXYZ(-14.6338,  -0.9115, -7.5162);
	seg_offset[1][4].SetXYZ(15.7437,  1.8780, -9.0642);
	seg_offset[1][5].SetXYZ(-0.5625,  0.7189, -17.0954);
	
	seg_offset[2][0].SetXYZ(14.6795,  -0.9260, -7.5641);
	seg_offset[2][1].SetXYZ(0.5776,  0.7675, -17.0816);
	seg_offset[2][2].SetXYZ(14.3709,  -1.5876, 8.2896);
	seg_offset[2][3].SetXYZ(0.7611,  -0.9218, 16.4052);
	seg_offset[2][4].SetXYZ(-15.8436,  1.8638, -9.0467);
	seg_offset[2][5].SetXYZ(-14.5454,  0.8040, 8.9976);
#else
	Double_t R = DIST_CORE_CORNER * 0.5; // average distance from centre of capsule to centre of segment
	
	cry_offset[0].SetXYZ( 0,                                 r+9, -DIST_CORE_CORNER);
	cry_offset[1].SetXYZ(-DIST_CORE_CORNER * ROOTTHREEOVER2, r+9,  DIST_CORE_CORNER / 2);
	cry_offset[2].SetXYZ( DIST_CORE_CORNER * ROOTTHREEOVER2, r+9,  DIST_CORE_CORNER / 2);
	
	// Offset from centre of a detector to centre of a segment
	seg_offset[0][0].SetXYZ(-R * ROOTTHREEOVER2, 0.0, -R / 2 );
	seg_offset[0][1].SetXYZ(-R * ROOTTHREEOVER2, 0.0,  R / 2 );
	seg_offset[0][2].SetXYZ( 0.0,                0.0, -R     );
	seg_offset[0][3].SetXYZ( R * ROOTTHREEOVER2, 0.0, -R / 2 );
	seg_offset[0][4].SetXYZ( 0.0,                0.0,  R     );
	seg_offset[0][5].SetXYZ( R * ROOTTHREEOVER2, 0.0,  R / 2 );
	
	seg_offset[1][0].SetXYZ( 0.0,                0.0,  R     );
	seg_offset[1][1].SetXYZ( R * ROOTTHREEOVER2, 0.0,  R / 2 );
	seg_offset[1][2].SetXYZ(-R * ROOTTHREEOVER2, 0.0,  R / 2 );
	seg_offset[1][3].SetXYZ(-R * ROOTTHREEOVER2, 0.0, -R / 2 );
	seg_offset[1][4].SetXYZ( R * ROOTTHREEOVER2, 0.0, -R / 2 );
	seg_offset[1][5].SetXYZ( 0.0,                0.0, -R     );
	
	seg_offset[2][0].SetXYZ( R * ROOTTHREEOVER2, 0.0, -R / 2 );
	seg_offset[2][1].SetXYZ( 0.0,                0.0, -R     );
	seg_offset[2][2].SetXYZ( R * ROOTTHREEOVER2, 0.0,  R / 2 );
	seg_offset[2][3].SetXYZ( 0.0,                0.0,  R     );
	seg_offset[2][4].SetXYZ(-R * ROOTTHREEOVER2, 0.0, -R / 2 );
	seg_offset[2][5].SetXYZ(-R * ROOTTHREEOVER2, 0.0,  R / 2 );
#endif
	
   	// Add the segment offsets to the detector offsets, so now
   	// seg_offset has the offset from the centre of the cluster
	for( unsigned char i = 0; i < ncry; i++ )
		for( unsigned char j = 0; j < nseg; j++ )
			seg_offset[i][j] += cry_offset[i];

	// Offsets
	double myalpha, mytheta, myphi;
	if( phi < TMath::Pi() ) {
		myalpha = alpha;
		myphi = TMath::Pi() / 2. + phi;
		mytheta = TMath::Pi() / 2. + theta;
	}
	else {
		myalpha = alpha - TMath::Pi();
		myphi = TMath::Pi() / 2. + phi;
		mytheta = TMath::Pi() / 2. + theta;
	}

	// Rotate cluster to appropriate angle
	clu_offset.RotateY(myalpha);
	clu_offset.RotateX(myphi);
	clu_offset.RotateZ(mytheta);
		
	// Rotate crystal to appropriate angle
	for( unsigned char i = 0; i < ncry; i++ ) {
		cry_offset[i].RotateY(myalpha);
		cry_offset[i].RotateX(myphi);
		cry_offset[i].RotateZ(mytheta);
	}

	// Rotate segments to appropriate angle
	for( unsigned char i = 0; i < ncry; i++ ) {
		for( unsigned char j = 0; j < nseg; j++ ) {
			seg_offset[i][j].RotateY(myalpha);
			seg_offset[i][j].RotateX(myphi);
			seg_offset[i][j].RotateZ(mytheta);
		}
	}
	
	// Shift Miniball so that target it as origin
	mbzoffset.SetXYZ( -1.0*z, 0.0, 0.0 );
	clu_offset += mbzoffset;
	for( unsigned char i = 0; i < ncry; i++ ) {
		cry_offset[i] += mbzoffset;
		for( unsigned char j = 0; j < nseg; j++ ) {
			 seg_offset[i][j] += mbzoffset;
		 }
	 }

 	return;
 	
}

