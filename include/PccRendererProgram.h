//Copyright(c) 2016 - 2025, InterDigital
//All rights reserved.
//See LICENSE under the root folder.

#ifndef _PROGRAM_RENDERER_APP_H_
#define _PROGRAM_RENDERER_APP_H_

#include "PccRendererDef.h"

class Shader;

/*! \class %Program
 * \brief %Program class.
 *
 *  This class is used to define a OpenGL shader program.
 */
class Program {
 public:
  /**
   * \brief Constructor.
   **/
  Program();

  /**
   * \brief Constructor.
   * \param  eShader %Shader object.
   **/
  Program( const std::vector<Shader>& eShader );

  /**
   * \brief Constructor.
   * \param  eName Name of the program.
   * \param  eVertexShader Source code of the vertex shader.
   * \param  eFragmentShader Source code of the fragment shader.
   */
  Program( std::string eName, const std::string& eVertexShader, const std::string& eFragmentShader );
  /**
   * \brief Constructor.vector
   * \param  eName Name of the program.
   * \param  eVertexShader Source code of the vertex shader.
   * \param  eGeometryShader Source code of the geometry shader.
   * \param  eFragmentShader Source code of the fragment shader.
   */
  Program( std::string        eName,
           const std::string& eVertexShader,
           const std::string& eGeometryShader,
           const std::string& eFragmentShader );

  //! \brief Desctructor.
  ~Program();

  /**
   * \brief Create program.
   * \param  eName Name of the program.
   * \param  eVertexShader Source code of the vertex shader.
   * \param  eFragmentShader Source code of the fragment shader.
   */
  void create( std::string eName, const std::string& eVertexShader, const std::string& eFragmentShader );
  /**
   * \brief Create program.
   * \param  eName Name of the program.
   * \param  eVertexShader Source code of the vertex shader.
   * \param  eGeometryShader Source code of the geometry shader.
   * \param  eFragmentShader Source code of the fragment shader.
   */
  void create( std::string        eName,
               const std::string& eVertexShader,
               const std::string& eGeometryShader,
               const std::string& eFragmentShader );

  //! \brief Installs a program object as part of current rendering state.
  void use() const;
  //! \brief Uninstalls a program object as part of current rendering state.
  void stop() const;

  //! \brief Returns the location of an attribute variable. \param pAttribName Name of the attribute. \return Location
  //! of an attribute variable.
  GLint attrib( const GLchar* pAttribName ) const;
  //! \brief Returns the location of an uniform variable. \param pUniformName Name of the uniform. \return Location of
  //! an uniform variable.
  GLint uniform( const GLchar* pUniformName ) const;

  //! Return the name of the program. \return Name of the program.
  const std::string& getName() { return m_eName; }

  inline bool isLoad() { return m_bLoad; }
  /**
   * \brief Specify the value of a matrix4x4 uniform variable.
   * \param pName Name of the uniform variable.
   * \param eMatrix Value of the Matrix.
   * \param bTranspose Transpose the matrix.
   **/
  void setUniform( const GLchar* pName, const Mat4& eMatrix, GLboolean bTranspose = GL_FALSE );
  /**
   * \brief Specify the value of a vector 3 uniform variable.
   * \param pName Name of the uniform variable.
   * \param eVector Value of the vector.
   **/
  void setUniform( const GLchar* pName, const Vec3& eVector );
  /**
   * \brief Specify the value of a vector 4 uniform variable.
   * \param pName Name of the uniform variable.
   * \param eVector Value of the vector.
   **/
  void setUniform( const GLchar* pName, const Vec4& eVector );
  /**
   * \brief Specify the value of a float uniform variable.
   * \param pName Name of the uniform variable.
   * \param fValue Value of the variable.
   **/
  void setUniform( const GLchar* pName, const float fValue );
  /**
   * \brief Specify the value of a int uniform variable.
   * \param pName Name of the uniform variable.
   * \param iValue Value of the variable.
   **/
  void setUniform( const GLchar* pName, const int iValue );

  void setUniform( const GLchar* pName, const std::vector<float>& pArrray );
  void setUniform( const GLchar* pName, const std::vector<Vec3>& pArrray );

  template <typename T>
  void setLightUniform( const char* eName, size_t iLightIndex, const T& eValue ) {
    std::ostringstream eStringStream;
    eStringStream << "allLights[" << iLightIndex << "]." << eName;
    std::string eString = eStringStream.str();
    setUniform( eString.c_str(), eValue );
  }

 private:
  void        loadShader( const std::vector<Shader>& eShaders );
  GLuint      m_uiObject = 0;
  std::string m_eName    = "";
  bool        m_bLoad    = false;
};

#endif  //~_PROGRAM_RENDERER_APP_H_
