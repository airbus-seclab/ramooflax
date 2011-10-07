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
import socket, errno

from utils import Utils

class GDB:
    def __init__(self, ip, port):
        self.ip = ip
        self.port = port
        self.__sk = None
        self.__cache = ""
        self.__waiting = False

    def __get_sk(self):
        return self.__sk

    def __connect(self):
        self.__sk = socket.socket()
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
        if Utils.debug:
            print "about to send",repr(data[0])
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

    def _send(self, data):
        self.__send(data, len(data))

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

    def ack(self):
        if Utils.debug:
            print "send ACK"
        self._send("+")

    def nak(self):
        if Utils.debug:
            print "send NAK"
        self._send("-")

    def intr(self):
        if Utils.debug:
            print "send INTR"
        self._send("\x03")

    def quit(self):
        if Utils.debug:
            print "send QUIT"
        self._send("$k#6b")
        self.recv_pkt(2, s_ack=False)
        self.__quit()

    def checksum(self, data):
        chk = 0
        for c in data:
            chk = (chk+ord(c))%256
        return chk

    def send_pkt(self, data):
        pkt = '$' + data + '#' + "%.2x" % (self.checksum(data))
        self._send(pkt)
        if Utils.debug:
            print "sent",repr(pkt)

    def recv_pkt(self, sz=0, r_ack=True, s_ack=True):
        if sz != 0:
            sz += 4 # $+sz+#+XX
        if r_ack:
            sz += 1 # ack/nak
        return self._recv_pkt(sz, ack=s_ack)

    def recv_raw(self, expected):
        if Utils.debug:
            print "receiving raw data"
        l = len(self.__cache)
        while l < expected:
            data, dl = self.__recv(expected - l)
            l += dl
            if Utils.debug:
                print "\r%d/%d" % (l, expected),
            self.__cache += data

        if Utils.debug:
            print ""

        data = self.__cache[:expected]
        self.__cache = self.__cache[expected:]
        return data

    def _recv_pkt(self, expected, chunck=4096, ack=True):
        l = len(self.__cache)
        if Utils.debug:
            print l,"in cache,",expected,"expected"

        while l < expected:
            data, dl = self.__recv(chunck)
            l += dl
            self.__cache += data

            if Utils.debug:
                print l,"in cache,",expected,"expected"

            if "$E" in self.__cache:
                break

        if Utils.debug:
            print "cache", repr(self.__cache)

        done = 0
        data = None
        while expected > 0:
            header = self.__cache[0]

            if header == '+':
                if Utils.debug:
                    print "ACK received"
                done = 1
            elif header == '$':
                done,data = self.__parse_pkt(self.__cache[:expected], ack)
            elif header == '-':
                print "NAK received !"
                return
            else:
                print "unknown header", repr(header)
                return

            self.__cache = self.__cache[done:]
            expected -= done

            if Utils.debug:
                print "still",len(self.__cache),"in cache"
                print "done",done,"still",expected,"to process"

        return data

    def __parse_pkt(self, pkt, ack=True):
        sh = pkt.find('#')
        el = sh+3
        rl = len(pkt)

        if sh == -1 or rl < el:
            print "bad packet:", repr(pkt)
            self.nak()
            return (rl, None)

        try:
            pkt = pkt[:el]
            content = pkt[1:sh]
            chk = int(pkt[sh+1:], 16)
        except:
            print "pkt",repr(pkt)

        if chk != self.checksum(content):
            print "bad packet:", repr(pkt)
            self.nak()
            return (el,None)

        if ack:
            self.ack()

        cl = len(content)
        if cl < 4:
            # short answer
            return (el,self.__parse_msg(content, cl))

        if Utils.debug:
            print "data packet"
        return (el,content)

    def __parse_msg(self, msg, sz):
        if sz == 2 and msg == "OK":
            if Utils.debug:
                print "received \"OK\""
        elif sz == 0:
            print "received \"Unsupported\""
        elif sz == 3 and msg[0] == 'E':
            self.__parse_error(msg[1:])
        return None

    def __parse_error(self, data):
        try:
            err = int(data,16)
            print "received error:", str(err)
        except ValueError:
            print "bad error value received:", repr(data)

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
        self.recv_pkt(2)

    def read_mem(self, addr, n):
        self.send_pkt("m"+addr+","+str(n))
        return self.recv_pkt(n*2)

    def write_mem(self, addr, val, n):
        self.send_pkt("M"+addr+","+str(n)+":"+val)
        self.recv_pkt(2)

    def set_mem_break(self, addr):
        self.send_pkt("Z0,"+addr+",1")
        self.recv_pkt(2)

    def del_mem_break(self, addr):
        self.send_pkt("z0,"+addr+",1")
        self.recv_pkt(2)

    def set_hrd_break(self, addr, knd, sz):
        self.send_pkt("Z"+knd+","+addr+","+sz)
        self.recv_pkt(2)

    def del_hrd_break(self, addr, knd, sz):
        self.send_pkt("z"+knd+","+addr+","+sz)
        self.recv_pkt(2)

    def resume(self, addr=None):
        if Utils.debug:
            print "send c"
        self._send("$c#63")
        self.recv_pkt()

    def singlestep(self, addr=None):
        if Utils.debug:
            print "send s"
        self._send("$s#73")
        self.recv_pkt()

    def stop_reason(self):
        if Utils.debug:
            print "send ?"
        self._send("$?#3f")
        self.recv_pkt()

    def _send_vmm_pkt(self, data):
        self.send_pkt("\x05"+data)

    def read_all_sr(self, sz):
        self._send_vmm_pkt("\x80")
        return self.recv_pkt(sz)

    def write_all_sr(self, data):
        #XXX
        print "NOT IMPLEMENTED"
        # self._send_vmm_pkt("\x81"+data)
        # self.recv_pkt(2)

    def read_sr(self, n, sz):
        self._send_vmm_pkt("\x82%.2x" % (n))
        return self.recv_pkt(sz)

    def write_sr(self, n, data):
        self._send_vmm_pkt("\x83%.2x=%s" % (n,data))
        self.recv_pkt(2)

    def set_lbr(self):
        self._send_vmm_pkt("\x84")
        return self.recv_pkt(2)

    def del_lbr(self):
        self._send_vmm_pkt("\x85")
        return self.recv_pkt(2)

    def get_lbr(self):
        self._send_vmm_pkt("\x86")
        return self.recv_pkt(4*8*2)

    def read_exception_mask(self):
        self._send_vmm_pkt("\x87")
        return self.recv_pkt(1*4*2)

    def write_exception_mask(self, data):
        self._send_vmm_pkt("\x88"+data)
        self.recv_pkt(2)

    def set_active_cr3(self, data):
        self._send_vmm_pkt("\x89"+data)
        self.recv_pkt(2)

    def del_active_cr3(self):
        self._send_vmm_pkt("\x8a")
        self.recv_pkt(2)

    def read_pmem(self, data, sz):
        self._send_vmm_pkt("\x8b"+data)
        self.recv_pkt(2)
        return self.recv_raw(sz) #raw data

    def write_pmem(self, cmd, data, sz):
        self._send_vmm_pkt("\x8c"+cmd)
        self.recv_pkt(2)
        self.__send(data, sz)

    def read_vmem(self, data, sz):
        self._send_vmm_pkt("\x8d"+data)
        self.recv_pkt(2)
        return self.recv_raw(sz) #raw data

    def write_vmem(self, cmd, data, sz):
        self._send_vmm_pkt("\x8e"+cmd)
        self.recv_pkt(2)
        self.__send(data, sz)

    def translate(self, data, sz):
        self._send_vmm_pkt("\x8f"+data)
        return self.recv_pkt(sz)

    def read_cr_read_mask(self):
        self._send_vmm_pkt("\x90")
        return self.recv_pkt(1*2*2)

    def write_cr_read_mask(self, data):
        self._send_vmm_pkt("\x91"+data)
        self.recv_pkt(2)

    def read_cr_write_mask(self):
        self._send_vmm_pkt("\x92")
        return self.recv_pkt(1*2*2)

    def write_cr_write_mask(self, data):
        self._send_vmm_pkt("\x93"+data)
        self.recv_pkt(2)

    def keep_active_cr3(self):
        self._send_vmm_pkt("\x94")
        self.recv_pkt(2)

    def set_affinity(self, data):
        self._send_vmm_pkt("\x95"+data)
        self.recv_pkt(2)
