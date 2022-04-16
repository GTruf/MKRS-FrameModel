#ifndef FRAMECOMBOBOXMODEL_H
#define FRAMECOMBOBOXMODEL_H

#include <QAbstractListModel>

class Frame;

class FrameComboBoxModel : public QAbstractListModel {
public:
    explicit FrameComboBoxModel(QObject* parent = nullptr);
    void AddFrame(const Frame* frame);
    void EraseFrame(const Frame* frame);

    QVariant data(const QModelIndex& index, int role) const override;
    int rowCount(const QModelIndex& = QModelIndex()) const override;
    bool removeRows(int row, int count, const QModelIndex& parent = QModelIndex()) override;

private:
    QList<const Frame*> _frames;
};

#endif // FRAMECOMBOBOXMODEL_H
