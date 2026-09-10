#include "mainwindow.h"
#include "theme.h"
#include "uihelpers.h"
#include "analysispage.h"
#include "decisionpage.h"
#include "definitionpage.h"
#include "designpage.h"
#include "requirementspage.h"
#include "workflowpage.h"

#include <QButtonGroup>
#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QMenuBar>
#include <QStatusBar>
#include <QVBoxLayout>
#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setWindowTitle(QString::fromUtf8("飞机概念设计平台"));
    resize(1440, 900);
    setMinimumSize(1100, 720);
    menuBar()->hide();
    statusBar()->hide();

    auto *root = new QWidget;
    root->setObjectName(QStringLiteral("RootShell"));
    auto *rootLay = new QVBoxLayout(root);
    rootLay->setContentsMargins(0, 0, 0, 0);
    rootLay->setSpacing(0);

    auto *top = new QFrame;
    top->setObjectName(QStringLiteral("TopBar"));
    top->setFixedHeight(54);
    auto *tl = new QHBoxLayout(top);
    tl->setContentsMargins(22, 0, 22, 0);
    tl->setSpacing(18);
    auto *brand = new QLabel(QString::fromUtf8("飞机概念设计平台"));
    brand->setObjectName(QStringLiteral("BrandLabel"));
    auto *divider = new QFrame;
    divider->setFrameShape(QFrame::VLine);
    divider->setStyleSheet(QStringLiteral("color: #d5dfe4;"));
    divider->setFixedHeight(18);
    auto *project = new QLabel(QString::fromUtf8("项目：HX-01 概念方案　/　基准构型 v12"));
    project->setObjectName(QStringLiteral("ProjectLabel"));
    m_secondary = makeButton(QString::fromUtf8("导入需求"));
    m_primary = makeButton(QString::fromUtf8("发布需求基线"), true);
    connect(m_secondary, &QPushButton::clicked, this, &MainWindow::onSecondaryClicked);
    connect(m_primary, &QPushButton::clicked, this, &MainWindow::onPrimaryClicked);
    tl->addWidget(brand);
    tl->addWidget(divider);
    tl->addWidget(project);
    tl->addStretch();
    tl->addWidget(m_secondary);
    tl->addWidget(m_primary);

    auto *func = new QFrame;
    func->setObjectName(QStringLiteral("FunctionBar"));
    auto *fl = new QHBoxLayout(func);
    fl->setContentsMargins(22, 8, 22, 8);
    fl->setSpacing(8);
    const QStringList modes = {
        QString::fromUtf8("设计需求"),
        QString::fromUtf8("飞机方案定义"),
        QString::fromUtf8("学科分析"),
        QString::fromUtf8("方案优化"),
        QString::fromUtf8("方案决策"),
        QString::fromUtf8("工作流")
    };
    auto *group = new QButtonGroup(this);
    group->setExclusive(true);
    for (int i = 0; i < modes.size(); ++i) {
        auto *btn = new QPushButton(modes[i]);
        btn->setObjectName(QStringLiteral("ModeButton"));
        btn->setCheckable(true);
        btn->setChecked(i == 0);
        btn->setCursor(Qt::PointingHandCursor);
        group->addButton(btn, i);
        fl->addWidget(btn, 1);
        m_modeButtons.append(btn);
        connect(btn, &QPushButton::clicked, this, [this, i]() { switchMode(i); });
    }

    m_pages = new QStackedWidget;
    m_requirementsPage = new RequirementsPage;
    m_pages->addWidget(m_requirementsPage);
    m_pages->addWidget(new DefinitionPage);
    m_pages->addWidget(new AnalysisPage);
    m_pages->addWidget(new DesignPage);
    m_pages->addWidget(new DecisionPage);
    m_pages->addWidget(new WorkflowPage);

    rootLay->addWidget(top);
    rootLay->addWidget(func);
    rootLay->addWidget(m_pages, 1);
    setCentralWidget(root);
    switchMode(0);
}

MainWindow::~MainWindow() = default;

void MainWindow::switchMode(int index)
{
    m_pages->setCurrentIndex(index);
    for (int i = 0; i < m_modeButtons.size(); ++i)
        m_modeButtons[i]->setChecked(i == index);
    updateActions(index);
}

void MainWindow::updateActions(int index)
{
    static const char *primary[] = {
        "发布需求基线", "保存方案定义", "保存分析集",
        "保存设计空间", "生成评估报告", "运行工作流"
    };
    static const char *secondary[] = {
        "导入需求", "完整性检查", "校验配置",
        "导入变量", "导出数据", "保存为模板"
    };
    m_primary->setText(QString::fromUtf8(primary[index]));
    m_secondary->setText(QString::fromUtf8(secondary[index]));
}

void MainWindow::onPrimaryClicked()
{
    if (m_pages->currentIndex() == 0 && m_requirementsPage) {
        m_requirementsPage->requestPublish();
        return;
    }
    QMessageBox::information(this, QString::fromUtf8("飞机概念设计平台"),
                             m_primary->text() + QString::fromUtf8(" — 原型交互已记录。"));
}

void MainWindow::onSecondaryClicked()
{
    if (m_pages->currentIndex() == 0 && m_requirementsPage) {
        m_requirementsPage->requestImport();
        return;
    }
    QMessageBox::information(this, QString::fromUtf8("飞机概念设计平台"),
                             m_secondary->text() + QString::fromUtf8(" — 原型交互已记录。"));
}
