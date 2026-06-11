#include "mainwindow.h"

#include <QAbstractButton>
#include <QMessageBox>
#include <algorithm>

MainWindow::MainWindow(QWidget *parent)
    : QWidget(parent), endDialogShown(false)
{
    setWindowTitle("2048 - CmpE 230");
    resize(500, 600);

    // Let the window receive keyboard input.
    setFocusPolicy(Qt::StrongFocus);

    hardModeTimer = new QTimer(this);
    // QTimer emits timeout() when Hard Mode needs an automatic move.
    connect(hardModeTimer, &QTimer::timeout, this, &MainWindow::onHardModeTimeout);

    setupUI();
    updateUI();
}

void MainWindow::setupUI()
{
    setStyleSheet("background-color: #faf8ef;");

    // The page layout centers the game content in the window.
    auto *pageLayout = new QVBoxLayout(this);
    pageLayout->setContentsMargins(24, 24, 24, 24);
    pageLayout->setSpacing(0);

    // A max width keeps the board from stretching too much on wide screens.
    contentWidget = new QWidget(this);
    contentWidget->setMaximumWidth(760);
    auto *mainLayout = new QVBoxLayout(contentWidget);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(12);

    auto *topLayout = new QHBoxLayout();
    topLayout->setSpacing(16);

    // Title and score cards share the top row.
    auto *titleLayout = new QVBoxLayout();
    titleLayout->setSpacing(0);
    titleLabel = new QLabel("2048", this);
    titleLabel->setStyleSheet("color: #776e65; font-size: 40px; font-weight: 800; background: transparent;");
    subtitleLabel = new QLabel("Join the tiles, get to 2048!", this);
    subtitleLabel->setStyleSheet("color: #9d9388; font-size: 13px; background: transparent;");
    titleLayout->addWidget(titleLabel);
    titleLayout->addWidget(subtitleLabel);
    topLayout->addLayout(titleLayout, 1);

    scoreLabel = new QLabel(this);
    bestScoreLabel = new QLabel(this);
    // Score cards use QLabel because the values are only displayed.
    const QString scoreCardStyle =
        "QLabel { background-color: #bbada0; color: #ffffff; border-radius: 5px;"
        " padding: 6px 14px; min-width: 76px; font-weight: 700; }";
    scoreLabel->setAlignment(Qt::AlignCenter);
    bestScoreLabel->setAlignment(Qt::AlignCenter);
    scoreLabel->setStyleSheet(scoreCardStyle);
    bestScoreLabel->setStyleSheet(scoreCardStyle);
    topLayout->addWidget(scoreLabel);
    topLayout->addWidget(bestScoreLabel);
    mainLayout->addLayout(topLayout);

    auto *btnLayout = new QHBoxLayout();
    btnLayout->setSpacing(8);
    normalModeBtn = new QPushButton("Normal", this);
    unlimitedModeBtn = new QPushButton("Unlimited", this);
    hardModeBtn = new QPushButton("Hard", this);
    undoBtn = new QPushButton("Undo (U)", this);
    restartBtn = new QPushButton("Restart (R)", this);

    normalModeBtn->setStyleSheet(getButtonStyle());
    unlimitedModeBtn->setStyleSheet(getButtonStyle());
    hardModeBtn->setStyleSheet(getButtonStyle());
    undoBtn->setStyleSheet(getButtonStyle(true));
    restartBtn->setStyleSheet(getButtonStyle(true));

    btnLayout->addWidget(normalModeBtn);
    btnLayout->addWidget(unlimitedModeBtn);
    btnLayout->addWidget(hardModeBtn);
    btnLayout->addWidget(undoBtn);
    btnLayout->addWidget(restartBtn);
    btnLayout->addStretch();
    mainLayout->addLayout(btnLayout);

    // Connect buttons to their handlers.
    connect(normalModeBtn, &QPushButton::clicked, this, &MainWindow::setModeNormal);
    connect(unlimitedModeBtn, &QPushButton::clicked, this, &MainWindow::setModeUnlimited);
    connect(hardModeBtn, &QPushButton::clicked, this, &MainWindow::setModeHard);
    connect(undoBtn, &QPushButton::clicked, this, &MainWindow::onUndoClicked);
    connect(restartBtn, &QPushButton::clicked, this, &MainWindow::onRestartClicked);

    boardWidget = new QWidget(this);
    boardWidget->setStyleSheet("background-color: #bbada0; border-radius: 6px;");
    // QGridLayout maps directly to the board rows and columns.
    gridLayout = new QGridLayout(boardWidget);
    gridLayout->setSpacing(10);

    QFont tileFont("Arial", 24, QFont::Bold);
    for (int r = 0; r < BOARD_N; ++r) {
        for (int c = 0; c < BOARD_M; ++c) {
            boardLabels[r][c] = new QLabel("", this);
            boardLabels[r][c]->setMinimumSize(80, 80);
            boardLabels[r][c]->setAlignment(Qt::AlignCenter);
            boardLabels[r][c]->setFont(tileFont);
            boardLabels[r][c]->setStyleSheet("background-color: #cdc1b4; border-radius: 5px;");
            gridLayout->addWidget(boardLabels[r][c], r, c);
        }
    }

    mainLayout->addWidget(boardWidget, 0, Qt::AlignHCenter);
    pageLayout->addWidget(contentWidget, 0, Qt::AlignHCenter | Qt::AlignTop);

    updateTileSizes();
}

void MainWindow::updateUI()
{
    // QLabel accepts simple rich text for the two-line score cards.
    scoreLabel->setText(QString("<span style='font-size: 9px;'>SCORE</span><br>"
                                "<span style='font-size: 18px;'>%1</span>")
                            .arg(engine.getScore()));
    bestScoreLabel->setText(QString("<span style='font-size: 9px;'>BEST</span><br>"
                                    "<span style='font-size: 18px;'>%1</span>")
                                .arg(engine.getBestScore()));

    // Redraw all board cells from the engine.
    for (int r = 0; r < BOARD_N; ++r) {
        for (int c = 0; c < BOARD_M; ++c) {
            int val = engine.getTile(r, c);
            if (val == 0) {
                boardLabels[r][c]->setText("");
                boardLabels[r][c]->setStyleSheet("background-color: #cdc1b4; border-radius: 5px;");
            } else {
                boardLabels[r][c]->setText(QString::number(val));
                boardLabels[r][c]->setStyleSheet(getTileColor(val));
            }
        }
    }

    updateControls();
    updateTileSizes();

    // Show each end-game dialog only once.
    if (engine.getState() == GameState::WON && !endDialogShown) {
        endDialogShown = true;
        hardModeTimer->stop();
        auto *msgBox = new QMessageBox(this);
        msgBox->setWindowTitle("2048");
        msgBox->setText("You Win!");
        // The win dialog gives a direct way to continue in Unlimited Mode.
        auto *continueBtn = msgBox->addButton("Continue Unlimited", QMessageBox::AcceptRole);
        msgBox->addButton("Restart", QMessageBox::ResetRole);
        msgBox->setAttribute(Qt::WA_DeleteOnClose);

        connect(msgBox, &QMessageBox::buttonClicked, this,
                [this, continueBtn](QAbstractButton *button) {
                    if (button == continueBtn) {
                        engine.setMode(GameMode::UNLIMITED);
                        endDialogShown = false;
                        hardModeTimer->stop();
                        updateUI();
                    } else {
                        onRestartClicked();
                    }
                });

        msgBox->show();
    } else if (engine.getState() == GameState::LOST && !endDialogShown) {
        endDialogShown = true;
        hardModeTimer->stop();
        auto *msgBox = new QMessageBox(this);
        msgBox->setWindowTitle("2048");
        msgBox->setText("Game Over!");
        msgBox->setAttribute(Qt::WA_DeleteOnClose);
        auto *restartBtn = msgBox->addButton("Restart", QMessageBox::AcceptRole);
        connect(msgBox, &QMessageBox::buttonClicked, this,
                [this, restartBtn](QAbstractButton *button) {
                    if (button == restartBtn) {
                        onRestartClicked();
                    }
                });
        msgBox->show();
    }
}

void MainWindow::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    // Recalculate square tiles after browser or window resizing.
    updateTileSizes();
}

void MainWindow::updateControls()
{
    const bool playing = engine.getState() == GameState::PLAYING;
    const bool won = engine.getState() == GameState::WON;

    // End-game states hide controls that should not be used anymore.
    normalModeBtn->setVisible(playing);
    unlimitedModeBtn->setVisible(playing || won);
    hardModeBtn->setVisible(playing);
    undoBtn->setVisible(playing);

    normalModeBtn->setEnabled(playing && engine.getMode() != GameMode::NORMAL);
    unlimitedModeBtn->setEnabled((playing && engine.getMode() != GameMode::UNLIMITED) || won);
    hardModeBtn->setEnabled(playing && engine.getMode() != GameMode::HARD);
    undoBtn->setEnabled(playing);
    restartBtn->setEnabled(true);
}

void MainWindow::updateTileSizes()
{
    if (!boardWidget || !gridLayout) {
        return;
    }

    // Width and height both limit the square tile size.
    const int spacing = 8;
    const int padding = 10;
    const int reservedHeight = 170;
    const int availableWidth = std::max(220, width() - 64);
    const int availableHeight = std::max(220, height() - reservedHeight);

    const int tileByWidth = (availableWidth - 2 * padding - spacing * (BOARD_M - 1)) / BOARD_M;
    const int tileByHeight = (availableHeight - 2 * padding - spacing * (BOARD_N - 1)) / BOARD_N;
    // Keep tiles square for both default and rectangular board sizes.
    const int tileSize = std::max(26, std::min({tileByWidth, tileByHeight, 104}));
    const int boardWidth = BOARD_M * tileSize + (BOARD_M - 1) * spacing + 2 * padding;
    const int boardHeight = BOARD_N * tileSize + (BOARD_N - 1) * spacing + 2 * padding;

    gridLayout->setSpacing(spacing);
    gridLayout->setContentsMargins(padding, padding, padding, padding);
    boardWidget->setFixedSize(boardWidth, boardHeight);

    QFont tileFont("Arial", std::max(14, tileSize / 3), QFont::Bold);
    for (int r = 0; r < BOARD_N; ++r) {
        for (int c = 0; c < BOARD_M; ++c) {
            boardLabels[r][c]->setFixedSize(tileSize, tileSize);
            boardLabels[r][c]->setFont(tileFont);
        }
    }
}

QString MainWindow::getButtonStyle(bool primary) const
{
    // Primary buttons are a little darker than mode buttons.
    const QString bg = primary ? "#8f7a66" : "#a8907f";
    const QString hover = primary ? "#9c8774" : "#b69d8a";
    return QString(
        "QPushButton { background-color: %1; color: #ffffff; border: none;"
        " border-radius: 6px; padding: 7px 13px; font-weight: 700; }"
        "QPushButton:hover { background-color: %2; }"
        "QPushButton:disabled { background-color: #eee4da; color: #bbb2a8; }")
        .arg(bg, hover);
}

void MainWindow::keyPressEvent(QKeyEvent *event)
{
    // After the game ends, only restart is allowed.
    if (engine.getState() != GameState::PLAYING) {
        if (event->key() == Qt::Key_R) onRestartClicked();
        return;
    }

    bool moved = false;

    switch (event->key()) {
        case Qt::Key_Up:
        case Qt::Key_W:
            moved = engine.slide(Direction::UP);
            break;
        case Qt::Key_Down:
        case Qt::Key_S:
            moved = engine.slide(Direction::DOWN);
            break;
        case Qt::Key_Left:
        case Qt::Key_A:
            moved = engine.slide(Direction::LEFT);
            break;
        case Qt::Key_Right:
        case Qt::Key_D:
            moved = engine.slide(Direction::RIGHT);
            break;
        case Qt::Key_U:
            onUndoClicked();
            return;
        case Qt::Key_R:
            onRestartClicked();
            return;
        default:
            QWidget::keyPressEvent(event);
            return;
    }

    if (moved) {
        updateUI();
        if (engine.getMode() == GameMode::HARD) {
            resetHardModeTimer();
        }
    }
}

void MainWindow::onRestartClicked()
{
    endDialogShown = false;
    engine.restart();
    // Restart in Hard Mode should also restart the countdown.
    if (engine.getMode() == GameMode::HARD) {
        resetHardModeTimer();
    }
    updateUI();
}

void MainWindow::onUndoClicked()
{
    if (engine.getState() != GameState::PLAYING) {
        return;
    }
    engine.undo();
    if (engine.getMode() == GameMode::HARD) {
        resetHardModeTimer();
    }
    updateUI();
}

void MainWindow::setModeNormal()
{
    if (engine.getMode() == GameMode::NORMAL || engine.getState() != GameState::PLAYING) {
        return;
    }
    engine.setMode(GameMode::NORMAL);
    hardModeTimer->stop();
    updateUI();
}

void MainWindow::setModeUnlimited()
{
    if (engine.getMode() == GameMode::UNLIMITED && engine.getState() == GameState::PLAYING) {
        return;
    }
    // Unlimited Mode can also resume a won board.
    engine.setMode(GameMode::UNLIMITED);
    hardModeTimer->stop();
    endDialogShown = false;
    updateUI();
}

void MainWindow::setModeHard()
{
    if (engine.getMode() == GameMode::HARD || engine.getState() != GameState::PLAYING) {
        return;
    }
    // Entering Hard Mode starts timing from this point.
    engine.setMode(GameMode::HARD);
    resetHardModeTimer();
    updateUI();
}

void MainWindow::resetHardModeTimer()
{
    hardModeTimer->start(5000);
}

void MainWindow::onHardModeTimeout()
{
    // Hard mode makes a random valid move after the timer expires.
    std::vector<Direction> dirs = {Direction::UP, Direction::DOWN, Direction::LEFT, Direction::RIGHT};
    bool moved = false;

    // Try directions until one of them changes the board.
    while (!moved && engine.getState() == GameState::PLAYING) {
        int r = std::rand() % 4;
        moved = engine.slide(dirs[r]);
    }

    if (moved) {
        updateUI();
        resetHardModeTimer();
    }
}

QString MainWindow::getTileColor(int value)
{
    QString bgColor;
    QString color = "#776e65";

    switch (value) {
        case 2: bgColor = "#eee4da"; break;
        case 4: bgColor = "#ede0c8"; break;
        case 8: bgColor = "#f2b179"; color = "#f9f6f2"; break;
        case 16: bgColor = "#f59563"; color = "#f9f6f2"; break;
        case 32: bgColor = "#f67c5f"; color = "#f9f6f2"; break;
        case 64: bgColor = "#f65e3b"; color = "#f9f6f2"; break;
        case 128: bgColor = "#edcf72"; color = "#f9f6f2"; break;
        case 256: bgColor = "#edcc61"; color = "#f9f6f2"; break;
        case 512: bgColor = "#edc850"; color = "#f9f6f2"; break;
        case 1024: bgColor = "#edc53f"; color = "#f9f6f2"; break;
        case 2048: bgColor = "#edc22e"; color = "#f9f6f2"; break;
        default: bgColor = "#3c3a32"; color = "#f9f6f2"; break;
    }

    return QString("background-color: %1; color: %2; border-radius: 5px;").arg(bgColor, color);
}
