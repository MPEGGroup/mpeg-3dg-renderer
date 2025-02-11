//Copyright(c) 2016 - 2025, InterDigital
//All rights reserved.
//See LICENSE under the root folder.

#include "PccRendererProgram.h"

#include <utility>
#include "PccRendererShader.h"

Program::Program() {}

Program::Program( std::string eName, const std::string& eVertexShader, const std::string& eFragmentShader ) :
    m_uiObject( 0 ), m_eName( std::move( eName ) ) {
  create( eName, eVertexShader, eFragmentShader );
}

Program::Program( std::string        eName,
                  const std::string& eVertexShader,
                  const std::string& eGeometryShader,
                  const std::string& eFragmentShader ) :
    m_uiObject( 0 ), m_eName( std::move( eName ) ) {
  create( eName, eVertexShader, eGeometryShader, eFragmentShader );
}

Program::Program( const std::vector<Shader>& eShaders ) { loadShader( eShaders ); }

Program::~Program() {
  if ( m_uiObject != 0 ) {
    glUseProgram( 0 );
    glDeleteProgram( m_uiObject );
    m_uiObject = 0;
  }
}

void Program::create( std::string eName, const std::string& eVertexShader, const std::string& eFragmentShader ) {
  m_eName = eName;
  std::vector<Shader> eShaders;
  eShaders.emplace_back( eVertexShader, GL_VERTEX_SHADER );
  eShaders.emplace_back( eFragmentShader, GL_FRAGMENT_SHADER );
  loadShader( eShaders );
}

void Program::create( std::string        eName,
                      const std::string& eVertexShader,
                      const std::string& eGeometryShader,
                      const std::string& eFragmentShader ) {
  m_eName = eName;
  std::vector<Shader> eShaders;
  eShaders.emplace_back( eVertexShader, GL_VERTEX_SHADER );
  eShaders.emplace_back( eGeometryShader, GL_GEOMETRY_SHADER );
  eShaders.emplace_back( eFragmentShader, GL_FRAGMENT_SHADER );
  loadShader( eShaders );
}

void Program::loadShader( const std::vector<Shader>& eShaders ) {
  if ( eShaders.empty() ) { throw std::runtime_error( "No eShaders were provided to create the program" ); }
  m_uiObject = glCreateProgram();
  if ( m_uiObject == 0 ) { throw std::runtime_error( "glCreateProgram failed" ); }
  for ( const auto& eShader : eShaders ) { glAttachShader( m_uiObject, eShader.getObject() ); }
  glLinkProgram( m_uiObject );
  for ( const auto& eShader : eShaders ) { glDetachShader( m_uiObject, eShader.getObject() ); }
  GLint status;
  glGetProgramiv( m_uiObject, GL_LINK_STATUS, &status );
  if ( status == GL_FALSE ) {
    std::string pMessage( "Program linking failure: " );
    GLint       iLength;
    glGetProgramiv( m_uiObject, GL_INFO_LOG_LENGTH, &iLength );
    std::vector<char> eString;
    eString.resize( iLength + 1 );
    glGetProgramInfoLog( m_uiObject, iLength, nullptr, eString.data() );
    pMessage += eString.data();
    eString.clear();
    glDeleteProgram( m_uiObject );
    m_uiObject = 0;
    printf( "%s \n", pMessage.c_str() );
    fflush( stdout );
    throw std::runtime_error( pMessage );
  }
  m_bLoad = true;
}

void  Program::use() const { glUseProgram( m_uiObject ); }
void  Program::stop() const { glUseProgram( 0 ); }
GLint Program::attrib( const GLchar* pName ) const {
  if ( !pName ) { throw std::runtime_error( "attribName was NULL" ); }
  GLint iAttrib = glGetAttribLocation( m_uiObject, pName );
  if ( iAttrib == -1 ) { throw std::runtime_error( std::string( "Program attribute not found: " ) + pName ); }
  return iAttrib;
}
GLint Program::uniform( const GLchar* pName ) const {
  if ( !pName ) { throw std::runtime_error( "uniformName was NULL" ); }
  GLint iUniform = glGetUniformLocation( m_uiObject, pName );
  if ( iUniform == -1 ) { throw std::runtime_error( std::string( "Program uniform not found: " ) + pName ); }
  return iUniform;
}
void Program::setUniform( const GLchar* pName, const Mat4& eMatrix, GLboolean bTranspose ) {
  glUniformMatrix4fv( uniform( pName ), 1, bTranspose, glm::value_ptr( eMatrix ) );
}
void Program::setUniform( const GLchar* pName, const float fValue ) { glUniform1f( uniform( pName ), fValue ); }
void Program::setUniform( const GLchar* pName, const int iValue ) { glUniform1i( uniform( pName ), iValue ); }
void Program::setUniform( const GLchar* pName, const Vec3& eVector ) {
  glUniform3fv( uniform( pName ), 1, glm::value_ptr( eVector ) );
}
void Program::setUniform( const GLchar* pName, const Vec4& eVector ) {
  glUniform4fv( uniform( pName ), 1, glm::value_ptr( eVector ) );
}
void Program::setUniform( const GLchar* pName, const std::vector<float>& pArrray ) {
  glUniform1fv( uniform( pName ), static_cast<GLsizei>( pArrray.size() ), pArrray.data() );
}
void Program::setUniform( const GLchar* pName, const std::vector<Vec3>& pArrray ) {
  glUniform3fv( uniform( pName ), static_cast<GLsizei>( pArrray.size() ),
                reinterpret_cast<const GLfloat*>( pArrray.data() ) );
}
