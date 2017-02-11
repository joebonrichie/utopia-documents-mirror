/*****************************************************************************
 *  
 *   This file is part of the Utopia Documents application.
 *       Copyright (c) 2008-2017 Lost Island Labs
 *           <info@utopiadocs.com>
 *   
 *   Utopia Documents is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU GENERAL PUBLIC LICENSE VERSION 3 as
 *   published by the Free Software Foundation.
 *   
 *   Utopia Documents is distributed in the hope that it will be useful, but
 *   WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
 *   Public License for more details.
 *   
 *   In addition, as a special exception, the copyright holders give
 *   permission to link the code of portions of this program with the OpenSSL
 *   library under certain conditions as described in each individual source
 *   file, and distribute linked combinations including the two.
 *   
 *   You must obey the GNU General Public License in all respects for all of
 *   the code used other than OpenSSL. If you modify file(s) with this
 *   exception, you may extend this exception to your version of the file(s),
 *   but you are not obligated to do so. If you do not wish to do so, delete
 *   this exception statement from your version.
 *   
 *   You should have received a copy of the GNU General Public License
 *   along with Utopia Documents. If not, see <http://www.gnu.org/licenses/>
 *  
 *****************************************************************************/

/**  Get 1st component of vector.  */
inline component_type x() const
{ return this->_x; }

/**  Set 1st component of vector to x_.  */
inline void x(const component_type & x_)
{ this->_x = x_; }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 2 > xx() const
{ return vector< component_type, 2 >(this->_x, this->_x); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 3 > xxx() const
{ return vector< component_type, 3 >(this->_x, this->_x, this->_x); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > xxxx() const
{ return vector< component_type, 4 >(this->_x, this->_x, this->_x, this->_x); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > xxxy() const
{ return vector< component_type, 4 >(this->_x, this->_x, this->_x, this->_y); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 3 > xxy() const
{ return vector< component_type, 3 >(this->_x, this->_x, this->_y); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > xxyx() const
{ return vector< component_type, 4 >(this->_x, this->_x, this->_y, this->_x); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > xxyy() const
{ return vector< component_type, 4 >(this->_x, this->_x, this->_y, this->_y); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 2 > xy() const
{ return vector< component_type, 2 >(this->_x, this->_y); }

/**  Set multiple components of vector.  */
inline void xy(const component_type & x_, const component_type & y_)
{ this->_x = x_; this->_y = y_; }

/**  Set multiple components of vector.  */
inline void xy(const vector< component_type, 2 > & xy_)
{ this->_x = xy_.x(); this->_y = xy_.y(); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 3 > xyx() const
{ return vector< component_type, 3 >(this->_x, this->_y, this->_x); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > xyxx() const
{ return vector< component_type, 4 >(this->_x, this->_y, this->_x, this->_x); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > xyxy() const
{ return vector< component_type, 4 >(this->_x, this->_y, this->_x, this->_y); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 3 > xyy() const
{ return vector< component_type, 3 >(this->_x, this->_y, this->_y); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > xyyx() const
{ return vector< component_type, 4 >(this->_x, this->_y, this->_y, this->_x); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > xyyy() const
{ return vector< component_type, 4 >(this->_x, this->_y, this->_y, this->_y); }

/**  Get 2nd component of vector.  */
inline component_type y() const
{ return this->_y; }

/**  Set 2nd component of vector to y_.  */
inline void y(const component_type & y_)
{ this->_y = y_; }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 2 > yx() const
{ return vector< component_type, 2 >(this->_y, this->_x); }

/**  Set multiple components of vector.  */
inline void yx(const component_type & y_, const component_type & x_)
{ this->_y = y_; this->_x = x_; }

/**  Set multiple components of vector.  */
inline void yx(const vector< component_type, 2 > & yx_)
{ this->_y = yx_.y(); this->_x = yx_.x(); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 3 > yxx() const
{ return vector< component_type, 3 >(this->_y, this->_x, this->_x); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > yxxx() const
{ return vector< component_type, 4 >(this->_y, this->_x, this->_x, this->_x); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > yxxy() const
{ return vector< component_type, 4 >(this->_y, this->_x, this->_x, this->_y); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 3 > yxy() const
{ return vector< component_type, 3 >(this->_y, this->_x, this->_y); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > yxyx() const
{ return vector< component_type, 4 >(this->_y, this->_x, this->_y, this->_x); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > yxyy() const
{ return vector< component_type, 4 >(this->_y, this->_x, this->_y, this->_y); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 2 > yy() const
{ return vector< component_type, 2 >(this->_y, this->_y); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 3 > yyx() const
{ return vector< component_type, 3 >(this->_y, this->_y, this->_x); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > yyxx() const
{ return vector< component_type, 4 >(this->_y, this->_y, this->_x, this->_x); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > yyxy() const
{ return vector< component_type, 4 >(this->_y, this->_y, this->_x, this->_y); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 3 > yyy() const
{ return vector< component_type, 3 >(this->_y, this->_y, this->_y); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > yyyx() const
{ return vector< component_type, 4 >(this->_y, this->_y, this->_y, this->_x); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > yyyy() const
{ return vector< component_type, 4 >(this->_y, this->_y, this->_y, this->_y); }
