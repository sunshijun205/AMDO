#include "analysispage.h"

#include "controller/analysispresenter.h"
#include "model/analysiscatalogs.h"
#include "model/analysisstore.h"
#include "service/analysisdocumentservice.h"
#include "uihelpers.h"

#include <QAbstractButton>
#include <QButtonGroup>
#include <QCheckBox>
#include <QComboBox>
#include <QFrame>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QScrollArea>
#include <QStackedWidget>
#include <QStyle>
#include <QVBoxLayout>

AnalysisPage::AnalysisPage(QWidget *parent)
    : QWidget(parent)
{
    buildUi();

    m_store.reset(new AnalysisStore);
    m_document.reset(new AnalysisDocumentService(m_store.get()));
    m_presenter = new AnalysisPresenter(this, m_document.get(), this);
}

AnalysisPage::~AnalysisPage() = default;

QWidget *AnalysisPage::makeFieldWidget(const AnalysisField &field, const QString &key)
{
    if (field.type == QLatin1String("check")) {
        QCheckBox *box = makeCheck(field.label, field.defaultValue == QString::fromUtf8("启用"));
        m_editors.insert(key, box);
        return box;
    }
    if (field.type == QLatin1String("select")) {
        QWidget *w = makeSelectField(field.label, field.defaultValue, field.options);
        if (QComboBox *combo = w->findChild<QComboBox *>())
            m_editors.insert(key, combo);
        return w;
    }
    QWidget *w = makeField(field.label, field.defaultValue, field.unit);
    if (QLineEdit *edit = w->findChild<QLineEdit *>())
        m_editors.insert(key, edit);
    return w;
}

QWidget *AnalysisPage::buildDomainPage(const AnalysisDomain &domain)
{
    auto *root = new QWidget;
    auto *lay = new QVBoxLayout(root);
    lay->setContentsMargins(20, 18, 20, 22);
    lay->setSpacing(14);
    lay->addWidget(makeHeading(domain.title, domain.subtitle,
                               QString::fromUtf8("配置模板"), domain.templates));

    auto *body = new QWidget;
    auto *hl = new QHBoxLayout(body);
    hl->setContentsMargins(0, 0, 0, 0);
    hl->setSpacing(14);

    auto *sections = new QWidget;
    auto *grid = new QGridLayout(sections);
    grid->setContentsMargins(0, 0, 0, 0);
    grid->setSpacing(12);
    for (int i = 0; i < domain.sections.size(); ++i) {
        const AnalysisSection &s = domain.sections[i];
        auto *sec = new QFrame;
        sec->setObjectName(QStringLiteral("Section"));
        auto *vl = new QVBoxLayout(sec);
        vl->setContentsMargins(12, 12, 12, 12);
        vl->setSpacing(10);
        auto *titleRow = new QWidget;
        auto *th = new QHBoxLayout(titleRow);
        th->setContentsMargins(0, 0, 0, 0);
        th->setSpacing(7);
        auto *bar = new QFrame;
        bar->setFixedSize(3, 15);
        bar->setStyleSheet(QStringLiteral("background: #0c9b88; border-radius: 2px;"));
        auto *h = new QLabel(s.title);
        h->setObjectName(QStringLiteral("SectionTitle"));
        th->addWidget(bar);
        th->addWidget(h);
        th->addStretch();
        vl->addWidget(titleRow);

        auto *fields = new QWidget;
        auto *fg = new QGridLayout(fields);
        fg->setContentsMargins(0, 0, 0, 0);
        fg->setHorizontalSpacing(10);
        fg->setVerticalSpacing(8);
        for (int fi = 0; fi < s.fields.size(); ++fi) {
            const QString key = analysisFieldKey(domain.id, s.title, s.fields[fi].label);
            fg->addWidget(makeFieldWidget(s.fields[fi], key), fi / 2, fi % 2);
        }
        vl->addWidget(fields);
        grid->addWidget(sec, i / 2, i % 2);
    }

    auto *aside = new QWidget;
    auto *al = new QVBoxLayout(aside);
    al->setContentsMargins(0, 0, 0, 0);
    al->setSpacing(12);
    aside->setFixedWidth(250);
    auto *sum = makePanel();
    sum->layout()->addWidget(makePanelTitle(QString::fromUtf8("当前配置摘要")));
    QVector<SummaryItem> summary;
    for (int i = 0; i < domain.summary.size(); ++i) {
        SummaryItem item;
        item.key = domain.summary[i].key;
        item.value = domain.summary[i].value;
        summary.append(item);
    }
    sum->layout()->addWidget(makeSummary(summary));
    al->addWidget(sum);
    auto *chk = makePanel();
    chk->layout()->addWidget(makePanelTitle(QString::fromUtf8("运行前检查")));
    chk->layout()->addWidget(makeBulletList(domain.checks));
    al->addWidget(chk);
    al->addStretch();

    hl->addWidget(sections, 1);
    hl->addWidget(aside);
    lay->addWidget(body);

    auto *run = new QFrame;
    run->setObjectName(QStringLiteral("RunBar"));
    auto *rl = new QHBoxLayout(run);
    rl->setContentsMargins(12, 10, 12, 10);
    auto *dot = new QFrame;
    dot->setFixedSize(8, 8);
    dot->setStyleSheet(QStringLiteral("background: #0c9b88; border-radius: 4px;"));
    auto *note = new QLabel(QString::fromUtf8("跨学科公共变量从项目基线继承；保存后写入本机分析集草稿。"));
    note->setObjectName(QStringLiteral("NoteLabel"));
    auto *saveBtn = makeButton(QString::fromUtf8("保存分析集"), true);
    connect(saveBtn, &QPushButton::clicked, this, &AnalysisPage::saveRequested);
    auto *checkBtn = makeButton(QString::fromUtf8("校验配置"));
    connect(checkBtn, &QPushButton::clicked, this, &AnalysisPage::validateRequested);
    rl->addWidget(dot);
    rl->addWidget(note);
    rl->addStretch();
    rl->addWidget(checkBtn);
    rl->addWidget(saveBtn);
    lay->addWidget(run);
    return root;
}

static QPushButton *makeDomainTab(int index, const QString &name, bool active)
{
    auto *btn = new QPushButton;
    btn->setObjectName(QStringLiteral("DomainTab"));
    btn->setCheckable(true);
    btn->setChecked(active);
    btn->setCursor(Qt::PointingHandCursor);
    auto *hl = new QHBoxLayout(btn);
    hl->setContentsMargins(4, 4, 8, 4);
    hl->setSpacing(7);
    auto *num = new QLabel(QString::number(index));
    num->setObjectName(active ? QStringLiteral("DomainNumActive") : QStringLiteral("DomainNum"));
    num->setProperty("role", QStringLiteral("num"));
    num->setFixedSize(25, 25);
    num->setAlignment(Qt::AlignCenter);
    num->setAttribute(Qt::WA_TransparentForMouseEvents);
    auto *lb = new QLabel(name);
    lb->setAttribute(Qt::WA_TransparentForMouseEvents);
    auto *dot = new QFrame;
    dot->setFixedSize(8, 8);
    dot->setStyleSheet(QStringLiteral("background: #0c9b88; border-radius: 4px;"));
    dot->setAttribute(Qt::WA_TransparentForMouseEvents);
    hl->addWidget(num);
    hl->addWidget(lb, 1);
    hl->addWidget(dot);
    return btn;
}

void AnalysisPage::buildUi()
{
    const QVector<AnalysisDomain> domains = AnalysisCatalogs::domains();

    auto *outer = new QVBoxLayout(this);
    outer->setContentsMargins(0, 0, 0, 0);
    outer->setSpacing(0);

    auto *header = new QFrame;
    header->setObjectName(QStringLiteral("RunBar"));
    auto *hb = new QHBoxLayout(header);
    hb->setContentsMargins(20, 10, 20, 10);
    hb->setSpacing(8);
    auto *verLabel = new QLabel(QString::fromUtf8("分析集版本"));
    verLabel->setObjectName(QStringLiteral("FieldLabel"));
    m_baselineBox = new QComboBox;
    m_baselineBox->setMinimumWidth(260);
    hb->addWidget(verLabel);
    hb->addWidget(m_baselineBox);
    hb->addStretch();
    m_copyDraftBtn = makeButton(QString::fromUtf8("另存为新草稿"));
    m_publishBtn = makeButton(QString::fromUtf8("发布分析集版本"), true);
    hb->addWidget(m_copyDraftBtn);
    hb->addWidget(m_publishBtn);
    outer->addWidget(header);
    connect(m_baselineBox, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
            this, &AnalysisPage::onBaselineChanged);
    connect(m_copyDraftBtn, &QPushButton::clicked, this, &AnalysisPage::copyToDraftRequested);
    connect(m_publishBtn, &QPushButton::clicked, this, &AnalysisPage::publishRequested);

    auto *content = new QWidget;
    auto *rootLay = new QHBoxLayout(content);
    rootLay->setContentsMargins(0, 0, 0, 0);
    rootLay->setSpacing(0);

    auto *side = new QFrame;
    side->setObjectName(QStringLiteral("SideBar"));
    side->setFixedWidth(238);
    auto *sl = new QVBoxLayout(side);
    sl->setContentsMargins(14, 18, 14, 18);
    sl->setSpacing(4);
    auto *sideTitle = new QLabel(QStringLiteral("ANALYSIS DOMAINS"));
    sideTitle->setObjectName(QStringLiteral("SideTitle"));
    sl->addWidget(sideTitle);

    auto *stack = new QStackedWidget;
    auto *group = new QButtonGroup(this);
    group->setExclusive(true);
    for (int i = 0; i < domains.size(); ++i) {
        auto *tab = makeDomainTab(i + 1, domains[i].name, i == 0);
        group->addButton(tab, i);
        sl->addWidget(tab);
        stack->addWidget(wrapScroll(buildDomainPage(domains[i])));
    }
    connect(group, &QButtonGroup::idClicked, this, [stack, group](int id) {
        stack->setCurrentIndex(id);
        const auto buttons = group->buttons();
        for (QAbstractButton *button : buttons) {
            auto *num = button->findChild<QLabel *>();
            if (!num || num->property("role").toString() != QLatin1String("num"))
                continue;
            const bool on = (group->id(button) == id);
            num->setObjectName(on ? QStringLiteral("DomainNumActive") : QStringLiteral("DomainNum"));
            num->style()->unpolish(num);
            num->style()->polish(num);
        }
    });
    sl->addStretch();
    auto *note = new QLabel(QString::fromUtf8("每个学科统一采用：分析方法 → 输入与工况 → 离散/模型 → 求解控制。跨学科公共变量从项目基线继承。"));
    note->setObjectName(QStringLiteral("SideNote"));
    note->setWordWrap(true);
    sl->addWidget(note);

    rootLay->addWidget(side);
    rootLay->addWidget(stack, 1);
    outer->addWidget(content, 1);

    m_status = new QLabel(QString::fromUtf8("就绪"));
    m_status->setObjectName(QStringLiteral("PageStatus"));
    m_status->setContentsMargins(20, 6, 20, 6);
    outer->addWidget(m_status);
}

void AnalysisPage::setDocument(const AnalysisDocument &doc)
{
    for (auto it = m_editors.constBegin(); it != m_editors.constEnd(); ++it) {
        if (!doc.values.contains(it.key()))
            continue;
        const QString value = doc.values.value(it.key());
        QWidget *w = it.value();
        if (auto *box = qobject_cast<QCheckBox *>(w)) {
            box->setChecked(value == QString::fromUtf8("启用"));
        } else if (auto *combo = qobject_cast<QComboBox *>(w)) {
            int idx = combo->findText(value);
            if (idx < 0) {
                combo->insertItem(0, value);
                idx = 0;
            }
            combo->setCurrentIndex(idx);
        } else if (auto *edit = qobject_cast<QLineEdit *>(w)) {
            edit->setText(value);
        }
    }
}

void AnalysisPage::setBaselines(const QVector<AnalysisBaselineInfo> &baselines, const QString &currentId)
{
    if (!m_baselineBox)
        return;
    m_updating = true;
    m_baselineBox->clear();
    m_baselineBox->addItem(QString::fromUtf8("草稿（可编辑）"), QStringLiteral("draft"));
    int current = 0;
    for (int i = 0; i < baselines.size(); ++i) {
        QString label = baselines[i].title;
        if (label.isEmpty())
            label = baselines[i].id;
        if (baselines[i].version > 0)
            label += QString::fromUtf8("  v%1").arg(baselines[i].version);
        if (!baselines[i].publishedAt.isEmpty())
            label += QStringLiteral("  ") + baselines[i].publishedAt;
        m_baselineBox->addItem(label, baselines[i].id);
        if (baselines[i].id == currentId)
            current = i + 1;
    }
    if (currentId == QLatin1String("draft"))
        current = 0;
    m_baselineBox->setCurrentIndex(current);
    m_updating = false;
}

void AnalysisPage::setReadOnly(bool readOnly)
{
    m_readOnly = readOnly;
    for (auto it = m_editors.constBegin(); it != m_editors.constEnd(); ++it)
        it.value()->setEnabled(!readOnly);
    if (m_copyDraftBtn)
        m_copyDraftBtn->setEnabled(readOnly);
    if (m_publishBtn)
        m_publishBtn->setEnabled(!readOnly);
}

void AnalysisPage::onBaselineChanged(int)
{
    if (m_updating || !m_baselineBox)
        return;
    emit switchBaselineRequested(m_baselineBox->currentData().toString());
}

void AnalysisPage::snapshot(QHash<QString, QString> *values) const
{
    values->clear();
    for (auto it = m_editors.constBegin(); it != m_editors.constEnd(); ++it) {
        QWidget *w = it.value();
        if (auto *box = qobject_cast<QCheckBox *>(w))
            values->insert(it.key(), box->isChecked() ? QString::fromUtf8("启用") : QString::fromUtf8("关闭"));
        else if (auto *combo = qobject_cast<QComboBox *>(w))
            values->insert(it.key(), combo->currentText());
        else if (auto *edit = qobject_cast<QLineEdit *>(w))
            values->insert(it.key(), edit->text());
    }
}

void AnalysisPage::requestSaveAll()
{
    emit saveRequested();
}

void AnalysisPage::requestValidate()
{
    emit validateRequested();
}

void AnalysisPage::setStatus(const QString &text, bool isError)
{
    if (!m_status)
        return;
    m_status->setText(text);
    m_status->setStyleSheet(isError ? QStringLiteral("color: #b46b22;") : QString());
}

void AnalysisPage::showError(const QString &message)
{
    QMessageBox::warning(this, QString::fromUtf8("学科分析"), message);
}

void AnalysisPage::reportValidation(const QStringList &issues)
{
    if (issues.isEmpty()) {
        QMessageBox::information(this, QString::fromUtf8("配置校验"),
                                 QString::fromUtf8("配置校验通过，未发现待处理项。"));
        return;
    }
    QString text = QString::fromUtf8("发现以下待处理项：\n\n");
    for (int i = 0; i < issues.size(); ++i)
        text += QString::fromUtf8("• ") + issues[i] + QLatin1Char('\n');
    QMessageBox::warning(this, QString::fromUtf8("配置校验"), text);
}
