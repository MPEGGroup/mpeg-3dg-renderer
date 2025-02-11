//Copyright(c) 2016 - 2025, InterDigital
//All rights reserved.
//See LICENSE under the root folder.

#ifndef _PRIMITIVE_RENDERER_APP_H_
#define _PRIMITIVE_RENDERER_APP_H_

#include "PccRendererDef.h"
#include "PccRendererProgram.h"
#include "PccRendererObjectMesh.h"

class Box;
class Mesh;
class Primitive {
 public:
  Primitive();
  ~Primitive();
  void initialize();

 protected:
  void    set( std::vector<Vec3>& points );
  void    set( std::vector<Vec3>& points, std::vector<Color3>& colors );
  void    set( std::vector<Vec3>& points, std::vector<Color4>& colors );
  GLuint  m_uiVAO = 0;
  GLuint  m_uiVBO = 0;
  GLuint  m_uiCBO = 0;
  Program m_eProgram;
  int     m_iNumber = -1;
};

class Background : public Primitive {
 public:
  void load( Vec3 a );
  void load( Vec3 a, Vec3 b );
  void draw();
};

class DistanceScale : public Primitive {
 public:
  void load();
  void draw();
};

class PointList : public Primitive {
 public:
  void load( std::vector<Vec3>& points, std::vector<Color3>& color, std::vector<Vec3>& direction );
  void draw( Mat4& eMatMod, Mat4& eMatPro, int iPointSize = 10, bool bDrawLines = true );
};

class Sphere : public Primitive {
 public:
  void load( Vec3 position, float size );
  void draw( Mat4& eMatMod, Mat4& eMatPro );
};

class Cube : public Primitive {
 public:
  void load( Vec3 eV0, Vec3 eV1 );
  void draw( Mat4& eMatMod, Mat4& eMatPro, Color4 color );
};

class Floor : public Primitive {
 public:
  void load( Box eBox, Color4& color );
  void draw( Mat4& eMatMod, Mat4& eMatPro, bool bLighting );

 private:
  ObjectMesh m_eObjectMesh;
};

class Arrow : public Primitive {
 public:
  void load( Vec3 eA, Vec3 eB, float fA, float fB );
  void draw( Mat4& eMatMod, Mat4& eMatPro, Color4 color );
};

class Cubes {
 public:
  Cubes();
  ~Cubes();
  void initialize();
  void load( float fSize, Box& boxObject, Box& boxSequence );
  void draw( Mat4& eMatMod, Mat4& eMatPro );

 private:
  Cube m_eCubeBoxSize;
  Cube m_eCubeObject;
  Cube m_eCubeSequence;
};

class Axes {
 public:
  Axes();
  ~Axes();
  void initialize();
  void load( float fSize );
  void draw( Mat4& eMatMod, Mat4& eMatPro );

 private:
  Arrow m_eAxes[3];
};

class RenderToTexture {
 public:
  RenderToTexture();
  ~RenderToTexture();
  void   load( int iWidth, int iHeight );
  void   draw();
  void   clear();
  GLuint getTexture() { return m_uiRenderedTexture; }

 private:
  int     m_iWidth              = 0;
  int     m_iHeight             = 0;
  GLuint  m_uiFrameBufferName   = 0;
  GLuint  m_uiRenderedTexture   = 0;
  GLuint  m_uiDepthRenderBuffer = 0;
  GLuint  m_uiDepthTexture      = 0;
  GLuint  m_uiQuadVertexBuffer  = 0;
  GLenum  m_uiDrawBuffers[2]    = {GL_COLOR_ATTACHMENT0, GL_DEPTH_ATTACHMENT};
  bool    m_bLoaded             = false;
  Program m_eProgram;
};

const std::vector<std::vector<Color3>> g_fBackground = {
    {{0.5f, 0.5f, 1.0f}, {0.0f, 0.0f, 0.0f}},  //  0: blue gradient
    {{0.0f, 0.0f, 0.0f}, {0.2f, 0.2f, 0.2f}},  //  1: black gradient
    {{0.2f, 0.2f, 0.2f}, {0.4f, 0.4f, 0.4f}},  //  2: black grey gradient
    {{0.4f, 0.4f, 0.4f}, {0.6f, 0.6f, 0.6f}},  //  3: medium grey gradient
    {{0.6f, 0.6f, 0.6f}, {0.8f, 0.8f, 0.8f}},  //  4: light grey gradient
    {{0.8f, 0.8f, 0.8f}, {1.0f, 1.0f, 1.0f}},  //  5: white gradient
    {{0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}},  //  6: black
    {{0.4f, 0.4f, 0.4f}, {0.4f, 0.4f, 0.4f}},  //  7: dark grey
    {{0.5f, 0.5f, 0.5f}, {0.5f, 0.5f, 0.5f}},  //  8: medium grey
    {{0.6f, 0.6f, 0.6f}, {0.6f, 0.6f, 0.6f}},  //  9: light grey
    {{1.0f, 1.0f, 1.0f}, {1.0f, 1.0f, 1.0f}}   // 10: white
};

const std::vector<Color3> g_pColorPalette = {{1.f, 1.f, 1.f},  {1.f, .5f, 0.f}, {1.f, 1.f, 0.f}, {0.f, 1.f, 0.f},
                                             {0.5f, 0.f, 1.f}, {0.f, 1.f, 1.f}, {0.f, 0.f, 1.f}, {1.f, 0.f, 1.f},
                                             {1.f, 0.f, 0.f},  {0.f, 0.5f, 1.f}};

const std::vector<Color3> g_pColorRgb = {
    {0.0000f, 0.0000f, 0.5073f}, {0.0000f, 0.0000f, 0.5230f}, {0.0000f, 0.0000f, 0.5386f}, {0.0000f, 0.0000f, 0.5543f},
    {0.0000f, 0.0000f, 0.5699f}, {0.0000f, 0.0000f, 0.5855f}, {0.0000f, 0.0000f, 0.6012f}, {0.0000f, 0.0000f, 0.6168f},
    {0.0000f, 0.0000f, 0.6325f}, {0.0000f, 0.0000f, 0.6481f}, {0.0000f, 0.0000f, 0.6637f}, {0.0000f, 0.0000f, 0.6794f},
    {0.0000f, 0.0000f, 0.6950f}, {0.0000f, 0.0000f, 0.7107f}, {0.0000f, 0.0000f, 0.7263f}, {0.0000f, 0.0000f, 0.7419f},
    {0.0000f, 0.0000f, 0.7576f}, {0.0000f, 0.0000f, 0.7732f}, {0.0000f, 0.0000f, 0.7889f}, {0.0000f, 0.0000f, 0.8045f},
    {0.0000f, 0.0000f, 0.8201f}, {0.0000f, 0.0000f, 0.8358f}, {0.0000f, 0.0000f, 0.8514f}, {0.0000f, 0.0000f, 0.8671f},
    {0.0000f, 0.0000f, 0.8827f}, {0.0000f, 0.0000f, 0.8983f}, {0.0000f, 0.0000f, 0.9140f}, {0.0000f, 0.0000f, 0.9296f},
    {0.0000f, 0.0000f, 0.9453f}, {0.0000f, 0.0000f, 0.9609f}, {0.0000f, 0.0000f, 0.9765f}, {0.0000f, 0.0000f, 0.9922f},
    {0.0000f, 0.0068f, 1.0000f}, {0.0000f, 0.0225f, 1.0000f}, {0.0000f, 0.0381f, 1.0000f}, {0.0000f, 0.0538f, 1.0000f},
    {0.0000f, 0.0694f, 1.0000f}, {0.0000f, 0.0850f, 1.0000f}, {0.0000f, 0.1007f, 1.0000f}, {0.0000f, 0.1163f, 1.0000f},
    {0.0000f, 0.1320f, 1.0000f}, {0.0000f, 0.1476f, 1.0000f}, {0.0000f, 0.1632f, 1.0000f}, {0.0000f, 0.1789f, 1.0000f},
    {0.0000f, 0.1945f, 1.0000f}, {0.0000f, 0.2102f, 1.0000f}, {0.0000f, 0.2258f, 1.0000f}, {0.0000f, 0.2414f, 1.0000f},
    {0.0000f, 0.2571f, 1.0000f}, {0.0000f, 0.2727f, 1.0000f}, {0.0000f, 0.2884f, 1.0000f}, {0.0000f, 0.3040f, 1.0000f},
    {0.0000f, 0.3196f, 1.0000f}, {0.0000f, 0.3353f, 1.0000f}, {0.0000f, 0.3509f, 1.0000f}, {0.0000f, 0.3666f, 1.0000f},
    {0.0000f, 0.3822f, 1.0000f}, {0.0000f, 0.3978f, 1.0000f}, {0.0000f, 0.4135f, 1.0000f}, {0.0000f, 0.4291f, 1.0000f},
    {0.0000f, 0.4448f, 1.0000f}, {0.0000f, 0.4604f, 1.0000f}, {0.0000f, 0.4761f, 1.0000f}, {0.0000f, 0.4917f, 1.0000f},
    {0.0000f, 0.5073f, 1.0000f}, {0.0000f, 0.5230f, 1.0000f}, {0.0000f, 0.5386f, 1.0000f}, {0.0000f, 0.5543f, 1.0000f},
    {0.0000f, 0.5699f, 1.0000f}, {0.0000f, 0.5855f, 1.0000f}, {0.0000f, 0.6012f, 1.0000f}, {0.0000f, 0.6168f, 1.0000f},
    {0.0000f, 0.6325f, 1.0000f}, {0.0000f, 0.6481f, 1.0000f}, {0.0000f, 0.6637f, 1.0000f}, {0.0000f, 0.6794f, 1.0000f},
    {0.0000f, 0.6950f, 1.0000f}, {0.0000f, 0.7107f, 1.0000f}, {0.0000f, 0.7263f, 1.0000f}, {0.0000f, 0.7419f, 1.0000f},
    {0.0000f, 0.7576f, 1.0000f}, {0.0000f, 0.7732f, 1.0000f}, {0.0000f, 0.7889f, 1.0000f}, {0.0000f, 0.8045f, 1.0000f},
    {0.0000f, 0.8201f, 1.0000f}, {0.0000f, 0.8358f, 1.0000f}, {0.0000f, 0.8514f, 1.0000f}, {0.0000f, 0.8671f, 1.0000f},
    {0.0000f, 0.8827f, 1.0000f}, {0.0000f, 0.8983f, 1.0000f}, {0.0000f, 0.9140f, 1.0000f}, {0.0000f, 0.9296f, 1.0000f},
    {0.0000f, 0.9453f, 1.0000f}, {0.0000f, 0.9609f, 1.0000f}, {0.0000f, 0.9765f, 1.0000f}, {0.0000f, 0.9922f, 1.0000f},
    {0.0068f, 1.0000f, 0.9922f}, {0.0225f, 1.0000f, 0.9765f}, {0.0381f, 1.0000f, 0.9609f}, {0.0538f, 1.0000f, 0.9453f},
    {0.0694f, 1.0000f, 0.9296f}, {0.0850f, 1.0000f, 0.9140f}, {0.1007f, 1.0000f, 0.8983f}, {0.1163f, 1.0000f, 0.8827f},
    {0.1320f, 1.0000f, 0.8671f}, {0.1476f, 1.0000f, 0.8514f}, {0.1632f, 1.0000f, 0.8358f}, {0.1789f, 1.0000f, 0.8201f},
    {0.1945f, 1.0000f, 0.8045f}, {0.2102f, 1.0000f, 0.7889f}, {0.2258f, 1.0000f, 0.7732f}, {0.2414f, 1.0000f, 0.7576f},
    {0.2571f, 1.0000f, 0.7419f}, {0.2727f, 1.0000f, 0.7263f}, {0.2884f, 1.0000f, 0.7107f}, {0.3040f, 1.0000f, 0.6950f},
    {0.3196f, 1.0000f, 0.6794f}, {0.3353f, 1.0000f, 0.6637f}, {0.3509f, 1.0000f, 0.6481f}, {0.3666f, 1.0000f, 0.6325f},
    {0.3822f, 1.0000f, 0.6168f}, {0.3978f, 1.0000f, 0.6012f}, {0.4135f, 1.0000f, 0.5855f}, {0.4291f, 1.0000f, 0.5699f},
    {0.4448f, 1.0000f, 0.5543f}, {0.4604f, 1.0000f, 0.5386f}, {0.4761f, 1.0000f, 0.5230f}, {0.4917f, 1.0000f, 0.5073f},
    {0.5073f, 1.0000f, 0.4917f}, {0.5230f, 1.0000f, 0.4761f}, {0.5386f, 1.0000f, 0.4604f}, {0.5543f, 1.0000f, 0.4448f},
    {0.5699f, 1.0000f, 0.4291f}, {0.5855f, 1.0000f, 0.4135f}, {0.6012f, 1.0000f, 0.3978f}, {0.6168f, 1.0000f, 0.3822f},
    {0.6325f, 1.0000f, 0.3666f}, {0.6481f, 1.0000f, 0.3509f}, {0.6637f, 1.0000f, 0.3353f}, {0.6794f, 1.0000f, 0.3196f},
    {0.6950f, 1.0000f, 0.3040f}, {0.7107f, 1.0000f, 0.2884f}, {0.7263f, 1.0000f, 0.2727f}, {0.7419f, 1.0000f, 0.2571f},
    {0.7576f, 1.0000f, 0.2414f}, {0.7732f, 1.0000f, 0.2258f}, {0.7889f, 1.0000f, 0.2102f}, {0.8045f, 1.0000f, 0.1945f},
    {0.8201f, 1.0000f, 0.1789f}, {0.8358f, 1.0000f, 0.1632f}, {0.8514f, 1.0000f, 0.1476f}, {0.8671f, 1.0000f, 0.1320f},
    {0.8827f, 1.0000f, 0.1163f}, {0.8983f, 1.0000f, 0.1007f}, {0.9140f, 1.0000f, 0.0850f}, {0.9296f, 1.0000f, 0.0694f},
    {0.9453f, 1.0000f, 0.0538f}, {0.9609f, 1.0000f, 0.0381f}, {0.9765f, 1.0000f, 0.0225f}, {0.9922f, 1.0000f, 0.0068f},
    {1.0000f, 0.9922f, 0.0000f}, {1.0000f, 0.9765f, 0.0000f}, {1.0000f, 0.9609f, 0.0000f}, {1.0000f, 0.9453f, 0.0000f},
    {1.0000f, 0.9296f, 0.0000f}, {1.0000f, 0.9140f, 0.0000f}, {1.0000f, 0.8983f, 0.0000f}, {1.0000f, 0.8827f, 0.0000f},
    {1.0000f, 0.8671f, 0.0000f}, {1.0000f, 0.8514f, 0.0000f}, {1.0000f, 0.8358f, 0.0000f}, {1.0000f, 0.8201f, 0.0000f},
    {1.0000f, 0.8045f, 0.0000f}, {1.0000f, 0.7889f, 0.0000f}, {1.0000f, 0.7732f, 0.0000f}, {1.0000f, 0.7576f, 0.0000f},
    {1.0000f, 0.7419f, 0.0000f}, {1.0000f, 0.7263f, 0.0000f}, {1.0000f, 0.7107f, 0.0000f}, {1.0000f, 0.6950f, 0.0000f},
    {1.0000f, 0.6794f, 0.0000f}, {1.0000f, 0.6637f, 0.0000f}, {1.0000f, 0.6481f, 0.0000f}, {1.0000f, 0.6325f, 0.0000f},
    {1.0000f, 0.6168f, 0.0000f}, {1.0000f, 0.6012f, 0.0000f}, {1.0000f, 0.5855f, 0.0000f}, {1.0000f, 0.5699f, 0.0000f},
    {1.0000f, 0.5543f, 0.0000f}, {1.0000f, 0.5386f, 0.0000f}, {1.0000f, 0.5230f, 0.0000f}, {1.0000f, 0.5073f, 0.0000f},
    {1.0000f, 0.4917f, 0.0000f}, {1.0000f, 0.4761f, 0.0000f}, {1.0000f, 0.4604f, 0.0000f}, {1.0000f, 0.4448f, 0.0000f},
    {1.0000f, 0.4291f, 0.0000f}, {1.0000f, 0.4135f, 0.0000f}, {1.0000f, 0.3978f, 0.0000f}, {1.0000f, 0.3822f, 0.0000f},
    {1.0000f, 0.3666f, 0.0000f}, {1.0000f, 0.3509f, 0.0000f}, {1.0000f, 0.3353f, 0.0000f}, {1.0000f, 0.3196f, 0.0000f},
    {1.0000f, 0.3040f, 0.0000f}, {1.0000f, 0.2884f, 0.0000f}, {1.0000f, 0.2727f, 0.0000f}, {1.0000f, 0.2571f, 0.0000f},
    {1.0000f, 0.2414f, 0.0000f}, {1.0000f, 0.2258f, 0.0000f}, {1.0000f, 0.2102f, 0.0000f}, {1.0000f, 0.1945f, 0.0000f},
    {1.0000f, 0.1789f, 0.0000f}, {1.0000f, 0.1632f, 0.0000f}, {1.0000f, 0.1476f, 0.0000f}, {1.0000f, 0.1320f, 0.0000f},
    {1.0000f, 0.1163f, 0.0000f}, {1.0000f, 0.1007f, 0.0000f}, {1.0000f, 0.0850f, 0.0000f}, {1.0000f, 0.0694f, 0.0000f},
    {1.0000f, 0.0538f, 0.0000f}, {1.0000f, 0.0381f, 0.0000f}, {1.0000f, 0.0225f, 0.0000f}, {1.0000f, 0.0068f, 0.0000f},
    {0.9922f, 0.0000f, 0.0000f}, {0.9765f, 0.0000f, 0.0000f}, {0.9609f, 0.0000f, 0.0000f}, {0.9453f, 0.0000f, 0.0000f},
    {0.9296f, 0.0000f, 0.0000f}, {0.9140f, 0.0000f, 0.0000f}, {0.8983f, 0.0000f, 0.0000f}, {0.8827f, 0.0000f, 0.0000f},
    {0.8671f, 0.0000f, 0.0000f}, {0.8514f, 0.0000f, 0.0000f}, {0.8358f, 0.0000f, 0.0000f}, {0.8201f, 0.0000f, 0.0000f},
    {0.8045f, 0.0000f, 0.0000f}, {0.7889f, 0.0000f, 0.0000f}, {0.7732f, 0.0000f, 0.0000f}, {0.7576f, 0.0000f, 0.0000f},
    {0.7419f, 0.0000f, 0.0000f}, {0.7263f, 0.0000f, 0.0000f}, {0.7107f, 0.0000f, 0.0000f}, {0.6950f, 0.0000f, 0.0000f},
    {0.6794f, 0.0000f, 0.0000f}, {0.6637f, 0.0000f, 0.0000f}, {0.6481f, 0.0000f, 0.0000f}, {0.6325f, 0.0000f, 0.0000f},
    {0.6168f, 0.0000f, 0.0000f}, {0.6012f, 0.0000f, 0.0000f}, {0.5855f, 0.0000f, 0.0000f}, {0.5699f, 0.0000f, 0.0000f},
    {0.5543f, 0.0000f, 0.0000f}, {0.5386f, 0.0000f, 0.0000f}, {0.5230f, 0.0000f, 0.0000f}, {0.5073f, 0.0000f, 0.0000f}};

#endif  // _MATH_RENDERER_APP_H_
