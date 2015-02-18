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
class Log(object):
    def __init__(self):
        self.__d = dict.fromkeys(('vm','cpu','mem',
                                  'brk','reg','ads',
                                  'evt','gdb', 'os'),
                                 False)
        self.__d["error"] = True

    # ie: vm=True, all=False, evt=False, ...
    def setup(self, **kwargs):
        if kwargs is None:
            return
        for k,v in kwargs.iteritems():
            if k == 'all':
                for n in self.__d:
                    self.__d[n] = v
            # create/modify
            else:
                self.__d[k] = v

    def __call__(self, k, string):
        if not k in self.__d or not self.__d[k]:
            return
        print string

# usable object
log = Log()
