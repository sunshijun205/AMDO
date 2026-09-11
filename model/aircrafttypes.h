#ifndef AIRCRAFTTYPES_H
#define AIRCRAFTTYPES_H

#include <QString>
#include <QVector>
#include <QtGlobal>

// 飞机方案定义（简化 CPACS 子集）领域 POD。无业务判断。

// 飞机语义数据模型：标识、单位与坐标语义。
struct AcSemantics {
    QString schema;             // 数据模型 schema，如 AircraftDM 2.3
    QString namespaceStr;       // 命名空间
    QString referenceStrategy;  // 引用策略（永久 ID 等）
    QString exchangeFormat;     // 交换格式（CPACS 子集 / JSON）
    QString lengthUnit;         // 长度单位
    QString massUnit;           // 质量单位
    QString coordinateSystem;   // 坐标系语义
    QString origin;             // 原点定义
};

// 总体方案配置：构型与总体布局。
struct AcConfiguration {
    QString category;           // 飞机类别
    QString wingLayout;         // 机翼布局
    QString tailConfig;         // 尾翼构型
    QString engineArrangement;  // 发动机布置
    QString gearType;           // 起落架形式
    QString cabinLayout;        // 客舱布局
    QString primaryMaterial;    // 主结构材料
    QString connectionStrategy; // 连接策略
    QString symmetry;           // 对称关系
};

// 系统/设备与安装定位。
struct AcSystemItem {
    QString id;        // 稳定标识，如 SYS-001
    QString object;    // 对象名（机翼/动力装置…）
    QString scheme;    // 方案
    QString mounting;  // 安装/定位
    QString material;  // 材料或属性
    QString status;    // 状态（有效/待确认）
};

// 参数化几何：尺寸与外形参数体系（不含几何内核，仅参数与驱动方式）。
struct AcParameter {
    QString id;         // 稳定标识，如 PARAM-001
    QString name;       // 参数名
    QString symbol;     // 符号
    double value = 0;
    bool valueKnown = false;
    QString unit;       // 单位
    QString driveType;  // 驱动方式（基准变量/设计变量/公式驱动/自动计算）
    QString note;       // 备注或公式
};

struct AircraftDocument {
    QString schemaVersion;   // amdo.aircraft.v1
    QString id;              // 草稿为 aircraft-draft；基线为 aircraft_vN
    QString title;           // 方案名称
    QString status;          // draft / published
    int version = 0;
    QString publishedAt;
    AcSemantics semantics;
    AcConfiguration configuration;
    QVector<AcSystemItem> systems;
    QVector<AcParameter> parameters;
    bool loadedKnown = false;
};

struct AircraftBaselineInfo {
    QString id;
    QString title;
    int version = 0;
    QString publishedAt;
    QString filePath;
};

struct AircraftKpis {
    int objectCount = 0;      // 对象实例数（系统 + 根对象）
    int typeCount = 0;        // 不同对象类型数
    int systemCount = 0;      // 系统/设备条目
    int parameterCount = 0;   // 几何参数条目
    int designVarCount = 0;   // 设计变量数
    int drivenCount = 0;      // 由公式/自动驱动的参数数
    int completeness = 0;     // 完整度百分比
    QString categoryText;     // 构型类别
};

struct AircraftCheckItem {
    QString message;
    bool blocking = false;
};

inline QString acFormatNumber(double value)
{
    const qint64 rounded = qRound64(value);
    if (qAbs(value - static_cast<double>(rounded)) < 1e-9)
        return QString::number(rounded);
    return QString::number(value, 'g', 6);
}

#endif
