#ifndef FRAME_H
#define FRAME_H

#include <QHash>
#include <QString>
#include <variant>

class Frame {
public:
    // [SlotName, Slot (обычный(его значение) / слот-фрейм)]
    using SlotValue = std::variant<QString, const Frame*>;
    using Slots = std::unordered_map<QString, SlotValue>;

    Frame() = default;
    explicit Frame(QString name);
    const QString& GetName() const;
    const QString& GetLongestFrameText() const;
    const QString& GetInfoText() const;
    const Slots& GetSlots() const;
    Slots& GetSlots();
    QStringList GetAllSlotsWithValues(const QStringList& semanticSearchSlotValues) const;
    bool Contains(const QString& slotName) const;
    void SetName(QString newName);
    void AddSlot(QString slotName, QString slotValue);
    void AddSlot(const Frame* slotFrame);
    void ReplaceSlotName(const QString& oldFrameName, QString newFrameName);
    void ReplaceSlotValue(const QString& slotName, QString slotValue);
    void EraseSlot(const QString& slotName);

private:
    QString _name;
    QString _longestSlotText;
    QString _infoText;
    Slots _slots;

    inline static const QString _frameHint = "Фрейм \"";

    void RecalculateLongestSlotText();
};

#endif // FRAME_H
