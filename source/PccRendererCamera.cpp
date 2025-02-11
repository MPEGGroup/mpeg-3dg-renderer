//Copyright(c) 2016 - 2025, InterDigital
//All rights reserved.
//See LICENSE under the root folder.

#include "PccRendererCamera.h"
#include "PccRendererObject.h"

Camera::Camera() {}
Camera::~Camera() {}

void Camera::create( const float fBoxSize, const float fFov ) {
  m_eRotation = Quaternion( 1, 0, 0, 0 );
  m_eCenter   = Vec3( fBoxSize / 2.f, fBoxSize / 2.f, fBoxSize / 2.f );
  m_fDist     = fBoxSize;
  m_fBoxSize  = fBoxSize;
  m_fFov      = fFov;
  remove();
}

void Camera::align( int iDirection, Box& box, int iZoom ) {
  auto s = box.size();
  auto c = box.center();
  Vec2 d = 3.2f * Vec2( ( std::max )( s[1], s[2] ), ( std::max )( s[0], s[2] ) );
  if ( iZoom > 0 ) {
    d /= 3.f;
    if ( iZoom == 1 ) { c[iDirection < 8 ? 1 : 0] += s[iDirection < 8 ? 1 : 0] / 3.f; }
    if ( iZoom == 2 ) { c[iDirection < 8 ? 1 : 0] += s[iDirection < 8 ? 1 : 0] / 6.f; }
    if ( iZoom == 4 ) { c[iDirection < 8 ? 1 : 0] -= s[iDirection < 8 ? 1 : 0] / 6.f; }
    if ( iZoom == 5 ) { c[iDirection < 8 ? 1 : 0] -= s[iDirection < 8 ? 1 : 0] / 3.f; }
  }
  m_eCenter     = c;
  m_fDist       = m_fBoxSize * 4.f;
  const float a = sqrt( 2.f ) / 2.f;
  switch ( iDirection ) {
    case 0: setLookAt( c + d[0] * Vec3( -1, +0, +0 ), c, Vec3( 0, 1, 0 ) ); break;  // 0   : -X: Right
    case 1: setLookAt( c + d[0] * Vec3( -a, +0, +a ), c, Vec3( 0, 1, 0 ) ); break;  // 45
    case 2: setLookAt( c + d[0] * Vec3( +0, +0, +1 ), c, Vec3( 0, 1, 0 ) ); break;  // 90  : +Y: Back
    case 3: setLookAt( c + d[0] * Vec3( +a, +0, +a ), c, Vec3( 0, 1, 0 ) ); break;  // 135
    case 4: setLookAt( c + d[0] * Vec3( +1, +0, +0 ), c, Vec3( 0, 1, 0 ) ); break;  // 180 : +X: Left
    case 5: setLookAt( c + d[0] * Vec3( +a, +0, -a ), c, Vec3( 0, 1, 0 ) ); break;  // 225
    case 6: setLookAt( c + d[0] * Vec3( +0, +0, -1 ), c, Vec3( 0, 1, 0 ) ); break;  // 270 : -Y: Face
    case 7: setLookAt( c + d[0] * Vec3( -a, +0, -a ), c, Vec3( 0, 1, 0 ) ); break;  // 315
    case 8: setLookAt( c + d[1] * Vec3( +0, -1, +0 ), c, Vec3( 1, 0, 0 ) ); break;  //     : +Z: Top
    case 9:
    case 10: setLookAt( c + d[1] * Vec3( +0, +1, +0 ), c, Vec3( 1, 0, 0 ) ); break;  //    : -Z: Botton
    default: printf( "CAMERA: Align not supported iDirection = %d \n", iDirection ); break;
  }
}

void Camera::set( Camera* pcCamera ) {
  m_eRotation = pcCamera->m_eRotation;
  m_eCenter   = pcCamera->m_eCenter;
  m_fDist     = pcCamera->m_fDist;
  m_fBoxSize  = pcCamera->m_fBoxSize;
  m_fFov      = pcCamera->m_fFov;
}

Vec3 Camera::getPosition() { return glm::toMat3( m_eRotation ) * Vec3( 0, 0, m_fDist ) + m_eCenter; }

void Camera::getLookAt( Vec3& ePos, Vec3& eCenter, Vec3& eUp ) const {
  Mat3 mat = glm::toMat3( m_eRotation );
  eCenter  = m_eCenter;
  ePos     = mat * Vec3( 0, 0, m_fDist ) + m_eCenter;
  eUp      = mat * Vec3( 0, 1, 0 );
}

void Camera::setLookAt( Vec3 ePos, Vec3 eCenter, Vec3 eUp ) {
  Vec3 eZ     = glm::normalize( ePos - eCenter );
  Vec3 eX     = glm::normalize( glm::cross( eUp, eZ ) );
  Vec3 eY     = glm::normalize( glm::cross( eZ, eX ) );
  m_eRotation = glm::toQuat( Mat3( eX, eY, eZ ) );
  m_eCenter   = eCenter;
  m_fDist     = glm::length( ePos - eCenter );
}

Quaternion Camera::getRotate( const double fX0, const double fY0, const double fX1, const double fY1 ) {
  Quaternion ret( 1, 0, 0, 0 );
  Vec3       eP0( static_cast<float>( fX0 ), static_cast<float>( fY0 ), 0 );
  Vec3       eP1( static_cast<float>( fX1 ), static_cast<float>( fY1 ), 0 );
  if ( glm::length( eP0 - eP1 ) >= 1.0e-16f ) {
    Vec3  eV0 = glm::normalize( projectToSphere( 0.9f, eP1 ) );
    Vec3  eV1 = glm::normalize( projectToSphere( 0.9f, eP0 ) );
    float fC  = glm::dot( eV1, eV0 );
    if ( fC > -1 ) {
      float fS = std::sqrt( ( 1 + fC ) * 2 );
      eV0      = glm::cross( eV0, eV1 ) * ( 1.f / fS );
      ret      = Quaternion( fS * 0.5f, eV0[0], eV0[1], eV0[2] );
    }
  } else {
  }
  return ret;
}

void Camera::rotate( const double fX0, const double fY0, const double fX1, const double fY1 ) {
  m_eRotation *= getRotate( fX0, fY0, fX1, fY1 );
}
void Camera::rotate( const double angle ) { m_eRotation *= Quaternion( Vec3( 0, angle, 0 ) ); }

void Camera::translate( float x, float y, float z ) { m_eCenter += glm::toMat3( m_eRotation ) * Vec3( x, y, z ); }

void Camera::zoom( float fZoom ) {
  if ( fZoom > 0 ) {
    m_fDist *= 1.f + fZoom / 1500.f;
    if ( m_fDist > 40.f * m_fBoxSize ) { m_fDist = 40.f * m_fBoxSize; }
  } else {
    m_fDist /= 1.f - fZoom / 1500.f;
    if ( m_fDist < 0.05f * m_fBoxSize ) { m_fDist = 0.05f * m_fBoxSize; }
  }
}

Vec3& Camera::projectToSphere( const float& fRadius, Vec3& ePoint ) const {
  const float fD = ePoint[0] * ePoint[0] + ePoint[1] * ePoint[1], fR = fRadius * fRadius;
  if ( fD < fR ) {
    ePoint[2] = std::sqrt( fR - fD );
  } else {
    ePoint[2] = 0;
    ePoint *= fRadius / glm::length( ePoint );
  }
  return ePoint;
}

void Camera::getPerspective( float& fFov, float& fNear, float& fFar ) {
  fFov  = m_fFov;
  fNear = m_fBoxSize / 100.f;
  fFar  = m_fDist + m_fBoxSize * 40.f;
}

void Camera::remove() { std::remove( m_sSyncFile.c_str() ); }

void Camera::write( bool bSynchronize, int iFrameIndex ) {
  std::stringstream ss;
  ss.write( (char const*)&bSynchronize, sizeof( bool ) );
  ss.write( (char const*)&iFrameIndex, sizeof( int32_t ) );
  ss.write( (char const*)&m_eRotation[0], sizeof( float ) );
  ss.write( (char const*)&m_eRotation[1], sizeof( float ) );
  ss.write( (char const*)&m_eRotation[2], sizeof( float ) );
  ss.write( (char const*)&m_eRotation[3], sizeof( float ) );
  ss.write( (char const*)&m_eCenter[0], sizeof( float ) );
  ss.write( (char const*)&m_eCenter[1], sizeof( float ) );
  ss.write( (char const*)&m_eCenter[2], sizeof( float ) );
  ss.write( (char const*)&m_fDist, sizeof( float ) );
  ss.write( (char const*)&m_fFov, sizeof( float ) );
  std::ofstream file( m_sSyncFile.c_str(), std::ofstream::binary );
  if ( !file.is_open() ) {
    printf( "CAMERA: Can't write %s \n", m_sSyncFile.c_str() );
    return;
  }
  file << ss.rdbuf();
  file.close();
}

void Camera::read( bool& bSynchronize, int& iFrameIndex ) {
  std::ifstream file( m_sSyncFile.c_str(), std::ifstream::binary );
  if ( !file.is_open() ) {
    bSynchronize = false;
  } else {
    std::stringstream ss;
    ss << file.rdbuf();
    file.close();
    ss.seekg( 0, std::ios::end );
    if ( ss.tellg() > 0 ) {
      ss.seekg( 0, std::ios::beg );
      ss.read( (char*)&bSynchronize, sizeof( bool ) );
      ss.read( (char*)&iFrameIndex, sizeof( int32_t ) );
      ss.read( (char*)&m_eRotation[0], sizeof( float ) );
      ss.read( (char*)&m_eRotation[1], sizeof( float ) );
      ss.read( (char*)&m_eRotation[2], sizeof( float ) );
      ss.read( (char*)&m_eRotation[3], sizeof( float ) );
      ss.read( (char*)&m_eCenter[0], sizeof( float ) );
      ss.read( (char*)&m_eCenter[1], sizeof( float ) );
      ss.read( (char*)&m_eCenter[2], sizeof( float ) );
      ss.read( (char*)&m_fDist, sizeof( float ) );
      ss.read( (char*)&m_fFov, sizeof( float ) );
    }
  }
}

static inline std::istream& operator>>( std::istream& stream, Vec3& v ) {
  stream >> v[0] >> v[1] >> v[2];
  return stream;
}
static std::ostream& operator<<( std::ostream& stream, Vec3 const& v ) {
#define ALIGN std::setprecision( std::numeric_limits<long double>::digits10 + 1 ) << std::setw( 20 )
  stream << ALIGN << v[0] << " " << ALIGN << v[1] << " " << ALIGN << v[2] << " ";
  return stream;
}

void Camera::writeViewpoint( const std::string& filename ) {
  std::ofstream file( filename.c_str() );
  if ( !file.is_open() ) {
    printf( "CAMERA: Can't write viewpoint file: %s \n", filename.c_str() );
  } else {
    Vec3 eP, eV, eU;
    getLookAt( eP, eV, eU );
    file << eP << eV << eU;
    file.close();
  }
}

void Camera::readViewpoint( const std::string& pFilename ) {
  std::ifstream file( pFilename.c_str() );
  if ( !file.is_open() ) {
    printf( "CAMERA: can't read viewpoint file: %s \n", pFilename.c_str() );
  } else {
    Vec3 eP, eV, eU;
    file >> eP >> eV >> eU;
    setLookAt( eP, eV, eU );
    file.close();
  }
}
