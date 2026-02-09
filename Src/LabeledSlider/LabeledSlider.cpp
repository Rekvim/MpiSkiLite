#include "LabeledSlider.h"

#include <QStylePainter>
#include <QStyleOptionSlider>
#include <QFontMetrics>
#include <QEvent>
#include <algorithm>

LabeledSlider::LabeledSlider(QWidget* parent)
    : QSlider(parent)
{
    setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Expanding);
}

void LabeledSlider::setTickLabels(const QMap<int, QString>& labels)
{
    m_labels = labels;
    updateWidthConstraints();
    updateGeometry();
    update();
}

void LabeledSlider::setLabelOffset(int px)
{
    m_labelOffset = std::max(0, px);
    updateWidthConstraints();
    updateGeometry();
    update();
}

void LabeledSlider::setTickLength(int px)
{
    m_tickLen = std::max(0, px);
    updateWidthConstraints();
    updateGeometry();
    update();
}

void LabeledSlider::setTickGap(int px)
{
    m_tickGap = std::max(0, px);
    updateWidthConstraints();
    updateGeometry();
    update();
}

void LabeledSlider::setLabelPointSize(int pt)
{
    m_labelPointSize = std::max(1, pt);
    updateWidthConstraints();
    updateGeometry();
    update();
}

QFont LabeledSlider::labelFont() const
{
    QFont f = font();
    f.setPointSize(m_labelPointSize);
    return f;
}

int LabeledSlider::labelsMaxTextWidth() const
{
    if (m_labels.isEmpty()) return 0;

    QFontMetrics fm(labelFont());  // ✅ тот же шрифт, что и в paint
    int w = 0;
    for (auto it = m_labels.cbegin(); it != m_labels.cend(); ++it)
        w = std::max(w, fm.horizontalAdvance(it.value()));
    return w;
}

int LabeledSlider::leftAreaWidth() const
{
    if (m_labels.isEmpty()) return 0;
    return labelsMaxTextWidth() + m_labelOffset + m_tickLen + m_tickGap + 2;
}

void LabeledSlider::updateWidthConstraints()
{
    if (orientation() != Qt::Vertical) return;

    const int sliderW = QSlider::sizeHint().width();
    const int minW = sliderW + leftAreaWidth();
    setMinimumWidth(minW);
}

QSize LabeledSlider::sizeHint() const
{
    QSize base = QSlider::sizeHint();

    if (orientation() == Qt::Vertical) {
        base.rwidth() += leftAreaWidth();
    } else {
        QFontMetrics fm(labelFont());
        base.rheight() += fm.height() + m_labelOffset + m_tickLen + 2;
    }
    return base;
}

QSize LabeledSlider::minimumSizeHint() const
{
    QSize base = QSlider::minimumSizeHint();

    if (orientation() == Qt::Vertical) {
        base.rwidth() += leftAreaWidth();
    } else {
        QFontMetrics fm(labelFont());
        base.rheight() += fm.height() + m_labelOffset + m_tickLen + 2;
    }
    return base;
}

bool LabeledSlider::event(QEvent* e)
{
    switch (e->type()) {
    case QEvent::Polish:
    case QEvent::StyleChange:
    case QEvent::FontChange:
        updateWidthConstraints();
        break;
    default:
        break;
    }
    return QSlider::event(e);
}

void LabeledSlider::paintEvent(QPaintEvent*)
{
    QStylePainter p(this);
    p.setRenderHint(QPainter::TextAntialiasing, true);

    QStyleOptionSlider opt;
    initStyleOption(&opt);

    opt.subControls = QStyle::SC_SliderGroove | QStyle::SC_SliderHandle;
    if (tickPosition() != QSlider::NoTicks)
        opt.subControls |= QStyle::SC_SliderTickmarks;

    // --- выделяем место слева под подписи (vertical) ---
    QRect sliderRect = rect();
    if (orientation() == Qt::Vertical && !m_labels.isEmpty()) {
        const int minSliderW = QSlider::sizeHint().width();
        int left = leftAreaWidth();
        if (sliderRect.width() < left + minSliderW)
            left = std::max(0, sliderRect.width() - minSliderW);
        sliderRect.adjust(left, 0, 0, 0);
    }

    opt.rect = sliderRect;
    p.drawComplexControl(QStyle::CC_Slider, opt);

    if (m_labels.isEmpty())
        return;

    const QRect groove = style()->subControlRect(QStyle::CC_Slider, &opt,
                                                 QStyle::SC_SliderGroove, this);

    const QFont lf = labelFont();
    p.setFont(lf);
    const QFontMetrics fm(lf);

    if (orientation() == Qt::Vertical) {
        for (auto it = m_labels.cbegin(); it != m_labels.cend(); ++it) {
            const int v = it.key();
            if (v < minimum() || v > maximum()) continue;

            QStyleOptionSlider o = opt;
            o.sliderPosition = v;

            const QRect handle = style()->subControlRect(QStyle::CC_Slider, &o,
                                                         QStyle::SC_SliderHandle, this);
            const int y = handle.center().y();

            // tick
            const int xTick0 = groove.left() - m_tickGap;
            const int xTick1 = xTick0 - m_tickLen;

            QPen tickPen(palette().color(QPalette::Mid));
            tickPen.setWidth(1);
            p.setPen(tickPen);
            p.drawLine(QPoint(xTick0, y), QPoint(xTick1, y));

            // text
            p.setPen(palette().color(QPalette::WindowText));

            // максимум доступной ширины для текста — до левого края виджета
            const int maxTextW = std::max(0, xTick1 - m_labelOffset);
            const QString text = fm.elidedText(it.value(), Qt::ElideLeft, maxTextW);

            const int textW = fm.horizontalAdvance(text);
            const int textH = fm.height();

            const int xText = xTick1 - m_labelOffset - textW;
            const int yText = y - textH / 2;

            QRect r(xText, yText, textW, textH);
            r = r.intersected(rect());
            if (r.isValid())
                p.drawText(r, Qt::AlignLeft | Qt::AlignVCenter, text);
        }
    }
}
