//Copyright(c) 2016 - 2025, InterDigital
//All rights reserved.
//See LICENSE under the root folder.


#include "PccRendererPrimitive.h"
#include "PccRendererProgram.h"
#include "PccRendererObject.h"

// clang-format off

// Render To Texture Shader
static const std::string  g_pRenderToTexureVertexShader = SHADER(  
  layout(location = 0) in vec3 vertexPosition_modelspace;  
  out vec2 UV;
  void main(){
    gl_Position =  vec4(vertexPosition_modelspace,1);
    UV = (vertexPosition_modelspace.xy + vec2( 1, 1 ) ) *.5;
  }
);

static const std::string g_pRenderToTexureFragmentShader = SHADER(
  in vec2 UV; 
  uniform sampler2D renderedTexture; 
  out vec4      color;      
  void main() {    
    color = vec4( texture( renderedTexture, UV ).xyz, 1.0 );       
  } 
); 

// Vertex shader
static const std::string g_pColorVertexShader = 
  SHADER(
    layout( location = 0 ) in vec3 vPosition; 
    layout( location = 1 ) in vec3 vColor; 
    out vec3 color; 
    void main() {
      color       = vColor;
      gl_Position = vec4( vPosition, 1.0 );
    } 
  );

static const std::string g_pColorModVertexShader =
  SHADER( 
    layout( location = 0 ) in vec3 vPosition; 
    layout( location = 1 ) in vec3 vColor;
    uniform mat4 ProjMat;
    uniform mat4 ModMat;
    out vec3 color;
    void main() {
      color = vColor;
      gl_Position = ProjMat * ModMat * vec4( vPosition, 1.0 );
    }
  );

static const std::string g_pColor4ModVertexShader =
  SHADER( 
    layout( location = 0 ) in vec3 vPosition;
    layout( location = 1 ) in vec4 vColor; 
    uniform mat4 ProjMat;
    uniform mat4 ModMat;
    out vec4 color;
    void main() {
      color = vColor;
      gl_Position = ProjMat * ModMat * vec4( vPosition, 1.0 );
    } 
  );

static const std::string g_pColorFragmentShader =
  SHADER( 
    in vec3 color; 
    out vec4 frag_color;
    void main() { frag_color = vec4( color, 1.0 ); } 
  );

static const std::string g_pColor4FragmentShader =
  SHADER( 
    in vec4 color; 
    out vec4 frag_color; 
    void main() { frag_color = color; } 
  );

// Default shader
static const std::string g_pDefaultVertexShader =
  SHADER(
    layout( location = 0 ) in vec3 vertex_position; 
    uniform mat4 ProjMat; 
    uniform mat4 ModMat;
    void main() { gl_Position = ProjMat * ModMat * vec4( vertex_position, 1.0 ); } 
  );

static const std::string g_pDefaultFragmentShader =
  SHADER( 
    uniform vec4 color; 
    out vec4 frag_colour; 
    void main() { frag_colour = color; }
  );
// clang-format on

Primitive::Primitive() {}
Primitive::~Primitive() { initialize(); }
void Primitive::initialize() {
  if ( m_iNumber != -1 ) {
    glDeleteBuffers( 1, &m_uiVBO );
    glDeleteBuffers( 1, &m_uiCBO );
    glDeleteVertexArrays( 1, &m_uiVAO );
    m_iNumber = -1;
  }
}

void Primitive::set( std::vector<Vec3>& points ) {
  initialize();
  glGenBuffers( 1, &m_uiVBO );
  glBindBuffer( GL_ARRAY_BUFFER, m_uiVBO );
  glBufferData( GL_ARRAY_BUFFER, points.size() * sizeof( Vec3 ), points.data(), GL_STATIC_DRAW );
  glGenVertexArrays( 1, &m_uiVAO );
  glBindVertexArray( m_uiVAO );
  glBindBuffer( GL_ARRAY_BUFFER, m_uiVBO );
  glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, 0, NULL );
  glEnableVertexAttribArray( 0 );
  m_iNumber = (int)points.size();
}

void Primitive::set( std::vector<Vec3>& points, std::vector<Color3>& colors ) {
  initialize();
  glGenBuffers( 1, &m_uiVBO );
  glBindBuffer( GL_ARRAY_BUFFER, m_uiVBO );
  glBufferData( GL_ARRAY_BUFFER, points.size() * sizeof( Vec3 ), points.data(), GL_STATIC_DRAW );
  glGenBuffers( 1, &m_uiCBO );
  glBindBuffer( GL_ARRAY_BUFFER, m_uiCBO );
  glBufferData( GL_ARRAY_BUFFER, colors.size() * sizeof( Color3 ), colors.data(), GL_STATIC_DRAW );
  glGenVertexArrays( 1, &m_uiVAO );
  glBindVertexArray( m_uiVAO );
  glBindBuffer( GL_ARRAY_BUFFER, m_uiVBO );
  glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, 0, NULL );
  glBindBuffer( GL_ARRAY_BUFFER, m_uiCBO );
  glVertexAttribPointer( 1, 3, GL_FLOAT, GL_FALSE, 0, NULL );
  glEnableVertexAttribArray( 0 );
  glEnableVertexAttribArray( 1 );
  m_iNumber = (int)points.size();
}

void Primitive::set( std::vector<Vec3>& points, std::vector<Color4>& colors ) {
  initialize();
  glGenBuffers( 1, &m_uiVBO );
  glBindBuffer( GL_ARRAY_BUFFER, m_uiVBO );
  glBufferData( GL_ARRAY_BUFFER, points.size() * sizeof( Vec3 ), points.data(), GL_STATIC_DRAW );
  glGenBuffers( 1, &m_uiCBO );
  glBindBuffer( GL_ARRAY_BUFFER, m_uiCBO );
  glBufferData( GL_ARRAY_BUFFER, colors.size() * sizeof( Color4 ), colors.data(), GL_STATIC_DRAW );
  glGenVertexArrays( 1, &m_uiVAO );
  glBindVertexArray( m_uiVAO );
  glBindBuffer( GL_ARRAY_BUFFER, m_uiVBO );
  glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, 0, NULL );
  glBindBuffer( GL_ARRAY_BUFFER, m_uiCBO );
  glVertexAttribPointer( 1, 4, GL_FLOAT, GL_FALSE, 0, NULL );
  glEnableVertexAttribArray( 0 );
  glEnableVertexAttribArray( 1 );
  m_iNumber = (int)points.size();
}

void Background::load( Vec3 a ) { load( a, a ); }

void Background::load( Vec3 a, Vec3 b ) {
  if ( !m_eProgram.isLoad() ) { m_eProgram.create( "Color", g_pColorVertexShader, g_pColorFragmentShader ); }
  std::vector<Vec3>   points = {Vec3( -1, 1, 0 ), Vec3( 1, 1, 0 ), Vec3( 1, -1, 0 ), Vec3( -1, -1, 0 )};
  std::vector<Color3> colors = {a, a, b, b};
  set( points, colors );
}

void Background::draw() {
  glDisable( GL_LIGHTING );
  glDisable( GL_DEPTH_TEST );
  glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
  m_eProgram.use();
  glBindVertexArray( m_uiVAO );
  glDrawArrays( GL_TRIANGLE_FAN, 0, m_iNumber );
  m_eProgram.stop();
  glEnable( GL_LIGHTING );
  glEnable( GL_DEPTH_TEST );
}

void DistanceScale::load() {
  if ( !m_eProgram.isLoad() ) { m_eProgram.create( "Color", g_pColorVertexShader, g_pColorFragmentShader ); }
  std::vector<Vec3>   points;
  std::vector<Color3> colors;
  for ( size_t i = 0; i < 256; i++ ) {
    float y = 1.8f * ( static_cast<float>( i ) / 256.f - 0.5f );
    points.push_back( Vec3( 0.92f, y, 0 ) );
    points.push_back( Vec3( 0.98f, y, 0 ) );
    colors.push_back( g_pColorRgb[i] );
    colors.push_back( g_pColorRgb[i] );
  }
  set( points, colors );
}

void DistanceScale::draw() {
  m_eProgram.use();
  glBindVertexArray( m_uiVAO );
  glDrawArrays( GL_TRIANGLE_STRIP, 0, m_iNumber );
  m_eProgram.stop();
}

void PointList::load( std::vector<Vec3>& pointsSrc, std::vector<Color3>& colorsSrc, std::vector<Vec3>& directions ) {
  if ( !m_eProgram.isLoad() ) { m_eProgram.create( "Color", g_pColor4ModVertexShader, g_pColor4FragmentShader ); }
  std::vector<Vec3>   points;
  std::vector<Color4> colors;
  for ( size_t i = 0; i < pointsSrc.size(); i++ ) {
    points.push_back( directions[i] );
    colors.push_back( Vec4( colorsSrc[i], 0.0 ) );
    points.push_back( pointsSrc[i] );
    colors.push_back( Vec4( colorsSrc[i], 1.0 ) );
    points.push_back( directions[i] );
    colors.push_back( Vec4( colorsSrc[i], 0.0 ) );
  }
  set( points, colors );
}

void PointList::draw( Mat4& eMatMod, Mat4& eMatPro, int iPointSize, bool bDrawLines ) {
  m_eProgram.use();
  m_eProgram.setUniform( "ModMat", eMatMod );
  m_eProgram.setUniform( "ProjMat", eMatPro );
  glBindVertexArray( m_uiVAO );
  glPointSize( (float)iPointSize );
  glDrawArrays( GL_POINTS, 0, m_iNumber );
  if ( bDrawLines ) { glDrawArrays( GL_LINE_STRIP, 0, m_iNumber ); }
  glBindVertexArray( 0 );
  m_eProgram.stop();
}

void Sphere::load( Vec3 c, float fSize ) {
  if ( !m_eProgram.isLoad() ) { m_eProgram.create( "Color", g_pColorModVertexShader, g_pColorFragmentShader ); }
  std::vector<Vec3>   points;
  std::vector<Color3> colors;
  float               s = fSize * 3.f / 4.f, w = (float)M_PI / 180.f;
  for ( int j = 0; j <= 360; j++ ) {
    points.push_back( Vec3( c[0], c[1] + s * std::sin( j * w ), c[2] + s * std::cos( j * w ) ) );
    colors.push_back( Color3( 1.0, 0.0, 0.0 ) );
  }
  for ( int j = 0; j <= 360 + 90; j++ ) {
    points.push_back( Vec3( c[0] + s * std::sin( j * w ), c[1], c[2] + s * std::cos( j * w ) ) );
    colors.push_back( Color3( 0.0, 1.0, 0.0 ) );
  }
  for ( int j = 90; j <= 360 + 90; j++ ) {
    points.push_back( Vec3( c[0] + s * std::sin( j * w ), c[1] + s * std::cos( j * w ), c[2] ) );
    colors.push_back( Color3( 0.0, 0.0, 1.0 ) );
  }
  set( points, colors );
}

void Sphere::draw( Mat4& eMatMod, Mat4& eMatPro ) {
  m_eProgram.use();
  m_eProgram.setUniform( "ModMat", eMatMod );
  m_eProgram.setUniform( "ProjMat", eMatPro );
  glBindVertexArray( m_uiVAO );
  glDrawArrays( GL_LINE_STRIP, 0, m_iNumber );
  glBindVertexArray( 0 );
  m_eProgram.stop();
}

void Cube::load( Vec3 eV0, Vec3 eV1 ) {
  if ( !m_eProgram.isLoad() ) { m_eProgram.create( "Color", g_pDefaultVertexShader, g_pDefaultFragmentShader ); }
  std::vector<Vec3> points;
  points.push_back( eV0 );
  points.push_back( Vec3( eV1[0], eV0[1], eV0[2] ) );
  points.push_back( Vec3( eV0[0], eV1[1], eV0[2] ) );
  points.push_back( Vec3( eV1[0], eV1[1], eV0[2] ) );
  points.push_back( Vec3( eV0[0], eV0[1], eV1[2] ) );
  points.push_back( Vec3( eV1[0], eV0[1], eV1[2] ) );
  points.push_back( Vec3( eV0[0], eV1[1], eV1[2] ) );
  points.push_back( eV1 );
  points.push_back( eV0 );
  points.push_back( Vec3( eV0[0], eV1[1], eV0[2] ) );
  points.push_back( Vec3( eV1[0], eV0[1], eV0[2] ) );
  points.push_back( Vec3( eV1[0], eV1[1], eV0[2] ) );
  points.push_back( Vec3( eV0[0], eV0[1], eV1[2] ) );
  points.push_back( Vec3( eV0[0], eV1[1], eV1[2] ) );
  points.push_back( Vec3( eV1[0], eV0[1], eV1[2] ) );
  points.push_back( eV1 );
  points.push_back( eV0 );
  points.push_back( Vec3( eV0[0], eV0[1], eV1[2] ) );
  points.push_back( Vec3( eV1[0], eV0[1], eV0[2] ) );
  points.push_back( Vec3( eV1[0], eV0[1], eV1[2] ) );
  points.push_back( Vec3( eV0[0], eV1[1], eV0[2] ) );
  points.push_back( Vec3( eV0[0], eV1[1], eV1[2] ) );
  points.push_back( Vec3( eV1[0], eV1[1], eV0[2] ) );
  points.push_back( eV1 );
  set( points );
}

void Cube::draw( Mat4& eMatMod, Mat4& eMatPro, Color4 color ) {
  m_eProgram.use();
  m_eProgram.setUniform( "ModMat", eMatMod );
  m_eProgram.setUniform( "ProjMat", eMatPro );
  m_eProgram.setUniform( "color", color );
  glBindVertexArray( m_uiVAO );
  glDrawArrays( GL_LINES, 0, m_iNumber );
  glBindVertexArray( 0 );
  m_eProgram.stop();
}

void Floor::load( Box eBox, Color4& eColor ) { m_eObjectMesh.createBox( eBox, eColor ); }
void Floor::draw( Mat4& eMatMod, Mat4& eMatPro, bool bLighting ) {
  m_eObjectMesh.loadProgram();
  auto& eProgram = m_eObjectMesh.getProgram();
  eProgram.use();
  eProgram.setUniform( "ModMat", eMatMod );
  eProgram.setUniform( "ProjMat", eMatPro );
  eProgram.setUniform( "forceColor", 0 );
  if ( m_eObjectMesh.getProgramIndex() == 1 ) { eProgram.setUniform( "PointSize", 0 ); }
  if ( m_eObjectMesh.getProgramIndex() == 3 ) { eProgram.setUniform( "PointSize", 1 ); }
  m_eObjectMesh.draw( bLighting );
  eProgram.stop();
}

void Arrow::load( Vec3 A, Vec3 B, float fA, float fB ) {
  if ( !m_eProgram.isLoad() ) { m_eProgram.create( "Color", g_pDefaultVertexShader, g_pDefaultFragmentShader ); }
  std::vector<Vec3> points;
  points.push_back( A );
  points.push_back( B );
  Vec3 X = B - A, Z, Y;
  X *= fA / glm::length( X );
  Vec3  V0( fB * ( -X[1] ) / fA, fB * ( X[0] ) / fA, 0 );
  Vec3  V1( fB * ( -X[2] ) / fA, 0, fB * ( X[0] ) / fA );
  Vec3  V2( 0, fB * ( -X[2] ) / fA, fB * ( X[1] ) / fA );
  float fNormZ0 = glm::dot( V0, V0 ), fNormZ1 = glm::dot( V1, V1 ), fNormZ2 = glm::dot( V2, V2 );
  if ( fNormZ1 > fNormZ2 ) {
    Z = V1;
    Y = fNormZ0 > fNormZ2 ? V0 : V2;
  } else {
    Z = V2;
    Y = fNormZ0 > fNormZ1 ? V0 : V1;
  }
  points.push_back( B );
  points.push_back( B - X + Y - Z );
  points.push_back( B - X + Y + Z );
  points.push_back( B - X - Y + Z );
  points.push_back( B );
  points.push_back( B - X - Y - Z );
  points.push_back( B - X + Y - Z );
  points.push_back( B - X - Y + Z );
  set( points );
}

void Arrow::draw( Mat4& eMatMod, Mat4& eMatPro, Color4 color ) {
  m_eProgram.use();
  m_eProgram.setUniform( "ModMat", eMatMod );
  m_eProgram.setUniform( "ProjMat", eMatPro );
  m_eProgram.setUniform( "color", color );
  glBindVertexArray( m_uiVAO );
  glDrawArrays( GL_TRIANGLE_STRIP, 0, m_iNumber );
  glDrawArrays( GL_LINES, 0, m_iNumber );
  glBindVertexArray( 0 );
  m_eProgram.stop();
}

Cubes::Cubes() {}
Cubes::~Cubes() { initialize(); }
void Cubes::initialize() {
  m_eCubeBoxSize.initialize();
  m_eCubeObject.initialize();
  m_eCubeSequence.initialize();
}
void Cubes::load( float fSize, Box& boxObject, Box& boxSequence ) {
  m_eCubeBoxSize.load( Vec3( 0, 0, 0 ), Vec3( fSize, fSize, fSize ) );
  m_eCubeObject.load( boxObject.min(), boxObject.max() );
  m_eCubeSequence.load( boxSequence.min(), boxSequence.max() );
}

void Cubes::draw( Mat4& eMatMod, Mat4& eMatPro ) {
  m_eCubeBoxSize.draw( eMatMod, eMatPro, Color4( 0.0, 0.0, 0.0, 0.5 ) );
  m_eCubeObject.draw( eMatMod, eMatPro, Color4( 0.0, 0.0, 1.0, 0.5 ) );
  m_eCubeSequence.draw( eMatMod, eMatPro, Color4( 0.0, 1.0, 0.0, 0.5 ) );
}

Axes::Axes() {}
Axes::~Axes() { initialize(); }
void Axes::initialize() {
  m_eAxes[0].initialize();
  m_eAxes[1].initialize();
  m_eAxes[2].initialize();
}

void Axes::load( float fSize ) {
  m_eAxes[0].load( Vec3( 0, 0, 0 ), Vec3( fSize, 0.0, 0.0 ), fSize / 25.f, fSize / 100.f );
  m_eAxes[1].load( Vec3( 0, 0, 0 ), Vec3( 0.0, fSize, 0.0 ), fSize / 25.f, fSize / 100.f );
  m_eAxes[2].load( Vec3( 0, 0, 0 ), Vec3( 0.0, 0.0, fSize ), fSize / 25.f, fSize / 100.f );
}

void Axes::draw( Mat4& eMatMod, Mat4& eMatPro ) {
  m_eAxes[0].draw( eMatMod, eMatPro, Color4( 1.0, 0.0, 0.0, 1.0 ) );
  m_eAxes[1].draw( eMatMod, eMatPro, Color4( 0.0, 1.0, 0.0, 1.0 ) );
  m_eAxes[2].draw( eMatMod, eMatPro, Color4( 0.0, 0.0, 1.0, 1.0 ) );
}

// Render to texture
static const GLfloat g_pQuadVertexBufferData[6][3] = {{-1.0f, -1.0f, 0.0f}, {1.0f, -1.0f, 0.0f}, {-1.0f, 1.0f, 0.0f},
                                                      {-1.0f, 1.0f, 0.0f},  {1.0f, -1.0f, 0.0f}, {1.0f, 1.0f, 0.0f}};
RenderToTexture::RenderToTexture() {}

RenderToTexture::~RenderToTexture() {
  if ( m_bLoaded ) {
    glDeleteFramebuffers( 1, &m_uiFrameBufferName );
    glDeleteTextures( 1, &m_uiRenderedTexture );
    glDeleteRenderbuffers( 1, &m_uiDepthRenderBuffer );
    glDeleteBuffers( 1, &m_uiQuadVertexBuffer );
  }
}

void RenderToTexture::load( int iWidth, int iHeight ) {
  m_iWidth  = iWidth;
  m_iHeight = iHeight;
  m_bLoaded = true;
  glGenFramebuffers( 1, &m_uiFrameBufferName );
  glBindFramebuffer( GL_FRAMEBUFFER, m_uiFrameBufferName );

  glGenTextures( 1, &m_uiRenderedTexture );
  glBindTexture( GL_TEXTURE_2D, m_uiRenderedTexture );
  glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB, m_iWidth, m_iHeight, 0, GL_RGB, GL_UNSIGNED_SHORT, 0 );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
  glFramebufferTexture( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, m_uiRenderedTexture, 0 );

  // The depth buffer
  glGenRenderbuffers( 1, &m_uiDepthRenderBuffer );
  glBindRenderbuffer( GL_RENDERBUFFER, m_uiDepthRenderBuffer );
  glRenderbufferStorage( GL_RENDERBUFFER, GL_DEPTH_COMPONENT, m_iWidth, m_iHeight );
  glFramebufferRenderbuffer( GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_uiDepthRenderBuffer );

  glGenTextures( 1, &m_uiDepthTexture );
  glBindTexture( GL_TEXTURE_2D, m_uiDepthTexture );
  glTexImage2D( GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, m_iWidth, m_iHeight, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0 );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_NONE );

  // Set "renderedTexture" as our colour attachement #0
  glFramebufferTexture( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, m_uiRenderedTexture, 0 );
  glFramebufferTexture( GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, m_uiDepthTexture, 0 );
  glDrawBuffers( 1, m_uiDrawBuffers );

  if ( glCheckFramebufferStatus( GL_FRAMEBUFFER ) != GL_FRAMEBUFFER_COMPLETE ) { return; }

  glGenBuffers( 1, &m_uiQuadVertexBuffer );
  glBindBuffer( GL_ARRAY_BUFFER, m_uiQuadVertexBuffer );
  glBufferData( GL_ARRAY_BUFFER, sizeof( g_pQuadVertexBufferData ), g_pQuadVertexBufferData, GL_STATIC_DRAW );

  m_eProgram.create( "RenderToTexture", g_pRenderToTexureVertexShader, g_pRenderToTexureFragmentShader );
}

void RenderToTexture::clear() {
  if ( m_bLoaded ) {
    glBindFramebuffer( GL_FRAMEBUFFER, m_uiFrameBufferName );
    glViewport( 0, 0, m_iWidth, m_iHeight );
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
  }
}

void RenderToTexture::draw() {
  if ( m_bLoaded ) {
    m_eProgram.use();
    glActiveTexture( GL_TEXTURE0 );
    glBindTexture( GL_TEXTURE_2D, m_uiRenderedTexture );
    m_eProgram.setUniform( "renderedTexture", 0 );
    glActiveTexture( GL_TEXTURE1 );
    glBindTexture( GL_TEXTURE_2D, m_uiDepthTexture );
    // m_eProgram.setUniform( "renderedDepth", 1 );
    glEnableVertexAttribArray( 0 );
    glBindBuffer( GL_ARRAY_BUFFER, m_uiQuadVertexBuffer );
    glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0 );
    glDrawArrays( GL_TRIANGLES, 0, 6 );
    glDisableVertexAttribArray( 0 );
  }
}
