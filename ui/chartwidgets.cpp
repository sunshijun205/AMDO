#include "chartwidgets.h"
#include "theme.h"

#include <QPainter>
#include <QPainterPath>
#include <QtMath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

ScatterChart::ScatterChart(QWidget *parent)
    : QWidget(parent)
{
    setMinimumHeight(220);
}

void ScatterChart::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    const int l = 46, t = 20, r = width() - 26, b = height() - 35;
    p.setPen(QPen(Theme::muted(), 1));
    p.drawLine(l, b, r, b);
    p.drawLine(l, t, l, b);
    p.setPen(QPen(Theme::line(), 1));
    for (int i = 1; i <= 3; ++i) {
        const int y = b - (b - t) * i / 4;
        p.drawLine(l, y, r, y);
    }
    const QPointF pts[] = {
        {62, 165}, {89, 138}, {111, 178}, {136, 123}, {158, 153},
        {181, 92}, {207, 130}, {229, 72}, {254, 109}, {280, 58},
        {307, 83}, {334, 45}, {354, 118}, {379, 68}, {406, 36},
        {432, 91}, {458, 52}, {488, 28}, {518, 77}, {546, 44}
    };
    const qreal sx = (r - l) / 560.0;
    const qreal sy = (b - t) / 210.0;
    for (int i = 0; i < 20; ++i) {
        p.setBrush(i % 4 == 0 ? Theme::blue() : Theme::accent());
        p.setPen(Qt::NoPen);
        p.setOpacity(0.75);
        p.drawEllipse(QPointF(l + (pts[i].x() - 40) * sx, t + pts[i].y() * sy * 0.85), 4, 4);
    }
    p.setOpacity(1);
    p.setPen(Theme::muted());
    p.setFont(QFont(QStringLiteral("Microsoft YaHei"), 9));
    p.drawText(QRect(l, height() - 22, r - l, 18), Qt::AlignHCenter,
               QString::fromUtf8("翼载荷 W/S"));
    p.save();
    p.translate(14, (t + b) / 2);
    p.rotate(-90);
    p.drawText(QRect(-40, -10, 80, 20), Qt::AlignCenter,
               QString::fromUtf8("推重比 T/W"));
    p.restore();
}

RadarChart::RadarChart(QWidget *parent)
    : QWidget(parent)
{
    setMinimumHeight(220);
}

void RadarChart::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    const QPointF c(width() / 2.0, height() / 2.0 + 4);
    const qreal R = qMin(width(), height()) * 0.38;
    const QStringList labels = {
        QString::fromUtf8("气动效率"),
        QString::fromUtf8("结构裕度"),
        QString::fromUtf8("重量"),
        QString::fromUtf8("推进效率"),
        QString::fromUtf8("操稳品质"),
        QString::fromUtf8("任务性能")
    };
    auto pt = [&](int i, qreal scale) {
        const qreal a = -M_PI / 2 + i * M_PI / 3;
        return QPointF(c.x() + R * scale * qCos(a), c.y() + R * scale * qSin(a));
    };
    p.setPen(QPen(Theme::line(), 1));
    p.setBrush(Qt::NoBrush);
    for (qreal s : {1.0, 0.66, 0.33}) {
        QPolygonF poly;
        for (int i = 0; i < 6; ++i)
            poly << pt(i, s);
        p.drawPolygon(poly);
    }
    for (int i = 0; i < 6; ++i)
        p.drawLine(c, pt(i, 1.0));

    const qreal data[6] = {0.86, 0.78, 0.70, 0.82, 0.74, 0.88};
    QPainterPath fill;
    QPolygonF dataPoly;
    for (int i = 0; i < 6; ++i)
        dataPoly << pt(i, data[i]);
    fill.addPolygon(dataPoly);
    fill.closeSubpath();
    p.setBrush(Theme::accentSoft());
    p.setPen(QPen(Theme::accent(), 1.5));
    p.drawPath(fill);

    p.setPen(Theme::muted());
    p.setFont(QFont(QStringLiteral("Microsoft YaHei"), 8));
    for (int i = 0; i < 6; ++i) {
        const QPointF lp = pt(i, 1.22);
        p.drawText(QRectF(lp.x() - 36, lp.y() - 8, 72, 16), Qt::AlignCenter, labels[i]);
    }
}

ParallelChart::ParallelChart(QWidget *parent)
    : QWidget(parent)
{
    setMinimumHeight(220);
}

void ParallelChart::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    const int n = 6;
    const int top = 25, bot = height() - 40;
    QVector<int> xs;
    for (int i = 0; i < n; ++i)
        xs << 40 + i * (width() - 80) / (n - 1);
    p.setPen(QPen(Theme::muted(), 1));
    for (int x : xs)
        p.drawLine(x, top, x, bot);

    const qreal lines[4][6] = {
        {0.22, 0.38, 0.30, 0.52, 0.26, 0.42},
        {0.40, 0.24, 0.48, 0.28, 0.44, 0.24},
        {0.55, 0.46, 0.22, 0.40, 0.32, 0.54},
        {0.32, 0.56, 0.40, 0.18, 0.54, 0.36}
    };
    for (int k = 0; k < 4; ++k) {
        QPolygonF poly;
        for (int i = 0; i < n; ++i)
            poly << QPointF(xs[i], top + (bot - top) * lines[k][i]);
        p.setPen(QPen(k == 0 ? Theme::accent() : Theme::blue(), k == 0 ? 2.0 : 1.5));
        p.setOpacity(k == 0 ? 1.0 : 0.75);
        p.drawPolyline(poly);
    }
    p.setOpacity(1);
    p.setPen(Theme::muted());
    p.setFont(QFont(QStringLiteral("Microsoft YaHei"), 8));
    const QStringList names = {
        QString::fromUtf8("任务燃油"), QStringLiteral("MTOW"),
        QString::fromUtf8("起飞场长"), QString::fromUtf8("结构裕度"),
        QStringLiteral("L/D"), QString::fromUtf8("操稳评分")
    };
    for (int i = 0; i < n; ++i)
        p.drawText(QRect(xs[i] - 40, height() - 28, 80, 20), Qt::AlignHCenter, names[i]);
}

EnvelopeChart::EnvelopeChart(QWidget *parent)
    : QWidget(parent)
{
    setMinimumHeight(220);
}

void EnvelopeChart::setPoints(const QVector<QPointF> &machAltitudeKm)
{
    m_points = machAltitudeKm;
    update();
}

void EnvelopeChart::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    const int l = 54, t = 20, r = width() - 32, b = height() - 36;
    p.setPen(QPen(Theme::muted(), 1));
    p.drawLine(l, b, r, b);
    p.drawLine(l, t, l, b);
    p.setPen(QPen(Theme::line(), 1));
    for (int i = 1; i <= 3; ++i)
        p.drawLine(l, b - (b - t) * i / 4, r, b - (b - t) * i / 4);

    QVector<QPointF> pts = m_points;
    if (pts.isEmpty()) {
        p.setPen(Theme::muted());
        p.setFont(QFont(QStringLiteral("Microsoft YaHei"), 9));
        p.drawText(QRect(l, height() - 22, r - l, 18), Qt::AlignHCenter,
                   QString::fromUtf8("马赫数 Ma"));
        p.save();
        p.translate(16, (t + b) / 2);
        p.rotate(-90);
        p.drawText(QRect(-40, -10, 80, 20), Qt::AlignCenter, QString::fromUtf8("高度 km"));
        p.restore();
        return;
    }

    qreal minMa = pts[0].x(), maxMa = pts[0].x();
    qreal minH = pts[0].y(), maxH = pts[0].y();
    for (int i = 1; i < pts.size(); ++i) {
        minMa = qMin(minMa, pts[i].x());
        maxMa = qMax(maxMa, pts[i].x());
        minH = qMin(minH, pts[i].y());
        maxH = qMax(maxH, pts[i].y());
    }
    if (maxMa - minMa < 1e-6) {
        minMa -= 0.1;
        maxMa += 0.1;
    }
    if (maxH - minH < 1e-6) {
        minH = 0;
        maxH = 12;
    }
    minMa = qMax(0.0, minMa - 0.05);
    maxMa += 0.05;
    minH = qMax(0.0, minH - 0.5);
    maxH += 0.5;

    auto map = [&](qreal ma, qreal h) {
        return QPointF(l + (ma - minMa) / (maxMa - minMa) * (r - l),
                       b - (h - minH) / (maxH - minH) * (b - t));
    };

    QPainterPath path;
    path.moveTo(map(pts[0].x(), pts[0].y()));
    for (int i = 1; i < pts.size(); ++i)
        path.lineTo(map(pts[i].x(), pts[i].y()));
    path.closeSubpath();
    p.setBrush(Theme::accentSoft());
    p.setPen(QPen(Theme::accent(), 1.5));
    p.drawPath(path);
    p.setPen(Qt::NoPen);
    for (int i = 0; i < pts.size(); ++i) {
        p.setBrush(i == 0 ? Theme::blue() : Theme::accent());
        p.drawEllipse(map(pts[i].x(), pts[i].y()), 5, 5);
    }

    p.setPen(Theme::muted());
    p.setFont(QFont(QStringLiteral("Microsoft YaHei"), 9));
    p.drawText(QRect(l, height() - 22, r - l, 18), Qt::AlignHCenter,
               QString::fromUtf8("马赫数 Ma"));
    p.save();
    p.translate(16, (t + b) / 2);
    p.rotate(-90);
    p.drawText(QRect(-40, -10, 80, 20), Qt::AlignCenter, QString::fromUtf8("高度 km"));
    p.restore();
}

AircraftTopView::AircraftTopView(QWidget *parent)
    : QWidget(parent)
{
    setMinimumHeight(330);
}

void AircraftTopView::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    p.fillRect(rect(), Theme::panel2());
    const qreal cx = width() / 2.0;
    const qreal cy = height() / 2.0;
    const qreal s = qMin(width() / 620.0, height() / 330.0);

    p.setPen(QPen(Theme::muted(), 1, Qt::DashLine));
    p.drawLine(QPointF(cx, 16 * s), QPointF(cx, height() - 16 * s));
    p.drawLine(QPointF(36 * s, cy), QPointF(width() - 36 * s, cy));

    QPainterPath air;
    air.moveTo(cx, cy - 147 * s);
    air.cubicTo(cx + 14 * s, cy - 120 * s, cx + 18 * s, cy - 59 * s, cx + 16 * s, cy - 29 * s);
    air.lineTo(cx + 238 * s, cy + 24 * s);
    air.lineTo(cx + 238 * s, cy + 40 * s);
    air.lineTo(cx + 16 * s, cy + 16 * s);
    air.lineTo(cx + 10 * s, cy + 105 * s);
    air.lineTo(cx + 80 * s, cy + 135 * s);
    air.lineTo(cx + 78 * s, cy + 147 * s);
    air.lineTo(cx, cy + 129 * s);
    air.lineTo(cx - 78 * s, cy + 147 * s);
    air.lineTo(cx - 80 * s, cy + 135 * s);
    air.lineTo(cx - 10 * s, cy + 105 * s);
    air.lineTo(cx - 16 * s, cy + 16 * s);
    air.lineTo(cx - 238 * s, cy + 40 * s);
    air.lineTo(cx - 238 * s, cy + 24 * s);
    air.lineTo(cx - 16 * s, cy - 29 * s);
    air.cubicTo(cx - 18 * s, cy - 59 * s, cx - 14 * s, cy - 120 * s, cx, cy - 147 * s);
    p.setBrush(Theme::accentSoft());
    p.setPen(QPen(Theme::accent(), 2));
    p.drawPath(air);

    p.setPen(QPen(Theme::muted(), 1));
    p.drawLine(QPointF(cx - 238 * s, cy + 59 * s), QPointF(cx + 238 * s, cy + 59 * s));
    p.setFont(QFont(QStringLiteral("Microsoft YaHei"), 10));
    p.drawText(QRectF(cx - 80, cy + 64 * s, 160, 18), Qt::AlignCenter,
               QString::fromUtf8("翼展 b = 34.15 m"));
    p.drawLine(QPointF(cx + 42 * s, cy - 147 * s), QPointF(cx + 42 * s, cy + 147 * s));
    p.drawText(QRectF(cx + 48 * s, cy - 10, 140, 18), Qt::AlignLeft | Qt::AlignVCenter,
               QString::fromUtf8("机长 L = 38.20 m"));
}

AircraftIsoView::AircraftIsoView(QWidget *parent)
    : QWidget(parent)
{
    setMinimumHeight(330);
}

void AircraftIsoView::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    p.fillRect(rect(), Theme::panel2());
    const qreal s = qMin(width() / 760.0, height() / 360.0);
    const qreal ox = width() / 2.0 - 380 * s;
    const qreal oy = height() / 2.0 - 180 * s;

    auto P = [&](qreal x, qreal y) { return QPointF(ox + x * s, oy + y * s); };

    p.setPen(QPen(Theme::muted(), 1, Qt::DashLine));
    p.drawLine(P(70, 292), P(690, 292));
    p.drawLine(P(380, 34), P(380, 324));

    QPainterPath air;
    air.moveTo(P(140, 220));
    air.lineTo(P(337, 185));
    air.lineTo(P(350, 82));
    air.quadTo(P(380, 26), P(410, 82));
    air.lineTo(P(423, 185));
    air.lineTo(P(620, 220));
    air.lineTo(P(612, 242));
    air.lineTo(P(422, 226));
    air.lineTo(P(413, 292));
    air.lineTo(P(493, 320));
    air.lineTo(P(486, 334));
    air.lineTo(P(380, 314));
    air.lineTo(P(274, 334));
    air.lineTo(P(267, 320));
    air.lineTo(P(347, 292));
    air.lineTo(P(338, 226));
    air.lineTo(P(148, 242));
    air.closeSubpath();
    p.setBrush(Theme::accentSoft());
    p.setPen(QPen(Theme::accent(), 2));
    p.drawPath(air);
    p.drawEllipse(P(278, 238), 31 * s, 16 * s);
    p.drawEllipse(P(500, 222), 31 * s, 16 * s);

    p.setPen(Theme::muted());
    p.setFont(QFont(QStringLiteral("Microsoft YaHei"), 10));
    p.drawText(P(515, 200), QString::fromUtf8("右发动机"));
    p.drawText(P(392, 70), QString::fromUtf8("机身"));
    p.drawText(P(120, 210), QString::fromUtf8("左翼"));
    p.drawText(P(448, 326), QString::fromUtf8("尾翼"));
}

MissionRail::MissionRail(const QStringList &segments, QWidget *parent)
    : QWidget(parent)
    , m_segments(segments)
{
    setMinimumHeight(64);
}

void MissionRail::setSegments(const QStringList &segments)
{
    m_segments = segments;
    update();
}

void MissionRail::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    if (m_segments.isEmpty())
        return;
    const int n = m_segments.size();
    const qreal step = width() / qreal(n);
    const qreal y = 14;
    p.setPen(QPen(Theme::line(), 3));
    p.drawLine(QPointF(step / 2, y), QPointF(width() - step / 2, y));
    p.setFont(QFont(QStringLiteral("Microsoft YaHei"), 9));
    for (int i = 0; i < n; ++i) {
        const qreal x = step * i + step / 2;
        p.setBrush(Theme::panel());
        p.setPen(QPen(Theme::accent(), 3));
        p.drawEllipse(QPointF(x, y), 7, 7);
        p.setPen(Theme::muted());
        p.drawText(QRectF(x - step / 2 + 2, 28, step - 4, 28), Qt::AlignHCenter | Qt::AlignTop, m_segments[i]);
    }
}
