//Copyright(c) 2016 - 2025, InterDigital
//All rights reserved.
//See LICENSE under the root folder.

#include "PccRendererParameters.h"

#include "PccRendererWindow.h"
#include "PccRendererSequence.h"

double getTime() {
  static std::chrono::high_resolution_clock::time_point eStart = std::chrono::high_resolution_clock::now();
  return std::chrono::duration<float>( std::chrono::high_resolution_clock::now() - eStart ).count();
}

void readSequence( RendererParameters& params, Sequence& eSequence ) {
  eSequence.setFps( params.getFps() );
  eSequence.setDropDups( params.getDropDups() );
  eSequence.setPlayBackward( params.getPlayBackward() );
  eSequence.readFile( params.getFile(), params.getFrameIndex(), params.getFrameNumber(), params.getBinaryFile() );
  eSequence.readFile( params.getFileSrc(), params.getFrameIndex(), params.getFrameNumber(), params.getBinaryFile(),
                      true );
  eSequence.readDirectory( params.getDir(), params.getFrameNumber(), params.getBinaryFile() );
  eSequence.readDirectory( params.getDirSrc(), params.getFrameNumber(), params.getBinaryFile(), true );
  if ( params.getBoxSize() > 0 ) {
    eSequence.setBoxSize( (float)params.getBoxSize() );
  } else {
    eSequence.setBoxSize( eSequence.getBox().getMaxSize() );
  }
  if ( !eSequence.check() ) {
    printf( "Sequence configuration is not correct\n" );
    exit( 0 );
  }
  // Normalize objects position and size
  eSequence.normalize( params.getScaleMode(), params.getCenter() );
}

void readScene( RendererParameters& params, Sequence& eScene ) {
  if ( !params.getScenePath().empty() ) {
    eScene.readFile( params.getScenePath(), 0, 1, false );
    if ( eScene.getNumFrames() == 0 ) {
      printf( "Can't load scene object: %s \n", params.getScenePath().c_str() );
      exit( 0 );
    }
  }
}

int main( int argc, char* argv[] ) {
  RendererParameters params;
  Sequence           eSequence;
  Sequence           eScene;

  // Input parameters
  if ( !params.parseCfg( argc, argv ) ) { return 0; }
  params.print();

  // Read objects, source and background scene
  readSequence( params, eSequence );
  readScene( params, eSequence );

  // Create window
  Window eWindow( std::string( "PccAppRenderer - MPEG 3DG Renderer by InterDigital" ), params );
  if ( !eWindow.exist() ) {
    printf( "Could not create GLFW window. \n" );
    fflush( stdout );
    return -1;
  }
  eWindow.setSequence( &eSequence );
  if ( eScene.getNumFrames() > 0 ) {
    eWindow.setScene( &eScene, params.getSceneScale(), params.getScenePosition(), params.getSceneRotation() );
  }
  if ( params.getSoftwareRenderer() ) {
    eWindow.softwareRendering();
    return 0;
  }
  while ( !eWindow.close() ) { eWindow.draw(); }
  return 0;
}
