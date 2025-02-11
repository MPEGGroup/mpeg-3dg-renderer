#pragma once
//Copyright(c) 2016 - 2025, InterDigital
//All rights reserved.
//See LICENSE under the root folder.

#ifndef _OBJECT_RENDERER_APP_H_
#define _OBJECT_RENDERER_APP_H_

#include "PccRendererDef.h"
#include "PccRendererProgram.h"
#include "PccRendererCamera.h"

enum ObjectType { POINTCLOUD = 0, MESH = 1 };

class Box {
 public:
  Box() {
    m_eMin = Vec3( ( std::numeric_limits<float>::max )(), ( std::numeric_limits<float>::max )(),
                   ( std::numeric_limits<float>::max )() );
    m_eMax = Vec3( -( std::numeric_limits<float>::max )(), -( std::numeric_limits<float>::max )(),
                   -( std::numeric_limits<float>::max )() );
  }
  Box( Vec3 eMin, Vec3 eMax ) : m_eMin( eMin ), m_eMax( eMax ) {}
  inline Vec3& min() { return m_eMin; }
  inline Vec3& max() { return m_eMax; }
  inline Vec3  center() { return m_eMin + ( m_eMax - m_eMin ) / 2.f; }
  inline Vec3  size() { return m_eMax - m_eMin; }
  inline float getMaxSize() {
    auto maxSize = m_eMax - m_eMin;
    return ( std::max )( maxSize[0], ( std::max )( maxSize[1], maxSize[2] ) );
  }
  void update( const Box& eBox ) {
    for ( uint8_t i = 0; i < 3; i++ ) {
      if ( m_eMin[i] > eBox.m_eMin[i] ) { m_eMin[i] = eBox.m_eMin[i]; }
      if ( m_eMax[i] < eBox.m_eMax[i] ) { m_eMax[i] = eBox.m_eMax[i]; }
    }
  }
  void update( const Vec3& point ) {
    for ( uint8_t i = 0; i < 3; i++ ) {
      if ( m_eMin[i] > point[i] ) { m_eMin[i] = point[i]; }
      if ( m_eMax[i] < point[i] ) { m_eMax[i] = point[i]; }
    }
  }

 private:
  Vec3 m_eMin;
  Vec3 m_eMax;
};

class Object {
 public:
  Object() {}
  ~Object() {}
  virtual ObjectType         getType()                                     = 0;
  virtual void               load()                                        = 0;
  virtual void               unload()                                      = 0;
  virtual void               draw( bool lighting )                         = 0;
  virtual void               getRigPoints( std::vector<Vec3>&   points,
                                           std::vector<Color3>& colors,
                                           std::vector<Vec3>&   directions ) = 0;
  virtual void               recomputeBoundingBox()                        = 0;
  virtual void               center( Box box, float fBoxSize )             = 0;
  virtual void               scale( Box box, float fBoxSize )              = 0;
  virtual void               sortVertex(const Camera& cam)                 = 0;
  virtual void               loadProgram()                                 = 0;
  virtual std::string        getInformation()                              = 0;
  inline int                 getFrameIndex() { return m_iFrameIndex; }
  inline float               getMaxDistance() { return m_fMaxDistance; }
  inline Box&                getBox() { return m_eBox; }
  virtual Program&           getProgram()                            = 0;
  virtual int32_t            getProgramIndex()                       = 0;
  virtual void               setProgramIndex( int32_t programIndex ) = 0;
  virtual size_t             getProgramNumber()                      = 0;
  virtual const std::string& getProgramName()                        = 0;

  const std::string& getFilename() { return m_eFilename; }
  void               printBoundingBox( std::string string ) {
    printf( "%s: Frame %4d: BB=[%8.2f;%8.2f][%8.2f;%8.2f][%8.2f;%8.2f] \n", string.c_str(), m_iFrameIndex,
            m_eBox.min()[0], m_eBox.max()[0], m_eBox.min()[1], m_eBox.max()[1], m_eBox.min()[2], m_eBox.max()[2] );
  }
  void setCameraPosition( Vec3 eCameraPosition ) { m_eCameraPosition = eCameraPosition; }
  void setMultiColorIndex( int iMultiColorIndex ) { m_iMultiColorIndex = iMultiColorIndex; }
  void setDisplayMetric( int iDisplayMetric ) { m_iDisplayMetric = iDisplayMetric; }
  void setDisplayDuplicate( bool bDisplayDuplicate ) { m_bDisplayDuplicate = bDisplayDuplicate; }
  void setTypeColor( int iTypeColor ) { m_iTypeColor = iTypeColor; }
  int  getMultiColorIndex() { return m_iMultiColorIndex; }
  void setSource( Object* pSource ) { m_pcSource = pSource; }

 protected:
  int         m_iFrameIndex       = 0;
  int         m_iMultiColorIndex  = 0;
  int         m_iDisplayMetric    = 0;
  bool        m_bDisplayDuplicate = 0;
  int         m_iTypeColor        = 0;
  bool        m_bLoad             = false;
  float       m_fMaxDistance      = 255.f;
  Vec3        m_eCameraPosition   = Vec3(0,0,0);
  Box         m_eBox;
  std::string m_eFilename = "";
  Object*     m_pcSource  = nullptr;
};

#endif  //~_OBJECT_RENDERER_APP_H_
