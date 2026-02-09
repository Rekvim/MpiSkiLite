#ifndef LABELEDSLIDER_H
#define LABELEDSLIDER_H

#pragma once
#include <QSlider>
#include <QMap>

class LabeledSlider : public QSlider
{
    Q_OBJECT
public:
    explicit LabeledSlider(QWidget* parent = nullptr);

    void setTickLabels(const QMap<int, QString>& labels);
    void setLabelOffset(int px);
    void setTickLength(int px);
    void setTickGap(int px);

    void setLabelPointSize(int pt);     // ✅ размер шрифта подписей
    int  labelPointSize() const { return m_labelPointSize; }

    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

protected:
    void paintEvent(QPaintEvent* e) override;
    bool event(QEvent* e) override;

private:
    QFont labelFont() const;            // ✅ единый шрифт подписей
    int labelsMaxTextWidth() const;
    int leftAreaWidth() const;
    void updateWidthConstraints();

private:
    QMap<int, QString> m_labels;

    int m_labelOffset    = 6;
    int m_tickLen        = 10;
    int m_tickGap        = 4;
    int m_labelPointSize = 10;          // ✅ по умолчанию 10
};

#endif // LABELEDSLIDER_H
