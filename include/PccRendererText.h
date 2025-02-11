//Copyright(c) 2016 - 2025, InterDigital
//All rights reserved.
//See LICENSE under the root folder.

#ifndef _TEXT_RENDERER_APP_H_
#define _TEXT_RENDERER_APP_H_

#include "PccRendererDef.h"
#include "PccRendererProgram.h"

class Program;

/*! \class Text
 * \brief Display text class.
 *
 *  This class is used to display text and message in the OpenGl window.
 */
class Text {
 public:
  Text();             //!<  \brief Constructor.
  ~Text();            //!<  \brief Destructor.
  void initialize();  //!<  \brief Initialize the characters and create the OpenGl textures.
  void initRect() { m_eRectP0 = m_eRectP1 = {-1, -1}; }  //!<  \brief Initialize the rectangle area rendering.
  void renderRect( float fSize, Vec4 eBack );  //!<  \brief Draw the rectangle around the previous rendered messages.

  /*!
   * \brief Draw a text message.
   * \param eString string of the text message.
   * \param fX      X coordinate where the message must be display.
   * \param fY      Y coordinate where the message must be display.
   * \param fSize   Size of the message.
   * \param eColor  Color of the message.
   * \param eBack   Background of the message.
   * Draw a text message in the OpenGl window.
   */
  void renderLine( std::string eString, float fX, float fY, float fSize, Vec3 eColor, Vec4 eBack );

  /*!
   * \brief Draw a multilines text message in the top left corner of the OpenGl window.
   * \param eString string of the multi-line text message.
   * \param fSize   Size of the message.
   * \param eColor  Color of the message.
   * \param eBack   Background of the message.
   *
   * Draw a multi-lines text message in the top left corner of the OpenGl window.
   *
   */
  void renderMultiLines( const std::string& eString,
                         float              fSize,
                         Vec3               eColor = {0.0f, 0.0f, 0.0f},
                         Vec4               eBack  = {0.3f, 0.7f, 0.9f, 0.5f} );

 private:
  /*! \class Character
   * \brief Object to store OpenGL texture and data to define character.
   *
   *  This class defines a character and store the OpenGL texture index and the parameters used to display the
   * character.
   */
  class Character {
   public:
    Character( GLuint uiTexture, Vec2 eSize, Vec2 eCoordinate ) :
        m_uiTexture( uiTexture ), m_eSize( eSize ), m_eCoordinate( eCoordinate ) {}
    ~Character() = default;

    inline GLuint getTexture() { return m_uiTexture; }
    inline Vec2&  getSize() { return m_eSize; }
    inline Vec2&  getCoordinate() { return m_eCoordinate; }

   private:
    GLuint m_uiTexture;
    Vec2   m_eSize;
    Vec2   m_eCoordinate;
  };
  void                   load();
  std::vector<Character> m_eCharacters;
  Program                m_eProgram;
  GLuint                 m_uiVAO;
  GLuint                 m_uiVBO;
  Vec2                   m_eRectP0;
  Vec2                   m_eRectP1;
};

#endif  //~_TEXT_RENDERER_APP_H_
