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
inline component_type r() const
{ return this->_x; }

/**  Set 1st component of vector to r_.  */
inline void r(const component_type & r_)
{ this->_x = r_; }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 2 > rr() const
{ return vector< component_type, 2 >(this->_x, this->_x); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 3 > rrr() const
{ return vector< component_type, 3 >(this->_x, this->_x, this->_x); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > rrrr() const
{ return vector< component_type, 4 >(this->_x, this->_x, this->_x, this->_x); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > rrrg() const
{ return vector< component_type, 4 >(this->_x, this->_x, this->_x, this->_y); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > rrrb() const
{ return vector< component_type, 4 >(this->_x, this->_x, this->_x, this->_z); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 3 > rrg() const
{ return vector< component_type, 3 >(this->_x, this->_x, this->_y); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > rrgr() const
{ return vector< component_type, 4 >(this->_x, this->_x, this->_y, this->_x); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > rrgg() const
{ return vector< component_type, 4 >(this->_x, this->_x, this->_y, this->_y); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > rrgb() const
{ return vector< component_type, 4 >(this->_x, this->_x, this->_y, this->_z); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 3 > rrb() const
{ return vector< component_type, 3 >(this->_x, this->_x, this->_z); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > rrbr() const
{ return vector< component_type, 4 >(this->_x, this->_x, this->_z, this->_x); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > rrbg() const
{ return vector< component_type, 4 >(this->_x, this->_x, this->_z, this->_y); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > rrbb() const
{ return vector< component_type, 4 >(this->_x, this->_x, this->_z, this->_z); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 2 > rg() const
{ return vector< component_type, 2 >(this->_x, this->_y); }

/**  Set multiple components of vector.  */
inline void rg(const component_type & r_, const component_type & g_)
{ this->_x = r_; this->_y = g_; }

/**  Set multiple components of vector.  */
inline void rg(const vector< component_type, 2 > & rg_)
{ this->_x = rg_.r(); this->_y = rg_.g(); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 3 > rgr() const
{ return vector< component_type, 3 >(this->_x, this->_y, this->_x); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > rgrr() const
{ return vector< component_type, 4 >(this->_x, this->_y, this->_x, this->_x); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > rgrg() const
{ return vector< component_type, 4 >(this->_x, this->_y, this->_x, this->_y); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > rgrb() const
{ return vector< component_type, 4 >(this->_x, this->_y, this->_x, this->_z); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 3 > rgg() const
{ return vector< component_type, 3 >(this->_x, this->_y, this->_y); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > rggr() const
{ return vector< component_type, 4 >(this->_x, this->_y, this->_y, this->_x); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > rggg() const
{ return vector< component_type, 4 >(this->_x, this->_y, this->_y, this->_y); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > rggb() const
{ return vector< component_type, 4 >(this->_x, this->_y, this->_y, this->_z); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 3 > rgb() const
{ return vector< component_type, 3 >(this->_x, this->_y, this->_z); }

/**  Set multiple components of vector as a new vector.  */
inline void rgb(const component_type & r_, const component_type & g_, const component_type & b_)
{ this->_x = r_; this->_y = g_; this->_z = b_; }

/**  Set multiple components of vector as a new vector.  */
inline void rgb(const component_type & r_, const vector< component_type, 2 > & gb_)
{ this->_x = r_; this->_y = gb_.g(); this->_z = gb_.b(); }

/**  Set multiple components of vector as a new vector.  */
inline void rgb(const vector< component_type, 2 > & rg_, const component_type & b_)
{ this->_x = rg_.r(); this->_y = rg_.g(); this->_z = b_; }

/**  Set multiple components of vector as a new vector.  */
inline void rgb(const vector< component_type, 3 > & rgb_)
{ this->_x = rgb_.r(); this->_y = rgb_.g(); this->_z = rgb_.b(); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > rgbr() const
{ return vector< component_type, 4 >(this->_x, this->_y, this->_z, this->_x); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > rgbg() const
{ return vector< component_type, 4 >(this->_x, this->_y, this->_z, this->_y); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > rgbb() const
{ return vector< component_type, 4 >(this->_x, this->_y, this->_z, this->_z); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 2 > rb() const
{ return vector< component_type, 2 >(this->_x, this->_z); }

/**  Set multiple components of vector.  */
inline void rb(const component_type & r_, const component_type & b_)
{ this->_x = r_; this->_z = b_; }

/**  Set multiple components of vector.  */
inline void rb(const vector< component_type, 2 > & rb_)
{ this->_x = rb_.r(); this->_z = rb_.b(); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 3 > rbr() const
{ return vector< component_type, 3 >(this->_x, this->_z, this->_x); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > rbrr() const
{ return vector< component_type, 4 >(this->_x, this->_z, this->_x, this->_x); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > rbrg() const
{ return vector< component_type, 4 >(this->_x, this->_z, this->_x, this->_y); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > rbrb() const
{ return vector< component_type, 4 >(this->_x, this->_z, this->_x, this->_z); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 3 > rbg() const
{ return vector< component_type, 3 >(this->_x, this->_z, this->_y); }

/**  Set multiple components of vector as a new vector.  */
inline void rbg(const component_type & r_, const component_type & b_, const component_type & g_)
{ this->_x = r_; this->_z = b_; this->_y = g_; }

/**  Set multiple components of vector as a new vector.  */
inline void rbg(const component_type & r_, const vector< component_type, 2 > & bg_)
{ this->_x = r_; this->_z = bg_.b(); this->_y = bg_.g(); }

/**  Set multiple components of vector as a new vector.  */
inline void rbg(const vector< component_type, 2 > & rb_, const component_type & g_)
{ this->_x = rb_.r(); this->_z = rb_.b(); this->_y = g_; }

/**  Set multiple components of vector as a new vector.  */
inline void rbg(const vector< component_type, 3 > & rbg_)
{ this->_x = rbg_.r(); this->_z = rbg_.b(); this->_y = rbg_.g(); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > rbgr() const
{ return vector< component_type, 4 >(this->_x, this->_z, this->_y, this->_x); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > rbgg() const
{ return vector< component_type, 4 >(this->_x, this->_z, this->_y, this->_y); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > rbgb() const
{ return vector< component_type, 4 >(this->_x, this->_z, this->_y, this->_z); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 3 > rbb() const
{ return vector< component_type, 3 >(this->_x, this->_z, this->_z); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > rbbr() const
{ return vector< component_type, 4 >(this->_x, this->_z, this->_z, this->_x); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > rbbg() const
{ return vector< component_type, 4 >(this->_x, this->_z, this->_z, this->_y); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > rbbb() const
{ return vector< component_type, 4 >(this->_x, this->_z, this->_z, this->_z); }

/**  Get 2nd component of vector.  */
inline component_type g() const
{ return this->_y; }

/**  Set 2nd component of vector to g_.  */
inline void g(const component_type & g_)
{ this->_y = g_; }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 2 > gr() const
{ return vector< component_type, 2 >(this->_y, this->_x); }

/**  Set multiple components of vector.  */
inline void gr(const component_type & g_, const component_type & r_)
{ this->_y = g_; this->_x = r_; }

/**  Set multiple components of vector.  */
inline void gr(const vector< component_type, 2 > & gr_)
{ this->_y = gr_.g(); this->_x = gr_.r(); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 3 > grr() const
{ return vector< component_type, 3 >(this->_y, this->_x, this->_x); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > grrr() const
{ return vector< component_type, 4 >(this->_y, this->_x, this->_x, this->_x); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > grrg() const
{ return vector< component_type, 4 >(this->_y, this->_x, this->_x, this->_y); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > grrb() const
{ return vector< component_type, 4 >(this->_y, this->_x, this->_x, this->_z); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 3 > grg() const
{ return vector< component_type, 3 >(this->_y, this->_x, this->_y); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > grgr() const
{ return vector< component_type, 4 >(this->_y, this->_x, this->_y, this->_x); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > grgg() const
{ return vector< component_type, 4 >(this->_y, this->_x, this->_y, this->_y); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > grgb() const
{ return vector< component_type, 4 >(this->_y, this->_x, this->_y, this->_z); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 3 > grb() const
{ return vector< component_type, 3 >(this->_y, this->_x, this->_z); }

/**  Set multiple components of vector as a new vector.  */
inline void grb(const component_type & g_, const component_type & r_, const component_type & b_)
{ this->_y = g_; this->_x = r_; this->_z = b_; }

/**  Set multiple components of vector as a new vector.  */
inline void grb(const component_type & g_, const vector< component_type, 2 > & rb_)
{ this->_y = g_; this->_x = rb_.r(); this->_z = rb_.b(); }

/**  Set multiple components of vector as a new vector.  */
inline void grb(const vector< component_type, 2 > & gr_, const component_type & b_)
{ this->_y = gr_.g(); this->_x = gr_.r(); this->_z = b_; }

/**  Set multiple components of vector as a new vector.  */
inline void grb(const vector< component_type, 3 > & grb_)
{ this->_y = grb_.g(); this->_x = grb_.r(); this->_z = grb_.b(); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > grbr() const
{ return vector< component_type, 4 >(this->_y, this->_x, this->_z, this->_x); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > grbg() const
{ return vector< component_type, 4 >(this->_y, this->_x, this->_z, this->_y); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > grbb() const
{ return vector< component_type, 4 >(this->_y, this->_x, this->_z, this->_z); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 2 > gg() const
{ return vector< component_type, 2 >(this->_y, this->_y); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 3 > ggr() const
{ return vector< component_type, 3 >(this->_y, this->_y, this->_x); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > ggrr() const
{ return vector< component_type, 4 >(this->_y, this->_y, this->_x, this->_x); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > ggrg() const
{ return vector< component_type, 4 >(this->_y, this->_y, this->_x, this->_y); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > ggrb() const
{ return vector< component_type, 4 >(this->_y, this->_y, this->_x, this->_z); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 3 > ggg() const
{ return vector< component_type, 3 >(this->_y, this->_y, this->_y); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > gggr() const
{ return vector< component_type, 4 >(this->_y, this->_y, this->_y, this->_x); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > gggg() const
{ return vector< component_type, 4 >(this->_y, this->_y, this->_y, this->_y); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > gggb() const
{ return vector< component_type, 4 >(this->_y, this->_y, this->_y, this->_z); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 3 > ggb() const
{ return vector< component_type, 3 >(this->_y, this->_y, this->_z); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > ggbr() const
{ return vector< component_type, 4 >(this->_y, this->_y, this->_z, this->_x); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > ggbg() const
{ return vector< component_type, 4 >(this->_y, this->_y, this->_z, this->_y); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > ggbb() const
{ return vector< component_type, 4 >(this->_y, this->_y, this->_z, this->_z); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 2 > gb() const
{ return vector< component_type, 2 >(this->_y, this->_z); }

/**  Set multiple components of vector.  */
inline void gb(const component_type & g_, const component_type & b_)
{ this->_y = g_; this->_z = b_; }

/**  Set multiple components of vector.  */
inline void gb(const vector< component_type, 2 > & gb_)
{ this->_y = gb_.g(); this->_z = gb_.b(); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 3 > gbr() const
{ return vector< component_type, 3 >(this->_y, this->_z, this->_x); }

/**  Set multiple components of vector as a new vector.  */
inline void gbr(const component_type & g_, const component_type & b_, const component_type & r_)
{ this->_y = g_; this->_z = b_; this->_x = r_; }

/**  Set multiple components of vector as a new vector.  */
inline void gbr(const component_type & g_, const vector< component_type, 2 > & br_)
{ this->_y = g_; this->_z = br_.b(); this->_x = br_.r(); }

/**  Set multiple components of vector as a new vector.  */
inline void gbr(const vector< component_type, 2 > & gb_, const component_type & r_)
{ this->_y = gb_.g(); this->_z = gb_.b(); this->_x = r_; }

/**  Set multiple components of vector as a new vector.  */
inline void gbr(const vector< component_type, 3 > & gbr_)
{ this->_y = gbr_.g(); this->_z = gbr_.b(); this->_x = gbr_.r(); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > gbrr() const
{ return vector< component_type, 4 >(this->_y, this->_z, this->_x, this->_x); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > gbrg() const
{ return vector< component_type, 4 >(this->_y, this->_z, this->_x, this->_y); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > gbrb() const
{ return vector< component_type, 4 >(this->_y, this->_z, this->_x, this->_z); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 3 > gbg() const
{ return vector< component_type, 3 >(this->_y, this->_z, this->_y); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > gbgr() const
{ return vector< component_type, 4 >(this->_y, this->_z, this->_y, this->_x); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > gbgg() const
{ return vector< component_type, 4 >(this->_y, this->_z, this->_y, this->_y); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > gbgb() const
{ return vector< component_type, 4 >(this->_y, this->_z, this->_y, this->_z); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 3 > gbb() const
{ return vector< component_type, 3 >(this->_y, this->_z, this->_z); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > gbbr() const
{ return vector< component_type, 4 >(this->_y, this->_z, this->_z, this->_x); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > gbbg() const
{ return vector< component_type, 4 >(this->_y, this->_z, this->_z, this->_y); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > gbbb() const
{ return vector< component_type, 4 >(this->_y, this->_z, this->_z, this->_z); }

/**  Get 3rd component of vector.  */
inline component_type b() const
{ return this->_z; }

/**  Set 3rd component of vector to b_.  */
inline void b(const component_type & b_)
{ this->_z = b_; }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 2 > br() const
{ return vector< component_type, 2 >(this->_z, this->_x); }

/**  Set multiple components of vector.  */
inline void br(const component_type & b_, const component_type & r_)
{ this->_z = b_; this->_x = r_; }

/**  Set multiple components of vector.  */
inline void br(const vector< component_type, 2 > & br_)
{ this->_z = br_.b(); this->_x = br_.r(); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 3 > brr() const
{ return vector< component_type, 3 >(this->_z, this->_x, this->_x); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > brrr() const
{ return vector< component_type, 4 >(this->_z, this->_x, this->_x, this->_x); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > brrg() const
{ return vector< component_type, 4 >(this->_z, this->_x, this->_x, this->_y); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > brrb() const
{ return vector< component_type, 4 >(this->_z, this->_x, this->_x, this->_z); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 3 > brg() const
{ return vector< component_type, 3 >(this->_z, this->_x, this->_y); }

/**  Set multiple components of vector as a new vector.  */
inline void brg(const component_type & b_, const component_type & r_, const component_type & g_)
{ this->_z = b_; this->_x = r_; this->_y = g_; }

/**  Set multiple components of vector as a new vector.  */
inline void brg(const component_type & b_, const vector< component_type, 2 > & rg_)
{ this->_z = b_; this->_x = rg_.r(); this->_y = rg_.g(); }

/**  Set multiple components of vector as a new vector.  */
inline void brg(const vector< component_type, 2 > & br_, const component_type & g_)
{ this->_z = br_.b(); this->_x = br_.r(); this->_y = g_; }

/**  Set multiple components of vector as a new vector.  */
inline void brg(const vector< component_type, 3 > & brg_)
{ this->_z = brg_.b(); this->_x = brg_.r(); this->_y = brg_.g(); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > brgr() const
{ return vector< component_type, 4 >(this->_z, this->_x, this->_y, this->_x); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > brgg() const
{ return vector< component_type, 4 >(this->_z, this->_x, this->_y, this->_y); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > brgb() const
{ return vector< component_type, 4 >(this->_z, this->_x, this->_y, this->_z); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 3 > brb() const
{ return vector< component_type, 3 >(this->_z, this->_x, this->_z); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > brbr() const
{ return vector< component_type, 4 >(this->_z, this->_x, this->_z, this->_x); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > brbg() const
{ return vector< component_type, 4 >(this->_z, this->_x, this->_z, this->_y); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > brbb() const
{ return vector< component_type, 4 >(this->_z, this->_x, this->_z, this->_z); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 2 > bg() const
{ return vector< component_type, 2 >(this->_z, this->_y); }

/**  Set multiple components of vector.  */
inline void bg(const component_type & b_, const component_type & g_)
{ this->_z = b_; this->_y = g_; }

/**  Set multiple components of vector.  */
inline void bg(const vector< component_type, 2 > & bg_)
{ this->_z = bg_.b(); this->_y = bg_.g(); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 3 > bgr() const
{ return vector< component_type, 3 >(this->_z, this->_y, this->_x); }

/**  Set multiple components of vector as a new vector.  */
inline void bgr(const component_type & b_, const component_type & g_, const component_type & r_)
{ this->_z = b_; this->_y = g_; this->_x = r_; }

/**  Set multiple components of vector as a new vector.  */
inline void bgr(const component_type & b_, const vector< component_type, 2 > & gr_)
{ this->_z = b_; this->_y = gr_.g(); this->_x = gr_.r(); }

/**  Set multiple components of vector as a new vector.  */
inline void bgr(const vector< component_type, 2 > & bg_, const component_type & r_)
{ this->_z = bg_.b(); this->_y = bg_.g(); this->_x = r_; }

/**  Set multiple components of vector as a new vector.  */
inline void bgr(const vector< component_type, 3 > & bgr_)
{ this->_z = bgr_.b(); this->_y = bgr_.g(); this->_x = bgr_.r(); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > bgrr() const
{ return vector< component_type, 4 >(this->_z, this->_y, this->_x, this->_x); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > bgrg() const
{ return vector< component_type, 4 >(this->_z, this->_y, this->_x, this->_y); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > bgrb() const
{ return vector< component_type, 4 >(this->_z, this->_y, this->_x, this->_z); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 3 > bgg() const
{ return vector< component_type, 3 >(this->_z, this->_y, this->_y); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > bggr() const
{ return vector< component_type, 4 >(this->_z, this->_y, this->_y, this->_x); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > bggg() const
{ return vector< component_type, 4 >(this->_z, this->_y, this->_y, this->_y); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > bggb() const
{ return vector< component_type, 4 >(this->_z, this->_y, this->_y, this->_z); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 3 > bgb() const
{ return vector< component_type, 3 >(this->_z, this->_y, this->_z); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > bgbr() const
{ return vector< component_type, 4 >(this->_z, this->_y, this->_z, this->_x); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > bgbg() const
{ return vector< component_type, 4 >(this->_z, this->_y, this->_z, this->_y); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > bgbb() const
{ return vector< component_type, 4 >(this->_z, this->_y, this->_z, this->_z); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 2 > bb() const
{ return vector< component_type, 2 >(this->_z, this->_z); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 3 > bbr() const
{ return vector< component_type, 3 >(this->_z, this->_z, this->_x); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > bbrr() const
{ return vector< component_type, 4 >(this->_z, this->_z, this->_x, this->_x); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > bbrg() const
{ return vector< component_type, 4 >(this->_z, this->_z, this->_x, this->_y); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > bbrb() const
{ return vector< component_type, 4 >(this->_z, this->_z, this->_x, this->_z); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 3 > bbg() const
{ return vector< component_type, 3 >(this->_z, this->_z, this->_y); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > bbgr() const
{ return vector< component_type, 4 >(this->_z, this->_z, this->_y, this->_x); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > bbgg() const
{ return vector< component_type, 4 >(this->_z, this->_z, this->_y, this->_y); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > bbgb() const
{ return vector< component_type, 4 >(this->_z, this->_z, this->_y, this->_z); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 3 > bbb() const
{ return vector< component_type, 3 >(this->_z, this->_z, this->_z); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > bbbr() const
{ return vector< component_type, 4 >(this->_z, this->_z, this->_z, this->_x); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > bbbg() const
{ return vector< component_type, 4 >(this->_z, this->_z, this->_z, this->_y); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > bbbb() const
{ return vector< component_type, 4 >(this->_z, this->_z, this->_z, this->_z); }
