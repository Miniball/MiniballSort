#ifndef __MINIBALLGEOMETRY_HH
#define __MINIBALLGEOMETRY_HH

#include <vector>
#include <iostream>

#include <TObject.h>
#include <TVector3.h>
#include <TMath.h>

#define ROOTTHREEOVER2 0.8660254
#define DIST_CORE_CORNER 34.0 // first iteration was 34.0 - new 34.5
#define NOMINAL_CLUSTER_DIST 400.0 // first iteration was 400.0 - new 430.0
#define INTERACTION_DEPTH 15.0 // first iteration was 0.0
#define INTER_CRYSTAL_GAP 2.5 // first iteration was 0.0

/// Functions to convert Miniball angles read from the frame

/// --------------------------------------------------------------------
/// Miniball Geometry class
/// --------------------------------------------------------------------
/// A geometry class that converts cluster angles read from the Miniball
/// frame into real-life angles for the analysis.
/// You can have theta and phi angles of the centres of each cluster,
/// crystal and segment in frame geometry or true/beam geometry.

class MiniballGeometry : public TObject {
	
public:
	
	/// Constructor
	MiniballGeometry() {};
	
	/// Destructor
	~MiniballGeometry() {};
	
	/// Setup cluster main routine
	void SetupCluster();
	/// Setup the cluster with coordinate values
	/// \param user_theta in the MB frame of reference [degrees]
	/// \param user_phi in the MB frame of reference [degrees]
	/// \param user_alpha in the MB frame of reference [degrees]
	/// \param user_r distance from target to detector [mm]
	/// \param user_z distance from target to origin in beam direction [mm]
	void SetupCluster( double user_theta, double user_phi, double user_alpha, double user_r, double user_z );
	
	/// Set the theta angle measured from the frame
	/// \param user_theta in the MB frame of reference [degrees]
	inline void SetCluTheta( double user_theta ){ theta = user_theta * TMath::DegToRad(); };
	
	/// Set the phi angle measured from the frame
	/// \param user_phi in the MB frame of reference [degrees]
	inline void SetCluPhi( double user_phi ){ phi = user_phi * TMath::DegToRad(); };
	
	/// Set the alpha angle measured from the frame
	/// \param user_alpha in the MB frame of reference [degrees]
	inline void SetCluAlpha( double user_alpha ){ alpha = user_alpha * TMath::DegToRad(); };
	
	/// Set the distance between the target and face of the cluster
	/// \param user_r distance from target to detector [mm]
	inline void SetCluR( double user_r ){ r = user_r; };
	
	/// Set the distance between the origin and the target position along the beam axis
	/// \param user_z distance from target to origin in beam direction [mm]
	inline void SetCluZ( double user_z ) { z = user_z; };
	
	/// Get the theta angle of the crystal with respect to the beam
	/// \param cry number of the MB Ge crystal counting from 0 to 2
	/// \return theta of cry in beam reference (radians)
	inline double GetCryTheta( unsigned char cry ){
		return GetCryVector(cry).Theta();
	};
	
	/// Get the phi angle of the crystal with respect to the beam
	/// \param cry number of the MB Ge crystal counting from 0 to 2
	/// \return phi of cry in beam reference (radians)
	inline double GetCryPhi( unsigned char cry ){
		return GetCryVector(cry).Phi();
	};
	
	/// Get a unit vector pointing towards the core
	/// \param cry number of the MB Ge crystal counting from 0 to 2
	/// \return TVector3 of the crystal  position with respect to the nominal centre
	inline TVector3 GetCryVector( unsigned char cry ){
		return GetSegVector(cry,0);
	};
	
	/// Get the theta angle of a segment with respect to the beam
	/// \param cry number of the MB Ge crystal counting from 0 to 2
	/// \param seg number of the segment within the crystal: 0 is core, 1-6 for segments
	/// \return theta of segment in beam reference (radians)
	inline double GetSegTheta( unsigned char cry, unsigned char seg ){
		return GetSegVector(cry,seg).Theta();
	};
	
	/// Get the phi angle of a segment with respect to the beam
	/// \param cry number of the MB Ge crystal counting from 0 to 2
	/// \param seg number of the segment within the crystal: 0 is core, 1-6 for segments
	/// \return phi of segment in beam reference (radians)
	inline double GetSegPhi( unsigned char cry, unsigned char seg ){
		return GetSegVector(cry,seg).Phi();
	};
	
	/// Get the x position of a segment with respect to the beam
	/// \param cry number of the MB Ge crystal counting from 0 to 2
	/// \param seg number of the segment within the crystal: 0 is core, 1-6 for segments
	/// \return x position of segment in beam reference (mm)
	inline double GetSegX( unsigned char cry, unsigned char seg ){
		return GetSegVector(cry,seg).X();
	};
	
	/// Get the y position of a segment with respect to the beam
	/// \param cry number of the MB Ge crystal counting from 0 to 2
	/// \param seg number of the segment within the crystal: 0 is core, 1-6 for segments
	/// \return y position of segment in beam reference (mm)
	inline double GetSegY( unsigned char cry, unsigned char seg ){
		return GetSegVector(cry,seg).Y();
	};
	
	/// Get the z position of a segment with respect to the beam
	/// \param cry number of the MB Ge crystal counting from 0 to 2
	/// \param seg number of the segment within the crystal: 0 is core, 1-6 for segments
	/// \return z position of segment in beam reference (mm)
	inline double GetSegZ( unsigned char cry, unsigned char seg ){
		return GetSegVector(cry,seg).Z();
	};
	
	/// Get a unit vector pointing towards the segment
	/// \param cry number of the MB Ge crystal counting from 0 to 2
	/// \param seg number of the segment within the crystal: 0 is core, 1-6 for segments
	/// \return TVector3 of the segment position with respect to the nominal centre
	inline TVector3 GetSegVector( unsigned char cry, unsigned char seg ){
		return seg_offset[cry][seg];
	};
	
private:
	
	// Segments etc
	const unsigned char ncry = 3;
	const unsigned char nseg = 7; // 6 segments plus core
	
	// Current values of theta, phi, alpha and r
	double theta;	///< theta angle in radians
	double phi;		///< phi angle in radians
	double alpha;	///< alpha angle in radians
	double r; 		///< distance from target to detector in mm
	double z; 		///< distance from target to origin (beam direction is positive) in mm
	
	// Geometry
	std::vector<std::vector<TVector3>> seg_offset;	///< vector for segment centre (0 = core)
	TVector3 mbzoffset;								///< Offset of target from origin in direction of beam in mm.
	///< This shift is independent of the CD detector distance which is relative.
	
	ClassDef( MiniballGeometry, 2 );
	
};

#endif
