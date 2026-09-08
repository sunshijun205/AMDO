#include "uihelpers.h"
#include "theme.h"

#include <QAbstractButton>
#include <QButtonGroup>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QMessageBox>
#include <QProgressBar>
#include <QScrollBar>
#include <QVBoxLayout>

static void applyStatusColor(QTableWidgetItem *item)
{
    const QString text = item->text();
    if (isWarnStatus(text))
        item->setForeground(Theme::warn());
    else if (isGoodStatus(text))
        item->setForeground(Theme::accent());
}

bool isWarnStatus(const QString &text)
{
    return text.contains(QString::fromUtf8("临界"))
        || text.contains(QString::fromUtf8("待"))
        || text.contains(QString::fromUtf8("草案"))
        || text.contains(QString::fromUtf8("需检查"))
        || text.contains(QString::fromUtf8("等待"))
        || text.contains(QString::fromUtf8("运行中"));
}

bool isGoodStatus(const QString &text)
{
    return text.contains(QString::fromUtf8("满足"))
        || text.contains(QString::fromUtf8("有效"))
        || text.contains(QString::fromUtf8("已定义"))
        || text.contains(QString::fromUtf8("已配置"))
        || text.contains(QString::fromUtf8("已映射"))
        || text.contains(QString::fromUtf8("已验证"))
        || text.contains(QString::fromUtf8("已分配"))
        || text.contains(QString::fromUtf8("完成"))
        || text.contains(QString::fromUtf8("就绪"))
        || text.contains(QString::fromUtf8("通过"))
        || text.contains(QString::fromUtf8("已同步"));
}

QPushButton *makeButton(const QString &text, bool primary, QWidget *parent)
{
    auto *btn = new QPushButton(text, parent);
    btn->setObjectName(primary ? QStringLiteral("PrimaryButton") : QStringLiteral("DefaultButton"));
    btn->setCursor(Qt::PointingHandCursor);
    return btn;
}

QComboBox *makeSelect(const QString &current, const QStringList &others, QWidget *parent)
{
    auto *box = new QComboBox(parent);
    box->addItem(current);
    if (others.isEmpty()) {
        box->addItem(QString::fromUtf8("项目默认"));
        box->addItem(QString::fromUtf8("自定义…"));
    } else {
        for (const QString &item : others) {
            if (item != current)
                box->addItem(item);
        }
    }
    box->setCurrentIndex(0);
    return box;
}

QLineEdit *makeInput(const QString &value, QWidget *parent)
{
    auto *edit = new QLineEdit(value, parent);
    return edit;
}

QWidget *makeLabeled(const QString &label, QWidget *control, QWidget *parent)
{
    auto *w = new QWidget(parent);
    auto *lay = new QVBoxLayout(w);
    lay->setContentsMargins(0, 0, 0, 0);
    lay->setSpacing(5);
    auto *lb = new QLabel(label);
    lb->setObjectName(QStringLiteral("FieldLabel"));
    lay->addWidget(lb);
    lay->addWidget(control);
    return w;
}

QWidget *makeField(const QString &label, const QString &value, const QString &unit, QWidget *parent)
{
    auto *edit = makeInput(value);
    if (unit.isEmpty())
        return makeLabeled(label, edit, parent);

    auto *row = new QWidget;
    auto *hl = new QHBoxLayout(row);
    hl->setContentsMargins(0, 0, 0, 0);
    hl->setSpacing(6);
    hl->addWidget(edit, 1);
    auto *unitLb = new QLabel(unit);
    unitLb->setObjectName(QStringLiteral("UnitLabel"));
    unitLb->setAlignment(Qt::AlignCenter);
    unitLb->setFixedWidth(68);
    unitLb->setFixedHeight(33);
    hl->addWidget(unitLb);
    return makeLabeled(label, row, parent);
}

QWidget *makeSelectField(const QString &label, const QString &current, const QStringList &others, QWidget *parent)
{
    return makeLabeled(label, makeSelect(current, others), parent);
}

QCheckBox *makeCheck(const QString &label, bool checked, QWidget *parent)
{
    auto *box = new QCheckBox(label, parent);
    box->setChecked(checked);
    return box;
}

QFrame *makePanel(QWidget *parent)
{
    auto *frame = new QFrame(parent);
    frame->setObjectName(QStringLiteral("Panel"));
    auto *lay = new QVBoxLayout(frame);
    lay->setContentsMargins(12, 12, 12, 12);
    lay->setSpacing(10);
    return frame;
}

QWidget *makePanelTitle(const QString &title, const QString &meta, const QString &subtitle, QWidget *parent)
{
    auto *w = new QWidget(parent);
    auto *hl = new QHBoxLayout(w);
    hl->setContentsMargins(0, 0, 0, 0);
    hl->setSpacing(8);

    auto *left = new QWidget;
    auto *vl = new QVBoxLayout(left);
    vl->setContentsMargins(0, 0, 0, 0);
    vl->setSpacing(2);
    auto *t = new QLabel(title);
    t->setObjectName(QStringLiteral("PanelTitle"));
    vl->addWidget(t);
    if (!subtitle.isEmpty()) {
        auto *s = new QLabel(subtitle);
        s->setObjectName(QStringLiteral("MutedLabel"));
        vl->addWidget(s);
    }
    hl->addWidget(left, 1);
    if (!meta.isEmpty()) {
        auto *m = new QLabel(meta);
        m->setObjectName(QStringLiteral("MetaLabel"));
        hl->addWidget(m, 0, Qt::AlignRight | Qt::AlignTop);
    }
    return w;
}

QWidget *makeKpis(const QVector<KpiItem> &items, QWidget *parent)
{
    auto *w = new QWidget(parent);
    auto *lay = new QHBoxLayout(w);
    lay->setContentsMargins(0, 0, 0, 0);
    lay->setSpacing(8);
    for (const KpiItem &item : items) {
        auto *card = new QFrame;
        card->setObjectName(QStringLiteral("KpiCard"));
        auto *vl = new QVBoxLayout(card);
        vl->setContentsMargins(12, 10, 12, 10);
        vl->setSpacing(5);
        auto *cap = new QLabel(item.label);
        cap->setObjectName(QStringLiteral("KpiCaption"));
        auto *valRow = new QWidget;
        auto *hl = new QHBoxLayout(valRow);
        hl->setContentsMargins(0, 0, 0, 0);
        hl->setSpacing(4);
        auto *val = new QLabel(item.value);
        val->setObjectName(QStringLiteral("KpiValue"));
        hl->addWidget(val);
        if (!item.unit.isEmpty()) {
            auto *u = new QLabel(item.unit);
            u->setObjectName(QStringLiteral("KpiUnit"));
            hl->addWidget(u);
        }
        hl->addStretch();
        vl->addWidget(cap);
        vl->addWidget(valRow);
        lay->addWidget(card, 1);
    }
    return w;
}

QWidget *makeSummary(const QVector<SummaryItem> &rows, QWidget *parent)
{
    auto *w = new QWidget(parent);
    auto *lay = new QVBoxLayout(w);
    lay->setContentsMargins(0, 0, 0, 0);
    lay->setSpacing(0);
    for (int i = 0; i < rows.size(); ++i) {
        auto *row = new QWidget;
        auto *hl = new QHBoxLayout(row);
        hl->setContentsMargins(0, 0, 0, 7);
        auto *k = new QLabel(rows[i].key);
        k->setObjectName(QStringLiteral("MutedLabel"));
        auto *v = new QLabel(rows[i].value);
        v->setStyleSheet(QStringLiteral("font-weight: 500;"));
        if (rows[i].warn)
            v->setObjectName(QStringLiteral("StatusWarn"));
        else if (rows[i].good)
            v->setObjectName(QStringLiteral("StatusGood"));
        hl->addWidget(k);
        hl->addStretch();
        hl->addWidget(v);
        lay->addWidget(row);
        if (i + 1 < rows.size()) {
            auto *line = new QFrame;
            line->setFrameShape(QFrame::HLine);
            line->setStyleSheet(QStringLiteral("color: #d5dfe4;"));
            lay->addWidget(line);
        }
    }
    return w;
}

QWidget *makeBulletList(const QStringList &items, QWidget *parent)
{
    auto *w = new QWidget(parent);
    auto *lay = new QVBoxLayout(w);
    lay->setContentsMargins(8, 0, 0, 0);
    lay->setSpacing(4);
    for (const QString &item : items) {
        auto *lb = new QLabel(QString::fromUtf8("•  ") + item);
        lb->setObjectName(QStringLiteral("MutedLabel"));
        lb->setWordWrap(true);
        lay->addWidget(lb);
    }
    return w;
}

QWidget *makeHeading(const QString &title, const QString &subtitle,
                     const QString &templateLabel, const QStringList &templateItems,
                     QWidget *parent)
{
    auto *w = new QWidget(parent);
    auto *hl = new QHBoxLayout(w);
    hl->setContentsMargins(0, 0, 0, 0);
    hl->setSpacing(18);

    auto *left = new QWidget;
    auto *vl = new QVBoxLayout(left);
    vl->setContentsMargins(0, 0, 0, 0);
    vl->setSpacing(4);
    auto *t = new QLabel(title);
    t->setObjectName(QStringLiteral("PageTitle"));
    auto *s = new QLabel(subtitle);
    s->setObjectName(QStringLiteral("PageSubtitle"));
    s->setWordWrap(true);
    vl->addWidget(t);
    vl->addWidget(s);
    hl->addWidget(left, 1);

    if (!templateLabel.isEmpty() && !templateItems.isEmpty()) {
        auto *tmpl = makeSelectField(templateLabel, templateItems.first(),
                                     templateItems.mid(1));
        tmpl->setMinimumWidth(220);
        tmpl->setMaximumWidth(280);
        hl->addWidget(tmpl, 0, Qt::AlignTop);
    }
    return w;
}

QWidget *makeMiniFields(const QList<QWidget *> &fields, int columns, QWidget *parent)
{
    auto *w = new QWidget(parent);
    auto *grid = new QGridLayout(w);
    grid->setContentsMargins(0, 0, 0, 0);
    grid->setHorizontalSpacing(10);
    grid->setVerticalSpacing(8);
    for (int i = 0; i < fields.size(); ++i)
        grid->addWidget(fields[i], i / columns, i % columns);
    return w;
}

QWidget *makeProgressRow(const QString &label, const QString &value, int percent, QWidget *parent)
{
    auto *w = new QWidget(parent);
    auto *vl = new QVBoxLayout(w);
    vl->setContentsMargins(0, 6, 0, 0);
    vl->setSpacing(4);
    auto *row = new QWidget;
    auto *hl = new QHBoxLayout(row);
    hl->setContentsMargins(0, 0, 0, 0);
    auto *l = new QLabel(label);
    l->setObjectName(QStringLiteral("MutedLabel"));
    l->setStyleSheet(QStringLiteral("font-size: 11px;"));
    auto *v = new QLabel(value);
    v->setStyleSheet(QStringLiteral("font-size: 11px; font-weight: 500;"));
    hl->addWidget(l);
    hl->addStretch();
    hl->addWidget(v);
    auto *bar = new QProgressBar;
    bar->setRange(0, 100);
    bar->setValue(percent);
    bar->setTextVisible(false);
    vl->addWidget(row);
    vl->addWidget(bar);
    return w;
}

QTableWidget *makeTable(const QStringList &headers, const QVector<QStringList> &rows,
                        const TableOptions &options, QWidget *parent)
{
    auto *table = new QTableWidget(rows.size(), headers.size(), parent);
    table->setHorizontalHeaderLabels(headers);
    table->verticalHeader()->setVisible(false);
    table->setShowGrid(false);
    table->setSelectionBehavior(QAbstractItemView::SelectRows);
    table->setSelectionMode(QAbstractItemView::SingleSelection);
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    table->setFocusPolicy(Qt::NoFocus);
    table->setAlternatingRowColors(false);
    table->horizontalHeader()->setStretchLastSection(true);
    table->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    table->verticalHeader()->setDefaultSectionSize(32);
    table->setMinimumHeight(qMax(80, 40 + rows.size() * 32));

    for (int r = 0; r < rows.size(); ++r) {
        const QStringList &row = rows[r];
        for (int c = 0; c < headers.size() && c < row.size(); ++c) {
            if (options.firstColumnCheck && c == 0) {
                auto *cb = new QCheckBox;
                cb->setChecked(options.firstChecksChecked);
                auto *wrap = new QWidget;
                auto *hl = new QHBoxLayout(wrap);
                hl->setContentsMargins(8, 0, 0, 0);
                hl->addWidget(cb);
                hl->addStretch();
                table->setCellWidget(r, c, wrap);
                continue;
            }
            if (options.chipColumns.contains(c)) {
                auto *chip = makeChip(row[c], r > 2);
                auto *wrap = new QWidget;
                auto *hl = new QHBoxLayout(wrap);
                hl->setContentsMargins(6, 0, 6, 0);
                hl->addWidget(chip);
                hl->addStretch();
                table->setCellWidget(r, c, wrap);
                continue;
            }
            auto *item = new QTableWidgetItem(row[c]);
            if (c == 0 || (headers.size() > 1 && c == 1 && headers[0] == QString::fromUtf8("启用")))
                item->setFont([&] {
                    QFont f = item->font();
                    f.setBold(true);
                    return f;
                }());
            if (c == headers.size() - 1 || options.warnRows.contains(r))
                applyStatusColor(item);
            table->setItem(r, c, item);
        }
    }
    return table;
}

QScrollArea *wrapScroll(QWidget *content, QWidget *parent)
{
    auto *sa = new QScrollArea(parent);
    sa->setWidgetResizable(true);
    sa->setFrameShape(QFrame::NoFrame);
    sa->setWidget(content);
    return sa;
}

QWidget *makeStatusText(const QString &text, bool warn, QWidget *parent)
{
    auto *lb = new QLabel(text, parent);
    lb->setObjectName(warn ? QStringLiteral("StatusWarn") : QStringLiteral("StatusGood"));
    return lb;
}

QLabel *makeChip(const QString &text, bool neutral, QWidget *parent)
{
    auto *lb = new QLabel(text, parent);
    lb->setObjectName(neutral ? QStringLiteral("ChipNeutral") : QStringLiteral("Chip"));
    lb->setAlignment(Qt::AlignCenter);
    return lb;
}

QFrame *makeFlowNode(const QString &title, const QString &subtitle, const QString &state, QWidget *parent)
{
    auto *frame = new QFrame(parent);
    if (state == QLatin1String("active"))
        frame->setObjectName(QStringLiteral("FlowNodeActive"));
    else if (state == QLatin1String("control"))
        frame->setObjectName(QStringLiteral("FlowNodeControl"));
    else if (state == QLatin1String("done"))
        frame->setObjectName(QStringLiteral("FlowNodeDone"));
    else if (state == QLatin1String("running"))
        frame->setObjectName(QStringLiteral("FlowNodeRunning"));
    else if (state == QLatin1String("pending"))
        frame->setObjectName(QStringLiteral("FlowNodePending"));
    else
        frame->setObjectName(QStringLiteral("FlowNode"));
    frame->setMinimumWidth(125);
    auto *vl = new QVBoxLayout(frame);
    vl->setContentsMargins(12, 10, 12, 10);
    vl->setSpacing(3);
    auto *t = new QLabel(title);
    t->setStyleSheet(QStringLiteral("font-size: 12px; font-weight: 500;"));
    t->setAlignment(Qt::AlignCenter);
    auto *s = new QLabel(subtitle);
    s->setObjectName(QStringLiteral("MutedLabel"));
    s->setStyleSheet(QStringLiteral("font-size: 11px;"));
    s->setAlignment(Qt::AlignCenter);
    vl->addWidget(t);
    vl->addWidget(s);
    return frame;
}

QLabel *makeFlowArrow(const QString &text, QWidget *parent)
{
    auto *lb = new QLabel(text, parent);
    lb->setStyleSheet(QStringLiteral("color: #0c9b88; font-size: 18px;"));
    lb->setAlignment(Qt::AlignCenter);
    return lb;
}

void wireDummyAction(QAbstractButton *button, QWidget *dialogParent)
{
    QObject::connect(button, &QAbstractButton::clicked, dialogParent, [button, dialogParent]() {
        QMessageBox::information(dialogParent,
                                 QString::fromUtf8("飞机概念设计平台"),
                                 button->text() + QString::fromUtf8(" — 原型交互已记录。"));
    });
}

SubTabBar::SubTabBar(const QVector<QPair<QString, QString>> &items,
                     const QString &activeId,
                     QWidget *parent)
    : QFrame(parent)
    , m_currentId(activeId)
{
    setObjectName(QStringLiteral("SubTabBar"));
    setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Fixed);
    auto *lay = new QHBoxLayout(this);
    lay->setContentsMargins(4, 4, 4, 4);
    lay->setSpacing(4);
    auto *group = new QButtonGroup(this);
    group->setExclusive(true);
    for (const auto &item : items) {
        auto *btn = new QPushButton(item.second);
        btn->setObjectName(QStringLiteral("SubTabButton"));
        btn->setCheckable(true);
        btn->setCursor(Qt::PointingHandCursor);
        btn->setProperty("tabId", item.first);
        btn->setChecked(item.first == activeId);
        group->addButton(btn);
        lay->addWidget(btn);
        connect(btn, &QPushButton::clicked, this, [this, item]() {
            if (m_currentId == item.first)
                return;
            m_currentId = item.first;
            emit currentChanged(item.first);
        });
    }
}

void SubTabBar::setActive(const QString &id)
{
    m_currentId = id;
    const auto buttons = findChildren<QPushButton *>();
    for (auto *btn : buttons)
        btn->setChecked(btn->property("tabId").toString() == id);
}

QString SubTabBar::currentId() const
{
    return m_currentId;
}

CandidateRow::CandidateRow(const QString &rank, const QString &name, const QString &desc,
                           const QString &score, QWidget *parent)
    : QFrame(parent)
{
    setStyleSheet(QStringLiteral("CandidateRow { border-bottom: 1px solid #d5dfe4; }"));
    auto *hl = new QHBoxLayout(this);
    hl->setContentsMargins(0, 8, 0, 8);
    hl->setSpacing(8);
    auto *badge = new QLabel(rank);
    badge->setObjectName(QStringLiteral("RankBadge"));
    badge->setFixedSize(26, 26);
    badge->setAlignment(Qt::AlignCenter);
    auto *mid = new QWidget;
    auto *vl = new QVBoxLayout(mid);
    vl->setContentsMargins(0, 0, 0, 0);
    vl->setSpacing(2);
    auto *n = new QLabel(name);
    n->setStyleSheet(QStringLiteral("font-size: 12px; font-weight: 500;"));
    auto *d = new QLabel(desc);
    d->setObjectName(QStringLiteral("MutedLabel"));
    d->setStyleSheet(QStringLiteral("font-size: 11px;"));
    vl->addWidget(n);
    vl->addWidget(d);
    auto *sc = new QLabel(score);
    sc->setStyleSheet(QStringLiteral("font-weight: 500;"));
    hl->addWidget(badge);
    hl->addWidget(mid, 1);
    hl->addWidget(sc);
}
