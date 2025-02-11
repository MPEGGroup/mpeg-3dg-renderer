//Copyright(c) 2016 - 2025, InterDigital
//All rights reserved.
//See LICENSE under the root folder.


#ifndef _CAMERA_PATH_RENDERER_APP_H_
#define _CAMERA_PATH_RENDERER_APP_H_

#include "PccRendererDef.h"

class Box;
/*! \class %CameraPath
 * \brief %CameraPath class.
 *
 *  This class is used to define a camera path.
 */
class CameraPath {
 private:
  class CameraPosition {
   public:
    CameraPosition( Vec3 ePosition      = Vec3(),
                    Vec3 eView          = Vec3(),
                    Vec3 eUp            = Vec3(),
                    bool bOrthographic  = false,
                    bool bFixedPosition = false,
                    int  iIndex         = 0 ) :
        m_ePosition( ePosition ),
        m_eView( eView ),
        m_eUp( eUp ),
        m_iIndex( iIndex ),
        m_bOrthographic( bOrthographic ),
        m_bFixedPosition( bFixedPosition ) {}
    CameraPosition( const CameraPosition& pos, int iIndex ) :
        m_ePosition( pos.m_ePosition ),
        m_eView( pos.m_eView ),
        m_eUp( pos.m_eUp ),
        m_iIndex( iIndex ),
        m_bOrthographic( pos.m_bOrthographic ),
        m_bFixedPosition( pos.m_bFixedPosition ) {}
    ~CameraPosition()       = default;
    CameraPosition& operator=( const CameraPosition& rhs ) = default;
    inline bool     operator==( CameraPosition& pos ) {
      return pos.pos() == m_ePosition && pos.view() == m_eView && pos.up() == m_eUp &&
             pos.orthographic() == m_bOrthographic && pos.fixedPosition() == m_bFixedPosition;
    }
    inline bool operator!=( CameraPosition& pos ) {
      return pos.pos() != m_ePosition || pos.view() != m_eView || pos.up() != m_eUp ||
             pos.orthographic() != m_bOrthographic || pos.fixedPosition() != m_bFixedPosition;
    }
    inline Vec3& pos() { return m_ePosition; }
    inline Vec3& view() { return m_eView; }
    inline Vec3& up() { return m_eUp; }
    inline bool& orthographic() { return m_bOrthographic; }
    inline bool& fixedPosition() { return m_bFixedPosition; }
    inline int&  index() { return m_iIndex; }
    inline void  setIndex( int index ) { m_iIndex = index; }

   private:
    Vec3 m_ePosition;
    Vec3 m_eView;
    Vec3 m_eUp;
    int  m_iIndex;
    bool m_bOrthographic;
    bool m_bFixedPosition;
  };

  class Spline {
   public:
    Spline();
    ~Spline();
    Spline& operator=( const Spline& rhs ) = default;
    void    add( int index, float fValue );
    bool    empty();
    void    compute();
    float   get( int index );
    void    clear();

   private:
    std::vector<Vec2> m_ePos;
    std::vector<Vec4> m_eCoef;
  };

 public:
  CameraPath();
  ~CameraPath();
  CameraPath& operator=( const CameraPath& rhs ) = default;
  bool        load( const std::string& pFilename );
  void        initialize();
  bool        save( const std::string& pFilename, int iStep = 30 );
  int         getNumPoints() { return static_cast<int>( m_ePath.size() ); }
  bool        exist() { return m_bExist; }
  void        getPose( Vec3& ePos, Vec3& eView, Vec3& eUp, bool& bSpline, bool& bOrthographic, int iIndex = -1 );
  void        trace();
  void        create( Box& box, size_t index );
  void        draw( bool bSpline );
  float       getDistance();
  inline int  getMaxIndex() { return m_ePath.back().index(); }
  inline int  getIndex() { return m_iIndex; }
  inline int  getLastIndex() { return m_iIndexLast; }
  inline void setIndex( int iIndex ) {
    m_iIndexLast = m_iIndex;
    m_iIndex     = iIndex < getMaxIndex() && iIndex >= 0 ? iIndex : 0;
  }
  void        add( Vec3 ePos, Vec3 eView, Vec3 eUp, bool bOrthographic, bool bFixedPosition, int iStep = 30 );
  inline void increaseIndex() {
    m_iIndexLast = m_iIndex;
    m_iIndex     = m_iIndex < getMaxIndex() ? m_iIndex + 1 : 0;
  }
  CameraPosition* getPoint( int i ) {
    if ( i >= 0 && i < static_cast<int>( m_ePath.size() ) ) { return &( m_ePath[i] ); }
    return nullptr;
  }
  void getListOfPoints( std::vector<Vec3>&   points,
                        std::vector<Color3>& color,
                        std::vector<Vec3>&   direction,
                        bool&                bSpline );

 private:
  void initSpline();
  bool haveFixedPosition();
  void addPoint( Box& box, int align, int zoom, bool orthographic, bool fixe, int duration );
  void addLine( Box& box, int align, int zoom1, int zoom2, bool orthographic, int duration );
  void addCurve( Box& box, int align1, int zoom1, int align2, int zoom2, bool orthographic, int duration );

  std::vector<CameraPosition> m_ePath;
  int                         m_iIndex     = 0;
  int                         m_iAddIndex  = 0;
  int                         m_iIndexLast = 0;
  Spline                      m_eSpline[9];
  bool                        m_bExist  = false;
  bool                        m_bSpline = false;
};

#endif  //~_CAMERA_PATH_RENDERER_APP_H_
