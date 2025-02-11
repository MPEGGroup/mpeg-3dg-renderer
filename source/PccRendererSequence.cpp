//Copyright(c) 2016 - 2025, InterDigital
//All rights reserved.
//See LICENSE under the root folder.


#include "PccRendererSequence.h"
#include "PccRendererObjectPointcloud.h"
#include "PccRendererObjectMesh.h"
#include "PccRendererPrimitive.h"

Sequence::Sequence() {}
Sequence::~Sequence() {
  m_eObject.clear();
  m_eObjectSrc.clear();
  m_pTypeName.clear();
}

bool Sequence::check() {
  if ( m_eObject.empty() ) {
    printf( "ERROR: No file have been load \n" );
    return false;
  }
  if ( m_eObjectSrc.size() > 0 && m_eObject.size() != m_eObjectSrc.size() ) {
    printf( "ERROR: Number of Src files and number of Input files must the same \n" );
    return false;
  }
  return true;
}

Object& Sequence::getObject() {
  auto& eObject   = m_bDisplaySource && m_eObjectSrc.size() > 0 ? m_eObjectSrc : m_eObject;
  int   iNumFrame = getNumFrames();
  if ( m_iFrameIndex < 0 && m_iFrameIndex > iNumFrame ) {
    printf( "Error: can't load object of frame index = %d is not in [%d;%d] \n", m_iFrameIndex, 0, iNumFrame );
    exit( -1 );
  }
  return *eObject[m_bPlayBackward && m_iFrameIndex >= (int)m_eObject.size() ? iNumFrame - 1 - m_iFrameIndex
                                                                            : m_iFrameIndex];
}

Object& Sequence::getObject( int iIndex ) {
  auto& eObject   = m_bDisplaySource && m_eObjectSrc.size() > 0 ? m_eObjectSrc : m_eObject;
  int   iNumFrame = getNumFrames();
  if ( iIndex < 0 && iIndex > iNumFrame ) {
    printf( "Error: can't load object of frame index = %d is not in [%d;%d] \n", iIndex, 0, iNumFrame );
    exit( -1 );
  }
  return *eObject[m_bPlayBackward && iIndex >= (int)m_eObject.size() ? iNumFrame - 1 - iIndex : iIndex];
}

const std::string& Sequence::getFilename() { return getObject().getFilename(); }
void               Sequence::load() { getObject().load(); }

void Sequence::unload() {
  for ( auto& eObject : m_eObject ) { eObject->unload(); }
  for ( auto& eObject : m_eObjectSrc ) { eObject->unload(); }
}

void Sequence::normalize( int32_t iScaleMode, bool bCenter ) {
  printf( "Normalize sequence size and position according to %f bounding box (scale = %d center = %d)\n", m_fBoxSize,
          iScaleMode, bCenter );
  printBoundingBox( "ORG" );
  if ( iScaleMode != 0 ) {
    for ( auto& eObject : m_eObject ) { eObject->scale( iScaleMode == 1 ? eObject->getBox() : m_eBox, m_fBoxSize ); }
    for ( auto& eObject : m_eObjectSrc ) { eObject->scale( iScaleMode == 1 ? eObject->getBox() : m_eBox, m_fBoxSize ); }
    recomputeBoundingBox();
    printBoundingBox( "SCA" );
  }
  if ( bCenter ) {
    for ( auto& eObject : m_eObject ) { eObject->center( m_eBox, m_fBoxSize ); }
    for ( auto& eObject : m_eObjectSrc ) { eObject->center( m_eBox, m_fBoxSize ); }
    recomputeBoundingBox();
    printBoundingBox( "CEN" );
  }
}

void Sequence::printBoundingBox( std::string string, bool bAll ) {
  if ( bAll ) {
    for ( auto& eObject : m_eObject ) { eObject->printBoundingBox( string ); }
  }
  printf( "%s: Sequence  : BB=[%8.2f;%8.2f][%8.2f;%8.2f][%8.2f;%8.2f] \n", string.c_str(), m_eBox.min()[0],
          m_eBox.max()[0], m_eBox.min()[1], m_eBox.max()[1], m_eBox.min()[2], m_eBox.max()[2] );
}

void Sequence::recomputeBoundingBox() {
  if ( static_cast<int>( m_eObject.size() ) > 0 ) {
    m_eBox = Box();
    for ( auto& pObject : m_eObject ) {
      pObject->recomputeBoundingBox();
      m_eBox.update( pObject->getBox() );
    }
  }
}

void Sequence::add( std::shared_ptr<Object> pObject, bool bSource ) {
  m_eBox.update( pObject->getBox() );
  ( bSource ? m_eObjectSrc : m_eObject ).push_back( pObject );
}

void Sequence::readDirectory( const std::string& sDir, int iFrameNumber, bool eBinaryFile, bool bSource ) {
  if ( !sDir.empty() ) {
    printf( "readDirectory %s bSource = %d \n", sDir.c_str(), bSource );
    readDirectory( sDir, "ply", iFrameNumber, eBinaryFile, bSource );
    if ( ( bSource ? m_eObjectSrc : m_eObject ).size() == 0 ) {
      readDirectory( sDir, "obj", iFrameNumber, eBinaryFile, bSource );
    }
    if ( bSource && m_eObject.size() == m_eObjectSrc.size() ) {
      for ( size_t i = 0; i < m_eObject.size(); i++ ) {
        m_eObject[i]->setSource( m_eObjectSrc[i].get() );
        m_eObjectSrc[i]->setSource( m_eObject[i].get() );
      }
    }
    printf("Source read object size = %zu and %zu \n",m_eObject.size(),m_eObjectSrc.size());
  }
}

#if MSVC
#include <windows.h>
void Sequence::getFileInDirector( std::string               sDirector,
                                  std::string               sExtension,
                                  std::vector<std::string>& eFileLists ) {
  WIN32_FIND_DATA eSearchData;
  memset( &eSearchData, 0, sizeof( WIN32_FIND_DATA ) );
  std::string eString = sDirector + std::string( "\\*." ) + sExtension;
  HANDLE      eHandle = FindFirstFile( eString.c_str(), &eSearchData );
  while ( eHandle != INVALID_HANDLE_VALUE ) {
    std::string eFile = std::string( eSearchData.cFileName );
    if ( eFile.substr( eFile.find_last_of( "." ) ).compare( "." + sExtension ) == 0 ) { eFileLists.push_back( eFile ); }
    if ( FindNextFile( eHandle, &eSearchData ) == FALSE ) { break; }
  }
  FindClose( eHandle );
  std::sort( eFileLists.begin(), eFileLists.end() );
}
#else
#include <dirent.h>
void Sequence::getFileInDirector( std::string               sDirector,
                                  std::string               sExtension,
                                  std::vector<std::string>& eFileLists ) {
  DIR* pDir;
  if ( ( pDir = opendir( sDirector.c_str() ) ) == nullptr ) {
    printf( "READER: the %s director can't be parse \n", sDirector.c_str() );
  } else {
    struct dirent* pDirent;
    while ( ( pDirent = readdir( pDir ) ) != nullptr ) {
      std::string eFile           = std::string( pDirent->d_name );
      int         iIndexLastPoint = eFile.find_last_of( '.' );
      if ( iIndexLastPoint != -1 && eFile.substr( iIndexLastPoint ) == "." + sExtension ) {
        eFileLists.push_back( eFile );
      }
    }
    std::sort( eFileLists.begin(), eFileLists.end() );
    closedir( pDir );
  }
}
#endif

void Sequence::readFile( const std::string& sFile, int iFrameIndex, int iFrameNumber, bool eBinaryFile, bool bSource ) {
  if ( !sFile.empty() ) {
    std::string sExtension = getExtension( sFile );
    auto &eObject = bSource ? m_eObjectSrc : m_eObject;
    if ( sExtension == "ply" ) {
      int  iNumRead  = 0;
      bool bReadDone = false;
      for ( int i = 0; i < iFrameNumber; i++ ) {
        auto pObject = std::make_shared<ObjectPointcloud>();
        eObject.push_back( pObject );
      }
#pragma omp parallel for
      for ( int i = 0; i < iFrameNumber; i++ ) {
        auto pObject = (std::dynamic_pointer_cast<ObjectPointcloud>)( eObject[i] );
        if ( pObject->read( sFile, iFrameIndex + i, eBinaryFile, m_iDropDups, m_pTypeName ) ) {
          bReadDone = true;
          PROGRESSBAR( iNumRead, iFrameNumber, "Read Ply files %3d", iFrameIndex + iNumRead );
          iNumRead++;
        }
      }
      if ( !bReadDone ) { eObject.clear(); }
    }
    if ( sExtension == "obj" || ( sExtension == "ply" && eObject.size() == 0 ) ) {
      int  iNumRead  = 0;
      bool bReadDone = false;
      for ( int i = 0; i < iFrameNumber; i++ ) {
        auto pObject = std::make_shared<ObjectMesh>();
        eObject.push_back( pObject );
      }
#pragma omp parallel for
      for ( int i = 0; i < iFrameNumber; i++ ) {
        auto pObject = (std::dynamic_pointer_cast<ObjectMesh>)( eObject[i] );
        if ( pObject->read( sFile, iFrameIndex + i ) ) {
          bReadDone = true;
          PROGRESSBAR( iNumRead, iFrameNumber, "Read Ply files %3d", iFrameIndex + iNumRead );
          iNumRead++;
        }
      }
      if ( !bReadDone ) { eObject.clear(); }
    }
    for ( auto& element : eObject ) { m_eBox.update( element->getBox() ); }
    if ( bSource && m_eObject.size() == m_eObjectSrc.size() ) {
      for ( size_t i = 0; i < m_eObject.size(); i++ ) {
        m_eObject[i]->setSource( m_eObjectSrc[i].get() );
        m_eObjectSrc[i]->setSource( m_eObject[i].get() );
      }
    }
  }
}

void Sequence::readDirectory( std::string pDirector,
                              std::string pExtension,
                              int         iFrameNumber,
                              bool        bBinary,
                              bool        bSource ) {
#ifdef MSVC
  char pNewName[512];
  GetFullPathName( pDirector.c_str(), 512, pNewName, NULL );
#else
  const char* pNewName = pDirector.c_str();
#endif
  std::vector<std::string> eFileLists = std::vector<std::string>();
  getFileInDirector( pNewName, pExtension, eFileLists );
  if ( eFileLists.empty() ) {
    printf( "READER: the %s director don't have any %s file. \n", pNewName, pExtension.c_str() );
    return;
  }
  if ( iFrameNumber == 0 || iFrameNumber > static_cast<int>( eFileLists.size() ) ) {
    iFrameNumber = static_cast<int>( eFileLists.size() );
  }
  printf( "READER: scan director: %s Frame number = %d \n", pNewName, iFrameNumber );
  auto &eObject = bSource ? m_eObjectSrc : m_eObject;
  if ( pExtension == "ply" ) {
    int  iNumRead  = 0;
    bool bReadDone = false;
    for ( int i = 0; i < iFrameNumber; i++ ) {
      auto pObject = std::make_shared<ObjectPointcloud>();
      eObject.push_back( pObject );
    }
#pragma omp parallel for
    for ( int i = 0; i < iFrameNumber; i++ ) {
      std::string ePath   = std::string( pNewName ) + eFileLists[i];
      auto        pObject = (std::dynamic_pointer_cast<ObjectPointcloud>)( eObject[i] );
      if ( pObject->read( ePath, -1, bBinary, m_iDropDups, m_pTypeName ) ) {
        bReadDone = true;
        PROGRESSBAR( iNumRead, iFrameNumber, "Read Ply files %3d", iNumRead );
        iNumRead++;
      }
    }
    if ( !bReadDone ) { eObject.clear(); }
  }
  printf("READ pExtension = %s eObject size = %zu \n",pExtension.c_str(), eObject.size());
  if ( pExtension == "obj" || ( pExtension == "ply" && eObject.size() == 0 ) ) {
    int iNumRead = 0;
    for ( int i = 0; i < iFrameNumber; i++ ) {
      auto pObject = std::make_shared<ObjectMesh>();
      eObject.push_back( pObject );
    }
#pragma omp parallel for
    for ( int i = 0; i < iFrameNumber; i++ ) {
      auto        pObject = (std::dynamic_pointer_cast<ObjectMesh>)( eObject[i] );
      std::string ePath   = std::string( pNewName ) + eFileLists[i];
      if ( pObject->read( ePath, -1 ) ) {
        PROGRESSBAR( iNumRead, iFrameNumber, "Read obj files %3d", iNumRead );
        iNumRead++;
      }
    }
  }
  for ( auto& element : eObject ) { m_eBox.update( element->getBox() ); }
}
