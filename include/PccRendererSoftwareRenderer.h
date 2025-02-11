//Copyright(c) 2016 - 2025, InterDigital
//All rights reserved.
//See LICENSE under the root folder.

#ifndef m_SOFTWARE_RENDERER_RENDERER_APP_H_
#define m_SOFTWARE_RENDERER_RENDERER_APP_H_

#include "PccRendererDef.h"

class Image;
class Object;
class ObjectMesh;
class ObjectPointcloud;
class Mesh;
class Box;
struct ShaderMesh;

class SoftwareRenderer {
 public:
  SoftwareRenderer( Image& image, Mat4& eMatMod, Mat4& eMatPro, bool bLighting );
  ~SoftwareRenderer();
  void initialiaze();
  void drawBackground( Color3 eColor );
  void drawFloor( Box eFloor, Color4& eColor );
  void drawObject( Object& eObject );
  void drawObject( ObjectMesh& eObject );
  void drawObject( ObjectPointcloud& eObject );
  void drawMesh( Mesh& eMesh, ShaderMesh& shader );

 private:
  inline Vec2 projToScreen( const Vec4& proj ) const {
    return Vec2( ( 0.5 * ( proj[0] / proj[3] ) + 0.5 ) * m_eViewport[2] + m_eViewport[0],
                 ( 0.5 * ( proj[1] / proj[3] ) + 0.5 ) * m_eViewport[3] + m_eViewport[1] );
  }
  inline Mat3x2 projToScreen( const Mat3x4& proj ) const {
    return Mat3x2( projToScreen( proj[0] ), projToScreen( proj[1] ), projToScreen( proj[2] ) );
  }

  class ScreenArea {
   public:
    ScreenArea( const Vec2i& eSize ) : m_eSize( eSize - 1 ) {}
    inline void init() {
      m_eMin = Vec2i( std::numeric_limits<int>::max() );
      m_eMax = Vec2i( -std::numeric_limits<int>::max() );
    }
    inline void set( Mat3x2 screen ) {
      init();
      for ( int i = 0; i < 3; i++ ) {
        for ( int j = 0; j < 2; j++ ) {
          m_eMin[j] = std::max( std::min( m_eMin[j], (int)( screen[i][j] ) ), 0 );
          m_eMax[j] = std::min( std::max( m_eMax[j], (int)( screen[i][j] ) ), m_eSize[j] );
        }
      }
    }
    inline void set( Vec2 screen, float delta ) {
      init();
      for ( int i = 0; i < 2; i++ ) {
        m_eMin[i] = std::max( std::min( m_eMin[i], (int)( screen[i] - delta ) ), 0 );
        m_eMax[i] = std::min( std::max( m_eMax[i], (int)( screen[i] + delta ) ), m_eSize[i] );
      }
    }
    inline const Vec2i& min() const { return m_eMin; }
    inline const Vec2i& max() const { return m_eMax; }

   private:
    Vec2i m_eSize;
    Vec2i m_eMin;
    Vec2i m_eMax;
  };
  friend class ScreenArea;

  Mat4               m_eMVP;
  Mat4               m_eMatMod;
  Mat4               m_eMatNrm;
  Vec4               m_eViewport;
  Image&             m_eImage;
  std::vector<float> m_fDepth;
  bool               m_bLighting;
};

#endif
