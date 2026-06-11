#pragma once

#include <QGridLayout>
#include <QHBoxLayout>
#include <QKeyEvent>
#include <QLabel>
#include <QPushButton>
#include <QResizeEvent>
#include <QTimer>
#include <QVBoxLayout>
#include <QWidget>

#include "gameengine.h"

class MainWindow : public QWidget
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);

protected:
    void keyPressEvent(QKeyEvent *event) override; // Handles movement shortcuts.
    void resizeEvent(QResizeEvent *event) override; // Keeps tiles square on resize.

private:
    GameEngine engine;
    QTimer *hardModeTimer;
    bool endDialogShown;

    // Main UI widgets.
    QWidget *contentWidget;
    QWidget *boardWidget;
    QLabel *titleLabel;
    QLabel *subtitleLabel;
    QLabel *scoreLabel;
    QLabel *bestScoreLabel;
    QLabel *boardLabels[BOARD_N][BOARD_M];
    QGridLayout *gridLayout;

    QPushButton *restartBtn;
    QPushButton *undoBtn;

    // Mode buttons.
    QPushButton *normalModeBtn;
    QPushButton *unlimitedModeBtn;
    QPushButton *hardModeBtn;

    void setupUI();
    void updateUI();
    void updateControls();
    void updateTileSizes();
    void resetHardModeTimer();
    QString getTileColor(int value);
    QString getButtonStyle(bool primary = false) const;

private slots:
    void onRestartClicked();
    void onUndoClicked();
    void onHardModeTimeout();
    void setModeNormal();
    void setModeUnlimited();
    void setModeHard();
};
