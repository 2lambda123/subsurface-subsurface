// SPDX-License-Identifier: GPL-2.0
#ifndef QPREFSFACEBOOK_H
#define QPREFSFACEBOOK_H
#include "core/pref.h"

#include <QObject>


class qPrefFacebook : public QObject {
	Q_OBJECT
	Q_PROPERTY(QString access_token READ access_token WRITE set_access_token NOTIFY access_tokenChanged);
	Q_PROPERTY(QString album_id READ album_id WRITE set_album_id NOTIFY album_idChanged);
	Q_PROPERTY(QString user_id READ user_id WRITE set_user_id NOTIFY user_idChanged);

public:
	qPrefFacebook(QObject *parent = NULL);
	static qPrefFacebook *instance();

	// Load/Sync local settings (disk) and struct preference
	static void loadSync(bool doSync);
	static void load() {loadSync(false); }
	static void sync() {loadSync(true); }

public:
	static QString access_token() { return prefs.facebook.access_token; }
	static QString album_id() { return prefs.facebook.album_id; }
	static QString user_id() { return prefs.facebook.user_id; }

public slots:
	static void set_access_token(const QString& value);
	static void set_album_id(const QString& value);
	static void set_user_id(const QString& value);

signals:
	void access_tokenChanged(const QString& value);
	void album_idChanged(const QString& value);
	void user_idChanged(const QString& value);

private:
	static void disk_access_token(bool doSync);
	static void disk_album_id(bool doSync);
	static void disk_user_id(bool doSync);
};

#endif
