//Copyright(c) 2016 - 2025, InterDigital
//All rights reserved.
//See LICENSE under the root folder.


#include "PccRendererParameters.h"
#include "PccRendererPrimitive.h"

static inline std::istream& operator>>( std::istream& stream, Vec3u8& v ) {
  uint16_t a = 0, b = 0, c = 0;
  stream >> a >> b >> c;
  v = Vec3u8( ( std::min )( a, (uint16_t)std::numeric_limits<uint8_t>::max() ),
              ( std::min )( b, (uint16_t)std::numeric_limits<uint8_t>::max() ),
              ( std::min )( c, (uint16_t)std::numeric_limits<uint8_t>::max() ) );
  return stream;
}
static inline std::istream& operator>>( std::istream& stream, Vec3& v ) {
  stream >> v[0] >> v[1] >> v[2];
  return stream;
}
static std::ostream& operator<<( std::ostream& stream, Vec3u8 const& v ) {
  stream << "'" << (int)v[0] << " " << (int)v[1] << " " << (int)v[2] << "'";
  return stream;
}
static std::ostream& operator<<( std::ostream& stream, Vec3 const& v ) {
  stream << "'" << v[0] << " " << v[1] << " " << v[2] << "'";
  return stream;
}

#include "program_options_lite.h"

RendererParameters::RendererParameters()  = default;
RendererParameters::~RendererParameters() = default;

void printVersion( df::program_options_lite::Options /*unused*/,
                   const std::string /*unused*/,
                   df::program_options_lite::ErrorReporter& errorReporter ) {
  printf( "PccAppRenderer version = %s \n", PCC_APP_RENDERER_VERSION );
  fflush( stdout );
  errorReporter.is_errored = true;
}

void printHelp( df::program_options_lite::Options& eOptions,
                const std::string /*unused*/,
                df::program_options_lite::ErrorReporter& errorReporter ) {
  printf( "%s", g_pCopyrightString.c_str() );
  printf( "PccAppRenderer configuration: input parameters must be:\n" );
  doHelp( std::cout, eOptions );
  errorReporter.is_errored = true;
}

bool RendererParameters::parseCfg( int argc, char* argv[] ) {
  df::program_options_lite::Options       eOptions;
  df::program_options_lite::ErrorReporter err;
  const Vec3u8                            defaultBackground = { 128, 128, 128 };
  Vec3u8                                  colorBackground   = defaultBackground;
  Vec3u8                                  colorFloor        = { 95, 95, 95 };
  // clang-format off
  eOptions.addOptions()
    ( "version",         printVersion,                          "Print version."                                         )
    ( "help",            printHelp,                             "Print help."                                            )
    ( "config",          df::program_options_lite::parseConfigFile,  "Parse configuration file."                         )
    ( "f,PlyFile",       m_pFile,              std::string(""), "Ply input filename."                                    )
    ( "d,PlyDir",        m_pDir,               std::string(""), "Ply input directory."                                   )
    ( "SrcFile",         m_pFileSrc,           std::string(""), "Source Ply filename (used for comparison)."             )
    ( "SrcDir",          m_pDirSrc,            std::string(""), "Source Ply directory (used for comparison)."            )
    ( "b,binary",        m_bCreateBinaryFiles, false,           "Create temp binary files."                              )
    ( "o,RgbFile",       m_pRgbFile,           std::string(""), "Output RGB 8bits filename (specify prefix file name)."  )
    ( "x,camera",        m_pCameraPathFile,    std::string(""), "Camera path filename."                                  )
    ( "y,viewpoint",     m_pViewpointFile,     std::string(""), "Viewpoint filename."                                    )
    ( "spline",          m_bSpline,            false,           "Interpolate the camera path by splines."                )
    ( "n,frameNumber",   m_iFrameNumber,       1,               "Fraumber."                                              )
    ( "i,frameIndex",    m_iFrameIndex,        0,               "Frame index."                                           )
    ( "fps",             m_fFps,               30.f,            "Frames per second."                                     )
    ( "a,align",         m_iAlign,             0,               "Align (0:X, 1:-X, 2:Y, 3:-Y 4:Z, 5:-Z)."                )
    ( "width",           m_iWidth,             800,             "Window width."                                          )
    ( "height",          m_iHeight,            600,             "Window height."                                         )
    ( "posx",            m_iPosX,              -1,              "Window position X."                                     )
    ( "posy",            m_iPosY,              -1,              "Window position Y."                                     )
    ( "size",            m_fPointSize,         1.f,             "Point size."                                            )
    ( "blendMode",       m_iBlendMode,         0,               "Blended point mode (0:Gaussian, 1:Linear)."             )
    ( "alphaFalloff",    m_fAlphaFalloff,      1.f,             "Blending alpha falloff."                                )
    ( "type",            m_iPointType,         0,               
        "Point type:\n"
        "  Point cloud: 0: cube, \n"
        "               1: circle, \n"
        "               2: point \n"
        "               3: blended point \n"
        "  Mesh:        0: surface, \n"
        "               1: surface+wireframe,  \n"
        "               2: wireframe,  \n"
        "               3: point"                                                                                        )
    ( "cameraPathIndex", m_iCameraPathIndex, -1,               "Camera path index: \n"      
      "  Preload camera path composed of three parts: \n"
      "    - 3 seconds from a fixed position (left or right) aligned with the bounding box of the sequence (orthographic rendering) \n"
      "    - 3 seconds from a fixed position (front) aligned with the bounding box of the sequence (orthographic rendering) \n"
      "    - 4 seconds of dynamic path around the models (perspective rendering). \n"      
      "  Mode:  \n"
      "    0: left  + front + front zoom in \n"
      "    1: right + front + front zoom in \n"
      "    2: left  + front + front zoom in right curved \n"
      "    3: right + front + front zoom in left curved \n"
      "    4: left  + front + near semicircle to right \n"
      "    5: right + front + near semicircle to left \n"
      "    6: left  + front + near circle to right \n"
      "    7: right + front + near circle to left \n"
      "    8: left  + front + upward spiral to the right \n"
      "    9: right + front + upward spiral to the left \n" 
      "   10: Left curved zoom for longdress, soldier, mitch, thomas, levi sequences \n"
      "   11: Zoom in for basketball sequence \n"
      "   12: Zoom in for dancer sequence \n"
      "   13: Zoom in for football sequence \n"                                                                          ) 
    ( "monitor",         m_iMonitor,           0,               "Monitor to display the window."                         )
    ( "backgroundIndex", m_iBackgroundIndex,   -1,              "Window background index."                               )
    ( "backgroundColor", colorBackground,      colorBackground, "Background color:\"R G B\"."                            )
    ( "floor",           m_bFloor,             false,           "Adds grey floor under objects."                         )
    ( "floorColor",      colorFloor,           colorFloor,      "Floor color:\"R G B\"."                                 )
    ( "depthMap",        m_bDepthMap,          false,           "Display depth map."                                     )
    ( "p,play",          m_bPlay,              false,           "Play the sequences."                                    )
    ( "playBackward",    m_bPlayBackward,      false,           "Play sequence forward and backward."                    )
    ( "r,rotate",        m_iRotate,            0,               "Auto-rotate (speed in [0;4])."                          )
    ( "overlay",         m_bOverlay,           true,            "Display overlay."                                       )
    ( "orthographic",    m_bOrthographic,      false,           "Orthographic projection."                               )
    ( "synchronize",     m_bSynchronize,       false,           "Synchronize multi-windows."                             )
    ( "box",             m_iBoxSize,           -1,              "Bounding box size."                                     )
    ( "dropdups",        m_iDropDups,          2,               "Drop same coordinate points (0:No, 1:drop, 2:average)." )
    ( "c,center",        m_bCenter,            false,           "Center the object in the bounding box."                 )
    ( "s,scale",         m_iScaleMode,         0,               
        "Scale mode:    0: disable, \n"
        "               1: scale according to the object bounding box. \n"
        "               2: scale according to the sequence bounding box. "                                               )
    ( "scenePath",       m_pScenePath,        std::string(""), "3D background scene path (obj object)."                  )
    ( "sceneScale",      m_fSceneScale,       1.0f,            "3D background scene scale."                              )
    ( "scenePos",        m_eScenePosition,    {0.f,0.f,0.f},   "3D background scene position: \"X Y Z\"."                )
    ( "sceneRot",        m_eSceneRotation,    {0.f,0.f,0.f},   "3D background scene rotation: \"X Y Z\"."                )
    ( "visible",         m_bVisible,          true,            "Open user interface."                                    )
    ( "lighting",        m_bLighting,         false,           "Enable lighting (only for mesh objects)."                )
    ( "softwareRenderer",m_bSoftwareRenderer, false,           "Pure software rendererer without OpenGL/GLFW. "
    "  Note: this mode disables GUI and screen renderering and only allows to create offscreen RGB videos."              );

  // clang-format on  

  setDefaults( eOptions );
  try {
    scanArgv( eOptions, argc, (const char**)argv, err );
  } catch ( df::program_options_lite::ParseFailure& e ) {
    printHelp( eOptions, "", err );
    std::cerr <<"Parsing error: option: \"" << e.arg << "\" and value: \"" << e.val << "\" are not supported. \n";
    return false;
  }
  if( argc == 1 || err.is_errored ) { 
    printHelp( eOptions, "", err ); 
    return false; 
  }
  if ( !check() ) {    
    printHelp( eOptions, "", err );
    error();
    return false;
  }  
  if( colorBackground != defaultBackground ){ m_iBackgroundIndex = -1; }
  m_eBackgroundColor = Vec3( colorBackground ) / 255.f;
  m_eFloorColor      = Vec4( colorFloor[0], colorFloor[1], colorFloor[2], 255.f ) / 255.f;
  return true;
}

// Check requiered parameters
bool RendererParameters::check( bool verbose ) {  
  m_iBackgroundIndex = ( std::min )( m_iBackgroundIndex, (int)g_fBackground.size() -1 );
  if ( m_pFile.empty() && m_pDir.empty() ) {
    if( verbose ) { printf( "Error: Ply file or director must be defined. \n" ); }
    return false;
  }
  if ( m_iAlign > 7 || m_iAlign < 0 ) {
    if( verbose ) {  printf( "Error: Align value not supported, %d not in[0;6].\n", m_iAlign ); }
    return false;
  }
  if ( m_iPointType < 0 || m_iPointType > 3 ) {
    if( verbose ) { printf( "Error: Point type value not supported, %d not in[0;3].\n", m_iPointType ); }
    return false;
  }
  if ( m_fPointSize <= 0 ) {
    if( verbose ) { printf( "Error: Point size value not supported, %f must be > 0.\n", m_fPointSize ); }
    return false;
  }
  if (m_fAlphaFalloff <= 0) {
      if (verbose) { printf("Error: Blend point focus value not supported, %f must be > 0.\n", m_fPointSize); }
      return false;
  }
  if (m_iBlendMode < 0 || m_iBlendMode > 1) {
      if (verbose) { printf("Error: Blend mode value not supported, %d not in[0;1].\n", m_iBlendMode); }
      return false;
  }
  if( m_bSoftwareRenderer ) {
    if( m_pRgbFile.empty() ){
      if( verbose ) { printf( "Error: SW rendereing need to define the RgbFile input parameter. \n" ); }
      return false;
    }
    if( m_iCameraPathIndex == -1 && m_pCameraPathFile.empty() ){
      if( verbose ) { printf( "Error: SW rendereing need to define a camera path. \n" ); }
      return false;
    }
  }
  return true;
}

void RendererParameters::error() {
  printf( "Error: configuration is not correct \n" );
  check( true );
  fflush(stdout);
}

void RendererParameters::print() {
  printf( " Ply input file  = %s \n", m_pFile.c_str() );
  printf( " Ply input dir   = %s \n", m_pDir.c_str() );
  printf( " Source file     = %s \n", m_pFileSrc.c_str() );
  printf( " Source dir      = %s \n", m_pDirSrc.c_str() );
  printf( " Output file     = %s \n", m_pRgbFile.c_str() );
  printf( " Depth map       = %d \n", m_bDepthMap );
  printf( " Frame number    = %d \n", m_iFrameNumber );
  printf( " Frame index     = %d \n", m_iFrameIndex );
  printf( " FPS             = %f \n", m_fFps );
  printf( " Play            = %d \n", m_bPlay );
  printf( " PlayBackward    = %d \n", m_bPlayBackward );
  printf( " Rotate          = %d \n", m_iRotate );
  printf( " Align           = %d \n", m_iAlign );
  printf( " BackgroundIndex = %d \n", m_iBackgroundIndex );
  printf( " BackgroundColor = ( %f, %f, %f ) \n",  m_eBackgroundColor[0],  m_eBackgroundColor[1],  m_eBackgroundColor[2]  );
  printf( " Center          = %d \n", m_bCenter );
  printf( " Point size      = %f \n", m_fPointSize );
  printf( " Point type      = %d \n", m_iPointType );
  printf( " Blend mode      = %s \n", m_iBlendMode==0?"0: Gaussian":"1: Linear");
  printf( " Point focus     = %f \n", m_fAlphaFalloff);
  printf( " Window size     = %d %d \n", m_iWidth, m_iHeight );
  printf( " Window pos      = %d %d \n", m_iPosX, m_iPosY );
  switch ( m_iScaleMode ) {
    case 0: printf( " Scale           = No scale [0;%d][0;%d][0;%d] \n", m_iBoxSize, m_iBoxSize, m_iBoxSize ); break;
    case 1: printf( " Scale           = by object [0;%d][0;%d][0;%d] \n", m_iBoxSize, m_iBoxSize, m_iBoxSize ); break;
    case 2: printf( " Scale           = by sequence [0;%d][0;%d][0;%d] \n", m_iBoxSize, m_iBoxSize, m_iBoxSize ); break;
    default:
      printf( " ERROR: scale mode to supported \n" );
      error();
      break;
  }
  printf( " Camera path     = %s \n", m_pCameraPathFile.c_str() );
  printf( " Camera path Idx = %d \n", m_iCameraPathIndex );
  printf( " Spline          = %d \n", m_bSpline );
  printf( " Viewpoint       = %s \n", m_pViewpointFile.c_str() );
  printf( " Overlay         = %d \n", m_bOverlay );
  printf( " Orthographic    = %d \n", m_bOrthographic );
  printf( " Synchronize     = %d \n", m_bSynchronize );
  printf( " Floor           = %d \n", m_bSynchronize );  
  printf( " FloorColor      = ( %f, %f, %f, %f ) \n", m_eFloorColor[0], m_eFloorColor[1], m_eFloorColor[2], m_eFloorColor[3] );
  printf( " ScenePath       = %s \n",  m_pScenePath.c_str() );
  printf( " SceneScale      = %f \n",  m_fSceneScale );
  printf( " ScenePosition   = ( %f, %f, %f ) \n", m_eScenePosition[0], m_eScenePosition[1], m_eScenePosition[2] );
  printf( " SceneRotation   = ( %f, %f, %f ) \n", m_eSceneRotation[0], m_eSceneRotation[1], m_eSceneRotation[2] );
  printf( " Visible         = %d \n",  m_bVisible );
  printf( " Lighting        = %d \n",  m_bLighting );
  printf( " SoftwareRenderer= %d \n",  m_bSoftwareRenderer );
}
