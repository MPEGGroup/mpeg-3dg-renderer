//Copyright(c) 2016 - 2025, InterDigital
//All rights reserved.
//See LICENSE under the root folder.

#include "PccRendererCameraPath.h"
#include "PccRendererCamera.h"
#include "PccRendererObject.h"
#include "PccRendererPrimitive.h"

CameraPath::Spline::Spline() { clear(); }
CameraPath::Spline::~Spline() { clear(); }

void CameraPath::Spline::clear() {
  m_ePos.clear();
  m_eCoef.clear();
}

void CameraPath::Spline::add( int iIndex, float fValue ) { m_ePos.emplace_back( (float)iIndex, fValue ); }
bool CameraPath::Spline::empty() { return m_ePos.empty(); }
void CameraPath::Spline::compute() {
  int                             size = static_cast<int>( m_ePos.size() );
  std::vector<float>              eV( size, 0 ), eH( size, 0 ), eS( size, 0 );
  std::vector<std::vector<float>> eMatrix( size, std::vector<float>( size, 0 ) );
  for ( int i = size - 1; i > 0; i-- ) {
    eV[i]     = ( m_ePos[i][1] - m_ePos[i - 1][1] ) / ( m_ePos[i][0] - m_ePos[i - 1][0] );
    eH[i - 1] = m_ePos[i][0] - m_ePos[i - 1][0];
  }
  for ( int i = 1; i < size - 1; i++ ) {
    eMatrix[i][i] = 2 * ( eH[i - 1] + eH[i] );
    if ( i != 1 ) {
      eMatrix[i][i - 1] = eH[i - 1];
      eMatrix[i - 1][i] = eH[i - 1];
    }
    eMatrix[i][size - 1] = 6 * ( eV[i + 1] - eV[i] );
  }
  for ( int i = 1; i < size - 2; i++ ) {
    float scale = eMatrix[i + 1][i] / eMatrix[i][i];
    for ( int j = 1; j <= size - 1; j++ ) { eMatrix[i + 1][j] -= scale * eMatrix[i][j]; }
  }
  for ( int i = size - 2; i > 0; i-- ) {
    float fSum = 0.f;
    for ( int j = i; j <= size - 2; j++ ) { fSum += eMatrix[i][j] * eS[j]; }
    eS[i] = ( eMatrix[i][size - 1] - fSum ) / eMatrix[i][i];
  }
  for ( int i = 0; i < size - 1; i++ ) {
    m_eCoef.emplace_back( ( eS[i + 1] - eS[i] ) / ( 6 * eH[i] ), eS[i] / 2.f,
                          ( m_ePos[i + 1][1] - m_ePos[i][1] ) / eH[i] - ( 2 * eH[i] * eS[i] + eS[i + 1] * eH[i] ) / 6,
                          m_ePos[i][1] );
  }
}

float CameraPath::Spline::get( int iIndex ) {
  float fX = (float)iIndex;
  for ( int i = 0; i < static_cast<int>( m_ePos.size() ) - 1; i++ ) {
    if ( m_ePos[i][0] <= fX && fX <= m_ePos[i + 1][0] ) {
      return (float)( m_eCoef[i][0] * pow( ( fX - m_ePos[i][0] ), 3 ) +
                      m_eCoef[i][1] * pow( ( fX - m_ePos[i][0] ), 2 ) + m_eCoef[i][2] * ( fX - m_ePos[i][0] ) +
                      m_eCoef[i][3] );
    }
  }
  printf( "CAMERA: Spline %f => -1 Spline define for [ %f; %f]  \n", fX, m_ePos[0][0], m_ePos[m_ePos.size() - 1][0] );
  return -1;
}

CameraPath::CameraPath() : m_bExist( false ), m_bSpline( false ) { m_ePath.clear(); }
CameraPath::~CameraPath() { m_ePath.clear(); }

void CameraPath::initialize() {
  m_ePath.clear();
  for ( auto& spline : m_eSpline ) { spline.clear(); }
  m_iIndex     = 0;
  m_iIndexLast = 0;
  m_iAddIndex  = 0;
  m_bExist     = false;
}

static inline std::istream& operator>>( std::istream& stream, Vec3& v ) {
  stream >> v[0] >> v[1] >> v[2];
  return stream;
}
#define ALIGN std::setprecision( std::numeric_limits<long double>::digits10 + 1 ) << std::setw( 20 )
static std::ostream& operator<<( std::ostream& stream, Vec3 const& v ) {
  stream << ALIGN << v[0] << " " << ALIGN << v[1] << " " << ALIGN << v[2] << " ";
  return stream;
}

bool CameraPath::load( const std::string& pFilename ) {
  std::ifstream file( pFilename.c_str() );
  if ( !file.is_open() ) {
    printf( "CAMERA: camera path file can't be read (%s).\n", pFilename.c_str() );
    fflush( stdout );
    return false;
  }
  for ( std::string line; std::getline( file, line ); ) {
    std::istringstream ss( line );
    ss >> std::ws;
    char c = ss.peek();
    if ( c != EOF && c != '#' ) {
      CameraPosition pos;
      ss >> pos.index() >> pos.pos() >> pos.view() >> pos.up() >> pos.orthographic() >> pos.fixedPosition();
      m_ePath.emplace_back( pos );
    }
  }
  file.close();
  m_bExist = !m_ePath.empty();
  return m_bExist;
}

bool CameraPath::save( const std::string& pFilename, int iStep ) {
  std::ofstream file( pFilename.c_str() );
  if ( !file.is_open() ) {
    printf( "CAMERA: camera path file can't be write (%s).\n", pFilename.c_str() );
    fflush( stdout );
    return false;
  }
  if ( m_ePath.back().fixedPosition() ) {
    auto& p = m_ePath.back();
    m_ePath.emplace_back( p.pos(), p.view(), p.up(), p.orthographic(), p.fixedPosition(), m_iAddIndex );
  }
  if ( m_ePath.back().index() > 0 && m_ePath.back().index() % iStep == 0 ) { m_ePath.back().index()--; }
  file << "#Index PosX PosY PosZ ViewX ViewY ViewZ UpX UpY UpZ Orthographic FixedPosition\n";
  for ( auto& v : m_ePath ) {
    file << std::setw( 5 ) << v.index() << v.pos() << v.view() << v.up() << std::setw( 2 ) << v.orthographic()
         << std::setw( 2 ) << v.fixedPosition() << std::endl;
  }
  file.close();
  return true;
}

void CameraPath::add( Vec3 ePos, Vec3 eView, Vec3 eUp, bool bOrthographic, bool bFixedPosition, int iStep ) {
  auto pos = CameraPosition( ePos, eView, eUp, bOrthographic, bFixedPosition, m_iAddIndex );
  if ( m_ePath.size() == 0 ) {
    m_ePath.push_back( pos );
  } else {
    if ( pos == m_ePath.back() && m_ePath.back().index() != 0 ) {
      m_ePath.back().setIndex( m_iAddIndex );
    } else {
      m_ePath.push_back( pos );
    }
  }
  m_iAddIndex += iStep;
}

bool CameraPath::haveFixedPosition() {
  for ( auto& i : m_ePath ) {
    if ( i.fixedPosition() ) { return true; }
  }
  return false;
}

void CameraPath::addPoint( Box& box, int align, int zoom, bool orthographic, bool fixe, int duration ) {
  Camera camera;
  camera.create( box.getMaxSize() );
  Vec3 ePos, eCenter, eUp;
  camera.align( align, box, zoom );
  camera.getLookAt( ePos, eCenter, eUp );
  add( ePos, eCenter, eUp, orthographic, fixe, duration );
}

void CameraPath::addLine( Box& box, int align, int zoom1, int zoom2, bool orthographic, int duration ) {
  Camera camera;
  camera.create( box.getMaxSize() );
  Vec3 ePos0, eCenter0, eUp0, ePos1, eCenter1, eUp1;
  camera.align( align, box, zoom1 );
  camera.getLookAt( ePos0, eCenter0, eUp0 );
  camera.align( align, box, zoom2 );
  camera.getLookAt( ePos1, eCenter1, eUp1 );
  for ( int i = 0; i < duration + 1; i++ ) {
    float s       = (float)i / (float)duration;
    Vec3  ePos    = ePos0 + ( ePos1 - ePos0 ) * s;
    Vec3  eCenter = eCenter0 + ( eCenter1 - eCenter0 ) * s;
    add( ePos, eCenter, eUp1, orthographic, true, 1 );
  }
}

void CameraPath::addCurve( Box& box, int align1, int zoom1, int align2, int zoom2, bool orthographic, int duration ) {
  Camera camera;
  camera.create( box.getMaxSize() );
  Vec3 ePos0, eCenter0, eUp0, ePos1, eCenter1, eUp1;
  camera.align( align1, box, zoom1 );
  camera.getLookAt( ePos0, eCenter0, eUp0 );
  camera.align( align2, box, zoom2 );
  camera.getLookAt( ePos1, eCenter1, eUp1 );
  Vec3        eVec0 = ePos0 - eCenter0;
  Vec3        eVec1 = ePos1 - eCenter1;
  const float PI2   = glm::pi<float>() / 2.f;
  for ( int i = 0; i < duration + 1; i++ ) {
    float s     = (float)i / (float)duration;
    float angle = PI2 * s;
    float s0    = cos( angle );
    float s1    = sin( angle );
    Vec3  ePos( eCenter0[0] + eVec0[0] * s0 + eVec1[0] * s1, ePos0[1] * ( 1.f - s ) + ePos1[1] * s,
               eCenter0[2] + eVec0[2] * s0 + eVec1[2] * s1 );
    Vec3  eCenter = eCenter0 * ( 1.f - s ) + eCenter1 * s;
    add( ePos, eCenter, eUp1, orthographic, true, 1 );
  }
}

void CameraPath::create( Box& box, size_t index ) {
  initialize();
  switch ( index ) {
    case 0:  // fixed left, fixed front and front zoom in
      addPoint( box, 4, 0, true, true, 90 );
      addPoint( box, 6, 0, true, true, 90 );
      addLine( box, 6, 0, 1, false, 120 );
      break;
    case 1:  // fixed right, fixed front and front zoom in
      addPoint( box, 0, 0, true, true, 90 );
      addPoint( box, 6, 0, true, true, 90 );
      addLine( box, 6, 0, 1, false, 120 );
      break;
    case 2:  // fixed left, fixed front and front zoom in right curved
      addPoint( box, 4, 0, true, true, 90 );
      addPoint( box, 6, 0, true, true, 90 );
      addCurve( box, 6, 0, 0, 2, false, 120 );
      break;
    case 3:  // fixed right, fixed front, front zoom in left curved
      addPoint( box, 0, 0, true, true, 90 );
      addPoint( box, 6, 0, true, true, 90 );
      addCurve( box, 6, 0, 4, 2, false, 120 );
      break;
    case 4:  // fixed left, fixed front and near semicircle to right
      addPoint( box, 4, 0, true, true, 90 );
      addPoint( box, 6, 0, true, true, 90 );
      addCurve( box, 4, 2, 6, 2, false, 60 );
      addCurve( box, 6, 2, 0, 2, false, 60 );
      break;
    case 5:  // fixed right, fixed front and near semicircle to left
      addPoint( box, 0, 0, true, true, 90 );
      addPoint( box, 6, 0, true, true, 90 );
      addCurve( box, 0, 2, 6, 2, false, 60 );
      addCurve( box, 6, 2, 4, 2, false, 60 );
      break;
    case 6:  // fixed left, fixed front and near circle to right
      addPoint( box, 4, 0, true, true, 90 );
      addPoint( box, 6, 0, true, true, 90 );
      addCurve( box, 6, 2, 0, 2, false, 30 );
      addCurve( box, 0, 2, 2, 2, false, 30 );
      addCurve( box, 2, 2, 4, 2, false, 30 );
      addCurve( box, 4, 2, 6, 2, false, 30 );
      break;
    case 7:  // fixed right, fixed front and near circle to left
      addPoint( box, 0, 0, true, true, 90 );
      addPoint( box, 6, 0, true, true, 90 );
      addCurve( box, 6, 2, 4, 2, false, 30 );
      addCurve( box, 4, 2, 2, 2, false, 30 );
      addCurve( box, 2, 2, 0, 2, false, 30 );
      addCurve( box, 0, 2, 6, 2, false, 30 );
      break;
    case 8:  // fixed left, fixed front and upward spiral to the right
      addPoint( box, 4, 0, true, true, 90 );
      addPoint( box, 6, 0, true, true, 90 );
      addCurve( box, 6, 5, 0, 4, false, 30 );
      addCurve( box, 0, 4, 2, 3, false, 30 );
      addCurve( box, 2, 3, 4, 2, false, 30 );
      addCurve( box, 4, 2, 6, 1, false, 30 );
      break;
    case 9:  // fixed right, fixed front and upward spiral to the left
      addPoint( box, 0, 0, true, true, 90 );
      addPoint( box, 6, 0, true, true, 90 );
      addCurve( box, 6, 5, 4, 4, false, 30 );
      addCurve( box, 4, 4, 2, 3, false, 30 );
      addCurve( box, 2, 3, 0, 2, false, 30 );
      addCurve( box, 0, 2, 6, 1, false, 30 );
      break;
    case 10:  // Left curved zoom for longdress, soldier, mitch, thomas, levi sequences
      addPoint( box, 2, 0, false, true, 60 );
      addCurve( box, 2, 0, 1, 1, false, 180 );
      addPoint( box, 1, 1, false, true, 59 );
      addPoint( box, 1, 1, false, true, 1 );
      break;
    case 11:  // Zoom in for basketball sequence
      addPoint( box, 6, 0, false, true, 60 );
      addLine( box, 6, 0, 1, false, 180 );
      addLine( box, 6, 1, 2, false, 59 );
      break;
    case 12:  // Zoom in for dancer sequence
      addPoint( box, 6, 0, false, true, 60 );
      addLine( box, 6, 0, 1, false, 180 );
      addPoint( box, 6, 1, false, true, 59 );
      addPoint( box, 6, 1, false, true, 1 );
      break;
    case 13:  // Zoom in for football sequence
      addPoint( box, 0, 0, false, true, 60 );
      addLine( box, 0, 0, 1, false, 180 );
      addPoint( box, 0, 1, false, true, 59 );
      addPoint( box, 0, 1, false, true, 1 );
      break;
    case 14:  // front, zoom in inleft curve and near front/right
      addCurve( box, 2, 0, 1, 1, false, 19 );
      addPoint( box, 1, 1, false, true, 1 );
      break;
  }
  m_bExist = true;
}

void CameraPath::initSpline() {
  if ( m_eSpline[0].empty() ) {
    for ( auto& i : m_ePath ) {
      for ( int32_t j = 0; j < 3; j++ ) { m_eSpline[j + 0].add( i.index(), i.pos()[j] ); }
      for ( int32_t j = 0; j < 3; j++ ) { m_eSpline[j + 3].add( i.index(), i.view()[j] ); }
      for ( int32_t j = 0; j < 3; j++ ) { m_eSpline[j + 6].add( i.index(), i.up()[j] ); }
    }
    for ( auto& i : m_eSpline ) { i.compute(); }
  }
}

void CameraPath::getPose( Vec3& ePos, Vec3& eView, Vec3& eUp, bool& bSpline, bool& bOrthographic, int iIndex ) {
  int size = static_cast<int>( m_ePath.size() );
  if ( iIndex == -1 ) { iIndex = m_iIndex; }
  if ( bSpline && size > 1 && haveFixedPosition() ) {
    printf( "Spline interpolation is disabled because fixed position points are used in the camera path \n" );
    fflush( stdout );
    bSpline = false;
  }
  if ( bSpline && size > 1 ) {
    initSpline();
    for ( int j = 0; j < 3; j++ ) { ePos[j] = m_eSpline[j + 0].get( iIndex ); }
    for ( int j = 0; j < 3; j++ ) { eView[j] = m_eSpline[j + 3].get( iIndex ); }
    for ( int j = 0; j < 3; j++ ) { eUp[j] = m_eSpline[j + 6].get( iIndex ); };
  } else {
    for ( int i = 0; i < size - 1; i++ ) {
      if ( m_ePath[i].index() <= iIndex && iIndex < m_ePath[i + 1].index() ) {
        if ( m_ePath[i].fixedPosition() ) {
          ePos  = m_ePath[i].pos();
          eView = m_ePath[i].view();
          eUp   = m_ePath[i].up();
        } else {
          auto fScale = (float)( iIndex - m_ePath[i].index() ) / (float)( m_ePath[i + 1].index() - m_ePath[i].index() );
          ePos        = m_ePath[i].pos() + ( m_ePath[i + 1].pos() - m_ePath[i].pos() ) * fScale;
          eView       = m_ePath[i].view() + ( m_ePath[i + 1].view() - m_ePath[i].view() ) * fScale;
          eUp         = m_ePath[i].up() + ( m_ePath[i + 1].up() - m_ePath[i].up() ) * fScale;
        }
        bOrthographic = m_ePath[i].orthographic();
        return;
      }
    }
    ePos          = m_ePath.back().pos();
    eView         = m_ePath.back().view();
    eUp           = m_ePath.back().up();
    bOrthographic = m_ePath.back().orthographic();
  }
}

float CameraPath::getDistance() {
  Vec3 ePos, eView;
  bool bFound = false;
  for ( int32_t i = 0; i < static_cast<int32_t>( m_ePath.size() ) - 1; i++ ) {
    if ( m_iIndex >= m_ePath[i].index() && m_iIndex < m_ePath[i + 1].index() ) {
      if ( m_ePath[i].fixedPosition() ) {
        ePos  = m_ePath[i].pos();
        eView = m_ePath[i].view();
      } else {
        auto fScale = (float)( m_iIndex - m_ePath[i].index() ) / (float)( m_ePath[i + 1].index() - m_ePath[i].index() );
        ePos        = m_ePath[i].pos() + ( m_ePath[i + 1].pos() - m_ePath[i].pos() ) * fScale;
        eView       = m_ePath[i].view() + ( m_ePath[i + 1].view() - m_ePath[i].view() ) * fScale;
      }
      bFound = true;
      break;
    }
  }
  if ( !bFound ) {
    ePos  = m_ePath.back().pos();
    eView = m_ePath.back().view();
  }
  return glm::length( ePos - eView );
}

void CameraPath::getListOfPoints( std::vector<Vec3>&   points,
                                  std::vector<Color3>& color,
                                  std::vector<Vec3>&   direction,
                                  bool&                bSpline ) {
  if ( m_bExist ) {
    Vec3   ePos, eView, eUp;
    bool   bOrtho     = false;
    size_t iNumPoints = m_ePath.size();
    for ( size_t i = 0; i < iNumPoints - 1; i++ ) {
      int iIndex0   = m_ePath[i].index();
      int iIndex1   = i + 1 != iNumPoints ? m_ePath[i + 1].index() : iIndex0 + 1;
      int iNumIndex = iIndex1 - iIndex0;
      for ( int j = 0; j < iNumIndex; j++ ) {
        getPose( ePos, eView, eUp, bSpline, bOrtho, iIndex0 + j );
        points.push_back( ePos );
        color.push_back( j == 0 ? Color3( bOrtho ? 0.9f : 0.7f, bOrtho ? 0.7f : 0.9f, 0.7f )
                                : Color3( 0.3f, 0.3f, 0.3f ) );
        direction.push_back( ePos + ( eView - ePos ) / 10.f );
      }
    }
    getPose( ePos, eView, eUp, bSpline, bOrtho, m_ePath[iNumPoints - 1].index() );
    points.push_back( ePos );
    color.push_back( Color3( bOrtho ? 0.9f : 0.7f, bOrtho ? 0.7f : 0.9f, 0.7f ) );
    direction.push_back( ePos + ( eView - ePos ) / 10.f );
  }
}
