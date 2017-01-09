#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTreeWidgetItem>

#include "glwidget.h"
#include "settings.h"

#include <vector>
#include <memory>

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

};

#endif // MAINWINDOW_H
