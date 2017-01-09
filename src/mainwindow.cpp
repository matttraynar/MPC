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

    m_isSimulating = false;

    ui->meshToolbar->removeTab(1);

    connect(this, SIGNAL(passWorldColour(QColor)), m_glWidget, SLOT(setWorldColour(QColor)));
    connect(this, SIGNAL(passPlaneColour(QColor)), m_glWidget, SLOT(setPlaneColour(QColor)));

    connect(this, SIGNAL(passLoadMesh(QString,QString, QVector3D)), m_glWidget, SLOT(addNewMesh(QString,QString, QVector3D)));
    connect(this, SIGNAL(passMeshShader(QString,ShaderSettings&)), m_glWidget, SLOT(setMeshShader(QString,ShaderSettings&)));

    connect(this, SIGNAL(runDistanceField(QString,DistanceFieldSettings&)), m_glWidget, SLOT(runDistanceField(QString,DistanceFieldSettings&)));
    connect(this, SIGNAL(runSpherePack(QString,SpherePackSettings&)), m_glWidget, SLOT(runSpherePack(QString,SpherePackSettings&)));

    connect(this, SIGNAL(passSimulating(bool)), m_glWidget, SLOT(toggleSimulation(bool)));

    connect(this, SIGNAL(passReset()), m_glWidget, SLOT(resetSimulation()));
    connect(this, SIGNAL(passNewStep(float)), m_glWidget, SLOT(setSimulation(float)));
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
        for(uint i = 0; i < m_shaderSettings.size(); ++i)
        {
            if(meshName == m_shaderSettings[i].first)
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

        m_shaderSettings.push_back(std::make_pair(meshName, new ShaderSettings));
        m_distanceSettings.push_back(std::make_pair(meshName, new DistanceFieldSettings));
        m_sphereSettings.push_back(std::make_pair(meshName, new SpherePackSettings));
        m_constraintSettings.push_back(std::make_pair(meshName, new ConstraintSettings));

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
    for(uint i = 0; i < m_shaderSettings.size(); ++i)
    {
        if(item->text(0) == m_shaderSettings[i].first)
        {
            //Update shader tab
            QColor colour(m_shaderSettings[i].second->red * 255,
                                m_shaderSettings[i].second->green * 255,
                                m_shaderSettings[i].second->blue * 255);

            QPalette palette = ui->meshColour->palette();
            palette.setColor(ui->meshColour->backgroundRole(), colour);

            ui->meshColour->setAutoFillBackground(true);
            ui->meshColour->setPalette(palette);

            ui->wireframeCheck->setChecked(m_shaderSettings[i].second->wireframe);
            ui->drawCheck->setChecked(m_shaderSettings[i].second->drawMesh);
            ui->skinCheck->setChecked(m_shaderSettings[i].second->skinMesh);

            //Update distance tab
            ui->resolutionX->setValue(m_distanceSettings[i].second->xRes);
            ui->resolutionY->setValue(m_distanceSettings[i].second->yRes);
            ui->resolutionZ->setValue(m_distanceSettings[i].second->zRes);

            ui->innerValue->setValue(m_distanceSettings[i].second->innerMargin);
            ui->outerValue->setValue(m_distanceSettings[i].second->outerMargin);
            ui->boundsValue->setValue(m_distanceSettings[i].second->meshMargin);

            //Update sphere pack
            ui->sphereSizeValue->setValue(m_sphereSettings[i].second->radius);
            ui->sphereSpacingValue->setValue(m_sphereSettings[i].second->spacing);
            ui->limitSpheres->setChecked(m_sphereSettings[i].second->isMax);

            if(ui->limitSpheres->isChecked())
            {
                ui->maxSphereNumValue->setValue(m_sphereSettings[i].second->maxNumber);
            }

            ui->drawSpheres->setChecked(m_sphereSettings[i].second->drawSpheres);

            //Update constraint tab
            ui->limitConstraints->setChecked(m_constraintSettings[i].second->isMax);

            if(ui->limitConstraints->isChecked())
            {
                ui->maxConstraintsValue->setValue(m_constraintSettings[i].second->maxNumber);
            }

            ui->strengthValue->setValue(m_constraintSettings[i].second->constraintStrength);
            ui->drawConstraintCheck->setChecked(m_constraintSettings[i].second->isDrawing);
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

        for(uint i = 0; i < m_shaderSettings.size(); ++i)
        {
            if(name == m_shaderSettings[i].first)
            {
                m_shaderSettings[i].second->red = (float)colour.red()/255.0f;
                m_shaderSettings[i].second->green = (float)colour.green()/255.0f;
                m_shaderSettings[i].second->blue = (float)colour.blue()/255.0f;

                emit passMeshShader(name, *m_shaderSettings[i].second);
            }
        }
    }
}

void MainWindow::on_wireframeCheck_toggled(bool checked)
{
    auto selected = ui->treeWidget->selectedItems();

    QString name = selected[0]->text(0);

    for(uint i = 0; i < m_shaderSettings.size(); ++i)
    {
        if(name == m_shaderSettings[i].first)
        {
            m_shaderSettings[i].second->wireframe = checked;

            emit passMeshShader(name, *m_shaderSettings[i].second);
        }
    }
}

void MainWindow::on_drawCheck_toggled(bool checked)
{
    auto selected = ui->treeWidget->selectedItems();

    QString name = selected[0]->text(0);

    for(uint i = 0; i < m_shaderSettings.size(); ++i)
    {
        if(name == m_shaderSettings[i].first)
        {
            m_shaderSettings[i].second->drawMesh = checked;

            emit passMeshShader(name, *m_shaderSettings[i].second);
        }
    }
}

void MainWindow::on_skinCheck_toggled(bool checked)
{
    auto selected = ui->treeWidget->selectedItems();

    QString name = selected[0]->text(0);

    for(uint i = 0; i < m_shaderSettings.size(); ++i)
    {
        if(name == m_shaderSettings[i].first)
        {
            m_shaderSettings[i].second->skinMesh = checked;

            emit passMeshShader(name, *m_shaderSettings[i].second);
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
    ui->runButton->setEnabled(false);
    ui->pauseButton->setEnabled(true);
    ui->resetButton->setEnabled(true);

    emit passSimulating(true);
}

void MainWindow::on_pauseButton_clicked()
{
    ui->runButton->setEnabled(true);
    ui->pauseButton->setEnabled(false);
    ui->resetButton->setEnabled(false);

    emit passSimulating(false);
}

void MainWindow::on_resetButton_clicked()
{
    ui->runButton->setEnabled(true);
    ui->pauseButton->setEnabled(false);
    ui->resetButton->setEnabled(false);

    emit passReset();
}

void MainWindow::on_comboBox_currentIndexChanged(int index)
{
    switch(index)
    {
    case 0:
        emit passNewStep(1.0f/60.0f);
        break;

    case 1:
        emit passNewStep(1.0f/35.0f);
        break;

    case 2:
        emit passNewStep(1.0f/35.0f);
        break;

    default:
        emit passNewStep(1.0f/60.0f);
        break;
    }
}

//Distance field settings
void MainWindow::on_resolutionX_valueChanged(double arg1)
{
    auto selected = ui->treeWidget->selectedItems();

    QString name = selected[0]->text(0);

    for(uint i = 0; i < m_distanceSettings.size(); ++i)
    {
        if(name == m_distanceSettings[i].first)
        {
            m_distanceSettings[i].second->xRes = arg1;
        }
    }
}

void MainWindow::on_resolutionY_valueChanged(double arg1)
{
    auto selected = ui->treeWidget->selectedItems();

    QString name = selected[0]->text(0);

    for(uint i = 0; i < m_distanceSettings.size(); ++i)
    {
        if(name == m_distanceSettings[i].first)
        {
            m_distanceSettings[i].second->yRes = arg1;
        }
    }
}

void MainWindow::on_resolutionZ_valueChanged(double arg1)
{
    auto selected = ui->treeWidget->selectedItems();

    QString name = selected[0]->text(0);

    for(uint i = 0; i < m_distanceSettings.size(); ++i)
    {
        if(name == m_distanceSettings[i].first)
        {
            m_distanceSettings[i].second->zRes = arg1;
        }
    }
}

void MainWindow::on_innerValue_valueChanged(double arg1)
{
    auto selected = ui->treeWidget->selectedItems();

    QString name = selected[0]->text(0);

    for(uint i = 0; i < m_distanceSettings.size(); ++i)
    {
        if(name == m_distanceSettings[i].first)
        {
            m_distanceSettings[i].second->innerMargin = arg1;
        }
    }
}

void MainWindow::on_outerValue_valueChanged(double arg1)
{
    auto selected = ui->treeWidget->selectedItems();

    QString name = selected[0]->text(0);

    for(uint i = 0; i < m_distanceSettings.size(); ++i)
    {
        if(name == m_distanceSettings[i].first)
        {
            m_distanceSettings[i].second->outerMargin = arg1;
        }
    }
}

void MainWindow::on_boundsValue_valueChanged(double arg1)
{
    auto selected = ui->treeWidget->selectedItems();

    QString name = selected[0]->text(0);

    for(uint i = 0; i < m_distanceSettings.size(); ++i)
    {
        if(name == m_distanceSettings[i].first)
        {
            m_distanceSettings[i].second->meshMargin = arg1;
        }
    }
}

void MainWindow::on_generateDistanceField_clicked()
{
    auto selected = ui->treeWidget->selectedItems();

    QString name = selected[0]->text(0);

    for(uint i = 0; i < m_distanceSettings.size(); ++i)
    {
        if(name == m_distanceSettings[i].first)
        {
            emit runDistanceField(name, *m_distanceSettings[i].second);
        }
    }
}

//Sphere pack settings

void MainWindow::on_sphereSizeValue_valueChanged(double arg1)
{
    auto selected = ui->treeWidget->selectedItems();

    QString name = selected[0]->text(0);

    for(uint i = 0; i < m_sphereSettings.size(); ++i)
    {
        if(name == m_sphereSettings[i].first)
        {
            m_sphereSettings[i].second->radius = arg1;
        }
    }
}

void MainWindow::on_sphereSpacingValue_valueChanged(double arg1)
{
    auto selected = ui->treeWidget->selectedItems();

    QString name = selected[0]->text(0);

    for(uint i = 0; i < m_sphereSettings.size(); ++i)
    {
        if(name == m_sphereSettings[i].first)
        {
            m_sphereSettings[i].second->spacing = arg1;
        }
    }
}

void MainWindow::on_maxSphereNumValue_valueChanged(int arg1)
{
    auto selected = ui->treeWidget->selectedItems();

    QString name = selected[0]->text(0);

    for(uint i = 0; i < m_sphereSettings.size(); ++i)
    {
        if(name == m_sphereSettings[i].first)
        {
            m_sphereSettings[i].second->maxNumber = arg1;
        }
    }
}

void MainWindow::on_limitSpheres_toggled(bool checked)
{
    auto selected = ui->treeWidget->selectedItems();

    QString name = selected[0]->text(0);

    for(uint i = 0; i < m_sphereSettings.size(); ++i)
    {
        if(name == m_sphereSettings[i].first)
        {
            m_sphereSettings[i].second->isMax = checked;
        }
    }
}

void MainWindow::on_drawSpheres_toggled(bool checked)
{
    auto selected = ui->treeWidget->selectedItems();

    QString name = selected[0]->text(0);

    for(uint i = 0; i < m_sphereSettings.size(); ++i)
    {
        if(name == m_sphereSettings[i].first)
        {
            m_sphereSettings[i].second->drawSpheres = checked;
        }
    }
}

void MainWindow::on_packSphereButton_clicked()
{
    auto selected = ui->treeWidget->selectedItems();

    QString name = selected[0]->text(0);

    for(uint i = 0; i < m_sphereSettings.size(); ++i)
    {
        if(name == m_sphereSettings[i].first)
        {
            emit runSpherePack(name, *m_sphereSettings[i].second);
        }
    }
}
