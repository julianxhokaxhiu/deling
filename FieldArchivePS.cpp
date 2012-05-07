/****************************************************************************
 ** Deling Final Fantasy VIII Field Editor
 ** Copyright (C) 2009-2012 Arzel J�r�me <myst6re@gmail.com>
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
#include "FieldArchivePS.h"

FieldArchivePS::FieldArchivePS()
	: FieldArchive(), iso(NULL)
{
}

FieldArchivePS::~FieldArchivePS()
{
	if(iso != NULL)					delete iso;
}

QString FieldArchivePS::archivePath() const
{
	return iso->fileName();
}

FF8DiscArchive *FieldArchivePS::getFF8DiscArchive() const
{
	return iso;
}

int FieldArchivePS::open(const QString &path, QProgressDialog *progress)
{
	Field *field;
	QString desc;
	int i, currentMap=0, fieldID=0;

	readOnly = true;

	iso = new FF8DiscArchive(path);
	if(!iso->open(QIODevice::ReadOnly))
		return 1;

	if(!iso->findIMG()) {
		qWarning() << "IMG not found";
		return 4;
	}

	const QList<FF8DiscFile> &fieldFiles = iso->fieldDirectory();
	int tocSize = fieldFiles.size();

	if(tocSize == 0)
		return 4;

	clearFields();
	setMapList(QStringList());
	int indexOf;

	quint32 freq = ((tocSize - 77) / 3)/100;
	progress->setRange(0, (tocSize - 77) / 3);

	for(i=77 ; i<tocSize ; i += 3) {
		QCoreApplication::processEvents();
		if(progress->wasCanceled()) {
			clearFields();
			return 2;
		}
		if(currentMap%freq == 0) {
			progress->setValue(currentMap);
		}

		QByteArray fieldData = iso->fileLZS(fieldFiles.at(i));

		if(!fieldData.isEmpty()) {
			field = new FieldPS(fieldData, i);
			if(field->isOpen()) {
				if(field->hasJsmFile())
					desc = Data::locations().value(field->getJsmFile()->mapID());
				else
					desc = QString();

				indexOf = mapList().indexOf(field->name());
				QString mapId = indexOf==-1 ? "~" : QString("%1").arg(indexOf, 3, 10, QChar('0'));

				fields.append(field);
				fieldsSortByName.insert(field->name(), fieldID);
				fieldsSortByDesc.insert(desc, fieldID);
				fieldsSortByMapId.insert(mapId, fieldID);
				++fieldID;
			} else {
				delete field;
			}
		}
		++currentMap;
	}

	if(fields.isEmpty()) {
		return 4;
	}

	Config::setValue("jp", iso->isJp());

	return 0;
}

bool FieldArchivePS::openBG(Field *field) const
{
	FieldPS *fieldPS = (FieldPS *)field;

	quint32 isoFieldID = fieldPS->isoFieldID();

	if(isoFieldID < 1 || (int)isoFieldID >= iso->fieldCount())
		return false;

	QByteArray mim = iso->fileLZS(iso->fieldFile(isoFieldID-1));
	if(mim.isEmpty())
		return false;

	QByteArray dat = iso->fileLZS(iso->fieldFile(isoFieldID));
	if(dat.isEmpty())
		return false;

	return fieldPS->open2(dat, mim);
}
