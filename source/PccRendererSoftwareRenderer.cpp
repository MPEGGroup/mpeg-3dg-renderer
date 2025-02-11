//Copyright(c) 2016 - 2025, InterDigital
//All rights reserved.
//See LICENSE under the root folder.

#include "PccRendererSoftwareRenderer.h"
#include "PccRendererImage.h"
#include "PccRendererObject.h"
#include "PccRendererObjectMesh.h"
#include "PccRendererObjectPointcloud.h"

struct ShaderMesh {
  ShaderMesh( Mesh& eMesh, Mat4& eMatMVP, Mat4& eMatNrm, bool bLighting ) :
      m_eMesh( eMesh ), m_eMatMVP( eMatMVP ), m_eMatNrm( eMatNrm ), m_bLighting( bLighting ) {}
  virtual Vec4 vertex( const int iFace, const int iVertex ) = 0;
  virtual Vec3 fragment( const Vec3 eCoord )                = 0;
  Mesh&        m_eMesh;
  Mat4&        m_eMatMVP;
  Mat3         m_eMatNrm;
  Vec3         m_eLightDirection  = Vec3( 1, 1, 1 );
  Vec3         m_eLightColor      = Vec3( 1, 1, 1 );
  Vec3         m_eMaterialAmbient = Vec3( 0.4, 0.4, 0.4 );
  Vec3         m_eMaterialDiffuse = Vec3( 0.6, 0.6, 0.6 );
  bool         m_bLighting        = true;
};

struct ShaderMeshCpv : ShaderMesh {
  Mat3 m_eColor;
  Mat3 m_eNorm;
  ShaderMeshCpv( Mesh& eMesh, Mat4& eMatMVP, Mat4& eMatNrm, bool bLighting ) :
      ShaderMesh( eMesh, eMatMVP, eMatNrm, bLighting ) {}
  Vec4 vertex( const int iFace, const int iVertex ) {
    const auto& vertex = m_eMesh.getVertex( m_eMesh.getIndice( iFace * 3 ) + iVertex );
    m_eColor           = glm::column( m_eColor, iVertex, vertex.color_ );
    if ( m_bLighting ) { m_eNorm = glm::column( m_eNorm, iVertex, glm::vec3( m_eMatNrm * vertex.normal_ ) ); }
    return m_eMatMVP * Vec4( vertex.position_, 1. );
  }
  Vec3 fragment( const Vec3 eCoord ) {
    Vec3 rgb = m_eColor * eCoord;
    if ( m_bLighting ) {
      float diff =
          std::max( glm::dot( glm::normalize( m_eNorm * eCoord ), glm::normalize( m_eLightDirection ) ), 0.0f );
      Vec3 ambient = rgb * m_eMaterialAmbient;
      Vec3 diffuse = rgb * m_eMaterialDiffuse * m_eLightColor * diff;
      rgb          = glm::clamp( ambient + diffuse, 0.0f, 1.0f );
    }
    return rgb;
  }
};

struct ShaderMeshMap : ShaderMesh {
  Mat3x2 m_eUV;
  Mat3   m_eNorm;
  ShaderMeshMap( Mesh& eMesh, Mat4& eMatMVP, Mat4& eMatNrm, bool bLighting ) :
      ShaderMesh( eMesh, eMatMVP, eMatNrm, bLighting ) {}
  Vec4 vertex( const int iFace, const int iVertex ) {
    const auto& vertex = m_eMesh.getVertex( m_eMesh.getIndice( iFace * 3 ) + iVertex );
    m_eUV              = glm::column( m_eUV, iVertex, vertex.texCoords_ );
    if ( m_bLighting ) { m_eNorm = glm::column( m_eNorm, iVertex, glm::vec3( m_eMatNrm * vertex.normal_ ) ); }
    return m_eMatMVP * Vec4( vertex.position_, 1. );
  }
  Vec3 fragment( const Vec3 eCoord ) {
    Vec3 rgb = m_eMesh.getTexture( 0 ).texture2DBilinear( m_eUV * eCoord ) / 256.f;
    if ( m_bLighting ) {
      float diff =
          std::max( glm::dot( glm::normalize( m_eNorm * eCoord ), glm::normalize( m_eLightDirection ) ), 0.0f );
      Vec3 ambient = rgb * m_eMaterialAmbient;
      Vec3 diffuse = rgb * m_eMaterialDiffuse * m_eLightColor * diff;
      rgb          = glm::clamp( ambient + diffuse, 0.0f, 1.0f );
    }
    return rgb;
  }
};

SoftwareRenderer::SoftwareRenderer( Image& image, Mat4& eMatMod, Mat4& eMatPro, bool bLighting ) :
    m_eMVP( eMatPro * eMatMod ),
    m_eMatNrm( glm::transpose( glm::inverse( eMatMod ) ) ),
    m_eImage( image ),
    m_bLighting( bLighting ) {
  m_eViewport = Vec4( 0, 0, m_eImage.getWidth(), m_eImage.getHeight() );
  m_fDepth.resize( m_eImage.getWidth() * m_eImage.getHeight(), std::numeric_limits<float>::max() );
}

SoftwareRenderer::~SoftwareRenderer() { m_fDepth.clear(); }

void SoftwareRenderer::drawObject( Object& eObject ) {
  switch ( eObject.getType() ) {
    case ObjectType::MESH: drawObject( *static_cast<ObjectMesh*>( &eObject ) ); break;
    case ObjectType::POINTCLOUD: drawObject( *static_cast<ObjectPointcloud*>( &eObject ) ); break;
  }
}

void SoftwareRenderer::drawObject( ObjectMesh& eObject ) {
  for ( auto& eMesh : eObject.getMeshes() ) {
    if ( eMesh.getUseColorPerVertex() ) {
      ShaderMeshCpv shader( eMesh, m_eMVP, m_eMatNrm, m_bLighting );
      drawMesh( eMesh, shader );
    } else {
      ShaderMeshMap shader( eMesh, m_eMVP, m_eMatNrm, m_bLighting );
      drawMesh( eMesh, shader );
    }
  }
}

void SoftwareRenderer::drawMesh( Mesh& eMesh, ShaderMesh& shader ) {
  ScreenArea area( m_eImage.getSize() );
  // printf("drawMesh = vertex = %zu face = %zu \n",eMesh.getNumberOfVertices(),eMesh.getNumberOfFaces() );
  // fflush(stdout);
  for ( int f = 0; f < (int)eMesh.getNumberOfFaces(); ++f ) {
    const Mat3x4 proj( shader.vertex( f, 0 ), shader.vertex( f, 1 ), shader.vertex( f, 2 ) );
    const Mat3x2 screen = projToScreen( proj );
    const Vec2   A( screen[1].y - screen[2].y, screen[2].x - screen[1].x );
    const Vec2   B( screen[2].y - screen[0].y, screen[0].x - screen[2].x );
    const double invDet = 1. / glm::determinant( Mat2( A, B ) );
    area.set( screen );
    // printf(" f = %d => %9d %9d x %9d %9d / image size = %d %d \n",f, area.min().x, area.max().x,  area.min().y,
    // area.max().y,
    //   m_eImage.getWidth(),m_eImage.getHeight());
    // fflush(stdout);
    for ( int x = area.min().x; x <= area.max().x; x++ ) {
      for ( int y = area.min().y; y <= area.max().y; y++ ) {
        const double a = glm::dot( A, Vec2( x, y ) - screen[2] ) * invDet;
        const double b = glm::dot( B, Vec2( x, y ) - screen[2] ) * invDet;
        const Vec3   coord( a, b, 1 - a - b );
        if ( coord[0] >= 0 && coord[1] >= 0 && coord[2] >= 0 ) {
          float d = glm::dot( glm::row( proj, 2 ), coord );
          if ( !std::isnan( d ) && d < m_fDepth[x + y * m_eImage.getWidth()] ) {
            m_fDepth[x + y * m_eImage.getWidth()] = d;
            m_eImage.set( x, y, shader.fragment( coord ) );
          }
        }
      }
    }
  }
}

void SoftwareRenderer::drawObject( ObjectPointcloud& eObject ) {
  ScreenArea area( m_eImage.getSize() );
  for ( int i = 0; i < (int)eObject.getNumPoints(); ++i ) {
    const Vec4 proj   = m_eMVP * Vec4( eObject.getPoints( i ), 1.f );
    const Vec2 screen = projToScreen( proj );
    area.set( screen, 500.f / proj[3] );
    for ( int x = area.min().x; x <= area.max().x; x++ ) {
      for ( int y = area.min().y; y <= area.max().y; y++ ) {
        if ( !std::isnan( proj[2] ) && proj[2] < m_fDepth[x + y * m_eImage.getWidth()] ) {
          m_fDepth[x + y * m_eImage.getWidth()] = (float)proj[2];
          m_eImage.set( x, y, eObject.getColors3( i ) );
        }
      }
    }
  }
}

void SoftwareRenderer::drawBackground( Color3 eColor ) { m_eImage.fill( eColor ); }

void SoftwareRenderer::drawFloor( Box eFloor, Color4& eColor ) {
  Mesh          eMesh( eFloor, eColor );
  ShaderMeshCpv shader( eMesh, m_eMVP, m_eMatNrm, m_bLighting );
  drawMesh( eMesh, shader );
}