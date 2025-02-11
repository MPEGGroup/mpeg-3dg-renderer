#!/bin/bash

echo "$@"


for name in `ls source/*.cpp `
do
  clang-tidy \
    -p ./build/  \
    -header-filter=.* \
    -fix \
    $name \
    -- \
    -I/srv/wp21/PCC/ricardj/softwares/mpeg-pcc-renderer/include  \
    -I/srv/wp21/PCC/ricardj/softwares/mpeg-pcc-renderer/external/glfw/include \
    -I/srv/wp21/PCC/ricardj/softwares/mpeg-pcc-renderer/external/glfw/deps \
    -I/srv/wp21/PCC/ricardj/softwares/mpeg-pcc-renderer/external/nanoflann \
    -I/srv/wp21/PCC/ricardj/softwares/mpeg-pcc-renderer/external/program-options-lite
done

  exit ;
  $@ 



  # -I/usr/include/c++/7.4.0 \  -I /usr/include/x86_64-linux-gnu/c++/7/32 \


# ;/srv/wp21/PCC/ricardj/softwares/mpeg-pcc-renderer/include/PccRendererDef.h;/srv/wp21/PCC/ricardj/softwares/mpeg-pcc-renderer/include/PccRendererMath.h;/srv/wp21/PCC/ricardj/softwares/mpeg-pcc-renderer/include/PccRendererPlyReader.h;/srv/wp21/PCC/ricardj/softwares/mpeg-pcc-renderer/include/PccRendererProgram.h;/srv/wp21/PCC/ricardj/softwares/mpeg-pcc-renderer/include/PccRendererShader.h;/srv/wp21/PCC/ricardj/softwares/mpeg-pcc-renderer/include/PccRendererText.h;/srv/wp21/PCC/ricardj/softwares/mpeg-pcc-renderer/include/PccRendererWindow.h;/srv/wp21/PCC/ricardj/softwares/mpeg-pcc-renderer/source/PccRendererCamera.cpp;/srv/wp21/PCC/ricardj/softwares/mpeg-pcc-renderer/source/PccRendererMain.cpp;/srv/wp21/PCC/ricardj/softwares/mpeg-pcc-renderer/source/PccRendererPlyReader.cpp;/srv/wp21/PCC/ricardj/softwares/mpeg-pcc-renderer/source/PccRendererProgram.cpp;/srv/wp21/PCC/ricardj/softwares/mpeg-pcc-renderer/source/PccRendererShader.cpp;/srv/wp21/PCC/ricardj/softwares/mpeg-pcc-renderer/source/PccRendererText.cpp;/srv/wp21/PCC/ricardj/softwares/mpeg-pcc-renderer/source/PccRendererWindow.cpp




  # -system-headers  

#clang-tidy --list-checks -checks='*' | grep "modernize"