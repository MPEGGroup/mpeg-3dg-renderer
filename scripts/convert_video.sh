#!/bin/bash

function isnumber(){ if [ -n "${1##[0-9]*}" ] ; then print_usage "is not a number"; fi }
function getsize()  
{ 
  name=$(basename "${1%.???}")
  for str in $(echo "$name" | tr "_" " ") 
  do
    if [[ $str =~ .*bit    ]] ; then  BD=$(echo "$str" | awk -F "bit" '{ print $1}' ); fi
    if [[ $str =~ .*Hz     ]] ; then FPS=$(echo "$str" | awk -F "Hz"  '{ print $1}' ); fi
    if [[ $str =~ .*fps    ]] ; then FPS=$(echo "$str" | awk -F "fps" '{ print $1}' ); fi
    if [[ $str =~ .*hz     ]] ; then FPS=$(echo "$str" | awk -F "Hz"  '{ print $1}' ); fi
    if [[ $str =~ .*Fps    ]] ; then FPS=$(echo "$str" | awk -F "fps" '{ print $1}' ); fi   
    if [[ $str =~ .*x.*    ]] ; then   x=$(echo "$str" | awk -F "x"   '{ print $1}' ); y=$(echo "$str" | awk -F "x"   '{ print $2}' ); 
    if [ -z "${x##[0-9]*}" ] && [ -z "${y##[0-9]*}" ] && [ "$x" != "" ] && [ "$y" != "" ] ; then  X=$x; Y=$y; fi; fi    
  done
  if [ "$BD" == "" ] || [ "$BD" != 8 ] && [ "$BD" != 9 ] && [ "$BD" != 10 ] && [ "$BD" != 11 ] && [ "$BD" != 12 ] && [ "$BD" != 16 ] ; then 
    print_usage "bit depth can't be read from the filename"; 
  fi
  if [ "$X"  == ""   ] ; then print_usage "width can't be read from the filename"; fi
  if [ "$Y"  == ""   ] ; then print_usage "height can't be read from the filename"; fi
  if [ "$BD" == "8"  ] ; then RGB="rgb24"; fi
  if [ "$BD" == "16" ] ; then RGB="rgb48"; fi
}

print_usage()
{
  echo "MPEG 3DG PCC - convert raw rgb24/rgb48 video to yuv420p10le, mp4 HEVC Lossless or mp4 HEVC QP18: "
  echo "";
  echo "    Usage:" 
  echo "       $0 -i <INPUT> [-o <OUTPUT>] [-h] ";
  echo "";
  echo "    Parameters:";
  echo "       -h                : print help"; 
  echo "       -i <INPUT>        : input video";
  echo "       -o <OUTPUT>       : output video";
  echo "       --videoType=*     : video type:";
  echo "                            - 0: rgb 16bits i444";
  echo "                            - 1: yuv 10bits p420";
  echo "                            - 2: HEVC/X265 Lossless";
  echo "                            - 3: HEVC/X265 QP18";
  echo "                            - 4: Graphics interchange format (GIF)";
  echo "                            - 5: zipped yuv 10bits p420";
  echo "       --threads=*       : ffmpeg threads."; 
  echo "       --ffmpeg=*        : ffmpeg path."; 
  echo "";
  echo "    Examples:";
  echo "      - $0 -i input.rgb "; 
  echo "      - $0 -i input.rgb -o output.yuv ";  
  echo "    ";
  if [ "$#" != 2 ] ; then echo  "ERROR: $1"; fi
  exit 0;
}

INPUT="";
VIDEOTYPE=3;
THREADS=0;
FFMPEG=ffmpeg;
FPS=30
while [[ $# -gt 0 ]]
do
  C=$1; 
  if [[ "$C" =~ [=] ]] ; then V=${1/[-a-zA-Z0-9]*=/}; else V=$2; shift; fi; 
  case "$C" in
    -h|--help         ) print_usage "number of parameters must be 1 or 2 ( != $# ).";;    
    -i|--input=*      ) INPUT=$V;;     
    -o|--output=*     ) OUTPUT=$V;;    
    --videoType=*     ) VIDEOTYPE=$V;;    
    --threads=*       ) THREADS=$V;;    
    --ffmpeg=*        ) FFMPEG=$V;;    
    *                 ) print_usage "unsupported arguments: $C ";;
  esac
  shift;
done
if [ "$INPUT" == "" ] ; then print_usage "INPUT must be defined";  fi
if [ ! -f "$INPUT"  ] ; then print_usage "$INPUT doesn't exist "; fi
getsize "$INPUT"


if [[ "$( uname -s )" =~ Linux ]]; 
then 
  if ! [ -x "$(command -v "${FFMPEG}")" ]; then echo 'Error: ffmpeg is not installed'; exit 1; fi  
else 
  if ! [ -x "$(command -v "${FFMPEG}")" ]; 
  then 
    FFMPEG="/c/Program Files/ffmpeg/bin/ffmpeg.exe"; 
    echo "Error: ffmpeg is not installed, force path: $FFMPEG";      
    if [ ! -f "$FFMPEG" ] ; then print_usage "$FFMPEG doesn't exist "; fi
  fi
fi  

PARAM_ENCODER="";
SUFFIXES=( "_rgb48.rgb" \
           "_10bit_p420.yuv" \
           "_x265lossless.mp4" \
           "_x265lossy.mp4" \
           ".gif"\
           ".zip" )
           
if [ "$OUTPUT" == "" ] ; then OUTPUT=$(echo "$INPUT" | awk -F "_" '{ print $1}')_${X}x${Y}_${SUFFIXES[${C}]}; fi  

echo "INPUT      = $INPUT "
echo "OUTPUT     = $OUTPUT "
echo "VIDEOTYPE  = $VIDEOTYPE "
echo "FFMPEG     = $FFMPEG "

PARAM="-y -f rawvideo -r ${FPS} -pix_fmt $RGB -s ${X}x${Y} -threads ${THREADS} -i ${INPUT} "
GIFCFG="fps=10,scale=320:-1:flags=lanczos,split[s0][s1];[s0]palettegen[p];[s1][p]paletteuse"

case "$VIDEOTYPE" in    
  0)
    echo "RGB => yuv: nothing to do";   
    CMD="mv $INPUT $OUTPUT";; 
  1) 
    echo "RGB => yuv"; 
    CMD="$FFMPEG ${PARAM} -c:v rawvideo -pix_fmt yuv420p10le $OUTPUT";;
  2) 
    PARAM_ENCODER="-c:v libx265 -preset fast -x265-params lossless=1";
    echo "RGB => X265 lossless: PARAM_ENCODER = $PARAM_ENCODER "
    CMD="$FFMPEG ${PARAM} $PARAM_ENCODER -pix_fmt yuv420p10le $OUTPUT";;
  3)
    PARAM_ENCODER="-c:v libx265 -crf 10 -tag:v hvc1 ";
    echo "RGB => X265 lossy: PARAM_ENCODER = $PARAM_ENCODER "
    CMD="$FFMPEG ${PARAM} $PARAM_ENCODER -pix_fmt yuv420p10le $OUTPUT";;
  4)
    echo "RGB => GIF "
    CMD="$FFMPEG ${PARAM} -vf \"${GIFCFG}\" -loop 0 $OUTPUT";;
  5)
    echo "RGB => yuv => zip";
    YUV=${OUTPUT%.???}_10bit_p420.yuv    
    if [[ "$( uname -s )" =~ Linux ]]; then 
      CMD="$FFMPEG ${PARAM} -c:v rawvideo -pix_fmt yuv420p10le ${YUV}; zip   -jm      ${OUTPUT} ${YUV}" 
    else 
      CMD="$FFMPEG ${PARAM} -c:v rawvideo -pix_fmt yuv420p10le ${YUV}; 7z.exe a -sdel ${OUTPUT} ${YUV}";   
    fi;;
esac

echo "$CMD"
eval "$CMD"



