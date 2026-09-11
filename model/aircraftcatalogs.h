#ifndef AIRCRAFTCATALOGS_H
#define AIRCRAFTCATALOGS_H

#include "model/aircrafttypes.h"

#include <QStringList>

class AircraftCatalogs
{
public:
    static QStringList categories();
    static QStringList wingLayouts();
    static QStringList tailConfigs();
    static QStringList engineArrangements();
    static QStringList gearTypes();
    static QStringList cabinLayouts();
    static QStringList materials();
    static QStringList driveTypes();

    // 可选的新建模板名称（首个约定为“空白方案”）。
    static QStringList templateNames();

    // 空白方案：仅带默认单位与坐标语义，其余留空供手工填写。
    static AircraftDocument blankDocument();

    // 按模板名称生成方案（未知模板回退为窄体运输机样板）。
    static AircraftDocument templateDocument(const QString &name);

    // 默认种子草稿（首次运行时写入）。
    static AircraftDocument seedDocument();
};

#endif
