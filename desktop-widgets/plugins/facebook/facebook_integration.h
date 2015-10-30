#ifndef FACEBOOK_INTEGRATION_H
#define FACEBOOK_INTEGRATION_H

#include "subsurface-core/isocialnetworkintegration.h"
#include <QString>

class FacebookPlugin : public QObject, public ISocialNetworkIntegration {
  Q_OBJECT
  Q_PLUGIN_METADATA(IID "org.subsurface.plugins.ISocialNetworkIntegration")
  Q_INTERFACES(ISocialNetworkIntegration)
public:
  explicit FacebookPlugin(QObject* parent = 0);
  virtual bool isConnected();
  virtual void requestLogin();
  virtual void requestLogoff();
  virtual QString socialNetworkIcon() const;
  virtual QString socialNetworkName() const;
  virtual void uploadCurrentDive();
};

#endif