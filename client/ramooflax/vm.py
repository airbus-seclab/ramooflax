#
# Copyright (C) 2011 EADS France, stephane duverger <stephane.duverger@eads.net>
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
import code, readline, rlcompleter
import sys, signal

import cpu
import memory
import gdb

from utils import Utils

class VMState:
    waiting     = 0
    working     = 1
    interactive = 2
    ready       = 3

class VM:
    """
Virtual Machine Controller
 - you can type ^D while interactive to let vm run
 - you can type ^C while not-interactive to get control back
 - you can type ^C while interactive to quit
 - you can access 'cpu' and 'mem' attributes (help(vm.cpu) ...)
    """
    def __init__(self, manufacturer, loc, mode="udp"):
        try:
            self.ip, self.port = loc.split(':')
            self.port = int(self.port)
            self.mode = mode
        except ValueError:
            print "give \"ip:port\" string"
            raise

        self.__variables = {}
        self.__session = False
        self.__state = VMState.working
        self.__stop_request = False
        self.__gdb = gdb.GDB(self.ip, self.port, self.mode)

        self.cpu = cpu.CPU(manufacturer, self.__gdb)
        self.mem = memory.Memory(self.cpu, self.__gdb)

        self.__setup_sig(signal.SIGINT, self.int_event)

    def __setup_sig(self, signum, handler):
        signal.signal(signum, handler)

    def __set_state(self, new_state):
        self.__state = new_state

    def __process_stop_request(self):
        self.__stop_request = False
        self.cpu._stop()

    def attach(self):
        try:
            self.__gdb.connect()
        except:
            raise

    def detach(self, quick=False):
        try:
            self.cpu._quit()
            self.mem._quit()
            self.__gdb.quit(quick)
        except:
            print "failed to detach properly"
        finally:
            sys.exit(0)

    def run(self, variables):
        self.__variables = variables
        self.attach()
        self.stop()
        self.__dispatch()

    def interact(self, variables):
        self.__variables = variables
        self.__dispatch()

    def interact2(self, variables):
        self.__variables = variables
        self.__dispatch2()

    def singlestep(self):
        self.__set_state(VMState.working)
        self.cpu._singlestep()
        self.__set_state(VMState.ready)
        return self.answer()

    def answer(self):
        self.__wait()
        return self.__process()

    def stop(self):
        self.__set_state(VMState.working)
        self.cpu._stop()
        self.__set_state(VMState.ready)
        return self.answer()

    def resume(self):
        self.__set_state(VMState.working)
        if self.__stop_request:
            self.__process_stop_request()
        else:
            self.mem._resume()
            self.cpu._resume()
        self.__set_state(VMState.ready)
        return self.answer()

    def __interact(self):
        self.__set_state(VMState.interactive)
        readline.parse_and_bind("tab: complete")
        code.interact("", local=self.__variables)
        self.__set_state(VMState.ready)

    def __wait(self):
        self.__set_state(VMState.waiting)
        self.cpu._recv_stop()
        self.__set_state(VMState.ready)

    def __process(self):
        self.__set_state(VMState.working)
        can_interact = self.cpu._process_stop(self)
        self.__set_state(VMState.ready)
        return can_interact

    # Keep on interacting on each stop reason
    # for which handler returns True
    def __dispatch(self):
        self.__session = True
        inter = True
        while inter:
            self.__interact()
            inter = self.resume()

    # Usefull when handler should not trigger
    # interactive mode, but still giving the
    # opportunity to manually break the VM (ctrl+c)
    #
    # similar to  'while True: vm.interact()'
    def __dispatch2(self):
        self.__session = True
        while True:
            if self.resume():
                self.__interact()

    def __ask_quit(self):
        sys.stdout.write("Really quit (y/n) ?")
        sys.stdout.flush()
        if sys.stdin.read(1) == 'y':
            return True
        return False

    def int_event(self, signum, frame):
        if self.__state == VMState.interactive or not self.__session:
            if self.__ask_quit():
                self.detach(True)
        elif self.__state == VMState.waiting and self.__gdb.waiting():
            self.__process_stop_request()
        else:
            self.__stop_request = True

        self.__setup_sig(signum, self.int_event)
