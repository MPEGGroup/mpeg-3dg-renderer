//Copyright(c) 2016 - 2025, InterDigital
//All rights reserved.
//See LICENSE under the root folder.


#include "PccRendererObjectMesh.h"
#include "PccRendererShader.h"

#include <tinyply.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"

std::vector<Program> ObjectMesh::m_ePrograms     = {};
int32_t              ObjectMesh::m_iProgramIndex = 0;

// Mesh shaders
// clang-format off
static const std::string g_pMeshVertexShader = SHADER(
  layout ( location = 0 ) in vec3 position;  
  layout ( location = 1 ) in vec4 color;
  layout ( location = 2 ) in vec2 texCoords;
  layout ( location = 3 ) in vec3 normal;
  out vec4 Color;
  out vec2 TexCoords;
  out vec3 Normal;
  uniform mat4 ProjMat;
  uniform mat4 ModMat;
  void main() {
    gl_Position = ProjMat * ModMat * vec4( position, 1.f );
    TexCoords = texCoords;
    Color = color;
    Normal = vec3( transpose( inverse( ModMat ) ) * vec4( normal, 0.f ) );
  }
);

static const std::string g_pVertexVertexShader = SHADER(
  layout ( location = 0 ) in vec3 position;
  layout ( location = 1 ) in vec4 color;
  layout ( location = 2 ) in vec2 texCoords;
  layout ( location = 3 ) in vec3 normal;
  out vec2 TexCoords;
  out vec4 Color;
  out vec3 Normal;
  uniform mat4  ProjMat;
  uniform mat4  ModMat;
  uniform float PointSize;
  void main() {
    gl_Position = ProjMat * ModMat * vec4( position, 1.f );
    gl_PointSize = PointSize * 4750.0 / gl_Position.w;
    TexCoords = texCoords;
    Color = color;
    Normal = vec3( transpose( inverse( ModMat ) ) * vec4( normal, 0.f ) );
  }  
);

static const std::string g_pWireframeGeometryShader = SHADER( 
  layout(triangles) in;
  layout(triangle_strip, max_vertices = 3) out;
  in vec2 TexCoords[]; 
  in vec4 Color[];
  in vec3 Normal[];
  out vec2 oTexCoords;
  out vec4 oColor;
  out vec3 oNormal;
  out vec3 distance;
  void main() {
      float a = length( gl_in[1].gl_Position.xyz - gl_in[2].gl_Position.xyz );
      float b = length( gl_in[2].gl_Position.xyz - gl_in[0].gl_Position.xyz );
      float c = length( gl_in[1].gl_Position.xyz - gl_in[0].gl_Position.xyz );
      float alpha = abs( sin( acos( ( b * b + c * c - a * a ) / ( 2.0 * b * c ) ) ) );
      float beta  = abs( sin( acos( ( a * a + c * c - b * b ) / ( 2.0 * a * c ) ) ) );
      oTexCoords = TexCoords[0];
      oColor = Color[0];
      oNormal = Normal[0];
      distance = vec3( c * beta / sqrt( gl_in[0].gl_Position.w ) , 0, 0 );
      gl_Position = gl_in[0].gl_Position;
      EmitVertex();
      oTexCoords = TexCoords[1];
      oColor = Color[1];
      oNormal = Normal[1];
      distance = vec3( 0, c * alpha / sqrt( gl_in[1].gl_Position.w ), 0 );
      gl_Position = gl_in[1].gl_Position;
      EmitVertex();
      oTexCoords = TexCoords[2];
      oColor = Color[2];
      oNormal = Normal[2];
      distance = vec3( 0, 0, b * alpha / sqrt( gl_in[2].gl_Position.w ) );
      gl_Position = gl_in[2].gl_Position;
      EmitVertex();
  }
);

static const std::string g_pMeshFragmentShader = SHADER( 
  in      vec2      TexCoords;
  in      vec4      Color;
  in      vec3      Normal;
  out     vec4      color;
  uniform sampler2D texture_diffuse;
  uniform vec3      materialAmbient;
  uniform vec3      materialDiffuse;
  uniform vec3      lightColor;
  uniform vec3      lightDirection;
  uniform float     forceColor;
  uniform float     hasTexture;
  uniform float     lighting;
  void main() {
    vec4 rgb;
    if ( forceColor <  0.5 ) { 
     rgb = hasTexture > 0.5 ? vec4( texture( texture_diffuse, vec2( TexCoords.x, 1.0 - TexCoords.y ) ) ) : Color; 
    } else { 
      rgb = vec4( vec3( ( forceColor - 1.f ) / 3.f ), 1.0 );
    }
    if( lighting < 0.5 ) {
      color = rgb;
    } else {      
      float diff   = max( dot( normalize( Normal ), normalize( lightDirection ) ), 0.0 );
      vec3 ambient = vec3( rgb ) * materialAmbient;
      vec3 diffuse = vec3( rgb ) * materialDiffuse * lightColor * diff;        
      color        = vec4( vec3( clamp( ambient + diffuse, 0.0, 1.0 ) ), 1.0 );
    }
  }
);

static const std::string g_pWireframeFragmentShader = SHADER(   
  in vec2           oTexCoords;
  in vec4           oColor;
  in vec3           oNormal;
  in vec3           distance;
  out vec4          color; 
  uniform sampler2D texture_diffuse;
  uniform vec3      materialAmbient;
  uniform vec3      materialDiffuse;
  uniform vec3      lightColor;
  uniform vec3      lightDirection;
  uniform float     lighting;
  uniform float     forceColor;
  uniform float     hasTexture;
  uniform float     PointSize;
  void main() {
    if ( min( distance[0], min( distance[1], distance[2] ) ) < PointSize ) {
        color = vec4( 0.5  );
    } else {   
      vec4 rgb;
      if ( forceColor <  0.5 ) { 
      rgb = hasTexture > 0.5 ? vec4( texture( texture_diffuse, vec2( oTexCoords.x, 1.0 - oTexCoords.y ) ) ) : oColor; 
      } else { 
        rgb = vec4( vec3( ( forceColor - 1.f ) / 3.f ), 1.0 );
      }
      if( lighting < 0.5 ) {
        color = rgb;
      } else {      
        float diff   = max( dot( normalize( oNormal ), normalize( lightDirection ) ), 0.0 );
        vec3 ambient = vec3( rgb ) * materialAmbient;
        vec3 diffuse = vec3( rgb ) * materialDiffuse * lightColor * diff;        
        color        = vec4( vec3( clamp( ambient + diffuse, 0.0, 1.0 ) ), 1.0 );
      }
    }
  }
);

// clang-format on

Mesh::Mesh() : m_bLoad( false ), m_bUseColorPerVertex( true ) {}
Mesh::~Mesh() {
  m_eVertices.clear();
  m_eIndices.clear();
  m_eTexture.clear();
  m_eFaceNormals.clear();
}

Mesh::Mesh( Box& box, Color4& eColor ) : m_bLoad( false ), m_bUseColorPerVertex( true ) { createBox( box, eColor ); }

void Mesh::createBox( Box& box, Color4& eColor ) {
  static const std::vector<Vec3u8> boxVerticesIdx = {
      {0, 0, 0}, {0, 0, 1}, {0, 1, 1}, {1, 1, 0}, {0, 0, 0}, {0, 1, 0}, {1, 0, 1}, {0, 0, 0}, {1, 0, 0},
      {1, 1, 0}, {1, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 1, 1}, {0, 1, 0}, {1, 0, 1}, {0, 0, 1}, {0, 0, 0},
      {0, 1, 1}, {0, 0, 1}, {1, 0, 1}, {1, 1, 1}, {1, 0, 0}, {1, 1, 0}, {1, 0, 0}, {1, 1, 1}, {1, 0, 1},
      {1, 1, 1}, {1, 1, 0}, {0, 1, 0}, {1, 1, 1}, {0, 1, 0}, {0, 1, 1}, {1, 1, 1}, {0, 1, 1}, {1, 0, 1}};
  Vec3 eV[2] = {box.min(), box.max()};
  for ( auto& e : boxVerticesIdx ) {
    m_eVertices.push_back( {Vec3( eV[e[0]][0], eV[e[1]][1], eV[e[2]][2] ), eColor, Vec2(), Vec3()} );
  }
  for ( size_t i = 0; i < boxVerticesIdx.size(); i++ ) { m_eIndices.push_back( (uint32_t)i ); }

  computeVertexNormals();
  // for ( auto& e : m_eVertices ) {
  //   printf( " norm = %f %f %f \n", e.normal_[0], e.normal_[1], e.normal_[2] );
  //   fflush( stdout );
  // }
}

void Mesh::load() {
  if ( !m_bLoad ) {
    glGenVertexArrays( 1, &m_uiVAO );
    glGenBuffers( 1, &m_uiVBO );
    glGenBuffers( 1, &m_uiEBO );
    glBindVertexArray( m_uiVAO );
    glBindBuffer( GL_ARRAY_BUFFER, m_uiVBO );
    glBufferData( GL_ARRAY_BUFFER, m_eVertices.size() * sizeof( Vertex ), &m_eVertices[0], GL_STATIC_DRAW );
    glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, m_uiEBO );
    glBufferData( GL_ELEMENT_ARRAY_BUFFER, m_eIndices.size() * sizeof( GLuint ), &m_eIndices[0], GL_STATIC_DRAW );
    glEnableVertexAttribArray( 0 );
    glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, sizeof( Vertex ), (GLvoid*)0 );
    glEnableVertexAttribArray( 1 );
    glVertexAttribPointer( 1, 4, GL_FLOAT, GL_FALSE, sizeof( Vertex ), (GLvoid*)offsetof( Vertex, color_ ) );
    glEnableVertexAttribArray( 2 );
    glVertexAttribPointer( 2, 2, GL_FLOAT, GL_FALSE, sizeof( Vertex ), (GLvoid*)offsetof( Vertex, texCoords_ ) );
    glEnableVertexAttribArray( 3 );
    glVertexAttribPointer( 3, 3, GL_FLOAT, GL_FALSE, sizeof( Vertex ), (GLvoid*)offsetof( Vertex, normal_ ) );
    glBindVertexArray( 0 );
    for ( auto& tex : m_eTexture ) {
      glGenTextures( 1, &( tex.id_ ) );
      glBindTexture( GL_TEXTURE_2D, tex.id_ );
      glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB, tex.width_, tex.height_, 0, GL_RGB, GL_UNSIGNED_BYTE, tex.data_.data() );
      glGenerateMipmap( GL_TEXTURE_2D );
      glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
      glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
      glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
      glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
      glBindTexture( GL_TEXTURE_2D, 0 );
    }
    m_bLoad = true;
  }
}

void Mesh::draw( Program& program, bool lighting ) {
  float hasTexture = m_eTexture.size() > 0;
  program.setUniform( "hasTexture", static_cast<float>( hasTexture ) );
  if ( program.getName().compare( 0, 4, "Fill" ) == 0 ) {
    Vec3  materialAmbient( 0.4, 0.4, 0.4 );
    Vec3  materialDiffuse( 0.6, 0.6, 0.6 );
    Vec3  lightColor( 1, 1, 1 );
    Vec3  lightDirection( 1, 1, 1 );
    float lightingFloat = (float)lighting;
    program.setUniform( "lighting", lightingFloat );
    program.setUniform( "materialAmbient", materialAmbient );
    program.setUniform( "materialDiffuse", materialDiffuse );
    program.setUniform( "lightColor", lightColor );
    program.setUniform( "lightDirection", lightDirection );
  }
  for ( GLuint i = 0; i < m_eTexture.size(); i++ ) {
    glActiveTexture( GL_TEXTURE0 + i );
    if ( m_eTexture[i].type_ == "texture_diffuse" ) { glBindTexture( GL_TEXTURE_2D, m_eTexture[i].id_ ); }
  }
  glBindVertexArray( m_uiVAO );
  glDrawElements( GL_TRIANGLES, (GLsizei)m_eIndices.size(), GL_UNSIGNED_INT, 0 );
  glBindVertexArray( 0 );
  for ( GLuint i = 0; i < m_eTexture.size(); i++ ) {
    glActiveTexture( GL_TEXTURE0 + i );
    glBindTexture( GL_TEXTURE_2D, 0 );
  }
}

void Mesh::unload() {
  if ( m_bLoad ) {
    for ( auto& texture : m_eTexture ) { glDeleteTextures( 1, &( texture.id_ ) ); }
    glDeleteBuffers( 1, &m_uiVBO );
    glDeleteBuffers( 1, &m_uiEBO );
    glDeleteBuffers( 1, &m_uiVAO );
    glDeleteVertexArrays( 1, &m_uiVAO );
    m_bLoad = false;
  }
}

void Mesh::scale( Vec3 center, float scale ) {
  for ( auto& point : m_eVertices ) { point.position_ = ( point.position_ - center ) * scale; }
}

void Mesh::center( Vec3 center ) {
  for ( auto& ePoint : m_eVertices ) { ePoint.position_ += center; }
}

void Mesh::recomputeBoundingBox() {
  m_eBox = Box();
  for ( auto& point : m_eVertices ) { m_eBox.update( point.position_ ); }
}

inline void Mesh::computeFaceNormals( bool normalize ) {
  m_eFaceNormals.resize( getNumberOfFaces() );
  for ( size_t i = 0; i < getNumberOfFaces(); i++ ) {
    m_eFaceNormals[i] =
        glm::cross( m_eVertices[m_eIndices[i * 3 + 1]].position_ - m_eVertices[m_eIndices[i * 3]].position_,
                    m_eVertices[m_eIndices[i * 3 + 2]].position_ - m_eVertices[m_eIndices[i * 3]].position_ );
    if ( normalize ) { normalizeNormal( m_eFaceNormals[i] ); }
  }
}

void Mesh::computeVertexNormals( bool normalize, bool noSeams ) {
  if ( m_eFaceNormals.size() == 0 ) { computeFaceNormals( false ); }
  if ( !noSeams ) {
    for ( size_t t = 0; t < getNumberOfFaces(); t++ ) {
      for ( size_t i = 0; i < 3; i++ ) { m_eVertices[m_eIndices[t * 3 + i]].normal_ = m_eFaceNormals[t]; }
    }
  } else {
    auto cmp = []( const Vec3& a, const Vec3& b ) {
      if ( a.x != b.x ) { return a.x < b.x; }
      if ( a.y != b.y ) { return a.y < b.y; }
      return a.z < b.z;
    };
    typedef std::map<int, Vec3>                                                             TmpVertNormals;
    typedef std::map<Vec3, TmpVertNormals, std::function<bool( const Vec3&, const Vec3& )>> TmpPosNormals;
    TmpPosNormals                                                                           tmpNormals( cmp );
    // First pass
    for ( size_t t = 0; t < getNumberOfFaces(); t++ ) {
      const auto& faceNormal = m_eFaceNormals[t];
      for ( size_t i = 0; i < 3; i++ ) {
        const GLuint idx     = m_eIndices[t * 3 + i];
        const Vec3   pos     = m_eVertices[idx].position_;
        auto         posIter = tmpNormals.find( pos );
        bool         found   = false;
        if ( posIter != tmpNormals.end() ) {
          auto vertIter = posIter->second.find( idx );
          if ( vertIter != posIter->second.end() ) {
            vertIter->second = vertIter->second + faceNormal;
            found            = true;
          }
        }
        if ( !found ) { tmpNormals[pos][idx] = faceNormal; }
      }
    }
    // Second pass
    for ( auto p = tmpNormals.begin(); p != tmpNormals.end(); p++ ) {
      glm::vec3 normal( 0.0f, 0.0f, 0.0f );
      for ( auto v = p->second.begin(); v != p->second.end(); v++ ) { normal += v->second; }
      if ( normalize ) { normalizeNormal( normal ); }
      for ( auto v = p->second.begin(); v != p->second.end(); v++ ) { m_eVertices[v->first].normal_ = normal; }
    }
  }
  m_eFaceNormals.clear();
}

ObjectMesh::ObjectMesh() {}
ObjectMesh::~ObjectMesh() { m_eMeshes.clear(); }

void ObjectMesh::load() {
  if ( !m_bLoad ) {
    for ( auto& mesh : m_eMeshes ) { mesh.load(); }
    m_bLoad = true;
  }
}

void ObjectMesh::unload() {
  if ( m_bLoad ) {
    for ( auto& mesh : m_eMeshes ) { mesh.unload(); }
    m_bLoad = false;
  }
}

void ObjectMesh::createBox( Box& eBox, Color4& eColor ) {
  if ( !m_bLoad ) {
    m_eMeshes.resize( 1 );
    m_eMeshes[0].createBox( eBox, eColor );
    load();
  }
}

void ObjectMesh::draw( bool lighting ) {
  // glEnable( GL_CULL_FACE );
  // glDisable( GL_CULL_FACE );
  switch ( m_iProgramIndex ) {
    case 0:
    case 1: glPolygonMode( GL_FRONT, GL_FILL ); break;
    case 2: glPolygonMode( GL_FRONT_AND_BACK, GL_LINE ); break;
    case 3: glPolygonMode( GL_FRONT_AND_BACK, GL_POINT ); break;
  }
  for ( auto& mesh : m_eMeshes ) { mesh.draw( m_ePrograms[m_iProgramIndex], lighting ); }
  glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
}

void ObjectMesh::computeVertexNormals() {
  for ( auto& mesh : m_eMeshes ) { mesh.computeVertexNormals(); }
}

void ObjectMesh::loadProgram() {
  if ( m_ePrograms.empty() ) {
    m_ePrograms.resize( 4 );
    m_ePrograms[0].create( "Fill", g_pMeshVertexShader, g_pMeshFragmentShader );
    m_ePrograms[1].create( "Fill+Edge", g_pMeshVertexShader, g_pWireframeGeometryShader, g_pWireframeFragmentShader );
    m_ePrograms[2].create( "Wireframe", g_pMeshVertexShader, g_pMeshFragmentShader );
    m_ePrograms[3].create( "Point", g_pVertexVertexShader, g_pMeshFragmentShader );
  }
}

bool ObjectMesh::read( std::string path, int32_t framesIndex ) {
  auto ext = getExtension( path );
  if ( ext == "obj" ) return readObj( path, framesIndex );
  if ( ext == "ply" ) return readPly( path, framesIndex );
  return false;
}

bool ObjectMesh::readObj( std::string path, int32_t framesIndex ) {
  m_iNumVertices = 0;
  m_iNumFaces    = 0;
  m_iNumTextures = 0;
  m_eFilename    = createFilename( path, framesIndex );
  m_eDirectory   = getDirectory( m_eFilename );
  tinyobj::ObjReaderConfig readerConfig;
  tinyobj::ObjReader       reader;
  readerConfig.mtl_search_path = m_eDirectory;
  if ( !reader.ParseFromFile( m_eFilename, readerConfig ) ) {
    if ( !reader.Error().empty() ) { std::cerr << "TinyObjReader: " << reader.Error(); }
    exit( 1 );
  }
  if ( !reader.Warning().empty() ) { std::cout << "TinyObjReader: " << reader.Warning(); }
  auto& attrib    = reader.GetAttrib();
  auto& shapes    = reader.GetShapes();
  auto& materials = reader.GetMaterials();
  // PrintInfo( attrib, shapes, materials );
  m_eMeshes.resize( shapes.size() );
  bool bUVCoordinates = false;
  for ( size_t s = 0; s < shapes.size(); s++ ) {  // Loop over shapes
    auto&  vertices    = m_eMeshes[s].getVertices();
    auto&  indices     = m_eMeshes[s].getIndices();
    size_t numVertices = 0;
    for ( auto& f : shapes[s].mesh.num_face_vertices ) { numVertices += f; }
    m_iNumVertices += (int)numVertices;
    m_iNumFaces += (int)numVertices;
    vertices.resize( numVertices );
    size_t index     = 0;
    bool   bUVSupOne = false;
    for ( size_t f = 0; f < shapes[s].mesh.num_face_vertices.size(); f++ ) {          // Loop over faces
      for ( size_t v = 0; v < size_t( shapes[s].mesh.num_face_vertices[f] ); v++ ) {  // Loop over vertices
        auto& idx        = shapes[s].mesh.indices[index];
        auto& vertex     = vertices[index];
        vertex.position_ = Vec3( attrib.vertices[3 * size_t( idx.vertex_index ) + 0],
                                 attrib.vertices[3 * size_t( idx.vertex_index ) + 1],
                                 attrib.vertices[3 * size_t( idx.vertex_index ) + 2] );
        if ( idx.texcoord_index >= 0 ) {
          vertex.color_     = Vec4( 0.0f, 0.0f, 0.0f, 0.0f );
          vertex.texCoords_ = Vec2( attrib.texcoords[2 * size_t( idx.texcoord_index ) + 0],
                                    attrib.texcoords[2 * size_t( idx.texcoord_index ) + 1] );
          bUVCoordinates    = true;
          if ( vertex.texCoords_[0] > 1.0f && vertex.texCoords_[1] > 1.f ) { bUVSupOne = true; }
        } else {
          vertex.texCoords_ = Vec2( 0.0f, 0.0f );
          vertex.color_     = Vec4( attrib.colors[3 * size_t( idx.vertex_index ) + 0],
                                attrib.colors[3 * size_t( idx.vertex_index ) + 1],
                                attrib.colors[3 * size_t( idx.vertex_index ) + 2], 1.f );
        }
        indices.push_back( (GLuint)index );
        index++;
      }
    }
    m_eMeshes[s].setUseColorPerVertex( !bUVCoordinates );
    int tid = shapes[s].mesh.material_ids[0] >= 0
                  ? shapes[s].mesh.material_ids[0]
                  : shapes.size() == 1 && materials.size() == 1 && shapes[s].mesh.material_ids[0] == -1 ? 0 : -1;
    if ( bUVCoordinates && tid == -1 ) { tid = 0; }
    if ( tid >= 0 ) {
      std::string diffuse_texname = materials.size() > 0 ? materials[tid].diffuse_texname : std::string();
      if ( bUVCoordinates && diffuse_texname.empty() ) {
        diffuse_texname = std::string( getBasename( getRemoveExtension( m_eFilename ) ) + ".png" );
      }
      readTextures( diffuse_texname, "texture_diffuse", m_eMeshes[s].getTextures() );
      if ( bUVSupOne ) {
        int  width  = m_eMeshes[s].getTextures().back().width_;
        int  height = m_eMeshes[s].getTextures().back().height_;
        Vec2 maxUV( width - 1, height - 1 );
        for ( auto& v : vertices ) {
          if ( maxUV[0] < v.texCoords_[0] ) { maxUV[0] = v.texCoords_[0]; }
          if ( maxUV[1] < v.texCoords_[1] ) { maxUV[1] = v.texCoords_[1]; }
        }
        maxUV[0] = (float)( ( ( (int)maxUV[0] / width ) + 1 ) * width - 1 );
        maxUV[1] = (float)( ( ( (int)maxUV[1] / height ) + 1 ) * height - 1 );
        for ( auto& v : vertices ) { v.texCoords_ /= maxUV; }
      }
      m_iNumTextures++;
    }
    m_eMeshes[s].recomputeBoundingBox();
    m_eBox.update( m_eMeshes[s].getBox() );
  }
  computeVertexNormals();
  return true;
}

template <typename T, typename D>
void templateConvert(std::shared_ptr<tinyply::PlyData> src,
                     const uint8_t numSrc,
                     std::vector<T> &dst,
                     const uint8_t numDst)
{
  const size_t numBytes = src->buffer.size_bytes();
  std::vector<D> data;
  data.resize(src->count * numSrc);
  std::memcpy(data.data(), src->buffer.get(), numBytes);
  if (numSrc == numDst)
  {
    dst.assign(data.begin(), data.end());
  }
  else
  {
    dst.resize(src->count * numDst);
    for (size_t i = 0; i < src->count; i++)
      for (size_t c = 0; c < numDst; c++)
        dst[i * numDst + c] = (T)data[i * numSrc + c];
  }
}

template<typename T>
void set(std::shared_ptr<tinyply::PlyData> src,
         const uint8_t numSrc, 
         std::vector<T>& dst,
         const uint8_t numDst, 
         std::string name)
{
  if (src)
  {
    switch (src->t)
    {
    case tinyply::Type::INT8:
      templateConvert<T,int8_t>(src, numSrc, dst, numDst);
      break;
    case tinyply::Type::UINT8:
      templateConvert<T,uint8_t>(src, numSrc, dst, numDst);
      break;
    case tinyply::Type::INT16:
      templateConvert<T,int16_t>(src, numSrc, dst, numDst);
      break;
    case tinyply::Type::UINT16:
      templateConvert<T,uint16_t>(src, numSrc, dst, numDst);
      break;
    case tinyply::Type::INT32:
      templateConvert<T,int32_t>(src, numSrc, dst, numDst);
      break;
    case tinyply::Type::UINT32:
      templateConvert<T,uint32_t>(src, numSrc, dst, numDst);
      break;
    case tinyply::Type::FLOAT32:
      templateConvert<T,float>(src, numSrc, dst, numDst);
      break;
    case tinyply::Type::FLOAT64:
      templateConvert<T,double>(src, numSrc, dst, numDst);
      break;
    default:
      printf("ERROR: PLY type not supported: %s \n", name.c_str());
      fflush(stdout);
      exit(-1);
      break;
    }
  }
}

bool ObjectMesh::readPly( std::string path, int32_t framesIndex ) {
  m_iNumVertices = 0;
  m_iNumFaces    = 0;
  m_iNumTextures = 0;
  m_eFilename    = createFilename( path, framesIndex );
  m_eDirectory   = getDirectory( m_eFilename );

  std::unique_ptr<std::istream> file_stream;
  file_stream.reset( new std::ifstream( m_eFilename.c_str(), std::ios::binary ) );
  if (!file_stream || file_stream->fail()) {
    printf("failed to open: %s \n", m_eFilename.c_str());
    return false;
  }
  tinyply::PlyFile file;
  file.parse_header( *file_stream );
  std::shared_ptr<tinyply::PlyData> _vertices, _normals, _colors, _colorsRGBA, _texcoords, _faces, _tripstrip, _uvfaces, _nrmfaces;

  // The header information can be used to programmatically extract properties on elements
  // known to exist in the header prior to reading the data. For brevity of this sample, properties
  // like vertex position are hard-coded:
  try {
    _vertices = file.request_properties_from_element( "vertex", {"x", "y", "z"} );
  } catch ( const std::exception& e ) { std::cerr << "skipping: " << e.what() << std::endl; }
  try {
    _normals = file.request_properties_from_element( "vertex", {"nx", "ny", "nz"} );
  } catch ( const std::exception& ) {}
  try {
    _colors = file.request_properties_from_element( "vertex", {"red", "green", "blue"} );
  } catch ( const std::exception& ) {}
  try {
    _colors = file.request_properties_from_element( "vertex", {"r", "g", "b"} );
  } catch ( const std::exception& ) {}
  try {
    _colorsRGBA = file.request_properties_from_element( "vertex", {"red", "green", "blue", "alpha"} );
  } catch ( const std::exception& ) {}
  try {
    _colorsRGBA = file.request_properties_from_element( "vertex", {"r", "g", "b", "a"} );
  } catch ( const std::exception& ) {}
  try {
    _texcoords = file.request_properties_from_element( "vertex", {"texture_u", "texture_v"} );
  } catch ( const std::exception& ) {}

  // Providing a list size hint (the last argument) is a 2x performance improvement. If you have
  // arbitrary ply files, it is best to leave this 0.
  try {
    _faces = file.request_properties_from_element( "face", {"vertex_indices"}, 3 );
  } catch ( const std::exception& e ) { std::cerr << "skipping: " << e.what() << std::endl; }
  try {
    _uvfaces = file.request_properties_from_element( "face", {"texcoord"}, 6 );
  } catch ( const std::exception& ) {}

  file.read(*file_stream);

  std::vector<float>   coords;
  std::vector<float>   colours;
  std::vector<float>   uvcoords;     
  std::vector<int>     trianglesuv; 
  std::vector<int32_t> triangles;
  set( _vertices, 3, coords, 3, "vertices" );
  set( _texcoords, 2, uvcoords, 2, "uvcoords" );
  set( _colors, 3, colours, 3, "colors" );
  set( _colorsRGBA, 4, colours, 3, "colorsRGBA" );
  set( _faces, 3, triangles, 3, "triangles" );
  if ( _uvfaces ) {
    const auto triCount = _uvfaces->count;
    trianglesuv.resize(triCount * 3);
    uvcoords.resize(triCount * 6);
    set( _uvfaces, 6, uvcoords, 6, "uvfaces" );
    for (size_t i = 0; i < triCount * 3; i++)
      trianglesuv[i] = i;
  } else {
    trianglesuv = triangles;
  }

  m_eMeshes.resize( 1 );
  bool bUVCoordinates = false;
  auto&  vertices    = m_eMeshes[0].getVertices();
  auto&  indices     = m_eMeshes[0].getIndices();
  size_t numVertices = _faces->count *3;
  size_t numFaces = _faces->count*3;
  m_iNumVertices += (int)numVertices;
  m_iNumFaces += (int)numFaces;
  vertices.resize( numVertices );
  size_t index     = 0;
  bool   bUVSupOne = false;
  if ( uvcoords.size() > 0 ) {
    for ( size_t f = 0; f < numFaces; f++ ) {
      const auto v      = triangles[f];
      auto&      vertex = vertices[f];
      vertex.position_  = Vec3( coords[3 * v + 0], coords[3 * v + 1], coords[3 * v + 2] );
      vertex.color_     = Vec4( 0.0f, 0.0f, 0.0f, 0.0f );
      vertex.texCoords_ = Vec2( uvcoords[2 * trianglesuv[f] + 0], uvcoords[2 * trianglesuv[f] + 1] );
      if ( !bUVSupOne && vertex.texCoords_[0] > 1.0f && vertex.texCoords_[1] > 1.f ) { bUVSupOne = true; }
      indices.push_back( (GLuint)f );
    }
    bUVCoordinates = true;
  } else {
    for ( size_t f = 0; f < numFaces; f++ ) {
      const auto v      = triangles[f];
      auto&      vertex = vertices[f];
      vertex.position_  = Vec3( coords[3 * v + 0], coords[3 * v + 1], coords[3 * v + 2] );
      vertex.texCoords_ = Vec2( 0.0f, 0.0f );
      vertex.color_     = Vec4( colours[3 * v + 0], colours[3 * v + 1], colours[3 * v + 2], 1.f );
      indices.push_back( (GLuint)f );
    }
  }
  m_eMeshes[0].setUseColorPerVertex( !bUVCoordinates );
  int tid = 0; 
  if ( tid >= 0 ) {
    std::string diffuse_texname = std::string();
    if ( bUVCoordinates && diffuse_texname.empty() ) {
      diffuse_texname = std::string( getBasename( getRemoveExtension( m_eFilename ) ) + ".png" );
    }
    readTextures( diffuse_texname, "texture_diffuse", m_eMeshes[0].getTextures() );
    if ( bUVSupOne ) {
      int  width  = m_eMeshes[0].getTextures().back().width_;
      int  height = m_eMeshes[0].getTextures().back().height_;
      Vec2 maxUV( width - 1, height - 1 );
      for ( auto& v : vertices ) {
        if ( maxUV[0] < v.texCoords_[0] ) { maxUV[0] = v.texCoords_[0]; }
        if ( maxUV[1] < v.texCoords_[1] ) { maxUV[1] = v.texCoords_[1]; }
      }
      maxUV[0] = (float)( ( ( (int)maxUV[0] / width ) + 1 ) * width - 1 );
      maxUV[1] = (float)( ( ( (int)maxUV[1] / height ) + 1 ) * height - 1 );
      for ( auto& v : vertices ) { v.texCoords_ /= maxUV; }
    }
    m_iNumTextures++;
  }
  m_eMeshes[0].recomputeBoundingBox();
  m_eBox.update( m_eMeshes[0].getBox() );
  computeVertexNormals();
  return true;
}

void ObjectMesh::readTextures( const std::string& name, std::string type, std::vector<Texture>& textures ) {
  bool alreadyLoad = false;
  for ( GLuint j = 0; j < textures.size(); j++ ) {
    if ( textures[j].path_ == name ) {
      textures.push_back( textures[j] );
      alreadyLoad = true;
      break;
    }
  }
  if ( !alreadyLoad ) {
    int      comp;
    Texture  texture;
    auto     filename = m_eDirectory + getSeparator() + name;
    uint8_t* image    = stbi_load( filename.c_str(), &texture.width_, &texture.height_, &comp, STBI_rgb );
    texture.data_.resize( 3 * texture.width_ * texture.height_ );
    std::memcpy( texture.data_.data(), image, 3 * texture.width_ * texture.height_ * sizeof( uint8_t ) );
    stbi_image_free( image );
    texture.type_ = type;
    texture.path_ = name;
    textures.push_back( texture );
  }
}

void ObjectMesh::recomputeBoundingBox() {
  m_eBox = Box();
  for ( auto& mesh : m_eMeshes ) {
    mesh.recomputeBoundingBox();
    m_eBox.update( mesh.getBox() );
  }
}

void ObjectMesh::center( Box box, float fBoxSize ) {
  Vec3 center = fBoxSize / 2.f - ( box.min() + ( box.max() - box.min() ) / 2.f );
  for ( auto& mesh : m_eMeshes ) { mesh.center( center ); }
}

void ObjectMesh::scale( Box box, float fBoxSize ) {
  if ( fBoxSize != 0.f ) {
    float fScale = fBoxSize / box.getMaxSize();
    for ( auto& mesh : m_eMeshes ) { mesh.scale( box.min(), fScale ); }
  }
}

std::string ObjectMesh::getInformation() {
  if ( m_iNumTextures > 0 ) {
    return stringFormat( " Points   = %9d Faces   = %8d with %d textures image%c", m_iNumVertices, m_iNumFaces,
                         m_iNumTextures, m_iNumTextures > 1 ? 's' : ' ' );
  } else {
    return stringFormat( " Points   = %9d Faces   = %8d with color by vertices", m_iNumVertices, m_iNumFaces );
  }
}
