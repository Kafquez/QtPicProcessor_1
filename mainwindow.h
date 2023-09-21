#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QVector>
#include <QMouseEvent>
#include <QLabel>
#include <QLineEdit>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_actOpen_triggered();
    void on_actRanCrop_triggered();
    void on_actSave_triggered();
    void on_actFlip_triggered();
    void on_actRotate_triggered();
    void on_actEnhance_triggered();
    void on_actResize_triggered();
    void on_actShake_triggered();

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

private:
    QLabel *labFormat;
    QLabel *labWidth;
    QLabel *labHeight;
    QLabel *labDepth;
    QLabel *labBP;
    QLabel *labSize;
    QLineEdit *editImg_Format_2;
    QLineEdit *editImg_Width_2;
    QLineEdit *editImg_Height_2;
    QLineEdit *editImg_Depth_2;
    QLineEdit *editImg_BitPlace_2;
    QLineEdit *editImg_SizeByte_2;
    QString saveDir; // 声明 saveDir 成员变量
    QStringList fileSuffixes; // 用于存储文件名后缀
    int savedImagesCount = 0;
    // 声明 showImageFeatures 函数
    void showImageFeatures(bool formatChanged = false);

    // 添加成员变量 m_filename
    QString m_filename;

    // 声明 saveAndAppendSuffix 函数
    void saveAndAppendSuffix(const QImage &image, const QString &suffix);

private:
    Ui::MainWindow *ui;
    QVector<QImage> openedImages;
    QVector<QImage> waitingToSaveImages;
    QPoint lastDragPos;
    double rotationAngle = 0.0;
    QImage m_image; // 用于存储当前图像
    QString getImageFormat(const QImage& image);
    QImage applyColorShake(const QImage &image, int strength);
};

#endif // MAINWINDOW_H
