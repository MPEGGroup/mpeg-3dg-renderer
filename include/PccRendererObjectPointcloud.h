//Copyright(c) 2016 - 2025, InterDigital
//All rights reserved.
//See LICENSE under the root folder.

#ifndef _OBJECT_PLY_RENDERER_APP_H_
#define _OBJECT_PLY_RENDERER_APP_H_

#include "PccRendererDef.h"
#include "PccRendererObject.h"
#include "PccRendererCamera.h"

/*! \class %ObjectPointcloud class
 * \brief %ObjectPointcloud class.
 *
 *  This class is used to store a data of a PLY models. The stored data are the coordinate: X, Y, Z, the color: R,G,B
 * and the alpha: A if these data are present in the files. the normal and the other data stored in the PLY files are
 * not stored and rendered by the current program.
 */

class RigParameters {
 public:
  RigParameters();
  ~RigParameters();
  bool               exist() { return !m_eMatrix.empty(); }
  float              getFrameToWorldScale() { return m_fFrameToWorldScale; }
  Vec3               getFrameToWorldTranslation() { return m_fFrameToWorldTranslation; }
  float              getWidth() { return m_fWidth; }
  std::vector<Mat4>& getMatrix() { return m_eMatrix; }
  Mat4&              getMatrix( size_t index ) { return m_eMatrix[index]; }
  size_t             getCount() { return m_eMatrix.size(); }
  void               setCount( size_t count ) { m_eMatrix.resize( count ); }
  void               setWidth( float value ) { m_fWidth = value; }
  void               setFrameToWorldScale( float value ) { m_fFrameToWorldScale = value; }
  void               setFrameToWorldTranslation( Vec3 vector ) { m_fFrameToWorldTranslation = vector; }
  void               trace();
  Vec3               getCameraPosition( size_t index ) {
    float a = ( m_eMatrix[index][3][0] - m_fFrameToWorldTranslation[0] ) / m_fFrameToWorldScale;
    float b = ( m_eMatrix[index][3][1] - m_fFrameToWorldTranslation[1] ) / m_fFrameToWorldScale;
    float c = ( m_eMatrix[index][3][2] - m_fFrameToWorldTranslation[2] ) / m_fFrameToWorldScale;
    return Vec3( a, b, c );
  }

 private:
  float             m_fFrameToWorldScale = 0.f;
  Vec3              m_fFrameToWorldTranslation = Vec3(0, 0, 0);
  float             m_fWidth = 0;
  std::vector<Mat4> m_eMatrix;
};

class ObjectPointcloud : public Object {
 public:
  //! Constructor.
  ObjectPointcloud();
  //! Destructor.
  ~ObjectPointcloud();

  ObjectType getType() { return ObjectType::POINTCLOUD; }
  void       load();
  void       unload();
  void       draw( bool lighting );
  void       loadProgram();

  bool read( const std::string&        pFilename,
             int                       iFrameIndex,
             bool                      bBinary,
             int                       iDropDups,
             std::vector<std::string>& pTypeName );

  void         recomputeBoundingBox();
  void         center( Box box, float fBoxSize );
  virtual void scale( Box box, float fBoxSize );
  virtual void sortVertex(const Camera& cam);
  /**
   * \brief allocate the current object.
   * \param bAlpha Alpha values are present.
   * \param iNumPoints Number of points.
   * \param iFrameIndex Index of the frame.
   */
  void allocate( bool bAlpha, bool bNormal, bool bType, int iNumPoints, int iFrameIndex, int iNumDuplicate );

  /**
   * \brief add point.
   * \param point Coordinates.
   * \param color color components.
   * \param normal Normals.
   * \param type Type component.
   */
  void add( Point point, Color4 color, Normal normal, uint8_t type ) {
    if ( m_iIndex >= m_iNumPoints ) {
      printf( "ObjectPointcloud: can't add %d points in ObjectPointcloud allocate for %d points \n", m_iIndex,
              m_iNumPoints );
    } else {
      m_pPoints[m_iIndex] = point;
      if ( m_bAlpha ) {
        m_pColors4[m_iIndex] = color / 255.f;
      } else {
        m_pColors3[m_iIndex] = glm::vec3( color ) / 255.f;
      }
      if ( m_bNormal ) { m_pNormals[m_iIndex] = normal; }
      if ( m_bType ) { m_pTypes[m_iIndex] = type; }
      m_eBox.update( point );
      m_iIndex++;
    }
  }

  //! Get the pointer to the stored data. \return Pointer to the stored data.
  inline size_t             getNumPoints() const { return m_iNumPoints; }
  inline Point*             getPoints() { return m_pPoints.data(); };
  inline std::vector<Point> getVectorPoints() { return m_pPoints; };
  inline Point&             getPoints( size_t iIndex ) { return m_pPoints[iIndex]; };
  inline Color3&            getColors3( size_t iIndex ) {
    if ( !m_bAlpha ) { return m_pColors3[iIndex]; }
    return *( reinterpret_cast<Color3*>( &( m_pColors4[iIndex] ) ) );
  };
  inline Color4 getColors4( size_t iIndex ) {
    if ( m_bAlpha ) { return m_pColors4[iIndex]; }
    Color4 convert;
    convert[0] = m_pColors3[iIndex][0];
    convert[1] = m_pColors3[iIndex][1];
    convert[2] = m_pColors3[iIndex][2];
    convert[3] = 1.f;
    return convert;
  }
  inline void*    getColors() { return ( m_bAlpha ? (void*)m_pColors4.data() : (void*)m_pColors3.data() ); };
  inline Color3*  getColors3() { return m_pColors3.data(); };
  inline Color4*  getColors4() { return m_pColors4.data(); };
  inline Normal*  getNormals() { return m_pNormals.data(); };
  inline uint8_t* getTypes() { return m_pTypes.data(); };

  //! Get the alpha boolean. \return Boolean indicate that the current object have alpha component.
  inline bool getAlpha() { return m_bAlpha; };
  //! Get the normal boolean. \return Boolean indicate that the current object have normal components.
  inline bool getNormal() { return m_bNormal; };

  //! Get the number of duplicate points. \return Number of duplicate points.
  inline int getNumDuplicate() { return m_iNumDuplicate; };
  void       setBox( float fXMin, float fXMax, float fYMin, float fYMax, float fZMin, float fZMax );

  Vec3                 getBoundingBoxCenterPosition() { return m_eBox.center(); }
  std::vector<Color3>& getMultiColors3() { return m_pMultiColors3; };
  inline void          setMultiColors3( size_t index, size_t color, Color3 eColor ) {
    m_pMultiColors3[index * m_eRigParameters.getCount() + color] = eColor / 255.f;
  }
  RigParameters&     getRigParameters() { return m_eRigParameters; }
  std::vector<float> computeWeigth();
  void getRigPoints( std::vector<Vec3>& points, std::vector<Color3>& color, std::vector<Vec3>& direction );

  // Reset the current object and clear all stored points.
  inline void reset() {
    m_pPoints.clear();
    m_pColors3.clear();
    m_pColors4.clear();
    m_pNormals.clear();
    m_pTypes.clear();
    m_iNumPoints = 0;
    m_iIndex     = 0;
    m_pMultiColors3.clear();
  }



  std::string getInformation();
  void        removeDuplicatePoints( int iDropDups );
  bool        readBinary( const std::string& eString, int iFrameIndex );
  void        writeBinary( const std::string& eString );
  void        createBinaryDirectory( const std::string& eString );

  // Must return the number of data points
  inline size_t kdtree_get_point_count() const { return m_iNumPoints; }

  // Returns the dim'th component of the idx'th point in the class:
  // Since this is inlined and the "dim" argument is typically an immediate value, the
  //  "if/else's" are actually solved at compile time.
  inline float kdtree_get_pt( const size_t iIndex, int iDim ) const {
    if ( iDim == 0 ) { return m_pPoints[iIndex][0]; }
    if ( iDim == 1 ) { return m_pPoints[iIndex][1]; }
    return m_pPoints[iIndex][2];
  }

  // Optional bounding-box computation: return false to default to a standard bbox computation loop.
  //   Return true if the BBOX was already computed by the class and returned in "bb" so it can be avoided to redo it
  //   again. Look at bb.size() to find out the expected dimensionality (e.g. 2 or 3 for point clouds)
  template <class BBOX>
  bool kdtree_get_bbox( BBOX& /* bb */ ) const {
    return false;
  }

  Program&           getProgram() { return m_ePrograms[m_iProgramIndex]; }
  int32_t            getProgramIndex() { return m_iProgramIndex; }
  void               setProgramIndex( int32_t programIndex ) { m_iProgramIndex = programIndex; }
  size_t             getProgramNumber() { return m_ePrograms.size(); }
  const std::string& getProgramName() { return m_ePrograms[m_iProgramIndex].getName(); }

 private:
  void computeDistance( std::vector<Color3>& eColors );
  void computeDuplicate( std::vector<Color3>& eColors );
  void colorBasedType( std::vector<Color3>& eColors );
 
  static std::vector<Program> m_ePrograms;
  static int32_t              m_iProgramIndex;
  GLuint                      m_uiVAO;
  GLuint                      m_uiVBO;
  GLuint                      m_uiCBO;
  GLuint                      m_uiMCBO;
  GLuint                      m_uiIBO;
  std::vector<Point>          m_pPoints;
  std::vector<Color3>         m_pColors3;
  std::vector<Color4>         m_pColors4;
  std::vector<Normal>         m_pNormals;
  std::vector<uint8_t>        m_pTypes;
  int                         m_iIndex  = 0;
  bool                        m_bAlpha  = false;
  bool                        m_bNormal = false;
  bool                        m_bType   = false;
  bool                        m_bSort   = true;
  std::vector<Color3>         m_pMultiColors3;
  RigParameters               m_eRigParameters;
  int                         m_iNumPoints    = 0;
  int                         m_iNumDuplicate = 0;
};

#endif  // _OBJECT_PLY_RENDERER_APP_H_
