#include "frame.h"

Frame::Frame(QString name) : _name(std::move(name)), _infoText(_frameHint + _name + "\"")
{
}

const QString& Frame::GetName() const {
    return _name;
}

const QString& Frame::GetLongestFrameText() const {
    return _longestSlotText.size() < _infoText.size() ? _infoText : _longestSlotText;
}

const QString& Frame::GetInfoText() const {
    return _infoText;
}

const Frame::Slots& Frame::GetSlots() const {
    return _slots;
}

Frame::Slots& Frame::GetSlots() {
    return _slots;
}

QStringList Frame::GetAllSlotsWithValues(const QStringList& semanticSearchSlotValues) const {
    QStringList result;

    struct GetSemanticSearchInfo {
        const QStringList& semanticSearchSlotValues;
        const QString& slotName;
        QString semanticSearchInfo = "";

        void operator()(const QString& regularSlotValue) {
            if (semanticSearchSlotValues.contains(regularSlotValue))
                semanticSearchInfo.append("Слот \"").append(slotName).append("\" со значением \"").append(regularSlotValue).append('\"');
        }

        void operator()(const Frame* slotFrame) {
            if (semanticSearchSlotValues.contains(slotFrame->GetName()))
                semanticSearchInfo.append("Слот \"Фрейм-ссылка\" со значением \"").append(slotFrame->GetName()).append('\"');
        }
    };

    for (const auto& [slotName, slotValueVariant] : _slots) {
        GetSemanticSearchInfo getSemanticSearchInfo{semanticSearchSlotValues, slotName};
        std::visit(getSemanticSearchInfo, slotValueVariant);

        if (!getSemanticSearchInfo.semanticSearchInfo.isEmpty())
            result << getSemanticSearchInfo.semanticSearchInfo;
    }

    return result;
}

bool Frame::Contains(const QString& slotName) const {
    return _slots.find(slotName) != _slots.end();
}

void Frame::SetName(QString newName) {
    _name = std::move(newName);
    _infoText = _frameHint + _name + "\"";
}

void Frame::AddSlot(QString slotName, QString slotValue) {
    QString tmpLongestSlotText = slotName + " (" + slotValue + ")";

    if (_longestSlotText.size() < tmpLongestSlotText.size())
        _longestSlotText = std::move(tmpLongestSlotText);

    _slots[std::move(slotName)] = std::move(slotValue);
}

void Frame::AddSlot(const Frame* slotFrame) {
    QString tmpLongestSlotText = "Фрейм-ссылка (\"" + slotFrame->GetName() + "\")";

    if (_longestSlotText.size() < tmpLongestSlotText.size())
        _longestSlotText = std::move(tmpLongestSlotText);

    _slots[slotFrame->GetName()] = slotFrame;
}

void Frame::ReplaceSlotName(const QString& oldFrameName, QString newFrameName) {
    auto node = _slots.extract(oldFrameName);
    node.key() = std::move(newFrameName);
    _slots.insert(std::move(node));
    RecalculateLongestSlotText();
}

void Frame::ReplaceSlotValue(const QString& slotName, QString slotValue) {
    // В данном случае по slotName вернётся именно std::variant, хранящий в себе QString
    _slots.at(slotName) = std::move(slotValue);
    RecalculateLongestSlotText();
}

void Frame::EraseSlot(const QString& slotName) {
    _slots.erase(slotName);
    RecalculateLongestSlotText();
}

void Frame::RecalculateLongestSlotText() {
    auto generateSlotInfoText = [](const std::pair<QString, std::variant<QString, const Frame*>>& frameSlot) -> QString {
        if (std::holds_alternative<QString>(frameSlot.second))
            return frameSlot.first + " (" + std::get<QString>(frameSlot.second) + ")";
        else
            return "Фрейм-ссылка (\"" + std::get<const Frame*>(frameSlot.second)->GetName() + "\")";
    };

    auto longestSlotInfoTextIt = std::max_element(_slots.begin(), _slots.end(), [=]
                                                 (const std::pair<QString, std::variant<QString, const Frame*>>& left,
                                                  const std::pair<QString, std::variant<QString, const Frame*>>& right)
    {
        const auto leftSlotInfoText = generateSlotInfoText(left);
        const auto rightSlotInfoText = generateSlotInfoText(right);
        return leftSlotInfoText.size() < rightSlotInfoText.size();
    });

    if (longestSlotInfoTextIt != _slots.end()) {
        if (std::holds_alternative<QString>(longestSlotInfoTextIt->second))
            _longestSlotText = longestSlotInfoTextIt->first + " (" + std::get<QString>(longestSlotInfoTextIt->second) + ")";
        else
            _longestSlotText = "Фрейм-ссылка (\"" + std::get<const Frame*>(longestSlotInfoTextIt->second)->GetName() + "\")";
    }
}
