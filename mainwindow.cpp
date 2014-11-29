#include "mainwindow.h"
#include "ui_mainwindow.h"

Player playerObject;
std::vector<Sun> sunFlowers;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    imageAdded = 0;

    if (Player::validPlayerFile())  // If player file is valid, set settings for most recent player.
    {
        for (int i = 0; i < Player::playerListSize(); i++)
        {
          ui->comboBox->addItem(Player::playerName(i)); // Add existing players to comboBox.
        }

        // Set ui elements for most recent player.
        ui->nameLabel->setText(Player::playerName(0));   // Set name to most recent player.
        ui->levelLabel->setText(Player::playerLevel(0));  // Set level to most recent player.
        ui->comboBox->setCurrentIndex(0);    // Set comboBox to most recent player.
    }
    else    // If invalid player file, clear file and only make new button available.
    {
        Player::clearPlayerList();  // Discard current file.

        // Set ui elements for if no players exist.
        ui->nameLabel->setText("");   // Delete player name.
        ui->levelLabel->setText("");  // Delete player level.

        // Disable buttons if no players.
        ui->startButton->setEnabled(false);
        ui->deleteButton->setEnabled(false);
        ui->restartButton->setEnabled(false);
    }

    ui->sunPointsLabel->setText("0");

    // Show PvZ logo until start is pressed.
    QPixmap *logo = new QPixmap(":/Images/Logo.png");
    scene = new QGraphicsScene(this); // scene holds all objects in the scene.
    scene->addPixmap(logo->scaledToWidth(ui->graphicsView->width()/1.5));
    ui->graphicsView->setScene(scene);

    // Disable all plant buttons.
    ui->peaShooterButton->setEnabled(false);
    ui->sunFlowerButton->setEnabled(false);
    ui->cherryBombButton->setEnabled(false);
    ui->wallNutButton->setEnabled(false);
    ui->potatoMineButton->setEnabled(false);
    ui->snowPeaButton->setEnabled(false);
    ui->chomperButton->setEnabled(false);
    ui->repeaterButton->setEnabled(false);
}

void MainWindow::on_comboBox_currentIndexChanged(int index)
{
    // Set ui elements for selected player.
    ui->nameLabel->setText(Player::playerName(index));   // Set name to selected player.
    ui->levelLabel->setText(Player::playerLevel(index));  // Set level to selected player.
}

void MainWindow::on_newButton_clicked()
{       
    // Add player info.
    QString timestamp = QDateTime::currentDateTime().toString("MM.dd.yyyy");    // Get current date.
    QString name = QInputDialog::getText(this, "Get Name", "What's your name? ", QLineEdit::Normal, "");
    QString level = QString::number(1); // Default: level 1.
    Player::addPlayer(timestamp, name, level);  // Add player to playerList.

    // Change ui elements.
    ui->comboBox->insertItem(0, name);  // Insert new name to beginning of combo box.
    ui->comboBox->setCurrentIndex(0);   // Set combo box to name.
    ui->nameLabel->setText(name);   // Set nameLabel to inputted name.
    ui->levelLabel->setText(level); // Set levelLabel to level.

    // Enable buttons in case they were disabled before.
    ui->startButton->setEnabled(true);
    ui->deleteButton->setEnabled(true);
    ui->restartButton->setEnabled(true);
}

void MainWindow::on_deleteButton_clicked()
{
    // Remove player info.
    Player::deletePlayer(ui->comboBox->currentIndex()); // Remove player from playerList.
    ui->comboBox->blockSignals(true);
    ui->comboBox->removeItem(ui->comboBox->currentIndex()); // Remove player from comboBox.

    // Set ui elements for selected player.
    if (ui->comboBox->count() > 0)
    {
        ui->nameLabel->setText(Player::playerName(ui->comboBox->currentIndex()));   // Set name to selected player.
        ui->levelLabel->setText(Player::playerLevel(ui->comboBox->currentIndex()));  // Set level to selected player.
    }
    else
    {
        ui->nameLabel->setText("");   // Delete name if last player.
        ui->levelLabel->setText("");  // Delete level if last player.
    }
}

void MainWindow::on_startButton_clicked()
{
    Player::makeMostRecent(ui->comboBox->currentIndex());   // Move selected player in pvz_players.csv to the top (most recent).

    // Set ui elements to selected player.
    ui->comboBox->blockSignals(true);
    ui->comboBox->removeItem(ui->comboBox->currentIndex()); // Remove selected item.
    ui->comboBox->insertItem(0, Player::playerName(0)); // Insert it back at the top.
    ui->comboBox->setCurrentIndex(0);   // Set combo box to top.
    ui->nameLabel->setText(Player::playerName(0));   // Set nameLabel to inputted name.
    ui->levelLabel->setText(Player::playerLevel(0)); // Set levelLabel to level (0 because new player).

    // Enable buttons in case they were disabled before.
    ui->startButton->setEnabled(false);
    ui->newButton->setEnabled(false);
    ui->deleteButton->setEnabled(false);
    ui->restartButton->setEnabled(false);

    QSound::play("C:/Users/Nelson/Downloads/Plants_vs._Zombies_(Main_Theme).wav");    // Play Plants vs. Zombies main theme.

    // Show frontyard image and send signal to start game.
    gameScreen = new GameScreen(ui->graphicsView);
    gameScreen->setFixedSize(ui->graphicsView->size());

    // Make scene and make dimensions same as graphicsView.
    scene = new QGraphicsScene(gameScreen);
    scene->setSceneRect(0, 0, gameScreen->width()-2, gameScreen->height()-2);

    // Add frontyard image to scene.
    QPixmap *frontyard = new QPixmap(":/Images/Frontyard.jpg");
    scene->addPixmap(frontyard->scaledToWidth(gameScreen->width()-4));

    connect(gameScreen,SIGNAL(click()),this,SLOT(addImage()));

    // Set scene and display.
    gameScreen->setScene(scene);
    gameScreen->show();

    moveTimer = new QTimer;
    connect(moveTimer, SIGNAL(timeout()), scene, SLOT(advance()));
    moveTimer->start(60);

    createTimer = new QTimer;
    connect(createTimer, SIGNAL(timeout()), this, SLOT(createSun()));
    createTimer->start(10000);

    destroyTimer = new QTimer;
    connect(destroyTimer, SIGNAL(timeout()), this, SLOT(destroySun()));

    updateSunPointsTimer = new QTimer;
    connect(updateSunPointsTimer, SIGNAL(timeout()), this, SLOT(updateSunPoints()));
    updateSunPointsTimer->start(10);

    // Enable all plant buttons.
    ui->peaShooterButton->setEnabled(true);
    ui->sunFlowerButton->setEnabled(true);
    ui->cherryBombButton->setEnabled(true);
    ui->wallNutButton->setEnabled(true);
    ui->potatoMineButton->setEnabled(true);
    ui->snowPeaButton->setEnabled(true);
    ui->chomperButton->setEnabled(true);
    ui->repeaterButton->setEnabled(true);
}

void MainWindow::on_quitButton_clicked()
{
    close();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_peaShooterButton_clicked()
{
    buttonClicked = 1;
    plantClicked = "peashooter";
}

void MainWindow::on_sunFlowerButton_clicked()
{
    buttonClicked = 1;
    plantClicked = "sunflower";
}

void MainWindow::on_cherryBombButton_clicked()
{
    buttonClicked = 1;
    plantClicked = "cherrybomb";
}

void MainWindow::on_wallNutButton_clicked()
{
    buttonClicked = 1;
    plantClicked = "wallnut";
}

void MainWindow::on_potatoMineButton_clicked()
{
    buttonClicked = 1;
    plantClicked = "potatomine";
}

void MainWindow::on_snowPeaButton_clicked()
{
    buttonClicked = 1;
    plantClicked = "snowpea";
}

void MainWindow::on_chomperButton_clicked()
{
    buttonClicked = 1;
    plantClicked = "chomper";
}

void MainWindow::on_repeaterButton_clicked()
{
    buttonClicked = 1;
    plantClicked = "repeater";
}

void MainWindow::createSun()
{
    sun = new Sun;
    scene->addItem(sun);
    destroyTimer->start(7500);
}

void MainWindow::createSunFlowerSun()
{

}

void MainWindow::destroySun()
{
    destroyTimer->stop();
    if (!sun->sunClicked)
        sun->destroySun();
    sun->sunClicked = false;
}

void MainWindow::destroySunFlowerSun()
{

}

void MainWindow::updateSunPoints()
{
    if (plantClicked == "peashooter" && imageAdded)
        sun->sunPoints -= 100;
    else if (plantClicked == "sunflower" && imageAdded)
        sun->sunPoints -= 50;
    else if (plantClicked == "cherrybomb" && imageAdded)
        sun->sunPoints -= 150;
    else if (plantClicked == "wallnut" && imageAdded)
        sun->sunPoints -= 50;
    else if (plantClicked == "potatomine" && imageAdded)
        sun->sunPoints -= 25;
    else if (plantClicked == "snowpea" && imageAdded)
        sun->sunPoints -= 175;
    else if (plantClicked == "repeater" && imageAdded)
        sun->sunPoints -= 150;
    else if (plantClicked == "chomper" && imageAdded)
        sun->sunPoints -= 200;

    imageAdded = 0;
    plantClicked = "0";
    ui->sunPointsLabel->setText(QString::number(sun->sunPoints));
}

void MainWindow::addImage()
{
    if (plantClicked == "peashooter")// && buttonClicked)
    {
        peaShooter = new PeaShooter;
        peaShooter->setPos(gameScreen->currentGridPoint);
        scene->addItem(peaShooter);

        disconnect(gameScreen,SIGNAL(click()),this,SLOT(addImage()));

        imageAdded = 1;
        buttonClicked = 0;
    }
    else if (plantClicked == "sunflower")// && buttonClicked)
    {
        sunFlower = new SunFlower;
        sunFlower->setPos(gameScreen->currentGridPoint);
        scene->addItem(sunFlower);

        sunFlowerSun = new Sun(gameScreen->currentGridPoint);
        scene->addItem(sunFlowerSun);
        sunFlowerSun->createTimer->start(24000);
        sunFlowerSun->destroyTimer->start(7500);
        connect(sunFlowerSun->destroyTimer, SIGNAL(timeout()), this, SLOT(createSunFlowerSun()));
        connect(sunFlowerSun->destroyTimer, SIGNAL(timeout()), this, SLOT(destroySunFlowerSun()));

        disconnect(gameScreen,SIGNAL(click()),this,SLOT(addImage()));

        imageAdded = 1;
        buttonClicked = 0;
    }
}
