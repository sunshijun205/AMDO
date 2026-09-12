#ifndef DESIGNPRESENTER_H
#define DESIGNPRESENTER_H

#include "model/analysistypes.h"
#include "model/studytypes.h"

#include <QObject>

class DesignPage;
class AnalysisStore;
class StudyService;
class AircraftStore;
class AircraftDocumentService;

class DesignPresenter : public QObject
{
    Q_OBJECT
public:
    DesignPresenter(DesignPage *view, AnalysisStore *analysisStore, StudyService *study,
                    AircraftStore *aircraftStore, AircraftDocumentService *aircraftDoc,
                    QObject *parent = nullptr);

public slots:
    void onExploreRequested();
    void onPromoteRequested();

private:
    DesignPage *m_view;
    AnalysisStore *m_analysisStore;
    StudyService *m_study;
    AircraftStore *m_aircraftStore;
    AircraftDocumentService *m_aircraftDoc;

    StudyResult m_lastResult;      // 最近一次探索结果
    AnalysisDocument m_lastBase;   // 最近一次探索的基准分析集
};

#endif
