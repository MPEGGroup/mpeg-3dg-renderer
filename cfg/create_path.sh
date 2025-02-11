#!/bin/bash  


pi=`echo "scale=10; 4*a(1)" | bc -l`; 


#### Cone up and down longdress_static
d=300;
for((i=0;i<${d};i++)) 
do 
  r=`echo "scale=8; 1024 + 2048 * s( ( ${d} - $i ) * ${pi} / ( 2 * ${d} ) )" | bc -l`; 
  z=`echo "scale=8;  256 +  640 * s( (        $i ) * ${pi} / ( 2 * ${d} ) )" | bc -l`;   
  x=`echo "scale=8;  512 - $r * c( 4 * ${pi} * ${i} / ${d} )" | bc -l`; 
  y=`echo "scale=8;  512 - $r * s( 4 * ${pi} * ${i} / ${d} )" | bc -l`; 
  printf "%4d  %15.8f %15.8f %15.8f      %12.8f %12.8f %12.8f     %12.8f %12.8f %12.8f \n"  $i    $x $z $y    512 $z 512  0 1 0; 
done   > longdress_static.txt 

for((i=0;i<${d};i++)) 
do 
  b=`echo "$d + $i " | bc `;
  r=`echo "scale=8; 1024 + 2048 * s( (        $i ) * ${pi} / ( 2 * ${d} ) )" | bc -l`; 
  z=`echo "scale=8;  256 +  640 * s( ( ${d} - $i ) * ${pi} / ( 2 * ${d} ) )" | bc -l`; 
  x=`echo "scale=8;  512 - $r * c( 4 * ${pi} * ${i} / ${d} )" | bc -l`; 
  y=`echo "scale=8;  512 - $r * s( 4 * ${pi} * ${i} / ${d} )" | bc -l`;   
  printf "%4d  %15.8f %15.8f %15.8f      %12.8f %12.8f %12.8f     %12.8f %12.8f %12.8f \n"  $b    $x $z $y    512 $z 512  0 1 0; 
done  >> longdress_static.txt 


#### Cone up longdress_static
d=300;
for((i=0;i<${d};i++)) 
do   
  r=`echo "scale=8; 1024 + 2048 * s( ( ${d} - $i ) * ${pi} / ( 2 * ${d} ) )" | bc -l`; 
  z=`echo "scale=8;  256 +  640 * s( (        $i ) * ${pi} / ( 2 * ${d} ) )" | bc -l`;   
  x=`echo "scale=8;  512 - $r * c( 4 * ${pi} * ${i} / ${d} )" | bc -l`; 
  y=`echo "scale=8;  512 - $r * s( 4 * ${pi} * ${i} / ${d} )" | bc -l`; 
  printf "%4d  %15.8f %15.8f %15.8f      %12.8f %12.8f %12.8f     %12.8f %12.8f %12.8f \n"  $i    $x $z $y    512 $z 512  0 1 0; 
done   > longdress_dynamic.txt 

#### Cylinder up red and back 
d=300;
for((i=0;i<${d};i++)) 
do 
  r=`echo "scale=8; 1024 + 2048 " | bc -l`; 
  z=`echo "scale=8;  256 +  640 * s( (        $i ) * ${pi} / ( 2 * ${d} ) )" | bc -l`;   
  x=`echo "scale=8;  512 - $r * c( 4 * ${pi} * ${i} / ${d} )" | bc -l`; 
  y=`echo "scale=8;  512 - $r * s( 4 * ${pi} * ${i} / ${d} )" | bc -l`; 
  printf "%4d  %15.8f %15.8f %15.8f      %12.8f %12.8f %12.8f     %12.8f %12.8f %12.8f \n"  $i    $x $z $y    512 $z 512  0 1 0; 
done  > redandback_dynamic.txt 
for((i=0;i<${d};i++)) 
do 
  b=`echo "$d + $i " | bc `;
  r=`echo "scale=8; 1024 + 2048 " | bc -l`; 
  z=`echo "scale=8;  256 +  640 * s( ( ${d} - $i ) * ${pi} / ( 2 * ${d} ) )" | bc -l`; 
  x=`echo "scale=8;  512 - $r * c( 4 * ${pi} * ${i} / ${d} )" | bc -l`; 
  y=`echo "scale=8;  512 - $r * s( 4 * ${pi} * ${i} / ${d} )" | bc -l`;   
  printf "%4d  %15.8f %15.8f %15.8f      %12.8f %12.8f %12.8f     %12.8f %12.8f %12.8f \n"  $b    $x $z $y    512 $z 512  0 1 0; 
done  >> redandback_dynamic.txt 

for((i=0;i<${d};i++)) 
do 
  b=`echo "$d + $i " | bc `;
  r=`echo "scale=8; 1024 + 2048 * s( (        $i ) * ${pi} / ( 2 * ${d} ) )" | bc -l`; 
  z=`echo "scale=8;  256 +  640 * s( ( ${d} - $i ) * ${pi} / ( 2 * ${d} ) )" | bc -l`; 
  x=`echo "scale=8;  512 - $r * c( 4 * ${pi} * ${i} / ${d} )" | bc -l`; 
  y=`echo "scale=8;  512 - $r * s( 4 * ${pi} * ${i} / ${d} )" | bc -l`;   
  printf "%4d  %15.8f %15.8f %15.8f      %12.8f %12.8f %12.8f     %12.8f %12.8f %12.8f \n"  $b    $x $z $y    512 $z 512  0 1 0; 
done  >> longdress_static.txt 


#### Circle 
d=300;
for((i=0;i<${d};i++)) 
do 
  x=`echo "scale=8; 512 - 2048 * c( 2 * ${pi} * ${i} / ${d} )" | bc -l`; 
  y=`echo "scale=8; 512 - 2048 * s( 2 * ${pi} * ${i} / ${d} )" | bc -l`; 
  printf "%4d  %15.8f %15.8f %15.8f      %12.8f %12.8f %12.8f     %12.8f %12.8f %12.8f \n"  $i    $x 512 $y    512 512 512  0 1 0; 
done  > circle.txt 

