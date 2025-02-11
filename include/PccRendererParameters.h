//Copyright(c) 2016 - 2025, InterDigital
//All rights reserved.
//See LICENSE under the root folder.


#ifndef _PARAMETERS_RENDERER_APP_H_
#define _PARAMETERS_RENDERER_APP_H_

#include "PccRendererDef.h"

class RendererParameters {
 public:
  RendererParameters();
  virtual ~RendererParameters();
  bool               parseCfg( int argc, char* argv[] );
  void               print();
  bool               check( bool verbose = false );
  void               error();
  inline std::string getFile() const { return m_pFile; }
  inline std::string getDir() const { return m_pDir; }
  inline std::string getFileSrc() const { return m_pFileSrc; }
  inline std::string getDirSrc() const { return m_pDirSrc; }
  inline std::string getRgbFile() const { return m_pRgbFile; }
  inline std::string getCameraPathFile() const { return m_pCameraPathFile; }
  inline std::string getViewpointFile() const { return m_pViewpointFile; }
  inline int         getFrameNumber() const { return m_iFrameNumber; }
  inline int         getFrameIndex() const { return m_iFrameIndex; }
  inline int         getAlign() const { return m_iAlign; }
  inline int         getWidth() const { return m_iWidth; }
  inline int         getHeight() const { return m_iHeight; }
  inline int         getPosX() const { return m_iPosX; }
  inline int         getPosY() const { return m_iPosY; }
  inline int         getPointType() const { return m_iPointType; }
  inline int         getBlendMode() const { return m_iBlendMode; }
  inline int         getMonitor() const { return m_iMonitor; }
  inline int         getBackgroundIndex() const { return m_iBackgroundIndex; }
  inline int         getBoxSize() const { return m_iBoxSize; }
  inline int         getScaleMode() const { return m_iScaleMode; }
  inline int         getRotate() const { return m_iRotate; }
  inline int         getDropDups() const { return m_iDropDups; }
  inline int         getCameraPathIndex() const { return m_iCameraPathIndex; }
  inline bool        getCenter() const { return m_bCenter; }
  inline bool        getPause() const { return !m_bPlay; }
  inline bool        getPlayBackward() const { return m_bPlayBackward; }
  inline bool        getSpline() const { return m_bSpline; }
  inline bool        getOverlay() const { return m_bOverlay; }
  inline bool        getOrthographic() const { return m_bOrthographic; }
  inline bool        getSynchronize() const { return m_bSynchronize; }
  inline bool        getBinaryFile() const { return m_bCreateBinaryFiles; }
  inline bool        getDepthMap() const { return m_bDepthMap; }
  inline bool        getFloor() const { return m_bFloor; }
  inline std::string getScenePath() const { return m_pScenePath; }
  inline float       getSceneScale() const { return m_fSceneScale; }
  inline Vec3        getScenePosition() const { return m_eScenePosition; }
  inline Vec3        getSceneRotation() const { return m_eSceneRotation; }
  inline Vec3        getBackgroundColor() const { return m_eBackgroundColor; }
  inline Vec4        getFloorColor() const { return m_eFloorColor; }
  inline float       getPointSize() const { return m_fPointSize; }
  inline float       getPointFocus() const { return m_fAlphaFalloff; }
  inline float       getFps() const { return m_fFps; }
  inline bool        getVisible() const { return m_bVisible; }
  inline bool        getLighting() const { return m_bLighting; }
  inline bool        getSoftwareRenderer() const { return m_bSoftwareRenderer; }

 private:
  std::string m_pFile;
  std::string m_pDir;
  std::string m_pFileSrc;
  std::string m_pDirSrc;
  std::string m_pRgbFile;
  std::string m_pCameraPathFile;
  std::string m_pViewpointFile;
  std::string m_pScenePath;
  int         m_iFrameNumber;
  int         m_iFrameIndex;
  int         m_iAlign;
  int         m_iWidth;
  int         m_iHeight;
  int         m_iPosX;
  int         m_iPosY;
  int         m_iPointType;
  int         m_iMonitor;
  int         m_iBackgroundIndex;
  int         m_iBoxSize;
  int         m_iScaleMode;
  int         m_iRotate;
  int         m_iDropDups;
  int         m_iCameraPathIndex;
  bool        m_bCenter;
  bool        m_bPlay;
  bool        m_bPlayBackward;
  bool        m_bCreateBinaryFiles;
  bool        m_bSpline;
  bool        m_bOverlay;
  bool        m_bSynchronize;
  bool        m_bDepthMap;
  bool        m_bFloor;
  bool        m_bVisible;
  bool        m_bOrthographic;
  bool        m_bLighting;
  bool        m_bSoftwareRenderer;
  float       m_fPointSize;
  int         m_iBlendMode;
  float       m_fAlphaFalloff;
  float       m_fFps;
  float       m_fSceneScale;
  Vec3        m_eScenePosition;
  Vec3        m_eSceneRotation;
  Vec3        m_eBackgroundColor;
  Vec4        m_eFloorColor;
};

#endif  //~_PARAMETERS_RENDERER_APP_H_
