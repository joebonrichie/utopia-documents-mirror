#!/bin/bash

###############################################################################
#   
#    This file is part of the Utopia Documents application.
#        Copyright (c) 2008-2017 Lost Island Labs
#            <info@utopiadocs.com>
#    
#    Utopia Documents is free software: you can redistribute it and/or modify
#    it under the terms of the GNU GENERAL PUBLIC LICENSE VERSION 3 as
#    published by the Free Software Foundation.
#    
#    Utopia Documents is distributed in the hope that it will be useful, but
#    WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
#    Public License for more details.
#    
#    In addition, as a special exception, the copyright holders give
#    permission to link the code of portions of this program with the OpenSSL
#    library under certain conditions as described in each individual source
#    file, and distribute linked combinations including the two.
#    
#    You must obey the GNU General Public License in all respects for all of
#    the code used other than OpenSSL. If you modify file(s) with this
#    exception, you may extend this exception to your version of the file(s),
#    but you are not obligated to do so. If you do not wish to do so, delete
#    this exception statement from your version.
#    
#    You should have received a copy of the GNU General Public License
#    along with Utopia Documents. If not, see <http://www.gnu.org/licenses/>
#   
###############################################################################


cp -f license_header xy_swizzles.h
cp -f license_header xyz_swizzles.h
cp -f license_header xyzw_swizzles.h
cp -f license_header rgb_swizzles.h
cp -f license_header rgba_swizzles.h

for i in 1 2 3 4; do
  colour_filename=rgb_swizzles.h
  vector_filename=xy_swizzles.h
  if [ `expr index $i 4` -ne 0 ]; then colour_filename=rgba_swizzles.h; fi
  if [ `expr index $i 4` -ne 0 ]; then vector_filename=xyzw_swizzles.h;
  elif [ `expr index $i 3` -ne 0 ]; then vector_filename=xyz_swizzles.h; fi
  
  x=`expr substr xyzw $i 1`;
  r=`expr substr rgba $i 1`;
  c=`expr substr stndrdth \`echo "(${i}*2)-1" | bc\` 2`;
  
  echo "/**  Get $i$c component of vector.  */" >> $vector_filename;
  echo "inline element_type $x() const"$'\n'"{ return this->_$x; }"$'\n' >> $vector_filename;
  echo "/**  Set $i$c component of vector to ${x}_.  */" >> $vector_filename;
  echo "inline void $x(const element_type & ${x}_)"$'\n'"{ this->_$x = ${x}_; }"$'\n' >> $vector_filename;
  echo "/**  Get $i$c component of vector.  */" >> $colour_filename;
  echo "inline element_type $r() const"$'\n'"{ return this->_$x; }"$'\n' >> $colour_filename;
  echo "/**  Set $i$c component of vector to ${r}_.  */" >> $colour_filename;
  echo "inline void $r(const element_type & ${r}_)"$'\n'"{ this->_$x = ${r}_; }"$'\n' >> $colour_filename;

  for j in 1 2 3 4; do
    colour_filename=rgb_swizzles.h
    vector_filename=xy_swizzles.h
    if [ `expr index $i$j 4` -ne 0 ]; then colour_filename=rgba_swizzles.h; fi
    if [ `expr index $i$j 4` -ne 0 ]; then vector_filename=xyzw_swizzles.h;
    elif [ `expr index $i$j 3` -ne 0 ]; then vector_filename=xyz_swizzles.h; fi
    
    y=`expr substr xyzw $j 1`;
    g=`expr substr rgba $j 1`;
    
    echo "/**  Get multiple components of vector as a new vector.  */" >> $vector_filename;
    echo "inline vector< element_type, 2 > $x$y() const"$'\n'"{ return vector< element_type, 2 >(this->_$x, this->_$y); }"$'\n' >> $vector_filename;
    echo "/**  Get multiple components of vector as a new vector.  */" >> $colour_filename;
    echo "inline vector< element_type, 2 > $r$g() const"$'\n'"{ return vector< element_type, 2 >(this->_$x, this->_$y); }"$'\n' >> $colour_filename;
    
    if [ "$x" != "$y" ]; then
      echo "/**  Set multiple components of vector.  */" >> $vector_filename;
      echo "inline void $x$y(const element_type & ${x}_, const element_type & ${y}_)"$'\n'"{ this->_$x = ${x}_; this->_$y = ${y}_; }"$'\n' >> $vector_filename;
      echo "/**  Set multiple components of vector.  */" >> $vector_filename;
      echo "inline void $x$y(const vector< element_type, 2 > & $x${y}_)"$'\n'"{ this->_$x = $x${y}_.$x(); this->_$y = $x${y}_.$y(); }"$'\n' >> $vector_filename;
      
      echo "/**  Set multiple components of vector.  */" >> $colour_filename;
      echo "inline void $r$g(const element_type & ${r}_, const element_type & ${g}_)"$'\n'"{ this->_$x = ${r}_; this->_$y = ${g}_; }"$'\n' >> $colour_filename;
      echo "/**  Set multiple components of vector.  */" >> $colour_filename;
      echo "inline void $r$g(const vector< element_type, 2 > & $r${g}_)"$'\n'"{ this->_$x = $r${g}_.$r(); this->_$y = $r${g}_.$g(); }"$'\n' >> $colour_filename;
    fi

    for k in 1 2 3 4; do
      colour_filename=rgb_swizzles.h
      vector_filename=xy_swizzles.h
      if [ `expr index $i$j$k 4` -ne 0 ]; then colour_filename=rgba_swizzles.h; fi
      if [ `expr index $i$j$k 4` -ne 0 ]; then vector_filename=xyzw_swizzles.h;
      elif [ `expr index $i$j$k 3` -ne 0 ]; then vector_filename=xyz_swizzles.h; fi
      
      z=`expr substr xyzw $k 1`;
      b=`expr substr rgba $k 1`;
      
      echo "/**  Get multiple components of vector as a new vector.  */" >> $vector_filename;
      echo "inline vector< element_type, 3 > $x$y$z() const"$'\n'"{ return vector< element_type, 3 >(this->_$x, this->_$y, this->_$z); }"$'\n' >> $vector_filename;
      echo "/**  Get multiple components of vector as a new vector.  */" >> $colour_filename;
      echo "inline vector< element_type, 3 > $r$g$b() const"$'\n'"{ return vector< element_type, 3 >(this->_$x, this->_$y, this->_$z); }"$'\n' >> $colour_filename;
      
      if [ `echo $x$'\n'$y$'\n'$z | sort | uniq | wc -l` -eq 3 ]; then
        echo "/**  Set multiple components of vector as a new vector.  */" >> $vector_filename;
        echo "inline void $x$y$z(const element_type & ${x}_, const element_type & ${y}_, const element_type & ${z}_)"$'\n'"{ this->_$x = ${x}_; this->_$y = ${y}_; this->_$z = ${z}_; }"$'\n' >> $vector_filename;
        echo "/**  Set multiple components of vector as a new vector.  */" >> $vector_filename;
        echo "inline void $x$y$z(const element_type & ${x}_, const vector< element_type, 2 > & $y${z}_)"$'\n'"{ this->_$x = ${x}_; this->_$y = $y${z}_.$y(); this->_$z = $y${z}_.$z(); }"$'\n' >> $vector_filename;
        echo "/**  Set multiple components of vector as a new vector.  */" >> $vector_filename;
        echo "inline void $x$y$z(const vector< element_type, 2 > & $x${y}_, const element_type & ${z}_)"$'\n'"{ this->_$x = $x${y}_.$x(); this->_$y = $x${y}_.$y(); this->_$z = ${z}_; }"$'\n' >> $vector_filename;
        echo "/**  Set multiple components of vector as a new vector.  */" >> $vector_filename;
        echo "inline void $x$y$z(const vector< element_type, 3 > & $x$y${z}_)"$'\n'"{ this->_$x = $x$y${z}_.$x(); this->_$y = $x$y${z}_.$y(); this->_$z = $x$y${z}_.$z(); }"$'\n' >> $vector_filename;
        
        echo "/**  Set multiple components of vector as a new vector.  */" >> $colour_filename;
        echo "inline void $r$g$b(const element_type & ${r}_, const element_type & ${g}_, const element_type & ${b}_)"$'\n'"{ this->_$x = ${r}_; this->_$y = ${g}_; this->_$z = ${b}_; }"$'\n' >> $colour_filename;
        echo "/**  Set multiple components of vector as a new vector.  */" >> $colour_filename;
        echo "inline void $r$g$b(const element_type & ${r}_, const vector< element_type, 2 > & $g${b}_)"$'\n'"{ this->_$x = ${r}_; this->_$y = $g${b}_.$g(); this->_$z = $g${b}_.$b(); }"$'\n' >> $colour_filename;
        echo "/**  Set multiple components of vector as a new vector.  */" >> $colour_filename;
        echo "inline void $r$g$b(const vector< element_type, 2 > & $r${g}_, const element_type & ${b}_)"$'\n'"{ this->_$x = $r${g}_.$r(); this->_$y = $r${g}_.$g(); this->_$z = ${b}_; }"$'\n' >> $colour_filename;
        echo "/**  Set multiple components of vector as a new vector.  */" >> $colour_filename;
        echo "inline void $r$g$b(const vector< element_type, 3 > & $r$g${b}_)"$'\n'"{ this->_$x = $r$g${b}_.$r(); this->_$y = $r$g${b}_.$g(); this->_$z = $r$g${b}_.$b(); }"$'\n' >> $colour_filename;
      fi

      for l in 1 2 3 4; do
        colour_filename=rgb_swizzles.h
        vector_filename=xy_swizzles.h
        if [ `expr index $i$j$k$l 4` -ne 0 ]; then colour_filename=rgba_swizzles.h; fi
        if [ `expr index $i$j$k$l 4` -ne 0 ]; then vector_filename=xyzw_swizzles.h;
        elif [ `expr index $i$j$k$l 3` -ne 0 ]; then vector_filename=xyz_swizzles.h; fi
      
        w=`expr substr xyzw $l 1`;
        a=`expr substr rgba $l 1`;
        
        echo "/**  Get multiple components of vector as a new vector.  */" >> $vector_filename;
        echo "inline vector< element_type, 4 > $x$y$z$w() const"$'\n'"{ return vector< element_type, 4 >(this->_$x, this->_$y, this->_$z, this->_$w); }"$'\n' >> $vector_filename;
        echo "/**  Get multiple components of vector as a new vector.  */" >> $colour_filename;
        echo "inline vector< element_type, 4 > $r$g$b$a() const"$'\n'"{ return vector< element_type, 4 >(this->_$x, this->_$y, this->_$z, this->_$w); }"$'\n' >> $colour_filename;

        if [ `echo $x$'\n'$y$'\n'$z$'\n'$w | sort | uniq | wc -l` -eq 4 ]; then
          echo "/**  Set multiple components of vector as a new vector.  */" >> $vector_filename;
          echo "inline void $x$y$z$w(const element_type & ${x}_, const element_type & ${y}_, const element_type & ${z}_, const element_type & ${w}_)"$'\n'"{ this->_$x = ${x}_; this->_$y = ${y}_; this->_$z = ${z}_; this->_$w = ${w}_; }"$'\n' >> $vector_filename;
          echo "/**  Set multiple components of vector as a new vector.  */" >> $vector_filename;
          echo "inline void $x$y$z$w(const element_type & ${x}_, const vector< element_type, 3 > & $y$z${w}_)"$'\n'"{ this->_$x = ${x}_; this->_$y = $y$z${w}_.$y(); this->_$z = $y$z${w}_.$z(); this->_$w = $y$z${w}_.$w(); }"$'\n' >> $vector_filename;
          echo "/**  Set multiple components of vector as a new vector.  */" >> $vector_filename;
          echo "inline void $x$y$z$w(const vector< element_type, 3 > & $x$y${z}_, const element_type & ${w}_)"$'\n'"{ this->_$x = $x$y${z}_.$x(); this->_$y = $x$y${z}_.$y(); this->_$z = $x$y${z}_.$z(); this->_$w = ${w}_; }"$'\n' >> $vector_filename;
          echo "/**  Set multiple components of vector as a new vector.  */" >> $vector_filename;
          echo "inline void $x$y$z$w(const element_type & ${x}_, const element_type & ${y}_, const vector< element_type, 2 > & $z${w}_)"$'\n'"{ this->_$x = ${x}_; this->_$y = ${y}_; this->_$z = $z${w}_.$z(); this->_$w = $z${w}_.$w(); }"$'\n' >> $vector_filename;
          echo "/**  Set multiple components of vector as a new vector.  */" >> $vector_filename;
          echo "inline void $x$y$z$w(const element_type & ${x}_, const vector< element_type, 2 > & $y${z}_, const element_type & ${w}_)"$'\n'"{ this->_$x = ${x}_; this->_$y = $y${z}_.$y(); this->_$z = $y${z}_.$z(); this->_$w = ${w}_; }"$'\n' >> $vector_filename;
          echo "/**  Set multiple components of vector as a new vector.  */" >> $vector_filename;
          echo "inline void $x$y$z$w(const vector< element_type, 2 > & $x${y}_, const element_type & ${z}_, const element_type & ${w}_)"$'\n'"{ this->_$x = $x${y}_.$x(); this->_$y = $x${y}_.$y(); this->_$z = ${z}_; this->_$w = ${w}_; }"$'\n' >> $vector_filename;
          echo "/**  Set multiple components of vector as a new vector.  */" >> $vector_filename;
          echo "inline void $x$y$z$w(const vector< element_type, 2 > & $x${y}_, const vector< element_type, 2 > & $z${w}_)"$'\n'"{ this->_$x = $x${y}_.$x(); this->_$y = $x${y}_.$y(); this->_$z = $z${w}_.$z(); this->_$w = $z${w}_.$w(); }"$'\n' >> $vector_filename;
          echo "/**  Set multiple components of vector as a new vector.  */" >> $vector_filename;
          echo "inline void $x$y$z$w(const vector< element_type, 4 > & $x$y$z${w}_)"$'\n'"{ this->_$x = $x$y$z${w}_.$x(); this->_$y = $x$y$z${w}_.$y(); this->_$z = $x$y$z${w}_.$z(); this->_$w = $x$y$z${w}_.$w(); }"$'\n' >> $vector_filename;
          
          echo "/**  Set multiple components of vector as a new vector.  */" >> $colour_filename;
          echo "inline void $r$g$b$a(const element_type & ${r}_, const element_type & ${g}_, const element_type & ${b}_, const element_type & ${a}_)"$'\n'"{ this->_$x = ${r}_; this->_$y = ${g}_; this->_$z = ${b}_; this->_$w = ${a}_; }"$'\n' >> $colour_filename;
          echo "/**  Set multiple components of vector as a new vector.  */" >> $colour_filename;
          echo "inline void $r$g$b$a(const element_type & ${r}_, const vector< element_type, 3 > & $g$b${a}_)"$'\n'"{ this->_$x = ${r}_; this->_$y = $g$b${a}_.$g(); this->_$z = $g$b${a}_.$b(); this->_$w = $g$b${a}_.$a(); }"$'\n' >> $colour_filename;
          echo "/**  Set multiple components of vector as a new vector.  */" >> $colour_filename;
          echo "inline void $r$g$b$a(const vector< element_type, 3 > & $r$g${b}_, const element_type & ${a}_)"$'\n'"{ this->_$x = $r$g${b}_.$r(); this->_$y = $r$g${b}_.$g(); this->_$z = $r$g${b}_.$b(); this->_$w = ${a}_; }"$'\n' >> $colour_filename;
          echo "/**  Set multiple components of vector as a new vector.  */" >> $colour_filename;
          echo "inline void $r$g$b$a(const element_type & ${r}_, const element_type & ${g}_, const vector< element_type, 2 > & $b${a}_)"$'\n'"{ this->_$x = ${r}_; this->_$y = ${g}_; this->_$z = $b${a}_.$b(); this->_$w = $b${a}_.$a(); }"$'\n' >> $colour_filename;
          echo "/**  Set multiple components of vector as a new vector.  */" >> $colour_filename;
          echo "inline void $r$g$b$a(const element_type & ${r}_, const vector< element_type, 2 > & $g${b}_, const element_type & ${a}_)"$'\n'"{ this->_$x = ${r}_; this->_$y = $g${b}_.$g(); this->_$z = $g${b}_.$b(); this->_$w = ${a}_; }"$'\n' >> $colour_filename;
          echo "/**  Set multiple components of vector as a new vector.  */" >> $colour_filename;
          echo "inline void $r$g$b$a(const vector< element_type, 2 > & $r${g}_, const element_type & ${b}_, const element_type & ${a}_)"$'\n'"{ this->_$x = $r${g}_.$r(); this->_$y = $r${g}_.$g(); this->_$z = ${b}_; this->_$w = ${a}_; }"$'\n' >> $colour_filename;
          echo "/**  Set multiple components of vector as a new vector.  */" >> $colour_filename;
          echo "inline void $r$g$b$a(const vector< element_type, 2 > & $r${g}_, const vector< element_type, 2 > & $b${a}_)"$'\n'"{ this->_$x = $r${g}_.$r(); this->_$y = $r${g}_.$g(); this->_$z = $b${a}_.$b(); this->_$w = $b${a}_.$a(); }"$'\n' >> $colour_filename;
          echo "/**  Set multiple components of vector as a new vector.  */" >> $colour_filename;
          echo "inline void $r$g$b$a(const vector< element_type, 4 > & $r$g$b${a}_)"$'\n'"{ this->_$x = $r$g$b${a}_.$r(); this->_$y = $r$g$b${a}_.$g(); this->_$z = $r$g$b${a}_.$b(); this->_$w = $r$g$b${a}_.$a(); }"$'\n' >> $colour_filename;
        fi

      done;
    done;
  done;
done
