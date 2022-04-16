#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "framecomboboxmodel.h"
#include <QMainWindow>
#include <QRegularExpressionValidator>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

private slots:
    void on_addFrame_clicked();
    void on_addSlot_clicked();
    void on_editFrame_clicked();
    void on_deleteFrame_clicked();
    void on_editSlot_clicked();
    void on_deleteSlot_clicked();
    void on_syntaxSearch_clicked();
    void on_semanticSearch_clicked();

private:
    Ui::MainWindow* ui;
    QRegularExpressionValidator _framePositionValidator;
    const QString _groupBoxEnabledTitle, _groupBoxDisabledTitle;
    FrameComboBoxModel _slotFramesModel, _targetFramesModel, _framesToEditModel;
    QString _filePath;

    void Init();
    void ResetFrameInfo();
    void ResetSlotInfo();
    void UpdateEditableSlotsOfFrame(const QString& editableFrameName);
    void LoadFromFile();
    void SaveToFile();
};

#endif // MAINWINDOW_H
