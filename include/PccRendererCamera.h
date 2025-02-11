//Copyright(c) 2016 - 2025, InterDigital
//All rights reserved.
//See LICENSE under the root folder.


#ifndef _CAMERA_RENDERER_APP_H_
#define _CAMERA_RENDERER_APP_H_

#include "PccRendererDef.h"

class Box;

/*! \class %Camera
 * \brief %Camera class.
 *
 *  This class is used to define a camera.
 */
class Camera {
 public:
  Camera();
  ~Camera();
  void         create( float fBoxSize, float fFov = 20 );
  void         getLookAt( Vec3& eEye, Vec3& eCenter, Vec3& eUp ) const;
  void         setLookAt( Vec3 eEye, Vec3 eCenter, Vec3 eUp );
  void         getPerspective( float& fFov, float& fNear, float& fFar );
  Vec3         getCenter() { return m_eCenter; }
  inline float getDistance() { return m_fDist; }
  Vec3         getPosition();
  Quaternion&  getRotation() { return m_eRotation; }
  float        getFov() { return m_fFov; }
  void         set( Camera* pcCamera );
  inline void  setDistance( float fDist ) { m_fDist = fDist; }
  void         setFov( float fFov ) {
    if ( m_fFov != fFov ) {
      if ( fFov >= 1.f && m_fFov >= 1.f ) {
        m_fDist = m_fDist * std::tan( m_fFov * (float)M_PI / 360.f ) / std::tan( fFov * (float)M_PI / 360.f );
      }
      m_fFov = fFov;
    }
  }
  // void setLookAt( Mat4& matrix );

  void       rotate( const double fX0, const double fY0, const double fX1, const double fY1 );
  void       rotate( const double angle );
  Quaternion getRotate( const double fX0, const double fY0, const double fX1, const double fY1 );
  void       zoom( float fZoom );
  void       translate( float x, float y, float z );
  void       align( int iDirection, Box& box, int zoom );
  void       remove();
  void       write( bool bSynchronize, int iFrameIndex );
  void       read( bool& bSynchronize, int& iFrameIndex );
  void       readViewpoint( const std::string& pFilename );
  void       writeViewpoint( const std::string& pFilename );
  void       printPositionParameters();

 private:
  Camera( const Camera& that );
  void  operator=( const Camera& that );
  Vec3& projectToSphere( const float& radius, Vec3& p ) const;

  Quaternion  m_eRotation;
  Vec3        m_eCenter;
  float       m_fDist;
  float       m_fBoxSize;
  float       m_fFov;
  std::string m_sSyncFile = "camera.txt";
};

#endif  //~_CAMERA_RENDERER_APP_H_
