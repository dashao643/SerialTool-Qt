#include "appconfig.h"
#include "customitem.h"

#include <QSerialPortInfo>
#include <QDir>

AppConfig::AppConfig(QObject *parent)
    : QObject{parent}
    , setting_(CONFIG_FILE_NAME,QSettings::IniFormat)
{

}

Config_t AppConfig::loadConfig()
{
    Config_t config;
    /// 窗口大小
    // config.windowSize.setWidth(setting_.value("MainWindow/Height", DEFAULT_WINDOW_WIDTH).toInt());
    // config.windowSize.setHeight(setting_.value("MainWindow/Width", DEFAULT_WINDOW_HEIGHT).toInt());
    /// 波特率,默认选择 115200
    int baudRateIndex = qMax(0, QSerialPortInfo::standardBaudRates().size() - 3);
    config.baudRateIndex = setting_.value("SerialConfig/BaudIndex", baudRateIndex).toInt();
    /// 校验
    config.check = (CheckDataIndex)setting_.value(
        "Send/CheckMode",
        (int)CheckDataIndex::NONE_CHECK).toInt();
    /// 文件保存路径
    config.filePath = setting_.value("ToolBar/saveFilePath",QDir::currentPath()).toString();
    /// 字体设置
    config.font = setting_.value("ShowWindow/font", QFont("Microsoft YaHei UI", 12)).value<QFont>();
    /// 窗口可见
    config.isDockVisible = setting_.value("ToolBar/dockVisible", true).toBool();
    // 发送文件配置
    config.sendFile.model = 0;
    config.sendFile.filePath = setting_.value("ToolBar/send/filePath",QDir::currentPath()).toString();
    config.sendFile.dataSize = setting_.value("ToolBar/send/dataSize", 1024).toInt();
    config.sendFile.cmd = setting_.value("ToolBar/send/cmd", "7F").toString();
    config.sendFile.ack = setting_.value("ToolBar/send/ack", "79").toString();
    config.sendFile.timeoutMs = setting_.value("ToolBar/send/timeoutMs", 200).toInt();

    return config;
}

void AppConfig::saveConfig(const Config_t &config)
{
    setting_.setValue("MainWindow/Height", config.windowSize);
    setting_.setValue("SerialConfig/BaudIndex", config.baudRateIndex);
    setting_.setValue("Send/CheckMode",config.check);
    setting_.setValue("ToolBar/filePath",config.filePath);
    setting_.setValue("ShowWindow/font", config.font);
    setting_.setValue("ToolBar/dockVisible", config.isDockVisible);

    setting_.setValue("ToolBar/send/filePath", config.sendFile.filePath);
    setting_.setValue("ToolBar/send/dataSize", config.sendFile.dataSize);
    setting_.setValue("ToolBar/send/cmd",config.sendFile.cmd);
    setting_.setValue("ToolBar/send/ack",config.sendFile.ack);
    setting_.setValue("ToolBar/send/timeoutMs", config.sendFile.timeoutMs);
}

QList<TabPageConfig> AppConfig::loadTabPage()
{
    QList<TabPageConfig> result;
    int pageCnt = setting_.value("tab_page/cnt", 1).toInt();

    for (int i = 0; i < pageCnt; i++) {
        TabPageConfig page;
        page.name = setting_.value(QString("tab_page/name/page%1").arg(i), QString("tab%1").arg(i+1)).toString();

        int itemCnt = setting_.value(QString("tab_page/page%1/item/cnt").arg(i), 5).toInt();
        for (int j = 0; j < itemCnt; j++) {
            ItemConfig item;
            QString key = QString("tab_page/page%1/item%2/").arg(i).arg(j);
            item.remark = setting_.value(key+"remark", "未命名").toString();
            item.content = setting_.value(key+"content").toString();
            item.model = (SendModel)setting_.value(key+"model", SendModel::ASCII).toInt();
            page.items.append(item);
        }
        result.append(page);
    }
    return result;
}

// QList<CustomItem*> AppConfig::loadListWidget(TabPage *tabPage, int index)
// {
//     QList<CustomItem*> itemList; // 保存创建的自定义项

//     int itemCnt = setting_.value(QString("tab_page/page%1/item/cnt").arg(index), 5).toInt();
//     QListWidget* list = tabPage->getListWidget();

//     for (int i = 0; i < itemCnt; i++) {
//         QListWidgetItem *item = new QListWidgetItem;
//         item->setSizeHint(QSize(0, CUSTOM_ITEM_HEIGHT));
//         CustomItem *addItem = new CustomItem();
//         list->addItem(item);
//         list->setItemWidget(item, addItem);

//         QString keyPrefix = QString("tab_page/page%1/item%2/").arg(index).arg(i);

//         QString remark = setting_.value(keyPrefix + "remark","未命名").toString();
//         QString content = setting_.value(keyPrefix + "content").toString();
//         int model = setting_.value(keyPrefix + "model", SendModel::ASCII).toInt();

//         addItem->setRemark(remark);
//         addItem->setContent(content);
//         addItem->setModel((SendModel)model);
//     }
// }

void AppConfig::saveTabPage(const QTabWidget *tabWiget)
{
    int pageCnt = tabWiget->count();
    setting_.setValue("tab_page/cnt", pageCnt);
    for(int i = 0; i < pageCnt; i++){
        QString name = tabWiget->tabText(i);
        setting_.setValue(QString("tab_page/name/page%1").arg(i), name);

        TabPage* tabPage = qobject_cast<TabPage*>(tabWiget->widget(i));
        saveListWidget(tabPage, i);
    }
}

void AppConfig::saveListWidget(TabPage *tabPage, int index)
{
    QListWidget *list = tabPage->getListWidget();
    int itemCnt = list->count();
    setting_.setValue(QString("tab_page/page%1/item/cnt").arg(index), itemCnt);

    for(int i = 0; i < itemCnt; i++){
        QListWidgetItem *item = list->item(i);
        // 强制转换成自定义项
        CustomItem *customItem = qobject_cast<CustomItem*>(list->itemWidget(item));
        if(!customItem) continue;

        QString keyPrefix = QString("tab_page/page%1/item%2/").arg(index).arg(i);

        setting_.setValue(keyPrefix + "remark", customItem->getRemark());
        setting_.setValue(keyPrefix + "content", customItem->getContent());
        setting_.setValue(keyPrefix + "model", (int)customItem->getModel());
    }
}
