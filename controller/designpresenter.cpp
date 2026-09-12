#include "controller/designpresenter.h"

#include "designpage.h"
#include "model/aircraftstore.h"
#include "model/aircrafttypes.h"
#include "model/analysisstore.h"
#include "service/aircraftcpacsservice.h"
#include "service/aircraftdocumentservice.h"
#include "service/studyservice.h"

#include <QRegExp>

DesignPresenter::DesignPresenter(DesignPage *view, AnalysisStore *analysisStore, StudyService *study,
                                 AircraftStore *aircraftStore, AircraftDocumentService *aircraftDoc,
                                 QObject *parent)
    : QObject(parent)
    , m_view(view)
    , m_analysisStore(analysisStore)
    , m_study(study)
    , m_aircraftStore(aircraftStore)
    , m_aircraftDoc(aircraftDoc)
{
    connect(m_view, &DesignPage::exploreRequested, this, &DesignPresenter::onExploreRequested);
    connect(m_view, &DesignPage::promoteRequested, this, &DesignPresenter::onPromoteRequested);
}

void DesignPresenter::onExploreRequested()
{
    if (!m_analysisStore || !m_study) {
        m_view->setStatus(QString::fromUtf8("探索服务未初始化"), true);
        return;
    }

    // 基准方案：所选优化基准（草稿 / analysis_vN）。
    const QString objectId = m_view->baseObjectId();
    AnalysisDocument base;
    QString detail;
    const bool ok = (objectId.isEmpty() || objectId == QLatin1String("draft"))
                        ? m_analysisStore->loadDraft(&base, &detail)
                        : m_analysisStore->loadBaseline(objectId, &base, &detail);
    if (!ok) {
        m_view->showError(QString::fromUtf8("加载分析集失败：%1").arg(detail));
        m_view->setStatus(QString::fromUtf8("加载分析集失败"), true);
        return;
    }
    if (base.id.isEmpty()) {
        m_view->setStatus(QString::fromUtf8("尚无分析集，请先在「学科分析」页配置并关联设计需求"), true);
        return;
    }

    StudyDefinition def = m_view->studyDefinition();
    def.baseObjectId = objectId;

    QString error;
    const StudyResult result = m_study->run(base, def, &error);
    m_lastBase = base;
    m_lastResult = result;

    if (result.points.isEmpty()) {
        m_view->setStatus(error.isEmpty() ? QString::fromUtf8("未生成任何设计点") : error, true);
        m_view->showStudyResult(result);
        return;
    }

    QString saveError;
    m_study->saveResult(result, nullptr, &saveError);
    m_view->showStudyResult(result);

    int feasible = 0;
    for (int i = 0; i < result.points.size(); ++i)
        if (result.points[i].feasibility == QString::fromUtf8("通过"))
            ++feasible;
    m_view->setStatus(QString::fromUtf8("设计空间探索完成：基准 %1，采样 %2 点，可行 %3 点%4")
                          .arg(objectId)
                          .arg(result.points.size())
                          .arg(feasible)
                          .arg(result.srdResolved ? QString() : QString::fromUtf8("（未关联设计需求，仅得几何/气动量）")));
}

void DesignPresenter::onPromoteRequested()
{
    if (!m_aircraftStore || !m_aircraftDoc) {
        m_view->setStatus(QString::fromUtf8("飞机方案服务未初始化"), true);
        return;
    }
    if (m_lastResult.bestIndex < 0 || m_lastResult.bestIndex >= m_lastResult.points.size()) {
        m_view->setStatus(QString::fromUtf8("请先运行探索得到最优点，再提升为飞机方案版本"), true);
        return;
    }

    // 基准飞机：由基准分析集的 sourceRevision(R00N) 解析到 aircraft_vN。
    QRegExp re(QStringLiteral("^R0*([0-9]+)$"));
    if (!re.exactMatch(m_lastBase.sourceRevision)) {
        m_view->setStatus(QString::fromUtf8("基准未关联飞机修订，无法提升为飞机方案版本"), true);
        return;
    }
    const int version = re.cap(1).toInt();
    const QString baselineId = QStringLiteral("aircraft_v%1").arg(version);
    AircraftDocument aircraft;
    QString detail;
    if (!m_aircraftStore->loadBaseline(baselineId, &aircraft, &detail)) {
        m_view->showError(QString::fromUtf8("加载基准飞机失败：%1").arg(detail));
        m_view->setStatus(QString::fromUtf8("加载基准飞机失败"), true);
        return;
    }

    // 用最优设计点的变量覆盖飞机参数（按 symbol）。
    const StudyPointResult &best = m_lastResult.points[m_lastResult.bestIndex];
    for (int i = 0; i < aircraft.parameters.size(); ++i) {
        AcParameter &p = aircraft.parameters[i];
        if (best.variables.contains(p.symbol)) {
            p.value = best.variables.value(p.symbol);
            p.valueKnown = true;
        }
    }
    aircraft.title = QString::fromUtf8("优化候选（自 %1）").arg(m_lastBase.sourceRevision);

    AircraftDocument published;
    QString error;
    if (!m_aircraftDoc->publishExternalBaseline(aircraft, &published, &error)) {
        m_view->showError(error);
        m_view->setStatus(error, true);
        return;
    }
    m_view->setStatus(QString::fromUtf8("已提升为新飞机方案版本：%1（CPACS %2）")
                          .arg(published.id, AircraftCpacsService::revisionId(published.version)));
}
