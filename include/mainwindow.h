#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTreeWidgetItem>

#include "glwidget.h"

#include <vector>
#include <memory>

struct ShaderSettings
{
    float red = 0.0f;
    float green = 0.0f;
    float blue = 0.0f;

    bool wireframe = false;
    bool skinMesh = false;
    bool drawMesh = true;
};

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
    void passMeshColour(QColor colour, QString meshName);
    void passWorldColour(QColor colour);
    void passPlaneColour(QColor colour);

    void passLoadMesh(QString fileName, QString meshName, QVector3D position);

    void passDrawMesh(bool isDrawing, QString meshName);
    void passWireframMesh(bool isWireframe, QString meshName);
    void passSimulating(bool isSimulating);

private slots:
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

private:
    Ui::MainWindow *ui;

    GLWidget* m_glWidget;

    std::vector< std::pair< QString, ShaderSettings*>> m_settings;

};

#endif // MAINWINDOW_H
