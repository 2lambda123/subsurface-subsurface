// SPDX-License-Identifier: GPL-2.0
#ifndef CLEANERTABLEMODEL_H
#define CLEANERTABLEMODEL_H

#include <QAbstractTableModel>
#include <QStringList>
#include <QPixmap>

/* Retrieve the trash icon pixmap, common to most table models */
const QPixmap &trashIcon();
const QPixmap &trashForbiddenIcon();
const QPixmap &editIcon();

/* When using a QAbstractTableModel, consider using this instead
 * of the default implementation, as it's easyer to setup the columns
 * and headers.
 * Most subsurface classes uses this one to save loads of lines
 * of code and share a consistent layout. */

class CleanerTableModel : public QAbstractTableModel {
	Q_OBJECT
public:
	explicit CleanerTableModel(QStringList headers, QObject *parent = 0);
	QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

private:
	QStringList headers;
	int columnCount(const QModelIndex &parent = QModelIndex()) const override;
};

#endif
