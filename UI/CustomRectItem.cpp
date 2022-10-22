#include <QDebug>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QFontMetrics>
#include "CustomRectItem.h"

CustomRectItem::CustomRectItem(qreal x, qreal y, qreal width, qreal height, QGraphicsItem *parent) : super(x, y, width, height, parent) {
    setDefaults();
}
CustomRectItem::CustomRectItem(const QRectF &rect, QGraphicsItem *parent) : super(rect, parent) {
    setDefaults();
}
CustomRectItem::CustomRectItem(QGraphicsItem *parent) : super(parent) {
    setDefaults();
}
void CustomRectItem::setDefaults() {
    m_labelItem = new QGraphicsTextItem();
    m_labelItem->setFlag(QGraphicsItem::ItemIgnoresTransformations);
    m_labelItem->setFont(QFont(m_textFontLabel, m_textSizeLabel));
    m_labelItem->setDefaultTextColor(m_textColorLabel);
    m_labelItem->setPos( 0, -labelFontHeight() * 1.25 );
    m_labelItem->setHtml(m_textLabel);
    m_labelItem->setParentItem(this);
    setFlag(QGraphicsItem::ItemSendsGeometryChanges);

    QPen comseticPen(Qt::black);
    comseticPen.setCosmetic(true);
    setPen( comseticPen );

    updateBackground();
}

void CustomRectItem::setTextLabel(const QString& text) {
    m_textLabel = text;
    m_labelItem->setHtml(text);
}
void CustomRectItem::setTextMain(const QString& text) {
    m_textMain = text;
}
void CustomRectItem::setTextSecondary(const QString& text) {
    m_textSecondary = text;
}
void CustomRectItem::setTextParamsLabel(const QColor color, int fontSize, QString fontName, int alignFlags) {
    m_textColorLabel = color;
    m_textSizeLabel = fontSize;
    m_textFontLabel = fontName;
    m_textLabelAlignFlags = alignFlags;
    m_labelItem->setFont(QFont(m_textFontLabel, m_textSizeLabel));
    m_labelItem->setDefaultTextColor(m_textColorLabel);
}
void CustomRectItem::setTextParamsMain(const QColor color, int fontSize, QString fontName, int alignFlags) {
    m_textColorMain = color;
    m_textSizeMain = fontSize;
    m_textFontMain = fontName;
    m_textMainAlignFlags = alignFlags;
}
void CustomRectItem::setTextParamsSecondary(const QColor color, int fontSize, QString fontName, int alignFlags) {
    m_textColorSecondary = color;
    m_textSizeSecondary = fontSize;
    m_textFontSecondary = fontName;
    m_textSecondaryAlignFlags = alignFlags;
}

void CustomRectItem::setBackgroundColor(const QColor color) {
    m_backgroundColor = color;
    updateBackground();
}

void CustomRectItem::setDuration(double dur) {
    m_duration = dur;
    updateBackground();
}

void CustomRectItem::setBlendIn(double blendIn) {
    m_blendIn = blendIn;
    updateBackground();
}

void CustomRectItem::setBlendOut(double blendOut) {
    m_blendOut = blendOut;
    updateBackground();
}

void CustomRectItem::setShotAction(shotAction* action) {
    m_shotAction = action;
}


void CustomRectItem::updateBackground() {
    QLinearGradient gradient(this->rect().topLeft(), this->rect().bottomRight());

    if (m_duration > 0 && m_blendIn > 0) {
        double blendInRatio = m_blendIn / m_duration;
        gradient.setColorAt(0.0, QColorConstants::Transparent);
        gradient.setColorAt(blendInRatio, m_backgroundColor);
    } else {
        gradient.setColorAt(0.0, m_backgroundColor);
    }

    if (m_duration > 0 && m_blendOut > 0) {
        double blendOutRatio = (m_duration - m_blendOut) / m_duration;
        gradient.setColorAt(blendOutRatio, m_backgroundColor);
        gradient.setColorAt(1.0, QColorConstants::Transparent);
    } else {
        gradient.setColorAt(1.0, m_backgroundColor);
    }
    this->setBrush( QBrush(gradient) );
}

void CustomRectItem::setBordersRect(const QRectF &newBordersRect)
{
    m_bordersRect = newBordersRect;
}

void CustomRectItem::setButtonImages(const QImage &newButtonImageEnabled, const QImage &newButtonImageDisabled)
{
    m_buttonImageEnabled = newButtonImageEnabled;
    m_buttonImageDisabled = newButtonImageDisabled;
}

QVariant CustomRectItem::data(const QString &key)
{
    return m_data.value(key);
}

void CustomRectItem::setData(const QString &newKey, const QVariant &newValue)
{
    m_data[newKey] = newValue;
}

int CustomRectItem::labelFontHeight() const {
    QFont fontLabel(m_textFontLabel, m_textSizeLabel);
    return QFontMetrics(fontLabel).height();
}

void CustomRectItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) {
    super::paint(painter, option, widget);
    bool drawBoth = !m_textMain.isEmpty() && !m_textSecondary.isEmpty();

    painter->save();
    double painterScale = painter->transform().m11();
    double scaleValue = scale() / painterScale;
    QRectF virtualRect = rect();
    virtualRect.setWidth( virtualRect.width() * painterScale );
    // to avoid painting on edges
    virtualRect.setTop( virtualRect.top() + 3.0 );
    virtualRect.setLeft( virtualRect.left() + 3.0 );
    virtualRect.setRight( virtualRect.right() - 3.0 );
    virtualRect.setBottom( virtualRect.bottom() - 3.0  );
    painter->scale(scaleValue, 1.0);
    //qDebug() << "virtualRect:" << virtualRect;
    painter->setRenderHint(QPainter::SmoothPixmapTransform);
    painter->setRenderHint(QPainter::TextAntialiasing);

    if (!m_buttonState && !m_buttonImageEnabled.isNull())
        painter->drawImage(3, 3, m_buttonImageEnabled);
    else if (m_buttonState && !m_buttonImageDisabled.isNull())
        painter->drawImage(3, 3, m_buttonImageDisabled);

    if (m_duration > 0 && m_blendIn > 0) {
        double blendInBorderX = (m_blendIn / m_duration) * virtualRect.width();
        QPen penLine(QColorConstants::Svg::orange, 1.5, Qt::DashLine, Qt::FlatCap, Qt::BevelJoin);
        painter->setPen(penLine);
        painter->drawLine(blendInBorderX, penLine.widthF(), blendInBorderX, virtualRect.height());
    }
    if (m_duration > 0 && m_blendOut > 0) {
        double blendOutBorderX = ((m_duration - m_blendOut) / m_duration) * virtualRect.width();
        QPen penLine(QColorConstants::Svg::orange, 1.5, Qt::DashLine, Qt::FlatCap, Qt::BevelJoin);
        painter->setPen(penLine);
        painter->drawLine(blendOutBorderX, penLine.widthF(), blendOutBorderX, virtualRect.height());
    }

    if (!m_textMain.isEmpty()) {
        QFont fontMain(m_textFontMain, m_textSizeMain);
        //fontMain.setBold(true);
        painter->setFont(fontMain);
        painter->setPen(QPen(m_textColorMain));
        bool fitsWidth = QFontMetrics(fontMain).horizontalAdvance(m_textMain) < virtualRect.width();
        int alignFlags = m_textMainAlignFlags;
        if (!alignFlags) {
            alignFlags |= fitsWidth ? Qt::AlignHCenter : Qt::AlignLeft;
            alignFlags |= drawBoth ? Qt::AlignTop : (Qt::AlignVCenter | Qt::TextWordWrap);
        }
        painter->drawText(virtualRect, alignFlags, m_textMain);
    }
    if (!m_textSecondary.isEmpty()) {
        QFont fontSecondary(m_textFontSecondary, m_textSizeSecondary);
        painter->setFont(fontSecondary);
        painter->setPen(QPen(m_textColorSecondary));
        bool fitsWidth = QFontMetrics(fontSecondary).horizontalAdvance(m_textSecondary) < virtualRect.width();
        int alignFlags = m_textSecondaryAlignFlags;
        if (!alignFlags) {
            alignFlags |= fitsWidth ? Qt::AlignHCenter : Qt::AlignLeft;
            alignFlags |= drawBoth ? Qt::AlignBottom : (Qt::AlignVCenter | Qt::TextWordWrap);
        }
        painter->drawText(virtualRect, alignFlags, m_textSecondary);
    }

    painter->restore();
}

QVariant CustomRectItem::itemChange(GraphicsItemChange change, const QVariant &value)
{
    if (change == ItemPositionChange && scene()) {
        // value is the new position.
        QPointF newPos = value.toPointF();
        if (m_bordersRect.isValid() && !m_bordersRect.contains(newPos)) {
            //qDebug() << "m_bordersRect: is valid and does not contain newPos!";
            // Keep the item inside the borders rect.
            newPos.setX(qMin(m_bordersRect.right(), qMax(newPos.x(), m_bordersRect.left())));
            newPos.setY(qMin(m_bordersRect.bottom(), qMax(newPos.y(), m_bordersRect.top())));
            return newPos;
        }
    }
    return super::itemChange(change, value);
}

void CustomRectItem::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event)
{
    //qDebug() << "mouseDoubleClickEvent! m_buttonState = " << m_buttonState;
    m_buttonState = !m_buttonState;
    this->update(this->boundingRect());
    emit onDoubleClick(m_buttonState);
    event->accept();
}

