# Copyright (c) 2008 Mark Lee <avant-wn@lazymalevolence.com>
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

import gettext

PREFIX = '/usr/local'
__version__ = '0.4.0'
__version_info__ = ()
for value in __version__.split('.'):
    if value.isdigit():
        value = int(value)
    __version_info__ += (value,)
del value
APPLET_BASEDIR = '/usr/local/share/avant-window-navigator/applets'

# gettext (internationalization/i18n) support
LOCALEDIR = '/usr/local/share/locale'
TEXTDOMAIN = 'awn-extras'
gettext.bindtextdomain(TEXTDOMAIN, LOCALEDIR)
gettext.textdomain(TEXTDOMAIN)
_ = gettext.gettext
