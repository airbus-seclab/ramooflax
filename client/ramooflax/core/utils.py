#
# Copyright (C) 2015 EADS France, stephane duverger <stephane.duverger@eads.net>
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License along
# with this program; if not, write to the Free Software Foundation, Inc.,
# 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
#
import log
import inspect, struct

def in_completer():
    return (inspect.stack()[3][3] == "attr_matches")

def pytrace():
    trace = []
    for stk in inspect.stack()[::-1][1:-2]:
        fil = stk[1].split('/')[-1]
        lin = stk[2]
        fct = stk[3]
        cod = stk[4][0][:-1].strip()
        trace.append("from %s:%s: in %s(): %s" % (fil,lin,fct,cod))
    return trace

def revert_string_bytes(s):
    rs = []
    for i in range(len(s)/2):
        rs.append(s[-2:])
        s = s[:-2]
    return "".join(rs)
