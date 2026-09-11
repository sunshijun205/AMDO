#include "model/aircraftcatalogs.h"

QStringList AircraftCatalogs::categories()
{
    return {
        QString::fromUtf8("窄体运输机"),
        QString::fromUtf8("支线客机"),
        QString::fromUtf8("公务机"),
        QString::fromUtf8("宽体运输机"),
        QString::fromUtf8("通用航空")
    };
}

QStringList AircraftCatalogs::wingLayouts()
{
    return {
        QString::fromUtf8("下单翼"),
        QString::fromUtf8("上单翼"),
        QString::fromUtf8("中单翼")
    };
}

QStringList AircraftCatalogs::tailConfigs()
{
    return {
        QString::fromUtf8("常规尾翼"),
        QString::fromUtf8("T 型尾翼"),
        QString::fromUtf8("V 尾"),
        QString::fromUtf8("H 型尾翼")
    };
}

QStringList AircraftCatalogs::engineArrangements()
{
    return {
        QString::fromUtf8("翼下双发"),
        QString::fromUtf8("尾吊双发"),
        QString::fromUtf8("翼下四发"),
        QString::fromUtf8("单发")
    };
}

QStringList AircraftCatalogs::gearTypes()
{
    return {
        QString::fromUtf8("前三点式"),
        QString::fromUtf8("后三点式"),
        QString::fromUtf8("自行车式")
    };
}

QStringList AircraftCatalogs::cabinLayouts()
{
    return {
        QString::fromUtf8("3-3 单通道"),
        QString::fromUtf8("2-2 单通道"),
        QString::fromUtf8("2-4-2 双通道"),
        QString::fromUtf8("货运布局")
    };
}

QStringList AircraftCatalogs::materials()
{
    return {
        QString::fromUtf8("CFRP + Al-Li"),
        QString::fromUtf8("铝合金为主"),
        QString::fromUtf8("复合材料为主"),
        QString::fromUtf8("钛合金局部增强")
    };
}

QStringList AircraftCatalogs::driveTypes()
{
    return {
        QString::fromUtf8("基准变量"),
        QString::fromUtf8("设计变量"),
        QString::fromUtf8("公式驱动"),
        QString::fromUtf8("自动计算")
    };
}

QStringList AircraftCatalogs::templateNames()
{
    return {
        QString::fromUtf8("空白方案"),
        QString::fromUtf8("窄体运输机（常规布局）"),
        QString::fromUtf8("支线客机（T 尾）")
    };
}

static AcSemantics defaultSemantics()
{
    AcSemantics s;
    s.schema = QStringLiteral("AircraftDM 2.3");
    s.namespaceStr = QStringLiteral("hx01/core");
    s.referenceStrategy = QString::fromUtf8("永久 ID");
    s.exchangeFormat = QString::fromUtf8("CPACS 子集 (JSON)");
    s.lengthUnit = QStringLiteral("m");
    s.massUnit = QStringLiteral("kg");
    s.coordinateSystem = QString::fromUtf8("机体系 X前 Y右 Z下");
    s.origin = QString::fromUtf8("机鼻基准面");
    return s;
}

AircraftDocument AircraftCatalogs::blankDocument()
{
    AircraftDocument d;
    d.schemaVersion = QStringLiteral("amdo.aircraft.v1");
    d.id = QStringLiteral("aircraft-draft");
    d.title = QString::fromUtf8("未命名方案");
    d.status = QStringLiteral("draft");
    d.version = 0;
    d.semantics = defaultSemantics();
    // 构型与系统/参数留空，供手工录入。
    d.loadedKnown = true;
    return d;
}

static AcParameter makeParam(const QString &id, const QString &name, const QString &symbol,
                             double value, const QString &unit, const QString &driveType,
                             const QString &note = QString())
{
    AcParameter p;
    p.id = id;
    p.name = name;
    p.symbol = symbol;
    p.value = value;
    p.valueKnown = true;
    p.unit = unit;
    p.driveType = driveType;
    p.note = note;
    return p;
}

static AcSystemItem makeSystem(const QString &id, const QString &object, const QString &scheme,
                               const QString &mounting, const QString &material, const QString &status)
{
    AcSystemItem s;
    s.id = id;
    s.object = object;
    s.scheme = scheme;
    s.mounting = mounting;
    s.material = material;
    s.status = status;
    return s;
}

AircraftDocument AircraftCatalogs::seedDocument()
{
    AircraftDocument d;
    d.schemaVersion = QStringLiteral("amdo.aircraft.v1");
    d.id = QStringLiteral("aircraft-draft");
    d.title = QString::fromUtf8("HX-01 概念方案");
    d.status = QStringLiteral("draft");
    d.version = 0;
    d.semantics = defaultSemantics();

    AcConfiguration c;
    c.category = QString::fromUtf8("窄体运输机");
    c.wingLayout = QString::fromUtf8("下单翼");
    c.tailConfig = QString::fromUtf8("常规尾翼");
    c.engineArrangement = QString::fromUtf8("翼下双发");
    c.gearType = QString::fromUtf8("前三点式");
    c.cabinLayout = QString::fromUtf8("3-3 单通道");
    c.primaryMaterial = QString::fromUtf8("CFRP + Al-Li");
    c.connectionStrategy = QString::fromUtf8("参数化连接点");
    c.symmetry = QString::fromUtf8("XZ 面镜像");
    d.configuration = c;

    d.systems = {
        makeSystem(QStringLiteral("SYS-001"), QString::fromUtf8("机翼"), QString::fromUtf8("后掠下单翼"),
                   QString::fromUtf8("基准面 X=14.2 m"), QString::fromUtf8("CFRP 翼盒"), QString::fromUtf8("有效")),
        makeSystem(QStringLiteral("SYS-002"), QString::fromUtf8("动力装置"), QString::fromUtf8("翼下双发"),
                   QStringLiteral("Y=±6.4 m"), QString::fromUtf8("涡扇 + 短舱"), QString::fromUtf8("有效")),
        makeSystem(QStringLiteral("SYS-003"), QString::fromUtf8("起落架"), QString::fromUtf8("前三点式"),
                   QString::fromUtf8("机身/机翼连接"), QString::fromUtf8("钛合金接头"), QString::fromUtf8("有效")),
        makeSystem(QStringLiteral("SYS-004"), QString::fromUtf8("燃油系统"), QString::fromUtf8("中央翼盒 + 两侧机翼"),
                   QString::fromUtf8("4 个油箱"), QString::fromUtf8("整体油箱"), QString::fromUtf8("有效")),
        makeSystem(QStringLiteral("SYS-005"), QString::fromUtf8("飞控系统"), QString::fromUtf8("电传操纵"),
                   QString::fromUtf8("3 余度"), QString::fromUtf8("分布式安装"), QString::fromUtf8("待确认"))
    };

    d.parameters = {
        makeParam(QStringLiteral("PARAM-001"), QString::fromUtf8("机身总长"), QStringLiteral("L_fus"),
                  38.20, QStringLiteral("m"), QString::fromUtf8("基准变量")),
        makeParam(QStringLiteral("PARAM-002"), QString::fromUtf8("机翼面积"), QStringLiteral("S_ref"),
                  124.0, QString::fromUtf8("m^2"), QString::fromUtf8("设计变量")),
        makeParam(QStringLiteral("PARAM-003"), QString::fromUtf8("展弦比"), QStringLiteral("AR"),
                  9.4, QString(), QString::fromUtf8("设计变量")),
        makeParam(QStringLiteral("PARAM-004"), QString::fromUtf8("机翼展长"), QStringLiteral("b"),
                  34.15, QStringLiteral("m"), QString::fromUtf8("公式驱动"), QStringLiteral("b = sqrt(S_ref * AR)")),
        makeParam(QStringLiteral("PARAM-005"), QString::fromUtf8("平均气动弦"), QStringLiteral("MAC"),
                  4.16, QStringLiteral("m"), QString::fromUtf8("自动计算")),
        makeParam(QStringLiteral("PARAM-006"), QString::fromUtf8("机翼后掠角"), QStringLiteral("Sweep25"),
                  25.0, QStringLiteral("deg"), QString::fromUtf8("设计变量")),
        makeParam(QStringLiteral("PARAM-007"), QString::fromUtf8("水平尾翼面积"), QStringLiteral("S_ht"),
                  31.4, QString::fromUtf8("m^2"), QString::fromUtf8("公式驱动"), QStringLiteral("尾容量系数驱动"))
    };

    d.loadedKnown = true;
    return d;
}

AircraftDocument AircraftCatalogs::templateDocument(const QString &name)
{
    if (name == QString::fromUtf8("空白方案"))
        return blankDocument();

    if (name == QString::fromUtf8("支线客机（T 尾）")) {
        AircraftDocument d = seedDocument();
        d.title = QString::fromUtf8("支线客机方案");
        d.configuration.category = QString::fromUtf8("支线客机");
        d.configuration.tailConfig = QString::fromUtf8("T 型尾翼");
        d.configuration.engineArrangement = QString::fromUtf8("尾吊双发");
        d.configuration.cabinLayout = QString::fromUtf8("2-2 单通道");
        for (int i = 0; i < d.systems.size(); ++i) {
            if (d.systems[i].object == QString::fromUtf8("动力装置")) {
                d.systems[i].scheme = QString::fromUtf8("尾吊双发");
                d.systems[i].mounting = QString::fromUtf8("后机身两侧");
            }
        }
        return d;
    }

    // 默认（窄体运输机常规布局）。
    return seedDocument();
}
