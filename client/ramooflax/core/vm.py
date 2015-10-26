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
import code, readline, rlcompleter
import sys, signal

import cpu
import memory
import gdb
import event
import log

class VMState:
    waiting     = 0
    working     = 1
    interactive = 2
    ready       = 3

    def __init__(self, s):
        self.__state = s
        self.__p_state = VMState.ready
        self.__str = {self.waiting:"waiting",
                      self.working:"working",
                      self.interactive:"interactive",
                      self.ready:"ready"}

    def update(self, s):
        self.__p_state = self.__state
        self.__state = s

    def restore(self):
        self.__state = self.__p_state

    def __str__(self):
        return self.__str.get(self.__state, "unknown")

    def __eq__(self, s):
        return self.__state == s

class VM:
    """
Virtual Machine Controller
 - you can type ^D while interactive to let vm run
 - you can type ^C while not-interactive to get control back
 - you can type ^C while interactive to quit
 - you can access 'cpu' and 'mem' attributes (help(vm.cpu) ...)
    """
    def __init__(self, manufacturer, loc, mode="udp"):
        self.__state = VMState(VMState.working)

        try:
            self.ip, self.port = loc.split(':')
            self.port = int(self.port)
            self.mode = mode
        except ValueError:
            log.log("error", "give \"ip:port\" string")
            raise

        self.__variables = {}
        self.__session = False
        self.__detach_filter = None
        self.__intr_filter = None
        self.__stop_request = False
        self.__gdb = gdb.GDB(self.ip, self.port, self.mode)

        self.cpu = cpu.CPU(manufacturer, self.__gdb)
        self.mem = memory.Memory(self.cpu, self.__gdb)
        self.__setup_sig(signal.SIGINT, self.int_event)

        self.__state.update(VMState.ready)

    def __setup_sig(self, signum, handler):
        signal.signal(signum, handler)

    def attach(self):
        try:
            self.__gdb.connect()
        except:
            raise

    def filter_detach(self, hdl):
        self.__detach_filter = hdl

    def filter_intr(self, hdl):
        self.__intr_filter = hdl

    # Quit properly
    # - if interactive (session) exit program
    # - can force exit (leave=True)
    # - can prevent waiting gdb 'kill' answer (quick=True)
    def detach(self, quick=False, leave=False):
        if self.__detach_filter is not None:
            self.__detach_filter(self)

        try:
            self.cpu._quit()
            self.mem._quit()
            self.__gdb.quit(quick)
        except:
            log.log("error", "failed to detach properly")
        finally:
            self.__setup_sig(signal.SIGINT, signal.SIG_DFL)
            if self.__session or leave:
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

    def interact_noresume(self):
        self.__interact()

    def answer(self, what=event.StopReason.every):
        inter = False # on 1st True
        while True:
            self.__wait()
            if self.__process() and not inter:
                inter = True

            if what == event.StopReason.every and not self.cpu.has_pending_reason():
                return inter

            if what == self.cpu.last_reason:
                if what != event.StopReason.gdb_trap or \
                        (self.cpu.sr.dr6 & (1<<14)) != 0:
                    return inter

    def stop(self):
        self.__stop()
        return self.answer(event.StopReason.gdb_int)

    def resume(self):
        if self.__stop_request:
            return self.stop()

        self.__resume()
        return self.answer()

    def singlestep(self):
        self.__resume(True)
        return self.answer(event.StopReason.gdb_trap)

    def __stop(self):
        if self.__stop_request:
            self.__stop_request = False
        self.__state.update(VMState.working)
        log.log("vm", "send stop request")
        self.cpu._stop()
        self.__state.restore()

    def __resume(self, sstep=False):
        self.__state.update(VMState.working)
        self.mem._resume()
        self.cpu._resume(sstep=sstep)
        self.__state.restore()

    def __wait(self):
        self.__state.update(VMState.waiting)
        self.cpu._recv_stop()
        self.__state.restore()

    def __process(self):
        self.__state.update(VMState.working)
        can_interact = self.cpu._process_stop(self)
        self.__state.restore()
        return can_interact

    def __interact(self):
        self.__state.update(VMState.interactive)
        readline.parse_and_bind("tab: complete")
        code.interact("", local=self.__variables)
        self.__state.restore()

    # Keep on interacting on each stop reason
    # for which handler returns True
    def __dispatch(self):
        self.__session = True
        inter = True
        while inter:
            self.__interact()
            inter = self.resume()
        self.__session = False

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
        self.__setup_sig(signum, self.int_event)

        log.log("vm", "interrupting (vm.state = %s) ..." % self.__state)
        log.log("vm", "gdb pkt pools:\n%s" % self.__gdb.pool_dump())

        if self.__intr_filter is not None:
            self.__intr_filter(self)

        if self.__state == VMState.interactive or not self.__session:
            if self.__ask_quit():
                self.detach(quick=True)
        elif self.__state == VMState.waiting and self.__gdb.waiting():
            self.__stop()
        else:
            self.__stop_request = True
