#!/usr/bin/python
#
# Copyright (c) 2008:
#   Mike Rooney (launchpad.net/~michael) <mrooney@gmail.com>
#
# Simply a few things that get imported by multiple files.
#
# This library is free software; you can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public
# License as published by the Free Software Foundation; either
# version 2 of the License, or (at your option) any later version.
#
# This library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public
# License along with this library; if not, write to the
# Free Software Foundation, Inc., 59 Temple Place - Suite 330,
# Boston, MA 02111-1307, USA.

def initGetText(APP):
    """
    Initialize localization, and put the gettext
    wrapper '_' in the global scope.
    """
    import os, locale, gettext
    DIR = os.path.dirname (__file__) + '/locale'
    gettext.bindtextdomain(APP, DIR)
    gettext.textdomain(APP)
    __builtins__['_'] = gettext.gettext

def debug(msg):
    """
    A small wrapper around debug/error printing,
    to make it easy to identify which applet the
    message came from.
    """
    print "Weather Applet: %s"%msg
