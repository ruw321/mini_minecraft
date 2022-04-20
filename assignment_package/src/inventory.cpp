#include "inventory.h"
#include "ui_inventory.h"
#include <QKeyEvent>
#include <QPixmap>

Inventory::Inventory(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Inventory)
{
    ui->setupUi(this);

    connect(ui->radioButtonGrass, SIGNAL(clicked()), this, SLOT(slot_setCurrBlockToGrass()));
    connect(ui->radioButtonDirt, SIGNAL(clicked()), this, SLOT(slot_setCurrBlockToDirt()));
    connect(ui->radioButtonStone, SIGNAL(clicked()), this, SLOT(slot_setCurrBlockToStone()));
    connect(ui->radioButtonSand, SIGNAL(clicked()), this, SLOT(slot_setCurrBlockToSand()));
    connect(ui->radioButtonSnow, SIGNAL(clicked()), this, SLOT(slot_setCurrBlockToSnow()));
    connect(ui->radioButtonIce, SIGNAL(clicked()), this, SLOT(slot_setCurrBlockToIce()));
    connect(ui->radioButtonRedStone, SIGNAL(clicked()), this, SLOT(slot_setCurrBlockToRedStone()));
    connect(ui->radioButtonPumpkin, SIGNAL(clicked()), this, SLOT(slot_setCurrBlockToPumpkin()));
}

Inventory::~Inventory()
{
    delete ui;
}

void Inventory::keyPressEvent(QKeyEvent *e) {
    if (e->key() == Qt::Key_I) {
        close();
    }
}

// update quantities
void Inventory::slot_setNumGrass(int n) {
    ui->numGrass->display(n);
}

void Inventory::slot_setNumDirt(int n) {
    ui->numDirt->display(n);
}

void Inventory::slot_setNumStone(int n) {
    ui->numStone->display(n);
}

void Inventory::slot_setNumSand(int n) {
    ui->numSand->display(n);
}

void Inventory::slot_setNumSnow(int n) {
    ui->numSnow->display(n);
}

void Inventory::slot_setNumIce(int n) {
    ui->numIce->display(n);
}

void Inventory::slot_setNumRedStone(int n) {
    ui->numRedStone->display(n);
}

void Inventory::slot_setNumPumpkin(int n) {
    ui->numPumpkin->display(n);
}

// set current block
void Inventory::slot_setCurrBlockToGrass() {
    if (ui->numGrass->value() > 0) {
        ui_main->mygl->currBlockType = GRASS;
    } else {
        ui_main->mygl->currBlockType = EMPTY;
    }
}

void Inventory::slot_setCurrBlockToDirt() {
    ui_main->mygl->currBlockType = DIRT;
}

void Inventory::slot_setCurrBlockToStone() {
    ui_main->mygl->currBlockType = STONE;
}

void Inventory::slot_setCurrBlockToSand() {
    ui_main->mygl->currBlockType = SAND;
}

void Inventory::slot_setCurrBlockToSnow() {
    ui_main->mygl->currBlockType = SNOW;
}

void Inventory::slot_setCurrBlockToIce() {
    ui_main->mygl->currBlockType = ICE;
}

void Inventory::slot_setCurrBlockToRedStone() {
    ui_main->mygl->currBlockType = REDSTONE;
}

void Inventory::slot_setCurrBlockToPumpkin() {
    ui_main->mygl->currBlockType = PUMPKIN;
}
