
#include "MiniballGeometry.hh"

ClassImp(MiniballGeometry)

void MiniballGeometry::SetupCluster( double user_theta, double user_phi, double user_alpha, double user_r, double user_z ) {

	// Set the user value
	theta = user_theta * TMath::DegToRad();
	phi = user_phi * TMath::DegToRad();
	alpha = user_alpha * TMath::DegToRad();
	r = user_r;
	z = user_z;
	
	// Resize arrays
	seg_offset.resize( ncry );
	for( unsigned int i = 0; i < ncry; ++i )
		seg_offset[i].resize( nseg );

	// Set individual values
	SetupCluster();

	return;

}


void MiniballGeometry::SetupCluster() {

	// Some constants
	double root32 = TMath::Sqrt(3.0) / 2.0;

	// Parameters determined by Hannah Kleis for new cryostats
	double interaction_depth = 18.0;				///< mm; ~25mm for higher energies (E>500keV)
	double inter_crystal_gap = 1.0;					///< mm; gap between the capsules inside the cryostat
	double dist_cap_crystal = 0.55;					///< mm; distance between capsule and crystal
	double cap_thickness = 0.70;					///< mm; thickness of the capsule walls
	double alpha_taper = 4.125 * TMath::DegToRad();	///< rad; taper angle from the crystal and capsule
	double dist_flat_to_flat = 61.35;				///< mm; from the hexagon shape in the front of the capsule
	double dist_core_corner = dist_flat_to_flat / TMath::Sqrt(3.0); ///< of capsule
	double R = 0.5 * dist_core_corner - dist_cap_crystal - cap_thickness;
	double focal_length = ( dist_core_corner + inter_crystal_gap * 0.5 );
	focal_length /= TMath::Tan( alpha_taper );		///< focal length using taper angle and distance from capsule core to next capsule

	// Old parameters to which the online angles were optimised
	if( type == 0 ) {
		
		interaction_depth = 0.0;
		dist_core_corner = 34.0;
		focal_length = 400.0;
		R = dist_core_corner * 0.5;
		alpha_taper = TMath::ATan( dist_core_corner / focal_length );
		
	}
	
	// These values assume we are going to rotate the crystals to make a cluster
	// The real life clock positions will be 4 hours earlier after rotation
	// Therefore we need to start 4 hours later, then we rotate to the right position
	// Z is beam direction
	// Y is out to the right (horizontal)
	// X is directly upwards (vertical)
	// Offset from centre of a detector to centre of a segment
	for( unsigned char i = 0; i < ncry; i++ ) {

		seg_offset[i][0].SetXYZ(  0.0  ,  0.0,        0.0 ); // core in centre
		seg_offset[i][1].SetXYZ(  R / 2, -R * root32, 0.0 ); // 10 o'clock
		seg_offset[i][2].SetXYZ( -R / 2, -R * root32, 0.0 ); // 8 o'clock
		seg_offset[i][3].SetXYZ(  R    ,  0.0,        0.0 ); // 12 o'clock
		seg_offset[i][4].SetXYZ(  R / 2,  R * root32, 0.0 ); // 2 o'clock
		seg_offset[i][5].SetXYZ( -R    ,  0.0,        0.0 ); // 6 o'clock
		seg_offset[i][6].SetXYZ( -R / 2,  R * root32, 0.0 ); // 4 o'clock
	
	}
	
	// Nominal distance for the Miniball clusters is at some larger distance
	// So we push the crystal out there, shift it out a bit
	// then rotate it around.
	for( unsigned char i = 0; i < ncry; i++ ) {
		
		for( unsigned char j = 0; j < nseg; j++ ) {
		
			// out to nominal distance in beam direction
			seg_offset[i][j].SetZ( focal_length );

			// Rotate out to focal point of crystal (goes up in MB frame)
			seg_offset[i][j].RotateY( alpha_taper );

			// rotate around the appropriate angle for each crystal
			seg_offset[i][j].RotateZ( -TMath::TwoPi() * i / 3.0 );
			
			// Now bring it all back to the correct distance and shift to the interaction depth
			seg_offset[i][j].SetZ( seg_offset[i][j].Z() - focal_length + interaction_depth + r );

		}
	
	}

	//std::cout << "X = " << seg_offset[0][0].X() << ", ";
	//std::cout << "Y = " << seg_offset[0][0].Y() << ", ";
	//std::cout << "Z = " << seg_offset[0][0].Z() << std::endl;
	
	// Offsets - We start now with the crystal in beam direction (X)
	double myalpha, myphi, mytheta;
	
	if( phi > TMath::Pi() || phi < 0 ) {
		myalpha = alpha;
		myphi = TMath::PiOver2() + phi;
		mytheta = theta;
	}
	else {
		myalpha = alpha;
		myphi = TMath::PiOver2() - phi;
		mytheta = -theta;
	}

	// Rotate segments to appropriate angle
	for( unsigned char i = 0; i < ncry; i++ ) {
		for( unsigned char j = 0; j < nseg; j++ ) {
			seg_offset[i][j].RotateZ( -myalpha ); // anti-clockwise to agree with old convention
			seg_offset[i][j].RotateY( myphi );
			seg_offset[i][j].RotateX( mytheta );
		}
	}
  
	// Shift Miniball so that target it as origin
	mbzoffset.SetXYZ( -z, 0.0, 0.0 );
	for( unsigned char i = 0; i < ncry; i++ ) {
		for( unsigned char j = 0; j < nseg; j++ ) {
			 seg_offset[i][j] += mbzoffset;
		 }
	 }

 	return;
 	
}

