#ifndef FRAMEMODELWIDGET_H
#define FRAMEMODELWIDGET_H

#include "frame.h"
#include <QFrame>

class QPainter;

class FrameModelWidget : public QFrame {
    Q_OBJECT

public:
    // [FrameName, [Frame, FramePosition]]
    using Frames = std::unordered_map<QString, std::pair<Frame, QPoint>>;

    explicit FrameModelWidget(QWidget* parent = nullptr);
    const Frames& GetFrames() const;
    QString SyntaxSearch(const QStringList& syntaxSearchSlotNames) const;
    QString SemanticSearch(const QStringList& semanticSearchSlotValues) const;
    Frame& At(const QString& frameName);
    bool Contains(const QString& frameName) const;
    const Frame* AddFrame(Frame frame, QPoint framePosition);
    bool IsEmpty() const;
    void ReplaceFrameCoords(const QString& frameName, const QString& x, const QString& y);
    void EraseFrame(const QString& erasableFrameName);
    void ReplaceFrameName(const QString& oldFrameName, QString newFrameName);

protected:
    void paintEvent(QPaintEvent*) override;

private:
    // [FrameName, [Frame, FramePosition]]
    Frames _frames;

    void DrawSlots(QPainter& painter, const Frame::Slots& frameSlots, const QRect& sourceFrameRect, QRect& tmpFrameRect);
    static void DrawLineWithArrow(QPainter& painter, QPoint start, QPoint end);
};

#endif // FRAMEMODELWIDGET_H
