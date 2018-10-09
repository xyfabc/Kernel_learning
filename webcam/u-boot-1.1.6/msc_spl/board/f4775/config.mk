#
# (C) Copyright 2006
# Stefan Roese, DENX Software Engineering, sr@denx.de.
#
# See file CREDITS for list of people who contributed to this
# project.
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License as
# published by the Free Software Foundation; either version 2 of
# the License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston,
# MA 02111-1307 USA
#
#
# Ingenic f4780 Platform
#

#
# TEXT_BASE for SPL:
#
# On JZ4780 platforms the SPL is located at 0xf4000800...0xf4000a00,
# in the first 14KBytes of memory space in tcsm. So we set
# TEXT_BASE to starting address in here.
#

#sd_boot
#TEXT_BASE = 0x80000200
TEXT_BASE = 0xf4000a00
