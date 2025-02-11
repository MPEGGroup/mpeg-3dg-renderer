//Copyright(c) 2016 - 2025, InterDigital
//All rights reserved.
//See LICENSE under the root folder.

#ifndef _SEQUENCE_RENDERER_APP_H_
#define _SEQUENCE_RENDERER_APP_H_

#include "PccRendererDef.h"
#include "PccRendererObject.h"

/*! \class %Sequence class
 * \brief %Sequence class.
 *
 *  This class is used to read and store a data of severeal PLY models based on ObjectPointcloud object list.
 */
class Sequence {
 public:
  Sequence();
  ~Sequence();

  inline int                getFrameIndex() { return m_iFrameIndex; }
  Object&                   getObject();
  Object&                   getObject( int i );
  inline Box&               getBox() { return m_eBox; }
  const std::string&        getFilename();
  std::vector<std::string>& getTypeName() { return m_pTypeName; }
  ObjectType                getObjectType() { return m_eObject[0]->getType(); }
  void                      loadProgram() { m_eObject[0]->loadProgram(); }
  Program&                  getProgram() { return m_eObject[0]->getProgram(); }
  int32_t                   getProgramIndex() { return m_eObject[0]->getProgramIndex(); }
  void                      setProgramIndex( int32_t programIndex ) { m_eObject[0]->setProgramIndex( programIndex ); }
  size_t                    getProgramNumber() { return m_eObject[0]->getProgramNumber(); }
  const std::string&        getProgramName() { return m_eObject[0]->getProgramName(); }
  float                     getBoxSize() { return m_fBoxSize; }
  float                     getFps() { return m_fFps; }
  bool                      getHaveSource() { return !m_eObjectSrc.empty(); }
  bool                      getDisplaySource() { return m_bDisplaySource; }
  void                      setDropDups( int32_t iDropDups ) { m_iDropDups = iDropDups; }
  void                      setPlayBackward( bool bPlayBackward ) { m_bPlayBackward = bPlayBackward; }
  void                      setFrameIndex( int32_t iFrameIndex ) { m_iFrameIndex = iFrameIndex; }
  void                      load();
  void                      unload();
  bool                      check();

  void readDirectory( const std::string& sDir, int iFrameNumber, bool eBinary, bool bSource = false );
  void readFile( const std::string& sFile, int iFrameIndex, int iFrameNumber, bool eBinary, bool bSource = false );
  void normalize( int32_t iScaleMode, bool bCenter );
  void printBoundingBox( std::string string, bool bAll = false );
  void recomputeBoundingBox();
  int  getNumFrames() { return (int)( m_bPlayBackward ? 2 * (int)m_eObject.size() - 1 : (int)m_eObject.size() ); }
  void setDisplaySource( bool bDisplaySource ) { m_bDisplaySource = bDisplaySource; }
  void setBoxSize( float fBoxSize ) { m_fBoxSize = fBoxSize == 0 ? m_eBox.getMaxSize() : fBoxSize; }
  void setFps( float fFps ) { m_fFps = fFps; }

  Box getFloor() {
    const float scale  = 1.4f;
    Vec3        center = m_eBox.center();
    Vec3        size   = m_eBox.size() / 2.f;
    Vec2        height( m_eBox.min()[1] - size[1] / 25.f, m_eBox.min()[1] );
    Box         floor( Vec3( center[0] - scale * size[0], height[0], center[2] - scale * size[2] ),
               Vec3( center[0] + scale * size[0], height[1], center[2] + scale * size[2] ) );
    return floor;
  }

 private:
  void add( std::shared_ptr<Object> pObject, bool bSource );
  void getFileInDirector( std::string sDirector, std::string sExtension, std::vector<std::string>& eFileLists );
  void readDirectory( std::string pDirector, std::string pExtension, int iFrameNumber, bool bBinary, bool bSource );

 protected:
  int                                   m_iDropDups      = 2;
  int                                   m_iFrameIndex    = 0;
  bool                                  m_bPlayBackward  = false;
  bool                                  m_bDisplaySource = false;
  float                                 m_fBoxSize       = 1024.0f;
  float                                 m_fFps           = 25.0f;
  std::vector<std::shared_ptr<Object> > m_eObject;
  std::vector<std::shared_ptr<Object> > m_eObjectSrc;
  std::vector<std::string>              m_pTypeName;
  Box                                   m_eBox;
};

#endif  // _SEQUENCE_RENDERER_APP_H_
