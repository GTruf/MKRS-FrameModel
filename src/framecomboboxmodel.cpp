#include "framecomboboxmodel.h"
#include "frame.h"

FrameComboBoxModel::FrameComboBoxModel(QObject* parent) : QAbstractListModel(parent)
{
}

void FrameComboBoxModel::AddFrame(const Frame* frame) {
    beginInsertRows(QModelIndex(), rowCount(),  rowCount());
    _frames.insert(rowCount(), frame);
    endInsertRows();
}

void FrameComboBoxModel::EraseFrame(const Frame* frame) {
    auto eraseIt = std::find(_frames.begin(), _frames.end(), frame);
    removeRows(std::distance(_frames.begin(), eraseIt), 1);
}

QVariant FrameComboBoxModel::data(const QModelIndex& index, int role) const {
    if (!index.isValid() || index.row() < 0 ||  index.row()  >= _frames.size())
            return QVariant();

    switch(role) {
        case Qt::DisplayRole:
            return _frames[index.row()]->GetName();
        default:
            return QVariant();
    }
}

int FrameComboBoxModel::rowCount(const QModelIndex&) const {
    return _frames.size();
}

bool FrameComboBoxModel::removeRows(int row, int count, const QModelIndex& parent) {
   if (parent.isValid() || _frames.isEmpty())
       return false;

   beginRemoveRows(parent, row, row + count - 1);
   for (int i = 0; i < count; ++i) {
       _frames.removeAt(row);
   }
   endRemoveRows();
   return true;
}
