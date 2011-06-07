/*
** Copyright (C) 2011 EADS France, stephane duverger <stephane.duverger@eads.net>
**
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 2 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License along
** with this program; if not, write to the Free Software Foundation, Inc.,
** 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>
#include <stdio.h>

int main(int argc, char **argv)
{
   int fd;
   struct termios t;

   if(argc != 2)
   {
      printf("%s <dev>\n", argv[0]);
      return -1;
   }

   if((fd=open(argv[1],O_RDWR)) == -1)
   {
      perror("open");
      return -1;
   }

   tcgetattr(fd, &t);
   cfmakeraw(&t);
   cfsetspeed(&t, B115200);
   tcsetattr(fd, TCSANOW, &t);
   close(fd);
   return 0;
}
