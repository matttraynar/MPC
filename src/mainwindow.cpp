#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QDebug>
#include <QFileDialog>
#include <QColorDialog>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    m_glWidget = new GLWidget(this);
    m_glWidget->lower();

    ui->LayoutForWidget->addWidget(m_glWidget);
    ui->ToolbarChecks->raise();
    ui->settingsToolbar->raise();
    ui->meshToolbar->raise();

    ui->meshToolbar->removeTab(1);

    connect(this, SIGNAL(passMeshColour(QColor,QString)), m_glWidget, SLOT(setMeshColour(QColor,QString)));
    connect(this, SIGNAL(passWorldColour(QColor)), m_glWidget, SLOT(setWorldColour(QColor)));
    connect(this, SIGNAL(passPlaneColour(QColor)), m_glWidget, SLOT(setPlaneColour(QColor)));
    connect(this, SIGNAL(passLoadMesh(QString,QString, QVector3D)), m_glWidget, SLOT(addNewMesh(QString,QString, QVector3D)));
    connect(this, SIGNAL(passDrawMesh(bool,QString)), m_glWidget, SLOT(toggleDrawMesh(bool,QString)));
    connect(this, SIGNAL(passWireframMesh(bool,QString)), m_glWidget, SLOT(toggleWireframeMesh(bool,QString)));
    connect(this, SIGNAL(passSimulating(bool)), m_glWidget, SLOT(toggleSimulation(bool)));
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::addMeshToList(QString name, QString hasSpherePack)
{
    QTreeWidgetItem* mesh = new QTreeWidgetItem(ui->treeWidget);
    mesh->setText(0, name);
    mesh->setText(1, hasSpherePack);
}

void MainWindow::on_horizontalSlider_valueChanged(int value)
{
    ui->strengthValue->setValue(float(value)/100.0f);
}

void MainWindow::on_loadMeshButton_clicked()
{
    if(ui->filenameLine->text().length() > 0)
    {
        QString fileName = ui->filenameLine->text();
        QString meshName;

        auto character = fileName.end();

        while((*character != QChar('/'))&& character != fileName.begin())
        {
            meshName.insert(0, *character);
            character--;
        }

        meshName.truncate(meshName.length() - 5);

        int existingCount = 0;
        for(uint i = 0; i < m_settings.size(); ++i)
        {
            if(meshName == m_settings[i].first)
            {
                existingCount++;
            }
        }

        if(existingCount > 0)
        {
            char newCount[10];
            meshName.append("_");
            meshName.append(itoa(existingCount,newCount,10));
        }

        addMeshToList(meshName,"No");

        m_settings.push_back(std::make_pair(meshName, new ShaderSettings));

        ui->filenameLine->setText(QString());

        QVector3D position(ui->positionX->value(),
                                     ui->positionY->value(),
                                     ui->positionZ->value());

        emit passLoadMesh(fileName, meshName, position);
    }
}

void MainWindow::on_filenameButton_clicked()
{
    QString filename = QFileDialog::getOpenFileName(this, tr("Open File"), QString(), "Obj Files (*.obj)");

    ui->filenameLine->setText(filename);
}

void MainWindow::on_treeWidget_itemClicked(QTreeWidgetItem *item, int column)
{
    for(uint i = 0; i < m_settings.size(); ++i)
    {
        if(item->text(0) == m_settings[i].first)
        {
            QColor colour(m_settings[i].second->red * 255, m_settings[i].second->green * 255, m_settings[i].second->blue * 255);
            QPalette palette = ui->meshColour->palette();
            palette.setColor(ui->meshColour->backgroundRole(), colour);
            ui->meshColour->setAutoFillBackground(true);
            ui->meshColour->setPalette(palette);

            ui->wireframeCheck->setChecked(m_settings[i].second->wireframe);
            ui->drawCheck->setChecked(m_settings[i].second->drawMesh);
            ui->skinCheck->setChecked(m_settings[i].second->skinMesh);
        }
    }
}

void MainWindow::on_meshColourButton_clicked()
{

    QColor colour = QColorDialog::getColor(Qt::white,this,"Choose Mesh Colour");

    if(colour.isValid())
    {
        QPalette palette = ui->meshColour->palette();
        palette.setColor(ui->meshColour->backgroundRole(), colour);
        ui->meshColour->setAutoFillBackground(true);
        ui->meshColour->setPalette(palette);

        auto selected = ui->treeWidget->selectedItems();

        QString name = selected[0]->text(0);

        for(uint i = 0; i < m_settings.size(); ++i)
        {
            if(name == m_settings[i].first)
            {
                m_settings[i].second->red = (float)colour.red()/255.0f;
                m_settings[i].second->green = (float)colour.green()/255.0f;
                m_settings[i].second->blue = (float)colour.blue()/255.0f;

                emit passMeshColour(colour, name);
            }
        }
    }
}

void MainWindow::on_wireframeCheck_toggled(bool checked)
{
    auto selected = ui->treeWidget->selectedItems();

    QString name = selected[0]->text(0);

    for(uint i = 0; i < m_settings.size(); ++i)
    {
        if(name == m_settings[i].first)
        {
            m_settings[i].second->wireframe = checked;
            emit passWireframMesh(checked, name);
        }
    }
}

void MainWindow::on_drawCheck_toggled(bool checked)
{
    auto selected = ui->treeWidget->selectedItems();

    QString name = selected[0]->text(0);

    for(uint i = 0; i < m_settings.size(); ++i)
    {
        if(name == m_settings[i].first)
        {
            m_settings[i].second->drawMesh = checked;
            emit passDrawMesh(checked, name);
        }
    }
}

void MainWindow::on_skinCheck_toggled(bool checked)
{
    auto selected = ui->treeWidget->selectedItems();

    QString name = selected[0]->text(0);

    for(uint i = 0; i < m_settings.size(); ++i)
    {
        if(name == m_settings[i].first)
        {
            m_settings[i].second->skinMesh = checked;
        }
    }
}

void MainWindow::on_groundColourButton_clicked()
{
    QColor colour = QColorDialog::getColor(Qt::white,this,"Choose Mesh Colour");

    if(colour.isValid())
    {
        QPalette palette = ui->groundColour->palette();
        palette.setColor(ui->groundColour->backgroundRole(), colour);
        ui->groundColour->setAutoFillBackground(true);
        ui->groundColour->setPalette(palette);

        emit passPlaneColour(colour);
    }
}

void MainWindow::on_worldColourButton_clicked()
{
    QColor colour = QColorDialog::getColor(Qt::white,this,"Choose Mesh Colour");

    if(colour.isValid())
    {
        QPalette palette = ui->worldColour->palette();
        palette.setColor(ui->worldColour->backgroundRole(), colour);
        ui->worldColour->setAutoFillBackground(true);
        ui->worldColour->setPalette(palette);

        emit passWorldColour(colour);
    }
}

void MainWindow::on_treeWidget_itemSelectionChanged()
{
    ui->meshToolbar->insertTab(1, ui->Shading, "Shading");
}

void MainWindow::on_runButton_clicked()
{
    emit passSimulating(true);
}
