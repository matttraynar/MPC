#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QDebug>
#include <QFileDialog>
#include <QColorDialog>
#include <QMessageBox>
#include <QtGui>
#include <QtCore>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    m_glWidget = new GLWidget(this);
    m_glWidget->lower();

    ui->LayoutForWidget->addWidget(m_glWidget);
    ui->ToolbarChecks->raise();
    ui->settingsCheck->hide();
    ui->settingsToolbar->raise();
    ui->settingsToolbar->hide();
    ui->meshToolbar->raise();

    m_isSimulating = false;

    ui->meshToolbar->removeTab(1);

    ui->settingsToolbar->removeTab(1);
    ui->settingsToolbar->removeTab(2);
    ui->settingsToolbar->removeTab(3);

    connect(this, SIGNAL(passWorldColour(QColor)), m_glWidget, SLOT(setWorldColour(QColor)));
    connect(this, SIGNAL(passPlaneColour(QColor)), m_glWidget, SLOT(setPlaneColour(QColor)));

    connect(this, SIGNAL(passLoadMesh(QString,QString, QVector3D)), m_glWidget, SLOT(addNewMesh(QString,QString, QVector3D)));
    connect(this, SIGNAL(passDeleteMesh(QString)), m_glWidget, SLOT(removeMesh(QString)));
    connect(this, SIGNAL(passMeshShader(QString,ShaderSettings&)), m_glWidget, SLOT(setMeshShader(QString,ShaderSettings&)));

    connect(this, SIGNAL(runDistanceField(QString,DistanceFieldSettings&)), m_glWidget, SLOT(runDistanceField(QString,DistanceFieldSettings&)));
    connect(this, SIGNAL(runSpherePack(QString,SpherePackSettings&)), m_glWidget, SLOT(runSpherePack(QString,SpherePackSettings&)));
    connect(this, SIGNAL(runConstraintGen(QString,ConstraintSettings&)), m_glWidget, SLOT(runConstraints(QString,ConstraintSettings&)));

    connect(this, SIGNAL(passDrawSpheres(QString,bool)), m_glWidget, SLOT(toggleDrawSpheres(QString,bool)));
    connect(this, SIGNAL(passDrawConstraints(QString,bool)), m_glWidget, SLOT(toggleDrawConstraints(QString,bool)));
    connect(this, SIGNAL(passSimulating(bool)), m_glWidget, SLOT(toggleSimulation(bool)));

    connect(this, SIGNAL(passReset()), m_glWidget, SLOT(resetSimulation()));
    connect(this, SIGNAL(passNewStep(float)), m_glWidget, SLOT(setSimulation(float)));

    connect(m_glWidget, SIGNAL(setSphereNumber(int)), this, SLOT(setSphereNumber(int)));
    connect(m_glWidget, SIGNAL(setMeshColour(QVector4D)), this, SLOT(setMeshColour(QVector4D)));
}

MainWindow::~MainWindow()
{
    delete ui;
}

//SLOTS
void MainWindow::setSphereNumber(int number)
{
    ui->numSphereValue->setValue(number);

    auto selected = ui->treeWidget->selectedItems();

    if(selected.length() == 0)
    {
        QMessageBox::critical(this, "Error - No Mesh Selected", "No mesh is selected in the mesh toolbar, settings cannot be saved");
        return;
    }

    QString name = selected[0]->text(0);

    for(uint i = 0; i < m_sphereSettings.size(); ++i)
    {
        if(name == m_sphereSettings[i].first)
        {
            m_sphereSettings[i].second->numSpheres = number;
        }
    }

}

void MainWindow::setMeshColour(QVector4D colour)
{
    QColor newColour(colour.x() * 255.0f,
                                colour.y() * 225.0f,
                                colour.z() * 255.0f,
                                1.0f);

    QPalette palette = ui->meshColour->palette();
    palette.setColor(ui->meshColour->backgroundRole(), newColour);
    ui->meshColour->setAutoFillBackground(true);
    ui->meshColour->setPalette(palette);

    auto selected = ui->treeWidget->selectedItems();

    if(selected.length() == 0)
    {
        return;
    }

    QString name = selected[0]->text(0);

    for(uint i = 0; i < m_shaderSettings.size(); ++i)
    {
        if(name == m_shaderSettings[i].first)
        {
            m_shaderSettings[i].second->red = (float)newColour.red()/255.0f;
            m_shaderSettings[i].second->green = (float)newColour.green()/255.0f;
            m_shaderSettings[i].second->blue = (float)newColour.blue()/255.0f;
        }
    }
}

void MainWindow::addMeshToList(QString name, QString hasSpherePack)
{
    QTreeWidgetItem* mesh = new QTreeWidgetItem(ui->treeWidget);
    mesh->setText(0, name);
    mesh->setText(1, hasSpherePack);
}

void MainWindow::on_loadMeshButton_clicked()
{
    if(ui->filenameLine->text().length() > 0)
    {
        QString message = "Loading Mesh : ";
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
        int maxCount = 0;
        bool allDigits = true;
        for(uint i = 0; i < m_existingMeshes.size(); ++i)
        {
            if(meshName == m_existingMeshes[i])
            {
                QString value;

                auto position = m_shaderSettings[i].first.end();
                position--;
                QChar character = *position;

                if(character.isDigit())
                {
                    while(character.isDigit() && position != value.begin())
                    {
                        value.insert(0, character);
                        position--;
                        character = *position;
                    }

                    if(atoi(value.toStdString().c_str()) > maxCount)
                    {
                        existingCount -= maxCount;
                        maxCount = atoi(value.toStdString().c_str());
                        existingCount += maxCount;
                    }

                }
                else
                {
                    allDigits = false;
                    existingCount++;
                }
            }
            else
            {
                allDigits = false;
            }
        }

        if(m_existingMeshes.size() == 0)
        {
            allDigits = false;
        }

        if(allDigits)
        {
            existingCount++;
        }

        m_existingMeshes.push_back(meshName);

        if(existingCount > 0)
        {
            char newCount[10];
            meshName.append("_");
            meshName.append(itoa(existingCount,newCount,10));
        }
        message.append(meshName);
        ui->statusBar->showMessage(message,1000);

        addMeshToList(meshName,"No");

        m_meshSettings.push_back(std::make_pair(meshName, new MeshSettings));
        m_shaderSettings.push_back(std::make_pair(meshName, new ShaderSettings));
        m_distanceSettings.push_back(std::make_pair(meshName, new DistanceFieldSettings));
        m_sphereSettings.push_back(std::make_pair(meshName, new SpherePackSettings));
        m_constraintSettings.push_back(std::make_pair(meshName, new ConstraintSettings));

        ui->filenameLine->setText(QString());

        QVector3D position(ui->positionX->value(),
                                     ui->positionY->value(),
                                     ui->positionZ->value());

        emit passLoadMesh(fileName, meshName, position);

        QVector4D meshColour = m_glWidget->getLastAddedColour();

        m_shaderSettings[m_shaderSettings.size() - 1].second->red = meshColour.x();
        m_shaderSettings[m_shaderSettings.size() - 1].second->green = meshColour.y();
        m_shaderSettings[m_shaderSettings.size() - 1].second->blue = meshColour.z();
    }
}

void MainWindow::on_deleteMeshButton_clicked()
{
    auto selected = ui->treeWidget->selectedItems();

    if(selected.length() == 0)
    {
        QMessageBox::critical(this, "Error - No Mesh Selected", "No mesh is selected in the mesh toolbar, nothing to delete");
        return;
    }

    QString name = selected[0]->text(0);

    uint size = m_shaderSettings.size();

    for(uint i = 0; i < size; ++i)
    {
        if(name == m_shaderSettings[i].first)
        {
            emit passDeleteMesh(name);

            m_existingMeshes.erase(m_existingMeshes.begin() + i);
            m_shaderSettings.erase(m_shaderSettings.begin() + i);
            m_distanceSettings.erase(m_distanceSettings.begin() + i);
            m_sphereSettings.erase(m_sphereSettings.begin() + i);
            m_constraintSettings.erase(m_constraintSettings.begin() + i);

            int i = ui->treeWidget->indexOfTopLevelItem(ui->treeWidget->currentItem());
            ui->treeWidget->takeTopLevelItem(i);

            auto selected = ui->treeWidget->selectedItems();

            if(selected.length() == 0)
            {
                ui->meshToolbar->removeTab(1);

                for(uint i = 0; i < ui->settingsToolbar->count(); ++i)
                {
                    ui->settingsToolbar->removeTab(1);
                }

                ui->settingsCheck->setChecked(false);
                ui->settingsCheck->hide();
            }
            break;
        }
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
            if(m_meshSettings[i].second->hasDistanceField)
            {
                ui->settingsToolbar->insertTab(1, ui->spherepack, "Sphere Packing");
            }
            else
            {
                ui->settingsToolbar->removeTab(1);
            }

            if(m_meshSettings[i].second->hasSpherePack)
            {
                ui->settingsToolbar->insertTab(2, ui->constraints, "Constraint Manager");
                ui->settingsToolbar->insertTab(3, ui->simulation, "Simulation");
            }
            else if(m_meshSettings[i].second->hasDistanceField)
            {
                ui->settingsToolbar->removeTab(2);
                ui->settingsToolbar->removeTab(2);
            }
            else
            {
                ui->settingsToolbar->removeTab(1);
                ui->settingsToolbar->removeTab(1);
                ui->settingsToolbar->removeTab(1);
            }


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

            ui->numSphereValue->setValue(m_sphereSettings[i].second->numSpheres);
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

//Shader settings

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

    ui->settingsCheck->show();
    ui->settingsCheck->setChecked(true);

    ui->deleteMeshButton->setEnabled(true);

    auto selected = ui->treeWidget->selectedItems();

    if(selected.length() == 0)
    {
        return;
    }

    QString name = selected[0]->text(0);

    for(uint i = 0; i < m_shaderSettings.size(); ++i)
    {
        if(name == m_shaderSettings[i].first)
        {
            name.append(" has been selected");
            ui->statusBar->showMessage(name, 1000);

            QColor newColour(m_shaderSettings[i].second->red * 255.0f,
                                        m_shaderSettings[i].second->green * 225.0f,
                                        m_shaderSettings[i].second->blue * 255.0f,
                                        1.0f);

            QPalette palette = ui->meshColour->palette();
            palette.setColor(ui->meshColour->backgroundRole(), newColour);
            ui->meshColour->setAutoFillBackground(true);
            ui->meshColour->setPalette(palette);
        }
    }

}

//Distance field settings

void MainWindow::on_resolutionX_valueChanged(double arg1)
{
    auto selected = ui->treeWidget->selectedItems();

    if(selected.length() == 0)
    {
        QMessageBox::critical(this, "Error - No Mesh Selected", "No mesh is selected in the mesh toolbar, settings cannot be saved");
        return;
    }

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

    if(selected.length() == 0)
    {
        QMessageBox::critical(this, "Error - No Mesh Selected", "No mesh is selected in the mesh toolbar, settings cannot be saved");
        return;
    }

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

    if(selected.length() == 0)
    {
        QMessageBox::critical(this, "Error - No Mesh Selected", "No mesh is selected in the mesh toolbar, settings cannot be saved");
        return;
    }

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

    if(selected.length() == 0)
    {
        QMessageBox::critical(this, "Error - No Mesh Selected", "No mesh is selected in the mesh toolbar, settings cannot be saved");
        return;
    }

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

    if(selected.length() == 0)
    {
        QMessageBox::critical(this, "Error - No Mesh Selected", "No mesh is selected in the mesh toolbar, settings cannot be saved");
        return;
    }

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

    if(selected.length() == 0)
    {
        QMessageBox::critical(this, "Error - No Mesh Selected", "No mesh is selected in the mesh toolbar, settings cannot be saved");
        return;
    }

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

    if(selected.length() == 0)
    {
        QMessageBox::critical(this, "Error - No Mesh Selected", "No mesh is selected in the mesh toolbar, cannot create distance field");
        return;
    }

    QString name = selected[0]->text(0);

    for(uint i = 0; i < m_distanceSettings.size(); ++i)
    {
        if(name == m_distanceSettings[i].first)
        {
            QString message = "Generating distance field for ";
            message.append(name);
            message.append("...");
            ui->statusBar->showMessage(message, 2500);

            emit runDistanceField(name, *m_distanceSettings[i].second);            

            m_meshSettings[i].second->hasDistanceField = true;

            ui->settingsToolbar->insertTab(1, ui->spherepack, "Sphere Packing");


            ui->statusBar->showMessage("Distance field complete", 1000);
        }
    }
}

//Sphere pack settings

void MainWindow::on_sphereSizeValue_valueChanged(double arg1)
{
    auto selected = ui->treeWidget->selectedItems();

    if(selected.length() == 0)
    {
        QMessageBox::critical(this, "Error - No Mesh Selected", "No mesh is selected in the mesh toolbar, settings cannot be saved");
        return;
    }

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

    if(selected.length() == 0)
    {
        QMessageBox::critical(this, "Error - No Mesh Selected", "No mesh is selected in the mesh toolbar, settings cannot be saved");
        return;
    }

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

    if(selected.length() == 0)
    {
        QMessageBox::critical(this, "Error - No Mesh Selected", "No mesh is selected in the mesh toolbar, settings cannot be saved");
        return;
    }

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

    if(selected.length() == 0)
    {
        QMessageBox::critical(this, "Error - No Mesh Selected", "No mesh is selected in the mesh toolbar, settings cannot be saved");
        return;
    }

    QString name = selected[0]->text(0);

    for(uint i = 0; i < m_sphereSettings.size(); ++i)
    {
        if(name == m_sphereSettings[i].first)
        {
            m_sphereSettings[i].second->isMax = checked;
        }
    }
}

void MainWindow::on_cullInner_toggled(bool checked)
{
    auto selected = ui->treeWidget->selectedItems();

    if(selected.length() == 0)
    {
        QMessageBox::critical(this, "Error - No Mesh Selected", "No mesh is selected in the mesh toolbar, settings cannot be saved");
        return;
    }

    QString name = selected[0]->text(0);

    for(uint i = 0; i < m_sphereSettings.size(); ++i)
    {
        if(name == m_sphereSettings[i].first)
        {
            m_sphereSettings[i].second->cullInner = checked;
        }
    }
}

void MainWindow::on_cullOuter_toggled(bool checked)
{
    auto selected = ui->treeWidget->selectedItems();

    if(selected.length() == 0)
    {
        QMessageBox::critical(this, "Error - No Mesh Selected", "No mesh is selected in the mesh toolbar, settings cannot be saved");
        return;
    }

    QString name = selected[0]->text(0);

    for(uint i = 0; i < m_sphereSettings.size(); ++i)
    {
        if(name == m_sphereSettings[i].first)
        {
            m_sphereSettings[i].second->cullOuter = checked;
        }
    }
}

void MainWindow::on_drawSpheres_toggled(bool checked)
{
    auto selected = ui->treeWidget->selectedItems();

    if(selected.length() == 0)
    {
        QMessageBox::critical(this, "Error - No Mesh Selected", "No mesh is selected in the mesh toolbar, settings cannot be saved");
        return;
    }

    QString name = selected[0]->text(0);

    for(uint i = 0; i < m_sphereSettings.size(); ++i)
    {
        if(name == m_sphereSettings[i].first)
        {
            m_sphereSettings[i].second->drawSpheres = checked;

            emit passDrawSpheres(name, checked);
        }
    }
}

void MainWindow::on_packSphereButton_clicked()
{
    auto selected = ui->treeWidget->selectedItems();

    if(selected.length() == 0)
    {
        QMessageBox::critical(this, "Error - No Mesh Selected", "No mesh is selected in the mesh toolbar, cannot run sphere pack algorithm");
        return;
    }

    QString name = selected[0]->text(0);

    for(uint i = 0; i < m_sphereSettings.size(); ++i)
    {
        if(name == m_sphereSettings[i].first)
        {
            QString message = "Running sphere packing algorithm on ";
            message.append(name);
            message.append("...");
            ui->statusBar->showMessage(message);

            emit runSpherePack(name, *m_sphereSettings[i].second);

            m_meshSettings[i].second->hasSpherePack = true;

            ui->settingsToolbar->insertTab(2, ui->constraints, "Constraint Manager");
            ui->settingsToolbar->insertTab(3, ui->simulation, "Simulation");

            ui->statusBar->showMessage("Sphere packing complete", 1000);
        }
    }
}

//Constraint settings

void MainWindow::on_addConstraints_clicked()
{
    auto selected = ui->treeWidget->selectedItems();

    if(selected.length() == 0)
    {
        QMessageBox::critical(this, "Error - No Mesh Selected", "No mesh is selected in the mesh toolbar, cannot create constraints");
        return;
    }

    QString name = selected[0]->text(0);

    for(uint i = 0; i < m_constraintSettings.size(); ++i)
    {
        if(name == m_constraintSettings[i].first)
        {
            QString message = "Adding constraints to ";
            message.append(name);
            message.append("...");
            ui->statusBar->showMessage(message, 2500);
            emit runConstraintGen(name, *m_constraintSettings[i].second);


            ui->statusBar->showMessage("Constraints complete", 1000);
        }
    }
}

void MainWindow::on_drawConstraintCheck_toggled(bool checked)
{
    auto selected = ui->treeWidget->selectedItems();

    if(selected.length() == 0)
    {
        QMessageBox::critical(this, "Error - No Mesh Selected", "No mesh is selected in the mesh toolbar, settings cannot be saved");
        return;
    }

    QString name = selected[0]->text(0);

    for(uint i = 0; i < m_constraintSettings.size(); ++i)
    {
        if(name == m_constraintSettings[i].first)
        {
            m_constraintSettings[i].second->isDrawing = checked;

            emit passDrawConstraints(name, checked);
        }
    }
}

void MainWindow::on_limitConstraints_toggled(bool checked)
{
    auto selected = ui->treeWidget->selectedItems();

    if(selected.length() == 0)
    {
        QMessageBox::critical(this, "Error - No Mesh Selected", "No mesh is selected in the mesh toolbar, settings cannot be saved");
        return;
    }

    QString name = selected[0]->text(0);

    for(uint i = 0; i < m_constraintSettings.size(); ++i)
    {
        if(name == m_constraintSettings[i].first)
        {
            m_constraintSettings[i].second->isMax = checked;
        }
    }
}

void MainWindow::on_maxConstraintsValue_valueChanged(int arg1)
{
    auto selected = ui->treeWidget->selectedItems();

    if(selected.length() == 0)
    {
        QMessageBox::critical(this, "Error - No Mesh Selected", "No mesh is selected in the mesh toolbar, settings cannot be saved");
        return;
    }

    QString name = selected[0]->text(0);

    for(uint i = 0; i < m_constraintSettings.size(); ++i)
    {
        if(name == m_constraintSettings[i].first)
        {
            m_constraintSettings[i].second->maxNumber = arg1;
        }
    }
}

void MainWindow::on_horizontalSlider_valueChanged(int value)
{
    ui->strengthValue->setValue(float(value)/100.0f);
}

void MainWindow::on_strengthValue_valueChanged(double arg1)
{
    ui->horizontalSlider->setValue((int)(arg1 * 100));

    auto selected = ui->treeWidget->selectedItems();

    if(selected.length() == 0)
    {
        QMessageBox::critical(this, "Error - No Mesh Selected", "No mesh is selected in the mesh toolbar, settings cannot be saved");
        return;
    }

    QString name = selected[0]->text(0);

    for(uint i = 0; i < m_constraintSettings.size(); ++i)
    {
        if(name == m_constraintSettings[i].first)
        {
            m_constraintSettings[i].second->constraintStrength = arg1;
        }
    }
}

//Simulation settings

void MainWindow::on_runButton_clicked()
{
    ui->statusBar->showMessage("Running simulation", 1000);

    ui->runButton->setEnabled(false);
    ui->pauseButton->setEnabled(true);
    ui->resetButton->setEnabled(true);

    emit passSimulating(true);
}

void MainWindow::on_pauseButton_clicked()
{
    ui->statusBar->showMessage("Simulation is paused");

    ui->runButton->setEnabled(true);
    ui->pauseButton->setEnabled(false);
    ui->resetButton->setEnabled(false);

    emit passSimulating(false);
}

void MainWindow::on_resetButton_clicked()
{
    ui->statusBar->showMessage("Simulation has been reset", 1000);

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
        emit passNewStep(1.0f/25.0f);
        break;

    default:
        emit passNewStep(1.0f/60.0f);
        break;
    }
}
