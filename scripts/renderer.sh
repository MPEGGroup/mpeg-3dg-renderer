#!/bin/bash

CURDIR=$(dirname "$( readlink -f "$0" )" );
MAINDIR=$(dirname "$CURDIR");

print_usage()
{
  echo "$0: Render PLY/OBJ sequences based on camera path and create video"
  echo "";
  echo "    Usage:" 
  echo "       $0 -i <INPUT> [-o <OUTPUT>] [-h] ";
  echo "";
  echo "    Parameters:";
  echo "       -h|--help             : help message."; 
  echo "       -i|--input=*          : input dir"; 
  echo "       -o|--output=*         : suffix of the name of the created video"; 
  echo "       -w|--width=*          : width of the created video"; 
  echo "       -h|--height=*         : height of the created video"; 
  echo "       -r|--rotation=*       : rotation"; 
  echo "       -f|--frame=*          : frame number"; 
  echo "       --psize=*             : point size"; 
  echo "       --rendererId=*        : renderer index"; 
  echo "                               - 0: cube";
  echo "                               - 1: circle";
  echo "                               - 2: point";
  echo "                               - 3: blended point";
  echo "       --blendMode=*         : blend mode for blended point:"; 
  echo "                               - 0: Gaussian";
  echo "                               - 1: Linear";
  echo "       --alphaFalloff=*      : blend alpha falloff"; 
  echo "       -v|--viewpoint=*      : viewpoint file"; 
  echo "       -x|--camera=*         : camera path file"; 
  echo "       --cameraPathIndex=*   : camera path index"; 
  echo "       -t|--videoType=*      : video type:";
  echo "                               - 0: rgb 16bits";
  echo "                               - 1: yuv 10bits";
  echo "                               - 2: HEVC/X265 Lossless";
  echo "                               - 3: HEVC/X265 QP18";
  echo "                               - 4: Graphics interchange format (GIF)";
  echo "                               - 5: zipped yuv 10bits";
  echo "       -pb|--playbackward=*  : playbackward"; 
  echo "       --floor=*             : draw floor box."; 
  echo "       --spline=*            : interpolate camera path by b-splines."; 
  echo "       --lighting=*          : enable lighting."; 
  echo "       --softwareRenderer=*  : use software rendering process"; 
  echo "       --force               : recreate video if file already exists."; 
  echo "       --novideo             : open renderer user interface."; 
  echo "       -b|--binary=*         : open binary version of the pointcloud files"; 
  echo "       --threads=*           : ffmpeg threads."; 
  echo "       --ffmpeg=*            : ffmpeg path."; 
  echo "";
  echo "    Examples:";
  echo "      - $0 -i input.rgb "; 
  echo "      - $0 -i input.rgb -o output.yuv ";  
  echo "    ";
  if [ "$#" != 0 ] ; then echo  "ERROR: $1"; fi
  exit 0;
}

# Default parameters
DIR="";
OUTPUT="";
WIDTH=1920
HEIGHT=1080
VIDEOTYPE=1
CAMERAPATH=""
CAMERAPATHINDEX="";
ROTATION=0
BINARY=0
FRAME=0
PLAYBACKWARD=1
VIEWPOINT=""
OTHERPARAMS="";
NOVIDEO=0;
SPLINE=0;
FLOOR=1;
LIGHTING=0;
SOFTWARERENDERER=0;
FORCE=0;
THREADS=0;
FFMPEG=ffmpeg
PSIZE=1;
MODE=0;
BLEND=0;
FOCUS=1;

while [[ $# -gt 0 ]]
do
  C=$1; 
  if [[ "$C" =~ -- ]] ; then
	if [[ "$C" =~ [=] ]] ; then V=${C/[-a-zA-Z0-9]*=/}; fi 
  else V=$2; shift; fi; 
  case "$C" in    
    --help               ) print_usage;;   
    -i|--input=*         ) DIR=$V"/";;     
    -o|--output=*        ) OUTPUT=$V;;     
    -w|--width=*         ) WIDTH=$V;;      
    -h|--height=*        ) HEIGHT=$V;;    
    -r|--rotation=*      ) ROTATION=$V;; 
    -b|--binary=*        ) BINARY=$V;; 
    -f|--frame=*         ) FRAME=$V;;    
    -v|--viewpoint=*     ) VIEWPOINT=$V;;    
    -x|--camera=*        ) CAMERAPATH=$V;;    
    --cameraPathIndex=*  ) CAMERAPATHINDEX=$V;;    
    -pb|--playbackward=* ) PLAYBACKWARD=$V;;    
    -t|--videoType=*     ) VIDEOTYPE=$V;;    
    --param=*            ) OTHERPARAMS="$OTHERPARAMS $V";;    
    --novideo=*          ) NOVIDEO=$V;;    
    --floor=*            ) FLOOR=$V;;    
    --spline=*           ) SPLINE=$V;;    
    --lighting=*         ) LIGHTING=$V;;    
    --softwareRenderer=* ) SOFTWARERENDERER=$V;;    
    --force              ) FORCE=1;;    
    --threads=*          ) THREADS=$V;;    
    --ffmpeg=*           ) FFMPEG=$V;;
	--psize=*            ) PSIZE=$V;;
	--rendererId=*       ) MODE=$V;;
	--blendMode=*        ) BLEND=$V;;
	--alphaFalloff=*       ) FOCUS=$V;;
    *                    ) print_usage "unsupported arguments: $C ";;
  esac
  shift;
done
if [ "$DIR" == "" ] ; then print_usage "input dir must be set"; exit; fi
if [ ! -d "$DIR"    ] ; then print_usage "input dir not exist: $DIR"; exit; fi
if [[ "$( uname -s )" =~ Linux ]]; 
then 
  RENDERER=${MAINDIR}/bin/linux/Release/PccAppRenderer
else 
  RENDERER=${MAINDIR}/bin/Windows/Release/PccAppRenderer.exe
fi  
if [ ! -f "$RENDERER" ] ; then print_usage "renderer application not found: $RENDERER"; exit; fi
# if [ ! -f "$CAMERAPATH" ] && [ "$CAMERAPATHINDEX" == "" ]; then 
#   print_usage "camera path or camera path index must be set."; 
#   exit; 
# fi
if [ "${VIDEOTYPE}" -lt 0 ] || [ "${VIDEOTYPE}" -gt 5 ] ; then print_usage "video type not correct: $VIDEOTYPE"; exit; fi
echo "OUTPUT = $OUTPUT";
NAME=$(basename "$DIR");
if [ "$OUTPUT" == "" ] 
then 
  VIDEODIR=$(dirname "$MAINDIR")/videos/${NAME}
  if [ ! -d "${VIDEODIR}" ] ; then mkdir -p "${VIDEODIR}"; fi 
  OUTPUT=${VIDEODIR}/${NAME}
  if [ "$CAMERAPATHINDEX" != ""  ] ; then OUTPUT=${OUTPUT}_CPI${CAMERAPATHINDEX};  fi
  OUTPUT=${OUTPUT}_L${LIGHTING}_SW${SOFTWARERENDERER};
fi
if [ "${NOVIDEO}" == "1" ] ;  then SOFTWARERENDERER=0; fi

SUFFIXES=( "_rgb48.rgb" \
           "_10bit_p420.yuv" \
           "_x265lossless.mp4" \
           "_x265lossy.mp4" \
           ".gif"\
           ".zip" )
VIDEO=${OUTPUT}_${WIDTH}x${HEIGHT}${SUFFIXES[${VIDEOTYPE}]}
VIDEOSIZE=unknow;
if [ -f "${VIDEO}" ] 
then
  if [ $FORCE == 1 ]
  then 
    rm -f "${VIDEO}";
  else
    VIDEOSIZE=${WIDTH}x${HEIGHT}
  fi
fi

echo "${VIDEO} size = $VIDEOSIZE "; 
echo "OTHERPARAMS = $OTHERPARAMS"
if [ ! -f "${VIDEO}" ] || [ "$VIDEOSIZE" != "${WIDTH}x${HEIGHT}" ] || [ "${NOVIDEO}" == 1 ]
then   
  PARAMS="\
    --PlyDir=${DIR} \
    --fps=30 \
    --width=${WIDTH} \
    --height=${HEIGHT} \
    --frameNumber=${FRAME} \
    --rotate=${ROTATION} \
    --playBackward=${PLAYBACKWARD} \
    --play=1 \
    --overlay=0 \
    --size=${PSIZE} \
    --type=${MODE} \
    --blendMode=${BLEND} \
    --alphaFalloff=${FOCUS} \
    --binary=${BINARY} \
    --visible=0 \
    --spline=${SPLINE} \
    --floor=${FLOOR} \
    --lighting=${LIGHTING} \
    --softwareRenderer=${SOFTWARERENDERER} ";
    
  if [ "$VIEWPOINT"       != ""  ] ; then PARAMS="$PARAMS --viewpoint=${VIEWPOINT}"; fi
  if [ "$CAMERAPATH"      != ""  ] ; then PARAMS="$PARAMS --camera=${CAMERAPATH}"; fi
  if [ "$CAMERAPATHINDEX" != ""  ] ; then PARAMS="$PARAMS --cameraPathIndex=${CAMERAPATHINDEX}"; fi
  if [ "${NOVIDEO}"       == "0" ] ; then OTHERPARAMS="$OTHERPARAMS --RgbFile=${OUTPUT} "; fi  
  CMD="${RENDERER} ${PARAMS} ${OTHERPARAMS}"
  echo -e "${CMD// --/ \\\\\\n   --}" 
  eval "$CMD"
    
  if [ "${NOVIDEO}" == 0 ] 
  then
    for INPUT in "${OUTPUT}"*.rgb;
    do 
      CMD="${CURDIR}/convert_video.sh \
            --input=${INPUT} \
            --output=${VIDEO} \
            --videoType=${VIDEOTYPE} \
            --threads=${THREADS} \
            --ffmpeg=${FFMPEG}"
      echo -e "${CMD// --/ \\\\\\n   --}" 
      eval "$CMD"
      if [ "$VIDEOTYPE" != 0 ] ; then  rm "${INPUT}"; fi
    done
  fi
else
  echo "${VIDEO} already exist ( size = $VIDEOSIZE )"; 
fi
