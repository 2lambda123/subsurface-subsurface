// SPDX-License-Identifier: GPL-2.0
#include "uploadDiveLogsDE.h"
#include <QDir>
#include <QDebug>
#include <zip.h>
#include <errno.h>
#include "core/display.h"
#include "core/errorhelper.h"
#include "core/qthelper.h"
#include "core/dive.h"
#include "core/membuffer.h"
#include "core/divesite.h"
#include "core/cloudstorage.h"
#include "core/settings/qPrefCloudStorage.h"


uploadDiveLogsDE *uploadDiveLogsDE::instance()
{
	static uploadDiveLogsDE *self = new uploadDiveLogsDE;

	return self;
}


uploadDiveLogsDE::uploadDiveLogsDE():
	reply(NULL),
	multipart(NULL)
{
	timeout.setSingleShot(true);
}


void uploadDiveLogsDE::doUpload(bool selected, const QString &userid, const QString &password)
{
	QString err;

	/* generate a temporary filename and create/open that file with zip_open */
	QString filename(QDir::tempPath() + "/divelogsde-upload.dld");

	// delete file if it exist
	QFile f(filename);
	if (f.open(QIODevice::ReadOnly)) {
		f.close();
		f.remove();
	}

	// Make zip file, with all dives, in divelogs.de format
	if (!prepareDives(selected, filename)) {
		report_error(tr("Failed to create upload file %s\n").toUtf8(), qPrintable(filename));
		emit uploadFinish(false, err);
		timeout.stop();
		return;
	}

	// And upload it
	uploadDives(filename, userid, password);
	timeout.stop();
}


bool uploadDiveLogsDE::prepareDives(bool selected, const QString &filename)
{
	static const char errPrefix[] = "divelog.de-uploadDiveLogsDE:";
	xsltStylesheetPtr xslt = NULL;
	struct zip *zip;

	xslt = get_stylesheet("divelogs-export.xslt");
	if (!xslt) {
		qDebug() << errPrefix << "missing stylesheet";
		report_error(tr("Stylesheet to export to divelogs.de is not found").toUtf8());
		return false;
	}

	// Prepare zip file
	int error_code;
	zip = zip_open(QFile::encodeName(QDir::toNativeSeparators(filename)), ZIP_CREATE, &error_code);
	if (!zip) {
		char buffer[1024];
		zip_error_to_str(buffer, sizeof buffer, error_code, errno);
		report_error(tr("Failed to create zip file for uploadDiveLogsDE: %s").toUtf8(), buffer);
		return false;
	}
	
	/* walk the dive list in chronological order */
	int i;
	struct dive *dive;
	for_each_dive (i, dive) {
		char xmlfilename[PATH_MAX];
		int streamsize;
		const char *membuf;
		xmlDoc *transformed;
		struct zip_source *s;
		struct membuffer mb = {};
		
		/*
		 * Get the i'th dive in XML format so we can process it.
		 * We need to save to a file before we can reload it back into memory...
		 */
		if (selected && !dive->selected)
			continue;

		/* make sure the buffer is empty and add the dive */
		struct dive_site *ds = dive->dive_site;
		mb.len = 0;
		if (ds) {
			put_format(&mb, "<divelog><divesites><site uuid='%8x' name='", ds->uuid);
			put_quoted(&mb, ds->name, 1, 0);
			put_format(&mb, "'");
			put_location(&mb, &ds->location, " gps='", "'");
			put_format(&mb, ">\n");
			if (ds->taxonomy.nr) {
				for (int j = 0; j < ds->taxonomy.nr; j++) {
					struct taxonomy *t = &ds->taxonomy.category[j];
					if (t->category != TC_NONE && t->category == prefs.geocoding.category[j] && t->value) {
						put_format(&mb, "  <geo cat='%d'", t->category);
						put_format(&mb, " origin='%d' value='", t->origin);
						put_quoted(&mb, t->value, 1, 0);
						put_format(&mb, "'/>\n");
					}
				}
			}
			put_format(&mb, "</site>\n</divesites>\n");
		}
		save_one_dive_to_mb(&mb, dive, false);

		if (ds) {
			put_format(&mb, "</divelog>\n");
		}
		membuf = mb_cstring(&mb);
		streamsize = strlen(membuf);
		/*
		 * Parse the memory buffer into XML document and
		 * transform it to divelogs.de format, finally dumping
		 * the XML into a character buffer.
		 */
		xmlDoc *doc = xmlReadMemory(membuf, streamsize, "divelog", NULL, 0);
		if (!doc) {
			qWarning() << errPrefix << "could not parse back into memory the XML file we've just created!";
			report_error(tr("internal error").toUtf8());
			zip_close(zip);
			QFile::remove(filename);
			xsltFreeStylesheet(xslt);
			return false;
		}
		free_buffer(&mb);
		
		transformed = xsltApplyStylesheet(xslt, doc, NULL);
		if (!transformed) {
			qWarning() << errPrefix << "XSLT transform failed for dive: " << i;
			report_error(tr("Conversion of dive %1 to divelogs.de format failed").arg(i).toUtf8());
			continue;
		}
		xmlDocDumpMemory(transformed, (xmlChar **)&membuf, &streamsize);
		xmlFreeDoc(doc);
		xmlFreeDoc(transformed);
		
		/*
		 * Save the XML document into a zip file.
		 */
		snprintf(xmlfilename, PATH_MAX, "%d.xml", i + 1);
		s = zip_source_buffer(zip, membuf, streamsize, 1);
		if (s) {
			int64_t ret = zip_add(zip, xmlfilename, s);
			if (ret == -1) {
				qDebug() << errPrefix << "failed to include dive:" << i;
			}
		}
	}
	xsltFreeStylesheet(xslt);
	if (zip_close(zip)) {
		int ze, se;
#if LIBZIP_VERSION_MAJOR >= 1
		zip_error_t *error = zip_get_error(zip);
		ze = zip_error_code_zip(error);
		se = zip_error_code_system(error);
#else
		zip_error_get(zip, &ze, &se);
#endif
		report_error(qPrintable(tr("error writing zip file: %s zip error %d system error %d - %s")),
					 qPrintable(QDir::toNativeSeparators(filename)), ze, se, zip_strerror(zip));
		return false;
	}
	return true;
}


void uploadDiveLogsDE::uploadDives(const QString &filename, const QString &userid, const QString &password)
{
	QHttpPart part1, part2, part3;
	static QNetworkRequest request;
	QString args;

	// Check if there is an earlier request open
	if (reply != NULL && reply->isOpen()) {
		reply->abort();
		delete reply;
		reply = NULL;
	}
	if (multipart != NULL) {
		delete multipart;
		multipart = NULL;
	}
	multipart = new QHttpMultiPart(QHttpMultiPart::FormDataType);

	// prepare header with filename (of all dives) and pointer to file
	args = "form-data; name=\"userfile\"; filename=\"" + filename + "\"";
	part1.setRawHeader("Content-Disposition", args.toLatin1());
	QFile *f = new QFile(filename);
	if (!f->open(QIODevice::ReadOnly)) {
		qDebug() << "ERROR opening zip file: " << filename;
		return;
	}
	part1.setBodyDevice(f);
	multipart->append(part1);

	// Add userid
	args = "form-data; name=\"user\"";
	part2.setRawHeader("Content-Disposition", args.toLatin1());
	part2.setBody(qPrefCloudStorage::divelogde_user().toUtf8());
	multipart->append(part2);

	// Add password
	args = "form-data; name=\"pass\"";
	part3.setRawHeader("Content-Disposition", args.toLatin1());
	part3.setBody(qPrefCloudStorage::divelogde_pass().toUtf8());
	multipart->append(part3);

	// Prepare network request
	request.setUrl(QUrl("https://divelogs.de/DivelogsDirectImport.php"));
	request.setRawHeader("Accept", "text/xml, application/xml");
	request.setRawHeader("User-Agent", getUserAgent().toUtf8());

	// Execute async.
	reply = manager()->post(request, multipart);

	// connect signals from upload process
	connect(reply, SIGNAL(finished()), this, SLOT(uploadFinished()));
	connect(reply, SIGNAL(error(QNetworkReply::NetworkError)), this,
		SLOT(uploadError(QNetworkReply::NetworkError)));
	connect(reply, SIGNAL(uploadProgress(qint64, qint64)), this,
		SLOT(updateProgress(qint64, qint64)));
	connect(&timeout, SIGNAL(timeout()), this, SLOT(uploadTimeout()));

	timeout.start(30000); // 30s
}


void uploadDiveLogsDE::updateProgress(qint64 current, qint64 total)
{
	if (!reply)
		return;
	if (total <= 0 || current <= 0)
		return;

	// Calculate percentage
	// And signal whoever wants to know
	qreal percentage = (float)current / (float)total;
	emit uploadProgress(percentage);

	// reset the timer: 30 seconds after we last got any data
	timeout.start();
}


void uploadDiveLogsDE::uploadFinished()
{
	QString err;

	if (!reply)
		return;

	// check what the server sent us: it might contain
	// an error condition, such as a failed login
	QByteArray xmlData = reply->readAll();
	reply->deleteLater();
	reply = NULL;
	char *resp = xmlData.data();
	if (resp) {
		char *parsed = strstr(resp, "<Login>");
		if (parsed) {
			if (strstr(resp, "<Login>succeeded</Login>")) {
				if (strstr(resp, "<FileCopy>failed</FileCopy>")) {
					report_error(tr("Upload failed").toUtf8());
					return;
				}
				err = tr("Upload successful");
				emit uploadFinish(true, err);
				timeout.stop();
				return;
			}
			err = tr("Login failed");
			report_error(err.toUtf8());
			emit uploadFinish(false, err);
			timeout.stop();
			return;
		}
		err = tr("Cannot parse response");
		report_error(tr("Cannot parse response").toUtf8());
		emit uploadFinish(false, err);
		timeout.stop();
	}
}


void uploadDiveLogsDE::uploadTimeout()
{
	QString err(tr("divelogs.de not responding"));
	report_error(err.toUtf8());
	emit uploadFinish(false, err);
	timeout.stop();
}


void uploadDiveLogsDE::uploadError(QNetworkReply::NetworkError error)
{
	QString err(tr("network error %1").arg(error));
	report_error(err.toUtf8());
	emit uploadFinish(false, err);
	timeout.stop();
}
