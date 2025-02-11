//Copyright(c) 2016 - 2025, InterDigital
//All rights reserved.
//See LICENSE under the root folder.

#include "PccRendererShader.h"

Shader::Shader( const std::string& eShaderCode, GLenum eShaderType ) : m_uiObject( 0 ) {
  GLint       iStatus, iLength = 0;
  const char* pCode = eShaderCode.c_str();
  m_uiObject        = glCreateShader( eShaderType );
  if ( m_uiObject == 0 ) { throw std::runtime_error( "glCreateShader failed" ); }
  glShaderSource( m_uiObject, 1, static_cast<const GLchar**>( &pCode ), nullptr );
  glCompileShader( m_uiObject );
  glGetShaderiv( m_uiObject, GL_COMPILE_STATUS, &iStatus );
  if ( iStatus == GL_FALSE ) {
    glGetShaderiv( m_uiObject, GL_INFO_LOG_LENGTH, &iLength );
    std::vector<char> pString;
    pString.resize( iLength + 1 );
    glGetShaderInfoLog( m_uiObject, iLength, nullptr, pString.data() );
    glDeleteShader( m_uiObject );
    m_uiObject           = 0;
    std::string eMessage = "Compile failure in shader: \n " + eShaderCode + "\n" + pString.data();
    printf( "%s \n", eMessage.c_str() );
    fflush( stdout );
    throw std::runtime_error( eMessage );
  }
}

Shader::~Shader() = default;
