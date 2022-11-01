#ifndef CUSTOMRECTITEM_H
#define CUSTOMRECTITEM_H

#include <QGraphicsRectItem>
#include <QGraphicsTextItem>
#include <QGraphicsProxyWidget>
#include <QObject>
#include <QPainter>
#include "YmlStructs.h"

class CustomRectItem : public QObject, public QGraphicsRectItem
{
    Q_OBJECT
    using super = QGraphicsRectItem;
private:
    QGraphicsTextItem* m_labelItem = nullptr;
    QString m_textLabel; 		// above block
    QString m_textMain;  		// inside upper
    QString m_textSecondary; 	// inside lower
    int m_textLabelAlignFlags = 0;
    int m_textMainAlignFlags = 0;
    int m_textSecondaryAlignFlags = 0;
    QString m_textFontLabel = "Segoe UI";
    QString m_textFontMain = "Arial";
    QString m_textFontSecondary = "Verdana";
    int m_textSizeLabel = 9;
    int m_textSizeMain = 10;
    int m_textSizeSecondary = 7;
    double m_duration = -1.0;
    double m_blendIn = -1.0, m_blendOut = -1.0; // duration
    ShotActionBase* m_shotAction = nullptr;
    QHash<QString, QVariant> m_data;
    QPixmap m_buttonImageEnabled = QPixmap();
    QPixmap m_buttonImageDisabled = QPixmap();
    bool m_buttonState = false;
    QRectF m_bordersRect;

    QColor m_backgroundColor = { QColorConstants::Svg::whitesmoke };
    QColor m_textColorLabel = { QColorConstants::Svg::black };
    QColor m_textColorMain = { QColorConstants::Svg::black };
    QColor m_textColorSecondary = { QColorConstants::Svg::maroon };
	int labelFontHeight() const;
    void setDefaults();
    void updateBackground();
	
protected:
	void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = nullptr) override;
    QVariant itemChange(QGraphicsItem::GraphicsItemChange change, const QVariant &value) override;
    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event) override;
    void contextMenuEvent(QGraphicsSceneContextMenuEvent *event) override;

public:
    CustomRectItem(qreal x, qreal y, qreal width, qreal height, QGraphicsItem *parent = nullptr);
    CustomRectItem(const QRectF &rect, QGraphicsItem *parent = nullptr);
    CustomRectItem(QGraphicsItem *parent = nullptr);

    void setTextParamsLabel(const QColor color, int fontSize = 12, QString fontName = "Segoe UI", int alignFlags = 0);
    void setTextParamsMain(const QColor color, int fontSize = 12, QString fontName = "Arial", int alignFlags = 0);
    void setTextParamsSecondary(const QColor color, int fontSize = 10, QString fontName = "Verdana", int alignFlags = 0);
    void setBackgroundColor(const QColor color);
    void setTextLabel(const QString& text);
    void setTextMain(const QString& text);
    void setTextSecondary(const QString& text);
    void setDuration(double dur);
    void setBlendIn(double blendIn);
    void setBlendOut(double blendOut);
    void setShotAction(ShotActionBase* action);

    QString textLabel() {
        return m_textLabel;
    }
	QString textMain() {
        return m_textMain;
    }
    QString textSecondary() {
        return m_textSecondary;
    }
	QColor textColorLabel() {
        return m_textColorLabel;
    }
    QColor textColorMain() {
        return m_textColorMain;
    }
    QColor textColorSecondary() {
        return m_textColorSecondary;
    }
    QColor colorBackground() {
        return m_backgroundColor;
    }
	int textSizeLabel() {
        return m_textSizeLabel;
    }
    int textSizeMain() {
        return m_textSizeMain;
    }
    int textSizeSecondary() {
        return m_textSizeSecondary;
    }
    double duration() {
        return m_duration;
    }
    double blendIn() {
        return m_blendIn;
    }
    double blendOut() {
        return m_blendOut;
    }
    ShotActionBase* getShotAction() {
        return m_shotAction;
    }
    const QRectF &bordersRect() {
        return m_bordersRect;
    }
    void setBordersRect(const QRectF &newBordersRect);
    void setButtonImages(const QPixmap &newButtonImageEnabled, const QPixmap &newButtonImageDisabled);

    QVariant data(const QString& key);
    void setData(const QString &newKey, const QVariant& newValue);

signals:
    void doubleClick(bool buttonState);
    void contextEvent(QPointF screenPos);
};

#endif // CUSTOMRECTITEM_H
