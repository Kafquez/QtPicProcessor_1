// mainwindow.cpp

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFileDialog>
#include <QDir>
#include <QMessageBox>
#include <QImage>
#include <QRandomGenerator>
#include <QTransform>
#include <QInputDialog>
#include <QDebug>
#include <QLineEdit>
#include <QListWidget>
#include <QDialogButtonBox>
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    // 开启 QLabel 的鼠标跟踪，以便后续的拖拽操作
    // 设置窗口标题
    setWindowTitle("图片预处理程序");
    // 初始化 saveDir
    saveDir = "";
    ui->sampLabel->setMouseTracking(true);
//    ui->sampLabel->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);

    ui->sampLabel->setAlignment(Qt::AlignCenter); // 设置图像居中显示
    ui->sampLabel->setScaledContents(false); // 图像自适应
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::showImageFeatures(bool formatChanged)
{
    if (formatChanged) // 格式转换后需要显示全部信息
    {
        QImage::Format fmt = m_image.format(); // 图像格式
        if (fmt == QImage::Format_RGB32)
            ui->editImg_Format_2->setText("32-bit RGB(0xffRRGGBB)");
        else if (fmt == QImage::Format_RGB16)
            ui->editImg_Format_2->setText("16-bit RGB565");
        else if (fmt == QImage::Format_RGB888)
            ui->editImg_Format_2->setText("24-bit RGB888");
        else if (fmt == QImage::Format_Grayscale8)
            ui->editImg_Format_2->setText("8-bit grayscale");
        else if (fmt == QImage::Format_Grayscale16)
            ui->editImg_Format_2->setText("16-bit grayscale");
        else if (fmt == QImage::Format_ARGB32)
            ui->editImg_Format_2->setText("32-bit ARGB(0xAARRGGBB)");
        else if (fmt == QImage::Format_Indexed8)
            ui->editImg_Format_2->setText("8-bit indexes into a colormap");
        else
            ui->editImg_Format_2->setText(QString("Format= %1,其他格式").arg(fmt));

        ui->editImg_Depth_2->setText(QString("%1 bits/pixel").arg(m_image.depth()));
        ui->editImg_BitPlace->setText(QString("%1 bits").arg(m_image.bitPlaneCount()));
    }

    // 缩放，或旋转之后显示大小信息
    ui->editImg_Height_2->setText(QString("%1 像素").arg(m_image.height()));
    ui->editImg_Width_2->setText(QString("%1 像素").arg(m_image.width()));

    qsizetype sz = m_image.sizeInBytes(); // 图像数据字节数
    if (sz < 1024 * 9)
        ui->editImg_SizeByte_2->setText(QString("%1 Bytes").arg(sz));
    else
        ui->editImg_SizeByte_2->setText(QString("%1 KB").arg(sz / 1024));

    // 设置图像属性到新的标签和线编辑控件
    ui->labFormat->setText("图像格式：");
    ui->editImg_Format_2->setText(ui->editImg_Format_2->text());

    ui->labWidth->setText("图像宽度：");
    ui->editImg_Width_2->setText(ui->editImg_Width_2->text());

    ui->labHeight->setText("图像高度：");
    ui->editImg_Height_2->setText(ui->editImg_Height_2->text());

    ui->labDepth->setText("图像深度：");
    ui->editImg_Depth_2->setText(ui->editImg_Depth_2->text());

    ui->labBP->setText("位平面数：");
    ui->editImg_BitPlace->setText(ui->editImg_BitPlace->text());

    ui->labSize->setText("图像大小：");
    ui->editImg_SizeByte_2->setText(ui->editImg_SizeByte_2->text());
}

void MainWindow::on_actOpen_triggered()
{
    QString folderPath = QFileDialog::getExistingDirectory(this, tr("选择文件夹"));

    if (!folderPath.isEmpty()) {
        QDir folderDir(folderPath);
        QStringList filters;
        filters << "*.jpg" << "*.jpeg" << "*.png" << "*.bmp"; // 支持的图片格式

        QStringList fileList = folderDir.entryList(filters, QDir::Files);

        if (fileList.isEmpty()) {
            QMessageBox::information(this, tr("提示"), tr("文件夹中没有支持的图片文件。"));
        } else {
            // 清空已打开的图片列表
            openedImages.clear();

            // 加载所有图片并保存到 openedImages
            for (const QString &filePath : fileList) {
                QImage image(folderDir.filePath(filePath));
                if (!image.isNull()) {
                    openedImages.append(image);
                }
            }

            // 显示第一张图片在 sampLabel 上，保持原长宽比
            if (!openedImages.isEmpty()) {
                m_image = openedImages.first(); // 保存当前图片
                QPixmap pixmap = QPixmap::fromImage(m_image);
                ui->sampLabel->setPixmap(pixmap.scaled(ui->sampLabel->size(), Qt::KeepAspectRatio));
                showImageFeatures(true); // 更新示例图片的属性
            }
        }
    }
}


void MainWindow::saveAndAppendSuffix(const QImage &image, const QString &suffix)
{
    // 构建带有后缀的文件名
    QString fileName = QString("image_%1%2.jpg").arg(savedImagesCount + 1).arg(suffix);

    // 保存图像
    if (image.save(fileName)) {
        waitingToSaveImages.append(image);
        fileSuffixes.append(suffix); // 添加后缀到列表中
        savedImagesCount++;
    } else {
        QMessageBox::information(this, tr("提示"), tr("保存图片失败：%1").arg(fileName));
    }
}







// 鼠标按下事件处理函数，用于实现图片拖拽
void MainWindow::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        // 记录鼠标按下的位置
        lastDragPos = event->pos();
        setCursor(Qt::ClosedHandCursor); // 更改鼠标样式为“抓手”
    }
}

// 鼠标移动事件处理函数，用于实现图片拖拽
void MainWindow::mouseMoveEvent(QMouseEvent *event)
{
    if (event->buttons() & Qt::LeftButton) {
        // 计算鼠标移动的位移
        QPoint delta = event->pos() - lastDragPos;
        // 更新图像的位置
        ui->sampLabel->move(ui->sampLabel->pos() + delta);
        // 更新上一次的位置
        lastDragPos = event->pos();
    }
}

// 鼠标释放事件处理函数，用于实现图片拖拽
void MainWindow::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        setCursor(Qt::ArrowCursor); // 恢复鼠标样式为箭头
    }
}

void MainWindow::on_actSave_triggered()
{
    if (waitingToSaveImages.isEmpty()) {
        QMessageBox::information(this, tr("提示"), tr("没有需要保存的图片。"));
        return;
    }

    // 弹出文件夹选择对话框，让用户选择保存目录
    saveDir = QFileDialog::getExistingDirectory(this, tr("选择保存目录"));

    if (saveDir.isEmpty()) {
        QMessageBox::information(this, tr("提示"), tr("未选择保存目录。"));
        return;
    }

    for (int i = 0; i < waitingToSaveImages.size(); ++i) {
        QString fileName = QString("image_%1%2.%3").arg(i + 1).arg(fileSuffixes[i]).arg("jpg");
        QString filePath = QDir(saveDir).filePath(fileName);

        if (!waitingToSaveImages[i].save(filePath)) {
            QMessageBox::information(this, tr("提示"), tr("保存图片失败：%1").arg(filePath));
        }
    }

    QMessageBox::information(this, tr("提示"), tr("已保存 %1 张图片到 %2 目录。").arg(waitingToSaveImages.size()).arg(saveDir));
}

void MainWindow::on_actRanCrop_triggered()
{
    // 清空等待保存的图片列表
    waitingToSaveImages.clear();
    fileSuffixes.clear(); // 清空后缀列表
    savedImagesCount = 0;

    // 遍历所有已打开的图片并进行随机裁剪
    for (int i = 0; i < openedImages.size(); ++i) {
        QImage image = openedImages[i];
        int width = image.width();
        int height = image.height();

        // 随机裁剪的左上角坐标
        int x = QRandomGenerator::global()->bounded(width);
        int y = QRandomGenerator::global()->bounded(height);

        // 随机裁剪的宽度和高度
        int cropWidth = QRandomGenerator::global()->bounded(width - x);
        int cropHeight = QRandomGenerator::global()->bounded(height - y);

        QImage croppedImage = image.copy(x, y, cropWidth, cropHeight);
        if (!croppedImage.isNull()) {
            saveAndAppendSuffix(croppedImage, "_rancrop"); // 保存并添加后缀
        }
    }

    // 将处理后的图片设置为示例图片
    if (!waitingToSaveImages.isEmpty()) {
        m_image = waitingToSaveImages.first();
        QPixmap pixmap = QPixmap::fromImage(m_image);
        ui->sampLabel->setPixmap(pixmap.scaled(ui->sampLabel->size(), Qt::KeepAspectRatio));

        // 刷新图片属性
        showImageFeatures(true);
    }

    QMessageBox::information(this, tr("提示"), tr("已随机裁剪 %1 张图片。").arg(waitingToSaveImages.size()));
}

void MainWindow::on_actFlip_triggered()
{
    // 清空等待保存的图片列表
    waitingToSaveImages.clear();
    fileSuffixes.clear(); // 清空后缀列表

    // 遍历所有已打开的图片并进行翻转
    for (int i = 0; i < openedImages.size(); ++i) {
        QImage image = openedImages[i];
        QImage flippedImage = image.mirrored(true, false); // 镜像翻转（水平翻转）

        // 调用保存和添加后缀的函数
        saveAndAppendSuffix(flippedImage, "_flip");
    }

    // 将处理后的图片设置为示例图片
    if (!waitingToSaveImages.isEmpty()) {
        m_image = waitingToSaveImages.first();
        QPixmap pixmap = QPixmap::fromImage(m_image);
        ui->sampLabel->setPixmap(pixmap.scaled(ui->sampLabel->size(), Qt::KeepAspectRatio));

        // 刷新图片属性
        showImageFeatures(true);
    }

    QMessageBox::information(this, tr("提示"), tr("已翻转 %1 张图片。").arg(waitingToSaveImages.size()));
}



void MainWindow::on_actRotate_triggered()
{
    // 提示用户输入旋转角度
    bool ok;
    double angle = QInputDialog::getDouble(this, tr("输入旋转角度"), tr("请输入旋转角度（顺时针）："), rotationAngle, -360, 360, 1, &ok);

    if (ok) {
        rotationAngle = angle;

        // 清空等待保存的图片列表
        waitingToSaveImages.clear();
        fileSuffixes.clear(); // 清空后缀列表

        // 遍历所有已打开的图片并进行旋转
        for (int i = 0; i < openedImages.size(); ++i) {
            QImage image = openedImages[i];
            QTransform transform;
            transform.rotate(rotationAngle);
            QImage rotatedImage = image.transformed(transform, Qt::FastTransformation);

            // 调用保存和添加后缀的函数
            saveAndAppendSuffix(rotatedImage, "_rotate");
        }

        // 将处理后的图片设置为示例图片
        if (!waitingToSaveImages.isEmpty()) {
            m_image = waitingToSaveImages.first();
            QPixmap pixmap = QPixmap::fromImage(m_image);
            ui->sampLabel->setPixmap(pixmap.scaled(ui->sampLabel->size(), Qt::KeepAspectRatio));

            // 刷新图片属性
            showImageFeatures(true);
        }

        QMessageBox::information(this, tr("提示"), tr("已旋转 %1 张图片。").arg(waitingToSaveImages.size()));
    }
}




void MainWindow::on_actEnhance_triggered()
{
    // 获取用户输入的亮度值
    bool ok;
    double brightnessFactor = QInputDialog::getDouble(this, tr("输入亮度值"), tr("请输入亮度值："), 1.0, 0.0, 100.0, 2, &ok);

    if (!ok) {
        // 用户取消了输入
        return;
    }

    // 清空等待保存的图片列表
    waitingToSaveImages.clear();
    fileSuffixes.clear(); // 清空后缀列表

    // 遍历所有已打开的图片并进行亮度增强处理
    for (int i = 0; i < openedImages.size(); ++i) {
        QImage image = openedImages[i];
        QImage enhancedImage = image;

        // 遍历图片的像素，对每个像素的亮度进行调整
        for (int y = 0; y < enhancedImage.height(); ++y) {
            for (int x = 0; x < enhancedImage.width(); ++x) {
                QColor pixelColor = enhancedImage.pixelColor(x, y);

                int newRed = qBound(0, int(pixelColor.redF() * brightnessFactor * 255.0), 255);
                int newGreen = qBound(0, int(pixelColor.greenF() * brightnessFactor * 255.0), 255);
                int newBlue = qBound(0, int(pixelColor.blueF() * brightnessFactor * 255.0), 255);

                pixelColor.setRed(newRed);
                pixelColor.setGreen(newGreen);
                pixelColor.setBlue(newBlue);

                enhancedImage.setPixelColor(x, y, pixelColor);
            }
        }

        // 调用保存和添加后缀的函数
        saveAndAppendSuffix(enhancedImage, "_enhance");
    }

    // 将处理后的图片设置为示例图片
    if (!waitingToSaveImages.isEmpty()) {
        m_image = waitingToSaveImages.first();
        QPixmap pixmap = QPixmap::fromImage(m_image);
        ui->sampLabel->setPixmap(pixmap.scaled(ui->sampLabel->size(), Qt::KeepAspectRatio));

        // 刷新图片属性
        showImageFeatures(true);
    }

    QMessageBox::information(this, tr("提示"), tr("已进行亮度增强处理，亮度值：%1").arg(brightnessFactor));
}




void MainWindow::on_actResize_triggered()
{
    // 弹出对话框，让用户选择缩放方式
    QStringList choices;
    choices << tr("等比例缩放") << tr("指定尺寸");
    bool ok;
    QString choice = QInputDialog::getItem(this, tr("选择缩放方式"), tr("请选择缩放方式："), choices, 0, false, &ok);

    if (!ok) {
        // 用户取消了选择
        return;
    }

    if (choice == tr("等比例缩放")) {
        // 用户选择等比例缩放
        double scaleFactor = QInputDialog::getDouble(this, tr("输入缩放比例"), tr("请输入缩放比例："), 1.0, 0.01, 100.0, 2, &ok);

        if (!ok) {
            // 用户取消了输入
            return;
        }

        // 清空等待保存的图片列表
        waitingToSaveImages.clear();
        fileSuffixes.clear(); // 清空后缀列表

        // 遍历所有已打开的图片并进行等比例缩放处理
        for (int i = 0; i < openedImages.size(); ++i) {
            QImage image = openedImages[i];
            QImage scaledImage = image.scaled(image.width() * scaleFactor, image.height() * scaleFactor, Qt::KeepAspectRatio);

            // 调用保存和添加后缀的函数
            saveAndAppendSuffix(scaledImage, "_scale");
        }

        // 将处理后的图片设置为示例图片
        if (!waitingToSaveImages.isEmpty()) {
            m_image = waitingToSaveImages.first();
            QPixmap pixmap = QPixmap::fromImage(m_image);
            ui->sampLabel->setPixmap(pixmap.scaled(ui->sampLabel->size(), Qt::KeepAspectRatio));

            // 刷新图片属性
            showImageFeatures(true);
        }

        QMessageBox::information(this, tr("提示"), tr("已进行等比例缩放，缩放比例：%1").arg(scaleFactor));
    } else if (choice == tr("指定尺寸")) {
        // 用户选择指定尺寸
        int newWidth = QInputDialog::getInt(this, tr("输入宽度"), tr("请输入新的宽度："), m_image.width(), 1, 5000, 1, &ok);
        if (!ok) {
            // 用户取消了输入
            return;
        }

        int newHeight = QInputDialog::getInt(this, tr("输入高度"), tr("请输入新的高度："), m_image.height(), 1, 5000, 1, &ok);
        if (!ok) {
            // 用户取消了输入
            return;
        }

        // 清空等待保存的图片列表
        waitingToSaveImages.clear();
        fileSuffixes.clear(); // 清空后缀列表

        // 遍历所有已打开的图片并进行指定尺寸缩放处理
        for (int i = 0; i < openedImages.size(); ++i) {
            QImage image = openedImages[i];
            QImage scaledImage = image.scaled(newWidth, newHeight, Qt::KeepAspectRatio);

            // 调用保存和添加后缀的函数
            saveAndAppendSuffix(scaledImage, "_resize");
        }

        // 将处理后的图片设置为示例图片
        if (!waitingToSaveImages.isEmpty()) {
            m_image = waitingToSaveImages.first();
            QPixmap pixmap = QPixmap::fromImage(m_image);
            ui->sampLabel->setPixmap(pixmap.scaled(ui->sampLabel->size(), Qt::KeepAspectRatio));

            // 刷新图片属性
            showImageFeatures(true);
        }

        QMessageBox::information(this, tr("提示"), tr("已进行指定尺寸缩放，新宽度：%1，新高度：%2").arg(newWidth).arg(newHeight));
    }
}



QImage MainWindow::applyColorShake(const QImage &image, int strength)
{
    QImage shakenImage = image;

    for (int y = 0; y < shakenImage.height(); ++y) {
        for (int x = 0; x < shakenImage.width(); ++x) {
            QColor pixelColor = shakenImage.pixelColor(x, y);

            // Generate random values for color channel shake
            int redShake = rand() % (2 * strength) - strength;
            int greenShake = rand() % (2 * strength) - strength;
            int blueShake = rand() % (2 * strength) - strength;

            int newRed = qBound(0, pixelColor.red() + redShake, 255);
            int newGreen = qBound(0, pixelColor.green() + greenShake, 255);
            int newBlue = qBound(0, pixelColor.blue() + blueShake, 255);

            pixelColor.setRed(newRed);
            pixelColor.setGreen(newGreen);
            pixelColor.setBlue(newBlue);

            shakenImage.setPixelColor(x, y, pixelColor);
        }
    }

    return shakenImage;
}

void MainWindow::on_actShake_triggered()
{
    // 获取用户输入的抖动强度
    bool ok;
    int shakeStrength = QInputDialog::getInt(this, tr("输入抖动强度"), tr("请输入抖动强度："), 25, 1, 100, 1, &ok);

    if (!ok) {
        // 用户取消了输入
        return;
    }

    // 清空等待保存的图片列表
    waitingToSaveImages.clear();
    fileSuffixes.clear(); // 清空后缀列表

    // Seed the random number generator
    srand(time(nullptr));

    // 遍历所有已打开的图片并进行颜色抖动处理
    for (int i = 0; i < openedImages.size(); ++i) {
        QImage image = openedImages[i];
        QImage shakenImage = applyColorShake(image, shakeStrength);

        // 调用保存和添加后缀的函数
        saveAndAppendSuffix(shakenImage, "_shake");
    }

    // 将处理后的图片设置为示例图片
    if (!waitingToSaveImages.isEmpty()) {
        m_image = waitingToSaveImages.first();
        QPixmap pixmap = QPixmap::fromImage(m_image);
        ui->sampLabel->setPixmap(pixmap.scaled(ui->sampLabel->size(), Qt::KeepAspectRatio));

        // 刷新图片属性
        showImageFeatures(true);
    }

    QMessageBox::information(this, tr("提示"), tr("已进行颜色抖动处理，抖动强度：%1").arg(shakeStrength));
}

