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
inline vector< component_type, 4 > rrra() const
{ return vector< component_type, 4 >(this->_x, this->_x, this->_x, this->_w); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > rrga() const
{ return vector< component_type, 4 >(this->_x, this->_x, this->_y, this->_w); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > rrba() const
{ return vector< component_type, 4 >(this->_x, this->_x, this->_z, this->_w); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 3 > rra() const
{ return vector< component_type, 3 >(this->_x, this->_x, this->_w); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > rrar() const
{ return vector< component_type, 4 >(this->_x, this->_x, this->_w, this->_x); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > rrag() const
{ return vector< component_type, 4 >(this->_x, this->_x, this->_w, this->_y); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > rrab() const
{ return vector< component_type, 4 >(this->_x, this->_x, this->_w, this->_z); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > rraa() const
{ return vector< component_type, 4 >(this->_x, this->_x, this->_w, this->_w); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > rgra() const
{ return vector< component_type, 4 >(this->_x, this->_y, this->_x, this->_w); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > rgga() const
{ return vector< component_type, 4 >(this->_x, this->_y, this->_y, this->_w); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > rgba() const
{ return vector< component_type, 4 >(this->_x, this->_y, this->_z, this->_w); }

/**  Set multiple components of vector as a new vector.  */
inline void rgba(const component_type & r_, const component_type & g_, const component_type & b_, const component_type & a_)
{ this->_x = r_; this->_y = g_; this->_z = b_; this->_w = a_; }

/**  Set multiple components of vector as a new vector.  */
inline void rgba(const component_type & r_, const vector< component_type, 3 > & gba_)
{ this->_x = r_; this->_y = gba_.g(); this->_z = gba_.b(); this->_w = gba_.a(); }

/**  Set multiple components of vector as a new vector.  */
inline void rgba(const vector< component_type, 3 > & rgb_, const component_type & a_)
{ this->_x = rgb_.r(); this->_y = rgb_.g(); this->_z = rgb_.b(); this->_w = a_; }

/**  Set multiple components of vector as a new vector.  */
inline void rgba(const component_type & r_, const component_type & g_, const vector< component_type, 2 > & ba_)
{ this->_x = r_; this->_y = g_; this->_z = ba_.b(); this->_w = ba_.a(); }

/**  Set multiple components of vector as a new vector.  */
inline void rgba(const component_type & r_, const vector< component_type, 2 > & gb_, const component_type & a_)
{ this->_x = r_; this->_y = gb_.g(); this->_z = gb_.b(); this->_w = a_; }

/**  Set multiple components of vector as a new vector.  */
inline void rgba(const vector< component_type, 2 > & rg_, const component_type & b_, const component_type & a_)
{ this->_x = rg_.r(); this->_y = rg_.g(); this->_z = b_; this->_w = a_; }

/**  Set multiple components of vector as a new vector.  */
inline void rgba(const vector< component_type, 2 > & rg_, const vector< component_type, 2 > & ba_)
{ this->_x = rg_.r(); this->_y = rg_.g(); this->_z = ba_.b(); this->_w = ba_.a(); }

/**  Set multiple components of vector as a new vector.  */
inline void rgba(const vector< component_type, 4 > & rgba_)
{ this->_x = rgba_.r(); this->_y = rgba_.g(); this->_z = rgba_.b(); this->_w = rgba_.a(); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 3 > rga() const
{ return vector< component_type, 3 >(this->_x, this->_y, this->_w); }

/**  Set multiple components of vector as a new vector.  */
inline void rga(const component_type & r_, const component_type & g_, const component_type & a_)
{ this->_x = r_; this->_y = g_; this->_w = a_; }

/**  Set multiple components of vector as a new vector.  */
inline void rga(const component_type & r_, const vector< component_type, 2 > & ga_)
{ this->_x = r_; this->_y = ga_.g(); this->_w = ga_.a(); }

/**  Set multiple components of vector as a new vector.  */
inline void rga(const vector< component_type, 2 > & rg_, const component_type & a_)
{ this->_x = rg_.r(); this->_y = rg_.g(); this->_w = a_; }

/**  Set multiple components of vector as a new vector.  */
inline void rga(const vector< component_type, 3 > & rga_)
{ this->_x = rga_.r(); this->_y = rga_.g(); this->_w = rga_.a(); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > rgar() const
{ return vector< component_type, 4 >(this->_x, this->_y, this->_w, this->_x); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > rgag() const
{ return vector< component_type, 4 >(this->_x, this->_y, this->_w, this->_y); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > rgab() const
{ return vector< component_type, 4 >(this->_x, this->_y, this->_w, this->_z); }

/**  Set multiple components of vector as a new vector.  */
inline void rgab(const component_type & r_, const component_type & g_, const component_type & a_, const component_type & b_)
{ this->_x = r_; this->_y = g_; this->_w = a_; this->_z = b_; }

/**  Set multiple components of vector as a new vector.  */
inline void rgab(const component_type & r_, const vector< component_type, 3 > & gab_)
{ this->_x = r_; this->_y = gab_.g(); this->_w = gab_.a(); this->_z = gab_.b(); }

/**  Set multiple components of vector as a new vector.  */
inline void rgab(const vector< component_type, 3 > & rga_, const component_type & b_)
{ this->_x = rga_.r(); this->_y = rga_.g(); this->_w = rga_.a(); this->_z = b_; }

/**  Set multiple components of vector as a new vector.  */
inline void rgab(const component_type & r_, const component_type & g_, const vector< component_type, 2 > & ab_)
{ this->_x = r_; this->_y = g_; this->_w = ab_.a(); this->_z = ab_.b(); }

/**  Set multiple components of vector as a new vector.  */
inline void rgab(const component_type & r_, const vector< component_type, 2 > & ga_, const component_type & b_)
{ this->_x = r_; this->_y = ga_.g(); this->_w = ga_.a(); this->_z = b_; }

/**  Set multiple components of vector as a new vector.  */
inline void rgab(const vector< component_type, 2 > & rg_, const component_type & a_, const component_type & b_)
{ this->_x = rg_.r(); this->_y = rg_.g(); this->_w = a_; this->_z = b_; }

/**  Set multiple components of vector as a new vector.  */
inline void rgab(const vector< component_type, 2 > & rg_, const vector< component_type, 2 > & ab_)
{ this->_x = rg_.r(); this->_y = rg_.g(); this->_w = ab_.a(); this->_z = ab_.b(); }

/**  Set multiple components of vector as a new vector.  */
inline void rgab(const vector< component_type, 4 > & rgab_)
{ this->_x = rgab_.r(); this->_y = rgab_.g(); this->_w = rgab_.a(); this->_z = rgab_.b(); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > rgaa() const
{ return vector< component_type, 4 >(this->_x, this->_y, this->_w, this->_w); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > rbra() const
{ return vector< component_type, 4 >(this->_x, this->_z, this->_x, this->_w); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > rbga() const
{ return vector< component_type, 4 >(this->_x, this->_z, this->_y, this->_w); }

/**  Set multiple components of vector as a new vector.  */
inline void rbga(const component_type & r_, const component_type & b_, const component_type & g_, const component_type & a_)
{ this->_x = r_; this->_z = b_; this->_y = g_; this->_w = a_; }

/**  Set multiple components of vector as a new vector.  */
inline void rbga(const component_type & r_, const vector< component_type, 3 > & bga_)
{ this->_x = r_; this->_z = bga_.b(); this->_y = bga_.g(); this->_w = bga_.a(); }

/**  Set multiple components of vector as a new vector.  */
inline void rbga(const vector< component_type, 3 > & rbg_, const component_type & a_)
{ this->_x = rbg_.r(); this->_z = rbg_.b(); this->_y = rbg_.g(); this->_w = a_; }

/**  Set multiple components of vector as a new vector.  */
inline void rbga(const component_type & r_, const component_type & b_, const vector< component_type, 2 > & ga_)
{ this->_x = r_; this->_z = b_; this->_y = ga_.g(); this->_w = ga_.a(); }

/**  Set multiple components of vector as a new vector.  */
inline void rbga(const component_type & r_, const vector< component_type, 2 > & bg_, const component_type & a_)
{ this->_x = r_; this->_z = bg_.b(); this->_y = bg_.g(); this->_w = a_; }

/**  Set multiple components of vector as a new vector.  */
inline void rbga(const vector< component_type, 2 > & rb_, const component_type & g_, const component_type & a_)
{ this->_x = rb_.r(); this->_z = rb_.b(); this->_y = g_; this->_w = a_; }

/**  Set multiple components of vector as a new vector.  */
inline void rbga(const vector< component_type, 2 > & rb_, const vector< component_type, 2 > & ga_)
{ this->_x = rb_.r(); this->_z = rb_.b(); this->_y = ga_.g(); this->_w = ga_.a(); }

/**  Set multiple components of vector as a new vector.  */
inline void rbga(const vector< component_type, 4 > & rbga_)
{ this->_x = rbga_.r(); this->_z = rbga_.b(); this->_y = rbga_.g(); this->_w = rbga_.a(); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > rbba() const
{ return vector< component_type, 4 >(this->_x, this->_z, this->_z, this->_w); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 3 > rba() const
{ return vector< component_type, 3 >(this->_x, this->_z, this->_w); }

/**  Set multiple components of vector as a new vector.  */
inline void rba(const component_type & r_, const component_type & b_, const component_type & a_)
{ this->_x = r_; this->_z = b_; this->_w = a_; }

/**  Set multiple components of vector as a new vector.  */
inline void rba(const component_type & r_, const vector< component_type, 2 > & ba_)
{ this->_x = r_; this->_z = ba_.b(); this->_w = ba_.a(); }

/**  Set multiple components of vector as a new vector.  */
inline void rba(const vector< component_type, 2 > & rb_, const component_type & a_)
{ this->_x = rb_.r(); this->_z = rb_.b(); this->_w = a_; }

/**  Set multiple components of vector as a new vector.  */
inline void rba(const vector< component_type, 3 > & rba_)
{ this->_x = rba_.r(); this->_z = rba_.b(); this->_w = rba_.a(); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > rbar() const
{ return vector< component_type, 4 >(this->_x, this->_z, this->_w, this->_x); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > rbag() const
{ return vector< component_type, 4 >(this->_x, this->_z, this->_w, this->_y); }

/**  Set multiple components of vector as a new vector.  */
inline void rbag(const component_type & r_, const component_type & b_, const component_type & a_, const component_type & g_)
{ this->_x = r_; this->_z = b_; this->_w = a_; this->_y = g_; }

/**  Set multiple components of vector as a new vector.  */
inline void rbag(const component_type & r_, const vector< component_type, 3 > & bag_)
{ this->_x = r_; this->_z = bag_.b(); this->_w = bag_.a(); this->_y = bag_.g(); }

/**  Set multiple components of vector as a new vector.  */
inline void rbag(const vector< component_type, 3 > & rba_, const component_type & g_)
{ this->_x = rba_.r(); this->_z = rba_.b(); this->_w = rba_.a(); this->_y = g_; }

/**  Set multiple components of vector as a new vector.  */
inline void rbag(const component_type & r_, const component_type & b_, const vector< component_type, 2 > & ag_)
{ this->_x = r_; this->_z = b_; this->_w = ag_.a(); this->_y = ag_.g(); }

/**  Set multiple components of vector as a new vector.  */
inline void rbag(const component_type & r_, const vector< component_type, 2 > & ba_, const component_type & g_)
{ this->_x = r_; this->_z = ba_.b(); this->_w = ba_.a(); this->_y = g_; }

/**  Set multiple components of vector as a new vector.  */
inline void rbag(const vector< component_type, 2 > & rb_, const component_type & a_, const component_type & g_)
{ this->_x = rb_.r(); this->_z = rb_.b(); this->_w = a_; this->_y = g_; }

/**  Set multiple components of vector as a new vector.  */
inline void rbag(const vector< component_type, 2 > & rb_, const vector< component_type, 2 > & ag_)
{ this->_x = rb_.r(); this->_z = rb_.b(); this->_w = ag_.a(); this->_y = ag_.g(); }

/**  Set multiple components of vector as a new vector.  */
inline void rbag(const vector< component_type, 4 > & rbag_)
{ this->_x = rbag_.r(); this->_z = rbag_.b(); this->_w = rbag_.a(); this->_y = rbag_.g(); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > rbab() const
{ return vector< component_type, 4 >(this->_x, this->_z, this->_w, this->_z); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > rbaa() const
{ return vector< component_type, 4 >(this->_x, this->_z, this->_w, this->_w); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 2 > ra() const
{ return vector< component_type, 2 >(this->_x, this->_w); }

/**  Set multiple components of vector.  */
inline void ra(const component_type & r_, const component_type & a_)
{ this->_x = r_; this->_w = a_; }

/**  Set multiple components of vector.  */
inline void ra(const vector< component_type, 2 > & ra_)
{ this->_x = ra_.r(); this->_w = ra_.a(); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 3 > rar() const
{ return vector< component_type, 3 >(this->_x, this->_w, this->_x); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > rarr() const
{ return vector< component_type, 4 >(this->_x, this->_w, this->_x, this->_x); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > rarg() const
{ return vector< component_type, 4 >(this->_x, this->_w, this->_x, this->_y); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > rarb() const
{ return vector< component_type, 4 >(this->_x, this->_w, this->_x, this->_z); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > rara() const
{ return vector< component_type, 4 >(this->_x, this->_w, this->_x, this->_w); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 3 > rag() const
{ return vector< component_type, 3 >(this->_x, this->_w, this->_y); }

/**  Set multiple components of vector as a new vector.  */
inline void rag(const component_type & r_, const component_type & a_, const component_type & g_)
{ this->_x = r_; this->_w = a_; this->_y = g_; }

/**  Set multiple components of vector as a new vector.  */
inline void rag(const component_type & r_, const vector< component_type, 2 > & ag_)
{ this->_x = r_; this->_w = ag_.a(); this->_y = ag_.g(); }

/**  Set multiple components of vector as a new vector.  */
inline void rag(const vector< component_type, 2 > & ra_, const component_type & g_)
{ this->_x = ra_.r(); this->_w = ra_.a(); this->_y = g_; }

/**  Set multiple components of vector as a new vector.  */
inline void rag(const vector< component_type, 3 > & rag_)
{ this->_x = rag_.r(); this->_w = rag_.a(); this->_y = rag_.g(); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > ragr() const
{ return vector< component_type, 4 >(this->_x, this->_w, this->_y, this->_x); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > ragg() const
{ return vector< component_type, 4 >(this->_x, this->_w, this->_y, this->_y); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > ragb() const
{ return vector< component_type, 4 >(this->_x, this->_w, this->_y, this->_z); }

/**  Set multiple components of vector as a new vector.  */
inline void ragb(const component_type & r_, const component_type & a_, const component_type & g_, const component_type & b_)
{ this->_x = r_; this->_w = a_; this->_y = g_; this->_z = b_; }

/**  Set multiple components of vector as a new vector.  */
inline void ragb(const component_type & r_, const vector< component_type, 3 > & agb_)
{ this->_x = r_; this->_w = agb_.a(); this->_y = agb_.g(); this->_z = agb_.b(); }

/**  Set multiple components of vector as a new vector.  */
inline void ragb(const vector< component_type, 3 > & rag_, const component_type & b_)
{ this->_x = rag_.r(); this->_w = rag_.a(); this->_y = rag_.g(); this->_z = b_; }

/**  Set multiple components of vector as a new vector.  */
inline void ragb(const component_type & r_, const component_type & a_, const vector< component_type, 2 > & gb_)
{ this->_x = r_; this->_w = a_; this->_y = gb_.g(); this->_z = gb_.b(); }

/**  Set multiple components of vector as a new vector.  */
inline void ragb(const component_type & r_, const vector< component_type, 2 > & ag_, const component_type & b_)
{ this->_x = r_; this->_w = ag_.a(); this->_y = ag_.g(); this->_z = b_; }

/**  Set multiple components of vector as a new vector.  */
inline void ragb(const vector< component_type, 2 > & ra_, const component_type & g_, const component_type & b_)
{ this->_x = ra_.r(); this->_w = ra_.a(); this->_y = g_; this->_z = b_; }

/**  Set multiple components of vector as a new vector.  */
inline void ragb(const vector< component_type, 2 > & ra_, const vector< component_type, 2 > & gb_)
{ this->_x = ra_.r(); this->_w = ra_.a(); this->_y = gb_.g(); this->_z = gb_.b(); }

/**  Set multiple components of vector as a new vector.  */
inline void ragb(const vector< component_type, 4 > & ragb_)
{ this->_x = ragb_.r(); this->_w = ragb_.a(); this->_y = ragb_.g(); this->_z = ragb_.b(); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > raga() const
{ return vector< component_type, 4 >(this->_x, this->_w, this->_y, this->_w); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 3 > rab() const
{ return vector< component_type, 3 >(this->_x, this->_w, this->_z); }

/**  Set multiple components of vector as a new vector.  */
inline void rab(const component_type & r_, const component_type & a_, const component_type & b_)
{ this->_x = r_; this->_w = a_; this->_z = b_; }

/**  Set multiple components of vector as a new vector.  */
inline void rab(const component_type & r_, const vector< component_type, 2 > & ab_)
{ this->_x = r_; this->_w = ab_.a(); this->_z = ab_.b(); }

/**  Set multiple components of vector as a new vector.  */
inline void rab(const vector< component_type, 2 > & ra_, const component_type & b_)
{ this->_x = ra_.r(); this->_w = ra_.a(); this->_z = b_; }

/**  Set multiple components of vector as a new vector.  */
inline void rab(const vector< component_type, 3 > & rab_)
{ this->_x = rab_.r(); this->_w = rab_.a(); this->_z = rab_.b(); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > rabr() const
{ return vector< component_type, 4 >(this->_x, this->_w, this->_z, this->_x); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > rabg() const
{ return vector< component_type, 4 >(this->_x, this->_w, this->_z, this->_y); }

/**  Set multiple components of vector as a new vector.  */
inline void rabg(const component_type & r_, const component_type & a_, const component_type & b_, const component_type & g_)
{ this->_x = r_; this->_w = a_; this->_z = b_; this->_y = g_; }

/**  Set multiple components of vector as a new vector.  */
inline void rabg(const component_type & r_, const vector< component_type, 3 > & abg_)
{ this->_x = r_; this->_w = abg_.a(); this->_z = abg_.b(); this->_y = abg_.g(); }

/**  Set multiple components of vector as a new vector.  */
inline void rabg(const vector< component_type, 3 > & rab_, const component_type & g_)
{ this->_x = rab_.r(); this->_w = rab_.a(); this->_z = rab_.b(); this->_y = g_; }

/**  Set multiple components of vector as a new vector.  */
inline void rabg(const component_type & r_, const component_type & a_, const vector< component_type, 2 > & bg_)
{ this->_x = r_; this->_w = a_; this->_z = bg_.b(); this->_y = bg_.g(); }

/**  Set multiple components of vector as a new vector.  */
inline void rabg(const component_type & r_, const vector< component_type, 2 > & ab_, const component_type & g_)
{ this->_x = r_; this->_w = ab_.a(); this->_z = ab_.b(); this->_y = g_; }

/**  Set multiple components of vector as a new vector.  */
inline void rabg(const vector< component_type, 2 > & ra_, const component_type & b_, const component_type & g_)
{ this->_x = ra_.r(); this->_w = ra_.a(); this->_z = b_; this->_y = g_; }

/**  Set multiple components of vector as a new vector.  */
inline void rabg(const vector< component_type, 2 > & ra_, const vector< component_type, 2 > & bg_)
{ this->_x = ra_.r(); this->_w = ra_.a(); this->_z = bg_.b(); this->_y = bg_.g(); }

/**  Set multiple components of vector as a new vector.  */
inline void rabg(const vector< component_type, 4 > & rabg_)
{ this->_x = rabg_.r(); this->_w = rabg_.a(); this->_z = rabg_.b(); this->_y = rabg_.g(); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > rabb() const
{ return vector< component_type, 4 >(this->_x, this->_w, this->_z, this->_z); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > raba() const
{ return vector< component_type, 4 >(this->_x, this->_w, this->_z, this->_w); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 3 > raa() const
{ return vector< component_type, 3 >(this->_x, this->_w, this->_w); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > raar() const
{ return vector< component_type, 4 >(this->_x, this->_w, this->_w, this->_x); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > raag() const
{ return vector< component_type, 4 >(this->_x, this->_w, this->_w, this->_y); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > raab() const
{ return vector< component_type, 4 >(this->_x, this->_w, this->_w, this->_z); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > raaa() const
{ return vector< component_type, 4 >(this->_x, this->_w, this->_w, this->_w); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > grra() const
{ return vector< component_type, 4 >(this->_y, this->_x, this->_x, this->_w); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > grga() const
{ return vector< component_type, 4 >(this->_y, this->_x, this->_y, this->_w); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > grba() const
{ return vector< component_type, 4 >(this->_y, this->_x, this->_z, this->_w); }

/**  Set multiple components of vector as a new vector.  */
inline void grba(const component_type & g_, const component_type & r_, const component_type & b_, const component_type & a_)
{ this->_y = g_; this->_x = r_; this->_z = b_; this->_w = a_; }

/**  Set multiple components of vector as a new vector.  */
inline void grba(const component_type & g_, const vector< component_type, 3 > & rba_)
{ this->_y = g_; this->_x = rba_.r(); this->_z = rba_.b(); this->_w = rba_.a(); }

/**  Set multiple components of vector as a new vector.  */
inline void grba(const vector< component_type, 3 > & grb_, const component_type & a_)
{ this->_y = grb_.g(); this->_x = grb_.r(); this->_z = grb_.b(); this->_w = a_; }

/**  Set multiple components of vector as a new vector.  */
inline void grba(const component_type & g_, const component_type & r_, const vector< component_type, 2 > & ba_)
{ this->_y = g_; this->_x = r_; this->_z = ba_.b(); this->_w = ba_.a(); }

/**  Set multiple components of vector as a new vector.  */
inline void grba(const component_type & g_, const vector< component_type, 2 > & rb_, const component_type & a_)
{ this->_y = g_; this->_x = rb_.r(); this->_z = rb_.b(); this->_w = a_; }

/**  Set multiple components of vector as a new vector.  */
inline void grba(const vector< component_type, 2 > & gr_, const component_type & b_, const component_type & a_)
{ this->_y = gr_.g(); this->_x = gr_.r(); this->_z = b_; this->_w = a_; }

/**  Set multiple components of vector as a new vector.  */
inline void grba(const vector< component_type, 2 > & gr_, const vector< component_type, 2 > & ba_)
{ this->_y = gr_.g(); this->_x = gr_.r(); this->_z = ba_.b(); this->_w = ba_.a(); }

/**  Set multiple components of vector as a new vector.  */
inline void grba(const vector< component_type, 4 > & grba_)
{ this->_y = grba_.g(); this->_x = grba_.r(); this->_z = grba_.b(); this->_w = grba_.a(); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 3 > gra() const
{ return vector< component_type, 3 >(this->_y, this->_x, this->_w); }

/**  Set multiple components of vector as a new vector.  */
inline void gra(const component_type & g_, const component_type & r_, const component_type & a_)
{ this->_y = g_; this->_x = r_; this->_w = a_; }

/**  Set multiple components of vector as a new vector.  */
inline void gra(const component_type & g_, const vector< component_type, 2 > & ra_)
{ this->_y = g_; this->_x = ra_.r(); this->_w = ra_.a(); }

/**  Set multiple components of vector as a new vector.  */
inline void gra(const vector< component_type, 2 > & gr_, const component_type & a_)
{ this->_y = gr_.g(); this->_x = gr_.r(); this->_w = a_; }

/**  Set multiple components of vector as a new vector.  */
inline void gra(const vector< component_type, 3 > & gra_)
{ this->_y = gra_.g(); this->_x = gra_.r(); this->_w = gra_.a(); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > grar() const
{ return vector< component_type, 4 >(this->_y, this->_x, this->_w, this->_x); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > grag() const
{ return vector< component_type, 4 >(this->_y, this->_x, this->_w, this->_y); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > grab() const
{ return vector< component_type, 4 >(this->_y, this->_x, this->_w, this->_z); }

/**  Set multiple components of vector as a new vector.  */
inline void grab(const component_type & g_, const component_type & r_, const component_type & a_, const component_type & b_)
{ this->_y = g_; this->_x = r_; this->_w = a_; this->_z = b_; }

/**  Set multiple components of vector as a new vector.  */
inline void grab(const component_type & g_, const vector< component_type, 3 > & rab_)
{ this->_y = g_; this->_x = rab_.r(); this->_w = rab_.a(); this->_z = rab_.b(); }

/**  Set multiple components of vector as a new vector.  */
inline void grab(const vector< component_type, 3 > & gra_, const component_type & b_)
{ this->_y = gra_.g(); this->_x = gra_.r(); this->_w = gra_.a(); this->_z = b_; }

/**  Set multiple components of vector as a new vector.  */
inline void grab(const component_type & g_, const component_type & r_, const vector< component_type, 2 > & ab_)
{ this->_y = g_; this->_x = r_; this->_w = ab_.a(); this->_z = ab_.b(); }

/**  Set multiple components of vector as a new vector.  */
inline void grab(const component_type & g_, const vector< component_type, 2 > & ra_, const component_type & b_)
{ this->_y = g_; this->_x = ra_.r(); this->_w = ra_.a(); this->_z = b_; }

/**  Set multiple components of vector as a new vector.  */
inline void grab(const vector< component_type, 2 > & gr_, const component_type & a_, const component_type & b_)
{ this->_y = gr_.g(); this->_x = gr_.r(); this->_w = a_; this->_z = b_; }

/**  Set multiple components of vector as a new vector.  */
inline void grab(const vector< component_type, 2 > & gr_, const vector< component_type, 2 > & ab_)
{ this->_y = gr_.g(); this->_x = gr_.r(); this->_w = ab_.a(); this->_z = ab_.b(); }

/**  Set multiple components of vector as a new vector.  */
inline void grab(const vector< component_type, 4 > & grab_)
{ this->_y = grab_.g(); this->_x = grab_.r(); this->_w = grab_.a(); this->_z = grab_.b(); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > graa() const
{ return vector< component_type, 4 >(this->_y, this->_x, this->_w, this->_w); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > ggra() const
{ return vector< component_type, 4 >(this->_y, this->_y, this->_x, this->_w); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > ggga() const
{ return vector< component_type, 4 >(this->_y, this->_y, this->_y, this->_w); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > ggba() const
{ return vector< component_type, 4 >(this->_y, this->_y, this->_z, this->_w); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 3 > gga() const
{ return vector< component_type, 3 >(this->_y, this->_y, this->_w); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > ggar() const
{ return vector< component_type, 4 >(this->_y, this->_y, this->_w, this->_x); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > ggag() const
{ return vector< component_type, 4 >(this->_y, this->_y, this->_w, this->_y); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > ggab() const
{ return vector< component_type, 4 >(this->_y, this->_y, this->_w, this->_z); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > ggaa() const
{ return vector< component_type, 4 >(this->_y, this->_y, this->_w, this->_w); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > gbra() const
{ return vector< component_type, 4 >(this->_y, this->_z, this->_x, this->_w); }

/**  Set multiple components of vector as a new vector.  */
inline void gbra(const component_type & g_, const component_type & b_, const component_type & r_, const component_type & a_)
{ this->_y = g_; this->_z = b_; this->_x = r_; this->_w = a_; }

/**  Set multiple components of vector as a new vector.  */
inline void gbra(const component_type & g_, const vector< component_type, 3 > & bra_)
{ this->_y = g_; this->_z = bra_.b(); this->_x = bra_.r(); this->_w = bra_.a(); }

/**  Set multiple components of vector as a new vector.  */
inline void gbra(const vector< component_type, 3 > & gbr_, const component_type & a_)
{ this->_y = gbr_.g(); this->_z = gbr_.b(); this->_x = gbr_.r(); this->_w = a_; }

/**  Set multiple components of vector as a new vector.  */
inline void gbra(const component_type & g_, const component_type & b_, const vector< component_type, 2 > & ra_)
{ this->_y = g_; this->_z = b_; this->_x = ra_.r(); this->_w = ra_.a(); }

/**  Set multiple components of vector as a new vector.  */
inline void gbra(const component_type & g_, const vector< component_type, 2 > & br_, const component_type & a_)
{ this->_y = g_; this->_z = br_.b(); this->_x = br_.r(); this->_w = a_; }

/**  Set multiple components of vector as a new vector.  */
inline void gbra(const vector< component_type, 2 > & gb_, const component_type & r_, const component_type & a_)
{ this->_y = gb_.g(); this->_z = gb_.b(); this->_x = r_; this->_w = a_; }

/**  Set multiple components of vector as a new vector.  */
inline void gbra(const vector< component_type, 2 > & gb_, const vector< component_type, 2 > & ra_)
{ this->_y = gb_.g(); this->_z = gb_.b(); this->_x = ra_.r(); this->_w = ra_.a(); }

/**  Set multiple components of vector as a new vector.  */
inline void gbra(const vector< component_type, 4 > & gbra_)
{ this->_y = gbra_.g(); this->_z = gbra_.b(); this->_x = gbra_.r(); this->_w = gbra_.a(); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > gbga() const
{ return vector< component_type, 4 >(this->_y, this->_z, this->_y, this->_w); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > gbba() const
{ return vector< component_type, 4 >(this->_y, this->_z, this->_z, this->_w); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 3 > gba() const
{ return vector< component_type, 3 >(this->_y, this->_z, this->_w); }

/**  Set multiple components of vector as a new vector.  */
inline void gba(const component_type & g_, const component_type & b_, const component_type & a_)
{ this->_y = g_; this->_z = b_; this->_w = a_; }

/**  Set multiple components of vector as a new vector.  */
inline void gba(const component_type & g_, const vector< component_type, 2 > & ba_)
{ this->_y = g_; this->_z = ba_.b(); this->_w = ba_.a(); }

/**  Set multiple components of vector as a new vector.  */
inline void gba(const vector< component_type, 2 > & gb_, const component_type & a_)
{ this->_y = gb_.g(); this->_z = gb_.b(); this->_w = a_; }

/**  Set multiple components of vector as a new vector.  */
inline void gba(const vector< component_type, 3 > & gba_)
{ this->_y = gba_.g(); this->_z = gba_.b(); this->_w = gba_.a(); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > gbar() const
{ return vector< component_type, 4 >(this->_y, this->_z, this->_w, this->_x); }

/**  Set multiple components of vector as a new vector.  */
inline void gbar(const component_type & g_, const component_type & b_, const component_type & a_, const component_type & r_)
{ this->_y = g_; this->_z = b_; this->_w = a_; this->_x = r_; }

/**  Set multiple components of vector as a new vector.  */
inline void gbar(const component_type & g_, const vector< component_type, 3 > & bar_)
{ this->_y = g_; this->_z = bar_.b(); this->_w = bar_.a(); this->_x = bar_.r(); }

/**  Set multiple components of vector as a new vector.  */
inline void gbar(const vector< component_type, 3 > & gba_, const component_type & r_)
{ this->_y = gba_.g(); this->_z = gba_.b(); this->_w = gba_.a(); this->_x = r_; }

/**  Set multiple components of vector as a new vector.  */
inline void gbar(const component_type & g_, const component_type & b_, const vector< component_type, 2 > & ar_)
{ this->_y = g_; this->_z = b_; this->_w = ar_.a(); this->_x = ar_.r(); }

/**  Set multiple components of vector as a new vector.  */
inline void gbar(const component_type & g_, const vector< component_type, 2 > & ba_, const component_type & r_)
{ this->_y = g_; this->_z = ba_.b(); this->_w = ba_.a(); this->_x = r_; }

/**  Set multiple components of vector as a new vector.  */
inline void gbar(const vector< component_type, 2 > & gb_, const component_type & a_, const component_type & r_)
{ this->_y = gb_.g(); this->_z = gb_.b(); this->_w = a_; this->_x = r_; }

/**  Set multiple components of vector as a new vector.  */
inline void gbar(const vector< component_type, 2 > & gb_, const vector< component_type, 2 > & ar_)
{ this->_y = gb_.g(); this->_z = gb_.b(); this->_w = ar_.a(); this->_x = ar_.r(); }

/**  Set multiple components of vector as a new vector.  */
inline void gbar(const vector< component_type, 4 > & gbar_)
{ this->_y = gbar_.g(); this->_z = gbar_.b(); this->_w = gbar_.a(); this->_x = gbar_.r(); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > gbag() const
{ return vector< component_type, 4 >(this->_y, this->_z, this->_w, this->_y); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > gbab() const
{ return vector< component_type, 4 >(this->_y, this->_z, this->_w, this->_z); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > gbaa() const
{ return vector< component_type, 4 >(this->_y, this->_z, this->_w, this->_w); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 2 > ga() const
{ return vector< component_type, 2 >(this->_y, this->_w); }

/**  Set multiple components of vector.  */
inline void ga(const component_type & g_, const component_type & a_)
{ this->_y = g_; this->_w = a_; }

/**  Set multiple components of vector.  */
inline void ga(const vector< component_type, 2 > & ga_)
{ this->_y = ga_.g(); this->_w = ga_.a(); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 3 > gar() const
{ return vector< component_type, 3 >(this->_y, this->_w, this->_x); }

/**  Set multiple components of vector as a new vector.  */
inline void gar(const component_type & g_, const component_type & a_, const component_type & r_)
{ this->_y = g_; this->_w = a_; this->_x = r_; }

/**  Set multiple components of vector as a new vector.  */
inline void gar(const component_type & g_, const vector< component_type, 2 > & ar_)
{ this->_y = g_; this->_w = ar_.a(); this->_x = ar_.r(); }

/**  Set multiple components of vector as a new vector.  */
inline void gar(const vector< component_type, 2 > & ga_, const component_type & r_)
{ this->_y = ga_.g(); this->_w = ga_.a(); this->_x = r_; }

/**  Set multiple components of vector as a new vector.  */
inline void gar(const vector< component_type, 3 > & gar_)
{ this->_y = gar_.g(); this->_w = gar_.a(); this->_x = gar_.r(); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > garr() const
{ return vector< component_type, 4 >(this->_y, this->_w, this->_x, this->_x); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > garg() const
{ return vector< component_type, 4 >(this->_y, this->_w, this->_x, this->_y); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > garb() const
{ return vector< component_type, 4 >(this->_y, this->_w, this->_x, this->_z); }

/**  Set multiple components of vector as a new vector.  */
inline void garb(const component_type & g_, const component_type & a_, const component_type & r_, const component_type & b_)
{ this->_y = g_; this->_w = a_; this->_x = r_; this->_z = b_; }

/**  Set multiple components of vector as a new vector.  */
inline void garb(const component_type & g_, const vector< component_type, 3 > & arb_)
{ this->_y = g_; this->_w = arb_.a(); this->_x = arb_.r(); this->_z = arb_.b(); }

/**  Set multiple components of vector as a new vector.  */
inline void garb(const vector< component_type, 3 > & gar_, const component_type & b_)
{ this->_y = gar_.g(); this->_w = gar_.a(); this->_x = gar_.r(); this->_z = b_; }

/**  Set multiple components of vector as a new vector.  */
inline void garb(const component_type & g_, const component_type & a_, const vector< component_type, 2 > & rb_)
{ this->_y = g_; this->_w = a_; this->_x = rb_.r(); this->_z = rb_.b(); }

/**  Set multiple components of vector as a new vector.  */
inline void garb(const component_type & g_, const vector< component_type, 2 > & ar_, const component_type & b_)
{ this->_y = g_; this->_w = ar_.a(); this->_x = ar_.r(); this->_z = b_; }

/**  Set multiple components of vector as a new vector.  */
inline void garb(const vector< component_type, 2 > & ga_, const component_type & r_, const component_type & b_)
{ this->_y = ga_.g(); this->_w = ga_.a(); this->_x = r_; this->_z = b_; }

/**  Set multiple components of vector as a new vector.  */
inline void garb(const vector< component_type, 2 > & ga_, const vector< component_type, 2 > & rb_)
{ this->_y = ga_.g(); this->_w = ga_.a(); this->_x = rb_.r(); this->_z = rb_.b(); }

/**  Set multiple components of vector as a new vector.  */
inline void garb(const vector< component_type, 4 > & garb_)
{ this->_y = garb_.g(); this->_w = garb_.a(); this->_x = garb_.r(); this->_z = garb_.b(); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > gara() const
{ return vector< component_type, 4 >(this->_y, this->_w, this->_x, this->_w); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 3 > gag() const
{ return vector< component_type, 3 >(this->_y, this->_w, this->_y); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > gagr() const
{ return vector< component_type, 4 >(this->_y, this->_w, this->_y, this->_x); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > gagg() const
{ return vector< component_type, 4 >(this->_y, this->_w, this->_y, this->_y); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > gagb() const
{ return vector< component_type, 4 >(this->_y, this->_w, this->_y, this->_z); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > gaga() const
{ return vector< component_type, 4 >(this->_y, this->_w, this->_y, this->_w); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 3 > gab() const
{ return vector< component_type, 3 >(this->_y, this->_w, this->_z); }

/**  Set multiple components of vector as a new vector.  */
inline void gab(const component_type & g_, const component_type & a_, const component_type & b_)
{ this->_y = g_; this->_w = a_; this->_z = b_; }

/**  Set multiple components of vector as a new vector.  */
inline void gab(const component_type & g_, const vector< component_type, 2 > & ab_)
{ this->_y = g_; this->_w = ab_.a(); this->_z = ab_.b(); }

/**  Set multiple components of vector as a new vector.  */
inline void gab(const vector< component_type, 2 > & ga_, const component_type & b_)
{ this->_y = ga_.g(); this->_w = ga_.a(); this->_z = b_; }

/**  Set multiple components of vector as a new vector.  */
inline void gab(const vector< component_type, 3 > & gab_)
{ this->_y = gab_.g(); this->_w = gab_.a(); this->_z = gab_.b(); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > gabr() const
{ return vector< component_type, 4 >(this->_y, this->_w, this->_z, this->_x); }

/**  Set multiple components of vector as a new vector.  */
inline void gabr(const component_type & g_, const component_type & a_, const component_type & b_, const component_type & r_)
{ this->_y = g_; this->_w = a_; this->_z = b_; this->_x = r_; }

/**  Set multiple components of vector as a new vector.  */
inline void gabr(const component_type & g_, const vector< component_type, 3 > & abr_)
{ this->_y = g_; this->_w = abr_.a(); this->_z = abr_.b(); this->_x = abr_.r(); }

/**  Set multiple components of vector as a new vector.  */
inline void gabr(const vector< component_type, 3 > & gab_, const component_type & r_)
{ this->_y = gab_.g(); this->_w = gab_.a(); this->_z = gab_.b(); this->_x = r_; }

/**  Set multiple components of vector as a new vector.  */
inline void gabr(const component_type & g_, const component_type & a_, const vector< component_type, 2 > & br_)
{ this->_y = g_; this->_w = a_; this->_z = br_.b(); this->_x = br_.r(); }

/**  Set multiple components of vector as a new vector.  */
inline void gabr(const component_type & g_, const vector< component_type, 2 > & ab_, const component_type & r_)
{ this->_y = g_; this->_w = ab_.a(); this->_z = ab_.b(); this->_x = r_; }

/**  Set multiple components of vector as a new vector.  */
inline void gabr(const vector< component_type, 2 > & ga_, const component_type & b_, const component_type & r_)
{ this->_y = ga_.g(); this->_w = ga_.a(); this->_z = b_; this->_x = r_; }

/**  Set multiple components of vector as a new vector.  */
inline void gabr(const vector< component_type, 2 > & ga_, const vector< component_type, 2 > & br_)
{ this->_y = ga_.g(); this->_w = ga_.a(); this->_z = br_.b(); this->_x = br_.r(); }

/**  Set multiple components of vector as a new vector.  */
inline void gabr(const vector< component_type, 4 > & gabr_)
{ this->_y = gabr_.g(); this->_w = gabr_.a(); this->_z = gabr_.b(); this->_x = gabr_.r(); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > gabg() const
{ return vector< component_type, 4 >(this->_y, this->_w, this->_z, this->_y); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > gabb() const
{ return vector< component_type, 4 >(this->_y, this->_w, this->_z, this->_z); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > gaba() const
{ return vector< component_type, 4 >(this->_y, this->_w, this->_z, this->_w); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 3 > gaa() const
{ return vector< component_type, 3 >(this->_y, this->_w, this->_w); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > gaar() const
{ return vector< component_type, 4 >(this->_y, this->_w, this->_w, this->_x); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > gaag() const
{ return vector< component_type, 4 >(this->_y, this->_w, this->_w, this->_y); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > gaab() const
{ return vector< component_type, 4 >(this->_y, this->_w, this->_w, this->_z); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > gaaa() const
{ return vector< component_type, 4 >(this->_y, this->_w, this->_w, this->_w); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > brra() const
{ return vector< component_type, 4 >(this->_z, this->_x, this->_x, this->_w); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > brga() const
{ return vector< component_type, 4 >(this->_z, this->_x, this->_y, this->_w); }

/**  Set multiple components of vector as a new vector.  */
inline void brga(const component_type & b_, const component_type & r_, const component_type & g_, const component_type & a_)
{ this->_z = b_; this->_x = r_; this->_y = g_; this->_w = a_; }

/**  Set multiple components of vector as a new vector.  */
inline void brga(const component_type & b_, const vector< component_type, 3 > & rga_)
{ this->_z = b_; this->_x = rga_.r(); this->_y = rga_.g(); this->_w = rga_.a(); }

/**  Set multiple components of vector as a new vector.  */
inline void brga(const vector< component_type, 3 > & brg_, const component_type & a_)
{ this->_z = brg_.b(); this->_x = brg_.r(); this->_y = brg_.g(); this->_w = a_; }

/**  Set multiple components of vector as a new vector.  */
inline void brga(const component_type & b_, const component_type & r_, const vector< component_type, 2 > & ga_)
{ this->_z = b_; this->_x = r_; this->_y = ga_.g(); this->_w = ga_.a(); }

/**  Set multiple components of vector as a new vector.  */
inline void brga(const component_type & b_, const vector< component_type, 2 > & rg_, const component_type & a_)
{ this->_z = b_; this->_x = rg_.r(); this->_y = rg_.g(); this->_w = a_; }

/**  Set multiple components of vector as a new vector.  */
inline void brga(const vector< component_type, 2 > & br_, const component_type & g_, const component_type & a_)
{ this->_z = br_.b(); this->_x = br_.r(); this->_y = g_; this->_w = a_; }

/**  Set multiple components of vector as a new vector.  */
inline void brga(const vector< component_type, 2 > & br_, const vector< component_type, 2 > & ga_)
{ this->_z = br_.b(); this->_x = br_.r(); this->_y = ga_.g(); this->_w = ga_.a(); }

/**  Set multiple components of vector as a new vector.  */
inline void brga(const vector< component_type, 4 > & brga_)
{ this->_z = brga_.b(); this->_x = brga_.r(); this->_y = brga_.g(); this->_w = brga_.a(); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > brba() const
{ return vector< component_type, 4 >(this->_z, this->_x, this->_z, this->_w); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 3 > bra() const
{ return vector< component_type, 3 >(this->_z, this->_x, this->_w); }

/**  Set multiple components of vector as a new vector.  */
inline void bra(const component_type & b_, const component_type & r_, const component_type & a_)
{ this->_z = b_; this->_x = r_; this->_w = a_; }

/**  Set multiple components of vector as a new vector.  */
inline void bra(const component_type & b_, const vector< component_type, 2 > & ra_)
{ this->_z = b_; this->_x = ra_.r(); this->_w = ra_.a(); }

/**  Set multiple components of vector as a new vector.  */
inline void bra(const vector< component_type, 2 > & br_, const component_type & a_)
{ this->_z = br_.b(); this->_x = br_.r(); this->_w = a_; }

/**  Set multiple components of vector as a new vector.  */
inline void bra(const vector< component_type, 3 > & bra_)
{ this->_z = bra_.b(); this->_x = bra_.r(); this->_w = bra_.a(); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > brar() const
{ return vector< component_type, 4 >(this->_z, this->_x, this->_w, this->_x); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > brag() const
{ return vector< component_type, 4 >(this->_z, this->_x, this->_w, this->_y); }

/**  Set multiple components of vector as a new vector.  */
inline void brag(const component_type & b_, const component_type & r_, const component_type & a_, const component_type & g_)
{ this->_z = b_; this->_x = r_; this->_w = a_; this->_y = g_; }

/**  Set multiple components of vector as a new vector.  */
inline void brag(const component_type & b_, const vector< component_type, 3 > & rag_)
{ this->_z = b_; this->_x = rag_.r(); this->_w = rag_.a(); this->_y = rag_.g(); }

/**  Set multiple components of vector as a new vector.  */
inline void brag(const vector< component_type, 3 > & bra_, const component_type & g_)
{ this->_z = bra_.b(); this->_x = bra_.r(); this->_w = bra_.a(); this->_y = g_; }

/**  Set multiple components of vector as a new vector.  */
inline void brag(const component_type & b_, const component_type & r_, const vector< component_type, 2 > & ag_)
{ this->_z = b_; this->_x = r_; this->_w = ag_.a(); this->_y = ag_.g(); }

/**  Set multiple components of vector as a new vector.  */
inline void brag(const component_type & b_, const vector< component_type, 2 > & ra_, const component_type & g_)
{ this->_z = b_; this->_x = ra_.r(); this->_w = ra_.a(); this->_y = g_; }

/**  Set multiple components of vector as a new vector.  */
inline void brag(const vector< component_type, 2 > & br_, const component_type & a_, const component_type & g_)
{ this->_z = br_.b(); this->_x = br_.r(); this->_w = a_; this->_y = g_; }

/**  Set multiple components of vector as a new vector.  */
inline void brag(const vector< component_type, 2 > & br_, const vector< component_type, 2 > & ag_)
{ this->_z = br_.b(); this->_x = br_.r(); this->_w = ag_.a(); this->_y = ag_.g(); }

/**  Set multiple components of vector as a new vector.  */
inline void brag(const vector< component_type, 4 > & brag_)
{ this->_z = brag_.b(); this->_x = brag_.r(); this->_w = brag_.a(); this->_y = brag_.g(); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > brab() const
{ return vector< component_type, 4 >(this->_z, this->_x, this->_w, this->_z); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > braa() const
{ return vector< component_type, 4 >(this->_z, this->_x, this->_w, this->_w); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > bgra() const
{ return vector< component_type, 4 >(this->_z, this->_y, this->_x, this->_w); }

/**  Set multiple components of vector as a new vector.  */
inline void bgra(const component_type & b_, const component_type & g_, const component_type & r_, const component_type & a_)
{ this->_z = b_; this->_y = g_; this->_x = r_; this->_w = a_; }

/**  Set multiple components of vector as a new vector.  */
inline void bgra(const component_type & b_, const vector< component_type, 3 > & gra_)
{ this->_z = b_; this->_y = gra_.g(); this->_x = gra_.r(); this->_w = gra_.a(); }

/**  Set multiple components of vector as a new vector.  */
inline void bgra(const vector< component_type, 3 > & bgr_, const component_type & a_)
{ this->_z = bgr_.b(); this->_y = bgr_.g(); this->_x = bgr_.r(); this->_w = a_; }

/**  Set multiple components of vector as a new vector.  */
inline void bgra(const component_type & b_, const component_type & g_, const vector< component_type, 2 > & ra_)
{ this->_z = b_; this->_y = g_; this->_x = ra_.r(); this->_w = ra_.a(); }

/**  Set multiple components of vector as a new vector.  */
inline void bgra(const component_type & b_, const vector< component_type, 2 > & gr_, const component_type & a_)
{ this->_z = b_; this->_y = gr_.g(); this->_x = gr_.r(); this->_w = a_; }

/**  Set multiple components of vector as a new vector.  */
inline void bgra(const vector< component_type, 2 > & bg_, const component_type & r_, const component_type & a_)
{ this->_z = bg_.b(); this->_y = bg_.g(); this->_x = r_; this->_w = a_; }

/**  Set multiple components of vector as a new vector.  */
inline void bgra(const vector< component_type, 2 > & bg_, const vector< component_type, 2 > & ra_)
{ this->_z = bg_.b(); this->_y = bg_.g(); this->_x = ra_.r(); this->_w = ra_.a(); }

/**  Set multiple components of vector as a new vector.  */
inline void bgra(const vector< component_type, 4 > & bgra_)
{ this->_z = bgra_.b(); this->_y = bgra_.g(); this->_x = bgra_.r(); this->_w = bgra_.a(); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > bgga() const
{ return vector< component_type, 4 >(this->_z, this->_y, this->_y, this->_w); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > bgba() const
{ return vector< component_type, 4 >(this->_z, this->_y, this->_z, this->_w); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 3 > bga() const
{ return vector< component_type, 3 >(this->_z, this->_y, this->_w); }

/**  Set multiple components of vector as a new vector.  */
inline void bga(const component_type & b_, const component_type & g_, const component_type & a_)
{ this->_z = b_; this->_y = g_; this->_w = a_; }

/**  Set multiple components of vector as a new vector.  */
inline void bga(const component_type & b_, const vector< component_type, 2 > & ga_)
{ this->_z = b_; this->_y = ga_.g(); this->_w = ga_.a(); }

/**  Set multiple components of vector as a new vector.  */
inline void bga(const vector< component_type, 2 > & bg_, const component_type & a_)
{ this->_z = bg_.b(); this->_y = bg_.g(); this->_w = a_; }

/**  Set multiple components of vector as a new vector.  */
inline void bga(const vector< component_type, 3 > & bga_)
{ this->_z = bga_.b(); this->_y = bga_.g(); this->_w = bga_.a(); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > bgar() const
{ return vector< component_type, 4 >(this->_z, this->_y, this->_w, this->_x); }

/**  Set multiple components of vector as a new vector.  */
inline void bgar(const component_type & b_, const component_type & g_, const component_type & a_, const component_type & r_)
{ this->_z = b_; this->_y = g_; this->_w = a_; this->_x = r_; }

/**  Set multiple components of vector as a new vector.  */
inline void bgar(const component_type & b_, const vector< component_type, 3 > & gar_)
{ this->_z = b_; this->_y = gar_.g(); this->_w = gar_.a(); this->_x = gar_.r(); }

/**  Set multiple components of vector as a new vector.  */
inline void bgar(const vector< component_type, 3 > & bga_, const component_type & r_)
{ this->_z = bga_.b(); this->_y = bga_.g(); this->_w = bga_.a(); this->_x = r_; }

/**  Set multiple components of vector as a new vector.  */
inline void bgar(const component_type & b_, const component_type & g_, const vector< component_type, 2 > & ar_)
{ this->_z = b_; this->_y = g_; this->_w = ar_.a(); this->_x = ar_.r(); }

/**  Set multiple components of vector as a new vector.  */
inline void bgar(const component_type & b_, const vector< component_type, 2 > & ga_, const component_type & r_)
{ this->_z = b_; this->_y = ga_.g(); this->_w = ga_.a(); this->_x = r_; }

/**  Set multiple components of vector as a new vector.  */
inline void bgar(const vector< component_type, 2 > & bg_, const component_type & a_, const component_type & r_)
{ this->_z = bg_.b(); this->_y = bg_.g(); this->_w = a_; this->_x = r_; }

/**  Set multiple components of vector as a new vector.  */
inline void bgar(const vector< component_type, 2 > & bg_, const vector< component_type, 2 > & ar_)
{ this->_z = bg_.b(); this->_y = bg_.g(); this->_w = ar_.a(); this->_x = ar_.r(); }

/**  Set multiple components of vector as a new vector.  */
inline void bgar(const vector< component_type, 4 > & bgar_)
{ this->_z = bgar_.b(); this->_y = bgar_.g(); this->_w = bgar_.a(); this->_x = bgar_.r(); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > bgag() const
{ return vector< component_type, 4 >(this->_z, this->_y, this->_w, this->_y); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > bgab() const
{ return vector< component_type, 4 >(this->_z, this->_y, this->_w, this->_z); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > bgaa() const
{ return vector< component_type, 4 >(this->_z, this->_y, this->_w, this->_w); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > bbra() const
{ return vector< component_type, 4 >(this->_z, this->_z, this->_x, this->_w); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > bbga() const
{ return vector< component_type, 4 >(this->_z, this->_z, this->_y, this->_w); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > bbba() const
{ return vector< component_type, 4 >(this->_z, this->_z, this->_z, this->_w); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 3 > bba() const
{ return vector< component_type, 3 >(this->_z, this->_z, this->_w); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > bbar() const
{ return vector< component_type, 4 >(this->_z, this->_z, this->_w, this->_x); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > bbag() const
{ return vector< component_type, 4 >(this->_z, this->_z, this->_w, this->_y); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > bbab() const
{ return vector< component_type, 4 >(this->_z, this->_z, this->_w, this->_z); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > bbaa() const
{ return vector< component_type, 4 >(this->_z, this->_z, this->_w, this->_w); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 2 > ba() const
{ return vector< component_type, 2 >(this->_z, this->_w); }

/**  Set multiple components of vector.  */
inline void ba(const component_type & b_, const component_type & a_)
{ this->_z = b_; this->_w = a_; }

/**  Set multiple components of vector.  */
inline void ba(const vector< component_type, 2 > & ba_)
{ this->_z = ba_.b(); this->_w = ba_.a(); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 3 > bar() const
{ return vector< component_type, 3 >(this->_z, this->_w, this->_x); }

/**  Set multiple components of vector as a new vector.  */
inline void bar(const component_type & b_, const component_type & a_, const component_type & r_)
{ this->_z = b_; this->_w = a_; this->_x = r_; }

/**  Set multiple components of vector as a new vector.  */
inline void bar(const component_type & b_, const vector< component_type, 2 > & ar_)
{ this->_z = b_; this->_w = ar_.a(); this->_x = ar_.r(); }

/**  Set multiple components of vector as a new vector.  */
inline void bar(const vector< component_type, 2 > & ba_, const component_type & r_)
{ this->_z = ba_.b(); this->_w = ba_.a(); this->_x = r_; }

/**  Set multiple components of vector as a new vector.  */
inline void bar(const vector< component_type, 3 > & bar_)
{ this->_z = bar_.b(); this->_w = bar_.a(); this->_x = bar_.r(); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > barr() const
{ return vector< component_type, 4 >(this->_z, this->_w, this->_x, this->_x); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > barg() const
{ return vector< component_type, 4 >(this->_z, this->_w, this->_x, this->_y); }

/**  Set multiple components of vector as a new vector.  */
inline void barg(const component_type & b_, const component_type & a_, const component_type & r_, const component_type & g_)
{ this->_z = b_; this->_w = a_; this->_x = r_; this->_y = g_; }

/**  Set multiple components of vector as a new vector.  */
inline void barg(const component_type & b_, const vector< component_type, 3 > & arg_)
{ this->_z = b_; this->_w = arg_.a(); this->_x = arg_.r(); this->_y = arg_.g(); }

/**  Set multiple components of vector as a new vector.  */
inline void barg(const vector< component_type, 3 > & bar_, const component_type & g_)
{ this->_z = bar_.b(); this->_w = bar_.a(); this->_x = bar_.r(); this->_y = g_; }

/**  Set multiple components of vector as a new vector.  */
inline void barg(const component_type & b_, const component_type & a_, const vector< component_type, 2 > & rg_)
{ this->_z = b_; this->_w = a_; this->_x = rg_.r(); this->_y = rg_.g(); }

/**  Set multiple components of vector as a new vector.  */
inline void barg(const component_type & b_, const vector< component_type, 2 > & ar_, const component_type & g_)
{ this->_z = b_; this->_w = ar_.a(); this->_x = ar_.r(); this->_y = g_; }

/**  Set multiple components of vector as a new vector.  */
inline void barg(const vector< component_type, 2 > & ba_, const component_type & r_, const component_type & g_)
{ this->_z = ba_.b(); this->_w = ba_.a(); this->_x = r_; this->_y = g_; }

/**  Set multiple components of vector as a new vector.  */
inline void barg(const vector< component_type, 2 > & ba_, const vector< component_type, 2 > & rg_)
{ this->_z = ba_.b(); this->_w = ba_.a(); this->_x = rg_.r(); this->_y = rg_.g(); }

/**  Set multiple components of vector as a new vector.  */
inline void barg(const vector< component_type, 4 > & barg_)
{ this->_z = barg_.b(); this->_w = barg_.a(); this->_x = barg_.r(); this->_y = barg_.g(); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > barb() const
{ return vector< component_type, 4 >(this->_z, this->_w, this->_x, this->_z); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > bara() const
{ return vector< component_type, 4 >(this->_z, this->_w, this->_x, this->_w); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 3 > bag() const
{ return vector< component_type, 3 >(this->_z, this->_w, this->_y); }

/**  Set multiple components of vector as a new vector.  */
inline void bag(const component_type & b_, const component_type & a_, const component_type & g_)
{ this->_z = b_; this->_w = a_; this->_y = g_; }

/**  Set multiple components of vector as a new vector.  */
inline void bag(const component_type & b_, const vector< component_type, 2 > & ag_)
{ this->_z = b_; this->_w = ag_.a(); this->_y = ag_.g(); }

/**  Set multiple components of vector as a new vector.  */
inline void bag(const vector< component_type, 2 > & ba_, const component_type & g_)
{ this->_z = ba_.b(); this->_w = ba_.a(); this->_y = g_; }

/**  Set multiple components of vector as a new vector.  */
inline void bag(const vector< component_type, 3 > & bag_)
{ this->_z = bag_.b(); this->_w = bag_.a(); this->_y = bag_.g(); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > bagr() const
{ return vector< component_type, 4 >(this->_z, this->_w, this->_y, this->_x); }

/**  Set multiple components of vector as a new vector.  */
inline void bagr(const component_type & b_, const component_type & a_, const component_type & g_, const component_type & r_)
{ this->_z = b_; this->_w = a_; this->_y = g_; this->_x = r_; }

/**  Set multiple components of vector as a new vector.  */
inline void bagr(const component_type & b_, const vector< component_type, 3 > & agr_)
{ this->_z = b_; this->_w = agr_.a(); this->_y = agr_.g(); this->_x = agr_.r(); }

/**  Set multiple components of vector as a new vector.  */
inline void bagr(const vector< component_type, 3 > & bag_, const component_type & r_)
{ this->_z = bag_.b(); this->_w = bag_.a(); this->_y = bag_.g(); this->_x = r_; }

/**  Set multiple components of vector as a new vector.  */
inline void bagr(const component_type & b_, const component_type & a_, const vector< component_type, 2 > & gr_)
{ this->_z = b_; this->_w = a_; this->_y = gr_.g(); this->_x = gr_.r(); }

/**  Set multiple components of vector as a new vector.  */
inline void bagr(const component_type & b_, const vector< component_type, 2 > & ag_, const component_type & r_)
{ this->_z = b_; this->_w = ag_.a(); this->_y = ag_.g(); this->_x = r_; }

/**  Set multiple components of vector as a new vector.  */
inline void bagr(const vector< component_type, 2 > & ba_, const component_type & g_, const component_type & r_)
{ this->_z = ba_.b(); this->_w = ba_.a(); this->_y = g_; this->_x = r_; }

/**  Set multiple components of vector as a new vector.  */
inline void bagr(const vector< component_type, 2 > & ba_, const vector< component_type, 2 > & gr_)
{ this->_z = ba_.b(); this->_w = ba_.a(); this->_y = gr_.g(); this->_x = gr_.r(); }

/**  Set multiple components of vector as a new vector.  */
inline void bagr(const vector< component_type, 4 > & bagr_)
{ this->_z = bagr_.b(); this->_w = bagr_.a(); this->_y = bagr_.g(); this->_x = bagr_.r(); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > bagg() const
{ return vector< component_type, 4 >(this->_z, this->_w, this->_y, this->_y); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > bagb() const
{ return vector< component_type, 4 >(this->_z, this->_w, this->_y, this->_z); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > baga() const
{ return vector< component_type, 4 >(this->_z, this->_w, this->_y, this->_w); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 3 > bab() const
{ return vector< component_type, 3 >(this->_z, this->_w, this->_z); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > babr() const
{ return vector< component_type, 4 >(this->_z, this->_w, this->_z, this->_x); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > babg() const
{ return vector< component_type, 4 >(this->_z, this->_w, this->_z, this->_y); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > babb() const
{ return vector< component_type, 4 >(this->_z, this->_w, this->_z, this->_z); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > baba() const
{ return vector< component_type, 4 >(this->_z, this->_w, this->_z, this->_w); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 3 > baa() const
{ return vector< component_type, 3 >(this->_z, this->_w, this->_w); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > baar() const
{ return vector< component_type, 4 >(this->_z, this->_w, this->_w, this->_x); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > baag() const
{ return vector< component_type, 4 >(this->_z, this->_w, this->_w, this->_y); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > baab() const
{ return vector< component_type, 4 >(this->_z, this->_w, this->_w, this->_z); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > baaa() const
{ return vector< component_type, 4 >(this->_z, this->_w, this->_w, this->_w); }

/**  Get 4th component of vector.  */
inline component_type a() const
{ return this->_w; }

/**  Set 4th component of vector to a_.  */
inline void a(const component_type & a_)
{ this->_w = a_; }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 2 > ar() const
{ return vector< component_type, 2 >(this->_w, this->_x); }

/**  Set multiple components of vector.  */
inline void ar(const component_type & a_, const component_type & r_)
{ this->_w = a_; this->_x = r_; }

/**  Set multiple components of vector.  */
inline void ar(const vector< component_type, 2 > & ar_)
{ this->_w = ar_.a(); this->_x = ar_.r(); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 3 > arr() const
{ return vector< component_type, 3 >(this->_w, this->_x, this->_x); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > arrr() const
{ return vector< component_type, 4 >(this->_w, this->_x, this->_x, this->_x); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > arrg() const
{ return vector< component_type, 4 >(this->_w, this->_x, this->_x, this->_y); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > arrb() const
{ return vector< component_type, 4 >(this->_w, this->_x, this->_x, this->_z); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > arra() const
{ return vector< component_type, 4 >(this->_w, this->_x, this->_x, this->_w); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 3 > arg() const
{ return vector< component_type, 3 >(this->_w, this->_x, this->_y); }

/**  Set multiple components of vector as a new vector.  */
inline void arg(const component_type & a_, const component_type & r_, const component_type & g_)
{ this->_w = a_; this->_x = r_; this->_y = g_; }

/**  Set multiple components of vector as a new vector.  */
inline void arg(const component_type & a_, const vector< component_type, 2 > & rg_)
{ this->_w = a_; this->_x = rg_.r(); this->_y = rg_.g(); }

/**  Set multiple components of vector as a new vector.  */
inline void arg(const vector< component_type, 2 > & ar_, const component_type & g_)
{ this->_w = ar_.a(); this->_x = ar_.r(); this->_y = g_; }

/**  Set multiple components of vector as a new vector.  */
inline void arg(const vector< component_type, 3 > & arg_)
{ this->_w = arg_.a(); this->_x = arg_.r(); this->_y = arg_.g(); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > argr() const
{ return vector< component_type, 4 >(this->_w, this->_x, this->_y, this->_x); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > argg() const
{ return vector< component_type, 4 >(this->_w, this->_x, this->_y, this->_y); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > argb() const
{ return vector< component_type, 4 >(this->_w, this->_x, this->_y, this->_z); }

/**  Set multiple components of vector as a new vector.  */
inline void argb(const component_type & a_, const component_type & r_, const component_type & g_, const component_type & b_)
{ this->_w = a_; this->_x = r_; this->_y = g_; this->_z = b_; }

/**  Set multiple components of vector as a new vector.  */
inline void argb(const component_type & a_, const vector< component_type, 3 > & rgb_)
{ this->_w = a_; this->_x = rgb_.r(); this->_y = rgb_.g(); this->_z = rgb_.b(); }

/**  Set multiple components of vector as a new vector.  */
inline void argb(const vector< component_type, 3 > & arg_, const component_type & b_)
{ this->_w = arg_.a(); this->_x = arg_.r(); this->_y = arg_.g(); this->_z = b_; }

/**  Set multiple components of vector as a new vector.  */
inline void argb(const component_type & a_, const component_type & r_, const vector< component_type, 2 > & gb_)
{ this->_w = a_; this->_x = r_; this->_y = gb_.g(); this->_z = gb_.b(); }

/**  Set multiple components of vector as a new vector.  */
inline void argb(const component_type & a_, const vector< component_type, 2 > & rg_, const component_type & b_)
{ this->_w = a_; this->_x = rg_.r(); this->_y = rg_.g(); this->_z = b_; }

/**  Set multiple components of vector as a new vector.  */
inline void argb(const vector< component_type, 2 > & ar_, const component_type & g_, const component_type & b_)
{ this->_w = ar_.a(); this->_x = ar_.r(); this->_y = g_; this->_z = b_; }

/**  Set multiple components of vector as a new vector.  */
inline void argb(const vector< component_type, 2 > & ar_, const vector< component_type, 2 > & gb_)
{ this->_w = ar_.a(); this->_x = ar_.r(); this->_y = gb_.g(); this->_z = gb_.b(); }

/**  Set multiple components of vector as a new vector.  */
inline void argb(const vector< component_type, 4 > & argb_)
{ this->_w = argb_.a(); this->_x = argb_.r(); this->_y = argb_.g(); this->_z = argb_.b(); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > arga() const
{ return vector< component_type, 4 >(this->_w, this->_x, this->_y, this->_w); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 3 > arb() const
{ return vector< component_type, 3 >(this->_w, this->_x, this->_z); }

/**  Set multiple components of vector as a new vector.  */
inline void arb(const component_type & a_, const component_type & r_, const component_type & b_)
{ this->_w = a_; this->_x = r_; this->_z = b_; }

/**  Set multiple components of vector as a new vector.  */
inline void arb(const component_type & a_, const vector< component_type, 2 > & rb_)
{ this->_w = a_; this->_x = rb_.r(); this->_z = rb_.b(); }

/**  Set multiple components of vector as a new vector.  */
inline void arb(const vector< component_type, 2 > & ar_, const component_type & b_)
{ this->_w = ar_.a(); this->_x = ar_.r(); this->_z = b_; }

/**  Set multiple components of vector as a new vector.  */
inline void arb(const vector< component_type, 3 > & arb_)
{ this->_w = arb_.a(); this->_x = arb_.r(); this->_z = arb_.b(); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > arbr() const
{ return vector< component_type, 4 >(this->_w, this->_x, this->_z, this->_x); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > arbg() const
{ return vector< component_type, 4 >(this->_w, this->_x, this->_z, this->_y); }

/**  Set multiple components of vector as a new vector.  */
inline void arbg(const component_type & a_, const component_type & r_, const component_type & b_, const component_type & g_)
{ this->_w = a_; this->_x = r_; this->_z = b_; this->_y = g_; }

/**  Set multiple components of vector as a new vector.  */
inline void arbg(const component_type & a_, const vector< component_type, 3 > & rbg_)
{ this->_w = a_; this->_x = rbg_.r(); this->_z = rbg_.b(); this->_y = rbg_.g(); }

/**  Set multiple components of vector as a new vector.  */
inline void arbg(const vector< component_type, 3 > & arb_, const component_type & g_)
{ this->_w = arb_.a(); this->_x = arb_.r(); this->_z = arb_.b(); this->_y = g_; }

/**  Set multiple components of vector as a new vector.  */
inline void arbg(const component_type & a_, const component_type & r_, const vector< component_type, 2 > & bg_)
{ this->_w = a_; this->_x = r_; this->_z = bg_.b(); this->_y = bg_.g(); }

/**  Set multiple components of vector as a new vector.  */
inline void arbg(const component_type & a_, const vector< component_type, 2 > & rb_, const component_type & g_)
{ this->_w = a_; this->_x = rb_.r(); this->_z = rb_.b(); this->_y = g_; }

/**  Set multiple components of vector as a new vector.  */
inline void arbg(const vector< component_type, 2 > & ar_, const component_type & b_, const component_type & g_)
{ this->_w = ar_.a(); this->_x = ar_.r(); this->_z = b_; this->_y = g_; }

/**  Set multiple components of vector as a new vector.  */
inline void arbg(const vector< component_type, 2 > & ar_, const vector< component_type, 2 > & bg_)
{ this->_w = ar_.a(); this->_x = ar_.r(); this->_z = bg_.b(); this->_y = bg_.g(); }

/**  Set multiple components of vector as a new vector.  */
inline void arbg(const vector< component_type, 4 > & arbg_)
{ this->_w = arbg_.a(); this->_x = arbg_.r(); this->_z = arbg_.b(); this->_y = arbg_.g(); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > arbb() const
{ return vector< component_type, 4 >(this->_w, this->_x, this->_z, this->_z); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > arba() const
{ return vector< component_type, 4 >(this->_w, this->_x, this->_z, this->_w); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 3 > ara() const
{ return vector< component_type, 3 >(this->_w, this->_x, this->_w); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > arar() const
{ return vector< component_type, 4 >(this->_w, this->_x, this->_w, this->_x); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > arag() const
{ return vector< component_type, 4 >(this->_w, this->_x, this->_w, this->_y); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > arab() const
{ return vector< component_type, 4 >(this->_w, this->_x, this->_w, this->_z); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > araa() const
{ return vector< component_type, 4 >(this->_w, this->_x, this->_w, this->_w); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 2 > ag() const
{ return vector< component_type, 2 >(this->_w, this->_y); }

/**  Set multiple components of vector.  */
inline void ag(const component_type & a_, const component_type & g_)
{ this->_w = a_; this->_y = g_; }

/**  Set multiple components of vector.  */
inline void ag(const vector< component_type, 2 > & ag_)
{ this->_w = ag_.a(); this->_y = ag_.g(); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 3 > agr() const
{ return vector< component_type, 3 >(this->_w, this->_y, this->_x); }

/**  Set multiple components of vector as a new vector.  */
inline void agr(const component_type & a_, const component_type & g_, const component_type & r_)
{ this->_w = a_; this->_y = g_; this->_x = r_; }

/**  Set multiple components of vector as a new vector.  */
inline void agr(const component_type & a_, const vector< component_type, 2 > & gr_)
{ this->_w = a_; this->_y = gr_.g(); this->_x = gr_.r(); }

/**  Set multiple components of vector as a new vector.  */
inline void agr(const vector< component_type, 2 > & ag_, const component_type & r_)
{ this->_w = ag_.a(); this->_y = ag_.g(); this->_x = r_; }

/**  Set multiple components of vector as a new vector.  */
inline void agr(const vector< component_type, 3 > & agr_)
{ this->_w = agr_.a(); this->_y = agr_.g(); this->_x = agr_.r(); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > agrr() const
{ return vector< component_type, 4 >(this->_w, this->_y, this->_x, this->_x); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > agrg() const
{ return vector< component_type, 4 >(this->_w, this->_y, this->_x, this->_y); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > agrb() const
{ return vector< component_type, 4 >(this->_w, this->_y, this->_x, this->_z); }

/**  Set multiple components of vector as a new vector.  */
inline void agrb(const component_type & a_, const component_type & g_, const component_type & r_, const component_type & b_)
{ this->_w = a_; this->_y = g_; this->_x = r_; this->_z = b_; }

/**  Set multiple components of vector as a new vector.  */
inline void agrb(const component_type & a_, const vector< component_type, 3 > & grb_)
{ this->_w = a_; this->_y = grb_.g(); this->_x = grb_.r(); this->_z = grb_.b(); }

/**  Set multiple components of vector as a new vector.  */
inline void agrb(const vector< component_type, 3 > & agr_, const component_type & b_)
{ this->_w = agr_.a(); this->_y = agr_.g(); this->_x = agr_.r(); this->_z = b_; }

/**  Set multiple components of vector as a new vector.  */
inline void agrb(const component_type & a_, const component_type & g_, const vector< component_type, 2 > & rb_)
{ this->_w = a_; this->_y = g_; this->_x = rb_.r(); this->_z = rb_.b(); }

/**  Set multiple components of vector as a new vector.  */
inline void agrb(const component_type & a_, const vector< component_type, 2 > & gr_, const component_type & b_)
{ this->_w = a_; this->_y = gr_.g(); this->_x = gr_.r(); this->_z = b_; }

/**  Set multiple components of vector as a new vector.  */
inline void agrb(const vector< component_type, 2 > & ag_, const component_type & r_, const component_type & b_)
{ this->_w = ag_.a(); this->_y = ag_.g(); this->_x = r_; this->_z = b_; }

/**  Set multiple components of vector as a new vector.  */
inline void agrb(const vector< component_type, 2 > & ag_, const vector< component_type, 2 > & rb_)
{ this->_w = ag_.a(); this->_y = ag_.g(); this->_x = rb_.r(); this->_z = rb_.b(); }

/**  Set multiple components of vector as a new vector.  */
inline void agrb(const vector< component_type, 4 > & agrb_)
{ this->_w = agrb_.a(); this->_y = agrb_.g(); this->_x = agrb_.r(); this->_z = agrb_.b(); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > agra() const
{ return vector< component_type, 4 >(this->_w, this->_y, this->_x, this->_w); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 3 > agg() const
{ return vector< component_type, 3 >(this->_w, this->_y, this->_y); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > aggr() const
{ return vector< component_type, 4 >(this->_w, this->_y, this->_y, this->_x); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > aggg() const
{ return vector< component_type, 4 >(this->_w, this->_y, this->_y, this->_y); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > aggb() const
{ return vector< component_type, 4 >(this->_w, this->_y, this->_y, this->_z); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > agga() const
{ return vector< component_type, 4 >(this->_w, this->_y, this->_y, this->_w); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 3 > agb() const
{ return vector< component_type, 3 >(this->_w, this->_y, this->_z); }

/**  Set multiple components of vector as a new vector.  */
inline void agb(const component_type & a_, const component_type & g_, const component_type & b_)
{ this->_w = a_; this->_y = g_; this->_z = b_; }

/**  Set multiple components of vector as a new vector.  */
inline void agb(const component_type & a_, const vector< component_type, 2 > & gb_)
{ this->_w = a_; this->_y = gb_.g(); this->_z = gb_.b(); }

/**  Set multiple components of vector as a new vector.  */
inline void agb(const vector< component_type, 2 > & ag_, const component_type & b_)
{ this->_w = ag_.a(); this->_y = ag_.g(); this->_z = b_; }

/**  Set multiple components of vector as a new vector.  */
inline void agb(const vector< component_type, 3 > & agb_)
{ this->_w = agb_.a(); this->_y = agb_.g(); this->_z = agb_.b(); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > agbr() const
{ return vector< component_type, 4 >(this->_w, this->_y, this->_z, this->_x); }

/**  Set multiple components of vector as a new vector.  */
inline void agbr(const component_type & a_, const component_type & g_, const component_type & b_, const component_type & r_)
{ this->_w = a_; this->_y = g_; this->_z = b_; this->_x = r_; }

/**  Set multiple components of vector as a new vector.  */
inline void agbr(const component_type & a_, const vector< component_type, 3 > & gbr_)
{ this->_w = a_; this->_y = gbr_.g(); this->_z = gbr_.b(); this->_x = gbr_.r(); }

/**  Set multiple components of vector as a new vector.  */
inline void agbr(const vector< component_type, 3 > & agb_, const component_type & r_)
{ this->_w = agb_.a(); this->_y = agb_.g(); this->_z = agb_.b(); this->_x = r_; }

/**  Set multiple components of vector as a new vector.  */
inline void agbr(const component_type & a_, const component_type & g_, const vector< component_type, 2 > & br_)
{ this->_w = a_; this->_y = g_; this->_z = br_.b(); this->_x = br_.r(); }

/**  Set multiple components of vector as a new vector.  */
inline void agbr(const component_type & a_, const vector< component_type, 2 > & gb_, const component_type & r_)
{ this->_w = a_; this->_y = gb_.g(); this->_z = gb_.b(); this->_x = r_; }

/**  Set multiple components of vector as a new vector.  */
inline void agbr(const vector< component_type, 2 > & ag_, const component_type & b_, const component_type & r_)
{ this->_w = ag_.a(); this->_y = ag_.g(); this->_z = b_; this->_x = r_; }

/**  Set multiple components of vector as a new vector.  */
inline void agbr(const vector< component_type, 2 > & ag_, const vector< component_type, 2 > & br_)
{ this->_w = ag_.a(); this->_y = ag_.g(); this->_z = br_.b(); this->_x = br_.r(); }

/**  Set multiple components of vector as a new vector.  */
inline void agbr(const vector< component_type, 4 > & agbr_)
{ this->_w = agbr_.a(); this->_y = agbr_.g(); this->_z = agbr_.b(); this->_x = agbr_.r(); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > agbg() const
{ return vector< component_type, 4 >(this->_w, this->_y, this->_z, this->_y); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > agbb() const
{ return vector< component_type, 4 >(this->_w, this->_y, this->_z, this->_z); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > agba() const
{ return vector< component_type, 4 >(this->_w, this->_y, this->_z, this->_w); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 3 > aga() const
{ return vector< component_type, 3 >(this->_w, this->_y, this->_w); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > agar() const
{ return vector< component_type, 4 >(this->_w, this->_y, this->_w, this->_x); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > agag() const
{ return vector< component_type, 4 >(this->_w, this->_y, this->_w, this->_y); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > agab() const
{ return vector< component_type, 4 >(this->_w, this->_y, this->_w, this->_z); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > agaa() const
{ return vector< component_type, 4 >(this->_w, this->_y, this->_w, this->_w); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 2 > ab() const
{ return vector< component_type, 2 >(this->_w, this->_z); }

/**  Set multiple components of vector.  */
inline void ab(const component_type & a_, const component_type & b_)
{ this->_w = a_; this->_z = b_; }

/**  Set multiple components of vector.  */
inline void ab(const vector< component_type, 2 > & ab_)
{ this->_w = ab_.a(); this->_z = ab_.b(); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 3 > abr() const
{ return vector< component_type, 3 >(this->_w, this->_z, this->_x); }

/**  Set multiple components of vector as a new vector.  */
inline void abr(const component_type & a_, const component_type & b_, const component_type & r_)
{ this->_w = a_; this->_z = b_; this->_x = r_; }

/**  Set multiple components of vector as a new vector.  */
inline void abr(const component_type & a_, const vector< component_type, 2 > & br_)
{ this->_w = a_; this->_z = br_.b(); this->_x = br_.r(); }

/**  Set multiple components of vector as a new vector.  */
inline void abr(const vector< component_type, 2 > & ab_, const component_type & r_)
{ this->_w = ab_.a(); this->_z = ab_.b(); this->_x = r_; }

/**  Set multiple components of vector as a new vector.  */
inline void abr(const vector< component_type, 3 > & abr_)
{ this->_w = abr_.a(); this->_z = abr_.b(); this->_x = abr_.r(); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > abrr() const
{ return vector< component_type, 4 >(this->_w, this->_z, this->_x, this->_x); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > abrg() const
{ return vector< component_type, 4 >(this->_w, this->_z, this->_x, this->_y); }

/**  Set multiple components of vector as a new vector.  */
inline void abrg(const component_type & a_, const component_type & b_, const component_type & r_, const component_type & g_)
{ this->_w = a_; this->_z = b_; this->_x = r_; this->_y = g_; }

/**  Set multiple components of vector as a new vector.  */
inline void abrg(const component_type & a_, const vector< component_type, 3 > & brg_)
{ this->_w = a_; this->_z = brg_.b(); this->_x = brg_.r(); this->_y = brg_.g(); }

/**  Set multiple components of vector as a new vector.  */
inline void abrg(const vector< component_type, 3 > & abr_, const component_type & g_)
{ this->_w = abr_.a(); this->_z = abr_.b(); this->_x = abr_.r(); this->_y = g_; }

/**  Set multiple components of vector as a new vector.  */
inline void abrg(const component_type & a_, const component_type & b_, const vector< component_type, 2 > & rg_)
{ this->_w = a_; this->_z = b_; this->_x = rg_.r(); this->_y = rg_.g(); }

/**  Set multiple components of vector as a new vector.  */
inline void abrg(const component_type & a_, const vector< component_type, 2 > & br_, const component_type & g_)
{ this->_w = a_; this->_z = br_.b(); this->_x = br_.r(); this->_y = g_; }

/**  Set multiple components of vector as a new vector.  */
inline void abrg(const vector< component_type, 2 > & ab_, const component_type & r_, const component_type & g_)
{ this->_w = ab_.a(); this->_z = ab_.b(); this->_x = r_; this->_y = g_; }

/**  Set multiple components of vector as a new vector.  */
inline void abrg(const vector< component_type, 2 > & ab_, const vector< component_type, 2 > & rg_)
{ this->_w = ab_.a(); this->_z = ab_.b(); this->_x = rg_.r(); this->_y = rg_.g(); }

/**  Set multiple components of vector as a new vector.  */
inline void abrg(const vector< component_type, 4 > & abrg_)
{ this->_w = abrg_.a(); this->_z = abrg_.b(); this->_x = abrg_.r(); this->_y = abrg_.g(); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > abrb() const
{ return vector< component_type, 4 >(this->_w, this->_z, this->_x, this->_z); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > abra() const
{ return vector< component_type, 4 >(this->_w, this->_z, this->_x, this->_w); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 3 > abg() const
{ return vector< component_type, 3 >(this->_w, this->_z, this->_y); }

/**  Set multiple components of vector as a new vector.  */
inline void abg(const component_type & a_, const component_type & b_, const component_type & g_)
{ this->_w = a_; this->_z = b_; this->_y = g_; }

/**  Set multiple components of vector as a new vector.  */
inline void abg(const component_type & a_, const vector< component_type, 2 > & bg_)
{ this->_w = a_; this->_z = bg_.b(); this->_y = bg_.g(); }

/**  Set multiple components of vector as a new vector.  */
inline void abg(const vector< component_type, 2 > & ab_, const component_type & g_)
{ this->_w = ab_.a(); this->_z = ab_.b(); this->_y = g_; }

/**  Set multiple components of vector as a new vector.  */
inline void abg(const vector< component_type, 3 > & abg_)
{ this->_w = abg_.a(); this->_z = abg_.b(); this->_y = abg_.g(); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > abgr() const
{ return vector< component_type, 4 >(this->_w, this->_z, this->_y, this->_x); }

/**  Set multiple components of vector as a new vector.  */
inline void abgr(const component_type & a_, const component_type & b_, const component_type & g_, const component_type & r_)
{ this->_w = a_; this->_z = b_; this->_y = g_; this->_x = r_; }

/**  Set multiple components of vector as a new vector.  */
inline void abgr(const component_type & a_, const vector< component_type, 3 > & bgr_)
{ this->_w = a_; this->_z = bgr_.b(); this->_y = bgr_.g(); this->_x = bgr_.r(); }

/**  Set multiple components of vector as a new vector.  */
inline void abgr(const vector< component_type, 3 > & abg_, const component_type & r_)
{ this->_w = abg_.a(); this->_z = abg_.b(); this->_y = abg_.g(); this->_x = r_; }

/**  Set multiple components of vector as a new vector.  */
inline void abgr(const component_type & a_, const component_type & b_, const vector< component_type, 2 > & gr_)
{ this->_w = a_; this->_z = b_; this->_y = gr_.g(); this->_x = gr_.r(); }

/**  Set multiple components of vector as a new vector.  */
inline void abgr(const component_type & a_, const vector< component_type, 2 > & bg_, const component_type & r_)
{ this->_w = a_; this->_z = bg_.b(); this->_y = bg_.g(); this->_x = r_; }

/**  Set multiple components of vector as a new vector.  */
inline void abgr(const vector< component_type, 2 > & ab_, const component_type & g_, const component_type & r_)
{ this->_w = ab_.a(); this->_z = ab_.b(); this->_y = g_; this->_x = r_; }

/**  Set multiple components of vector as a new vector.  */
inline void abgr(const vector< component_type, 2 > & ab_, const vector< component_type, 2 > & gr_)
{ this->_w = ab_.a(); this->_z = ab_.b(); this->_y = gr_.g(); this->_x = gr_.r(); }

/**  Set multiple components of vector as a new vector.  */
inline void abgr(const vector< component_type, 4 > & abgr_)
{ this->_w = abgr_.a(); this->_z = abgr_.b(); this->_y = abgr_.g(); this->_x = abgr_.r(); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > abgg() const
{ return vector< component_type, 4 >(this->_w, this->_z, this->_y, this->_y); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > abgb() const
{ return vector< component_type, 4 >(this->_w, this->_z, this->_y, this->_z); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > abga() const
{ return vector< component_type, 4 >(this->_w, this->_z, this->_y, this->_w); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 3 > abb() const
{ return vector< component_type, 3 >(this->_w, this->_z, this->_z); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > abbr() const
{ return vector< component_type, 4 >(this->_w, this->_z, this->_z, this->_x); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > abbg() const
{ return vector< component_type, 4 >(this->_w, this->_z, this->_z, this->_y); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > abbb() const
{ return vector< component_type, 4 >(this->_w, this->_z, this->_z, this->_z); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > abba() const
{ return vector< component_type, 4 >(this->_w, this->_z, this->_z, this->_w); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 3 > aba() const
{ return vector< component_type, 3 >(this->_w, this->_z, this->_w); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > abar() const
{ return vector< component_type, 4 >(this->_w, this->_z, this->_w, this->_x); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > abag() const
{ return vector< component_type, 4 >(this->_w, this->_z, this->_w, this->_y); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > abab() const
{ return vector< component_type, 4 >(this->_w, this->_z, this->_w, this->_z); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > abaa() const
{ return vector< component_type, 4 >(this->_w, this->_z, this->_w, this->_w); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 2 > aa() const
{ return vector< component_type, 2 >(this->_w, this->_w); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 3 > aar() const
{ return vector< component_type, 3 >(this->_w, this->_w, this->_x); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > aarr() const
{ return vector< component_type, 4 >(this->_w, this->_w, this->_x, this->_x); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > aarg() const
{ return vector< component_type, 4 >(this->_w, this->_w, this->_x, this->_y); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > aarb() const
{ return vector< component_type, 4 >(this->_w, this->_w, this->_x, this->_z); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > aara() const
{ return vector< component_type, 4 >(this->_w, this->_w, this->_x, this->_w); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 3 > aag() const
{ return vector< component_type, 3 >(this->_w, this->_w, this->_y); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > aagr() const
{ return vector< component_type, 4 >(this->_w, this->_w, this->_y, this->_x); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > aagg() const
{ return vector< component_type, 4 >(this->_w, this->_w, this->_y, this->_y); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > aagb() const
{ return vector< component_type, 4 >(this->_w, this->_w, this->_y, this->_z); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > aaga() const
{ return vector< component_type, 4 >(this->_w, this->_w, this->_y, this->_w); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 3 > aab() const
{ return vector< component_type, 3 >(this->_w, this->_w, this->_z); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > aabr() const
{ return vector< component_type, 4 >(this->_w, this->_w, this->_z, this->_x); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > aabg() const
{ return vector< component_type, 4 >(this->_w, this->_w, this->_z, this->_y); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > aabb() const
{ return vector< component_type, 4 >(this->_w, this->_w, this->_z, this->_z); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > aaba() const
{ return vector< component_type, 4 >(this->_w, this->_w, this->_z, this->_w); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 3 > aaa() const
{ return vector< component_type, 3 >(this->_w, this->_w, this->_w); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > aaar() const
{ return vector< component_type, 4 >(this->_w, this->_w, this->_w, this->_x); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > aaag() const
{ return vector< component_type, 4 >(this->_w, this->_w, this->_w, this->_y); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > aaab() const
{ return vector< component_type, 4 >(this->_w, this->_w, this->_w, this->_z); }

/**  Get multiple components of vector as a new vector.  */
inline vector< component_type, 4 > aaaa() const
{ return vector< component_type, 4 >(this->_w, this->_w, this->_w, this->_w); }
