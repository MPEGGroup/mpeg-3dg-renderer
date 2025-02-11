#pragma once
//Copyright(c) 2016 - 2025, InterDigital
//All rights reserved.
//See LICENSE under the root folder.

#ifndef _IMAGE_RENDERER_APP_H_
#define _IMAGE_RENDERER_APP_H_

#include "PccRendererDef.h"

class Image {
 public:
  Image( int iWidth = 0, int iHeight = 0, int iNbComp = 3 ) { allocate( iWidth, iHeight, iNbComp ); }
  Image( Image& eImage, int iNbComp ) { copy( eImage, iNbComp ); }
  ~Image() { m_eData.clear(); }

  void allocate( int iWidth, int iHeight, int iNbComp = 3 ) {
    m_iWidth  = iWidth;
    m_iHeight = iHeight;
    m_iNbComp = iNbComp;
    if ( m_iNbComp != 3 && m_iNbComp != 4 ) {
      printf( "Image number of componant equal to %d is not supported \n", m_iNbComp );
      exit( -1 );
    }
    m_eData.resize( m_iWidth * iHeight * m_iNbComp, 0 );
  }
  inline Vec2i     getSize() { return Vec2i( m_iWidth, m_iHeight ); }
  inline int       getWidth() const { return m_iWidth; }
  inline int       getHeight() const { return m_iHeight; }
  inline uint16_t* data() { return m_eData.data(); }
  void             fill( Color3 eColor ) {
    const auto   r    = ( uint16_t )( ( std::numeric_limits<uint16_t>::max )() * eColor[0] );
    const auto   g    = ( uint16_t )( ( std::numeric_limits<uint16_t>::max )() * eColor[1] );
    const auto   b    = ( uint16_t )( ( std::numeric_limits<uint16_t>::max )() * eColor[2] );
    const size_t size = m_iWidth * m_iHeight * m_iNbComp;
    for ( size_t i = 0; i < size; i += m_iNbComp ) {
      m_eData[i + 0] = r;
      m_eData[i + 1] = g;
      m_eData[i + 2] = b;
    }
  }

  inline void set( size_t i, size_t j, Color3 eColor ) {
    for ( int c = 0; c < 3; c++ ) {
      m_eData[( j * m_iWidth + i ) * 3 + c] =
          ( uint16_t )( glm::clamp( ( std::numeric_limits<uint16_t>::max )() * eColor[c] + 0.5f, 0.f,
                                    (float)( std::numeric_limits<uint16_t>::max )() ) );
    }
  }

  void copy( Image& eSrc, int iNbComp ) {
    allocate( eSrc.m_iWidth, eSrc.m_iHeight, iNbComp );
    if ( m_iNbComp == eSrc.m_iNbComp ) {
      memcpy( data(), eSrc.data(), m_iWidth * m_iHeight * m_iNbComp * sizeof( uint16_t ) );
    } else {
      const int iMinNbComp = ( std::min )( m_iNbComp, eSrc.m_iNbComp );
      uint16_t *pSrc = eSrc.data(), *pDst = data();
      for ( int i = m_iHeight * m_iWidth; i > 0; i--, pSrc += eSrc.m_iNbComp, pDst += m_iNbComp ) {
        for ( int c = 0; c < iMinNbComp; c++ ) { *pDst = *pSrc; }
      }
    }
  }

  void write( FILE* file, int iNbComp = 3 ) {
    if ( iNbComp != m_iNbComp ) {
      Image eSave( *this, iNbComp );
      eSave.write( file, iNbComp );
    } else {
      for ( int i = m_iHeight - 1, iStride = iNbComp * m_iWidth; i >= 0; i-- ) {
        fwrite( data() + i * iStride, iStride * sizeof( uint16_t ), 1, file );
      }
    }
  }

 private:
  int                   m_iWidth;
  int                   m_iHeight;
  int                   m_iNbComp;
  std::vector<uint16_t> m_eData;
};

#endif