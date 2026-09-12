#ifndef DEFINITIONPRESENTER_H
#define DEFINITIONPRESENTER_H

#include <QObject>
#include <QString>

class DefinitionPage;
class AircraftDocumentService;
class AircraftImportExportService;

class DefinitionPresenter : public QObject
{
    Q_OBJECT
public:
    DefinitionPresenter(DefinitionPage *view,
                        AircraftDocumentService *document,
                        AircraftImportExportService *io,
                        QObject *parent = nullptr);

public slots:
    void onLoadRequested();
    void onSwitchBaselineRequested(const QString &id);
    void onCopyBaselineRequested();
    void onNewBlankRequested();
    void onNewFromTemplateRequested(const QString &templateName);
    void onSaveAllRequested();
    void onSaveSemanticsRequested();
    void onSaveConfigurationRequested();
    void onSaveParametersRequested();
    void onAddSystemRequested();
    void onRemoveSystemRequested();
    void onAddParameterRequested();
    void onRemoveParameterRequested();
    void onPublishRequested();
    void onCaseSnapshotRequested();
    void onImportPathRequested(const QString &path);
    void onExportRequested();
    void onIntegrityCheckRequested();

private:
    bool persistAll(QString *errorMessage);
    void refreshView();
    void fail(const QString &error);

    DefinitionPage *m_view;
    AircraftDocumentService *m_document;
    AircraftImportExportService *m_io;
};

#endif
