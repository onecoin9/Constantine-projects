#ifndef SETTING_H
#define SETTING_H

#include <QDialog>
#include <QSettings>
#include <QString>

namespace Ui {
    class settingDialog;
}

class settingDialog : public QDialog
{
    Q_OBJECT

public:
    explicit settingDialog(QWidget* parent = nullptr);
    ~settingDialog();

signals:
    void settingsChanged();
    void viewSettingsChanged(const QStringList& visibleViews);
    void gridSizeChanged(int rows, int cols, int baseRows, int baseCols);

private slots:
    void onGridSizeChanged();
    virtual void accept() override;
    void onSceneChanged(const QString& scene);
    void onBrowseButtonClicked();
    void onSaveSettings();

private:
    bool loadSettings();
    void initUI();
    void setupConnections();
    void initDefaultValues();
    void updateUIForScene(const QString& scene);
    void hideAllControls();
    void showAgingTestControls();
    void showAG06Controls();
    void showAP8000Controls();
    void setCoordTimeControlsVisible(bool visible);
    void setCoordControlsVisible(bool visible);
    void setTimeControlsVisible(bool visible);
private:
    Ui::settingDialog* ui;
    QSettings settings;
    QString currentScene;

    settingDialog(const settingDialog&) = delete;
    settingDialog& operator=(const settingDialog&) = delete;

signals:
    void sceneChanged(const QString& scene);

private slots:
    void on_comboBoxScene_currentTextChanged(const QString& text);

private:
    void saveSettings();

};

#endif // SETTING_H