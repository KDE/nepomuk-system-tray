/*
   Copyright (C) 2010 by Serebriyskiy Artem <v.for.vandal at gmail.com>

   This library is free software; you can redistribute it and/or modify
   it under the terms of the GNU Library General Public License as published by
   the Free Software Foundation; either version 3, or (at your option)
   any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */


#ifndef NEPOMUK_SYSTRAY_EXPORT_H
#define NEPOMUK_SYSTRAY_EXPORT_H

/* needed for KDE_EXPORT and KDE_IMPORT macros */
#include <kdemacros.h>

#ifndef NEPOMUKSYSTRAY_EXPORT
# if defined(MAKE_NEPOMUKSYSTRAY_LIB)
   /* We are building this library */
#  define NEPOMUKSYSTRAY_EXPORT KDE_EXPORT
# else
   /* We are using this library */
#  define NEPOMUKSYSTRAY_EXPORT KDE_IMPORT
# endif
#endif


#endif

