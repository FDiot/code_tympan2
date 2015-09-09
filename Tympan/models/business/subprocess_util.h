/*
 * Copyright (C) <2014> <EDF-R&D> <FRANCE>
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/

/**
 * \file Tympan/models/business/subprocess_util.h
 * \brief Utilitaires pour les interactions entre l'application tympan et des sous-
 * processus python
 */


#ifndef TY_M_B_SUBPROCESS_UTIL
#define TY_M_B_SUBPROCESS_UTIL

#include <qstring.h>
#include <QTemporaryFile>
#include "Tympan/gui/dllexport.h"

bool TYMPAN_EXPORT python(QStringList args, std::string& error_msg);
bool TYMPAN_EXPORT must_keep_tmp_files();
bool TYMPAN_EXPORT init_tmp_file(QTemporaryFile& tmp_file, bool keep_file=false);

#endif // TY_M_B_SUBPROCESS_UTIL
