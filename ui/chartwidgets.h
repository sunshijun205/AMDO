#ifndef CHARTWIDGETS_H
#define CHARTWIDGETS_H

#include <QWidget>
#include <QVector>
#include <QPointF>
#include <QStringList>

class ScatterChart : public QWidget
{
    Q_OBJECT
public:
    explicit ScatterChart(QWidget *parent = nullptr);
    QSize sizeHint() const override { return QSize(600, 230); }
protected:
    void paintEvent(QPaintEvent *event) override;
};

class RadarChart : public QWidget
{
    Q_OBJECT
public:
    explicit RadarChart(QWidget *parent = nullptr);
    QSize sizeHint() const override { return QSize(420, 230); }
protected:
    void paintEvent(QPaintEvent *event) override;
};

class ParallelChart : public QWidget
{
    Q_OBJECT
public:
    explicit ParallelChart(QWidget *parent = nullptr);
    QSize sizeHint() const override { return QSize(580, 230); }
protected:
    void paintEvent(QPaintEvent *event) override;
};

class EnvelopeChart : public QWidget
{
    Q_OBJECT
public:
    explicit EnvelopeChart(QWidget *parent = nullptr);
    QSize sizeHint() const override { return QSize(600, 230); }
protected:
    void paintEvent(QPaintEvent *event) override;
};

class AircraftTopView : public QWidget
{
    Q_OBJECT
public:
    explicit AircraftTopView(QWidget *parent = nullptr);
    QSize sizeHint() const override { return QSize(620, 330); }
protected:
    void paintEvent(QPaintEvent *event) override;
};

class AircraftIsoView : public QWidget
{
    Q_OBJECT
public:
    explicit AircraftIsoView(QWidget *parent = nullptr);
    QSize sizeHint() const override { return QSize(760, 360); }
protected:
    void paintEvent(QPaintEvent *event) override;
};

class MissionRail : public QWidget
{
    Q_OBJECT
public:
    explicit MissionRail(const QStringList &segments, QWidget *parent = nullptr);
    QSize sizeHint() const override { return QSize(700, 70); }
protected:
    void paintEvent(QPaintEvent *event) override;
private:
    QStringList m_segments;
};

#endif
