#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTreeWidgetItem>

#include "glwidget.h"
#include "settings.h"

#include <vector>
#include <memory>

//Similar to the source file of this class this is mainly just signals and slots
//and should hopefully be easy to understand

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    void addMeshToList(QString name, QString hasSpherePack);

signals:
    void passWorldColour(QColor colour);
    void passPlaneColour(QColor colour);

    void passLoadMesh(QString fileName, QString meshName, QVector3D position);
    void passDeleteMesh(QString fileName);

    void passMeshShader(QString meshName, ShaderSettings &settings);

    void runDistanceField(QString meshName, DistanceFieldSettings &settings);
    void runSpherePack(QString meshName, SpherePackSettings &settings);
    void runConstraintGen(QString meshName, ConstraintSettings &settings);

    void passDrawSpheres(QString meshName, bool drawSpheres);
    void passDrawConstraints(QString meshName, bool drawConstraints);
    void passSimulating(bool isSimulating);

    void passReset();
    void passNewStep(float stepValue);

    void passMoveDown();
    void passMoveUp();
    void passStopMoving();

private slots:
    void setSphereNumber(int number);

    void setMeshColour(QVector4D colour);

    void on_horizontalSlider_valueChanged(int value);

    void on_loadMeshButton_clicked();

    void on_filenameButton_clicked();

    void on_treeWidget_itemClicked(QTreeWidgetItem *item, int column);

    void on_meshColourButton_clicked();

    void on_wireframeCheck_toggled(bool checked);

    void on_drawCheck_toggled(bool checked);

    void on_skinCheck_toggled(bool checked);

    void on_groundColourButton_clicked();

    void on_worldColourButton_clicked();

    void on_treeWidget_itemSelectionChanged();

    void on_runButton_clicked();

    void on_pauseButton_clicked();

    void on_resetButton_clicked();

    void on_comboBox_currentIndexChanged(int index);

    void on_resolutionX_valueChanged(double arg1);

    void on_resolutionY_valueChanged(double arg1);

    void on_resolutionZ_valueChanged(double arg1);

    void on_innerValue_valueChanged(double arg1);

    void on_outerValue_valueChanged(double arg1);

    void on_boundsValue_valueChanged(double arg1);

    void on_sphereSizeValue_valueChanged(double arg1);

    void on_sphereSpacingValue_valueChanged(double arg1);

    void on_maxSphereNumValue_valueChanged(int arg1);

    void on_limitSpheres_toggled(bool checked);

    void on_drawSpheres_toggled(bool checked);

    void on_generateDistanceField_clicked();

    void on_packSphereButton_clicked();

    void on_addConstraints_clicked();

    void on_drawConstraintCheck_toggled(bool checked);

    void on_limitConstraints_toggled(bool checked);

    void on_maxConstraintsValue_valueChanged(int arg1);

    void on_strengthValue_valueChanged(double arg1);

    void on_deleteMeshButton_clicked();

    void on_cullInner_toggled(bool checked);

    void on_cullOuter_toggled(bool checked);

    void on_action1_Cube_triggered();

    void on_actionDragon_triggered();

    void on_actionBunny_triggered();

    void on_actionStep_triggered();

    void on_actionCollision_Example_1_triggered();

    void on_actionCollision_Example_2_triggered();

    void on_actionCollision_Example_3_triggered();

    void on_moveDown_pressed();

    void on_moveUp_pressed();

    void on_moveDown_released();

    void on_moveUp_released();

    void on_fixed_toggled(bool checked);

    void on_spring_toggled(bool checked);

    void on_actionCollision_Example_4_triggered();

private:
    Ui::MainWindow *ui;

    GLWidget* m_glWidget;

    bool m_isSimulating;

    std::vector<QString> m_existingMeshes;
    std::vector< std::pair< QString, MeshSettings*>> m_meshSettings;
    std::vector< std::pair< QString, ShaderSettings*>> m_shaderSettings;
    std::vector< std::pair< QString, DistanceFieldSettings*>> m_distanceSettings;
    std::vector< std::pair< QString, SpherePackSettings*>> m_sphereSettings;
    std::vector< std::pair< QString, ConstraintSettings*>> m_constraintSettings;

    std::vector<QTreeWidgetItem* > m_treeItems;

};

#endif // MAINWINDOW_H
