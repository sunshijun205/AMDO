#ifndef ANALYSISPRESENTER_H
#define ANALYSISPRESENTER_H

#include <QObject>

class AnalysisPage;
class AnalysisDocumentService;

class AnalysisPresenter : public QObject
{
    Q_OBJECT
public:
    AnalysisPresenter(AnalysisPage *view, AnalysisDocumentService *document,
                      QObject *parent = nullptr);

public slots:
    void onLoadRequested();
    void onSaveRequested();
    void onValidateRequested();
    void onPublishRequested();
    void onSwitchBaselineRequested(const QString &id);
    void onCopyToDraftRequested();

private:
    void refreshView();

    AnalysisPage *m_view;
    AnalysisDocumentService *m_document;
};

#endif
