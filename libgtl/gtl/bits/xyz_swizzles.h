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
inline vector< component_type, 4 > xxxz() const
{ return vector< component_type, 4 >(this->_x, this->_x, this->_x, this->_z); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > xxyz() const
{ return vector< component_type, 4 >(this->_x, this->_x, this->_y, this->_z); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 3 > xxz() const
{ return vector< component_type, 3 >(this->_x, this->_x, this->_z); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > xxzx() const
{ return vector< component_type, 4 >(this->_x, this->_x, this->_z, this->_x); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > xxzy() const
{ return vector< component_type, 4 >(this->_x, this->_x, this->_z, this->_y); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > xxzz() const
{ return vector< component_type, 4 >(this->_x, this->_x, this->_z, this->_z); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > xyxz() const
{ return vector< component_type, 4 >(this->_x, this->_y, this->_x, this->_z); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > xyyz() const
{ return vector< component_type, 4 >(this->_x, this->_y, this->_y, this->_z); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 3 > xyz() const
{ return vector< component_type, 3 >(this->_x, this->_y, this->_z); }

/**  Set multiple components of vector as a new vector.  */
inline void xyz(const component_type & x_, const component_type & y_, const component_type & z_)
{ this->_x = x_; this->_y = y_; this->_z = z_; }

/**  Set multiple components of vector as a new vector.  */
inline void xyz(const component_type & x_, const vector< component_type, 2 > & yz_)
{ this->_x = x_; this->_y = yz_.y(); this->_z = yz_.z(); }

/**  Set multiple components of vector as a new vector.  */
inline void xyz(const vector< component_type, 2 > & xy_, const component_type & z_)
{ this->_x = xy_.x(); this->_y = xy_.y(); this->_z = z_; }

/**  Set multiple components of vector as a new vector.  */
inline void xyz(const vector< component_type, 3 > & xyz_)
{ this->_x = xyz_.x(); this->_y = xyz_.y(); this->_z = xyz_.z(); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > xyzx() const
{ return vector< component_type, 4 >(this->_x, this->_y, this->_z, this->_x); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > xyzy() const
{ return vector< component_type, 4 >(this->_x, this->_y, this->_z, this->_y); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > xyzz() const
{ return vector< component_type, 4 >(this->_x, this->_y, this->_z, this->_z); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 2 > xz() const
{ return vector< component_type, 2 >(this->_x, this->_z); }

/**  Set multiple components of vector.  */
inline void xz(const component_type & x_, const component_type & z_)
{ this->_x = x_; this->_z = z_; }

/**  Set multiple components of vector.  */
inline void xz(const vector< component_type, 2 > & xz_)
{ this->_x = xz_.x(); this->_z = xz_.z(); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 3 > xzx() const
{ return vector< component_type, 3 >(this->_x, this->_z, this->_x); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > xzxx() const
{ return vector< component_type, 4 >(this->_x, this->_z, this->_x, this->_x); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > xzxy() const
{ return vector< component_type, 4 >(this->_x, this->_z, this->_x, this->_y); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > xzxz() const
{ return vector< component_type, 4 >(this->_x, this->_z, this->_x, this->_z); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 3 > xzy() const
{ return vector< component_type, 3 >(this->_x, this->_z, this->_y); }

/**  Set multiple components of vector as a new vector.  */
inline void xzy(const component_type & x_, const component_type & z_, const component_type & y_)
{ this->_x = x_; this->_z = z_; this->_y = y_; }

/**  Set multiple components of vector as a new vector.  */
inline void xzy(const component_type & x_, const vector< component_type, 2 > & zy_)
{ this->_x = x_; this->_z = zy_.z(); this->_y = zy_.y(); }

/**  Set multiple components of vector as a new vector.  */
inline void xzy(const vector< component_type, 2 > & xz_, const component_type & y_)
{ this->_x = xz_.x(); this->_z = xz_.z(); this->_y = y_; }

/**  Set multiple components of vector as a new vector.  */
inline void xzy(const vector< component_type, 3 > & xzy_)
{ this->_x = xzy_.x(); this->_z = xzy_.z(); this->_y = xzy_.y(); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > xzyx() const
{ return vector< component_type, 4 >(this->_x, this->_z, this->_y, this->_x); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > xzyy() const
{ return vector< component_type, 4 >(this->_x, this->_z, this->_y, this->_y); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > xzyz() const
{ return vector< component_type, 4 >(this->_x, this->_z, this->_y, this->_z); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 3 > xzz() const
{ return vector< component_type, 3 >(this->_x, this->_z, this->_z); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > xzzx() const
{ return vector< component_type, 4 >(this->_x, this->_z, this->_z, this->_x); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > xzzy() const
{ return vector< component_type, 4 >(this->_x, this->_z, this->_z, this->_y); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > xzzz() const
{ return vector< component_type, 4 >(this->_x, this->_z, this->_z, this->_z); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > yxxz() const
{ return vector< component_type, 4 >(this->_y, this->_x, this->_x, this->_z); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > yxyz() const
{ return vector< component_type, 4 >(this->_y, this->_x, this->_y, this->_z); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 3 > yxz() const
{ return vector< component_type, 3 >(this->_y, this->_x, this->_z); }

/**  Set multiple components of vector as a new vector.  */
inline void yxz(const component_type & y_, const component_type & x_, const component_type & z_)
{ this->_y = y_; this->_x = x_; this->_z = z_; }

/**  Set multiple components of vector as a new vector.  */
inline void yxz(const component_type & y_, const vector< component_type, 2 > & xz_)
{ this->_y = y_; this->_x = xz_.x(); this->_z = xz_.z(); }

/**  Set multiple components of vector as a new vector.  */
inline void yxz(const vector< component_type, 2 > & yx_, const component_type & z_)
{ this->_y = yx_.y(); this->_x = yx_.x(); this->_z = z_; }

/**  Set multiple components of vector as a new vector.  */
inline void yxz(const vector< component_type, 3 > & yxz_)
{ this->_y = yxz_.y(); this->_x = yxz_.x(); this->_z = yxz_.z(); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > yxzx() const
{ return vector< component_type, 4 >(this->_y, this->_x, this->_z, this->_x); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > yxzy() const
{ return vector< component_type, 4 >(this->_y, this->_x, this->_z, this->_y); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > yxzz() const
{ return vector< component_type, 4 >(this->_y, this->_x, this->_z, this->_z); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > yyxz() const
{ return vector< component_type, 4 >(this->_y, this->_y, this->_x, this->_z); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > yyyz() const
{ return vector< component_type, 4 >(this->_y, this->_y, this->_y, this->_z); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 3 > yyz() const
{ return vector< component_type, 3 >(this->_y, this->_y, this->_z); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > yyzx() const
{ return vector< component_type, 4 >(this->_y, this->_y, this->_z, this->_x); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > yyzy() const
{ return vector< component_type, 4 >(this->_y, this->_y, this->_z, this->_y); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > yyzz() const
{ return vector< component_type, 4 >(this->_y, this->_y, this->_z, this->_z); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 2 > yz() const
{ return vector< component_type, 2 >(this->_y, this->_z); }

/**  Set multiple components of vector.  */
inline void yz(const component_type & y_, const component_type & z_)
{ this->_y = y_; this->_z = z_; }

/**  Set multiple components of vector.  */
inline void yz(const vector< component_type, 2 > & yz_)
{ this->_y = yz_.y(); this->_z = yz_.z(); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 3 > yzx() const
{ return vector< component_type, 3 >(this->_y, this->_z, this->_x); }

/**  Set multiple components of vector as a new vector.  */
inline void yzx(const component_type & y_, const component_type & z_, const component_type & x_)
{ this->_y = y_; this->_z = z_; this->_x = x_; }

/**  Set multiple components of vector as a new vector.  */
inline void yzx(const component_type & y_, const vector< component_type, 2 > & zx_)
{ this->_y = y_; this->_z = zx_.z(); this->_x = zx_.x(); }

/**  Set multiple components of vector as a new vector.  */
inline void yzx(const vector< component_type, 2 > & yz_, const component_type & x_)
{ this->_y = yz_.y(); this->_z = yz_.z(); this->_x = x_; }

/**  Set multiple components of vector as a new vector.  */
inline void yzx(const vector< component_type, 3 > & yzx_)
{ this->_y = yzx_.y(); this->_z = yzx_.z(); this->_x = yzx_.x(); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > yzxx() const
{ return vector< component_type, 4 >(this->_y, this->_z, this->_x, this->_x); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > yzxy() const
{ return vector< component_type, 4 >(this->_y, this->_z, this->_x, this->_y); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > yzxz() const
{ return vector< component_type, 4 >(this->_y, this->_z, this->_x, this->_z); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 3 > yzy() const
{ return vector< component_type, 3 >(this->_y, this->_z, this->_y); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > yzyx() const
{ return vector< component_type, 4 >(this->_y, this->_z, this->_y, this->_x); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > yzyy() const
{ return vector< component_type, 4 >(this->_y, this->_z, this->_y, this->_y); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > yzyz() const
{ return vector< component_type, 4 >(this->_y, this->_z, this->_y, this->_z); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 3 > yzz() const
{ return vector< component_type, 3 >(this->_y, this->_z, this->_z); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > yzzx() const
{ return vector< component_type, 4 >(this->_y, this->_z, this->_z, this->_x); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > yzzy() const
{ return vector< component_type, 4 >(this->_y, this->_z, this->_z, this->_y); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > yzzz() const
{ return vector< component_type, 4 >(this->_y, this->_z, this->_z, this->_z); }

/**  Get 3rd component of vector.  */
inline component_type z() const
{ return this->_z; }

/**  Set 3rd component of vector to z_.  */
inline void z(const component_type & z_)
{ this->_z = z_; }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 2 > zx() const
{ return vector< component_type, 2 >(this->_z, this->_x); }

/**  Set multiple components of vector.  */
inline void zx(const component_type & z_, const component_type & x_)
{ this->_z = z_; this->_x = x_; }

/**  Set multiple components of vector.  */
inline void zx(const vector< component_type, 2 > & zx_)
{ this->_z = zx_.z(); this->_x = zx_.x(); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 3 > zxx() const
{ return vector< component_type, 3 >(this->_z, this->_x, this->_x); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > zxxx() const
{ return vector< component_type, 4 >(this->_z, this->_x, this->_x, this->_x); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > zxxy() const
{ return vector< component_type, 4 >(this->_z, this->_x, this->_x, this->_y); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > zxxz() const
{ return vector< component_type, 4 >(this->_z, this->_x, this->_x, this->_z); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 3 > zxy() const
{ return vector< component_type, 3 >(this->_z, this->_x, this->_y); }

/**  Set multiple components of vector as a new vector.  */
inline void zxy(const component_type & z_, const component_type & x_, const component_type & y_)
{ this->_z = z_; this->_x = x_; this->_y = y_; }

/**  Set multiple components of vector as a new vector.  */
inline void zxy(const component_type & z_, const vector< component_type, 2 > & xy_)
{ this->_z = z_; this->_x = xy_.x(); this->_y = xy_.y(); }

/**  Set multiple components of vector as a new vector.  */
inline void zxy(const vector< component_type, 2 > & zx_, const component_type & y_)
{ this->_z = zx_.z(); this->_x = zx_.x(); this->_y = y_; }

/**  Set multiple components of vector as a new vector.  */
inline void zxy(const vector< component_type, 3 > & zxy_)
{ this->_z = zxy_.z(); this->_x = zxy_.x(); this->_y = zxy_.y(); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > zxyx() const
{ return vector< component_type, 4 >(this->_z, this->_x, this->_y, this->_x); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > zxyy() const
{ return vector< component_type, 4 >(this->_z, this->_x, this->_y, this->_y); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > zxyz() const
{ return vector< component_type, 4 >(this->_z, this->_x, this->_y, this->_z); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 3 > zxz() const
{ return vector< component_type, 3 >(this->_z, this->_x, this->_z); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > zxzx() const
{ return vector< component_type, 4 >(this->_z, this->_x, this->_z, this->_x); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > zxzy() const
{ return vector< component_type, 4 >(this->_z, this->_x, this->_z, this->_y); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > zxzz() const
{ return vector< component_type, 4 >(this->_z, this->_x, this->_z, this->_z); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 2 > zy() const
{ return vector< component_type, 2 >(this->_z, this->_y); }

/**  Set multiple components of vector.  */
inline void zy(const component_type & z_, const component_type & y_)
{ this->_z = z_; this->_y = y_; }

/**  Set multiple components of vector.  */
inline void zy(const vector< component_type, 2 > & zy_)
{ this->_z = zy_.z(); this->_y = zy_.y(); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 3 > zyx() const
{ return vector< component_type, 3 >(this->_z, this->_y, this->_x); }

/**  Set multiple components of vector as a new vector.  */
inline void zyx(const component_type & z_, const component_type & y_, const component_type & x_)
{ this->_z = z_; this->_y = y_; this->_x = x_; }

/**  Set multiple components of vector as a new vector.  */
inline void zyx(const component_type & z_, const vector< component_type, 2 > & yx_)
{ this->_z = z_; this->_y = yx_.y(); this->_x = yx_.x(); }

/**  Set multiple components of vector as a new vector.  */
inline void zyx(const vector< component_type, 2 > & zy_, const component_type & x_)
{ this->_z = zy_.z(); this->_y = zy_.y(); this->_x = x_; }

/**  Set multiple components of vector as a new vector.  */
inline void zyx(const vector< component_type, 3 > & zyx_)
{ this->_z = zyx_.z(); this->_y = zyx_.y(); this->_x = zyx_.x(); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > zyxx() const
{ return vector< component_type, 4 >(this->_z, this->_y, this->_x, this->_x); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > zyxy() const
{ return vector< component_type, 4 >(this->_z, this->_y, this->_x, this->_y); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > zyxz() const
{ return vector< component_type, 4 >(this->_z, this->_y, this->_x, this->_z); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 3 > zyy() const
{ return vector< component_type, 3 >(this->_z, this->_y, this->_y); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > zyyx() const
{ return vector< component_type, 4 >(this->_z, this->_y, this->_y, this->_x); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > zyyy() const
{ return vector< component_type, 4 >(this->_z, this->_y, this->_y, this->_y); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > zyyz() const
{ return vector< component_type, 4 >(this->_z, this->_y, this->_y, this->_z); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 3 > zyz() const
{ return vector< component_type, 3 >(this->_z, this->_y, this->_z); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > zyzx() const
{ return vector< component_type, 4 >(this->_z, this->_y, this->_z, this->_x); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > zyzy() const
{ return vector< component_type, 4 >(this->_z, this->_y, this->_z, this->_y); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > zyzz() const
{ return vector< component_type, 4 >(this->_z, this->_y, this->_z, this->_z); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 2 > zz() const
{ return vector< component_type, 2 >(this->_z, this->_z); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 3 > zzx() const
{ return vector< component_type, 3 >(this->_z, this->_z, this->_x); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > zzxx() const
{ return vector< component_type, 4 >(this->_z, this->_z, this->_x, this->_x); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > zzxy() const
{ return vector< component_type, 4 >(this->_z, this->_z, this->_x, this->_y); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > zzxz() const
{ return vector< component_type, 4 >(this->_z, this->_z, this->_x, this->_z); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 3 > zzy() const
{ return vector< component_type, 3 >(this->_z, this->_z, this->_y); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > zzyx() const
{ return vector< component_type, 4 >(this->_z, this->_z, this->_y, this->_x); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > zzyy() const
{ return vector< component_type, 4 >(this->_z, this->_z, this->_y, this->_y); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > zzyz() const
{ return vector< component_type, 4 >(this->_z, this->_z, this->_y, this->_z); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 3 > zzz() const
{ return vector< component_type, 3 >(this->_z, this->_z, this->_z); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > zzzx() const
{ return vector< component_type, 4 >(this->_z, this->_z, this->_z, this->_x); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > zzzy() const
{ return vector< component_type, 4 >(this->_z, this->_z, this->_z, this->_y); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > zzzz() const
{ return vector< component_type, 4 >(this->_z, this->_z, this->_z, this->_z); }
