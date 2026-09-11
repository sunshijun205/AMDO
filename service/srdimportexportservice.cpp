#include "service/srdimportexportservice.h"

#include "model/srdcatalogs.h"
#include "model/srdstore.h"

#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QTextStream>

#include <yaml-cpp/yaml.h>

#include <string>

namespace {
std::string toU8(const QString &s)
{
    const QByteArray bytes = s.toUtf8();
    return std::string(bytes.constData(), static_cast<size_t>(bytes.size()));
}
}

SrdImportExportService::SrdImportExportService(SrdStore *store)
    : m_store(store)
{
}

bool SrdImportExportService::importSrdJson(const QString &path, SrdDocument *out, QString *errorMessage)
{
    if (!m_store) {
        if (errorMessage)
            *errorMessage = QString::fromUtf8("导入失败：存储未初始化");
        return false;
    }
    QString detail;
    if (!m_store->readDocumentFile(path, out, &detail)) {
        if (errorMessage)
            *errorMessage = QString::fromUtf8("导入需求失败：%1").arg(detail);
        return false;
    }
    return true;
}

static QStringList splitCsvLine(const QString &line)
{
    QStringList cells;
    QString cur;
    bool inQuote = false;
    for (int i = 0; i < line.size(); ++i) {
        const QChar ch = line.at(i);
        if (ch == QLatin1Char('"')) {
            inQuote = !inQuote;
            continue;
        }
        if (ch == QLatin1Char(',') && !inQuote) {
            cells.append(cur.trimmed());
            cur.clear();
            continue;
        }
        cur.append(ch);
    }
    cells.append(cur.trimmed());
    return cells;
}

bool SrdImportExportService::importNeedsCsv(const QString &path, QVector<SrdPerformanceNeed> *out,
                                            QString *errorMessage)
{
    if (!out) {
        if (errorMessage)
            *errorMessage = QString::fromUtf8("导入失败：输出参数为空");
        return false;
    }

    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        if (errorMessage)
            *errorMessage = QString::fromUtf8("导入 CSV 失败：%1").arg(file.errorString());
        return false;
    }

    QTextStream stream(&file);
    stream.setCodec("UTF-8");
    if (stream.atEnd()) {
        if (errorMessage)
            *errorMessage = QString::fromUtf8("CSV 为空");
        return false;
    }

    const QString headerLine = stream.readLine();
    const QStringList header = splitCsvLine(headerLine);
    int colMetric = header.indexOf(QStringLiteral("metricId"));
    int colRel = header.indexOf(QStringLiteral("relation"));
    int colValue = header.indexOf(QStringLiteral("value"));
    int colUnit = header.indexOf(QStringLiteral("unit"));
    int colGrade = header.indexOf(QStringLiteral("grade"));
    int colName = header.indexOf(QStringLiteral("name"));
    if (colMetric < 0 || colRel < 0 || colValue < 0) {
        if (errorMessage)
            *errorMessage = QString::fromUtf8("CSV 缺少必需列 metricId, relation, value");
        return false;
    }

    int seq = 1;
    out->clear();
    while (!stream.atEnd()) {
        const QString line = stream.readLine();
        if (line.trimmed().isEmpty())
            continue;
        const QStringList cells = splitCsvLine(line);
        SrdPerformanceNeed n;
        n.metricId = cells.value(colMetric);
        SrdCatalogMetric metric;
        if (!SrdCatalogs::findMetric(n.metricId, &metric)) {
            if (errorMessage)
                *errorMessage = QString::fromUtf8("CSV 含未知指标：%1").arg(n.metricId);
            return false;
        }
        n.id = QStringLiteral("NEED-IMP-%1").arg(seq, 3, 10, QLatin1Char('0'));
        ++seq;
        n.name = colName >= 0 ? cells.value(colName) : metric.name;
        if (n.name.isEmpty())
            n.name = metric.name;
        n.relation = cells.value(colRel);
        bool ok = false;
        n.value = cells.value(colValue).toDouble(&ok);
        n.valueKnown = ok;
        n.unit = colUnit >= 0 ? cells.value(colUnit) : metric.unit;
        n.grade = colGrade >= 0 ? cells.value(colGrade) : QString::fromUtf8("目标");
        n.sourceKind = QString::fromUtf8("市场需求");
        out->append(n);
    }
    return true;
}

QString SrdImportExportService::defaultExportPath(const SrdDocument &doc) const
{
    QString name = doc.id.isEmpty() ? QStringLiteral("srd-draft") : doc.id;
    name.replace(QLatin1Char('/'), QLatin1Char('_'));
    if (!m_store)
        return name + QLatin1String("-evaluation-spec.yaml");
    return m_store->rootDir() + QLatin1Char('/') + name + QLatin1String("-evaluation-spec.yaml");
}

bool SrdImportExportService::exportEvaluationSpec(const QString &path, const SrdDocument &doc,
                                                  QString *errorMessage)
{
    YAML::Node root;
    root["schema"] = std::string("amdo.evaluation-spec.v1");
    root["baselineId"] = toU8(doc.id);
    root["title"] = toU8(doc.title);
    root["publishedAt"] = toU8(doc.publishedAt);
    root["status"] = toU8(doc.status);

    YAML::Node missions(YAML::NodeType::Sequence);
    for (int i = 0; i < doc.missions.size(); ++i) {
        YAML::Node m;
        m["id"] = toU8(doc.missions[i].id);
        m["name"] = toU8(doc.missions[i].name);
        m["boundary"] = toU8(doc.missions[i].boundary);
        m["environment"] = toU8(doc.missions[i].environment);
        m["status"] = toU8(doc.missions[i].status);
        missions.push_back(m);
    }
    root["missions"] = missions;

    YAML::Node conditions(YAML::NodeType::Sequence);
    for (int i = 0; i < doc.conditions.size(); ++i) {
        YAML::Node c;
        c["id"] = toU8(doc.conditions[i].id);
        c["phase"] = toU8(doc.conditions[i].phase);
        c["altitude"] = toU8(doc.conditions[i].altitude);
        c["speed"] = toU8(doc.conditions[i].speed);
        c["configuration"] = toU8(doc.conditions[i].configuration);
        c["atmosphere"] = toU8(doc.conditions[i].atmosphere);
        conditions.push_back(c);
    }
    root["flightConditions"] = conditions;

    YAML::Node metrics(YAML::NodeType::Sequence);
    YAML::Node constraints(YAML::NodeType::Sequence);
    for (int i = 0; i < doc.requirements.size(); ++i) {
        const SrdRequirement &r = doc.requirements[i];
        YAML::Node m;
        m["id"] = toU8(r.metricId);
        m["name"] = toU8(r.metricName);
        m["unit"] = toU8(r.unit);
        m["domain"] = toU8(r.domain);
        m["responseId"] = toU8(r.analysisResponseId);
        metrics.push_back(m);

        YAML::Node c;
        c["reqId"] = toU8(r.id);
        c["metricId"] = toU8(r.metricId);
        c["op"] = toU8(r.relation);
        if (r.boundKnown)
            c["value"] = r.boundValue;
        c["unit"] = toU8(r.unit);
        c["kind"] = toU8(r.constraintKind);
        c["source"] = toU8(r.source);
        constraints.push_back(c);
    }
    root["metrics"] = metrics;
    root["constraints"] = constraints;

    YAML::Emitter emitter;
    emitter << root;
    if (!emitter.good()) {
        if (errorMessage)
            *errorMessage = QString::fromUtf8("导出评价规格失败：YAML 序列化错误（%1）")
                                .arg(QString::fromStdString(emitter.GetLastError()));
        return false;
    }

    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        if (errorMessage)
            *errorMessage = QString::fromUtf8("导出评价规格失败：%1").arg(file.errorString());
        return false;
    }
    QByteArray bytes(emitter.c_str(), static_cast<int>(emitter.size()));
    bytes.append('\n');
    if (file.write(bytes) != bytes.size()) {
        if (errorMessage)
            *errorMessage = QString::fromUtf8("导出评价规格写入不完整");
        return false;
    }
    return true;
}
