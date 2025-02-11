//Copyright(c) 2016 - 2025, InterDigital
//All rights reserved.
//See LICENSE under the root folder.

#include "PccRendererWindow.h"
#include "PccRendererPrimitive.h"
#include "PccRendererParameters.h"
#include "PccRendererSoftwareRenderer.h"
#include "PccRendererImage.h"

Window::Window( std::string name, RendererParameters& params ) : m_sWindowName( name ) {
  m_iWidth            = params.getWidth();
  m_iHeight           = params.getHeight();
  m_iWindowWidth      = m_iWidth;
  m_iWindowHeight     = m_iHeight;
  m_bFullscreen       = m_iWidth == 0 || m_iHeight == 0;
  m_bSoftwareRenderer = params.getSoftwareRenderer();
  if ( !m_bSoftwareRenderer ) {
    if ( glfwInit() == GL_FALSE ) { return; }
#if defined( __APPLE__ )
    glfwWindowHint( GLFW_CONTEXT_VERSION_MAJOR, 3 );
    glfwWindowHint( GLFW_CONTEXT_VERSION_MINOR, 2 );
    glfwWindowHint( GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE );
    glfwWindowHint( GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE );
#endif
    if ( !( params.getRgbFile().empty() || params.getVisible() ) ) {
      glfwWindowHint( GLFW_VISIBLE, GLFW_FALSE );
      m_bRenderToTexture = true;
    }
    if ( m_bRenderToTexture ) {
      m_bOverlay    = false;
      m_bDrawSphere = false;
      m_dTime       = glfwGetTime();
      m_dTimeLast   = m_dTime;
      m_pGlfwWindow = glfwCreateWindow( 64, 64, m_sWindowName.c_str(), nullptr, nullptr );
      glfwMakeContextCurrent( m_pGlfwWindow );
      m_iWidth  = m_iWindowWidth;
      m_iHeight = m_iWindowHeight;
    } else {
      if ( m_bFullscreen ) {
        m_bFullscreenAsked = true;
        int  iNumMonitor   = 0;
        auto ppMonitor     = glfwGetMonitors( &iNumMonitor );
        int  iMonitor      = CLIP( params.getMonitor(), 0, iNumMonitor - 1 );
        auto info          = glfwGetVideoMode( ppMonitor[iMonitor] );
        m_pGlfwWindow      = glfwCreateWindow( info->width, info->height, name.c_str(), ppMonitor[iMonitor], nullptr );
      } else {
        m_pGlfwWindow = glfwCreateWindow( m_iWindowWidth, m_iWindowHeight, name.c_str(), nullptr, nullptr );
      }
      m_dTime     = glfwGetTime();
      m_dTimeLast = m_dTime;
      glfwMakeContextCurrent( m_pGlfwWindow );
      setCallback();
      glfwGetWindowSize( m_pGlfwWindow, &m_iWidth, &m_iHeight );
      if ( m_iWidth != params.getWidth() || m_iHeight != params.getHeight() ) {
        printf( "System can't initilialize window of size = %d x %x ( created window size = %d x %d ) \n",
                params.getWidth(), params.getHeight(), m_iWidth, m_iHeight );
        fflush( stdout );
        exit( -1 );
      }
    }
    m_dTime     = glfwGetTime();
    m_dTimeLast = m_dTime;
    gladLoadGL( glfwGetProcAddress );
    glfwSwapInterval( 1 );
    if ( GLAD_GL_VERSION_3_0 ) {}
    glClearColor( 0.f, 0.f, 0.f, 1.f );
    // GL version info
    std::cout << "GLFW version                : " << glfwGetVersionString() << std::endl;
    std::cout << "GL_VERSION                  : " << glGetString( GL_VERSION ) << std::endl;
    std::cout << "GL_VENDOR                   : " << glGetString( GL_VENDOR ) << std::endl;
    std::cout << "GL_RENDERER                 : " << glGetString( GL_RENDERER ) << std::endl;
    std::cout << "GL_SHADING_LANGUAGE_VERSION : " << glGetString( GL_SHADING_LANGUAGE_VERSION ) << std::endl;
    if ( m_bRenderToTexture ) { m_eRenderToTexture.load( m_iWidth, m_iHeight ); }
    initialize();
    resize( m_pGlfwWindow, m_iWidth, m_iHeight );
  }
  updateMatrix();
  log( g_pCopyrightString.c_str() );
  log( "PccAppRenderer starting: Press h for help...\n" );
  help();
  set( params );
}

Window::~Window() {
  FCLOSE( m_pOutputRgbFile );
  FCLOSE( m_pSaveRgbFile );
  if ( !m_bSoftwareRenderer ) { glfwTerminate(); }
}

void Window::initialize() {
  glEnable( GL_POINT_SMOOTH );
  glPointSize( 1.0 );
  glEnable( GL_BLEND );
  glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
  m_eText.initialize();
}
bool Window::close() { return glfwWindowShouldClose( m_pGlfwWindow ) == 1; }
void Window::wait() { glfwWaitEventsTimeout( 1.f / ( m_pcSequence->getFps() * 10.f ) ); }
void Window::help() {
  printf( "%s\n", g_pHelpString.c_str() );
  fflush( stdout );
}

void Window::drawOverlay() {
  if ( m_bPause ) {
    m_dTimeStart   = glfwGetTime();
    m_iFrameNumber = 0;
  }
  Vec3    eColor( 0, 0, 0 );
  Vec4    eBack( 0, 0, 0, 0 );
  float   fSize   = 16.f;
  Object& eObject = m_pcSequence->getObject();
  if ( m_bOverlay || m_bHelp || m_bLog || ( ( m_iDisplayMetric > 0 ) && m_pcSequence->getHaveSource() ) ||
       m_bDisplayDuplicate ) {
    const Color3& eBackground = m_iBackgroundIndex > 0 ? g_fBackground[m_iBackgroundIndex][0] : m_eBackgroundColor;
    float         fLum        = 0.2126f * eBackground[0] + 0.7152f * eBackground[1] + 0.0722f * eBackground[2];
    float         fColor      = fLum < 0.7f ? 1.f : 0.f;
    eColor                    = Vec3( fColor, fColor, fColor );
    eBack                     = Vec4( fColor, fColor, fColor, 0.2f );
  }
  if ( !m_bHelp && ( ( ( m_iDisplayMetric > 0 ) && m_pcSequence->getHaveSource() ) || m_bDisplayDuplicate ) ) {
    Vec3  eC( 1, 1, 1 );
    Vec4  eB( 0, 0, 0, 0 );
    int   iNum = 6;
    float fMax = eObject.getMaxDistance();
    for ( int i = 0; i <= 6; i++ ) {
      m_eText.initRect();
      m_eText.renderLine( stringFormat( "%5.1f", ( fMax * i ) / iNum ), 922, 50.f + i * 895.f / iNum, 10, eC, eB );
      m_eText.renderRect( fSize, eB );
    }
  }
  if ( m_bOverlay ) {
    float fX = 84, fY = 24;
    m_eText.initRect();
    int         size   = 70;
    std::string string = m_pcSequence->getFilename();
    std::string rotate = m_iRotate ? stringFormat( " Rot. = %d", m_iRotate ) : "";
    for ( int cut = static_cast<int>( string.length() ) / size; cut >= 0; cut-- ) {
      m_eText.renderLine( stringFormat( " %s %-70s ", cut == 0 ? "Model    =" : "          ",
                                        string.substr( cut * size, size ).c_str() ),
                          fX, fY, fSize, eColor, eBack );
      fY += fSize;
    }
    if ( m_pcSequence->getObjectType() == ObjectType::POINTCLOUD ) {
      if ( m_iMultiColorIndex == -3 ) {
        m_eText.renderLine( " Color    = main ", fX, fY, fSize, eColor, eBack );
      } else if ( m_iMultiColorIndex < 0 ) {
        m_eText.renderLine(
            stringFormat( " Color    = %s color ", m_iMultiColorIndex == -2 ? "closest" : "interpolate" ), fX, fY,
            fSize, eColor, eBack );
      } 
      else {
        m_eText.renderLine( stringFormat( " Color    = force color ( %d )", m_iMultiColorIndex ), fX, fY, fSize, eColor,
                            eBack );
      }
      fY += fSize;
    }
    m_eText.renderLine( stringFormat( "%s ", eObject.getInformation().c_str() ), fX, fY, fSize, eColor, eBack );
    fY += fSize;
    if (m_pcSequence->getObjectType() == ObjectType::POINTCLOUD && m_pcSequence->getProgramIndex() == 3) {
        auto s = "";
        if (m_iBlendMode == 0) s = "Gaussian";
        if (m_iBlendMode == 1) s = "Linear";        
        m_eText.renderLine(stringFormat(" Blend Mode = %-7s    Alpha Falloff = %5.2f", s , m_fAlphaFalloff),
                          fX, fY, fSize, eColor, eBack);
        fY += fSize;
    }

    m_eText.renderLine( stringFormat( " Frame    = %3d / %3d Program = %-9s Size = %6.2f Box = %-7.2f %s",
                                      m_pcSequence->getFrameIndex(), m_pcSequence->getNumFrames(),
                                      m_pcSequence->getProgramName().c_str(), m_fPointSize, m_pcSequence->getBoxSize(),
                                      rotate.c_str() ),
                        fX, fY, fSize, eColor, eBack );
    fY += fSize;
    if ( m_bCameraPath && m_eCameraPath.exist() ) {
      m_eText.renderLine(
          stringFormat( " Path     = %3d / %3d", m_eCameraPath.getLastIndex(), m_eCameraPath.getMaxIndex() + 1 ), fX,
          fY, fSize, eColor, eBack );
      fY += fSize;
    }
    if ( !m_bPause ) {
      m_eText.renderLine(
          stringFormat( " Index    = %9d FPS     = %8.2f  Duration = %8.2f", m_iFrameNumber,
                        m_iFrameNumber / ( glfwGetTime() - m_dTimeStart ), glfwGetTime() - m_dTimeStart ),
          fX, fY, fSize, eColor, eBack );
    }
    if ( m_iDisplayMetric > 0 && m_pcSequence->getHaveSource() ) {
      std::string eDistType[5] = {"", "Point", "Y", "U", "V"};
      m_eText.renderLine( stringFormat( " Distance from %s %s ", m_bDisplaySrc ? "source to main" : "main to source",
                                        eDistType[m_iDisplayMetric].c_str() ),
                          fX, fY, fSize, eColor, eBack );
      fY += fSize;
    }
    m_eText.renderRect( fSize, eBack );
  }
  if ( m_bHelp ) {
    m_eText.renderMultiLines( g_pHelpString, fSize, Vec3( 0.f, 0.f, 0.f ), Vec4( 1.f, 1.f, 1.f, 1.f ) );
  } else {
    if ( m_bLog || ( m_bOverlay && glfwGetTime() - m_dLogStart < 3 ) ) {
      m_eText.renderMultiLines( m_pLog, fSize, eColor, eBack );
    }
  }
}

void Window::drawObject() {
  if ( m_pcScene != nullptr ) {
    auto& eScene = m_pcScene->getObject();
    // glEnable( GL_CULL_FACE );    glCullFace( GL_BACK );
    eScene.setCameraPosition( m_eCamera.getPosition() );
    eScene.setMultiColorIndex( m_iMultiColorIndex );
    glEnable( GL_PROGRAM_POINT_SIZE );
    glDisable( GL_BLEND );
    glDisable( GL_POINT_SMOOTH );
    auto& programScene = m_pcScene->getProgram();
    programScene.use();
    glm::mat4 scale     = glm::scale( Vec3( m_fSceneScale, m_fSceneScale, m_fSceneScale ) );
    glm::mat4 translate = glm::translate( glm::mat4( 1 ), m_eScenePosition );
    glm::mat4 rotate    = glm::mat4( 1 );
    rotate              = glm::rotate( rotate, glm::radians( m_eSceneRotation[0] ), glm::vec3( 1.0f, 0.0f, 0.0f ) );
    rotate              = glm::rotate( rotate, glm::radians( m_eSceneRotation[1] ), glm::vec3( 0.0f, 1.0f, 0.0f ) );
    rotate              = glm::rotate( rotate, glm::radians( m_eSceneRotation[2] ), glm::vec3( 0.0f, 0.0f, 1.0f ) );
    glm::mat4 mat       = m_eMatMod * rotate * translate * scale;
    programScene.setUniform( "ModMat", mat );
    programScene.setUniform( "ProjMat", m_eMatPro );
    programScene.setUniform( "forceColor", static_cast<float>( m_iForceColor ) );
    glLineWidth( m_fPointSize );
    if ( m_pcScene->getProgramIndex() % 2 == 1 ) { programScene.setUniform( "PointSize", m_fPointSize ); }
    eScene.draw( m_bLighting );
    programScene.stop();
    glLineWidth( 1 );
    glEnable( GL_BLEND );
    glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
    glEnable( GL_POINT_SMOOTH );
    glDisable( GL_PROGRAM_POINT_SIZE );
    glEnable( GL_DEPTH_TEST );
  }
  auto& eObject = m_pcSequence->getObject();
  eObject.setCameraPosition( m_eCamera.getPosition() );
  eObject.setMultiColorIndex( m_iMultiColorIndex );
  glEnable( GL_PROGRAM_POINT_SIZE );
  glDisable( GL_BLEND );
  glDisable( GL_POINT_SMOOTH );
  m_iTypeColor =
      m_pcSequence->getTypeName().empty() ? 0 : m_iTypeColor % (int)( m_pcSequence->getTypeName().size() + 1 );
  auto& program = m_pcSequence->getProgram();
  program.use();
  program.setUniform( "ModMat", m_eMatMod );
  program.setUniform( "ProjMat", m_eMatPro );
  program.setUniform( "forceColor", static_cast<float>( m_iForceColor ) );
  if ( eObject.getType() == ObjectType::POINTCLOUD ) {
    program.setUniform( "PointSize", m_fPointSize );
    if ( m_pcSequence->getProgramIndex() == 0 ) { program.setUniform( "DepthMap", static_cast<int>( m_bDepthMap ) ); }
    program.setUniform( "posCamera", m_eCamera.getPosition() );
    if ( m_pcSequence->getProgramIndex() == 3 ) { 
        eObject.sortVertex(m_eCamera);
        program.setUniform("iBlendMode", m_iBlendMode);
        program.setUniform("fAlphaFalloff", m_fAlphaFalloff);
    }
  } else if ( eObject.getType() == ObjectType::MESH ) {
    glLineWidth( m_fPointSize );
    if ( m_pcSequence->getProgramIndex() == 1 ) { program.setUniform( "PointSize", m_fPointSize / 30.f ); }
    if ( m_pcSequence->getProgramIndex() == 3 ) {
      program.setUniform( "PointSize", m_fPointSize / m_pcSequence->getBox().getMaxSize() );
    }
  }
  eObject.draw( m_bLighting );
  program.stop();
  m_iMultiColorIndex = eObject.getMultiColorIndex();
  glLineWidth( 1 );
  glEnable( GL_BLEND );
  glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
  glEnable( GL_POINT_SMOOTH );
  glDisable( GL_PROGRAM_POINT_SIZE );
}

void Window::drawElements() {
  if ( !m_bCameraPath ) { m_dTimePath = 0.0; }
  if ( m_bDrawSphere && ( glfwGetMouseButton( m_pGlfwWindow, GLFW_MOUSE_BUTTON_1 ) != GLFW_RELEASE ||
                          glfwGetMouseButton( m_pGlfwWindow, GLFW_MOUSE_BUTTON_2 ) != GLFW_RELEASE ||
                          glfwGetMouseButton( m_pGlfwWindow, GLFW_MOUSE_BUTTON_3 ) != GLFW_RELEASE ) ) {
    m_eSphere.load( m_eCamera.getCenter(), m_pcSequence->getBoxSize() );
    m_eSphere.draw( m_eMatMod, m_eMatPro );
  }
  if ( m_bDrawPath ) {
    std::vector<Vec3>   points, direction;
    std::vector<Color3> color;
    m_eCameraPath.getListOfPoints( points, color, direction, m_bSpline );
    m_ePointList.load( points, color, direction );
    m_ePointList.draw( m_eMatMod, m_eMatPro );
  }
  if ( m_bDrawAxes ) {
    m_eAxes.load( m_pcSequence->getBoxSize() );
    m_eAxes.draw( m_eMatMod, m_eMatPro );
  }
  if ( m_bDrawBox ) {
    m_eCubes.load( m_pcSequence->getBoxSize(), m_pcSequence->getObject().getBox(), m_pcSequence->getBox() );
    m_eCubes.draw( m_eMatMod, m_eMatPro );
    std::vector<Vec3>   points, direction;
    std::vector<Color3> color;
    m_pcSequence->getObject().getRigPoints( points, color, direction );
    m_ePointList.load( points, color, direction );
    m_ePointList.draw( m_eMatMod, m_eMatPro, 25, true );
  }
  if ( !m_bHelp && ( ( ( m_iDisplayMetric > 0 ) && m_pcSequence->getHaveSource() ) || m_bDisplayDuplicate ) ) {
    m_eDistanceScale.load();
    m_eDistanceScale.draw();
  }
  drawOverlay();
}

void Window::draw() {
  update();
  m_eRenderToTexture.clear();
  m_eBackground.draw();
  if ( m_bFloor ) {
    m_eFloor.load( m_pcSequence->getFloor(), m_eFloorColor );
    m_eFloor.draw( m_eMatMod, m_eMatPro, m_bLighting );
  }
  drawObject();
  drawElements();
  glfwSwapBuffers( m_pGlfwWindow );
  m_eRenderToTexture.draw();
  if ( m_iSaveCameraPath != 0 ) { recordCameraPath(); }
  if ( m_bSaveViewpoint != 0 ) { recordViewpoint(); }
  if ( m_bSceneSaveCoordinate ) { saveSceneCoordinate(); }
  if ( m_iRenderingIndex > 0 && ( m_pOutputRgbFile || m_bSave ) ) { renderYuv(); }
  if ( !m_bRenderToTexture ) {}
  if ( m_iRenderingIndex == 0 ) { m_iRenderingIndex++; }
  if ( m_bInteractive ) { wait(); }
  glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
}

void Window::load() {
  if ( !m_pcSequence->getHaveSource() ) {
    m_bDisplaySrc    = false;
    m_iDisplayMetric = 0;
  }
  if ( m_iTypeColor > 0 && m_pcSequence->getTypeName().empty() ) { m_iTypeColor = 0; }
  m_pcSequence->setDisplaySource( m_bDisplaySrc );
  m_pcSequence->unload();
  auto& eObject = m_pcSequence->getObject();
  eObject.setDisplayMetric( m_iDisplayMetric );
  eObject.setDisplayDuplicate( m_bDisplayDuplicate );
  eObject.setTypeColor( m_iTypeColor );
  m_pcSequence->load();
  if ( m_pcScene != nullptr ) { m_pcScene->load(); }
  m_iFrameNumber++;
}

bool is_first= true; 
void Window::update() {
  bool   bNewPosition = is_first ? true :false;
  is_first = false;
  double dTime        = glfwGetTime();
  if ( !m_bRenderToTexture ) { glfwGetWindowSize( m_pGlfwWindow, &m_iWidth, &m_iHeight ); }
  if ( m_iBackgroundIndex < 0 ) {
    m_eBackground.load( m_eBackgroundColor );
  } else {
    m_eBackground.load( g_fBackground[m_iBackgroundIndex][0], g_fBackground[m_iBackgroundIndex][1] );
  }
  if ( m_pcSequence->getProgramIndex() != m_iProgram ) {
    m_iProgram = m_iProgram % ( static_cast<int>( m_pcSequence->getProgramNumber() ) );
    m_pcSequence->setProgramIndex( m_iProgram );
  }
  if ( !m_pOutputRgbFile ) {
    if ( m_bFullscreen != m_bFullscreenAsked ) {
      m_bFullscreen = m_bFullscreenAsked;
      GLFWwindow* pGlfwWindowNew;
      if ( m_bFullscreen ) {
        int           iNumMonitor = 0, iWinPosX = 0, iWinPosY = 0, iMonPosX = 0, iMonPosY = 0, iBest = 0;
        GLFWmonitor** ppMonitor = glfwGetMonitors( &iNumMonitor );
        glfwGetWindowPos( m_pGlfwWindow, &iWinPosX, &iWinPosY );
        m_iWindowWidth     = m_iWidth;
        m_iWindowHeight    = m_iHeight;
        m_iWindowPositionX = iWinPosX;
        m_iWindowPositionY = iWinPosY;
        for ( int i = 0; i < iNumMonitor; i++ ) {
          glfwGetMonitorPos( ppMonitor[i], &iMonPosX, &iMonPosY );
          if ( iMonPosX <= iWinPosX && iWinPosX < iMonPosX + glfwGetVideoMode( ppMonitor[i] )->width &&
               iMonPosY <= iWinPosX && iWinPosY < iMonPosY + glfwGetVideoMode( ppMonitor[i] )->height ) {
            iBest = i;
          }
        }
        auto info = glfwGetVideoMode( ppMonitor[iBest] );
        pGlfwWindowNew =
            glfwCreateWindow( info->width, info->height, m_sWindowName.c_str(), ppMonitor[iBest], m_pGlfwWindow );
        glfwDestroyWindow( m_pGlfwWindow );
        m_pGlfwWindow = pGlfwWindowNew;
        glfwMakeContextCurrent( m_pGlfwWindow );
      } else {
        pGlfwWindowNew =
            glfwCreateWindow( m_iWindowWidth, m_iWindowHeight, m_sWindowName.c_str(), nullptr, m_pGlfwWindow );
        glfwDestroyWindow( m_pGlfwWindow );
        m_pGlfwWindow = pGlfwWindowNew;
        glfwMakeContextCurrent( m_pGlfwWindow );
        glfwSetWindowPos( m_pGlfwWindow, m_iWindowPositionX, m_iWindowPositionY );
        glfwShowWindow( m_pGlfwWindow );
      }
      setCallback();
      initialize();
      m_bReload = true;
    }
    if ( m_bSynchronize != m_bSynchronizeAsked ) {
      m_bSynchronize = m_bSynchronizeAsked;     
      if ( !m_bSynchronize ) { m_eCamera.remove(); }
    }
    if ( m_iRotate != 0 ) {
      m_eCamera.rotate( 0.f, 0.f, (float)( dTime - m_dTimeLast ) * (float)( m_iRotate ) / 4.f, 0.f );
      bNewPosition = true;
    }
    if ( m_fDeltaRotate != 0 ) {
      m_eCamera.rotate( 0.f, 0.f, m_fDeltaRotate, 0.f );
      m_fDeltaRotate = 0.f;
      bNewPosition   = true;
    }
    if ( m_bAlign ) {
      m_eCamera.align( m_iAlign, m_pcSequence->getBox(), m_iZoom );
      bNewPosition = true;
      m_bAlign     = false;
    }
    double fX0 = 0, fY0 = 0, fX1 = 0, fY1 = 0;
    getMousePosition( fX0, fY0, fX1, fY1 );
    if ( glfwGetMouseButton( m_pGlfwWindow, GLFW_MOUSE_BUTTON_1 ) != GLFW_RELEASE ) {
      if ( glfwGetKey( m_pGlfwWindow, GLFW_KEY_LEFT_SHIFT ) ) {
        auto rotation = m_eCamera.getRotate( fX0, fY0, fX1, fY1 );
        m_eSceneRotation =
            glm::degrees( glm::eulerAngles( Quaternion( glm::radians( m_eSceneRotation ) ) * rotation ) );
      } else {
        m_eCamera.rotate( fX0, fY0, fX1, fY1 );
      }
      bNewPosition = true;
    }
    if ( glfwGetMouseButton( m_pGlfwWindow, GLFW_MOUSE_BUTTON_2 ) != GLFW_RELEASE ) {
      float x = static_cast<float>( ( fX0 - fX1 ) * m_iWidth ), y = static_cast<float>( ( fY0 - fY1 ) * m_iHeight );
      if ( glfwGetKey( m_pGlfwWindow, GLFW_KEY_LEFT_SHIFT ) ) {
        m_eScenePosition -= glm::toMat3( m_eCamera.getRotation() ) * Vec3( x / 10.f, y / 10.f, 0.0f );
      } else {
        m_eCamera.translate( x, y, 0.f );
      }
      bNewPosition = true;
    }
    if ( glfwGetMouseButton( m_pGlfwWindow, GLFW_MOUSE_BUTTON_3 ) != GLFW_RELEASE ) {
      if ( glfwGetKey( m_pGlfwWindow, GLFW_KEY_LEFT_SHIFT ) ) {
        m_fSceneScale *= 1.f + static_cast<float>( -( fY0 - fY1 ) * m_iHeight / 1000.f );
      } else {
        m_eCamera.zoom( static_cast<float>( -( fY0 - fY1 ) * m_iHeight * 3.f ) );
      }
      bNewPosition = true;
    }
    if ( m_iScroll != 0 ) {
      if ( glfwGetKey( m_pGlfwWindow, GLFW_KEY_LEFT_SHIFT ) ) {
        m_fSceneScale *= 1.f + (float)m_iScroll / 1500.f;
      } else {
        m_eCamera.zoom( static_cast<float>( m_iScroll ) );
      }
      m_iScroll    = 0;
      bNewPosition = true;
    }
    if ( m_bPause ) {
      if ( m_bReload || m_iShift != 0 ) {
        m_bPause = true;
        m_pcSequence->setFrameIndex(
            CLIP( m_pcSequence->getFrameIndex() + m_iShift, 0, m_pcSequence->getNumFrames() - 1 ) );
        bNewPosition = true;
        load();
      }
    } else {
      int iNewFrameIndex =
          static_cast<int>( m_pcSequence->getFps() * ( dTime - m_dTimeStart ) ) % ( m_pcSequence->getNumFrames() );
      if ( m_pcSequence->getFrameIndex() != iNewFrameIndex || m_bReload ) {
        if ( m_pcSequence->getFrameIndex() + 1 != iNewFrameIndex && iNewFrameIndex != 0 ) {
          log( "Frame index jump from %d to %d, you may have some resource problems\n", m_pcSequence->getFrameIndex(),
               iNewFrameIndex );
        }
        m_pcSequence->setFrameIndex( iNewFrameIndex );
        load();
        bNewPosition = true;
      }
    }
    m_iShift  = 0;
    m_bReload = false;
  }
  
  if ( bNewPosition && m_bSynchronizeAsked ) { 
    m_eCamera.write( m_bSynchronizeAsked, m_pcSequence->getFrameIndex() ); 
  }
  int iFrameIndex = m_pcSequence->getFrameIndex();  
  if( !bNewPosition){
    m_eCamera.read( m_bSynchronize, iFrameIndex );
  }
  if ( m_bSynchronizeAsked && m_pcSequence->getFrameIndex() != iFrameIndex ) {
    m_pcSequence->setFrameIndex( iFrameIndex % m_pcSequence->getNumFrames() );
    load();
  }
  if ( m_pOutputRgbFile && !m_bPause && m_bCameraPath ) {
    m_pcSequence->setFrameIndex(
        ( m_eCameraPath.exist() ? m_eCameraPath.getIndex() : m_pcSequence->getFrameIndex() + 1 ) %
        m_pcSequence->getNumFrames() );
    load();
  }
  m_bSynchronize = m_bSynchronizeAsked;
  updateMatrix();
  m_dTimeLast = dTime;
}

Mat4 Window::getMatPro() {
  float fovY, zNear, zFar;
  if ( !m_bOrthographic ) { m_eCamera.setFov( m_fFov ); }
  m_eCamera.getPerspective( fovY, zNear, zFar );
  if ( m_bOrthographic || fovY < 1.f ) {
    float fSizeX = m_eCamera.getDistance() * getAspectRatio() / 6.f, fSizeY = m_eCamera.getDistance() / 6.f;
    return glm::ortho( -fSizeX, fSizeX, -fSizeY, fSizeY, zNear, zFar );
  } else {
    GLdouble fH = zNear * std::tan( fovY * M_PI / 360 );
    GLdouble fW = fH * getAspectRatio();
    return Mat4( ( 2.f * zNear ) / ( 2.f * fW ), 0, 0, 0, 0, ( 2.f * zNear ) / ( 2.f * fH ), 0, 0, 0, 0,
                 -( zFar + zNear ) / ( zFar - zNear ), -1, 0, 0, -( 2 * zFar * zNear ) / ( zFar - zNear ), 0 );
  }
}

Mat4 Window::getMatMod() {
  Vec3 eEye, eCenter, eUp;
  if ( m_bCameraPath && m_eCameraPath.exist() ) {
    m_bDrawPath = false;
    m_eCameraPath.getPose( eEye, eCenter, eUp, m_bSpline, m_bOrthographic );
    if ( m_iRenderingIndex > 0 ) {
      if ( m_pOutputRgbFile ) {
        m_eCameraPath.increaseIndex();
      } else {
        double dTime = glfwGetTime();
        if ( m_dTimePath == 0.0 ) {
          m_dTimePath = dTime;
          if ( !m_bPause ) {
            m_dTimeStart = dTime;
            m_pcSequence->setFrameIndex( 0 );
          }
        }
        m_eCameraPath.setIndex( static_cast<int>( m_pcSequence->getFps() * ( dTime - m_dTimePath ) ) %
                                ( m_eCameraPath.getMaxIndex() ) );
      }
    }
    m_eCamera.setLookAt( eEye, eCenter, eUp );
  }
  m_eCamera.getLookAt( eEye, eCenter, eUp );
  return glm::lookAt( eEye, eCenter, eUp );
}

void Window::updateMatrix() {
  m_eMatMod = getMatMod();
  m_eMatPro = getMatPro();
}

void Window::softwareRendering() {
  std::vector<Image> eImages;
  eImages.resize( m_eCameraPath.getMaxIndex() );
#pragma omp parallel for
  for ( int i = 0; i < m_eCameraPath.getMaxIndex(); i++ ) {
    CameraPath eCameraPath = m_eCameraPath;
    Vec3       eEye, eCenter, eUp;
    eImages[i].allocate( m_iWidth, m_iHeight );
    eCameraPath.setIndex( i );
    eCameraPath.getPose( eEye, eCenter, eUp, m_bSpline, m_bOrthographic );
    auto             eMatMod = glm::lookAt( eEye, eCenter, eUp );
    auto             eMatPro = getMatPro();
    SoftwareRenderer renderer( eImages[i], eMatMod, eMatPro, m_bLighting );
    renderer.drawBackground( m_eBackgroundColor );
    if ( m_bFloor ) { renderer.drawFloor( m_pcSequence->getFloor(), m_eFloorColor ); }
    renderer.drawObject( m_pcSequence->getObject( m_bPause ? 0 : i % m_pcSequence->getNumFrames() ) );
  }
  for ( auto& eImage : eImages ) { eImage.write( m_pOutputRgbFile ); }
}

void Window::saveYuv( FILE* pFile ) {
  Image image( m_iWidth, m_iHeight );
  glReadBuffer( m_bRenderToTexture ? m_eRenderToTexture.getTexture() : GL_FRONT );
  glReadPixels( 0, 0, m_iWidth, m_iHeight, GL_RGB, GL_UNSIGNED_SHORT, image.data() );
  image.write( pFile, m_bDepthMap ? 1 : 3 );
  log( "saveYuv %dx%d  Pos = %4d Frame = %d \n", m_iWidth, m_iHeight,
       m_eCameraPath.exist() ? m_eCameraPath.getLastIndex() : -1, m_pcSequence->getFrameIndex() );
}


void   Window::renderYuv() {
  if ( m_pOutputRgbFile != nullptr ) {
    int total = 0;
    if ( m_eCameraPath.exist() ) {
      if ( m_eCameraPath.getIndex() == 0 ) {
        glfwSetWindowShouldClose( m_pGlfwWindow, GL_TRUE );
        return;
      }
      total = m_eCameraPath.getMaxIndex();
    } else {
      m_bPause            = false;
      int iSavedNumber = ( std::max )( 100, m_pcSequence->getNumFrames() );
      if ( ( m_iRotate == 0 && m_bPause ) || m_iSavedIndex == iSavedNumber ) {
        glfwSetWindowShouldClose( m_pGlfwWindow, GL_TRUE );
        return;
      }
      if ( m_iRotate > 0 ) { m_eCamera.rotate( m_iRotate * glm::pi<double>() / ( 2. * iSavedNumber ) ); }
      total = iSavedNumber;
    }
    saveYuv( m_pOutputRgbFile );
    PROGRESSBAR(m_iSavedIndex, total, "Exporting frame %d to rgb:", m_iSavedIndex+1);
    m_iSavedIndex++;
  } else {
    if ( m_bSave ) {
      if ( m_pSaveRgbFile == nullptr ) {
        std::string pString =
            stringFormat( "save_%s_%dx%d_%dbit_i444.rgb", getDate().c_str(), m_iWidth, m_iHeight, 16 );
        if ( ( m_pSaveRgbFile = fopen( pString.c_str(), "wb" ) ) == nullptr ) {
          log( "Error: output yuv file can't be open: %s \n", pString.c_str() );
          return;
        }
      }
      saveYuv( m_pSaveRgbFile );
    } else {
      FCLOSE( m_pSaveRgbFile );
    }
  }
}

void Window::recordViewpoint() {
  std::string pName = stringFormat( "viewpoint_%s.txt", getDate().c_str() );
  log( "Save viewpoint in the file: %s\n", pName.c_str() );
  m_eCamera.writeViewpoint( pName );
  m_bSaveViewpoint = false;
}

void Window::recordCameraPath() {
  Vec3 ePos, eView, eUp;
  switch ( m_iSaveCameraPath ) {
    case 1:
      log( "Record camera path \n" );
      m_eCameraPath.initialize();
      m_iSaveCameraPath = 2;
      m_bDrawPath       = false;
      break;
    case 3:
    case 4:
      m_eCamera.getLookAt( ePos, eView, eUp );
      m_eCameraPath.add( ePos, eView, eUp, m_bOrthographic, m_iSaveCameraPath == 4 );
      m_iSaveCameraPath = 2;
      log( "Save point in the camera path (%d points saved)\n", m_eCameraPath.getNumPoints() );
      break;
    case 5:
      if ( m_eCameraPath.getNumPoints() > 0 ) {
        auto pName = stringFormat( "camerapath_%s_%04d_points.txt", getDate().c_str(), m_eCameraPath.getNumPoints() );
        log( "Save camera path in the file: %s\n", pName.c_str() );
        m_eCameraPath.save( pName );
      }
      m_iSaveCameraPath = 0;
      break;
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

void Window::saveSceneCoordinate() {
  if ( m_pcScene != nullptr ) {
    std::string pScene = getBasename( getDirectory( m_pcScene->getFilename() ) ).c_str();
    auto        pName  = stringFormat( "%s_%s.txt", pScene.c_str(), getDate().c_str() );
    log( "Save scene orientation in the file: %s\n", pName.c_str() );
    std::ofstream file( pName.c_str() );
    if ( !file.is_open() ) {
      printf( "WINDOW: scene orientation file can't be write (%s).\n", pName.c_str() );
      fflush( stdout );
    }
    file << "# " << pScene << "\n";
    file << "scenePath  : " << m_pcScene->getFilename() << " # Scene obj path \n";
    file << "sceneScale : " << m_fSceneScale << " # Scene scale \n";
    file << "scenePos   : " << m_eScenePosition << " # Scene position \n";
    file << "sceneRot   : " << m_eSceneRotation << " # Scene position \n";
    file.close();
    m_bSceneSaveCoordinate = false;
  }
}

void Window::setSequence( Sequence* pcSequence ) {
  m_pcSequence = pcSequence;
  m_pcSequence->setFrameIndex( 0 );
  m_eCamera.create( m_pcSequence->getBoxSize() );
  m_eCamera.align( m_iAlign, m_pcSequence->getBox(), m_iZoom );
  if ( m_iCameraPathIndex >= 0 ) {
    m_eCameraPath.create( m_pcSequence->getBox(), m_iCameraPathIndex );
    Vec3 eEye, eCenter, eUp;
    m_eCameraPath.getPose( eEye, eCenter, eUp, m_bSpline, m_bOrthographic );
    m_eCamera.setLookAt( eEye, eCenter, eUp );
  }
  if (m_bViewPoint) {
      m_eCamera.readViewpoint(m_sViewpointFile);
  }
  if ( !m_bSoftwareRenderer ) {
    m_pcSequence->loadProgram();
    load();
  }
}

void Window::setScene( Sequence* pcScene, float scale, Vec3 position, Vec3 rotation ) {
  m_pcScene        = pcScene;
  m_fSceneScale    = scale;
  m_eSceneRotation = rotation;
  m_eScenePosition = position;
  m_pcScene->loadProgram();
  m_pcScene->setFrameIndex( 0 );
  if ( m_pcScene != nullptr ) { m_pcScene->load(); }
}

void Window::set( RendererParameters& params ) {
  if ( params.getWidth() != 0 && params.getPosX() != -1 && params.getPosY() != -1 ) {
    glfwSetWindowPos( m_pGlfwWindow, params.getPosX(), params.getPosY() );
  }
  m_fPointSize        = params.getPointSize();
  m_iProgram          = params.getPointType();
  m_iBlendMode        = params.getBlendMode();
  m_fAlphaFalloff     = params.getPointFocus();
  m_bSpline           = params.getSpline();
  m_iRotate           = params.getRotate();
  m_bPause            = params.getPause();
  m_bFloor            = params.getFloor();
  m_bDepthMap         = params.getDepthMap();
  m_bOverlay          = params.getOverlay();
  m_bOrthographic     = params.getOrthographic();
  m_bSynchronizeAsked = params.getSynchronize();
  m_eFloorColor       = params.getFloorColor();
  m_iBackgroundIndex  = params.getBackgroundIndex();
  m_eBackgroundColor  = params.getBackgroundColor();
  m_iAlign            = params.getAlign();
  m_iCameraPathIndex  = params.getCameraPathIndex();
  m_bLighting         = params.getLighting();

  if (!params.getViewpointFile().empty()) { m_sViewpointFile = params.getViewpointFile(); m_bViewPoint = true; }
  if ( !params.getCameraPathFile().empty() ) {
    log( "Load Camera path %s \n", params.getCameraPathFile().c_str() );
    if ( !m_eCameraPath.load( params.getCameraPathFile() ) ) { return; }
    Vec3 eEye, eCenter, eUp;
    m_eCameraPath.getPose( eEye, eCenter, eUp, m_bSpline, m_bOrthographic );
    m_eCamera.setLookAt( eEye, eCenter, eUp );
  }
  if ( !params.getRgbFile().empty() ) {
    FCLOSE( m_pOutputRgbFile );
    std::string pString = stringFormat( "%s_%s_%dx%d_%dbit%s", params.getRgbFile().c_str(), getDate().c_str(), m_iWidth,
                                        m_iHeight, 16, m_bDepthMap ? ".y" : "_i444.rgb" );
    m_pOutputRgbFile    = fopen( pString.c_str(), "wb" );
    if ( m_pOutputRgbFile == nullptr ) {
      log( "Error: output file can't be open: %s \n", pString.c_str() );
      return;
    }
    m_bCameraPath  = true;
    m_bInteractive = false;
  }
}

// GLFW CALLBACK
Window* Window::Callback::m_pWindow = nullptr;
void    Window::Callback::setWindow( Window* pWindow ) { Callback::m_pWindow = pWindow; }
void    Window::Callback::key( GLFWwindow* window, int key, int scancode, int action, int mods ) {
  m_pWindow->key( window, key, scancode, action, mods );
}
void Window::Callback::mouseScroll( GLFWwindow* window, double dDeltaX, double dDeltaY ) {
  m_pWindow->mouseScroll( window, dDeltaX, dDeltaY );
}
void Window::Callback::resize( GLFWwindow* window, int iWidth, int iHeight ) {
  m_pWindow->resize( window, iWidth, iHeight );
}
void Window::setCallback() {
  Callback::setWindow( this );
  glfwSetKeyCallback( m_pGlfwWindow, Callback::key );
  glfwSetFramebufferSizeCallback( m_pGlfwWindow, Callback::resize );
  glfwSetScrollCallback( m_pGlfwWindow, Callback::mouseScroll );
}

void Window::resize( GLFWwindow* pGlfwWindow, int iWidth, int iHeight ) {
  int iWinPosX = 0, iWinPosY = 0;
  glfwMakeContextCurrent( pGlfwWindow );
  glViewport( 0, 0, iWidth, iHeight );
  glfwGetWindowPos( pGlfwWindow, &iWinPosX, &iWinPosY );
}

void Window::getMousePosition( double& dX0, double& dY0, double& dX1, double& dY1 ) {
  if ( m_bInteractive ) {
    const int r = m_iWidth < m_iHeight ? m_iWidth : m_iHeight;
    dX0         = m_dMouseX;
    dY0         = m_dMouseY;
    glfwGetCursorPos( m_pGlfwWindow, &m_dMouseX, &m_dMouseY );
    m_dMouseX = ( 2.0 * m_dMouseX - m_iWidth ) / r;
    m_dMouseY = ( m_iHeight - 2.0 * m_dMouseY ) / r;
    dX1       = m_dMouseX;
    dY1       = m_dMouseY;
  }
}

void Window::mouseScroll( GLFWwindow* pWindow, double /*unused*/, double dDeltaY ) {
  if ( m_bInteractive ) {
    if ( glfwGetKey( pWindow, GLFW_KEY_LEFT_ALT ) || glfwGetKey(pWindow, GLFW_KEY_RIGHT_ALT)) {
      if ( dDeltaY > 0 ) {
        m_fPointSize = CLIP( m_fPointSize * ( 1.f + (float)dDeltaY / 8.f ), 0.001f, 20.f );
      } else {
        m_fPointSize = CLIP( m_fPointSize / ( 1.f - (float)dDeltaY / 8.f ), 0.001f, 20.f );
      }
    } else if ( glfwGetKey( pWindow, GLFW_KEY_LEFT_CONTROL ) || glfwGetKey(pWindow, GLFW_KEY_RIGHT_CONTROL)) {
      m_iScroll += static_cast<int>( 50.f * dDeltaY );
    } else {
      m_iScroll += static_cast<int>( 250.f * dDeltaY );
    }
  }
}

void Window::key( GLFWwindow* pGlfwWindow, int key, int scancode, int action, int /*unused*/ ) {
  if ( key == GLFW_KEY_ESCAPE || key == GLFW_KEY_A ) { glfwSetWindowShouldClose( pGlfwWindow, GL_TRUE ); }
  if ( m_bInteractive ) {
    if ( glfwGetKey( pGlfwWindow, GLFW_KEY_LEFT_SHIFT ) || glfwGetKey(pGlfwWindow, GLFW_KEY_RIGHT_SHIFT)) {
      switch ( key ) {
        case GLFW_KEY_S: m_fSceneScale *= 1.1f; break;
        case GLFW_KEY_X: m_fSceneScale /= 1.1f; break;
        case GLFW_KEY_D: m_eScenePosition[0] += 50.f; break;
        case GLFW_KEY_C: m_eScenePosition[0] -= 50.f; break;
        case GLFW_KEY_F: m_eScenePosition[1] += 50.f; break;
        case GLFW_KEY_V: m_eScenePosition[1] -= 50.f; break;
        case GLFW_KEY_G: m_eScenePosition[2] += 50.f; break;
        case GLFW_KEY_B: m_eScenePosition[2] -= 50.f; break;
        case GLFW_KEY_N: m_eSceneRotation[0] += 10.f; break;
        case GLFW_KEY_H: m_eSceneRotation[0] -= 10.f; break;
        case GLFW_KEY_J: m_eSceneRotation[1] += 10.f; break;
        case GLFW_KEY_M: m_eSceneRotation[1] -= 10.f; break;
        case GLFW_KEY_K: m_eSceneRotation[2] += 10.f; break;
        case GLFW_KEY_COMMA: m_eSceneRotation[2] -= 10.f; break;
        case GLFW_KEY_Z: m_bSceneSaveCoordinate = true; break;
      }
    }
    if ( action == GLFW_PRESS ) {
      switch ( key ) {
        case GLFW_KEY_Q:
          m_bDisplaySrc = !m_bDisplaySrc;
          m_bReload     = true;
          m_iTypeColor  = 0;
          break;
        case GLFW_KEY_W:
          m_iDisplayMetric    = ( m_iDisplayMetric + 1 ) % 5;
          m_bReload           = true;
          m_iTypeColor        = 0;
          m_bDisplayDuplicate = false;
          break;
        case GLFW_KEY_F:
          m_bDisplayDuplicate = !m_bDisplayDuplicate;
          m_iDisplayMetric    = 0;
          m_bReload           = true;
          m_iTypeColor        = 0;
          break;
        case GLFW_KEY_E:
          m_iTypeColor        = m_iTypeColor + 1;
          m_bReload           = true;
          m_bDisplayDuplicate = false;
          m_iDisplayMetric    = 0;
          break;
        case GLFW_KEY_B: m_bDrawBox = !m_bDrawBox; break;
        case GLFW_KEY_C: m_iBackgroundIndex = ( m_iBackgroundIndex + 1 ) % g_fBackground.size(); break;
        case GLFW_KEY_D: m_bOverlay = !m_bOverlay; break;
        case GLFW_KEY_G: m_bCameraPath = !m_bCameraPath; break;
        case GLFW_KEY_H: m_bHelp = !m_bHelp; break;
        case GLFW_KEY_I: m_iSaveCameraPath = m_iSaveCameraPath == 2 ? 3 : m_iSaveCameraPath; break;
        case GLFW_KEY_J: m_bLighting = !m_bLighting; break;
        case GLFW_KEY_K: m_iSaveCameraPath = m_iSaveCameraPath == 2 ? 4 : m_iSaveCameraPath; break;
        case GLFW_KEY_L:
          m_iCameraPathIndex = m_iCameraPathIndex < 0 || m_iCameraPathIndex == 9 ? 0 : m_iCameraPathIndex + 1;
          m_eCameraPath.create( m_pcSequence->getBox(), m_iCameraPathIndex );
          break;
        case GLFW_KEY_SEMICOLON: m_bSpline = !m_bSpline; break;
        case GLFW_KEY_O: m_bOrthographic = !m_bOrthographic; break;
        case GLFW_KEY_P: m_bDrawPath = !m_bDrawPath; break;
        case GLFW_KEY_S: m_bSynchronizeAsked = !m_bSynchronizeAsked; break;
        case GLFW_KEY_N: m_iForceColor = m_iForceColor == 4 ? 0 : m_iForceColor + 1; break;
        case GLFW_KEY_R: m_iProgram++; break;
        case GLFW_KEY_T: m_bLog = !m_bLog; break;
        case GLFW_KEY_U: m_iSaveCameraPath = m_iSaveCameraPath == 0 ? 1 : 5; break;
        case GLFW_KEY_V: m_bDrawSphere = !m_bDrawSphere; break;
        case GLFW_KEY_X: m_bDrawAxes = !m_bDrawAxes; break;
        case GLFW_KEY_Y: m_bSaveViewpoint = true; break;
        case GLFW_KEY_Z: m_bSave = !m_bSave; break;
        case GLFW_KEY_SPACE: m_bPause = !m_bPause; break;
        case GLFW_KEY_F11: m_bFullscreenAsked = !m_bFullscreenAsked; break;
        case GLFW_KEY_UP: m_iShift = -999999; break;
        case GLFW_KEY_DOWN: m_iShift = 999999; break;
        case GLFW_KEY_TAB:
          m_iMultiColorIndex = glfwGetKey( pGlfwWindow, GLFW_KEY_LEFT_CONTROL ) || glfwGetKey(pGlfwWindow, GLFW_KEY_RIGHT_CONTROL) ? -3 : m_iMultiColorIndex + 1;
          m_bReload          = true;
          break;
        case GLFW_KEY_1:
        case GLFW_KEY_2:
          m_iAlign = glfwGetKey( pGlfwWindow, GLFW_KEY_1 ) ? ( m_iAlign + 2 ) % 11 : m_iAlign < 2 ? 10 : m_iAlign - 2;
          m_iAlign &= ~( 1UL );
          m_bAlign = true;
          break;
        case GLFW_KEY_3:
        case GLFW_KEY_4:
          m_iAlign = glfwGetKey( pGlfwWindow, GLFW_KEY_3 ) ? ( m_iAlign + 1 ) % 8 : !m_iAlign ? 7 : m_iAlign - 1;
          m_bAlign = true;
          break;
        case GLFW_KEY_5: m_fDeltaRotate = -0.02f; break;
        case GLFW_KEY_6: m_fDeltaRotate = 0.02f; break;
        case GLFW_KEY_7:
          if ( m_iRotate < 4 ) { m_iRotate++; }
          break;
        case GLFW_KEY_8:
          if ( m_iRotate > 0 ) { m_iRotate--; }
          break;
        case GLFW_KEY_9:
          m_iZoom  = m_iZoom == 5 ? 0 : m_iZoom + 1;
          m_bAlign = true;
          break;
        case GLFW_KEY_0:
          m_iZoom  = m_iZoom == 0 ? 5 : m_iZoom - 1;
          m_bAlign = true;
          break;
        case GLFW_KEY_LEFT: m_iShift--; break;
        case GLFW_KEY_RIGHT: m_iShift++; break;        
        case GLFW_KEY_PAGE_DOWN: m_fAlphaFalloff -= 0.1f; break;
        case GLFW_KEY_PAGE_UP: m_fAlphaFalloff += 0.1f; break;
        case GLFW_KEY_END: m_iBlendMode = (m_iBlendMode + 1) % 2; break;
        
        //Don't print message when command keys managed elsewhere are pressed
        case GLFW_KEY_LEFT_ALT:
        case GLFW_KEY_LEFT_CONTROL:
        case GLFW_KEY_LEFT_SHIFT:
        case GLFW_KEY_RIGHT_ALT:
        case GLFW_KEY_RIGHT_CONTROL:
        case GLFW_KEY_RIGHT_SHIFT: 
            break;
        default: printf( "key %d '%s 'not supported \n", key, glfwGetKeyName(key, scancode)); break;
      }
    } else {
      if ( action != GLFW_RELEASE ) {
        switch ( key ) {
          case GLFW_KEY_LEFT: m_iShift--; break;
          case GLFW_KEY_RIGHT: m_iShift++; break;
          case GLFW_KEY_5: m_fDeltaRotate = -0.02f; break;
          case GLFW_KEY_6: m_fDeltaRotate = 0.02f; break;
          case GLFW_KEY_PAGE_DOWN: m_fAlphaFalloff -= 0.1f; break;
          case GLFW_KEY_PAGE_UP: m_fAlphaFalloff += 0.1f; break;
        }
      }
    }
  }
}
