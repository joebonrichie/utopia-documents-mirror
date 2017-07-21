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

import utopia.citation
import uuid

## Test keyspecs

# All the following keyspecs are equivalent, and should all normalise to be
# identical with the first of them.

def test_keyspecs():
    kss = [
        'a/b/c:d',
        'a/b/c#d',
        'a.b.c:d',
        'a.b.c#d',
        'a[b][c]:d',
        'a[b][c]#d',
        'a/b.c:d',
        'a[b]/c#d'
    ]

    for ks in kss:
        nks = utopia.citation.normalise_keyspec(ks)
        assert(nks == kss[0])

# Picking from objects with varying keyspecs should be well behaved

key1 = 'abcdefg1'
key2 = 'abcdefg2'
key3 = 'abcdefg3'

obj1 = {'key': key1, 'a': 'b1', 'c': {'d1': 'e1'}, 'links': ['g1', 'h1', 'link'], 'i': ['j1', {'k': 'l1', 'm': ['n1', 'o1']}]}
obj2 = {'key': key2, 'provenance': {'whence': 'place'}, 'a': 'b2', 'c': {'d2': 'e2'}, 'links': ['g2', 'h2', 'link'], 'i': ['j2', {'k': 'l2', 'm': ['n2', 'o2']}]}
obj3 = {'key': key3, 'provenance': {'whence': 'place'}, 'a': 'b3', 'p': 'q'}

def test_picking():
    assert(utopia.citation.pick(obj1, 'a', None) == 'b1')
    assert(utopia.citation.pick(obj1, 'c/d1', None) == 'e1')
    assert(utopia.citation.pick(obj1, 'links', None) == ['g1', 'h1', 'link'])
    assert(utopia.citation.pick(obj1, 'links[0]', None) == 'g1')
    assert(utopia.citation.pick(obj1, 'i/1.k', None) == 'l1')
    assert(utopia.citation.pick(obj1, 'i/1[m].1', None) == 'o1')

## Test refspec generation

def test_refspecs():
    rs1 = utopia.citation.refspec(obj1, 'links.0')
    rs2 = utopia.citation.refspec(obj2, 'i.1/m[1]')

    assert(rs1 == '@abcdefg1:links/0')
    assert(rs2 == '@abcdefg2:i/1/m/1')

## Test picking from multiple

objs = [obj1, obj2, obj3]

def test_pick_from():
    assert(utopia.citation.pick_from(objs, 'a') == 'b1')
    assert(utopia.citation.pick_from(objs, 'a:') == 'b1')
    assert(utopia.citation.pick_from(objs, 'a:*') == ['b1', 'b2', 'b3'])
    assert(utopia.citation.pick_from(objs, 'a:place') == 'b2')
    assert(utopia.citation.pick_from(objs, 'a:place*') == ['b2', 'b3'])
    assert(utopia.citation.pick_from(objs, 'links') == ['g1', 'h1', 'link', 'g2', 'h2'])
    assert(utopia.citation.pick_from(objs, 'links:*') == [['g1', 'h1', 'link'], ['g2', 'h2', 'link']])
    assert(utopia.citation.pick_from(objs, 'i') == ['j1', {'k': 'l1', 'm': ['n1', 'o1']}])
    assert(utopia.citation.pick_from(objs, 'c/d2') == 'e2')
    assert(utopia.citation.pick_from(objs, 'c/d2:place') == 'e2')
    assert(utopia.citation.pick_from(objs, 'c/d2:place*') == ['e2'])

    f = utopia.citation.flatten(objs)
    import pprint
    pprint.PrettyPrinter(indent=2).pprint(f)


test_pick_from()

