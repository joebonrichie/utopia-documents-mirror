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

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > xxxw() const
{ return vector< component_type, 4 >(this->_x, this->_x, this->_x, this->_w); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > xxyw() const
{ return vector< component_type, 4 >(this->_x, this->_x, this->_y, this->_w); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > xxzw() const
{ return vector< component_type, 4 >(this->_x, this->_x, this->_z, this->_w); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 3 > xxw() const
{ return vector< component_type, 3 >(this->_x, this->_x, this->_w); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > xxwx() const
{ return vector< component_type, 4 >(this->_x, this->_x, this->_w, this->_x); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > xxwy() const
{ return vector< component_type, 4 >(this->_x, this->_x, this->_w, this->_y); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > xxwz() const
{ return vector< component_type, 4 >(this->_x, this->_x, this->_w, this->_z); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > xxww() const
{ return vector< component_type, 4 >(this->_x, this->_x, this->_w, this->_w); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > xyxw() const
{ return vector< component_type, 4 >(this->_x, this->_y, this->_x, this->_w); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > xyyw() const
{ return vector< component_type, 4 >(this->_x, this->_y, this->_y, this->_w); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > xyzw() const
{ return vector< component_type, 4 >(this->_x, this->_y, this->_z, this->_w); }

/**  Set multiple components of vector as a new vector.  */
inline void xyzw(const component_type & x_, const component_type & y_, const component_type & z_, const component_type & w_)
{ this->_x = x_; this->_y = y_; this->_z = z_; this->_w = w_; }

/**  Set multiple components of vector as a new vector.  */
inline void xyzw(const component_type & x_, const vector< component_type, 3 > & yzw_)
{ this->_x = x_; this->_y = yzw_.y(); this->_z = yzw_.z(); this->_w = yzw_.w(); }

/**  Set multiple components of vector as a new vector.  */
inline void xyzw(const vector< component_type, 3 > & xyz_, const component_type & w_)
{ this->_x = xyz_.x(); this->_y = xyz_.y(); this->_z = xyz_.z(); this->_w = w_; }

/**  Set multiple components of vector as a new vector.  */
inline void xyzw(const component_type & x_, const component_type & y_, const vector< component_type, 2 > & zw_)
{ this->_x = x_; this->_y = y_; this->_z = zw_.z(); this->_w = zw_.w(); }

/**  Set multiple components of vector as a new vector.  */
inline void xyzw(const component_type & x_, const vector< component_type, 2 > & yz_, const component_type & w_)
{ this->_x = x_; this->_y = yz_.y(); this->_z = yz_.z(); this->_w = w_; }

/**  Set multiple components of vector as a new vector.  */
inline void xyzw(const vector< component_type, 2 > & xy_, const component_type & z_, const component_type & w_)
{ this->_x = xy_.x(); this->_y = xy_.y(); this->_z = z_; this->_w = w_; }

/**  Set multiple components of vector as a new vector.  */
inline void xyzw(const vector< component_type, 2 > & xy_, const vector< component_type, 2 > & zw_)
{ this->_x = xy_.x(); this->_y = xy_.y(); this->_z = zw_.z(); this->_w = zw_.w(); }

/**  Set multiple components of vector as a new vector.  */
inline void xyzw(const vector< component_type, 4 > & xyzw_)
{ this->_x = xyzw_.x(); this->_y = xyzw_.y(); this->_z = xyzw_.z(); this->_w = xyzw_.w(); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 3 > xyw() const
{ return vector< component_type, 3 >(this->_x, this->_y, this->_w); }

/**  Set multiple components of vector as a new vector.  */
inline void xyw(const component_type & x_, const component_type & y_, const component_type & w_)
{ this->_x = x_; this->_y = y_; this->_w = w_; }

/**  Set multiple components of vector as a new vector.  */
inline void xyw(const component_type & x_, const vector< component_type, 2 > & yw_)
{ this->_x = x_; this->_y = yw_.y(); this->_w = yw_.w(); }

/**  Set multiple components of vector as a new vector.  */
inline void xyw(const vector< component_type, 2 > & xy_, const component_type & w_)
{ this->_x = xy_.x(); this->_y = xy_.y(); this->_w = w_; }

/**  Set multiple components of vector as a new vector.  */
inline void xyw(const vector< component_type, 3 > & xyw_)
{ this->_x = xyw_.x(); this->_y = xyw_.y(); this->_w = xyw_.w(); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > xywx() const
{ return vector< component_type, 4 >(this->_x, this->_y, this->_w, this->_x); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > xywy() const
{ return vector< component_type, 4 >(this->_x, this->_y, this->_w, this->_y); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > xywz() const
{ return vector< component_type, 4 >(this->_x, this->_y, this->_w, this->_z); }

/**  Set multiple components of vector as a new vector.  */
inline void xywz(const component_type & x_, const component_type & y_, const component_type & w_, const component_type & z_)
{ this->_x = x_; this->_y = y_; this->_w = w_; this->_z = z_; }

/**  Set multiple components of vector as a new vector.  */
inline void xywz(const component_type & x_, const vector< component_type, 3 > & ywz_)
{ this->_x = x_; this->_y = ywz_.y(); this->_w = ywz_.w(); this->_z = ywz_.z(); }

/**  Set multiple components of vector as a new vector.  */
inline void xywz(const vector< component_type, 3 > & xyw_, const component_type & z_)
{ this->_x = xyw_.x(); this->_y = xyw_.y(); this->_w = xyw_.w(); this->_z = z_; }

/**  Set multiple components of vector as a new vector.  */
inline void xywz(const component_type & x_, const component_type & y_, const vector< component_type, 2 > & wz_)
{ this->_x = x_; this->_y = y_; this->_w = wz_.w(); this->_z = wz_.z(); }

/**  Set multiple components of vector as a new vector.  */
inline void xywz(const component_type & x_, const vector< component_type, 2 > & yw_, const component_type & z_)
{ this->_x = x_; this->_y = yw_.y(); this->_w = yw_.w(); this->_z = z_; }

/**  Set multiple components of vector as a new vector.  */
inline void xywz(const vector< component_type, 2 > & xy_, const component_type & w_, const component_type & z_)
{ this->_x = xy_.x(); this->_y = xy_.y(); this->_w = w_; this->_z = z_; }

/**  Set multiple components of vector as a new vector.  */
inline void xywz(const vector< component_type, 2 > & xy_, const vector< component_type, 2 > & wz_)
{ this->_x = xy_.x(); this->_y = xy_.y(); this->_w = wz_.w(); this->_z = wz_.z(); }

/**  Set multiple components of vector as a new vector.  */
inline void xywz(const vector< component_type, 4 > & xywz_)
{ this->_x = xywz_.x(); this->_y = xywz_.y(); this->_w = xywz_.w(); this->_z = xywz_.z(); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > xyww() const
{ return vector< component_type, 4 >(this->_x, this->_y, this->_w, this->_w); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > xzxw() const
{ return vector< component_type, 4 >(this->_x, this->_z, this->_x, this->_w); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > xzyw() const
{ return vector< component_type, 4 >(this->_x, this->_z, this->_y, this->_w); }

/**  Set multiple components of vector as a new vector.  */
inline void xzyw(const component_type & x_, const component_type & z_, const component_type & y_, const component_type & w_)
{ this->_x = x_; this->_z = z_; this->_y = y_; this->_w = w_; }

/**  Set multiple components of vector as a new vector.  */
inline void xzyw(const component_type & x_, const vector< component_type, 3 > & zyw_)
{ this->_x = x_; this->_z = zyw_.z(); this->_y = zyw_.y(); this->_w = zyw_.w(); }

/**  Set multiple components of vector as a new vector.  */
inline void xzyw(const vector< component_type, 3 > & xzy_, const component_type & w_)
{ this->_x = xzy_.x(); this->_z = xzy_.z(); this->_y = xzy_.y(); this->_w = w_; }

/**  Set multiple components of vector as a new vector.  */
inline void xzyw(const component_type & x_, const component_type & z_, const vector< component_type, 2 > & yw_)
{ this->_x = x_; this->_z = z_; this->_y = yw_.y(); this->_w = yw_.w(); }

/**  Set multiple components of vector as a new vector.  */
inline void xzyw(const component_type & x_, const vector< component_type, 2 > & zy_, const component_type & w_)
{ this->_x = x_; this->_z = zy_.z(); this->_y = zy_.y(); this->_w = w_; }

/**  Set multiple components of vector as a new vector.  */
inline void xzyw(const vector< component_type, 2 > & xz_, const component_type & y_, const component_type & w_)
{ this->_x = xz_.x(); this->_z = xz_.z(); this->_y = y_; this->_w = w_; }

/**  Set multiple components of vector as a new vector.  */
inline void xzyw(const vector< component_type, 2 > & xz_, const vector< component_type, 2 > & yw_)
{ this->_x = xz_.x(); this->_z = xz_.z(); this->_y = yw_.y(); this->_w = yw_.w(); }

/**  Set multiple components of vector as a new vector.  */
inline void xzyw(const vector< component_type, 4 > & xzyw_)
{ this->_x = xzyw_.x(); this->_z = xzyw_.z(); this->_y = xzyw_.y(); this->_w = xzyw_.w(); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > xzzw() const
{ return vector< component_type, 4 >(this->_x, this->_z, this->_z, this->_w); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 3 > xzw() const
{ return vector< component_type, 3 >(this->_x, this->_z, this->_w); }

/**  Set multiple components of vector as a new vector.  */
inline void xzw(const component_type & x_, const component_type & z_, const component_type & w_)
{ this->_x = x_; this->_z = z_; this->_w = w_; }

/**  Set multiple components of vector as a new vector.  */
inline void xzw(const component_type & x_, const vector< component_type, 2 > & zw_)
{ this->_x = x_; this->_z = zw_.z(); this->_w = zw_.w(); }

/**  Set multiple components of vector as a new vector.  */
inline void xzw(const vector< component_type, 2 > & xz_, const component_type & w_)
{ this->_x = xz_.x(); this->_z = xz_.z(); this->_w = w_; }

/**  Set multiple components of vector as a new vector.  */
inline void xzw(const vector< component_type, 3 > & xzw_)
{ this->_x = xzw_.x(); this->_z = xzw_.z(); this->_w = xzw_.w(); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > xzwx() const
{ return vector< component_type, 4 >(this->_x, this->_z, this->_w, this->_x); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > xzwy() const
{ return vector< component_type, 4 >(this->_x, this->_z, this->_w, this->_y); }

/**  Set multiple components of vector as a new vector.  */
inline void xzwy(const component_type & x_, const component_type & z_, const component_type & w_, const component_type & y_)
{ this->_x = x_; this->_z = z_; this->_w = w_; this->_y = y_; }

/**  Set multiple components of vector as a new vector.  */
inline void xzwy(const component_type & x_, const vector< component_type, 3 > & zwy_)
{ this->_x = x_; this->_z = zwy_.z(); this->_w = zwy_.w(); this->_y = zwy_.y(); }

/**  Set multiple components of vector as a new vector.  */
inline void xzwy(const vector< component_type, 3 > & xzw_, const component_type & y_)
{ this->_x = xzw_.x(); this->_z = xzw_.z(); this->_w = xzw_.w(); this->_y = y_; }

/**  Set multiple components of vector as a new vector.  */
inline void xzwy(const component_type & x_, const component_type & z_, const vector< component_type, 2 > & wy_)
{ this->_x = x_; this->_z = z_; this->_w = wy_.w(); this->_y = wy_.y(); }

/**  Set multiple components of vector as a new vector.  */
inline void xzwy(const component_type & x_, const vector< component_type, 2 > & zw_, const component_type & y_)
{ this->_x = x_; this->_z = zw_.z(); this->_w = zw_.w(); this->_y = y_; }

/**  Set multiple components of vector as a new vector.  */
inline void xzwy(const vector< component_type, 2 > & xz_, const component_type & w_, const component_type & y_)
{ this->_x = xz_.x(); this->_z = xz_.z(); this->_w = w_; this->_y = y_; }

/**  Set multiple components of vector as a new vector.  */
inline void xzwy(const vector< component_type, 2 > & xz_, const vector< component_type, 2 > & wy_)
{ this->_x = xz_.x(); this->_z = xz_.z(); this->_w = wy_.w(); this->_y = wy_.y(); }

/**  Set multiple components of vector as a new vector.  */
inline void xzwy(const vector< component_type, 4 > & xzwy_)
{ this->_x = xzwy_.x(); this->_z = xzwy_.z(); this->_w = xzwy_.w(); this->_y = xzwy_.y(); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > xzwz() const
{ return vector< component_type, 4 >(this->_x, this->_z, this->_w, this->_z); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > xzww() const
{ return vector< component_type, 4 >(this->_x, this->_z, this->_w, this->_w); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 2 > xw() const
{ return vector< component_type, 2 >(this->_x, this->_w); }

/**  Set multiple components of vector.  */
inline void xw(const component_type & x_, const component_type & w_)
{ this->_x = x_; this->_w = w_; }

/**  Set multiple components of vector.  */
inline void xw(const vector< component_type, 2 > & xw_)
{ this->_x = xw_.x(); this->_w = xw_.w(); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 3 > xwx() const
{ return vector< component_type, 3 >(this->_x, this->_w, this->_x); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > xwxx() const
{ return vector< component_type, 4 >(this->_x, this->_w, this->_x, this->_x); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > xwxy() const
{ return vector< component_type, 4 >(this->_x, this->_w, this->_x, this->_y); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > xwxz() const
{ return vector< component_type, 4 >(this->_x, this->_w, this->_x, this->_z); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > xwxw() const
{ return vector< component_type, 4 >(this->_x, this->_w, this->_x, this->_w); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 3 > xwy() const
{ return vector< component_type, 3 >(this->_x, this->_w, this->_y); }

/**  Set multiple components of vector as a new vector.  */
inline void xwy(const component_type & x_, const component_type & w_, const component_type & y_)
{ this->_x = x_; this->_w = w_; this->_y = y_; }

/**  Set multiple components of vector as a new vector.  */
inline void xwy(const component_type & x_, const vector< component_type, 2 > & wy_)
{ this->_x = x_; this->_w = wy_.w(); this->_y = wy_.y(); }

/**  Set multiple components of vector as a new vector.  */
inline void xwy(const vector< component_type, 2 > & xw_, const component_type & y_)
{ this->_x = xw_.x(); this->_w = xw_.w(); this->_y = y_; }

/**  Set multiple components of vector as a new vector.  */
inline void xwy(const vector< component_type, 3 > & xwy_)
{ this->_x = xwy_.x(); this->_w = xwy_.w(); this->_y = xwy_.y(); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > xwyx() const
{ return vector< component_type, 4 >(this->_x, this->_w, this->_y, this->_x); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > xwyy() const
{ return vector< component_type, 4 >(this->_x, this->_w, this->_y, this->_y); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > xwyz() const
{ return vector< component_type, 4 >(this->_x, this->_w, this->_y, this->_z); }

/**  Set multiple components of vector as a new vector.  */
inline void xwyz(const component_type & x_, const component_type & w_, const component_type & y_, const component_type & z_)
{ this->_x = x_; this->_w = w_; this->_y = y_; this->_z = z_; }

/**  Set multiple components of vector as a new vector.  */
inline void xwyz(const component_type & x_, const vector< component_type, 3 > & wyz_)
{ this->_x = x_; this->_w = wyz_.w(); this->_y = wyz_.y(); this->_z = wyz_.z(); }

/**  Set multiple components of vector as a new vector.  */
inline void xwyz(const vector< component_type, 3 > & xwy_, const component_type & z_)
{ this->_x = xwy_.x(); this->_w = xwy_.w(); this->_y = xwy_.y(); this->_z = z_; }

/**  Set multiple components of vector as a new vector.  */
inline void xwyz(const component_type & x_, const component_type & w_, const vector< component_type, 2 > & yz_)
{ this->_x = x_; this->_w = w_; this->_y = yz_.y(); this->_z = yz_.z(); }

/**  Set multiple components of vector as a new vector.  */
inline void xwyz(const component_type & x_, const vector< component_type, 2 > & wy_, const component_type & z_)
{ this->_x = x_; this->_w = wy_.w(); this->_y = wy_.y(); this->_z = z_; }

/**  Set multiple components of vector as a new vector.  */
inline void xwyz(const vector< component_type, 2 > & xw_, const component_type & y_, const component_type & z_)
{ this->_x = xw_.x(); this->_w = xw_.w(); this->_y = y_; this->_z = z_; }

/**  Set multiple components of vector as a new vector.  */
inline void xwyz(const vector< component_type, 2 > & xw_, const vector< component_type, 2 > & yz_)
{ this->_x = xw_.x(); this->_w = xw_.w(); this->_y = yz_.y(); this->_z = yz_.z(); }

/**  Set multiple components of vector as a new vector.  */
inline void xwyz(const vector< component_type, 4 > & xwyz_)
{ this->_x = xwyz_.x(); this->_w = xwyz_.w(); this->_y = xwyz_.y(); this->_z = xwyz_.z(); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > xwyw() const
{ return vector< component_type, 4 >(this->_x, this->_w, this->_y, this->_w); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 3 > xwz() const
{ return vector< component_type, 3 >(this->_x, this->_w, this->_z); }

/**  Set multiple components of vector as a new vector.  */
inline void xwz(const component_type & x_, const component_type & w_, const component_type & z_)
{ this->_x = x_; this->_w = w_; this->_z = z_; }

/**  Set multiple components of vector as a new vector.  */
inline void xwz(const component_type & x_, const vector< component_type, 2 > & wz_)
{ this->_x = x_; this->_w = wz_.w(); this->_z = wz_.z(); }

/**  Set multiple components of vector as a new vector.  */
inline void xwz(const vector< component_type, 2 > & xw_, const component_type & z_)
{ this->_x = xw_.x(); this->_w = xw_.w(); this->_z = z_; }

/**  Set multiple components of vector as a new vector.  */
inline void xwz(const vector< component_type, 3 > & xwz_)
{ this->_x = xwz_.x(); this->_w = xwz_.w(); this->_z = xwz_.z(); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > xwzx() const
{ return vector< component_type, 4 >(this->_x, this->_w, this->_z, this->_x); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > xwzy() const
{ return vector< component_type, 4 >(this->_x, this->_w, this->_z, this->_y); }

/**  Set multiple components of vector as a new vector.  */
inline void xwzy(const component_type & x_, const component_type & w_, const component_type & z_, const component_type & y_)
{ this->_x = x_; this->_w = w_; this->_z = z_; this->_y = y_; }

/**  Set multiple components of vector as a new vector.  */
inline void xwzy(const component_type & x_, const vector< component_type, 3 > & wzy_)
{ this->_x = x_; this->_w = wzy_.w(); this->_z = wzy_.z(); this->_y = wzy_.y(); }

/**  Set multiple components of vector as a new vector.  */
inline void xwzy(const vector< component_type, 3 > & xwz_, const component_type & y_)
{ this->_x = xwz_.x(); this->_w = xwz_.w(); this->_z = xwz_.z(); this->_y = y_; }

/**  Set multiple components of vector as a new vector.  */
inline void xwzy(const component_type & x_, const component_type & w_, const vector< component_type, 2 > & zy_)
{ this->_x = x_; this->_w = w_; this->_z = zy_.z(); this->_y = zy_.y(); }

/**  Set multiple components of vector as a new vector.  */
inline void xwzy(const component_type & x_, const vector< component_type, 2 > & wz_, const component_type & y_)
{ this->_x = x_; this->_w = wz_.w(); this->_z = wz_.z(); this->_y = y_; }

/**  Set multiple components of vector as a new vector.  */
inline void xwzy(const vector< component_type, 2 > & xw_, const component_type & z_, const component_type & y_)
{ this->_x = xw_.x(); this->_w = xw_.w(); this->_z = z_; this->_y = y_; }

/**  Set multiple components of vector as a new vector.  */
inline void xwzy(const vector< component_type, 2 > & xw_, const vector< component_type, 2 > & zy_)
{ this->_x = xw_.x(); this->_w = xw_.w(); this->_z = zy_.z(); this->_y = zy_.y(); }

/**  Set multiple components of vector as a new vector.  */
inline void xwzy(const vector< component_type, 4 > & xwzy_)
{ this->_x = xwzy_.x(); this->_w = xwzy_.w(); this->_z = xwzy_.z(); this->_y = xwzy_.y(); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > xwzz() const
{ return vector< component_type, 4 >(this->_x, this->_w, this->_z, this->_z); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > xwzw() const
{ return vector< component_type, 4 >(this->_x, this->_w, this->_z, this->_w); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 3 > xww() const
{ return vector< component_type, 3 >(this->_x, this->_w, this->_w); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > xwwx() const
{ return vector< component_type, 4 >(this->_x, this->_w, this->_w, this->_x); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > xwwy() const
{ return vector< component_type, 4 >(this->_x, this->_w, this->_w, this->_y); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > xwwz() const
{ return vector< component_type, 4 >(this->_x, this->_w, this->_w, this->_z); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > xwww() const
{ return vector< component_type, 4 >(this->_x, this->_w, this->_w, this->_w); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > yxxw() const
{ return vector< component_type, 4 >(this->_y, this->_x, this->_x, this->_w); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > yxyw() const
{ return vector< component_type, 4 >(this->_y, this->_x, this->_y, this->_w); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > yxzw() const
{ return vector< component_type, 4 >(this->_y, this->_x, this->_z, this->_w); }

/**  Set multiple components of vector as a new vector.  */
inline void yxzw(const component_type & y_, const component_type & x_, const component_type & z_, const component_type & w_)
{ this->_y = y_; this->_x = x_; this->_z = z_; this->_w = w_; }

/**  Set multiple components of vector as a new vector.  */
inline void yxzw(const component_type & y_, const vector< component_type, 3 > & xzw_)
{ this->_y = y_; this->_x = xzw_.x(); this->_z = xzw_.z(); this->_w = xzw_.w(); }

/**  Set multiple components of vector as a new vector.  */
inline void yxzw(const vector< component_type, 3 > & yxz_, const component_type & w_)
{ this->_y = yxz_.y(); this->_x = yxz_.x(); this->_z = yxz_.z(); this->_w = w_; }

/**  Set multiple components of vector as a new vector.  */
inline void yxzw(const component_type & y_, const component_type & x_, const vector< component_type, 2 > & zw_)
{ this->_y = y_; this->_x = x_; this->_z = zw_.z(); this->_w = zw_.w(); }

/**  Set multiple components of vector as a new vector.  */
inline void yxzw(const component_type & y_, const vector< component_type, 2 > & xz_, const component_type & w_)
{ this->_y = y_; this->_x = xz_.x(); this->_z = xz_.z(); this->_w = w_; }

/**  Set multiple components of vector as a new vector.  */
inline void yxzw(const vector< component_type, 2 > & yx_, const component_type & z_, const component_type & w_)
{ this->_y = yx_.y(); this->_x = yx_.x(); this->_z = z_; this->_w = w_; }

/**  Set multiple components of vector as a new vector.  */
inline void yxzw(const vector< component_type, 2 > & yx_, const vector< component_type, 2 > & zw_)
{ this->_y = yx_.y(); this->_x = yx_.x(); this->_z = zw_.z(); this->_w = zw_.w(); }

/**  Set multiple components of vector as a new vector.  */
inline void yxzw(const vector< component_type, 4 > & yxzw_)
{ this->_y = yxzw_.y(); this->_x = yxzw_.x(); this->_z = yxzw_.z(); this->_w = yxzw_.w(); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 3 > yxw() const
{ return vector< component_type, 3 >(this->_y, this->_x, this->_w); }

/**  Set multiple components of vector as a new vector.  */
inline void yxw(const component_type & y_, const component_type & x_, const component_type & w_)
{ this->_y = y_; this->_x = x_; this->_w = w_; }

/**  Set multiple components of vector as a new vector.  */
inline void yxw(const component_type & y_, const vector< component_type, 2 > & xw_)
{ this->_y = y_; this->_x = xw_.x(); this->_w = xw_.w(); }

/**  Set multiple components of vector as a new vector.  */
inline void yxw(const vector< component_type, 2 > & yx_, const component_type & w_)
{ this->_y = yx_.y(); this->_x = yx_.x(); this->_w = w_; }

/**  Set multiple components of vector as a new vector.  */
inline void yxw(const vector< component_type, 3 > & yxw_)
{ this->_y = yxw_.y(); this->_x = yxw_.x(); this->_w = yxw_.w(); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > yxwx() const
{ return vector< component_type, 4 >(this->_y, this->_x, this->_w, this->_x); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > yxwy() const
{ return vector< component_type, 4 >(this->_y, this->_x, this->_w, this->_y); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > yxwz() const
{ return vector< component_type, 4 >(this->_y, this->_x, this->_w, this->_z); }

/**  Set multiple components of vector as a new vector.  */
inline void yxwz(const component_type & y_, const component_type & x_, const component_type & w_, const component_type & z_)
{ this->_y = y_; this->_x = x_; this->_w = w_; this->_z = z_; }

/**  Set multiple components of vector as a new vector.  */
inline void yxwz(const component_type & y_, const vector< component_type, 3 > & xwz_)
{ this->_y = y_; this->_x = xwz_.x(); this->_w = xwz_.w(); this->_z = xwz_.z(); }

/**  Set multiple components of vector as a new vector.  */
inline void yxwz(const vector< component_type, 3 > & yxw_, const component_type & z_)
{ this->_y = yxw_.y(); this->_x = yxw_.x(); this->_w = yxw_.w(); this->_z = z_; }

/**  Set multiple components of vector as a new vector.  */
inline void yxwz(const component_type & y_, const component_type & x_, const vector< component_type, 2 > & wz_)
{ this->_y = y_; this->_x = x_; this->_w = wz_.w(); this->_z = wz_.z(); }

/**  Set multiple components of vector as a new vector.  */
inline void yxwz(const component_type & y_, const vector< component_type, 2 > & xw_, const component_type & z_)
{ this->_y = y_; this->_x = xw_.x(); this->_w = xw_.w(); this->_z = z_; }

/**  Set multiple components of vector as a new vector.  */
inline void yxwz(const vector< component_type, 2 > & yx_, const component_type & w_, const component_type & z_)
{ this->_y = yx_.y(); this->_x = yx_.x(); this->_w = w_; this->_z = z_; }

/**  Set multiple components of vector as a new vector.  */
inline void yxwz(const vector< component_type, 2 > & yx_, const vector< component_type, 2 > & wz_)
{ this->_y = yx_.y(); this->_x = yx_.x(); this->_w = wz_.w(); this->_z = wz_.z(); }

/**  Set multiple components of vector as a new vector.  */
inline void yxwz(const vector< component_type, 4 > & yxwz_)
{ this->_y = yxwz_.y(); this->_x = yxwz_.x(); this->_w = yxwz_.w(); this->_z = yxwz_.z(); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > yxww() const
{ return vector< component_type, 4 >(this->_y, this->_x, this->_w, this->_w); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > yyxw() const
{ return vector< component_type, 4 >(this->_y, this->_y, this->_x, this->_w); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > yyyw() const
{ return vector< component_type, 4 >(this->_y, this->_y, this->_y, this->_w); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > yyzw() const
{ return vector< component_type, 4 >(this->_y, this->_y, this->_z, this->_w); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 3 > yyw() const
{ return vector< component_type, 3 >(this->_y, this->_y, this->_w); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > yywx() const
{ return vector< component_type, 4 >(this->_y, this->_y, this->_w, this->_x); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > yywy() const
{ return vector< component_type, 4 >(this->_y, this->_y, this->_w, this->_y); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > yywz() const
{ return vector< component_type, 4 >(this->_y, this->_y, this->_w, this->_z); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > yyww() const
{ return vector< component_type, 4 >(this->_y, this->_y, this->_w, this->_w); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > yzxw() const
{ return vector< component_type, 4 >(this->_y, this->_z, this->_x, this->_w); }

/**  Set multiple components of vector as a new vector.  */
inline void yzxw(const component_type & y_, const component_type & z_, const component_type & x_, const component_type & w_)
{ this->_y = y_; this->_z = z_; this->_x = x_; this->_w = w_; }

/**  Set multiple components of vector as a new vector.  */
inline void yzxw(const component_type & y_, const vector< component_type, 3 > & zxw_)
{ this->_y = y_; this->_z = zxw_.z(); this->_x = zxw_.x(); this->_w = zxw_.w(); }

/**  Set multiple components of vector as a new vector.  */
inline void yzxw(const vector< component_type, 3 > & yzx_, const component_type & w_)
{ this->_y = yzx_.y(); this->_z = yzx_.z(); this->_x = yzx_.x(); this->_w = w_; }

/**  Set multiple components of vector as a new vector.  */
inline void yzxw(const component_type & y_, const component_type & z_, const vector< component_type, 2 > & xw_)
{ this->_y = y_; this->_z = z_; this->_x = xw_.x(); this->_w = xw_.w(); }

/**  Set multiple components of vector as a new vector.  */
inline void yzxw(const component_type & y_, const vector< component_type, 2 > & zx_, const component_type & w_)
{ this->_y = y_; this->_z = zx_.z(); this->_x = zx_.x(); this->_w = w_; }

/**  Set multiple components of vector as a new vector.  */
inline void yzxw(const vector< component_type, 2 > & yz_, const component_type & x_, const component_type & w_)
{ this->_y = yz_.y(); this->_z = yz_.z(); this->_x = x_; this->_w = w_; }

/**  Set multiple components of vector as a new vector.  */
inline void yzxw(const vector< component_type, 2 > & yz_, const vector< component_type, 2 > & xw_)
{ this->_y = yz_.y(); this->_z = yz_.z(); this->_x = xw_.x(); this->_w = xw_.w(); }

/**  Set multiple components of vector as a new vector.  */
inline void yzxw(const vector< component_type, 4 > & yzxw_)
{ this->_y = yzxw_.y(); this->_z = yzxw_.z(); this->_x = yzxw_.x(); this->_w = yzxw_.w(); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > yzyw() const
{ return vector< component_type, 4 >(this->_y, this->_z, this->_y, this->_w); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > yzzw() const
{ return vector< component_type, 4 >(this->_y, this->_z, this->_z, this->_w); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 3 > yzw() const
{ return vector< component_type, 3 >(this->_y, this->_z, this->_w); }

/**  Set multiple components of vector as a new vector.  */
inline void yzw(const component_type & y_, const component_type & z_, const component_type & w_)
{ this->_y = y_; this->_z = z_; this->_w = w_; }

/**  Set multiple components of vector as a new vector.  */
inline void yzw(const component_type & y_, const vector< component_type, 2 > & zw_)
{ this->_y = y_; this->_z = zw_.z(); this->_w = zw_.w(); }

/**  Set multiple components of vector as a new vector.  */
inline void yzw(const vector< component_type, 2 > & yz_, const component_type & w_)
{ this->_y = yz_.y(); this->_z = yz_.z(); this->_w = w_; }

/**  Set multiple components of vector as a new vector.  */
inline void yzw(const vector< component_type, 3 > & yzw_)
{ this->_y = yzw_.y(); this->_z = yzw_.z(); this->_w = yzw_.w(); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > yzwx() const
{ return vector< component_type, 4 >(this->_y, this->_z, this->_w, this->_x); }

/**  Set multiple components of vector as a new vector.  */
inline void yzwx(const component_type & y_, const component_type & z_, const component_type & w_, const component_type & x_)
{ this->_y = y_; this->_z = z_; this->_w = w_; this->_x = x_; }

/**  Set multiple components of vector as a new vector.  */
inline void yzwx(const component_type & y_, const vector< component_type, 3 > & zwx_)
{ this->_y = y_; this->_z = zwx_.z(); this->_w = zwx_.w(); this->_x = zwx_.x(); }

/**  Set multiple components of vector as a new vector.  */
inline void yzwx(const vector< component_type, 3 > & yzw_, const component_type & x_)
{ this->_y = yzw_.y(); this->_z = yzw_.z(); this->_w = yzw_.w(); this->_x = x_; }

/**  Set multiple components of vector as a new vector.  */
inline void yzwx(const component_type & y_, const component_type & z_, const vector< component_type, 2 > & wx_)
{ this->_y = y_; this->_z = z_; this->_w = wx_.w(); this->_x = wx_.x(); }

/**  Set multiple components of vector as a new vector.  */
inline void yzwx(const component_type & y_, const vector< component_type, 2 > & zw_, const component_type & x_)
{ this->_y = y_; this->_z = zw_.z(); this->_w = zw_.w(); this->_x = x_; }

/**  Set multiple components of vector as a new vector.  */
inline void yzwx(const vector< component_type, 2 > & yz_, const component_type & w_, const component_type & x_)
{ this->_y = yz_.y(); this->_z = yz_.z(); this->_w = w_; this->_x = x_; }

/**  Set multiple components of vector as a new vector.  */
inline void yzwx(const vector< component_type, 2 > & yz_, const vector< component_type, 2 > & wx_)
{ this->_y = yz_.y(); this->_z = yz_.z(); this->_w = wx_.w(); this->_x = wx_.x(); }

/**  Set multiple components of vector as a new vector.  */
inline void yzwx(const vector< component_type, 4 > & yzwx_)
{ this->_y = yzwx_.y(); this->_z = yzwx_.z(); this->_w = yzwx_.w(); this->_x = yzwx_.x(); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > yzwy() const
{ return vector< component_type, 4 >(this->_y, this->_z, this->_w, this->_y); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > yzwz() const
{ return vector< component_type, 4 >(this->_y, this->_z, this->_w, this->_z); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > yzww() const
{ return vector< component_type, 4 >(this->_y, this->_z, this->_w, this->_w); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 2 > yw() const
{ return vector< component_type, 2 >(this->_y, this->_w); }

/**  Set multiple components of vector.  */
inline void yw(const component_type & y_, const component_type & w_)
{ this->_y = y_; this->_w = w_; }

/**  Set multiple components of vector.  */
inline void yw(const vector< component_type, 2 > & yw_)
{ this->_y = yw_.y(); this->_w = yw_.w(); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 3 > ywx() const
{ return vector< component_type, 3 >(this->_y, this->_w, this->_x); }

/**  Set multiple components of vector as a new vector.  */
inline void ywx(const component_type & y_, const component_type & w_, const component_type & x_)
{ this->_y = y_; this->_w = w_; this->_x = x_; }

/**  Set multiple components of vector as a new vector.  */
inline void ywx(const component_type & y_, const vector< component_type, 2 > & wx_)
{ this->_y = y_; this->_w = wx_.w(); this->_x = wx_.x(); }

/**  Set multiple components of vector as a new vector.  */
inline void ywx(const vector< component_type, 2 > & yw_, const component_type & x_)
{ this->_y = yw_.y(); this->_w = yw_.w(); this->_x = x_; }

/**  Set multiple components of vector as a new vector.  */
inline void ywx(const vector< component_type, 3 > & ywx_)
{ this->_y = ywx_.y(); this->_w = ywx_.w(); this->_x = ywx_.x(); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > ywxx() const
{ return vector< component_type, 4 >(this->_y, this->_w, this->_x, this->_x); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > ywxy() const
{ return vector< component_type, 4 >(this->_y, this->_w, this->_x, this->_y); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > ywxz() const
{ return vector< component_type, 4 >(this->_y, this->_w, this->_x, this->_z); }

/**  Set multiple components of vector as a new vector.  */
inline void ywxz(const component_type & y_, const component_type & w_, const component_type & x_, const component_type & z_)
{ this->_y = y_; this->_w = w_; this->_x = x_; this->_z = z_; }

/**  Set multiple components of vector as a new vector.  */
inline void ywxz(const component_type & y_, const vector< component_type, 3 > & wxz_)
{ this->_y = y_; this->_w = wxz_.w(); this->_x = wxz_.x(); this->_z = wxz_.z(); }

/**  Set multiple components of vector as a new vector.  */
inline void ywxz(const vector< component_type, 3 > & ywx_, const component_type & z_)
{ this->_y = ywx_.y(); this->_w = ywx_.w(); this->_x = ywx_.x(); this->_z = z_; }

/**  Set multiple components of vector as a new vector.  */
inline void ywxz(const component_type & y_, const component_type & w_, const vector< component_type, 2 > & xz_)
{ this->_y = y_; this->_w = w_; this->_x = xz_.x(); this->_z = xz_.z(); }

/**  Set multiple components of vector as a new vector.  */
inline void ywxz(const component_type & y_, const vector< component_type, 2 > & wx_, const component_type & z_)
{ this->_y = y_; this->_w = wx_.w(); this->_x = wx_.x(); this->_z = z_; }

/**  Set multiple components of vector as a new vector.  */
inline void ywxz(const vector< component_type, 2 > & yw_, const component_type & x_, const component_type & z_)
{ this->_y = yw_.y(); this->_w = yw_.w(); this->_x = x_; this->_z = z_; }

/**  Set multiple components of vector as a new vector.  */
inline void ywxz(const vector< component_type, 2 > & yw_, const vector< component_type, 2 > & xz_)
{ this->_y = yw_.y(); this->_w = yw_.w(); this->_x = xz_.x(); this->_z = xz_.z(); }

/**  Set multiple components of vector as a new vector.  */
inline void ywxz(const vector< component_type, 4 > & ywxz_)
{ this->_y = ywxz_.y(); this->_w = ywxz_.w(); this->_x = ywxz_.x(); this->_z = ywxz_.z(); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > ywxw() const
{ return vector< component_type, 4 >(this->_y, this->_w, this->_x, this->_w); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 3 > ywy() const
{ return vector< component_type, 3 >(this->_y, this->_w, this->_y); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > ywyx() const
{ return vector< component_type, 4 >(this->_y, this->_w, this->_y, this->_x); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > ywyy() const
{ return vector< component_type, 4 >(this->_y, this->_w, this->_y, this->_y); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > ywyz() const
{ return vector< component_type, 4 >(this->_y, this->_w, this->_y, this->_z); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > ywyw() const
{ return vector< component_type, 4 >(this->_y, this->_w, this->_y, this->_w); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 3 > ywz() const
{ return vector< component_type, 3 >(this->_y, this->_w, this->_z); }

/**  Set multiple components of vector as a new vector.  */
inline void ywz(const component_type & y_, const component_type & w_, const component_type & z_)
{ this->_y = y_; this->_w = w_; this->_z = z_; }

/**  Set multiple components of vector as a new vector.  */
inline void ywz(const component_type & y_, const vector< component_type, 2 > & wz_)
{ this->_y = y_; this->_w = wz_.w(); this->_z = wz_.z(); }

/**  Set multiple components of vector as a new vector.  */
inline void ywz(const vector< component_type, 2 > & yw_, const component_type & z_)
{ this->_y = yw_.y(); this->_w = yw_.w(); this->_z = z_; }

/**  Set multiple components of vector as a new vector.  */
inline void ywz(const vector< component_type, 3 > & ywz_)
{ this->_y = ywz_.y(); this->_w = ywz_.w(); this->_z = ywz_.z(); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > ywzx() const
{ return vector< component_type, 4 >(this->_y, this->_w, this->_z, this->_x); }

/**  Set multiple components of vector as a new vector.  */
inline void ywzx(const component_type & y_, const component_type & w_, const component_type & z_, const component_type & x_)
{ this->_y = y_; this->_w = w_; this->_z = z_; this->_x = x_; }

/**  Set multiple components of vector as a new vector.  */
inline void ywzx(const component_type & y_, const vector< component_type, 3 > & wzx_)
{ this->_y = y_; this->_w = wzx_.w(); this->_z = wzx_.z(); this->_x = wzx_.x(); }

/**  Set multiple components of vector as a new vector.  */
inline void ywzx(const vector< component_type, 3 > & ywz_, const component_type & x_)
{ this->_y = ywz_.y(); this->_w = ywz_.w(); this->_z = ywz_.z(); this->_x = x_; }

/**  Set multiple components of vector as a new vector.  */
inline void ywzx(const component_type & y_, const component_type & w_, const vector< component_type, 2 > & zx_)
{ this->_y = y_; this->_w = w_; this->_z = zx_.z(); this->_x = zx_.x(); }

/**  Set multiple components of vector as a new vector.  */
inline void ywzx(const component_type & y_, const vector< component_type, 2 > & wz_, const component_type & x_)
{ this->_y = y_; this->_w = wz_.w(); this->_z = wz_.z(); this->_x = x_; }

/**  Set multiple components of vector as a new vector.  */
inline void ywzx(const vector< component_type, 2 > & yw_, const component_type & z_, const component_type & x_)
{ this->_y = yw_.y(); this->_w = yw_.w(); this->_z = z_; this->_x = x_; }

/**  Set multiple components of vector as a new vector.  */
inline void ywzx(const vector< component_type, 2 > & yw_, const vector< component_type, 2 > & zx_)
{ this->_y = yw_.y(); this->_w = yw_.w(); this->_z = zx_.z(); this->_x = zx_.x(); }

/**  Set multiple components of vector as a new vector.  */
inline void ywzx(const vector< component_type, 4 > & ywzx_)
{ this->_y = ywzx_.y(); this->_w = ywzx_.w(); this->_z = ywzx_.z(); this->_x = ywzx_.x(); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > ywzy() const
{ return vector< component_type, 4 >(this->_y, this->_w, this->_z, this->_y); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > ywzz() const
{ return vector< component_type, 4 >(this->_y, this->_w, this->_z, this->_z); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > ywzw() const
{ return vector< component_type, 4 >(this->_y, this->_w, this->_z, this->_w); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 3 > yww() const
{ return vector< component_type, 3 >(this->_y, this->_w, this->_w); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > ywwx() const
{ return vector< component_type, 4 >(this->_y, this->_w, this->_w, this->_x); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > ywwy() const
{ return vector< component_type, 4 >(this->_y, this->_w, this->_w, this->_y); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > ywwz() const
{ return vector< component_type, 4 >(this->_y, this->_w, this->_w, this->_z); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > ywww() const
{ return vector< component_type, 4 >(this->_y, this->_w, this->_w, this->_w); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > zxxw() const
{ return vector< component_type, 4 >(this->_z, this->_x, this->_x, this->_w); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > zxyw() const
{ return vector< component_type, 4 >(this->_z, this->_x, this->_y, this->_w); }

/**  Set multiple components of vector as a new vector.  */
inline void zxyw(const component_type & z_, const component_type & x_, const component_type & y_, const component_type & w_)
{ this->_z = z_; this->_x = x_; this->_y = y_; this->_w = w_; }

/**  Set multiple components of vector as a new vector.  */
inline void zxyw(const component_type & z_, const vector< component_type, 3 > & xyw_)
{ this->_z = z_; this->_x = xyw_.x(); this->_y = xyw_.y(); this->_w = xyw_.w(); }

/**  Set multiple components of vector as a new vector.  */
inline void zxyw(const vector< component_type, 3 > & zxy_, const component_type & w_)
{ this->_z = zxy_.z(); this->_x = zxy_.x(); this->_y = zxy_.y(); this->_w = w_; }

/**  Set multiple components of vector as a new vector.  */
inline void zxyw(const component_type & z_, const component_type & x_, const vector< component_type, 2 > & yw_)
{ this->_z = z_; this->_x = x_; this->_y = yw_.y(); this->_w = yw_.w(); }

/**  Set multiple components of vector as a new vector.  */
inline void zxyw(const component_type & z_, const vector< component_type, 2 > & xy_, const component_type & w_)
{ this->_z = z_; this->_x = xy_.x(); this->_y = xy_.y(); this->_w = w_; }

/**  Set multiple components of vector as a new vector.  */
inline void zxyw(const vector< component_type, 2 > & zx_, const component_type & y_, const component_type & w_)
{ this->_z = zx_.z(); this->_x = zx_.x(); this->_y = y_; this->_w = w_; }

/**  Set multiple components of vector as a new vector.  */
inline void zxyw(const vector< component_type, 2 > & zx_, const vector< component_type, 2 > & yw_)
{ this->_z = zx_.z(); this->_x = zx_.x(); this->_y = yw_.y(); this->_w = yw_.w(); }

/**  Set multiple components of vector as a new vector.  */
inline void zxyw(const vector< component_type, 4 > & zxyw_)
{ this->_z = zxyw_.z(); this->_x = zxyw_.x(); this->_y = zxyw_.y(); this->_w = zxyw_.w(); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > zxzw() const
{ return vector< component_type, 4 >(this->_z, this->_x, this->_z, this->_w); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 3 > zxw() const
{ return vector< component_type, 3 >(this->_z, this->_x, this->_w); }

/**  Set multiple components of vector as a new vector.  */
inline void zxw(const component_type & z_, const component_type & x_, const component_type & w_)
{ this->_z = z_; this->_x = x_; this->_w = w_; }

/**  Set multiple components of vector as a new vector.  */
inline void zxw(const component_type & z_, const vector< component_type, 2 > & xw_)
{ this->_z = z_; this->_x = xw_.x(); this->_w = xw_.w(); }

/**  Set multiple components of vector as a new vector.  */
inline void zxw(const vector< component_type, 2 > & zx_, const component_type & w_)
{ this->_z = zx_.z(); this->_x = zx_.x(); this->_w = w_; }

/**  Set multiple components of vector as a new vector.  */
inline void zxw(const vector< component_type, 3 > & zxw_)
{ this->_z = zxw_.z(); this->_x = zxw_.x(); this->_w = zxw_.w(); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > zxwx() const
{ return vector< component_type, 4 >(this->_z, this->_x, this->_w, this->_x); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > zxwy() const
{ return vector< component_type, 4 >(this->_z, this->_x, this->_w, this->_y); }

/**  Set multiple components of vector as a new vector.  */
inline void zxwy(const component_type & z_, const component_type & x_, const component_type & w_, const component_type & y_)
{ this->_z = z_; this->_x = x_; this->_w = w_; this->_y = y_; }

/**  Set multiple components of vector as a new vector.  */
inline void zxwy(const component_type & z_, const vector< component_type, 3 > & xwy_)
{ this->_z = z_; this->_x = xwy_.x(); this->_w = xwy_.w(); this->_y = xwy_.y(); }

/**  Set multiple components of vector as a new vector.  */
inline void zxwy(const vector< component_type, 3 > & zxw_, const component_type & y_)
{ this->_z = zxw_.z(); this->_x = zxw_.x(); this->_w = zxw_.w(); this->_y = y_; }

/**  Set multiple components of vector as a new vector.  */
inline void zxwy(const component_type & z_, const component_type & x_, const vector< component_type, 2 > & wy_)
{ this->_z = z_; this->_x = x_; this->_w = wy_.w(); this->_y = wy_.y(); }

/**  Set multiple components of vector as a new vector.  */
inline void zxwy(const component_type & z_, const vector< component_type, 2 > & xw_, const component_type & y_)
{ this->_z = z_; this->_x = xw_.x(); this->_w = xw_.w(); this->_y = y_; }

/**  Set multiple components of vector as a new vector.  */
inline void zxwy(const vector< component_type, 2 > & zx_, const component_type & w_, const component_type & y_)
{ this->_z = zx_.z(); this->_x = zx_.x(); this->_w = w_; this->_y = y_; }

/**  Set multiple components of vector as a new vector.  */
inline void zxwy(const vector< component_type, 2 > & zx_, const vector< component_type, 2 > & wy_)
{ this->_z = zx_.z(); this->_x = zx_.x(); this->_w = wy_.w(); this->_y = wy_.y(); }

/**  Set multiple components of vector as a new vector.  */
inline void zxwy(const vector< component_type, 4 > & zxwy_)
{ this->_z = zxwy_.z(); this->_x = zxwy_.x(); this->_w = zxwy_.w(); this->_y = zxwy_.y(); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > zxwz() const
{ return vector< component_type, 4 >(this->_z, this->_x, this->_w, this->_z); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > zxww() const
{ return vector< component_type, 4 >(this->_z, this->_x, this->_w, this->_w); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > zyxw() const
{ return vector< component_type, 4 >(this->_z, this->_y, this->_x, this->_w); }

/**  Set multiple components of vector as a new vector.  */
inline void zyxw(const component_type & z_, const component_type & y_, const component_type & x_, const component_type & w_)
{ this->_z = z_; this->_y = y_; this->_x = x_; this->_w = w_; }

/**  Set multiple components of vector as a new vector.  */
inline void zyxw(const component_type & z_, const vector< component_type, 3 > & yxw_)
{ this->_z = z_; this->_y = yxw_.y(); this->_x = yxw_.x(); this->_w = yxw_.w(); }

/**  Set multiple components of vector as a new vector.  */
inline void zyxw(const vector< component_type, 3 > & zyx_, const component_type & w_)
{ this->_z = zyx_.z(); this->_y = zyx_.y(); this->_x = zyx_.x(); this->_w = w_; }

/**  Set multiple components of vector as a new vector.  */
inline void zyxw(const component_type & z_, const component_type & y_, const vector< component_type, 2 > & xw_)
{ this->_z = z_; this->_y = y_; this->_x = xw_.x(); this->_w = xw_.w(); }

/**  Set multiple components of vector as a new vector.  */
inline void zyxw(const component_type & z_, const vector< component_type, 2 > & yx_, const component_type & w_)
{ this->_z = z_; this->_y = yx_.y(); this->_x = yx_.x(); this->_w = w_; }

/**  Set multiple components of vector as a new vector.  */
inline void zyxw(const vector< component_type, 2 > & zy_, const component_type & x_, const component_type & w_)
{ this->_z = zy_.z(); this->_y = zy_.y(); this->_x = x_; this->_w = w_; }

/**  Set multiple components of vector as a new vector.  */
inline void zyxw(const vector< component_type, 2 > & zy_, const vector< component_type, 2 > & xw_)
{ this->_z = zy_.z(); this->_y = zy_.y(); this->_x = xw_.x(); this->_w = xw_.w(); }

/**  Set multiple components of vector as a new vector.  */
inline void zyxw(const vector< component_type, 4 > & zyxw_)
{ this->_z = zyxw_.z(); this->_y = zyxw_.y(); this->_x = zyxw_.x(); this->_w = zyxw_.w(); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > zyyw() const
{ return vector< component_type, 4 >(this->_z, this->_y, this->_y, this->_w); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > zyzw() const
{ return vector< component_type, 4 >(this->_z, this->_y, this->_z, this->_w); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 3 > zyw() const
{ return vector< component_type, 3 >(this->_z, this->_y, this->_w); }

/**  Set multiple components of vector as a new vector.  */
inline void zyw(const component_type & z_, const component_type & y_, const component_type & w_)
{ this->_z = z_; this->_y = y_; this->_w = w_; }

/**  Set multiple components of vector as a new vector.  */
inline void zyw(const component_type & z_, const vector< component_type, 2 > & yw_)
{ this->_z = z_; this->_y = yw_.y(); this->_w = yw_.w(); }

/**  Set multiple components of vector as a new vector.  */
inline void zyw(const vector< component_type, 2 > & zy_, const component_type & w_)
{ this->_z = zy_.z(); this->_y = zy_.y(); this->_w = w_; }

/**  Set multiple components of vector as a new vector.  */
inline void zyw(const vector< component_type, 3 > & zyw_)
{ this->_z = zyw_.z(); this->_y = zyw_.y(); this->_w = zyw_.w(); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > zywx() const
{ return vector< component_type, 4 >(this->_z, this->_y, this->_w, this->_x); }

/**  Set multiple components of vector as a new vector.  */
inline void zywx(const component_type & z_, const component_type & y_, const component_type & w_, const component_type & x_)
{ this->_z = z_; this->_y = y_; this->_w = w_; this->_x = x_; }

/**  Set multiple components of vector as a new vector.  */
inline void zywx(const component_type & z_, const vector< component_type, 3 > & ywx_)
{ this->_z = z_; this->_y = ywx_.y(); this->_w = ywx_.w(); this->_x = ywx_.x(); }

/**  Set multiple components of vector as a new vector.  */
inline void zywx(const vector< component_type, 3 > & zyw_, const component_type & x_)
{ this->_z = zyw_.z(); this->_y = zyw_.y(); this->_w = zyw_.w(); this->_x = x_; }

/**  Set multiple components of vector as a new vector.  */
inline void zywx(const component_type & z_, const component_type & y_, const vector< component_type, 2 > & wx_)
{ this->_z = z_; this->_y = y_; this->_w = wx_.w(); this->_x = wx_.x(); }

/**  Set multiple components of vector as a new vector.  */
inline void zywx(const component_type & z_, const vector< component_type, 2 > & yw_, const component_type & x_)
{ this->_z = z_; this->_y = yw_.y(); this->_w = yw_.w(); this->_x = x_; }

/**  Set multiple components of vector as a new vector.  */
inline void zywx(const vector< component_type, 2 > & zy_, const component_type & w_, const component_type & x_)
{ this->_z = zy_.z(); this->_y = zy_.y(); this->_w = w_; this->_x = x_; }

/**  Set multiple components of vector as a new vector.  */
inline void zywx(const vector< component_type, 2 > & zy_, const vector< component_type, 2 > & wx_)
{ this->_z = zy_.z(); this->_y = zy_.y(); this->_w = wx_.w(); this->_x = wx_.x(); }

/**  Set multiple components of vector as a new vector.  */
inline void zywx(const vector< component_type, 4 > & zywx_)
{ this->_z = zywx_.z(); this->_y = zywx_.y(); this->_w = zywx_.w(); this->_x = zywx_.x(); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > zywy() const
{ return vector< component_type, 4 >(this->_z, this->_y, this->_w, this->_y); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > zywz() const
{ return vector< component_type, 4 >(this->_z, this->_y, this->_w, this->_z); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > zyww() const
{ return vector< component_type, 4 >(this->_z, this->_y, this->_w, this->_w); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > zzxw() const
{ return vector< component_type, 4 >(this->_z, this->_z, this->_x, this->_w); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > zzyw() const
{ return vector< component_type, 4 >(this->_z, this->_z, this->_y, this->_w); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > zzzw() const
{ return vector< component_type, 4 >(this->_z, this->_z, this->_z, this->_w); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 3 > zzw() const
{ return vector< component_type, 3 >(this->_z, this->_z, this->_w); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > zzwx() const
{ return vector< component_type, 4 >(this->_z, this->_z, this->_w, this->_x); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > zzwy() const
{ return vector< component_type, 4 >(this->_z, this->_z, this->_w, this->_y); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > zzwz() const
{ return vector< component_type, 4 >(this->_z, this->_z, this->_w, this->_z); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > zzww() const
{ return vector< component_type, 4 >(this->_z, this->_z, this->_w, this->_w); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 2 > zw() const
{ return vector< component_type, 2 >(this->_z, this->_w); }

/**  Set multiple components of vector.  */
inline void zw(const component_type & z_, const component_type & w_)
{ this->_z = z_; this->_w = w_; }

/**  Set multiple components of vector.  */
inline void zw(const vector< component_type, 2 > & zw_)
{ this->_z = zw_.z(); this->_w = zw_.w(); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 3 > zwx() const
{ return vector< component_type, 3 >(this->_z, this->_w, this->_x); }

/**  Set multiple components of vector as a new vector.  */
inline void zwx(const component_type & z_, const component_type & w_, const component_type & x_)
{ this->_z = z_; this->_w = w_; this->_x = x_; }

/**  Set multiple components of vector as a new vector.  */
inline void zwx(const component_type & z_, const vector< component_type, 2 > & wx_)
{ this->_z = z_; this->_w = wx_.w(); this->_x = wx_.x(); }

/**  Set multiple components of vector as a new vector.  */
inline void zwx(const vector< component_type, 2 > & zw_, const component_type & x_)
{ this->_z = zw_.z(); this->_w = zw_.w(); this->_x = x_; }

/**  Set multiple components of vector as a new vector.  */
inline void zwx(const vector< component_type, 3 > & zwx_)
{ this->_z = zwx_.z(); this->_w = zwx_.w(); this->_x = zwx_.x(); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > zwxx() const
{ return vector< component_type, 4 >(this->_z, this->_w, this->_x, this->_x); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > zwxy() const
{ return vector< component_type, 4 >(this->_z, this->_w, this->_x, this->_y); }

/**  Set multiple components of vector as a new vector.  */
inline void zwxy(const component_type & z_, const component_type & w_, const component_type & x_, const component_type & y_)
{ this->_z = z_; this->_w = w_; this->_x = x_; this->_y = y_; }

/**  Set multiple components of vector as a new vector.  */
inline void zwxy(const component_type & z_, const vector< component_type, 3 > & wxy_)
{ this->_z = z_; this->_w = wxy_.w(); this->_x = wxy_.x(); this->_y = wxy_.y(); }

/**  Set multiple components of vector as a new vector.  */
inline void zwxy(const vector< component_type, 3 > & zwx_, const component_type & y_)
{ this->_z = zwx_.z(); this->_w = zwx_.w(); this->_x = zwx_.x(); this->_y = y_; }

/**  Set multiple components of vector as a new vector.  */
inline void zwxy(const component_type & z_, const component_type & w_, const vector< component_type, 2 > & xy_)
{ this->_z = z_; this->_w = w_; this->_x = xy_.x(); this->_y = xy_.y(); }

/**  Set multiple components of vector as a new vector.  */
inline void zwxy(const component_type & z_, const vector< component_type, 2 > & wx_, const component_type & y_)
{ this->_z = z_; this->_w = wx_.w(); this->_x = wx_.x(); this->_y = y_; }

/**  Set multiple components of vector as a new vector.  */
inline void zwxy(const vector< component_type, 2 > & zw_, const component_type & x_, const component_type & y_)
{ this->_z = zw_.z(); this->_w = zw_.w(); this->_x = x_; this->_y = y_; }

/**  Set multiple components of vector as a new vector.  */
inline void zwxy(const vector< component_type, 2 > & zw_, const vector< component_type, 2 > & xy_)
{ this->_z = zw_.z(); this->_w = zw_.w(); this->_x = xy_.x(); this->_y = xy_.y(); }

/**  Set multiple components of vector as a new vector.  */
inline void zwxy(const vector< component_type, 4 > & zwxy_)
{ this->_z = zwxy_.z(); this->_w = zwxy_.w(); this->_x = zwxy_.x(); this->_y = zwxy_.y(); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > zwxz() const
{ return vector< component_type, 4 >(this->_z, this->_w, this->_x, this->_z); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > zwxw() const
{ return vector< component_type, 4 >(this->_z, this->_w, this->_x, this->_w); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 3 > zwy() const
{ return vector< component_type, 3 >(this->_z, this->_w, this->_y); }

/**  Set multiple components of vector as a new vector.  */
inline void zwy(const component_type & z_, const component_type & w_, const component_type & y_)
{ this->_z = z_; this->_w = w_; this->_y = y_; }

/**  Set multiple components of vector as a new vector.  */
inline void zwy(const component_type & z_, const vector< component_type, 2 > & wy_)
{ this->_z = z_; this->_w = wy_.w(); this->_y = wy_.y(); }

/**  Set multiple components of vector as a new vector.  */
inline void zwy(const vector< component_type, 2 > & zw_, const component_type & y_)
{ this->_z = zw_.z(); this->_w = zw_.w(); this->_y = y_; }

/**  Set multiple components of vector as a new vector.  */
inline void zwy(const vector< component_type, 3 > & zwy_)
{ this->_z = zwy_.z(); this->_w = zwy_.w(); this->_y = zwy_.y(); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > zwyx() const
{ return vector< component_type, 4 >(this->_z, this->_w, this->_y, this->_x); }

/**  Set multiple components of vector as a new vector.  */
inline void zwyx(const component_type & z_, const component_type & w_, const component_type & y_, const component_type & x_)
{ this->_z = z_; this->_w = w_; this->_y = y_; this->_x = x_; }

/**  Set multiple components of vector as a new vector.  */
inline void zwyx(const component_type & z_, const vector< component_type, 3 > & wyx_)
{ this->_z = z_; this->_w = wyx_.w(); this->_y = wyx_.y(); this->_x = wyx_.x(); }

/**  Set multiple components of vector as a new vector.  */
inline void zwyx(const vector< component_type, 3 > & zwy_, const component_type & x_)
{ this->_z = zwy_.z(); this->_w = zwy_.w(); this->_y = zwy_.y(); this->_x = x_; }

/**  Set multiple components of vector as a new vector.  */
inline void zwyx(const component_type & z_, const component_type & w_, const vector< component_type, 2 > & yx_)
{ this->_z = z_; this->_w = w_; this->_y = yx_.y(); this->_x = yx_.x(); }

/**  Set multiple components of vector as a new vector.  */
inline void zwyx(const component_type & z_, const vector< component_type, 2 > & wy_, const component_type & x_)
{ this->_z = z_; this->_w = wy_.w(); this->_y = wy_.y(); this->_x = x_; }

/**  Set multiple components of vector as a new vector.  */
inline void zwyx(const vector< component_type, 2 > & zw_, const component_type & y_, const component_type & x_)
{ this->_z = zw_.z(); this->_w = zw_.w(); this->_y = y_; this->_x = x_; }

/**  Set multiple components of vector as a new vector.  */
inline void zwyx(const vector< component_type, 2 > & zw_, const vector< component_type, 2 > & yx_)
{ this->_z = zw_.z(); this->_w = zw_.w(); this->_y = yx_.y(); this->_x = yx_.x(); }

/**  Set multiple components of vector as a new vector.  */
inline void zwyx(const vector< component_type, 4 > & zwyx_)
{ this->_z = zwyx_.z(); this->_w = zwyx_.w(); this->_y = zwyx_.y(); this->_x = zwyx_.x(); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > zwyy() const
{ return vector< component_type, 4 >(this->_z, this->_w, this->_y, this->_y); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > zwyz() const
{ return vector< component_type, 4 >(this->_z, this->_w, this->_y, this->_z); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > zwyw() const
{ return vector< component_type, 4 >(this->_z, this->_w, this->_y, this->_w); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 3 > zwz() const
{ return vector< component_type, 3 >(this->_z, this->_w, this->_z); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > zwzx() const
{ return vector< component_type, 4 >(this->_z, this->_w, this->_z, this->_x); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > zwzy() const
{ return vector< component_type, 4 >(this->_z, this->_w, this->_z, this->_y); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > zwzz() const
{ return vector< component_type, 4 >(this->_z, this->_w, this->_z, this->_z); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > zwzw() const
{ return vector< component_type, 4 >(this->_z, this->_w, this->_z, this->_w); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 3 > zww() const
{ return vector< component_type, 3 >(this->_z, this->_w, this->_w); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > zwwx() const
{ return vector< component_type, 4 >(this->_z, this->_w, this->_w, this->_x); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > zwwy() const
{ return vector< component_type, 4 >(this->_z, this->_w, this->_w, this->_y); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > zwwz() const
{ return vector< component_type, 4 >(this->_z, this->_w, this->_w, this->_z); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > zwww() const
{ return vector< component_type, 4 >(this->_z, this->_w, this->_w, this->_w); }

/**  Get 4th component of vector.  */
inline component_type w() const
{ return this->_w; }

/**  Set 4th component of vector to w_.  */
inline void w(const component_type & w_)
{ this->_w = w_; }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 2 > wx() const
{ return vector< component_type, 2 >(this->_w, this->_x); }

/**  Set multiple components of vector.  */
inline void wx(const component_type & w_, const component_type & x_)
{ this->_w = w_; this->_x = x_; }

/**  Set multiple components of vector.  */
inline void wx(const vector< component_type, 2 > & wx_)
{ this->_w = wx_.w(); this->_x = wx_.x(); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 3 > wxx() const
{ return vector< component_type, 3 >(this->_w, this->_x, this->_x); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > wxxx() const
{ return vector< component_type, 4 >(this->_w, this->_x, this->_x, this->_x); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > wxxy() const
{ return vector< component_type, 4 >(this->_w, this->_x, this->_x, this->_y); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > wxxz() const
{ return vector< component_type, 4 >(this->_w, this->_x, this->_x, this->_z); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > wxxw() const
{ return vector< component_type, 4 >(this->_w, this->_x, this->_x, this->_w); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 3 > wxy() const
{ return vector< component_type, 3 >(this->_w, this->_x, this->_y); }

/**  Set multiple components of vector as a new vector.  */
inline void wxy(const component_type & w_, const component_type & x_, const component_type & y_)
{ this->_w = w_; this->_x = x_; this->_y = y_; }

/**  Set multiple components of vector as a new vector.  */
inline void wxy(const component_type & w_, const vector< component_type, 2 > & xy_)
{ this->_w = w_; this->_x = xy_.x(); this->_y = xy_.y(); }

/**  Set multiple components of vector as a new vector.  */
inline void wxy(const vector< component_type, 2 > & wx_, const component_type & y_)
{ this->_w = wx_.w(); this->_x = wx_.x(); this->_y = y_; }

/**  Set multiple components of vector as a new vector.  */
inline void wxy(const vector< component_type, 3 > & wxy_)
{ this->_w = wxy_.w(); this->_x = wxy_.x(); this->_y = wxy_.y(); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > wxyx() const
{ return vector< component_type, 4 >(this->_w, this->_x, this->_y, this->_x); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > wxyy() const
{ return vector< component_type, 4 >(this->_w, this->_x, this->_y, this->_y); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > wxyz() const
{ return vector< component_type, 4 >(this->_w, this->_x, this->_y, this->_z); }

/**  Set multiple components of vector as a new vector.  */
inline void wxyz(const component_type & w_, const component_type & x_, const component_type & y_, const component_type & z_)
{ this->_w = w_; this->_x = x_; this->_y = y_; this->_z = z_; }

/**  Set multiple components of vector as a new vector.  */
inline void wxyz(const component_type & w_, const vector< component_type, 3 > & xyz_)
{ this->_w = w_; this->_x = xyz_.x(); this->_y = xyz_.y(); this->_z = xyz_.z(); }

/**  Set multiple components of vector as a new vector.  */
inline void wxyz(const vector< component_type, 3 > & wxy_, const component_type & z_)
{ this->_w = wxy_.w(); this->_x = wxy_.x(); this->_y = wxy_.y(); this->_z = z_; }

/**  Set multiple components of vector as a new vector.  */
inline void wxyz(const component_type & w_, const component_type & x_, const vector< component_type, 2 > & yz_)
{ this->_w = w_; this->_x = x_; this->_y = yz_.y(); this->_z = yz_.z(); }

/**  Set multiple components of vector as a new vector.  */
inline void wxyz(const component_type & w_, const vector< component_type, 2 > & xy_, const component_type & z_)
{ this->_w = w_; this->_x = xy_.x(); this->_y = xy_.y(); this->_z = z_; }

/**  Set multiple components of vector as a new vector.  */
inline void wxyz(const vector< component_type, 2 > & wx_, const component_type & y_, const component_type & z_)
{ this->_w = wx_.w(); this->_x = wx_.x(); this->_y = y_; this->_z = z_; }

/**  Set multiple components of vector as a new vector.  */
inline void wxyz(const vector< component_type, 2 > & wx_, const vector< component_type, 2 > & yz_)
{ this->_w = wx_.w(); this->_x = wx_.x(); this->_y = yz_.y(); this->_z = yz_.z(); }

/**  Set multiple components of vector as a new vector.  */
inline void wxyz(const vector< component_type, 4 > & wxyz_)
{ this->_w = wxyz_.w(); this->_x = wxyz_.x(); this->_y = wxyz_.y(); this->_z = wxyz_.z(); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > wxyw() const
{ return vector< component_type, 4 >(this->_w, this->_x, this->_y, this->_w); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 3 > wxz() const
{ return vector< component_type, 3 >(this->_w, this->_x, this->_z); }

/**  Set multiple components of vector as a new vector.  */
inline void wxz(const component_type & w_, const component_type & x_, const component_type & z_)
{ this->_w = w_; this->_x = x_; this->_z = z_; }

/**  Set multiple components of vector as a new vector.  */
inline void wxz(const component_type & w_, const vector< component_type, 2 > & xz_)
{ this->_w = w_; this->_x = xz_.x(); this->_z = xz_.z(); }

/**  Set multiple components of vector as a new vector.  */
inline void wxz(const vector< component_type, 2 > & wx_, const component_type & z_)
{ this->_w = wx_.w(); this->_x = wx_.x(); this->_z = z_; }

/**  Set multiple components of vector as a new vector.  */
inline void wxz(const vector< component_type, 3 > & wxz_)
{ this->_w = wxz_.w(); this->_x = wxz_.x(); this->_z = wxz_.z(); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > wxzx() const
{ return vector< component_type, 4 >(this->_w, this->_x, this->_z, this->_x); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > wxzy() const
{ return vector< component_type, 4 >(this->_w, this->_x, this->_z, this->_y); }

/**  Set multiple components of vector as a new vector.  */
inline void wxzy(const component_type & w_, const component_type & x_, const component_type & z_, const component_type & y_)
{ this->_w = w_; this->_x = x_; this->_z = z_; this->_y = y_; }

/**  Set multiple components of vector as a new vector.  */
inline void wxzy(const component_type & w_, const vector< component_type, 3 > & xzy_)
{ this->_w = w_; this->_x = xzy_.x(); this->_z = xzy_.z(); this->_y = xzy_.y(); }

/**  Set multiple components of vector as a new vector.  */
inline void wxzy(const vector< component_type, 3 > & wxz_, const component_type & y_)
{ this->_w = wxz_.w(); this->_x = wxz_.x(); this->_z = wxz_.z(); this->_y = y_; }

/**  Set multiple components of vector as a new vector.  */
inline void wxzy(const component_type & w_, const component_type & x_, const vector< component_type, 2 > & zy_)
{ this->_w = w_; this->_x = x_; this->_z = zy_.z(); this->_y = zy_.y(); }

/**  Set multiple components of vector as a new vector.  */
inline void wxzy(const component_type & w_, const vector< component_type, 2 > & xz_, const component_type & y_)
{ this->_w = w_; this->_x = xz_.x(); this->_z = xz_.z(); this->_y = y_; }

/**  Set multiple components of vector as a new vector.  */
inline void wxzy(const vector< component_type, 2 > & wx_, const component_type & z_, const component_type & y_)
{ this->_w = wx_.w(); this->_x = wx_.x(); this->_z = z_; this->_y = y_; }

/**  Set multiple components of vector as a new vector.  */
inline void wxzy(const vector< component_type, 2 > & wx_, const vector< component_type, 2 > & zy_)
{ this->_w = wx_.w(); this->_x = wx_.x(); this->_z = zy_.z(); this->_y = zy_.y(); }

/**  Set multiple components of vector as a new vector.  */
inline void wxzy(const vector< component_type, 4 > & wxzy_)
{ this->_w = wxzy_.w(); this->_x = wxzy_.x(); this->_z = wxzy_.z(); this->_y = wxzy_.y(); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > wxzz() const
{ return vector< component_type, 4 >(this->_w, this->_x, this->_z, this->_z); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > wxzw() const
{ return vector< component_type, 4 >(this->_w, this->_x, this->_z, this->_w); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 3 > wxw() const
{ return vector< component_type, 3 >(this->_w, this->_x, this->_w); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > wxwx() const
{ return vector< component_type, 4 >(this->_w, this->_x, this->_w, this->_x); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > wxwy() const
{ return vector< component_type, 4 >(this->_w, this->_x, this->_w, this->_y); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > wxwz() const
{ return vector< component_type, 4 >(this->_w, this->_x, this->_w, this->_z); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > wxww() const
{ return vector< component_type, 4 >(this->_w, this->_x, this->_w, this->_w); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 2 > wy() const
{ return vector< component_type, 2 >(this->_w, this->_y); }

/**  Set multiple components of vector.  */
inline void wy(const component_type & w_, const component_type & y_)
{ this->_w = w_; this->_y = y_; }

/**  Set multiple components of vector.  */
inline void wy(const vector< component_type, 2 > & wy_)
{ this->_w = wy_.w(); this->_y = wy_.y(); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 3 > wyx() const
{ return vector< component_type, 3 >(this->_w, this->_y, this->_x); }

/**  Set multiple components of vector as a new vector.  */
inline void wyx(const component_type & w_, const component_type & y_, const component_type & x_)
{ this->_w = w_; this->_y = y_; this->_x = x_; }

/**  Set multiple components of vector as a new vector.  */
inline void wyx(const component_type & w_, const vector< component_type, 2 > & yx_)
{ this->_w = w_; this->_y = yx_.y(); this->_x = yx_.x(); }

/**  Set multiple components of vector as a new vector.  */
inline void wyx(const vector< component_type, 2 > & wy_, const component_type & x_)
{ this->_w = wy_.w(); this->_y = wy_.y(); this->_x = x_; }

/**  Set multiple components of vector as a new vector.  */
inline void wyx(const vector< component_type, 3 > & wyx_)
{ this->_w = wyx_.w(); this->_y = wyx_.y(); this->_x = wyx_.x(); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > wyxx() const
{ return vector< component_type, 4 >(this->_w, this->_y, this->_x, this->_x); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > wyxy() const
{ return vector< component_type, 4 >(this->_w, this->_y, this->_x, this->_y); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > wyxz() const
{ return vector< component_type, 4 >(this->_w, this->_y, this->_x, this->_z); }

/**  Set multiple components of vector as a new vector.  */
inline void wyxz(const component_type & w_, const component_type & y_, const component_type & x_, const component_type & z_)
{ this->_w = w_; this->_y = y_; this->_x = x_; this->_z = z_; }

/**  Set multiple components of vector as a new vector.  */
inline void wyxz(const component_type & w_, const vector< component_type, 3 > & yxz_)
{ this->_w = w_; this->_y = yxz_.y(); this->_x = yxz_.x(); this->_z = yxz_.z(); }

/**  Set multiple components of vector as a new vector.  */
inline void wyxz(const vector< component_type, 3 > & wyx_, const component_type & z_)
{ this->_w = wyx_.w(); this->_y = wyx_.y(); this->_x = wyx_.x(); this->_z = z_; }

/**  Set multiple components of vector as a new vector.  */
inline void wyxz(const component_type & w_, const component_type & y_, const vector< component_type, 2 > & xz_)
{ this->_w = w_; this->_y = y_; this->_x = xz_.x(); this->_z = xz_.z(); }

/**  Set multiple components of vector as a new vector.  */
inline void wyxz(const component_type & w_, const vector< component_type, 2 > & yx_, const component_type & z_)
{ this->_w = w_; this->_y = yx_.y(); this->_x = yx_.x(); this->_z = z_; }

/**  Set multiple components of vector as a new vector.  */
inline void wyxz(const vector< component_type, 2 > & wy_, const component_type & x_, const component_type & z_)
{ this->_w = wy_.w(); this->_y = wy_.y(); this->_x = x_; this->_z = z_; }

/**  Set multiple components of vector as a new vector.  */
inline void wyxz(const vector< component_type, 2 > & wy_, const vector< component_type, 2 > & xz_)
{ this->_w = wy_.w(); this->_y = wy_.y(); this->_x = xz_.x(); this->_z = xz_.z(); }

/**  Set multiple components of vector as a new vector.  */
inline void wyxz(const vector< component_type, 4 > & wyxz_)
{ this->_w = wyxz_.w(); this->_y = wyxz_.y(); this->_x = wyxz_.x(); this->_z = wyxz_.z(); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > wyxw() const
{ return vector< component_type, 4 >(this->_w, this->_y, this->_x, this->_w); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 3 > wyy() const
{ return vector< component_type, 3 >(this->_w, this->_y, this->_y); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > wyyx() const
{ return vector< component_type, 4 >(this->_w, this->_y, this->_y, this->_x); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > wyyy() const
{ return vector< component_type, 4 >(this->_w, this->_y, this->_y, this->_y); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > wyyz() const
{ return vector< component_type, 4 >(this->_w, this->_y, this->_y, this->_z); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > wyyw() const
{ return vector< component_type, 4 >(this->_w, this->_y, this->_y, this->_w); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 3 > wyz() const
{ return vector< component_type, 3 >(this->_w, this->_y, this->_z); }

/**  Set multiple components of vector as a new vector.  */
inline void wyz(const component_type & w_, const component_type & y_, const component_type & z_)
{ this->_w = w_; this->_y = y_; this->_z = z_; }

/**  Set multiple components of vector as a new vector.  */
inline void wyz(const component_type & w_, const vector< component_type, 2 > & yz_)
{ this->_w = w_; this->_y = yz_.y(); this->_z = yz_.z(); }

/**  Set multiple components of vector as a new vector.  */
inline void wyz(const vector< component_type, 2 > & wy_, const component_type & z_)
{ this->_w = wy_.w(); this->_y = wy_.y(); this->_z = z_; }

/**  Set multiple components of vector as a new vector.  */
inline void wyz(const vector< component_type, 3 > & wyz_)
{ this->_w = wyz_.w(); this->_y = wyz_.y(); this->_z = wyz_.z(); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > wyzx() const
{ return vector< component_type, 4 >(this->_w, this->_y, this->_z, this->_x); }

/**  Set multiple components of vector as a new vector.  */
inline void wyzx(const component_type & w_, const component_type & y_, const component_type & z_, const component_type & x_)
{ this->_w = w_; this->_y = y_; this->_z = z_; this->_x = x_; }

/**  Set multiple components of vector as a new vector.  */
inline void wyzx(const component_type & w_, const vector< component_type, 3 > & yzx_)
{ this->_w = w_; this->_y = yzx_.y(); this->_z = yzx_.z(); this->_x = yzx_.x(); }

/**  Set multiple components of vector as a new vector.  */
inline void wyzx(const vector< component_type, 3 > & wyz_, const component_type & x_)
{ this->_w = wyz_.w(); this->_y = wyz_.y(); this->_z = wyz_.z(); this->_x = x_; }

/**  Set multiple components of vector as a new vector.  */
inline void wyzx(const component_type & w_, const component_type & y_, const vector< component_type, 2 > & zx_)
{ this->_w = w_; this->_y = y_; this->_z = zx_.z(); this->_x = zx_.x(); }

/**  Set multiple components of vector as a new vector.  */
inline void wyzx(const component_type & w_, const vector< component_type, 2 > & yz_, const component_type & x_)
{ this->_w = w_; this->_y = yz_.y(); this->_z = yz_.z(); this->_x = x_; }

/**  Set multiple components of vector as a new vector.  */
inline void wyzx(const vector< component_type, 2 > & wy_, const component_type & z_, const component_type & x_)
{ this->_w = wy_.w(); this->_y = wy_.y(); this->_z = z_; this->_x = x_; }

/**  Set multiple components of vector as a new vector.  */
inline void wyzx(const vector< component_type, 2 > & wy_, const vector< component_type, 2 > & zx_)
{ this->_w = wy_.w(); this->_y = wy_.y(); this->_z = zx_.z(); this->_x = zx_.x(); }

/**  Set multiple components of vector as a new vector.  */
inline void wyzx(const vector< component_type, 4 > & wyzx_)
{ this->_w = wyzx_.w(); this->_y = wyzx_.y(); this->_z = wyzx_.z(); this->_x = wyzx_.x(); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > wyzy() const
{ return vector< component_type, 4 >(this->_w, this->_y, this->_z, this->_y); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > wyzz() const
{ return vector< component_type, 4 >(this->_w, this->_y, this->_z, this->_z); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > wyzw() const
{ return vector< component_type, 4 >(this->_w, this->_y, this->_z, this->_w); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 3 > wyw() const
{ return vector< component_type, 3 >(this->_w, this->_y, this->_w); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > wywx() const
{ return vector< component_type, 4 >(this->_w, this->_y, this->_w, this->_x); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > wywy() const
{ return vector< component_type, 4 >(this->_w, this->_y, this->_w, this->_y); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > wywz() const
{ return vector< component_type, 4 >(this->_w, this->_y, this->_w, this->_z); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > wyww() const
{ return vector< component_type, 4 >(this->_w, this->_y, this->_w, this->_w); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 2 > wz() const
{ return vector< component_type, 2 >(this->_w, this->_z); }

/**  Set multiple components of vector.  */
inline void wz(const component_type & w_, const component_type & z_)
{ this->_w = w_; this->_z = z_; }

/**  Set multiple components of vector.  */
inline void wz(const vector< component_type, 2 > & wz_)
{ this->_w = wz_.w(); this->_z = wz_.z(); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 3 > wzx() const
{ return vector< component_type, 3 >(this->_w, this->_z, this->_x); }

/**  Set multiple components of vector as a new vector.  */
inline void wzx(const component_type & w_, const component_type & z_, const component_type & x_)
{ this->_w = w_; this->_z = z_; this->_x = x_; }

/**  Set multiple components of vector as a new vector.  */
inline void wzx(const component_type & w_, const vector< component_type, 2 > & zx_)
{ this->_w = w_; this->_z = zx_.z(); this->_x = zx_.x(); }

/**  Set multiple components of vector as a new vector.  */
inline void wzx(const vector< component_type, 2 > & wz_, const component_type & x_)
{ this->_w = wz_.w(); this->_z = wz_.z(); this->_x = x_; }

/**  Set multiple components of vector as a new vector.  */
inline void wzx(const vector< component_type, 3 > & wzx_)
{ this->_w = wzx_.w(); this->_z = wzx_.z(); this->_x = wzx_.x(); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > wzxx() const
{ return vector< component_type, 4 >(this->_w, this->_z, this->_x, this->_x); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > wzxy() const
{ return vector< component_type, 4 >(this->_w, this->_z, this->_x, this->_y); }

/**  Set multiple components of vector as a new vector.  */
inline void wzxy(const component_type & w_, const component_type & z_, const component_type & x_, const component_type & y_)
{ this->_w = w_; this->_z = z_; this->_x = x_; this->_y = y_; }

/**  Set multiple components of vector as a new vector.  */
inline void wzxy(const component_type & w_, const vector< component_type, 3 > & zxy_)
{ this->_w = w_; this->_z = zxy_.z(); this->_x = zxy_.x(); this->_y = zxy_.y(); }

/**  Set multiple components of vector as a new vector.  */
inline void wzxy(const vector< component_type, 3 > & wzx_, const component_type & y_)
{ this->_w = wzx_.w(); this->_z = wzx_.z(); this->_x = wzx_.x(); this->_y = y_; }

/**  Set multiple components of vector as a new vector.  */
inline void wzxy(const component_type & w_, const component_type & z_, const vector< component_type, 2 > & xy_)
{ this->_w = w_; this->_z = z_; this->_x = xy_.x(); this->_y = xy_.y(); }

/**  Set multiple components of vector as a new vector.  */
inline void wzxy(const component_type & w_, const vector< component_type, 2 > & zx_, const component_type & y_)
{ this->_w = w_; this->_z = zx_.z(); this->_x = zx_.x(); this->_y = y_; }

/**  Set multiple components of vector as a new vector.  */
inline void wzxy(const vector< component_type, 2 > & wz_, const component_type & x_, const component_type & y_)
{ this->_w = wz_.w(); this->_z = wz_.z(); this->_x = x_; this->_y = y_; }

/**  Set multiple components of vector as a new vector.  */
inline void wzxy(const vector< component_type, 2 > & wz_, const vector< component_type, 2 > & xy_)
{ this->_w = wz_.w(); this->_z = wz_.z(); this->_x = xy_.x(); this->_y = xy_.y(); }

/**  Set multiple components of vector as a new vector.  */
inline void wzxy(const vector< component_type, 4 > & wzxy_)
{ this->_w = wzxy_.w(); this->_z = wzxy_.z(); this->_x = wzxy_.x(); this->_y = wzxy_.y(); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > wzxz() const
{ return vector< component_type, 4 >(this->_w, this->_z, this->_x, this->_z); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > wzxw() const
{ return vector< component_type, 4 >(this->_w, this->_z, this->_x, this->_w); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 3 > wzy() const
{ return vector< component_type, 3 >(this->_w, this->_z, this->_y); }

/**  Set multiple components of vector as a new vector.  */
inline void wzy(const component_type & w_, const component_type & z_, const component_type & y_)
{ this->_w = w_; this->_z = z_; this->_y = y_; }

/**  Set multiple components of vector as a new vector.  */
inline void wzy(const component_type & w_, const vector< component_type, 2 > & zy_)
{ this->_w = w_; this->_z = zy_.z(); this->_y = zy_.y(); }

/**  Set multiple components of vector as a new vector.  */
inline void wzy(const vector< component_type, 2 > & wz_, const component_type & y_)
{ this->_w = wz_.w(); this->_z = wz_.z(); this->_y = y_; }

/**  Set multiple components of vector as a new vector.  */
inline void wzy(const vector< component_type, 3 > & wzy_)
{ this->_w = wzy_.w(); this->_z = wzy_.z(); this->_y = wzy_.y(); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > wzyx() const
{ return vector< component_type, 4 >(this->_w, this->_z, this->_y, this->_x); }

/**  Set multiple components of vector as a new vector.  */
inline void wzyx(const component_type & w_, const component_type & z_, const component_type & y_, const component_type & x_)
{ this->_w = w_; this->_z = z_; this->_y = y_; this->_x = x_; }

/**  Set multiple components of vector as a new vector.  */
inline void wzyx(const component_type & w_, const vector< component_type, 3 > & zyx_)
{ this->_w = w_; this->_z = zyx_.z(); this->_y = zyx_.y(); this->_x = zyx_.x(); }

/**  Set multiple components of vector as a new vector.  */
inline void wzyx(const vector< component_type, 3 > & wzy_, const component_type & x_)
{ this->_w = wzy_.w(); this->_z = wzy_.z(); this->_y = wzy_.y(); this->_x = x_; }

/**  Set multiple components of vector as a new vector.  */
inline void wzyx(const component_type & w_, const component_type & z_, const vector< component_type, 2 > & yx_)
{ this->_w = w_; this->_z = z_; this->_y = yx_.y(); this->_x = yx_.x(); }

/**  Set multiple components of vector as a new vector.  */
inline void wzyx(const component_type & w_, const vector< component_type, 2 > & zy_, const component_type & x_)
{ this->_w = w_; this->_z = zy_.z(); this->_y = zy_.y(); this->_x = x_; }

/**  Set multiple components of vector as a new vector.  */
inline void wzyx(const vector< component_type, 2 > & wz_, const component_type & y_, const component_type & x_)
{ this->_w = wz_.w(); this->_z = wz_.z(); this->_y = y_; this->_x = x_; }

/**  Set multiple components of vector as a new vector.  */
inline void wzyx(const vector< component_type, 2 > & wz_, const vector< component_type, 2 > & yx_)
{ this->_w = wz_.w(); this->_z = wz_.z(); this->_y = yx_.y(); this->_x = yx_.x(); }

/**  Set multiple components of vector as a new vector.  */
inline void wzyx(const vector< component_type, 4 > & wzyx_)
{ this->_w = wzyx_.w(); this->_z = wzyx_.z(); this->_y = wzyx_.y(); this->_x = wzyx_.x(); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > wzyy() const
{ return vector< component_type, 4 >(this->_w, this->_z, this->_y, this->_y); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > wzyz() const
{ return vector< component_type, 4 >(this->_w, this->_z, this->_y, this->_z); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > wzyw() const
{ return vector< component_type, 4 >(this->_w, this->_z, this->_y, this->_w); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 3 > wzz() const
{ return vector< component_type, 3 >(this->_w, this->_z, this->_z); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > wzzx() const
{ return vector< component_type, 4 >(this->_w, this->_z, this->_z, this->_x); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > wzzy() const
{ return vector< component_type, 4 >(this->_w, this->_z, this->_z, this->_y); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > wzzz() const
{ return vector< component_type, 4 >(this->_w, this->_z, this->_z, this->_z); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > wzzw() const
{ return vector< component_type, 4 >(this->_w, this->_z, this->_z, this->_w); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 3 > wzw() const
{ return vector< component_type, 3 >(this->_w, this->_z, this->_w); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > wzwx() const
{ return vector< component_type, 4 >(this->_w, this->_z, this->_w, this->_x); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > wzwy() const
{ return vector< component_type, 4 >(this->_w, this->_z, this->_w, this->_y); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > wzwz() const
{ return vector< component_type, 4 >(this->_w, this->_z, this->_w, this->_z); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > wzww() const
{ return vector< component_type, 4 >(this->_w, this->_z, this->_w, this->_w); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 2 > ww() const
{ return vector< component_type, 2 >(this->_w, this->_w); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 3 > wwx() const
{ return vector< component_type, 3 >(this->_w, this->_w, this->_x); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > wwxx() const
{ return vector< component_type, 4 >(this->_w, this->_w, this->_x, this->_x); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > wwxy() const
{ return vector< component_type, 4 >(this->_w, this->_w, this->_x, this->_y); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > wwxz() const
{ return vector< component_type, 4 >(this->_w, this->_w, this->_x, this->_z); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > wwxw() const
{ return vector< component_type, 4 >(this->_w, this->_w, this->_x, this->_w); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 3 > wwy() const
{ return vector< component_type, 3 >(this->_w, this->_w, this->_y); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > wwyx() const
{ return vector< component_type, 4 >(this->_w, this->_w, this->_y, this->_x); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > wwyy() const
{ return vector< component_type, 4 >(this->_w, this->_w, this->_y, this->_y); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > wwyz() const
{ return vector< component_type, 4 >(this->_w, this->_w, this->_y, this->_z); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > wwyw() const
{ return vector< component_type, 4 >(this->_w, this->_w, this->_y, this->_w); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 3 > wwz() const
{ return vector< component_type, 3 >(this->_w, this->_w, this->_z); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > wwzx() const
{ return vector< component_type, 4 >(this->_w, this->_w, this->_z, this->_x); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > wwzy() const
{ return vector< component_type, 4 >(this->_w, this->_w, this->_z, this->_y); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > wwzz() const
{ return vector< component_type, 4 >(this->_w, this->_w, this->_z, this->_z); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > wwzw() const
{ return vector< component_type, 4 >(this->_w, this->_w, this->_z, this->_w); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 3 > www() const
{ return vector< component_type, 3 >(this->_w, this->_w, this->_w); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > wwwx() const
{ return vector< component_type, 4 >(this->_w, this->_w, this->_w, this->_x); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > wwwy() const
{ return vector< component_type, 4 >(this->_w, this->_w, this->_w, this->_y); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > wwwz() const
{ return vector< component_type, 4 >(this->_w, this->_w, this->_w, this->_z); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > wwww() const
{ return vector< component_type, 4 >(this->_w, this->_w, this->_w, this->_w); }
