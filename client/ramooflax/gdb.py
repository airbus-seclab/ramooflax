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
import socket, errno, sys
from event import StopReason
from utils import Utils

class GDBDiag():
    ok = 0
    ns = 1
    er = 2
    un = 3
    def __init__(self, typ, data=None):
        self._dico = {GDBDiag.ok:"OK",
                      GDBDiag.ns:"Unsupported",
                      GDBDiag.er:"Error",
                      GDBDiag.un:"Unknown",
                      }
        self.data = data
        self.type = typ

    def __str__(self):
        return self._dico.get(self.value, None)

class GDBError(Exception):
    generic = 0x00
    mem     = 0x0e
    oom     = 0x0c
    invalid = 0x16

    def __init__(self, value):
        self.value = value
        self._dico = {GDBError.generic:"generic",
                      GDBError.mem:"memory",
                      GDBError.oom:"out of memory",
                      GDBError.invalid:"invalid",
                      GDBError.hdr:"unknown header",
                      }
    def __str__(self):
        return self._dico.get(self.value, "unknown")

class GDB:
    def __init__(self, ip, port, mode):
        self.ip = ip
        self.port = port
        if mode == "udp":
            self.mode = socket.SOCK_DGRAM
        else:
            self.mode = socket.SOCK_STREAM

        self.__sk = None
        self.__cache = ""
        self.__waiting = False
        self.__pool = { "stop":{0:[]}, "diag":{0:[]}, "data":{} }

    def __get_sk(self):
        return self.__sk

    def __connect(self):
        self.__sk = socket.socket(socket.AF_INET, self.mode)
        try:
            self.__sk.connect((self.ip,self.port))
        except socket.error as (err, msg):
            print "connect to \""+str(self.ip),str(self.port)+"\":",msg
            raise

    def waiting(self):
        return self.__waiting

    def __recv(self, chunck):
        while True:
            try:
                self.__waiting = True
                data = self.__sk.recv(chunck)
                self.__waiting = False
                dl = len(data)
                if dl == 0:
                    raise socket.error(errno.EIO, "Input/Output error")
                return (data,dl)
            except socket.error as (err, msg):
                if err != errno.EINTR:
                    print "recv:",msg
                    raise

    def __send(self, data, sz):
        done = 0
        while done < sz:
            try:
                sent  = self.__sk.send(data)
                done += sent
                data  = data[sent:]
                if Utils.debug:
                    print "sent",sent,str(done)+"/"+str(sz)
            except socket.error as (err, msg):
                print "send:",msg
                raise

    def __quit(self):
        try:
            self.__sk.shutdown(socket.SHUT_RDWR)
            self.__sk.close()
        except socket.error as (err, msg):
            print "disconnect:",msg
            raise
        else:
            print "disconnected from remote"

    def connect(self):
        try:
            self.__connect()
            self.ack()
        except:
            raise

    def checksum(self, data):
        chk = 0
        for c in data:
            chk = (chk+ord(c))%256
        return chk

    def __cache_fill(self):
        dt, sz = self.__recv(4096)
        self.__cache += dt

        if Utils.debug:
            print "cache fill %d/%d" % (sz,len(self.__cache))

        return sz

    def __cache_get(self, n):
        while len(self.__cache) < n:
            self.__cache_fill()

        dt = self.__cache[:n]
        self.__cache = self.__cache[n:]
        return dt

    def __cache_insert(self, dt):
        self.__cache = dt+self.__cache
        if Utils.debug:
            print "re-insert into cache:",repr(dt)

    def ack(self):
        if Utils.debug:
            print "send ACK"
        self.__send("+", 1)

    def nak(self):
        if Utils.debug:
            print "send NAK"
        self.__send("-", 1)

    def wait_ack(self):
        if Utils.debug:
            print "wait ACK/NAK"

        ack = self.__cache_get(1)
        if ack == '+':
            if Utils.debug:
                print "rcv ACK"
            return True

        if ack == '-':
            if Utils.debug:
                print "rcv NAK"
            return False

        # de-synchronized, ignore
        if Utils.debug:
            print "rcv ?? ACK/NAK =",ack
        self.__cache_insert(ack)
        return True

    def send_raw(self, data, wack=True):
        while True:
            self.__send(data, len(data))
            if Utils.debug:
                print "sent",repr(data)
            if not wack or self.wait_ack():
                break

    def send_pkt(self, data, wack=True):
        pkt = '$' + data + '#' + "%.2x" % (self.checksum(data))
        self.send_raw(pkt, wack)

    def send_vmm_pkt(self, data):
        self.send_pkt("\x05"+data)

    def recv_raw(self, expected):
        return self.__cache_get(expected)

    def __recv_pkt(self, sack):
        ws = 0
        pkt = None
        while pkt is None:
            while len(self.__cache) < 4:
                self.__cache_fill()

            if Utils.debug:
                print "rcv pkt: search [%d:] = %r" % (ws,repr(self.__cache[ws:]))
            tag = self.__cache[ws:].find('#')
            if tag == -1:
                ws = len(self.__cache)
                self.__cache_fill()
                continue

            psz = ws+tag+3
            while len(self.__cache) < psz:
                self.__cache_fill()

            pkt = self.__parse_pkt(self.__cache_get(psz), ws+tag, sack)

        return pkt

    def __parse_pkt(self, pkt, tag, sack=True):
        #XXX: prevent
        while pkt[0] == '+' or pkt[0] == '-':
            if Utils.debug:
                print "lost ACK/NAK ... ignore"
            pkt  = pkt[1:]
            tag -= 1

        if pkt[0] != '$':
            if Utils.debug:
                print "rcv unknown pkt:", repr(pkt)
            if sack:
                self.nak()
            return None

        msg = pkt[1:tag]
        pchk = int(pkt[tag+1:], 16)
        vchk = self.checksum(msg)

        if pchk != vchk:
            if Utils.debug:
                print "bad pkt checksum: %r | %r = 0x%x" \
                    % (repr(pkt),repr(msg),vchk)
            if sack:
                self.nak()
            return None

        if sack:
            self.ack()

        sz = len(msg)
        if sz < 4:
            return self.__parse_diag(msg, sz)

        if msg[0] == 'T':
            return self.__parse_stop(msg, sz)

        return msg

    # TXXmd:YY;04:a..a;05:b..b;[16|08]:c..c;
    def __parse_stop(self, msg, sz):
        msg = msg[1:-1].split(';')
        hdr = msg.pop(0)

        rson = int(hdr[:2], 16)
        mode = int(hdr[5:], 16)
        stop = StopReason(rson, mode, msg)

        if Utils.debug:
            print "rcv stop reason", stop

        return stop

    def __parse_diag(self, msg, sz):
        if sz == 0:
            if Utils.debug:
                print "rcv Unsupported"
            return GDBDiag(GDBDiag.ns)

        if sz == 2 and msg == "OK":
            if Utils.debug:
                print "rcv OK"
            return GDBDiag(GDBDiag.ok)

        if sz == 3 and msg[0] == 'E':
            er = GDBError(int(msg,16))
            if Utils.debug:
                print "recv ERR",er
            return GDBDiag(GDBDiag.er, er)

        return GDBDiag(GDBDiag.un, msg)

    def __pool_get(self, key, sz, sack):
        pool = self.__pool[key]

        if not pool.has_key(sz):
            pool[sz] = []

        while len(pool[sz]) == 0:
            pkt = self.__recv_pkt(sack)

            if isinstance(pkt, GDBDiag):
                self.__pool["diag"][0].append(pkt)
            elif isinstance(pkt, StopReason):
                self.__pool["stop"][0].append(pkt)
            else:
                if not self.__pool["data"].has_key(len(pkt)):
                     self.__pool["data"][len(pkt)] = []

                self.__pool["data"][len(pkt)].append(pkt)

        return pool[sz].pop(0)

    def recv_diag(self, sack=True):
        return self.__pool_get("diag", 0, sack)

    def recv_stop(self):
        return self.__pool_get("stop", 0, sack=False)

    def recv_pkt(self, sz, sack=True):
        return self.__pool_get("data", sz, sack)

    def intr(self):
        if Utils.debug:
            print "send intr"
        self.send_raw("\x03", False)

    def stop_reason(self):
        if Utils.debug:
            print "send ?"
        self.send_raw("$?#3f", False)

    def resume(self, addr=None):
        if Utils.debug:
            print "send c"
        self.send_raw("$c#63")

    def singlestep(self, addr=None):
        if Utils.debug:
            print "send s"
        self.send_raw("$s#73")

    def quit(self, quick=False):
        if Utils.debug:
            print "send quit"
        self.send_raw("$k#6b")
        if not quick:
            self.recv_diag(sack=False)
        self.__quit()

    def read_all_gpr(self, sz):
        self.send_pkt("g")
        return self.recv_pkt(sz)

    def write_all_gpr(self, data):
        #XXX
        print "NOT IMPLEMENTED"
        # self.send_pkt("G"+data)
        # self.recv_pkt(2)

    def read_gpr(self, n, sz):
        self.send_pkt("p%.2x" % (n))
        return self.recv_pkt(sz)

    def write_gpr(self, n, data):
        self.send_pkt("P%.2x=%s" % (n,data))
        self.recv_diag()

    def read_mem(self, addr, n):
        self.send_pkt("m"+addr+","+str(n))
        return self.recv_pkt(n*2)

    def write_mem(self, addr, val, n):
        self.send_pkt("M"+addr+","+str(n)+":"+val)
        self.recv_diag()

    def set_mem_break(self, addr):
        self.send_pkt("Z0,"+addr+",1")
        self.recv_diag()

    def del_mem_break(self, addr):
        self.send_pkt("z0,"+addr+",1")
        self.recv_diag()

    def set_hrd_break(self, addr, knd, sz):
        self.send_pkt("Z"+knd+","+addr+","+sz)
        self.recv_diag()

    def del_hrd_break(self, addr, knd, sz):
        self.send_pkt("z"+knd+","+addr+","+sz)
        self.recv_diag()

    def read_all_sr(self, sz):
        self.send_vmm_pkt("\x80")
        return self.recv_pkt(sz)

    def write_all_sr(self, data):
        #XXX
        print "NOT IMPLEMENTED"
        # self.send_vmm_pkt("\x81"+data)
        # self.recv_diag()

    def read_sr(self, n, sz):
        self.send_vmm_pkt("\x82%.2x" % (n))
        return self.recv_pkt(sz)

    def write_sr(self, n, data):
        self.send_vmm_pkt("\x83%.2x=%s" % (n,data))
        self.recv_diag()

    def set_lbr(self):
        self.send_vmm_pkt("\x84")
        self.recv_diag()

    def del_lbr(self):
        self.send_vmm_pkt("\x85")
        self.recv_diag()

    def get_lbr(self):
        self.send_vmm_pkt("\x86")
        return self.recv_pkt(4*8*2)

    def read_exception_mask(self):
        self.send_vmm_pkt("\x87")
        return self.recv_pkt(1*4*2)

    def write_exception_mask(self, data):
        self.send_vmm_pkt("\x88"+data)
        self.recv_diag()

    def set_active_cr3(self, data):
        self.send_vmm_pkt("\x89"+data)
        self.recv_diag()

    def del_active_cr3(self):
        self.send_vmm_pkt("\x8a")
        self.recv_diag()

    def read_pmem(self, data, sz):
        self.send_vmm_pkt("\x8b"+data)
        self.recv_diag()
        return self.recv_raw(sz)

    def write_pmem(self, cmd, data, sz):
        self.send_vmm_pkt("\x8c"+cmd)
        self.recv_diag()
        self.__send(data, sz)

    def read_vmem(self, data, sz):
        self.send_vmm_pkt("\x8d"+data)
        self.recv_diag()
        return self.recv_raw(sz)

    def write_vmem(self, cmd, data, sz):
        self.send_vmm_pkt("\x8e"+cmd)
        self.recv_diag()
        self.__send(data, sz)

    def translate(self, data, sz):
        self.send_vmm_pkt("\x8f"+data)
        return self.recv_pkt(sz)

    def read_cr_read_mask(self):
        self.send_vmm_pkt("\x90")
        return self.recv_pkt(1*2*2)

    def write_cr_read_mask(self, data):
        self.send_vmm_pkt("\x91"+data)
        self.recv_diag()

    def read_cr_write_mask(self):
        self.send_vmm_pkt("\x92")
        return self.recv_pkt(1*2*2)

    def write_cr_write_mask(self, data):
        self.send_vmm_pkt("\x93"+data)
        self.recv_diag()

    def keep_active_cr3(self):
        self.send_vmm_pkt("\x94")
        self.recv_diag()

    def set_affinity(self, data):
        self.send_vmm_pkt("\x95"+data)
        self.recv_diag()

    def clear_exception(self):
        self.send_vmm_pkt("\x96")
        self.recv_diag()
