#include "pluginmanager.h"

#include <QApplication>
#include <QDir>
#include <QPluginLoader>

static QList<ISocialNetworkIntegration*> _socialNetworks;

PluginManager& PluginManager::instance() {
	static PluginManager self;
	return self;
}

void PluginManager::loadPlugins() {
	QDir pluginsDir(qApp->applicationDirPath());

#if defined(Q_OS_WIN)
	if (pluginsDir.dirName().toLower() == "debug" || pluginsDir.dirName().toLower() == "release")
		pluginsDir.cdUp();
#elif defined(Q_OS_MAC)
	if (pluginsDir.dirName() == "MacOS") {
		pluginsDir.cdUp();
		pluginsDir.cdUp();
		pluginsDir.cdUp();
	}
#endif
	pluginsDir.cd("plugins");

	foreach (const QString& fileName, pluginsDir.entryList(QDir::Files)) {
		QPluginLoader loader(pluginsDir.absoluteFilePath(fileName));
		QObject *plugin = loader.instance();
		if(!plugin) {
			continue;
		}

		if (ISocialNetworkIntegration *social = qobject_cast<ISocialNetworkIntegration*>(plugin)){
			_socialNetworks.push_back(social);
		}
    }
}

QList<ISocialNetworkIntegration*> PluginManager::socialNetworkIntegrationPlugins() const {
	return _socialNetworks;
}