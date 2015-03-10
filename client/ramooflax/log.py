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
    reset   = "\033[0m"
    black   = "\033[30m"
    red     = "\033[31m"
    green   = "\033[32m"
    yellow  = "\033[33m"
    blue    = "\033[34m"
    magenta = "\033[35m"
    cyan    = "\033[36m"
    white   = "\033[37m"

    def __init__(self):
        keys = ('vm','cpu','mem',
                'brk','reg','ads',
                'evt','gdb', 'os')

        self.__tag = dict.fromkeys(keys, False)
        self.__col = dict.fromkeys(keys, Log.reset)
        self.__xset("error", True, Log.reset)

    def __xset(self, k, on, cl):
        self.__tag[k] = on
        self.__col[k] = cl

    def __set(self, k, v):
        if type(v) is tuple:
            self.__xset(k, v[0], v[1])
        else:
            self.__xset(k, v, Log.reset)

    # ie: vm=True, all=False, evt=(False,Log.blue), ...
    def setup(self, **kwargs):
        if kwargs is None:
            return

        for k,v in kwargs.iteritems():
            if k == 'all':
                for n in self.__tag:
                    self.__set(n,v)
            else: # create/modify
                self.__set(k,v)

    def __call__(self, k, string):
        if not k in self.__tag or not self.__tag[k]:
            return
        print self.__col[k]+string+Log.reset

# usable object
log = Log()
