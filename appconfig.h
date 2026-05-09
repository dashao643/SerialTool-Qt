#ifndef APPCONFIG_H
#define APPCONFIG_H

#include "dataStructure.h"
#include "tabpage.h"

#include <QObject>
#include <QSettings>
#include <QTabWidget>

const QString CONFIG_FILE_NAME = "setting.ini";

class AppConfig : public QObject
{
    Q_OBJECT
public:
    explicit AppConfig(QObject *parent = nullptr);
    Config_t loadConfig();
    void saveConfig(const Config_t &config);
    QList<TabPageConfig> loadTabPage();
    void saveTabPage(const QTabWidget *tabWiget);

private:
    QSettings setting_;

    // QList<CustomItem*> loadListWidget(TabPage *tabPage, int index);
    void saveListWidget(TabPage *tabPage, int index);

signals:
};

#endif // APPCONFIG_H
