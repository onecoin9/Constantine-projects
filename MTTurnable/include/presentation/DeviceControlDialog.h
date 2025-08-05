#ifndef DEVICECONTROLDIALOG_H
#define DEVICECONTROLDIALOG_H

#include <QDialog>
#include <memory> // For std::shared_ptr

// Forward declarations
namespace Core {
    class CoreEngine;
}
class QLineEdit;
class QPushButton;

namespace Presentation {

class DeviceControlDialog : public QDialog
{
    Q_OBJECT

public:
    explicit DeviceControlDialog(std::shared_ptr<Core::CoreEngine> coreEngine, QWidget *parent = nullptr);
    ~DeviceControlDialog();

private slots:
    void onMoveTurntableClicked();
    void onStartAcquisitionClicked();
    void onStopAcquisitionClicked();

private:
    void setupUi();

    std::shared_ptr<Core::CoreEngine> m_coreEngine;

    // UI elements for turntable
    QLineEdit* m_turntableIdInput;
    QLineEdit* m_turntableAngleInput;
    QPushButton* m_turntableMoveButton;

    // UI elements for test board
    QLineEdit* m_testBoardIdInput;
    QPushButton* m_startAcqButton;
    QPushButton* m_stopAcqButton;
};

} // namespace Presentation

#endif // DEVICECONTROLDIALOG_H
