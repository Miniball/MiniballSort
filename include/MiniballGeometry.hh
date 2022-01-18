#ifndef __MINIBALLGEOMETRY_HH
#define __MINIBALLGEOMETRY_HH

#include <vector>

#include <TObject.h>
#include <TVector3.h>
#include <TMath.h>

#define ROOTTHREEOVER2 0.8660254

#define DIST_CORE_CORNER 34

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
	void SetCluTheta( double user_theta );
	
	/// Set the phi angle measured from the frame
	/// \param user_phi in the MB frame of reference [degrees]
	void SetCluPhi( double user_phi );
	
	/// Set the alpha angle measured from the frame
	/// \param user_alpha in the MB frame of reference [degrees]
	void SetCluAlpha( double user_alpha );
	
	/// Set the distance between the target and face of the cluster
	/// \param user_r distance from target to detector [mm]
	void SetCluR( double user_r );
	
	/// Set the distance between the origin and the target position along the beam axis
	/// \param user_z distance from target to origin in beam direction [mm]
	void SetCluZ( double user_z );

	/// Get the theta angle of the crystal with respect to the beam
	/// \param cry number of the MB Ge crystal counting from 0 to 2
	/// \return theta of cry in beam reference (degrees)
	double GetCryTheta( unsigned char cry );

	/// Get the phi angle of the crystal with respect to the beam
	/// \param cry number of the MB Ge crystal counting from 0 to 2
	/// \return phi of cry in beam reference (degrees)
	double GetCryPhi( unsigned char cry );

	/// Get a unit vector pointing towards the core
	/// \param cry number of the MB Ge crystal counting from 0 to 2
	/// \return phi of segment in beam reference (degrees)
	TVector3 GetCryVector( unsigned char cry );

	/// Get the theta angle of a segment with respect to the beam
	/// \param cry number of the MB Ge crystal counting from 0 to 2
	/// \param seg number of the segment within the crystal: 0 is core, 1-6 for segments
	/// \return theta of segment in beam reference (degrees)
	double GetSegTheta( unsigned char cry, unsigned char seg );

	/// Get the phi angle of a segment with respect to the beam
	/// \param cry number of the MB Ge crystal counting from 0 to 2
	/// \param seg number of the segment within the crystal: 0 is core, 1-6 for segments
	/// \return phi of segment in beam reference (degrees)
	double GetSegPhi( unsigned char cry, unsigned char seg );

	/// Get a unit vector pointing towards the segment
	/// \param cry number of the MB Ge crystal counting from 0 to 2
	/// \param seg number of the segment within the crystal: 0 is core, 1-6 for segments
	/// \return phi of segment in beam reference (degrees)
	TVector3 GetSegVector( unsigned char cry, unsigned char seg );

	/// Get theta of a vector in Miniball system (return deg)
	/// \param v vector from target to detector
	/// \return theta in the Miniball system (degrees)
	inline double MBTheta( TVector3 &v ) {
		double mytheta = v.Phi() * TMath::RadToDeg();
		double myphi = v.Theta() * TMath::RadToDeg();
		if( mytheta < 0 ) {
			myphi = 360 - myphi;
			mytheta = - mytheta;
		}
		return mytheta;
	};

	/// Get phi of a vector in Miniball system (return deg)
	/// \param v vector from target to detector
	/// \return phi in the Miniball system (degrees)
	inline double MBPhi( TVector3 &v ) {
		double mytheta = v.Phi() * TMath::RadToDeg();
		double myphi = v.Theta() * TMath::RadToDeg();
		if( mytheta < 0 ) {
			myphi = 360 - myphi;
			mytheta = -mytheta;
		}
		return myphi;
	};
	
	/// Get true theta of a vector (return deg)
	/// \param v vector from target to detector
	/// \return theta in the beam system (degrees)
	inline double TrueTheta( TVector3 &v ) {
		return TMath::ACos( v.x() / v.Mag() ) * TMath::RadToDeg();
	};
	
	/// Get the true phi of a vector (return deg)
	/// \param v vector from target to detector
	/// \return phi in the beam system (degrees)
	inline double TruePhi( TVector3 &v ) {
		TVector3 v2( -v.y(), v.z(), v.x() );
		double phi = v2.Phi() * TMath::RadToDeg();
		if( phi < 0 ) phi += 360;
		phi -= 90;
		if( phi < 0 ) phi += 360;
		return phi;
	};
	
	private:
	
	// Segments etc
	const unsigned char ncry = 3;
	const unsigned char nseg = 6;

	// Current values of theta, phi, alpha and r
	double theta;	///< theta angle in deg
	double phi;		///< phi angle in deg
	double alpha;	///< alpha angle in deg
	double r; 		///< distance from target to detector in mm
	double z; 		///< distance from target to origin (beam direction is positive) in mm
	
	// Geometry
	TVector3 clu_offset;							///< vector for cluster centre
	std::vector<TVector3> cry_offset;				///< vector for crystal centre
	std::vector<std::vector<TVector3>> seg_offset;	///< vector for segment centre
	TVector3 mbzoffset;								///< Offset of target from origin in direction of beam in mm.
													///< This shift is independent of the CD detector distance which is relative.

	ClassDef( MiniballGeometry, 1 );
	
};

#endif
