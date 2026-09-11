#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QPushButton>
#include <QStackedWidget>
#include <QVector>

class RequirementsPage;
class DefinitionPage;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

private:
    void switchMode(int index);
    void updateActions(int index);
    void onPrimaryClicked();
    void onSecondaryClicked();

    QStackedWidget *m_pages = nullptr;
    RequirementsPage *m_requirementsPage = nullptr;
    DefinitionPage *m_definitionPage = nullptr;
    QPushButton *m_primary = nullptr;
    QPushButton *m_secondary = nullptr;
    QVector<QPushButton *> m_modeButtons;
};

#endif
