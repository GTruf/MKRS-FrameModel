#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QMessageBox>
#include <QFile>
#include <QTextStream>

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent), ui(new Ui::MainWindow), _framePositionValidator(QRegularExpression("\\d{4}")),
    _groupBoxEnabledTitle("QGroupBox::title { color: black; }"), _groupBoxDisabledTitle("QGroupBox::title { color: gray; }"),
    _filePath(QString(PROJECT_PATH).append("/resource/frame_model.fm"))
{
    ui->setupUi(this);
    Init();
    LoadFromFile();
}

void MainWindow::Init() {
    ui->xFrame->setValidator(&_framePositionValidator);
    ui->yFrame->setValidator(&_framePositionValidator);
    ui->xNewFrame->setValidator(&_framePositionValidator);
    ui->yNewFrame->setValidator(&_framePositionValidator);
    ui->slotTypeGroupBox->setStyleSheet(_groupBoxDisabledTitle);

    ui->slotFrames->setModel(&_slotFramesModel);
    ui->targetFrames->setModel(&_targetFramesModel);
    ui->framesToEdit->setModel(&_framesToEditModel);

    for (auto slotTypeButton : {ui->slotRegularType, ui->slotFrameType}) {
        connect(slotTypeButton, &QRadioButton::clicked, this, [=]() {
            const auto isSlotRegular = slotTypeButton == ui->slotRegularType;

            ui->slotNameValueLabel->setEnabled(isSlotRegular);
            ui->slotName->setEnabled(isSlotRegular);
            ui->slotValue->setEnabled(isSlotRegular);

            ui->slotFrameLabel->setEnabled(!isSlotRegular);
            ui->slotFrames->setEnabled(!isSlotRegular);

            ResetSlotInfo();
        });
    }

    connect(ui->framesToEdit, &QComboBox::currentTextChanged, this, [=](const QString& editableFrameName) {
        UpdateEditableSlotsOfFrame(editableFrameName);
    });

    connect(ui->editableSlotsOfEditableFrame, &QComboBox::currentTextChanged, this, [=](const QString& editableSlotName) {
        ui->editableSlotType->clear();
        ui->needsToChangedSlotValue->setChecked(false);
        ui->newSlotName->clear();
        ui->newValueOfRegularSlot->clear();

        if (!editableSlotName.isEmpty()) {
            const auto& editableFrameSlots = ui->frameModel->At(ui->framesToEdit->currentText()).GetSlots();

            if (std::holds_alternative<QString>(editableFrameSlots.at(editableSlotName))) {
                ui->editableSlotType->setText("Обычный слот");
                ui->slotEditInfoGroupBox->setEnabled(true);
            }
            else {
                ui->editableSlotType->setText("Фрейм");
                ui->slotEditInfoGroupBox->setEnabled(false);
            }
        }
    });
}

void MainWindow::on_addFrame_clicked() {
    const auto frameName = ui->frameName->text();
    const auto xFrame = ui->xFrame->text();
    const auto yFrame = ui->yFrame->text();

    if (frameName.isEmpty() || xFrame.isEmpty() || yFrame.isEmpty()) {
        QMessageBox::critical(nullptr, "Ошибка при добавлении фрейма", "Все поля при добавлении фрейма должны быть заполнены");
        return;
    }

    if (ui->frameModel->Contains(frameName)) {
        QMessageBox::critical(nullptr, "Ошибка при добавлении фрейма", "Фрейм \"" + frameName + "\" уже существует");
        return;
    }

    Frame frame(frameName);
    QPoint framePosition(QPoint(xFrame.toInt(), yFrame.toInt()));

    const auto* addedFrame = ui->frameModel->AddFrame(std::move(frame), framePosition);
    ui->frameModel->update();

    _slotFramesModel.AddFrame(addedFrame);
    _targetFramesModel.AddFrame(addedFrame);
    _framesToEditModel.AddFrame(addedFrame);

    ui->addSlotGroupBox->setEnabled(true);
    ui->editFrameGroupBox->setEnabled(true);
    ui->slotTypeGroupBox->setEnabled(true);
    ui->slotTypeGroupBox->setStyleSheet(_groupBoxEnabledTitle);

    ResetFrameInfo();
}

void MainWindow::on_addSlot_clicked() {
    auto& frameModel = *ui->frameModel;
    auto& targetFrame = frameModel.At(ui->targetFrames->currentText());

    if (ui->slotRegularType->isChecked()) {
        const auto slotName = ui->slotName->text();
        const auto slotValue = ui->slotValue->text().isEmpty() ? "Значение" : ui->slotValue->text();

        if (slotName.isEmpty()) {
            QMessageBox::critical(nullptr, "Ошибка при добавлении обычного слота", "Имя слота должно быть заполнено");
            return;
        }

        if (targetFrame.GetName() == slotName) {
            QMessageBox::critical(nullptr, "Ошибка при добавлении обычного слота", "Фрейм не может содержать одноимённый слот");
            return;
        }

        if (targetFrame.Contains(slotName)) {
            QMessageBox::critical(nullptr, "Ошибка при добавлении обычного слота",
                                  "Фрейм \"" + targetFrame.GetName() + "\" уже содержит слот \"" + slotName + "\"");
            return;
        }

        targetFrame.AddSlot(std::move(slotName), slotValue.isEmpty() ? "Значение" : std::move(slotValue));
    }
    else {
        const auto& slotFrame = frameModel.At(ui->slotFrames->currentText());

        if (targetFrame.GetName() == slotFrame.GetName()) {
            QMessageBox::critical(nullptr, "Ошибка при добавлении слота-фрейма", "Фрейм не может содержать одноимённый слот");
            return;
        }

        if (targetFrame.Contains(slotFrame.GetName())) {
            QMessageBox::critical(nullptr, "Ошибка при добавлении слота-фрейма",
                                  "Фрейм \"" + targetFrame.GetName() + "\" уже содержит слот \"" + slotFrame.GetName() + "\"");
            return;
        }

        targetFrame.AddSlot(&slotFrame);
    }

    if (targetFrame.GetName() == ui->framesToEdit->currentText()) {
        UpdateEditableSlotsOfFrame(targetFrame.GetName());
    }

    ui->frameModel->update();
    ResetSlotInfo();
}

void MainWindow::on_editFrame_clicked() {
    auto newFrameName = ui->newFrameName->text();
    ui->frameModel->ReplaceFrameCoords(ui->framesToEdit->currentText(), ui->xNewFrame->text(), ui->yNewFrame->text());
    ui->xNewFrame->clear();
    ui->yNewFrame->clear();

    if (!newFrameName.isEmpty()) {
        if (ui->frameModel->Contains(newFrameName)) {
            QMessageBox::critical(nullptr, "Ошибка при редактировании фрейма", "Фрейм \"" + newFrameName + "\" уже существует");
            return;
        }

        auto& frame = ui->frameModel->At(ui->framesToEdit->currentText());

        if (frame.Contains(newFrameName)) {
            QMessageBox::critical(nullptr, "Ошибка при редактировании фрейма",
                                  "Во фрейме \"" + frame.GetName() + "\" уже содержится слот с именем \"" + newFrameName + "\"");
            return;
        }

        ui->frameModel->ReplaceFrameName(ui->framesToEdit->currentText(), std::move(newFrameName));
        ui->slotFrames->update();
        ui->targetFrames->update();
        ui->framesToEdit->update();
        ui->newFrameName->clear();
    }

    ui->frameModel->update();
}

void MainWindow::on_deleteFrame_clicked() {
    const auto currentEditableFrameName = ui->framesToEdit->currentText();
    const auto& frame = ui->frameModel->At(currentEditableFrameName);

    ui->frameModel->EraseFrame(currentEditableFrameName);
    ui->frameModel->update();
    ui->newFrameName->clear();

    _slotFramesModel.EraseFrame(&frame);
    _targetFramesModel.EraseFrame(&frame);
    _framesToEditModel.EraseFrame(&frame);

    if (ui->frameModel->IsEmpty()) {
        ui->addSlotGroupBox->setEnabled(false);
        ui->editFrameGroupBox->setEnabled(false);
        ui->slotTypeGroupBox->setEnabled(false);
        ui->slotTypeGroupBox->setStyleSheet(_groupBoxDisabledTitle);
    }
}

void MainWindow::on_editSlot_clicked() {
    auto currentSlotName = ui->editableSlotsOfEditableFrame->currentText();

    // Если у редактируемого фрейма есть слоты (в таком случае в комбобоксе будет значение)
    if (!currentSlotName.isEmpty()) {
        // Чтобы галочка с ui->needsToChangedSlotValue при изменении имени слота не убиралась (потому что испустится сигнал
        // currentTextChanged, который отловится в connect'е, определённом в методе Init, который снимет галочку)
        ui->editableSlotsOfEditableFrame->blockSignals(true);

        auto newSlotName = ui->newSlotName->text();
        auto& editableFrame = ui->frameModel->At(ui->framesToEdit->currentText());

        // В принципе ReplaceSlotName и ReplaceSlotValue можно объединить в один метод
        if (!newSlotName.isEmpty()) {
            if (ui->frameModel->Contains(newSlotName)) {
                QMessageBox::critical(nullptr, "Ошибка при редактировании слота", "Фрейм \"" + newSlotName + "\" уже существует");
                return;
            }

            if (editableFrame.Contains(newSlotName)) {
                QMessageBox::critical(nullptr, "Ошибка при редактировании слота",
                                      "Фрейм \"" + editableFrame.GetName() + "\" уже содержит слот \"" + newSlotName + "\"");
                return;
            }

            editableFrame.ReplaceSlotName(currentSlotName, newSlotName);
            currentSlotName = std::move(newSlotName);
            ui->editableSlotsOfEditableFrame->setItemText(ui->editableSlotsOfEditableFrame->currentIndex(), currentSlotName);
        }

        if (ui->needsToChangedSlotValue->isChecked()) {
            auto newSlotValue = ui->newValueOfRegularSlot->text();
            editableFrame.ReplaceSlotValue(currentSlotName, newSlotValue.isEmpty() ? "Значение" : std::move(newSlotValue));
        }

        ui->needsToChangedSlotValue->setChecked(false);
        ui->newSlotName->clear();
        ui->newValueOfRegularSlot->clear();
        ui->frameModel->update();
        ui->editableSlotsOfEditableFrame->blockSignals(false);
    }
}

void MainWindow::on_deleteSlot_clicked() {
    auto currentSlotName = ui->editableSlotsOfEditableFrame->currentText();

    // Если у редактируемого фрейма есть слоты (в таком случае в комбобоксе будет значение)
    if (!currentSlotName.isEmpty()) {
        auto& editableFrame = ui->frameModel->At(ui->framesToEdit->currentText());
        editableFrame.EraseSlot(currentSlotName);
        ui->editableSlotsOfEditableFrame->removeItem(ui->editableSlotsOfEditableFrame->currentIndex());
        ui->frameModel->update();
    }
}

void MainWindow::on_syntaxSearch_clicked() {
    const auto syntaxSearchSlotNamesText = ui->syntaxSearchSlotNames->text();

    if (syntaxSearchSlotNamesText.isEmpty()) {
        QMessageBox::critical(nullptr, "Ошибка синтаксического поиска", "Введите имена слотов через точку с запятой для синтаксического поиска");
        return;
    }

    const auto syntaxSearchSlotNames = syntaxSearchSlotNamesText.split(';');
    const auto syntaxSearchResult = ui->frameModel->SyntaxSearch(syntaxSearchSlotNames);

    QMessageBox::information(nullptr, "Результат синтаксического поиска", syntaxSearchResult);
}

void MainWindow::on_semanticSearch_clicked() {
    const auto semanticSearchSlotValuesText = ui->semanticSearchSlotValues->text();

    if (semanticSearchSlotValuesText.isEmpty()) {
        QMessageBox::critical(nullptr, "Ошибка семантического поиска", "Введите значения слотов через точку с запятой для семантического поиска");
        return;
    }

    const auto semanticSearchSlotValues = semanticSearchSlotValuesText.split(';');
    const auto semanticSearchResult = ui->frameModel->SemanticSearch(semanticSearchSlotValues);

    QMessageBox::information(nullptr, "Результат семантического поиска", semanticSearchResult);
}

void MainWindow::ResetFrameInfo() {
    ui->frameName->clear();
    ui->xFrame->clear();
    ui->yFrame->clear();
}

void MainWindow::ResetSlotInfo() {
    ui->slotName->clear();
    ui->slotValue->clear();
    ui->slotFrames->setCurrentIndex(0);
    ui->targetFrames->setCurrentIndex(0);
}

void MainWindow::UpdateEditableSlotsOfFrame(const QString& editableFrameName) {
    ui->editableSlotsOfEditableFrame->clear();

    if (!editableFrameName.isEmpty()) {
        for (const auto& [slotFrameName, _] : ui->frameModel->At(editableFrameName).GetSlots()) {
            ui->editableSlotsOfEditableFrame->addItem(slotFrameName);
        }
    }
}

void MainWindow::LoadFromFile() {
/* |  0  |    1   |  2 | 3 |    <--- Индексы в записи о фрейме
 *  Фрейм Водитель 1125 150     <--- Так хранится в файле запись о фрейме
 */

/* |  0 |   1   |    2   |      3     |      4      |    5   |  <--- Индексы в записи о слоте
 *  Слот Человек Значение Фрейм-ссылка Целевой_Фрейм Водитель   <--- Так хранится в файле запись о слоте-фрейме
 *
 * |  0 |     1    |    2   |  3 |      4      |                     5                    |  <--- Индексы в записи о слоте
 *  Слот GPS-трекер Значение Есть Целевой_Фрейм Выделенная_для_перевозки_пассажиров_машина   <--- Так хранится в файле запись об обычном слоте
 */

    QFile file(_filePath);

    if (file.open(QFile::ReadOnly)) {
        QTextStream in(&file);

        while (!in.atEnd()) {
            QString line = in.readLine();
            QStringList splitLine = line.split(' ');

            if (splitLine[0] == "Фрейм") {
                Frame frame(splitLine[1].replace('_', ' '));
                const auto* addedFrame = ui->frameModel->AddFrame(std::move(frame), QPoint(splitLine[2].toInt(), splitLine[3].toInt()));

                _slotFramesModel.AddFrame(addedFrame);
                _targetFramesModel.AddFrame(addedFrame);
                _framesToEditModel.AddFrame(addedFrame);
            }
            else if (splitLine[0] == "Слот"){
                if (splitLine[3] == "Фрейм-ссылка") {
                    const auto& frameSlot = ui->frameModel->At(splitLine[1].replace('_', ' '));
                    ui->frameModel->At(splitLine[5].replace('_', ' ')).AddSlot(&frameSlot);
                }
                else {
                    ui->frameModel->At(splitLine[5].replace('_', ' ')).AddSlot(splitLine[1].replace('_', ' '), splitLine[3].replace('_', ' '));
                }
            }
        }

        file.close();

        if (!ui->frameModel->IsEmpty()) {
            ui->addSlotGroupBox->setEnabled(true);
            ui->editFrameGroupBox->setEnabled(true);
            ui->slotTypeGroupBox->setEnabled(true);
            ui->slotTypeGroupBox->setStyleSheet(_groupBoxEnabledTitle);
            UpdateEditableSlotsOfFrame(ui->framesToEdit->currentText());
        }
    }
}

void MainWindow::SaveToFile() {
    QFile file(_filePath);
    file.open(QFile::WriteOnly);
    QTextStream out(&file);

    // Сначала сохранение просто всех фреймов
    for (const auto& [frameName, frameWithPosition] : ui->frameModel->GetFrames()) {
        out << QString::fromUtf8("Фрейм ") << QString(frameName).replace(' ', '_') << " " <<
               QString::number(frameWithPosition.second.x()) << ' ' << QString::number(frameWithPosition.second.y()) << '\n';
    }

    // Сохранение всех слотов всех фреймов
    for (const auto& [frameName, frameWithPosition] : ui->frameModel->GetFrames()) {
        for (const auto& [slotName, frameSlot] : frameWithPosition.first.GetSlots()) {
            QString slotValue;

            if (std::holds_alternative<QString>(frameSlot))
                slotValue = QString(std::get<QString>(frameSlot)).replace(' ', '_');
            else
                slotValue = QString::fromUtf8("Фрейм-ссылка");

            out << QString::fromUtf8("Слот ") << QString(slotName).replace(' ', '_') <<
                   QString::fromUtf8(" Значение ") << slotValue <<
                   QString::fromUtf8(" Целевой_Фрейм ") << QString(frameName).replace(' ', '_') << '\n';
        }
    }

    file.close();
}

MainWindow::~MainWindow() {
    SaveToFile();
    delete ui;
}
