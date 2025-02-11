//Copyright(c) 2016 - 2025, InterDigital
//All rights reserved.
//See LICENSE under the root folder.

#ifndef _WINDOW_RENDERER_APP_H_
#define _WINDOW_RENDERER_APP_H_

#include "PccRendererDef.h"
#include "PccRendererCamera.h"
#include "PccRendererCameraPath.h"
#include "PccRendererText.h"
#include "PccRendererSequence.h"
#include "PccRendererObject.h"
#include "PccRendererPrimitive.h"

class RendererParameters;
/*! \class %Window
 * \brief %Window class.
 *
 *  This class is define a rendering window and all the rendering parameters.
 */
class Window {
 public:
  Window( std::string name, RendererParameters& params );
  ~Window();
  bool exist() { return m_pGlfwWindow != nullptr || m_bSoftwareRenderer; }
  void setSequence( Sequence* pcSequence );
  void setScene( Sequence* pcSphere, float scale, Vec3 position, Vec3 rotation );
  void draw();
  bool close();
  void log( const std::string& pString ) {
    if ( std::count( m_pLog.begin(), m_pLog.end(), '\n' ) > 12 ) { m_pLog = m_pLog.substr( m_pLog.find( '\n' ) + 1 ); }
    m_pLog += pString;
    m_dLogStart = glfwGetTime();
  }
  template <typename... Args>
  void log( const char* pFormat, Args... args ) {
    size_t      iSize = snprintf( nullptr, 0, pFormat, args... );
    std::string pString;
    pString.reserve( iSize + 1 );
    pString.resize( iSize );
    snprintf( &pString[0], iSize + 1, pFormat, args... );
    log( pString );
  }
  void softwareRendering();

 private:
  class Callback {
   public:
    Callback()                  = delete;
    Callback( const Callback& ) = delete;
    Callback( Callback&& )      = delete;
    ~Callback()                 = delete;
    static void setWindow( Window* pWindow );
    static void key( GLFWwindow* pGlfwWindow, int iKey, int scancode, int iAction, int /*unused*/ );
    static void mouseScroll( GLFWwindow* pGLFWwindow, double dDeltaX, double dDeltaY );
    static void resize( GLFWwindow* pGLFWwindow, int iWidth, int iHeight );

   private:
    static Window* m_pWindow;
  };
  void  set( RendererParameters& params );
  void  recordCameraPath();
  void  recordViewpoint();
  void  updateMatrix();
  void  update();
  void  wait();
  void  initialize();
  void  load();
  void  renderYuv();
  void  saveYuv( FILE* pFile );
  void  getMousePosition( double& fX0, double& fY0, double& fX1, double& fY1 );
  void  drawBox();
  void  drawObject();
  void  drawElements();
  void  drawOverlay();
  void  setCallback();
  void  mouseScroll( GLFWwindow* pGLFWwindow, double dDeltaX, double dDeltaY );
  void  resize( GLFWwindow* pGLFWwindow, int iWidth, int iHeight );
  void  key( GLFWwindow* pGlfwWindow, int key, int /*unused*/, int action, int /*unused*/ );
  float getAspectRatio() { return static_cast<float>( m_iWidth ) / static_cast<float>( m_iHeight ); }
  void  help();
  Mat4  getMatPro();
  Mat4  getMatMod();
  void  saveSceneCoordinate();

  GLFWwindow*     m_pGlfwWindow    = nullptr;
  Sequence*       m_pcSequence     = nullptr;
  Sequence*       m_pcScene        = nullptr;
  FILE*           m_pOutputRgbFile = nullptr;
  FILE*           m_pSaveRgbFile   = nullptr;
  std::string     m_sWindowName    = "";
  std::string     m_sViewpointFile;
  Camera          m_eCamera;
  CameraPath      m_eCameraPath;
  Text            m_eText;
  std::string     m_pLog;
  Background      m_eBackground;
  Sphere          m_eSphere;
  Cubes           m_eCubes;
  Axes            m_eAxes;
  Floor           m_eFloor;
  DistanceScale   m_eDistanceScale;
  PointList       m_ePointList;
  RenderToTexture m_eRenderToTexture;
  int             m_iWidth               = 800;
  int             m_iHeight              = 800;
  int             m_iWindowWidth         = 800;
  int             m_iWindowHeight        = 600;
  int             m_iWindowPositionX     = 100;
  int             m_iWindowPositionY     = 100;
  int             m_iFrameNumber         = 0;
  int             m_iRenderingIndex      = 0;
  double          m_dMouseX              = 0.0;
  double          m_dMouseY              = 0.0;
  double          m_dTime                = 0.0;
  double          m_dTimeLast            = 0.0;
  double          m_dTimeStart           = 0.0;
  double          m_dTimePath            = 0.0;
  double          m_dLogStart            = 0.0;
  bool            m_bSoftwareRenderer    = false;
  bool            m_bFullscreen          = false;
  bool            m_bFullscreenAsked     = false;
  bool            m_bReload              = false;
  bool            m_bSynchronize         = false;
  bool            m_bSynchronizeAsked    = false;
  bool            m_bDepthMap            = false;
  bool            m_bPause               = true;
  bool            m_bInteractive         = true;
  bool            m_bOverlay             = true;
  bool            m_bHelp                = false;
  bool            m_bLog                 = false;
  bool            m_bSave                = false;
  bool            m_bSaveViewpoint       = false;
  bool            m_bAddPoints           = false;
  bool            m_bSpline              = true;
  bool            m_bViewPoint           = false;
  bool            m_bCameraPath          = false;
  bool            m_bDrawPath            = false;
  bool            m_bDrawAxes            = false;
  bool            m_bDrawBox             = false;
  bool            m_bDrawSphere          = true;
  bool            m_bOrthographic        = false;
  bool            m_bDisplaySrc          = false;
  bool            m_bDisplayDuplicate    = false;
  bool            m_bFloor               = false;
  int             m_bAlign               = false;
  int             m_iZoom                = 0;
  bool            m_bSceneSaveCoordinate = false;
  bool            m_bRenderToTexture     = false;
  bool            m_bLighting            = true;
  int             m_iDisplayMetric       = 0;  // 0: off 1: point, 2,3,4: YUV
  int             m_iRotate              = 0;
  int             m_iForceColor          = 0;
  int             m_iTypeColor           = 0;
  int             m_iProgram             = 0;
  int             m_iBackgroundIndex     = 0;
  int             m_iAlign               = -1;
  int             m_iScroll              = 0;
  int             m_iShift               = 0;
  int             m_iSaveCameraPath      = 0;
  int             m_iSavedIndex          = 0;
  int             m_iCameraPathIndex     = -1;
  int             m_iMultiColorIndex     = -1;
  float           m_fPointSize           = 1.f;
  float           m_fAlphaFalloff        = 1.f;
  int             m_iBlendMode           = 0;
  float           m_fFov                 = 20.f;
  float           m_fSceneScale          = 1;
  float           m_fDeltaRotate         = 0.f;
  Vec3            m_eSceneRotation       = Vec3();
  Vec3            m_eScenePosition       = Vec3();
  Vec3            m_eBackgroundColor     = Vec3( -1 );
  Vec4            m_eFloorColor          = Vec4();
  Mat4            m_eMatMod              = Mat4();
  Mat4            m_eMatPro              = Mat4();
};
#endif  //~_WINDOW_RENDERER_APP_H_
