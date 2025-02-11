//Copyright(c) 2016 - 2025, InterDigital
//All rights reserved.
//See LICENSE under the root folder.

#ifndef _SHADER_RENDERER_APP_H_
#define _SHADER_RENDERER_APP_H_

#include "PccRendererDef.h"

/*! \class %Shader
 * \brief %Shader class.
 *
 *  This class is used to define a OpenGL shader.
 */
class Shader {
 public:
  //! Constructor \param  pShaderCode Source code of the shader. \param eShaderType Type of the shader: fragment, vertex
  //! or geometry shaders.
  Shader( const std::string& pShaderCode, GLenum eShaderType );
  //! Destructor.
  ~Shader();
  //! Accessor to the OpenGL shader index. \return Index of the OpenGl shader.
  GLuint getObject() const { return m_uiObject; }

 private:
  GLuint m_uiObject;
};

#endif  //~_SHADER_RENDERER_APP_H_
