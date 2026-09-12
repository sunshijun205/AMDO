#ifndef AIRCRAFTCPACSSERVICE_H
#define AIRCRAFTCPACSSERVICE_H

#include "model/aircrafttypes.h"

#include <QString>

class AircraftStore;
class QXmlStreamWriter;

// 将飞机方案文档序列化为 CPACS（简化子集）飞机语义主数据 aircraft_Rxxx.cpacs.xml。
// 每个已发布方案版本对应一份修订，供评价/优化/分析三流共用。
class AircraftCpacsService
{
public:
    explicit AircraftCpacsService(AircraftStore *store);

    // 按 doc.version 写出 cpacs/aircraft_R<version>.cpacs.xml。
    bool exportRevision(const AircraftDocument &doc, QString *outPath = nullptr,
                        QString *errorMessage = nullptr);

    // 将方案冻结为不可变的分析用例输入 cases/case_<n>.input.cpacs.xml（含用例登记）。
    bool exportCaseSnapshot(const AircraftDocument &doc, int caseNumber,
                            QString *outPath = nullptr, QString *errorMessage = nullptr);

    // 修订号（Rxxx）与目标文件路径，便于状态提示与外部引用。
    static QString revisionId(int version);
    QString revisionPath(int version) const;

private:
    // 用例上下文；number <= 0 表示非用例快照（普通修订）。
    struct CaseInfo {
        int number = -1;
        QString sourceBaseline;
        QString sourceRevision;
    };
    void writeDocument(QXmlStreamWriter &writer, const AircraftDocument &doc,
                       const CaseInfo &caseInfo) const;

    AircraftStore *m_store;
};

#endif
