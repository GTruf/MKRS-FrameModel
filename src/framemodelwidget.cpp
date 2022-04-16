#include "framemodelwidget.h"
#include <QPainter>
#include <cmath>

FrameModelWidget::FrameModelWidget(QWidget* parent) : QFrame(parent)
{
}

const FrameModelWidget::Frames& FrameModelWidget::GetFrames() const {
    return _frames;
}

QString FrameModelWidget::SyntaxSearch(const QStringList& syntaxSearchSlotNames) const {
    const auto isNeedToSearchFrameReference = syntaxSearchSlotNames.contains("Фрейм-ссылка");
    QString syntaxSearchResult = syntaxSearchSlotNames.join(", ").prepend("Результат синтаксического поиска для слотов \"").append("\":\n");

    for (const auto& [_, frameWithPosition] : _frames) {
        const auto& frameSlots = frameWithPosition.first.GetSlots();

        for (const auto& [slotName, slotValueVariant] : frameSlots) {
            if (isNeedToSearchFrameReference && std::holds_alternative<const Frame*>(slotValueVariant)) {
                syntaxSearchResult.append("\"Фрейм-ссылка\"").append(" содержится во фрейме \"").append(frameWithPosition.first.GetName()).
                                   append("\" со значением \"").append(std::get<const Frame*>(slotValueVariant)->GetName()).append("\"\n");
            }
            else if (std::holds_alternative<QString>(slotValueVariant) && syntaxSearchSlotNames.contains(slotName)) {
                syntaxSearchResult.append('\"').append(slotName).append("\" содержится во фрейме \"").append(frameWithPosition.first.GetName()).
                                   append("\" со значением \"").append(std::get<QString>(slotValueVariant)).append("\"\n");
            }
        }
    }

    return syntaxSearchResult;
}

QString FrameModelWidget::SemanticSearch(const QStringList& semanticSearchSlotValues) const {
    QString semanticSearchResult = semanticSearchSlotValues.join(", ").prepend("Результат семантического поиска для значения слотов \"").append("\":\n");

    for (const auto& [_, frameWithPosition] : _frames) {
        const auto slotsWithSearchValues = frameWithPosition.first.GetAllSlotsWithValues(semanticSearchSlotValues);

        if (!slotsWithSearchValues.isEmpty()) {
            semanticSearchResult.append("Содержится во фрейме \"").append(frameWithPosition.first.GetName()).append("\":\n");

            for (const auto& slotSemanticSearchInfo : slotsWithSearchValues) {
                semanticSearchResult.append("    — ").append(slotSemanticSearchInfo).append('\n');
            }
        }
    }

    return semanticSearchResult;
}

void FrameModelWidget::EraseFrame(const QString& erasableFrameName) {
    for (auto& [frameName, frameWithPosition] : _frames) {
        if (frameName != erasableFrameName) {
            auto& frameSlots = frameWithPosition.first.GetSlots();
            auto foundErasableFrameIt = frameSlots.find(erasableFrameName);

            // Если удаляемый фрейм найден, как слот в каком-то другом фрейме, удаляем его из слотов этого фрейма тоже
            if (foundErasableFrameIt != frameSlots.end()) {
                // Если найденный слот с именем erasableFrameName действительно является фреймом
                // (ссылкой на фрейм, который удаляется), тогда удаляем его из списка слотов у рассматриваемого фрейма
                if (std::holds_alternative<const Frame*>(foundErasableFrameIt->second)) {
                    frameSlots.erase(foundErasableFrameIt);
                }
            }
        }
    }

    _frames.erase(erasableFrameName);
}

void FrameModelWidget::ReplaceFrameName(const QString& oldFrameName, QString newFrameName) {
    for (auto& [frameName, frameWithPosition] : _frames) {
        if (frameName != oldFrameName) {
            auto& frameSlots = frameWithPosition.first.GetSlots();
            auto foundErasableFrameIt = frameSlots.find(oldFrameName);

            // Если изменяемый фрейм найден, как слот в каком-то другом фрейме, изменяем его имя (ключ) в слотах этого фрейма тоже
            if (foundErasableFrameIt != frameSlots.end()) {
                frameWithPosition.first.ReplaceSlotName(oldFrameName, newFrameName);
            }
        }
    }

    auto node = _frames.extract(oldFrameName);
    node.mapped().first.SetName(newFrameName);
    node.key() = std::move(newFrameName);
    _frames.insert(std::move(node));
}

Frame& FrameModelWidget::At(const QString& frameName) {
    return _frames.at(frameName).first;
}

bool FrameModelWidget::Contains(const QString& frameName) const {
    return _frames.find(frameName) != _frames.end();
}

const Frame* FrameModelWidget::AddFrame(Frame frame, QPoint framePosition) {
    auto& mappedElement = _frames[frame.GetName()];
    mappedElement = std::make_pair(std::move(frame), framePosition);
    return &mappedElement.first;
}

bool FrameModelWidget::IsEmpty() const {
    return _frames.empty();
}

void FrameModelWidget::ReplaceFrameCoords(const QString& frameName, const QString& x, const QString& y) {
    auto& [frame, frameCoords] = _frames.at(frameName);

    if (!x.isEmpty()) frameCoords.setX(x.toInt());
    if (!y.isEmpty()) frameCoords.setY(y.toInt());
}

void FrameModelWidget::paintEvent(QPaintEvent*) {
    QPainter painter(this);
    painter.setRenderHints(QPainter::Antialiasing);
    painter.setBrush(QBrush(QColor(211, 223, 172))); // #d3dfac

    QFont font = painter.font();
    font.setPixelSize(16);
    painter.setFont(font);

    for (const auto& [_, frameWithPosition] : _frames) {
        const auto frameInfoText = frameWithPosition.first.GetInfoText();
        const auto longestFrameText = frameWithPosition.first.GetLongestFrameText();

        QFontMetrics fontMetrics(font);
        const int rectWidth = fontMetrics.horizontalAdvance(longestFrameText) + 40;

        const auto sourceFrameRect = QRect(frameWithPosition.second, QSize(rectWidth, 75 + 20 * frameWithPosition.first.GetSlots().size()));
        auto tmpFrameRect = sourceFrameRect;

        painter.drawRect(tmpFrameRect);

        tmpFrameRect.setTop(tmpFrameRect.y() - tmpFrameRect.height() + 35);
        painter.drawText(tmpFrameRect, Qt::AlignCenter, frameInfoText);

        tmpFrameRect.setTop(tmpFrameRect.y() + tmpFrameRect.height() * 0.5 + 15);
        painter.drawLine(QPoint(tmpFrameRect.x() + 15, tmpFrameRect.y()), QPoint(tmpFrameRect.x() + tmpFrameRect.width() - 15, tmpFrameRect.y()));

        tmpFrameRect.setTop(tmpFrameRect.y() + 5);
        tmpFrameRect.setLeft(tmpFrameRect.x() + 15);

        font.setBold(true); painter.setFont(font);
        painter.drawText(tmpFrameRect, Qt::AlignLeft, "Слоты:");
        font.setBold(false); painter.setFont(font);

        DrawSlots(painter, frameWithPosition.first.GetSlots(), sourceFrameRect, tmpFrameRect);
    }
}

void FrameModelWidget::DrawSlots(QPainter& painter, const Frame::Slots& frameSlots, const QRect& sourceFrameRect, QRect& tmpFrameRect) {
    for (const auto& [slotName, frameSlotVariant] : frameSlots) {
        QString slotInfoText;
        const Frame* slotFrame = nullptr;

        if (std::holds_alternative<QString>(frameSlotVariant)) {
            slotInfoText.append(slotName).append(" (").append(std::get<QString>(frameSlotVariant)).append(')');
        }
        else {
            slotFrame = std::get<const Frame*>(frameSlotVariant);
            slotInfoText.prepend("Фрейм-ссылка (\"").append(slotFrame->GetName()).append("\")");
        }

        tmpFrameRect.setTop(tmpFrameRect.y() + 20);
        painter.drawText(tmpFrameRect, Qt::AlignLeft, slotInfoText);

        if (slotFrame) {
            const auto slotFrameHeight = 75 + 20 * slotFrame->GetSlots().size();
            auto slotFramePosition = _frames.at(slotFrame->GetName()).second;

            if (slotFramePosition.x() >= sourceFrameRect.x() + sourceFrameRect.width() * 0.5) {
                if (slotFramePosition.x() <= sourceFrameRect.x() + sourceFrameRect.width() &&
                    slotFramePosition.y() > sourceFrameRect.y() + sourceFrameRect.height())
                {
                    DrawLineWithArrow(painter, sourceFrameRect.bottomRight(), slotFramePosition);
                }
                else {
                    slotFramePosition.setY(slotFramePosition.y() + slotFrameHeight);
                    DrawLineWithArrow(painter, sourceFrameRect.topRight(), slotFramePosition);
                }
            }
            else {
                QFontMetrics fontMetrics(painter.font());
                const int slotFrameRectWidth = fontMetrics.horizontalAdvance(slotFrame->GetLongestFrameText()) + 40;

                if (slotFramePosition.x() + slotFrameRectWidth >= sourceFrameRect.x() &&
                    slotFramePosition.y() > sourceFrameRect.y() + sourceFrameRect.height())
                {
                    slotFramePosition.setX(slotFramePosition.x() + slotFrameRectWidth);
                    DrawLineWithArrow(painter, sourceFrameRect.bottomLeft(), slotFramePosition);
                }
                else {
                    slotFramePosition.setX(slotFramePosition.x() + slotFrameRectWidth);
                    slotFramePosition.setY(slotFramePosition.y() + slotFrameHeight);
                    DrawLineWithArrow(painter, sourceFrameRect.topLeft(), slotFramePosition);
                }
            }
        }
    }
}

void FrameModelWidget::DrawLineWithArrow(QPainter& painter, QPoint start, QPoint end) {
    constexpr double arrowHeadSize = 10; // Размер треугольника на конце стрелки

    painter.save();
    painter.setPen(Qt::black);
    painter.setBrush(Qt::black);

    const QLineF line(end, start);
    const double angle = std::atan2(-line.dy(), line.dx());
    const QPointF arrowP1 = line.p1() + QPointF(std::sin(angle + M_PI / 3) * arrowHeadSize, std::cos(angle + M_PI / 3) * arrowHeadSize);
    const QPointF arrowP2 = line.p1() + QPointF(std::sin(angle + M_PI - M_PI / 3) * arrowHeadSize, std::cos(angle + M_PI - M_PI / 3) * arrowHeadSize);

    QPolygonF arrowHead;
    arrowHead.clear();
    arrowHead << line.p1() << arrowP1 << arrowP2;
    painter.drawLine(line);
    painter.drawPolygon(arrowHead);
    painter.restore();
}
