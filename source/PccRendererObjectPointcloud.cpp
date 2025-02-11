//Copyright(c) 2016 - 2025, InterDigital
//All rights reserved.
//See LICENSE under the root folder.

#include "PccRendererObjectPointcloud.h"
#include "PccRendererShader.h"
#include "PccRendererPrimitive.h"

#include <sys/stat.h>
#include <sys/types.h>
#include <cerrno>
#include <map>

#include "nanoflann.hpp"
#include "KDTreeVectorOfVectorsAdaptor.h"

using namespace nanoflann;

#ifdef WIN32
#include <windows.h>
void mkdir( const char* pDirectory, int ) { CreateDirectory( pDirectory, NULL ); }
#else
#include <sys/dir.h>
#endif

std::vector<Program> ObjectPointcloud::m_ePrograms     = {};
int32_t              ObjectPointcloud::m_iProgramIndex = 0;

// Point vertex shader
// clang-format off
static const std::string g_pPointVertexShader = SHADER(
  uniform mat4  ProjMat;
  uniform mat4  ModMat;
  uniform int   count;
  uniform int   mode;
  uniform vec3  posCamera;  
  uniform vec3  posRig[ 14 ];
  uniform float PointSize;
  in  vec3      point;
  in  vec4      color;
  layout(location = 0) in vec4 colorM[ 14 ];
  out vec4      vColor;
  void main() {          
    vColor = color;  
    if( count > 0 &&  mode != -3 ) {
      if ( mode == -1 ) { 
        float sumW = 0; 
        vColor = vec4(0.0, 0.0, 0.0, 1.0);
        vec3 cam = normalize( posCamera - point ); 
        for(int i=0;i<count;i++) {
          float w = pow( max( 0.f, dot( cam, normalize( posRig[i] - point ) ) ), 10.f );
          vColor += w * colorM[i];
          sumW   += w;   
        }
        if( sumW > 0.000001f ) { vColor /= sumW; } else { vColor = color; }
      } else {
        vColor = colorM[mode];  
      }      
    }
    gl_Position = ProjMat * ModMat * vec4( point, 1.f );
    gl_PointSize = PointSize *  4750.0 / gl_Position.w;
  }
);

// Surface vertex shader (cube and splat)
static const std::string g_pSurfaceVertexShader = SHADER(
  uniform int   count;
  uniform int   mode;
  uniform vec3  posCamera;  
  uniform vec3  posRig[ 14 ];
  in  vec3      point;
  in  vec4      color;
  layout(location = 0)  in  vec4 colorM[ 14 ];
  out vec4      vColor;
  void main() {
    vColor = color;    
    if( count > 0 &&  mode != -3 ) {
      if ( mode == -1 ) { 
        float sumW = 0; 
        vColor = vec4(0.0, 0.0, 0.0, 1.0);
        vec3 cam = normalize( posCamera - point ); 
        for(int i=0;i<count;i++) {
          float w = pow( max( 0.f, dot( cam, normalize( posRig[i] - point ) ) ), 10.f );
          vColor += w * colorM[i];
          sumW   += w;   
        }
        if( sumW > 0.000001f ) { vColor /= sumW; } else { vColor = color; }
      } else {
        vColor = colorM[mode];  
      }      
    }
    gl_Position = vec4( point, 1.f );   
  }
);

// Splat geometry shaders
static const std::string g_pSplatGeometryShader = SHADER(
  layout(points) in;
  layout( triangle_strip, max_vertices = 4) out;  
  uniform mat4  ProjMat;
  uniform mat4  ModMat;
  uniform float PointSize;
  in  vec4  vColor [];
  out vec4  fColor;
  out vec2  fCoord;
  void main() {
    for ( int i = 0; i < gl_in.length(); i++)  {
      vec4 pos =  ProjMat * ModMat * gl_in[i].gl_Position;
      fColor  = vColor[i];
      fCoord  = vec2(  1,-1 ); gl_Position = pos + ProjMat * ( PointSize * vec4(  1, -1, 0, 0 ) ); EmitVertex();
      fCoord  = vec2(  1, 1 ); gl_Position = pos + ProjMat * ( PointSize * vec4(  1,  1, 0, 0 ) ); EmitVertex();
      fCoord  = vec2( -1,-1 ); gl_Position = pos + ProjMat * ( PointSize * vec4( -1, -1, 0, 0 ) ); EmitVertex();
      fCoord  = vec2( -1, 1 ); gl_Position = pos + ProjMat * ( PointSize * vec4( -1,  1, 0, 0 ) ); EmitVertex();
      EndPrimitive();
    } 
  }
);

// Blended splat geometry shaders
static const std::string g_pBlendedSplatGeometryShader = SHADER(
    layout(points) in;
layout(triangle_strip, max_vertices = 3) out;
uniform mat4  ProjMat;
uniform mat4  ModMat;
uniform float PointSize;
in  vec4  vColor[];
out vec4  fColor;
out vec2  fCoord;
const vec2 coord[] = vec2[3]( vec2(0.0,2.0), vec2(sqrt(3.0),-1.0), vec2(-sqrt(3.0),-1.0));
void main() {
    for (int i = 0; i < gl_in.length(); i++) {
        vec4 pos = ProjMat * ModMat * gl_in[i].gl_Position;
        fColor = vColor[i];
        fCoord = coord[0]; gl_Position = pos + ProjMat * (PointSize * vec4(coord[0], 0, 0)); EmitVertex();
        fCoord = coord[1]; gl_Position = pos + ProjMat * (PointSize * vec4(coord[1], 0, 0)); EmitVertex();
        fCoord = coord[2]; gl_Position = pos + ProjMat * (PointSize * vec4(coord[2], 0, 0)); EmitVertex();
        EndPrimitive();
    }
}
);


// Cube geometry shaders
static const std::string g_pCubeGeometryShader = SHADER( 
  layout( points ) in;
  layout( triangle_strip, max_vertices = 32 ) out;
  uniform mat4 ProjMat;
  uniform mat4 ModMat;
  uniform float PointSize;
  uniform int   DepthMap;
  in  vec4 vColor[];
  out vec4 fColor;
  float fMin = 1000.f;
  float fMax = 4000.f;
  void main() {    
    float d = PointSize / 2.f; 
    for( int i = 0; i < gl_in.length(); i++ ) {
      mat4 eMat = ProjMat * ModMat;
      vec4 ePos = eMat * gl_in[i].gl_Position;
      if( DepthMap == 1 ) {
        float value = 0.2f + 0.6f * ( ePos[2] - fMin ) / ( fMax - fMin ); 
        if      ( ePos[2] > fMax )  fColor = vec4( 0.8f, 0.8f, 0.8f, 1.f );
        else if ( ePos[2] < fMin )  fColor = vec4( 0.2f, 0.2f, 0.2f, 1.f ) ;
        else                        fColor = vec4( value, value, value, 1.f );
      }  else {
        fColor = vColor[i];
      }     
      gl_Position = ePos + eMat * vec4(  d, -d, -d, 0 ); EmitVertex(); 
      gl_Position = ePos + eMat * vec4( -d, -d, -d, 0 ); EmitVertex(); 
      gl_Position = ePos + eMat * vec4(  d,  d, -d, 0 ); EmitVertex(); 
      gl_Position = ePos + eMat * vec4( -d,  d, -d, 0 ); EmitVertex(); 
      gl_Position = ePos + eMat * vec4(  d,  d,  d, 0 ); EmitVertex(); 
      gl_Position = ePos + eMat * vec4( -d,  d,  d, 0 ); EmitVertex(); 
      gl_Position = ePos + eMat * vec4(  d, -d,  d, 0 ); EmitVertex(); 
      gl_Position = ePos + eMat * vec4( -d, -d,  d, 0 ); EmitVertex(); EndPrimitive();
      gl_Position = ePos + eMat * vec4(  d,  d, -d, 0 ); EmitVertex(); 
      gl_Position = ePos + eMat * vec4(  d,  d,  d, 0 ); EmitVertex(); 
      gl_Position = ePos + eMat * vec4(  d, -d, -d, 0 ); EmitVertex(); 
      gl_Position = ePos + eMat * vec4(  d, -d,  d, 0 ); EmitVertex(); 
      gl_Position = ePos + eMat * vec4( -d, -d, -d, 0 ); EmitVertex(); 
      gl_Position = ePos + eMat * vec4( -d, -d,  d, 0 ); EmitVertex(); 
      gl_Position = ePos + eMat * vec4( -d,  d, -d, 0 ); EmitVertex(); 
      gl_Position = ePos + eMat * vec4( -d,  d,  d, 0 ); EmitVertex(); EndPrimitive();
    } 
  }
);

// Splat fragment shaders
static const std::string g_pSplatFragmentShaderRTT = SHADER(
  in  vec4  fColor;
  uniform float forceColor;
  in  vec2 fCoord;
  layout(location = 0) out vec4  color; 
  layout(location = 1) out float depth; 
  void main() { 
    if( exp( -dot( fCoord, fCoord ) ) < 0.5 ) { 
      discard; 
    } else { 
      if      ( forceColor <  0.5 ) { color = fColor; } 
      else { color = vec4( vec3( ( forceColor - 1.f ) / 3.f ), 1.0 ); }
    } 
    depth = gl_FragCoord.z;
  }
);

// Point fragment shaders
static const std::string g_pPointFragmentShaderRTT = SHADER( 
  in  vec4  vColor;
  uniform float forceColor;
  layout(location = 0) out vec4  color; 
  layout(location = 1) out float depth; 
  void main() {
    if ( forceColor <  0.5 ) { color = vColor; } 
    else { color = vec4( vec3( ( forceColor - 1.f ) / 3.f ), 1.0 ); }
    depth = gl_FragCoord.z;
  }
);

// Cube fragment shaders
static const std::string g_pCubeFragmentShaderRTT = SHADER(
  in  vec4  fColor;
  uniform float forceColor;
  layout(location = 0) out vec4  color; 
  layout(location = 1) out float depth; 
  void main() {
    if ( forceColor <  0.5 ) { color = fColor; } 
    else { color = vec4( vec3( ( forceColor - 1.f ) / 3.f ), 1.0 ); }
    depth = gl_FragCoord.z;
  }
);

// Splat fragment shaders
static const std::string g_pSplatFragmentShader = SHADER(
  in  vec4  fColor;
  uniform float forceColor;
  in  vec2 fCoord;
  out vec4 color;
  void main() { 
    if( exp( -dot( fCoord, fCoord ) ) < 0.5 ) { 
      discard; 
    } else { 
      if      ( forceColor <  0.5 ) { color = fColor; } 
      else { color = vec4( vec3( ( forceColor - 1.f ) / 3.f ), 1.0 ); }
    } 
  }
);

// Blended splat fragment shaders
static const std::string g_pBlendedSplatFragmentShader = SHADER(
    in  vec4  fColor;
uniform float forceColor;
uniform int   iBlendMode;
uniform float fAlphaFalloff;
in  vec2 fCoord;
out vec4 color;
void main() {
  float sq_norm = fCoord.x * fCoord.x + fCoord.y * fCoord.y;
  if (sq_norm > 1.0) { discard; }
  if (forceColor < 0.5) { color = fColor;} 
  else { color = vec4(vec3((forceColor - 1.f) / 3.f), 1.0); }
  if (iBlendMode == 0) { color.a = exp(-sq_norm * fAlphaFalloff); }
  else { color.a = 1.0-clamp(sqrt(sq_norm* fAlphaFalloff), 0.0, 1.0); }
}
);

// Point fragment shaders
static const std::string g_pPointFragmentShader = SHADER( 
  in  vec4  vColor;
  uniform float forceColor;
  out vec4 color;
  void main() {
    if ( forceColor <  0.5 ) { color = vColor; } 
    else { color = vec4( vec3( ( forceColor - 1.f ) / 3.f ), 1.0 ); }
  }
);

// Cube fragment shaders
static const std::string g_pCubeFragmentShader = SHADER(
  in  vec4  fColor;
  uniform float forceColor;
  out vec4 color;
  void main() {
    if ( forceColor <  0.5 ) { color = fColor; } 
    else { color = vec4( vec3( ( forceColor - 1.f ) / 3.f ), 1.0 ); }
  }
);
// clang-format on

RigParameters::RigParameters() { m_eMatrix.clear(); }
RigParameters::~RigParameters() { m_eMatrix.clear(); }

void RigParameters::trace() {
  int32_t i   = 0;
  auto&   vec = m_fFrameToWorldTranslation;
  printf( "RigParameters: \n" );
  printf( "   - Count                   = %zu \n", m_eMatrix.size() );
  printf( "   - Width                   = %12.8f \n", m_fWidth );
  printf( "   - FrameToWorldScale       = %12.8f \n", m_fFrameToWorldScale );
  printf( "   - FrameToWorldTranslation = %12.8f %12.8f %12.8f \n", vec[0], vec[1], vec[2] );
  for ( auto& m : m_eMatrix ) {
    printf( "   - Matrix %4d              = %12.8f %12.8f %12.8f %12.8f \n", i++, m[0][0], m[0][1], m[0][2], m[0][3] );
    printf( "                               %12.8f %12.8f %12.8f %12.8f \n", m[1][0], m[1][1], m[1][2], m[1][3] );
    printf( "                               %12.8f %12.8f %12.8f %12.8f \n", m[2][0], m[2][1], m[2][2], m[2][3] );
    printf( "                               %12.8f %12.8f %12.8f %12.8f \n", m[3][0], m[3][1], m[3][2], m[3][3] );
  }
}

ObjectPointcloud::ObjectPointcloud() { reset(); }
ObjectPointcloud::~ObjectPointcloud() { reset(); }

void ObjectPointcloud::allocate( bool bAlpha,
                                 bool bNormal,
                                 bool bType,
                                 int  iNumPoints,
                                 int  iFrameIndex,
                                 int  iNumDuplicate ) {
  m_pPoints.clear();
  m_pColors3.clear();
  m_pColors4.clear();
  m_pNormals.clear();
  m_pTypes.clear();
  m_bAlpha        = bAlpha;
  m_bNormal       = bNormal;
  m_bType         = bType;
  m_bSort         = false;
  m_iNumPoints    = iNumPoints;
  m_iNumDuplicate = iNumDuplicate;
  m_iFrameIndex   = iFrameIndex;
  m_iIndex        = 0;
  m_eBox          = Box();
  m_pPoints.resize( iNumPoints );
  if ( m_bAlpha ) {
    m_pColors4.resize( iNumPoints );
  } else {
    m_pColors3.resize( iNumPoints );
  }
  if ( m_bNormal ) { m_pNormals.resize( iNumPoints ); }
  if ( m_bType ) { m_pTypes.resize( iNumPoints ); }
  if ( m_eRigParameters.getCount() > 0 ) { m_pMultiColors3.resize( m_eRigParameters.getCount() * iNumPoints ); }
}

void ObjectPointcloud::setBox( float fXMin, float fXMax, float fYMin, float fYMax, float fZMin, float fZMax ) {
  m_eBox = Box( Vec3( fXMin, fYMin, fZMin ), Vec3( fXMax, fYMax, fZMax ) );
}

void ObjectPointcloud::center( Box box, float fBoxSize ) {
  Vec3 center = fBoxSize / 2.f - ( box.min() + ( box.max() - box.min() ) / 2.f );
  for ( auto& ePoint : m_pPoints ) { ePoint += center; }
}

void ObjectPointcloud::scale( Box box, float fBoxSize ) {
  if ( fBoxSize != 0.f ) {
    float fScale = fBoxSize / box.getMaxSize();
    for ( auto& ePoint : m_pPoints ) { ePoint = ( ePoint - box.min() ) * fScale; }
  } else {
    printf( "ObjectPointcloud: ignore scale to box of size 0\n" );
  }
}

void ObjectPointcloud::recomputeBoundingBox() {
  m_eBox = Box();
  for ( auto& ePoint : m_pPoints ) { m_eBox.update( ePoint ); }
}

void ObjectPointcloud::removeDuplicatePoints( int iDropDups ) {
  if ( iDropDups == 0 ) { return; }
  int    iNumDuplicate   = 0;
  size_t iNumMultiColors = m_eRigParameters.getCount();
  struct Coordinate {
    int m_iIndex;
    int m_iNumber;
  };
  std::map<float, std::map<float, std::map<float, Coordinate>>> eMap;
  for ( int i = 0; i < m_iNumPoints; i++ ) {
    float x = m_pPoints[i][0], y = m_pPoints[i][1], z = m_pPoints[i][2];
    if ( eMap.find( x ) != eMap.end() && eMap[x].find( y ) != eMap[x].end() &&
         eMap[x][y].find( z ) != eMap[x][y].end() ) {
      Coordinate& eCoordinate = eMap[x][y][z];
      if ( iDropDups == 2 ) {
        if ( m_bAlpha ) {
          m_pColors4[eCoordinate.m_iIndex] += m_pColors4[i];
        } else {
          m_pColors3[eCoordinate.m_iIndex] += m_pColors3[i];
        }
        if ( iNumMultiColors ) {
          for ( size_t k = 0; k < iNumMultiColors; k++ ) {
            m_pMultiColors3[eCoordinate.m_iIndex * iNumMultiColors + k] += m_pMultiColors3[i * iNumMultiColors + k];
          }
        }
        eCoordinate.m_iNumber++;
      }
      iNumDuplicate++;
    } else {
      Coordinate eCoordinate;
      eCoordinate.m_iIndex  = i;
      eCoordinate.m_iNumber = 1;
      eMap[x][y][z]         = eCoordinate;
    }
  }

  if ( iNumDuplicate > 0 ) {
    std::vector<Point>   pNewPoints;
    std::vector<Color3>  pNewColors3;
    std::vector<Color4>  pNewColors4;
    std::vector<Normal>  pNewNormals;
    std::vector<uint8_t> pNewTypes;
    std::vector<Color3>  pNewMultiColors3;
    m_iNumDuplicate   = iNumDuplicate;
    int iNewNumPoints = m_iNumPoints - iNumDuplicate;
    pNewPoints.resize( iNewNumPoints );
    if ( m_bAlpha ) {
      pNewColors4.resize( iNewNumPoints );
    } else {
      pNewColors3.resize( iNewNumPoints );
    }
    if ( m_bNormal ) { pNewNormals.resize( iNewNumPoints ); }
    if ( m_bType ) { pNewTypes.resize( iNewNumPoints ); }
    if ( iNumMultiColors ) { pNewMultiColors3.resize( iNewNumPoints * iNumMultiColors ); }
    size_t iIndex = 0;
    for ( int i = 0; i < m_iNumPoints; i++ ) {
      float       x = m_pPoints[i][0], y = m_pPoints[i][1], z = m_pPoints[i][2];
      Coordinate& eCoordinate = eMap[x][y][z];
      if ( i == eCoordinate.m_iIndex ) {
        if ( eCoordinate.m_iNumber > 1 ) {
          if ( m_bAlpha ) {
            m_pColors4[i] /= static_cast<float>( eCoordinate.m_iNumber );
          } else {
            m_pColors3[i] /= static_cast<float>( eCoordinate.m_iNumber );
          }
          if ( iNumMultiColors ) {
            for ( size_t k = 0; k < iNumMultiColors; k++ ) {
              m_pMultiColors3[i * iNumMultiColors + k] /= static_cast<float>( eCoordinate.m_iNumber );
            }
          }
        }
        pNewPoints[iIndex] = m_pPoints[i];
        if ( m_bAlpha ) {
          pNewColors4[iIndex] = m_pColors4[i];
        } else {
          pNewColors3[iIndex] = m_pColors3[i];
        }
        if ( m_bNormal ) { pNewNormals[iIndex] = m_pNormals[i]; }
        if ( m_bType ) { pNewTypes[iIndex] = m_pTypes[i]; }
        if ( iNumMultiColors > 0 ) {
          for ( size_t k = 0; k < iNumMultiColors; k++ ) {
            pNewMultiColors3[iIndex * iNumMultiColors + k] = m_pMultiColors3[i * iNumMultiColors + k];
          }
        }
        iIndex++;
      }
    }
    m_pPoints.clear();
    if ( m_bAlpha ) {
      m_pColors4.clear();
    } else {
      m_pColors3.clear();
    }
    if ( m_bNormal ) { m_pNormals.clear(); }
    if ( m_bType ) { m_pTypes.clear(); }
    if ( iNumMultiColors > 0 ) { m_pMultiColors3.clear(); }
    m_pPoints.swap( pNewPoints );
    if ( m_bAlpha ) {
      m_pColors4.swap( pNewColors4 );
    } else {
      m_pColors3.swap( pNewColors3 );
    }
    if ( m_bNormal ) { m_pNormals.swap( pNewNormals ); }
    if ( m_bType ) { m_pTypes.swap( pNewTypes ); }
    if ( iNumMultiColors > 0 ) { m_pMultiColors3.swap( pNewMultiColors3 ); }
    m_iNumPoints = iNewNumPoints;
  }
}

void ObjectPointcloud::sortVertex(const Camera &cam) {
    m_bSort = true;
    std::vector<unsigned int> sorted_index;
    std::vector<float> point_distance;
    sorted_index.resize(m_iNumPoints);
    point_distance.resize(m_iNumPoints);
    Vec3 pos, center, up, norm;
    cam.getLookAt(pos, center, up);
    norm = glm::normalize(center - pos);

    for (int i = 0; i < m_iNumPoints; i++) {
        sorted_index[i] = i;
        Vec3 v = m_pPoints[i]-pos;
        point_distance[i] = glm::dot(v, norm);
    }
    std::sort(sorted_index.begin(), sorted_index.end(), [&point_distance](int i, int j){
        return ((point_distance[i] > point_distance[j]) || (point_distance[i] == point_distance[j]) && i>j);
    });
    glBindVertexArray(m_uiVAO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_uiIBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sorted_index.size() * sizeof(unsigned int), &sorted_index[0], GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void ObjectPointcloud::createBinaryDirectory( const std::string& sString ) { mkdir( sString.c_str(), 0777 ); }

bool ObjectPointcloud::readBinary( const std::string& pString, int iFrameIndex ) {
  bool          bError = false;
  std::ifstream infile;
  infile.open( pString.c_str(), std::ios::binary | std::ios::in );
  if ( !infile || !infile.is_open() ) { return false; }
  infile.seekg( 0, std::ifstream::end );
  auto length = infile.tellg();
  infile.seekg( 0, std::ifstream::beg );
  float fXMin = 0.f, fXMax = 0.f, fYMin = 0.f, fYMax = 0.f, fZMin = 0.f, fZMax = 0.f;
  infile.read( reinterpret_cast<char*>( &m_iNumPoints ), sizeof( uint32_t ) );
  infile.read( reinterpret_cast<char*>( &m_iNumDuplicate ), sizeof( uint32_t ) );
  infile.read( reinterpret_cast<char*>( &m_bAlpha ), sizeof( bool ) );
  infile.read( reinterpret_cast<char*>( &m_bNormal ), sizeof( bool ) );
  infile.read( reinterpret_cast<char*>( &m_bType ), sizeof( bool ) );
  infile.read( reinterpret_cast<char*>( &fXMin ), sizeof( float ) );
  infile.read( reinterpret_cast<char*>( &fXMax ), sizeof( float ) );
  infile.read( reinterpret_cast<char*>( &fYMin ), sizeof( float ) );
  infile.read( reinterpret_cast<char*>( &fYMax ), sizeof( float ) );
  infile.read( reinterpret_cast<char*>( &fZMin ), sizeof( float ) );
  infile.read( reinterpret_cast<char*>( &fZMax ), sizeof( float ) );
  uint32_t iCountMultiColors = 0;
  infile.read( reinterpret_cast<char*>( &iCountMultiColors ), sizeof( uint32_t ) );
  if ( bError || iCountMultiColors > 255 ) {
    reset();
    printf( "PLYREADER: Error in binary file reading process. \n" );
    fflush( stdout );
    infile.close();
    std::remove( pString.c_str() );
    return false;
  }
  m_eRigParameters.setCount( iCountMultiColors );
  allocate( m_bAlpha, m_bNormal, m_bType, m_iNumPoints, iFrameIndex, m_iNumDuplicate );
  setBox( fXMin, fXMax, fYMin, fYMax, fZMin, fZMax );
  infile.read( reinterpret_cast<char*>( getPoints() ), m_iNumPoints * sizeof( float ) * 3 );
  infile.read( static_cast<char*>( getColors() ), m_iNumPoints * sizeof( float ) * ( 3 + m_bAlpha ) );
  if ( m_bNormal ) { infile.read( reinterpret_cast<char*>( getNormals() ), m_iNumPoints * sizeof( float ) * 3 ); }
  if ( m_bType ) { infile.read( reinterpret_cast<char*>( getTypes() ), m_iNumPoints * sizeof( uint8_t ) ); }

  if ( iCountMultiColors > 0 ) {
    float scale, width;
    Vec3  translation;
    infile.read( reinterpret_cast<char*>( &scale ), sizeof( float ) );
    infile.read( reinterpret_cast<char*>( &translation.x ), sizeof( float ) * 3 );
    infile.read( reinterpret_cast<char*>( &width ), sizeof( float ) );
    m_eRigParameters.setFrameToWorldTranslation( translation );
    m_eRigParameters.setFrameToWorldScale( scale );
    m_eRigParameters.setWidth( width );
    for ( size_t i = 0; i < iCountMultiColors; i++ ) {
      infile.read( reinterpret_cast<char*>( &m_eRigParameters.getMatrix( i ) ), sizeof( float ) * 16 );
    }
    infile.read( reinterpret_cast<char*>( m_pMultiColors3.data() ),
                 m_iNumPoints * iCountMultiColors * sizeof( float ) * 3 );
  }
  if ( length != infile.tellg() ) {
    bError = true;
    printf( "error file have not a correct size  \n" );
  }
  if ( !infile ) { bError = true; }
  if ( bError ) {
    reset();
    printf( "PLYREADER: Error in binary file reading process. \n" );
    infile.close();
    std::remove( pString.c_str() );
    return false;
  }
  infile.close();
  return true;
}

void ObjectPointcloud::writeBinary( const std::string& filename ) {
  std::ofstream outfile;
  std::string   directory = getDirectory( filename );
  if ( !dirExists( directory ) ) { createBinaryDirectory( directory ); }
  outfile.open( filename.c_str(), std::ios::binary | std::ios::out );
  float fXMin = m_eBox.min()[0], fYMin = m_eBox.min()[1], fZMin = m_eBox.min()[2];
  float fXMax = m_eBox.max()[0], fYMax = m_eBox.max()[1], fZMax = m_eBox.max()[2];
  outfile.write( reinterpret_cast<char*>( &m_iNumPoints ), sizeof( uint32_t ) );
  outfile.write( reinterpret_cast<char*>( &m_iNumDuplicate ), sizeof( uint32_t ) );
  outfile.write( reinterpret_cast<char*>( &m_bAlpha ), sizeof( bool ) );
  outfile.write( reinterpret_cast<char*>( &m_bNormal ), sizeof( bool ) );
  outfile.write( reinterpret_cast<char*>( &m_bType ), sizeof( bool ) );
  outfile.write( reinterpret_cast<char*>( &fXMin ), sizeof( float ) );
  outfile.write( reinterpret_cast<char*>( &fXMax ), sizeof( float ) );
  outfile.write( reinterpret_cast<char*>( &fYMin ), sizeof( float ) );
  outfile.write( reinterpret_cast<char*>( &fYMax ), sizeof( float ) );
  outfile.write( reinterpret_cast<char*>( &fZMin ), sizeof( float ) );
  outfile.write( reinterpret_cast<char*>( &fZMax ), sizeof( float ) );
  auto iCountMultiColors = static_cast<uint32_t>( m_eRigParameters.getCount() );
  outfile.write( reinterpret_cast<char*>( &iCountMultiColors ), sizeof( uint32_t ) );
  outfile.write( reinterpret_cast<char*>( getPoints() ), m_iNumPoints * sizeof( float ) * 3 );
  outfile.write( static_cast<char*>( getColors() ), m_iNumPoints * sizeof( float ) * ( 3 + m_bAlpha ) );
  if ( m_bNormal ) { outfile.write( reinterpret_cast<char*>( getNormals() ), m_iNumPoints * sizeof( float ) * 3 ); }
  if ( m_bType ) { outfile.write( reinterpret_cast<char*>( getTypes() ), m_iNumPoints * sizeof( uint8_t ) ); }
  if ( iCountMultiColors > 0 ) {
    float scale = m_eRigParameters.getFrameToWorldScale();
    auto  vec   = m_eRigParameters.getFrameToWorldTranslation();
    float width = m_eRigParameters.getWidth();
    outfile.write( reinterpret_cast<char*>( &scale ), sizeof( float ) );
    outfile.write( reinterpret_cast<char*>( &vec ), sizeof( float ) * 3 );
    outfile.write( reinterpret_cast<char*>( &width ), sizeof( float ) );
    for ( size_t i = 0; i < iCountMultiColors; i++ ) {
      outfile.write( reinterpret_cast<char*>( &( m_eRigParameters.getMatrix( i ) ) ), sizeof( float ) * 16 );
    }
    outfile.write( reinterpret_cast<char*>( m_pMultiColors3.data() ),
                   m_iNumPoints * iCountMultiColors * sizeof( float ) * 3 );
  }
  outfile.close();
}

typedef float ( *CastFunction )( unsigned char* pPointer );
inline float castUChar( unsigned char* pPointer ) { return (float)( *( (unsigned char*)pPointer ) ); }
inline float castChar( unsigned char* pPointer ) { return (float)( *( (char*)pPointer ) ); }
inline float castUShort( unsigned char* pPointer ) { return (float)( *( (uint16_t*)pPointer ) ); }
inline float castShort( unsigned char* pPointer ) { return (float)( *( (short*)pPointer ) ); }
inline float castUInt( unsigned char* pPointer ) { return (float)( *( (unsigned int*)pPointer ) ); }
inline float castInt( unsigned char* pPointer ) { return (float)( *( (int*)pPointer ) ); }
inline float castFloat( unsigned char* pPointer ) { return *( (float*)( pPointer ) ); }
inline float castDouble( unsigned char* pPointer ) { return (float)( *( (double*)pPointer ) ); }
inline float castZero( unsigned char* ) { return 0; }

void getSize( const std::string& pFormat, int& iSize, CastFunction& eCastFunction ) {
  // clang-format off
  if      ( pFormat == "float"   ) { iSize = 4; eCastFunction = castFloat;   } 
  else if ( pFormat == "uchar"   ) { iSize = 1; eCastFunction = castUChar;   } 
  else if ( pFormat == "uint8"   ) { iSize = 1; eCastFunction = castUChar;   }   
  else if ( pFormat == "char"    ) { iSize = 1; eCastFunction = castChar;    } 
  else if ( pFormat == "int8"    ) { iSize = 1; eCastFunction = castChar;    } 
  else if ( pFormat == "ushort"  ) { iSize = 2; eCastFunction = castUShort;  } 
  else if ( pFormat == "uint16"  ) { iSize = 2; eCastFunction = castUShort;  } 
  else if ( pFormat == "short"   ) { iSize = 2; eCastFunction = castShort;   } 
  else if ( pFormat == "int16"   ) { iSize = 2; eCastFunction = castShort;   } 
  else if ( pFormat == "uint"    ) { iSize = 4; eCastFunction = castUInt;    }
  else if ( pFormat == "uint32"  ) { iSize = 4; eCastFunction = castUInt;    } 
  else if ( pFormat == "int"     ) { iSize = 4; eCastFunction = castInt;     } 
  else if ( pFormat == "int32"   ) { iSize = 4; eCastFunction = castInt;     }
  else if ( pFormat == "float32" ) { iSize = 4; eCastFunction = castFloat;   }
  else if ( pFormat == "float64" ) { iSize = 8; eCastFunction = castDouble;  }
  else if ( pFormat == "double"  ) { iSize = 8; eCastFunction = castDouble;  } 
  else {    iSize         = 0;    eCastFunction = castZero;  }
  // clang-format on
}

bool ObjectPointcloud::read( const std::string&        pFilename,
                             int                       iFrameIndex,
                             bool                      bBinary,
                             int                       iDropDups,
                             std::vector<std::string>& pTypeName ) {
  m_eFilename = createFilename( pFilename, iFrameIndex );
  std::string strBinary;
  if ( bBinary ) {
    strBinary = getBinaryName( m_eFilename, iDropDups );
    if ( exist( strBinary ) && readBinary( strBinary, iFrameIndex ) ) { return true; }
  }
  std::string   eLine, s1, s2;
  std::ifstream inputPly;
  inputPly.open( m_eFilename.c_str(), std::ios::in | std::ios::binary );
  if ( !inputPly.is_open() ) {
    printf( "\nPointcloudReader: Couldn't open %s \n", m_eFilename.c_str() );
    exit( 1 );
  }
  bool bFormatError = false, bAscii = false, bLittleEndian = false, bInVertexElements = false,
       bInHeaderElements = false;
  int iNumPoints = 0, iSize = 0, iOrder = 0, iIndex = 0, iNumFaces = 0, iNumHeader = 0, iSizeHeader = 0;
  const std::vector<std::string> pProperty = {
      "x",      "y",       "z",      "red",    "green",   "blue",   "alpha",  "nx",      "ny",     "nz",      "type",
      "red0",   "green0",  "blue0",  "red1",   "green1",  "blue1",  "red2",   "green2",  "blue2",  "red3",    "green3",
      "blue3",  "red4",    "green4", "blue4",  "red5",    "green5", "blue5",  "red6",    "green6", "blue6",   "red7",
      "green7", "blue7",   "red8",   "green8", "blue8",   "red9",   "green9", "blue9",   "red10",  "green10", "blue10",
      "red11",  "green11", "blue11", "red12",  "green12", "blue12", "red13",  "green13", "blue13"};
  std::vector<int>          pSize, pOrder, pIndex;
  std::vector<CastFunction> pCast;
  pSize.clear();
  pOrder.clear();
  pIndex.clear();
  pCast.clear();
  pSize.resize( pProperty.size() + 1, 0 );
  pOrder.resize( pProperty.size(), -1 );
  pIndex.resize( pProperty.size(), 0 );
  pCast.resize( pProperty.size() + 1, castZero );
  getline( inputPly, eLine );
  while ( eLine.find( "end_header" ) != 0 ) {
    eLine = eLine.erase( eLine.size() );
    eLine.erase( std::remove( eLine.begin(), eLine.end(), '\n' ), eLine.end() );
    eLine.erase( std::remove( eLine.begin(), eLine.end(), '\r' ), eLine.end() );
    if ( static_cast<int>( eLine.find( "format " ) ) > -1 ) {
      std::string format = eLine.substr(
          eLine.find( ' ' ) + 1, eLine.find_first_of( ' ', eLine.find_first_of( ' ' ) + 1 ) - eLine.find( ' ' ) - 1 );
      if ( format == "ascii" ) {
        bAscii = true;
      } else if ( format == "binary_little_endian" ) {
        bLittleEndian = true;
      } else {
        printf( "PointcloudReader: Format is not ascii or binary_little_endian: format = \"%s\" \n", format.c_str() );
        bFormatError = true;
      }
    } else if ( static_cast<int>( eLine.find( "element" ) ) == 0 ) {
      // printf("read element : %s \n", eLine.c_str() );
      if ( static_cast<int>( eLine.find( "header_data " ) ) > 0 ) {
        eLine             = eLine.substr( eLine.find( "header_data " ) );
        iNumHeader        = atoi( eLine.substr( eLine.find( ' ' ) ).c_str() );
        bInHeaderElements = true;
        bInVertexElements = false;
      } else if ( static_cast<int>( eLine.find( "vertex " ) ) > 0 ) {
        eLine             = eLine.substr( eLine.find( "vertex " ) );
        iNumPoints        = atoi( eLine.substr( eLine.find( ' ' ) ).c_str() );
        bInHeaderElements = false;
        bInVertexElements = true;
      } else if ( static_cast<int>( eLine.find( "face " ) ) > 0 ) {
        eLine             = eLine.substr( eLine.find( "face " ) );
        iNumFaces         = atoi( eLine.substr( eLine.find( ' ' ) ).c_str() );
        bInHeaderElements = false;
        bInVertexElements = false;
      } else {
        bInHeaderElements = false;
        bInVertexElements = false;
      }
      // printf(" => bInHeaderElements = %d bInVertexElements = %d \n",bInHeaderElements,bInVertexElements );
    } else if ( static_cast<int>( eLine.find( "comment" ) ) == 0 ) {
      std::string key = "POINT_TYPE: ";
      if ( static_cast<int>( eLine.find( key ) ) > 0 ) {
        eLine      = eLine.substr( eLine.find( key ) + key.length() );
        size_t pos = eLine.find( ' ' ), init = 0;
        while ( pos != std::string::npos ) {
          pTypeName.push_back( eLine.substr( init, pos - init ) );
          init = pos + 1;
          pos  = eLine.find( ' ', init );
        }
        if ( init < eLine.size() ) {
          pTypeName.push_back( eLine.substr( init, std::min( pos, eLine.size() ) - init + 1 ) );
        }
        for ( auto& str : pTypeName ) { printf( "   element = %s \n", str.c_str() ); }
      }
      key = " frame_to_world_scale ";
      if ( static_cast<int>( eLine.find( key ) ) > 0 ) {
        eLine = eLine.substr( eLine.find( key ) + key.length() );
        m_eRigParameters.setFrameToWorldScale( static_cast<float>( atof( eLine.c_str() ) ) );
      }
      key = " frame_to_world_translation ";
      if ( static_cast<int>( eLine.find( key ) ) > 0 ) {
        eLine   = eLine.substr( eLine.find( key ) + key.length() );
        float a = static_cast<float>( atof( eLine.c_str() ) );
        eLine   = eLine.substr( eLine.find( ' ' ) + 1 );
        float b = static_cast<float>( atof( eLine.c_str() ) );
        eLine   = eLine.substr( eLine.find( ' ' ) + 1 );
        float c = static_cast<float>( atof( eLine.c_str() ) );
        eLine   = eLine.substr( eLine.find( ' ' ) + 1 );
        m_eRigParameters.setFrameToWorldTranslation( Vec3( a, b, c ) );
      }
      key = " width ";
      if ( static_cast<int>( eLine.find( key ) ) > 0 ) {
        eLine = eLine.substr( eLine.find( key ) + key.length() );
        m_eRigParameters.setWidth( static_cast<float>( atoi( eLine.c_str() ) ) );
      }
      key = " rig_count ";
      if ( static_cast<int>( eLine.find( key ) ) > 0 ) {
        eLine = eLine.substr( eLine.find( key ) + key.length() );
        m_eRigParameters.setCount( atoi( eLine.c_str() ) );
      }
      key = " rig_matrix_";
      if ( static_cast<int>( eLine.find( key ) ) > 0 ) {
        eLine                  = eLine.substr( eLine.find( key ) + key.length() );
        size_t index           = atoi( eLine.c_str() );
        eLine                  = eLine.substr( eLine.find( ' ' ) + 1 );
        Mat4&              mat = m_eRigParameters.getMatrix( index );
        std::vector<float> read;
        read.resize( 16 );
        for ( auto& r : read ) {
          r     = static_cast<float>( atof( eLine.c_str() ) );
          eLine = eLine.substr( eLine.find( ' ' ) + 1 );
        }
        mat = Mat4( read[0], read[1], read[2], read[3], read[4], read[5], read[6], read[7], read[8], read[9], read[10],
                    read[11], read[12], read[13], read[14], read[15] );
      }
    } else {
      if ( bInVertexElements && static_cast<int>( eLine.find( "property " ) ) == 0 ) {
        eLine          = eLine.substr( eLine.find( ' ' ) + 1, eLine.size() - eLine.find( ' ' ) - 1 );
        std::string eV = eLine.substr( 0, eLine.find_last_of( ' ' ) ),
                    eT = eLine.substr( eLine.find_last_of( ' ' ) + 1 );
        CastFunction eCast;
        getSize( eV, iSize, eCast );
        bool find = false;
        for ( size_t i = 0; i < pProperty.size() && !find; i++ ) {
          if ( eT == pProperty[i] ) {
            pSize[i]  = iSize;
            pOrder[i] = iOrder;
            pIndex[i] = iIndex;
            pCast[i]  = eCast;
            find      = true;
            // printf("PointcloudReader: Property %2zu: %16s Type = %16s \n",i,pProperty[i].c_str(),eV.c_str());
          }
        }
#if 1
        // Fix for xdprod contents
        if ( !find ) {
          const std::vector<std::string> pPropertyXdprod = {"diffuse_red", "diffuse_green", "diffuse_blue"};
          for ( size_t i = 0; i < pPropertyXdprod.size() && !find; i++ ) {
            if ( eT == pPropertyXdprod[i] ) {
              pSize[3 + i]  = iSize;
              pOrder[3 + i] = iOrder;
              pIndex[3 + i] = iIndex;
              pCast[3 + i]  = eCast;
              find          = true;
            }
          }
        }
#endif
        if ( !find ) {
          pSize[pProperty.size()] += iSize;
          pCast[pProperty.size()] = eCast;
          printf( "PointcloudReader: Property \"%s\" is not supported ( %s ) \n", eT.c_str(), eV.c_str() );
          fflush( stdout );
        }
        iIndex += iSize;
        iOrder++;
      } else if ( bInHeaderElements && static_cast<int>( eLine.find( "property " ) ) == 0 ) {
        eLine          = eLine.substr( eLine.find( ' ' ) + 1, eLine.size() - eLine.find( ' ' ) - 1 );
        std::string eV = eLine.substr( 0, eLine.find_last_of( ' ' ) ),
                    eT = eLine.substr( eLine.find_last_of( ' ' ) + 1 );
        CastFunction eCast;
        getSize( eV, iSize, eCast );
        iSizeHeader += iSize;
      }
    }
    getline( inputPly, eLine );
  }
  if ( pSize[0] == 0 ) {
    printf( "PointcloudReader: X must be define \n" );
    bFormatError = true;
  }
  if ( pSize[1] == 0 ) {
    printf( "PointcloudReader: Y must be define \n" );
    bFormatError = true;
  }
  if ( pSize[2] == 0 ) {
    printf( "PointcloudReader: Z must be define \n" );
    bFormatError = true;
  }
  if ( iNumFaces > 0 ) {
    // printf( "PointcloudReader: number of face must be equal to 0 (iNumFaces = %d) \n", iNumFaces );
    bFormatError = true;
  }
  if ( bFormatError ) {
    // printf( "PointcloudReader: format not yet supported \n" );
    // fflush( stdout );
    reset();
    return false;
  }
  allocate( pSize[6] != 0, pSize[7] != 0 || pSize[8] != 0 || pSize[9] != 0, pSize[10] != 0, iNumPoints, iFrameIndex,
            0 );
  const size_t countMultiColors = m_eRigParameters.getCount();
  if ( bAscii ) {
    std::vector<float> pF;
    pF.resize( iOrder + 1 );
    float* pS = nullptr;
    pS        = pF.data() + 1;
    memset( pF.data(), 0, ( iOrder - 1 ) * sizeof( float ) );
    for ( long int i = 0; i < iNumPoints; i++ ) {
      for ( size_t k = 0; k < static_cast<size_t>( iOrder ); k++ ) { inputPly >> pS[k]; }
      add( Point( pS[pOrder[0]], pS[pOrder[1]], pS[pOrder[2]] ),
           Color4( pS[pOrder[3]], pS[pOrder[4]], pS[pOrder[5]], pS[pOrder[6]] ),
           Normal( pS[pOrder[7]], pS[pOrder[8]], pS[pOrder[9]] ), static_cast<uint8_t>( pS[pOrder[10]] ) );
      if ( countMultiColors > 0 ) {
        for ( size_t k = 0; k < countMultiColors; k++ ) {
          setMultiColors3(
              i, k, Color3( pS[pOrder[11 + k * 3 + 0]], pS[pOrder[11 + k * 3 + 1]], pS[pOrder[11 + k * 3 + 2]] ) );
        }
      }
    }
    pF.clear();
  } else if ( bLittleEndian ) {
    std::vector<unsigned char> pC;
    pC.resize( iIndex );
    if ( iNumHeader > 0 ) { inputPly.ignore( iNumHeader * iSizeHeader ); }
    for ( size_t i = 0; i < static_cast<size_t>( iNumPoints ); i++ ) {
      inputPly.read( reinterpret_cast<char*>( pC.data() ), iIndex * sizeof( unsigned char ) );
#if 0
      add( Point( pCast[0]( pC.data() + pIndex[0] ), pCast[1]( pC.data() + pIndex[1] ),
                  pCast[2]( pC.data() + pIndex[2] ) ),
           Color3( pCast[3]( pC.data() + pIndex[3] ), pCast[4]( pC.data() + pIndex[4] ),
                   pCast[5]( pC.data() + pIndex[5] ) ) );
#else
      add( Point( pCast[0]( pC.data() + pIndex[0] ), pCast[1]( pC.data() + pIndex[1] ),
                  pCast[2]( pC.data() + pIndex[2] ) ),
           Color4( pCast[3]( pC.data() + pIndex[3] ), pCast[4]( pC.data() + pIndex[4] ),
                   pCast[5]( pC.data() + pIndex[5] ), pCast[6]( pC.data() + pIndex[6] ) ),
           Normal( pCast[7]( pC.data() + pIndex[7] ), pCast[8]( pC.data() + pIndex[8] ),
                   pCast[9]( pC.data() + pIndex[9] ) ),
           static_cast<uint8_t>( pCast[10]( pC.data() + pIndex[10] ) ) );
#endif
      if ( countMultiColors > 0 ) {
        for ( size_t k = 0; k < countMultiColors; k++ ) {
          setMultiColors3( i, k,
                           Color3( pCast[11 + k * 3 + 0]( pC.data() + pIndex[11 + k * 3 + 0] ),
                                   pCast[11 + k * 3 + 1]( pC.data() + pIndex[11 + k * 3 + 1] ),
                                   pCast[11 + k * 3 + 2]( pC.data() + pIndex[11 + k * 3 + 2] ) ) );
        }
      }
    }
    pC.clear();
  }
  inputPly.close();
  if ( iDropDups > 0 ) { removeDuplicatePoints( iDropDups ); }
  if ( bBinary ) { writeBinary( strBinary ); }
  if ( m_eRigParameters.exist() ) { m_eRigParameters.trace(); }
  return true;
}

std::vector<float> ObjectPointcloud::computeWeigth() {
  auto&              matrix = m_eRigParameters.getMatrix();
  std::vector<float> weight;
  weight.resize( matrix.size(), 0.f );
  Vec3 point     = m_eBox.center();
  Vec3 vecCamera = glm::normalize( m_eCameraPosition - point );
  for ( size_t i = 0; i < matrix.size(); i++ ) {
    Vec3 posRig = m_eRigParameters.getCameraPosition( i );
    Vec3 vecRig = glm::normalize( posRig - point );
    weight[i]   = std::max( glm::dot( vecCamera, vecRig ), 0.0f );
    weight[i]   = pow( weight[i], 10.f );
  }
  return weight;
}

void ObjectPointcloud::getRigPoints( std::vector<Vec3>&   points,
                                     std::vector<Color3>& colors,
                                     std::vector<Vec3>&   directions ) {
  if ( m_eRigParameters.exist() ) {
    const Color3 red = {1, 0, 0}, black = {0, 0, 0};
    auto&        matrix   = m_eRigParameters.getMatrix();
    auto         weigth   = computeWeigth();
    size_t       indexMax = std::max_element( weigth.begin(), weigth.end() ) - weigth.begin();
    for ( size_t i = 0; i < matrix.size(); i++ ) {
      Quaternion q( matrix[i] );
      Vec3       d( 2 * ( q.x * q.z + q.w * q.y ), 2 * ( q.y * q.z - q.w * q.x ), 1 - 2 * ( q.x * q.x + q.y * q.y ) );
      auto       pos = m_eRigParameters.getCameraPosition( i );
      auto&      color =
          ( m_iMultiColorIndex >= 0 && static_cast<int>( i ) == m_iMultiColorIndex )
              ? red
              : ( m_iMultiColorIndex == -2 || m_iMultiColorIndex == -1 )
                    ? ( weigth[i] <= 0
                            ? black
                            : ( i == indexMax ? red : g_pColorRgb[static_cast<int>( 255.f * ( 1 - weigth[i] ) )] ) )
                    : g_pColorRgb[static_cast<int>( 255.f * i / matrix.size() )];
      points.push_back( pos );
      colors.push_back( color );
      directions.push_back( pos - 1000.f * glm::normalize( d ) );
    }
  }
}

// color space conversion to YUV
auto convertRGBtoYUV_BT709( const Color3& rgb ) -> Color3 {
  Color3 yuv;
  yuv[0] = ( 0.2126f * rgb[0] + 0.7152f * rgb[0] + 0.0722f * rgb[0] );
  yuv[1] = ( -0.1146f * rgb[1] - 0.3854f * rgb[1] + 0.5000f * rgb[1] ) + 0.5f;
  yuv[2] = ( 0.5000f * rgb[2] - 0.4542f * rgb[2] - 0.0458f * rgb[2] ) + 0.5f;
  return yuv;
}

auto colorDistance( const Color3& rgb0, const Color3& rgb1, int component ) -> float {
  Color3 yuv0 = convertRGBtoYUV_BT709( rgb0 ), yuv1 = convertRGBtoYUV_BT709( rgb1 );
  float  distance = 0;
  switch ( component ) {
    case 0: distance = ( yuv0[0] - yuv1[0] ) * ( yuv0[0] - yuv1[0] ); break;
    case 1: distance = ( yuv0[1] - yuv1[1] ) * ( yuv0[1] - yuv1[1] ); break;
    case 2: distance = ( yuv0[2] - yuv1[2] ) * ( yuv0[2] - yuv1[2] ); break;
  }
  return distance;
}

void ObjectPointcloud::computeDistance( std::vector<Color3>& eColors ) {
  std::vector<float> eDistance;
  eColors.resize( m_iNumPoints );
  eDistance.resize( m_iNumPoints );
  ObjectPointcloud& eSource = *( reinterpret_cast<ObjectPointcloud*>( m_pcSource ) );
  typedef KDTreeSingleIndexAdaptor<L2_Simple_Adaptor<float, ObjectPointcloud>, ObjectPointcloud, 3> KdTree;
  KdTree eKdTree( 3, eSource, KDTreeSingleIndexAdaptorParams( 10 ) );
  eKdTree.buildIndex();
  for ( size_t i = 0; i < static_cast<size_t>( m_iNumPoints ); i++ ) {
    size_t                         iIndex = 0;
    nanoflann::KNNResultSet<float> eResultSet( 1 );
    eResultSet.init( &iIndex, &eDistance[i] );
    eKdTree.findNeighbors( eResultSet, glm::value_ptr( m_pPoints[i] ), nanoflann::SearchParams( 10 ) );
    if ( m_iDisplayMetric >= 2 ) {
      eDistance[i] = colorDistance( getColors3( i ), eSource.getColors3( iIndex ), m_iDisplayMetric - 2 );
    }
  }
  m_fMaxDistance = *std::max_element( std::begin( eDistance ), std::end( eDistance ) );
  m_fMaxDistance = m_fMaxDistance < 0.00000001f ? 1.f : m_fMaxDistance;
  for ( size_t i = 0; i < static_cast<size_t>( m_iNumPoints ); i++ ) {
    eColors[i] = g_pColorRgb[static_cast<int>( 255.f * eDistance[i] / m_fMaxDistance )];
  }
}

void ObjectPointcloud::computeDuplicate( std::vector<Color3>& eColors ) {
  std::vector<float> eDistance;
  eColors.resize( m_iNumPoints );
  eDistance.resize( m_iNumPoints );
  typedef KDTreeSingleIndexAdaptor<L2_Simple_Adaptor<float, ObjectPointcloud>, ObjectPointcloud, 3> KdTree;
  KdTree eKdTree( 3, *this, KDTreeSingleIndexAdaptorParams( 10 ) );
  eKdTree.buildIndex();
  const auto radius = static_cast<uint8_t>( 1.f );
  for ( size_t i = 0; i < static_cast<size_t>( m_iNumPoints ); i++ ) {
    nanoflann::SearchParams               params;
    std::vector<std::pair<size_t, float>> values;
    const size_t nMatches = eKdTree.radiusSearch( glm::value_ptr( m_pPoints[i] ), radius, values, params );
    eDistance[i]          = static_cast<float>( nMatches ) - 1;
  }
  m_fMaxDistance = *std::max_element( std::begin( eDistance ), std::end( eDistance ) );
  m_fMaxDistance = m_fMaxDistance < 0.00001f ? 1.f : m_fMaxDistance;
  for ( size_t i = 0; i < static_cast<size_t>( m_iNumPoints ); i++ ) {
    eColors[i] = g_pColorRgb[static_cast<int>( 255.f * eDistance[i] / m_fMaxDistance )];
  }
}

void ObjectPointcloud::colorBasedType( std::vector<Color3>& eColors ) {
  eColors.resize( m_iNumPoints );
  size_t                                                                                            count = 0;
  typedef KDTreeSingleIndexAdaptor<L2_Simple_Adaptor<float, ObjectPointcloud>, ObjectPointcloud, 3> KdTree;
  KdTree eKdTree( 3, *this, KDTreeSingleIndexAdaptorParams( 10 ) );
  eKdTree.buildIndex();
  const auto radius = static_cast<uint8_t>( 1.f );
  for ( size_t i = 0; i < static_cast<size_t>( m_iNumPoints ); i++ ) {
    nanoflann::SearchParams               params;
    std::vector<std::pair<size_t, float>> values;
    const size_t nMatches = eKdTree.radiusSearch( glm::value_ptr( m_pPoints[i] ), radius, values, params );
    bool         match    = getTypes()[i] == m_iTypeColor - 1;
    for ( size_t j = 0; !match && j < nMatches; j++ ) {
      if ( getTypes()[values[j].first] == m_iTypeColor - 1 ) { match = true; }
    }
    if ( match ) { count++; }
    eColors[i] = match ? g_pColorPalette[m_iTypeColor - 1] : getColors3()[i];
  }
  printf( " %9d / %9d points of type %d  \n", static_cast<int>( count ), m_iNumPoints, m_iTypeColor - 1 );
}

void ObjectPointcloud::loadProgram() {
  m_ePrograms.resize( 4 );
  m_ePrograms[0].create( "Cube", g_pSurfaceVertexShader, g_pCubeGeometryShader, g_pCubeFragmentShader );
  m_ePrograms[1].create( "Circle", g_pSurfaceVertexShader, g_pSplatGeometryShader, g_pSplatFragmentShader );
  m_ePrograms[2].create( "Point", g_pPointVertexShader, g_pPointFragmentShader );
  m_ePrograms[3].create( "Blended", g_pSurfaceVertexShader, g_pBlendedSplatGeometryShader, g_pBlendedSplatFragmentShader);
}

void ObjectPointcloud::load() {
  if ( !m_bLoad ) {
    auto& program = m_ePrograms[m_iProgramIndex];
    glGenVertexArrays( 1, &m_uiVAO );
    glGenBuffers( 1, &m_uiVBO );
    glGenBuffers( 1, &m_uiCBO );
    if ( m_eRigParameters.exist() ) { glGenBuffers( 1, &m_uiMCBO ); }
    glGenBuffers(1, &m_uiIBO);
    glBindVertexArray( m_uiVAO );
    glBindBuffer( GL_ARRAY_BUFFER, m_uiVBO );
    glBufferData( GL_ARRAY_BUFFER, 3 * m_iNumPoints * sizeof( float ), m_pPoints.data(), GL_STATIC_DRAW );
    glEnableVertexAttribArray( program.attrib( "point" ) );
    glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, 0, nullptr );
    glVertexAttribPointer( program.attrib( "point" ), 3, GL_FLOAT, GL_FALSE, 0, nullptr );

    glBindBuffer( GL_ARRAY_BUFFER, m_uiCBO );
    if ( !( ( m_iDisplayMetric > 0 ) || m_bDisplayDuplicate || m_iTypeColor > 0 ) ) {
      glBufferData( GL_ARRAY_BUFFER, ( 3 + getAlpha() ) * m_iNumPoints * sizeof( float ), getColors(), GL_STATIC_DRAW );
      glVertexAttribPointer( program.attrib( "color" ), 3 + getAlpha(), GL_FLOAT, GL_FALSE, 0, nullptr );
    } else {
      std::vector<Color3> eColors;
      if ( ( m_iDisplayMetric > 0 ) ) {
        computeDistance( eColors );
      } else if ( m_bDisplayDuplicate ) {
        computeDuplicate( eColors );
      } else if ( m_iTypeColor > 0 ) {
        colorBasedType( eColors );
      }
      glBufferData( GL_ARRAY_BUFFER, 3 * m_iNumPoints * sizeof( float ), eColors.data(), GL_STATIC_DRAW );
      glVertexAttribPointer( m_ePrograms[m_iProgramIndex].attrib( "color" ), 3, GL_FLOAT, GL_FALSE, 0, nullptr );
    }
    glEnableVertexAttribArray( program.attrib( "color" ) );
    glBindBuffer( GL_ARRAY_BUFFER, 0 );

    if ( m_eRigParameters.exist() ) {
      size_t count = m_eRigParameters.getCount();
      glBindBuffer( GL_ARRAY_BUFFER, m_uiMCBO );
      glBufferData( GL_ARRAY_BUFFER, 3 * sizeof( float ) * count * m_iNumPoints, getMultiColors3().data(),
                    GL_STATIC_DRAW );
      for ( size_t i = 0; i < count; i++ ) {
        glEnableVertexAttribArray( static_cast<GLuint>( i ) );
        glVertexAttribPointer( static_cast<GLuint>( i ), 3, GL_FLOAT, GL_FALSE,
                               static_cast<GLsizei>( 3 * count * sizeof( float ) ),
                               (void*)( i * 3 * sizeof( float ) ) );
      }
      glEnableVertexAttribArray( program.attrib( "colorM" ) );
      glBindBuffer( GL_ARRAY_BUFFER, 0 );
    }
    glBindVertexArray( 0 );
    m_bLoad = true;
  }
}

void ObjectPointcloud::unload() {
  if ( m_bLoad ) {
    glDeleteBuffers( 1, &m_uiVBO );
    glDeleteBuffers( 1, &m_uiCBO );
    glDeleteBuffers( 1, &m_uiMCBO );
    glDeleteVertexArrays( 1, &m_uiVAO );
    m_bLoad = false;
  }
}

void ObjectPointcloud::draw( bool ) {
  if ( !m_eRigParameters.exist() || m_iMultiColorIndex >= static_cast<int>( m_eRigParameters.getCount() ) ) {
    m_iMultiColorIndex = -3;
  }
  size_t iMode = m_iMultiColorIndex;
  if ( m_iMultiColorIndex == -2 ) {
    std::vector<float> weigth = computeWeigth();
    iMode                     = std::max_element( weigth.begin(), weigth.end() ) - weigth.begin();
  }
  auto&             program = m_ePrograms[m_iProgramIndex];
  std::vector<Vec3> posRig;
  for ( size_t i = 0; i < m_eRigParameters.getCount(); i++ ) {
    posRig.push_back( m_eRigParameters.getCameraPosition( i ) );
  }
  program.setUniform( "mode", static_cast<int>( iMode ) );
  program.setUniform( "count", static_cast<int>( m_eRigParameters.getCount() ) );
  program.setUniform( "posRig", posRig );

  glBindVertexArray( m_uiVAO );
  
  if (m_bSort) {
    glEnable(GL_BLEND);
    glDisable(GL_DEPTH_TEST);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_uiIBO);
    glDrawElements(GL_POINTS, m_iNumPoints, GL_UNSIGNED_INT, (void*)0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glEnable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);
    m_bSort = false;
  }
  else {
      glDrawArrays(GL_POINTS, 0, m_iNumPoints);
  }
  glBindVertexArray( 0 );
}

std::string ObjectPointcloud::getInformation() {
  return stringFormat( " Points   = %9d Duplicate = %8d ", m_iNumPoints, m_iNumDuplicate );
}
