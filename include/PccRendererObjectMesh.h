//Copyright(c) 2016 - 2025, InterDigital
//All rights reserved.
//See LICENSE under the root folder.

#ifndef _OBJECT_MESH_RENDERER_APP_H_
#define _OBJECT_MESH_RENDERER_APP_H_

#include "PccRendererDef.h"
#include "PccRendererObject.h"
#include "PccRendererProgram.h"
#include "PccRendererCamera.h"

struct Vertex {
  Vec3 position_;
  Vec4 color_;
  Vec2 texCoords_;
  Vec3 normal_;
};

struct Texture {
  GLuint               id_;
  std::string          type_;
  std::string          path_;
  std::vector<uint8_t> data_;
  int32_t              width_;
  int32_t              height_;

  // Note: the texture image accessors below were copied from mmMetrics / mmRendererSW

  // no sanity check for performance reasons
  inline Vec3 fetchRGB( const Vec2i& coord ) const {
    const size_t index = ( coord.y * width_ + coord.x ) * 3;
    return Vec3( data_[index + 0], data_[index + 1], data_[index + 2] );
  }

  // clamp the map i,j. j is flipped. mapCoord expressed in image space.
  inline Vec2i clampCoords( const Vec2i& mapCoord ) {
    return Vec2i( std::min( std::max( mapCoord.x, 0 ), width_ - 1 ),
                  height_ - 1 - std::min( std::max( mapCoord.y, 0 ), height_ - 1 ) );
  }

  // converts the uv coordinates from uv space to image space, doing CLAMP and v flip
  inline Vec2i mapCoordClamped( const Vec2& uv ) {
    return clampCoords( Vec2i( (int)( uv[0] * width_ ), (int)( uv[1] * height_ ) ) );
  }

  // texture lookup using ij in image space with clamp and j flip
  inline Vec3 image2D( const Vec2i& ij ) { return fetchRGB( clampCoords( ij ) ); }

  // texture lookup using uv (in uv space) with clamp and v flip
  inline Vec3 texture2D( const Vec2& uv ) { return fetchRGB( mapCoordClamped( uv ) ); }

  // texture lookup with bilinear filtering and v flip
  inline Vec3 texture2DBilinear( const Vec2& uv ) {
    const Vec2  pos  = uv * Vec2( width_, height_ ) - Vec2( 0.5 );
    const Vec2  f    = fract( pos );
    const Vec2i posi = glm::floor( pos );
    const Vec3  tl   = image2D( posi );
    const Vec3  tr   = image2D( posi + Vec2i( 1, 0 ) );
    const Vec3  bl   = image2D( posi + Vec2i( 0, 1 ) );
    const Vec3  br   = image2D( posi + Vec2i( 1, 1 ) );
    const Vec3  tA   = glm::mix( tl, tr, f.x );
    const Vec3  tB   = glm::mix( bl, br, f.x );
    return glm::mix( tA, tB, f.y );
  }
};

class Mesh {
 public:
  Mesh();
  Mesh( Box& box, Color4& eColor );
  ~Mesh();

  void                  load();
  void                  unload();
  void                  draw( Program& program, bool lighting );
  std::vector<Vertex>&  getVertices() { return m_eVertices; }
  std::vector<GLuint>&  getIndices() { return m_eIndices; }
  std::vector<Texture>& getTextures() { return m_eTexture; }
  Vertex&               getVertex( size_t i ) { return m_eVertices[i]; }
  GLuint&               getIndice( size_t i ) { return m_eIndices[i]; }
  Texture&              getTexture( size_t i ) { return m_eTexture[i]; }
  const Box&            getBox() { return m_eBox; }
  void                  recomputeBoundingBox();
  void                  center( Vec3 center );
  void                  scale( Vec3 center, float fScale );
  bool                  getUseColorPerVertex() { return m_bUseColorPerVertex; }
  void                  setUseColorPerVertex( bool bValue ) { m_bUseColorPerVertex = bValue; }
  size_t                getNumberOfVertices() { return m_eVertices.size(); }
  size_t                getNumberOfFaces() { return m_eIndices.size() / 3; }
  void                  computeFaceNormals( bool normalize = true );
  void                  computeVertexNormals( bool normalize = true, bool noSeams = true );
  void                  createBox( Box& box, Color4& eColor );

 private:
  inline void normalizeNormal( Vec3& normal ) const {
    normal = glm::normalize( normal );
    if ( std::isnan( normal[0] ) ) { normal = glm::vec3( 0.0F, 0.0F, 1.0F ); }
  }
  std::vector<Vertex>  m_eVertices;
  std::vector<Vec3>    m_eFaceNormals;
  std::vector<GLuint>  m_eIndices;
  std::vector<Texture> m_eTexture;
  GLuint               m_uiVAO;
  GLuint               m_uiVBO;
  GLuint               m_uiEBO;
  Box                  m_eBox;
  bool                 m_bLoad;
  bool                 m_bUseColorPerVertex;
};

class ObjectMesh : public Object {
 public:
  ObjectMesh();
  ~ObjectMesh();
  ObjectType         getType() { return ObjectType::MESH; }
  void               load();
  void               unload();
  void               draw( bool lighting );
  void               loadProgram();
  void               recomputeBoundingBox();
  void               center( Box box, float fBoxSize );
  void               scale( Box box, float fBoxSize );
  bool               read( std::string path, int32_t framesIndex );
  void               sortVertex(const Camera& cam) {} //Stub. Needed for PCC rendering
  void               getRigPoints( std::vector<Vec3>&, std::vector<Color3>&, std::vector<Vec3>& ) {}
  std::vector<Mesh>& getMeshes() { return m_eMeshes; }
  std::string        getInformation();
  Program&           getProgram() { return m_ePrograms[m_iProgramIndex]; }
  int32_t            getProgramIndex() { return m_iProgramIndex; }
  void               setProgramIndex( int32_t programIndex ) { m_iProgramIndex = programIndex; }
  size_t             getProgramNumber() { return m_ePrograms.size(); }
  const std::string& getProgramName() { return m_ePrograms[m_iProgramIndex].getName(); }
  void               computeVertexNormals();
  void               createBox( Box& box, Color4& eColor );
  

 private:
  bool                        readObj( std::string path, int32_t framesIndex );
  bool                        readPly( std::string path, int32_t framesIndex );
  void                        readTextures( const std::string& name, std::string type, std::vector<Texture>& textures );
  static std::vector<Program> m_ePrograms;
  static int32_t              m_iProgramIndex;
  std::vector<Mesh>           m_eMeshes;
  std::string                 m_eDirectory;
  int32_t                     m_iNumVertices = 0;
  int32_t                     m_iNumFaces    = 0;
  int32_t                     m_iNumTextures = 0;
};

#endif  //~_OBJECT_MESH_RENDERER_APP_H_
