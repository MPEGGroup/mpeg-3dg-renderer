//Copyright(c) 2016 - 2025, InterDigital
//All rights reserved.
//See LICENSE under the root folder.

#ifndef _DEFINITION_RENDERER_APP_H_
#define _DEFINITION_RENDERER_APP_H_

#define NOMINMAX
#define _USE_MATH_DEFINES
#include <set>
#include <cmath>
#include <cstdlib>
#include <cstddef>
#include <cstdio>
#include <cassert>
#include <vector>
#include <iostream>
#include <fstream>
#include <istream>
#include <sstream>
#include <string>
#include <cstring>
#include <chrono>
#include <algorithm>
#include <sys/stat.h>
#include <sys/types.h>
#include <cerrno>
#include <map>
#include <memory>
#include <iomanip>
#include <functional>

#ifdef WIN32
#include <windows.h>
#endif

#include <glad/gl.h>

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_access.hpp>
#include <glm/ext/matrix_relational.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/transform.hpp>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include "PccRendererConfig.h"

static const std::string g_pCopyrightString =
R"(-------------------------------------------------------------------------------------------
|                                   Point Cloud Renderer                                  |
|        Copyright 2016-2025 - InterDigital CE Patent Holding - All Rights Reserved       |
|                                                                                         |
| This software is being made available under the BSD License, available in the           |
| root folder. This software may be subject to InterDigital and other third party and     |
| contributor rights, including patent rights, and no such rights are granted under       |
| this license.                                                                           |
-------------------------------------------------------------------------------------------
)";

static const std::string g_pHelpString = g_pCopyrightString + R"(
Azerty keyboard shortcuts (Querty): 
 + General:                                      + Playback control:   
  - h           : Help.                           - Space       : Play/Stop.          
  - q|Escape (a): Exit.                           - Up/Down     : First/Last frame. 
  - d           : Draw overlay.                   - Left/Right  : Previous/Next frame.
  - t           : Draw logs.                      - w        (z): Save video.  
  - s           : Synchronize windows.            
                                                 + Record camera path and view point: 
 + Configure interface and rendering:             - y           : Save viewpoint.
  - F11         : Fullscreen.                     - u           : Start/Stop path recording.
  - x           : Axes.                           - i           : Insert fixed position.
  - b           : Bounding box and camera rig.    - k           : Insert position.
  - v           : Rotate circles.                 - l           : Load a preloaded path. 
  - o           : Ortho/Perspective rendering.    - g           : Camera path rendering.
  - f           : Display duplicate points.       - m        (:): Segment/Spline camera path.
  - r           : Change rendering mode.          - p           : Draw camera path.   
  - c           : Change background color.           
  - n           : Change point cloud color.      + 3D background scene:   
  - j           : lighting.                       - Mouse+Shift: Change position and rotation.
                                                  - Shift+s/x   : +/- scale.
 + Alignment mode:                                - Shift+d/c   : +/- X position.
  - 1-2         : Faces of the bounding box.      - Shift+f/v   : +/- Y position.
  - 3-4         : Horizontal octagon vertices.    - Shift+g/b   : +/- Z position.
  - 5-6         : Horizontal rotation.            - Shift+h/n   : +/- X rotation.
  - 7-8         : +/- auto-rotate speed.          - Shift+j/,(m): +/- Y rotation.
  - 9-0         : Zoom.                           - Shift+k/;(,): +/- Z rotation.
                                                  - Shift+w  (x): Save coordinates.
 + Multi-color mode:                              
  - Tab         : Switch between model colors:   + Distance mode between main and source ply:
                  main, closest, interpolate      - z        (w): Display distance.  
                  or fixed color [0;N].           - a        (q): Switch. 
  - Ctrl+Tab    : Main color.                     
                                                  
 + Blended points rendering mode:
  - Page Up/Down: Increase/decrease alpha falloff
  - End         : Change blending mode

Mouse: 
  - Left button : Rotate.             - Scroll|Center+Ctrl: Zoom.
  - Right button: Translate.          - Scroll+Alt        : Point size.     

PccAppRenderer version: )" + PCC_APP_RENDERER_VERSION;

// TYPE DEFINITION
typedef glm::u8vec3 Vec3u8;
typedef glm::ivec2  Vec2i;
typedef glm::ivec3  Vec3i;
typedef glm::vec2   Vec2;
typedef glm::vec3   Vec3;
typedef glm::vec4   Vec4;
typedef glm::mat2   Mat2;
typedef glm::mat3   Mat3;
typedef glm::mat4   Mat4;
typedef glm::mat3x2 Mat3x2;
typedef glm::mat3x4 Mat3x4;
typedef glm::quat   Quaternion;
typedef glm::vec3   Point;
typedef glm::vec3   Color3;
typedef glm::vec4   Color4;
typedef glm::vec3   Normal;

// PROGRESSBAR PRINT
#define PROGRESSBAR( iIndex, iNum, pString, ... )                                                          \
  {                                                                                                        \
    auto iPBValue = (int)( ( (float)( iIndex + 1 ) / (float)iNum ) * 100 );                                \
    printf( "\r" );                                                                                        \
    printf( pString, ##__VA_ARGS__ );                                                                      \
    printf( ": %9d / %9d : %3d%%[%.*d%*s]", iIndex + 1, iNum, iPBValue, iPBValue, 0, 100 - iPBValue, "" ); \
    if ( ( iIndex ) + 1 == ( iNum ) ) { printf( "\n" ); }                                                  \
    fflush( stdout );                                                                                      \
  }

#ifdef WIN32
#define snprintf _snprintf
#endif
template <typename... Args>
std::string stringFormat( const char* pFormat, Args... args ) {
  size_t      iSize = snprintf( nullptr, 0, pFormat, args... );
  std::string pString;
  pString.reserve( iSize + 1 );
  pString.resize( iSize );
  snprintf( &pString[0], iSize + 1, pFormat, args... );
  return pString;
}

static const std::string getDate() {
  time_t eTime;
  time( &eTime );
  struct tm* pTm = localtime( &eTime );
  return stringFormat( "%4d%02d%02d-%02dh%02dm%02ds", 1900 + pTm->tm_year, pTm->tm_mon + 1, pTm->tm_mday, pTm->tm_hour,
                       pTm->tm_min, pTm->tm_sec );
}

static inline bool exist( const std::string& pString ) {
  struct stat buffer;
  return ( stat( pString.c_str(), &buffer ) == 0 );
}

static bool dirExists( const std::string& pString ) {
  struct stat sb;
  if ( stat( pString.c_str(), &sb ) == 0
       // && S_ISDIR( sb.st_mode )
  ) {
    return true;
  }
  return false;
}

#ifndef _WIN32
static char getSeparator() { return '/'; }
#else
static char getSeparator() { return '\\'; }
#endif

static char getSeparator( const std::string& eFilename ) {
  auto pos = ( std::min )( eFilename.find_last_of( '/' ), eFilename.find_last_of( '\\' ) );
  return pos != std::string::npos ? eFilename[pos] : getSeparator();
}

static std::string createFilename( const std::string& pFilename, int iFrameIndex ) {
  std::string sFilename = stringFormat( pFilename.c_str(), iFrameIndex );
#ifdef MSVC
  char pNewName[512];
  GetFullPathName( sFilename.c_str(), 512, pNewName, NULL );
  sFilename = pNewName;
#endif
  return sFilename;
}

static std::string getExtension( const std::string& eFilename ) {
  auto position = eFilename.find_last_of( '.' );
  if ( position != std::string::npos ) {
    return eFilename.substr( position + 1 );
  } else {
    return std::string( "" );
  }
}

static std::string getRemoveExtension( const std::string& eFilename ) {
  auto position = eFilename.find_last_of( '.' );
  if ( position != std::string::npos ) { return eFilename.substr( 0, position ); }
  return eFilename;
}

static std::string getDirectory( const std::string& string ) {
  auto position = string.find_last_of( getSeparator( string ) );
  if ( position != std::string::npos ) { return string.substr( 0, position ); }
  return string;
}

static std::string getBasename( const std::string& string ) {
  auto position = string.find_last_of( getSeparator() );
  if ( position != std::string::npos ) { return string.substr( position + 1, string.length() ); }
  return string;
}

static std::string getBinaryName( const std::string& eFilename, int iDropDups ) {
  return getDirectory( eFilename ) + getSeparator() + ".binary" + getSeparator() +
         getRemoveExtension( getBasename( eFilename ) ) + stringFormat( "_dup%d.bpcfi", iDropDups );
}

static void trace( Mat4& mat ) {
  printf( "Matrix %12.8f %12.8f %12.8f %12.8f \n", mat[0][0], mat[0][1], mat[0][2], mat[0][3] );
  printf( "       %12.8f %12.8f %12.8f %12.8f \n", mat[1][0], mat[1][1], mat[1][2], mat[1][3] );
  printf( "       %12.8f %12.8f %12.8f %12.8f \n", mat[2][0], mat[2][1], mat[2][2], mat[2][3] );
  printf( "       %12.8f %12.8f %12.8f %12.8f \n", mat[3][0], mat[3][1], mat[3][2], mat[3][3] );
}
static void trace( Vec3& vec ) { printf( "Vec3   %12.8f %12.8f %12.8f \n", vec[0], vec[1], vec[2] ); }

#define CLIP( n, lower, upper ) ( ( ( n ) < ( lower ) ) ? ( lower ) : ( ( ( n ) > ( upper ) ) ? ( upper ) : ( n ) ) )
#define FCLOSE( file ) \
  if ( file ) {        \
    fclose( file );    \
    file = nullptr;    \
  }

#define SHADER_VERSION "400"
#define SHADER( shader ) "#version " SHADER_VERSION "\n" #shader

#endif  // _DEFINITION_RENDERER_APP_H_