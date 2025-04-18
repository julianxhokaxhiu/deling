/****************************************************************************
 ** Copyright (C) 2009-2021 Arzel Jérôme <myst6re@gmail.com>
 **
 ** This program is free software: you can redistribute it and/or modify
 ** it under the terms of the GNU General Public License as published by
 ** the Free Software Foundation, either version 3 of the License, or
 ** (at your option) any later version.
 **
 ** This program is distributed in the hope that it will be useful,
 ** but WITHOUT ANY WARRANTY; without even the implied warranty of
 ** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 ** GNU General Public License for more details.
 **
 ** You should have received a copy of the GNU General Public License
 ** along with this program.  If not, see <http://www.gnu.org/licenses/>.
 ****************************************************************************/
#include "ArgumentsPack.h"

ArgumentsPack::ArgumentsPack() : ArgumentsPackUnpack()
{
	_ADD_FLAG(_OPTION_NAMES("f", "force"),
	          "Overwrite destination file if exists.");
	_ADD_ARGUMENT(_OPTION_NAMES("c", "compression"), "Compression format ([lzs], lz4, none).", "compression-format", "lzs");
	_ADD_ARGUMENT("prefix", "Custom directory prefix inside the target archive (default \"c:\\ff8\\data\\\")", "prefix", "c:\\ff8\\data\\");

	_parser.addPositionalArgument("directory", QCoreApplication::translate("ArgumentsPack", "Input directory."));
	_parser.addPositionalArgument("file", QCoreApplication::translate("ArgumentsPack", "Input file or directory."));

	parse();
}

bool ArgumentsPack::force() const
{
	return _parser.isSet("force");
}

QString ArgumentsPack::prefix() const
{
	QString pre = _parser.value("prefix");
	
	if (pre.isEmpty()) {
		return "c:\\ff8\\data\\";
	}
	
	return pre;
}

FiCompression ArgumentsPack::compressionFormat() const
{
	QString compression = _parser.value("compression");
	if (compression.isEmpty() || compression.toLower() == "lzs") {
		return FiCompression::CompressionLzs;
	}
	if (compression.toLower() == "lz4") {
		return FiCompression::CompressionLz4;
	}
	if (compression.toLower() == "none") {
		return FiCompression::CompressionNone;
	}
	
	qWarning() << qPrintable(
	    QCoreApplication::translate("Arguments", "Error: unknown compression, available values: lzs, lz4, none"));
	exit(1);
}

void ArgumentsPack::parse()
{
	_parser.process(*qApp);

	if (_parser.positionalArguments().size() > 3) {
		qWarning() << qPrintable(
		    QCoreApplication::translate("Arguments", "Error: too much parameters"));
		exit(1);
	}

	QStringList paths = wilcardParse();
	if (paths.size() == 2) {
		// Source directory
		if (QDir(paths.first()).exists()) {
			_directory = paths.takeFirst();
		} else {
			qWarning() << qPrintable(
			    QCoreApplication::translate("Arguments", "Error: source directory does not exist:"))
			           << qPrintable(paths.first());
			exit(1);
		}

		if (!paths.isEmpty()) {
			_path = paths.first();
		}
	}
	mapNamesFromFiles();
}
