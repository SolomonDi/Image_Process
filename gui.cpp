#include<QApplication>
#include<QMainWindow>
#include<QPushButton>
#include<QButtonGroup>
#include<QLabel>
#include<QVBoxLayout>
#include<QHBoxLayout>
#include<QGroupBox>
#include<QFileDialog>
#include<QMessageBox>
#include<QImage>
#include<QSlider>
#include<QPixmap>
#include<QPainter>
#include<QSpinBox>
#include<QMouseEvent>
#include<functional>
#include<algorithm>

#include"im_read.hpp"
#include"gpu_process.hpp"
#include"process.hpp"
#include"image_stats.hpp"

static const char* kDarkStyle = R"(

    QMainWindow, QWidget {
        background-color: #1a1a1a;
        color: #d4d4d4;
        font-family: 'Segoe UI', sans-serif;
        font-size: 12px;
    }
    QGroupBox {
        background-color: #222222;
        border: 1px solid #2a2a2a;
        border-radius: 8px;
        margin-top: 14px;
        padding: 10px;
        font-weight: 600;
        color: #9a9a9a;
    }
    QGroupBox::title {
        subcontrol-origin: margin;
        left: 10px;
        padding: 0 6px;
        color: #4a90e2;
    }
    QPushButton {
        background-color: #2a2a2a;
        color: #d4d4d4;
        border: none;
        border-radius: 6px;
        padding: 8px 14px;
        font-weight: 600;
    }
    QPushButton:hover { background-color: #363636; }
    QPushButton:checked {
        background-color: #4a90e2;
        color: #ffffff;
    }
    QLabel#statsValue {
        color: #9fd88a;
        font-family: Consolas, monospace;
    }
)";


class ImageLabel : public QLabel {
public:
    explicit ImageLabel(QWidget *parent = nullptr) : QLabel(parent) {
        setAlignment(Qt::AlignCenter);
    }

    std::function<void(QRect)> onSelectionChanged;

    void setBaseImage(const QImage& image) {
        baseImage = image;
        selectionRect = QRect();
        updateDisplay();
    }

    QRect selectionInImageCoords() const {

        if (baseImage.isNull() || displayedRect.isEmpty() || selectionRect.isEmpty())
            return QRect();

        QRect sel = selectionRect.intersected(displayedRect);
        if (sel.isEmpty()) return QRect();

        double sx = (double)baseImage.width() / displayedRect.width();
        double sy = (double)baseImage.height() / displayedRect.height();

        int x0 = static_cast<int>((sel.left() - displayedRect.left()) * sx);
        int y0 = static_cast<int>((sel.top() - displayedRect.top()) * sy);

        int w = std::max(1, static_cast<int>(sel.width() * sx));
        int h = std::max(1, static_cast<int>(sel.height() * sy));

        return QRect(x0, y0, w, h);
    }

protected:
    void resizeEvent(QResizeEvent* event) override { QLabel::resizeEvent(event); updateDisplay(); }

    void mousePressEvent(QMouseEvent* event) override {
        if (baseImage.isNull()) return;
        dragging = true;
        dragStart = event->pos();
        selectionRect = QRect();
        updateDisplay();
    }

    void mouseMoveEvent(QMouseEvent* event) override {
        if (!dragging) return;
        selectionRect = QRect(dragStart, event->pos()).normalized();
        updateDisplay();
    }

    void mouseReleaseEvent(QMouseEvent* event) override {
        if (!dragging) return;
        dragging = false;
        selectionRect = QRect(dragStart, event->pos()).normalized();
        updateDisplay();
        if (onSelectionChanged) onSelectionChanged(selectionInImageCoords());
    }

private:
    void updateDisplay() {
        if (baseImage.isNull()) return;

        QPixmap scaled = QPixmap::fromImage(baseImage).scaled(size(), Qt::KeepAspectRatio,
            Qt::SmoothTransformation);

        int x = (width() - scaled.width()) / 2;
        int y = (height() - scaled.height()) / 2;
        displayedRect = QRect(x, y, scaled.width(), scaled.height());

        QPixmap canvas(size());
        canvas.fill(QColor("#111111"));
        QPainter painter(&canvas);
        painter.drawPixmap(displayedRect, scaled);

        if (!selectionRect.isEmpty()) {
            painter.setPen(QPen(QColor("#4a90e2"), 2));
            painter.setBrush(QColor(74, 144, 226, 50));
            painter.drawRect(selectionRect);
        }
        painter.end();

        setPixmap(canvas);
    }

    QImage baseImage;
    QRect displayedRect;
    QRect selectionRect;
    QPoint dragStart;
    bool dragging = false;
};


class MainWindow : public QMainWindow {
public:
    MainWindow() {
        setWindowTitle("Raw Image Process");
        resize(1150, 680);
        setStyleSheet(kDarkStyle);

        auto* central = new QWidget(this);
        auto* rootLayout = new QHBoxLayout(central);
        rootLayout->setContentsMargins(14, 14, 14, 14);
        rootLayout->setSpacing(14);

        preview = new ImageLabel();
        preview->setMinimumSize(600, 600);
        preview->onSelectionChanged = [this](QRect r) { onRegionSelected(r); };
        rootLayout->addWidget(preview, 3);

        auto* rightPanel = new QVBoxLayout();
        rightPanel->setSpacing(12);

        // file group
        auto* fileGroup = new QGroupBox("File");
        auto* fileLayout = new QVBoxLayout(fileGroup);
        auto* loadbtn = new QPushButton("Open RAW");
        fileLayout->addWidget(loadbtn);
        rightPanel->addWidget(fileGroup);

        // backend group
        auto* backendGroup = new QGroupBox("Device to use");
        auto* backLayout = new QHBoxLayout(backendGroup);
        backLayout->setSpacing(4);
        cpu_btn = new QPushButton("CPU");
        gpu_btn = new QPushButton("GPU");
        cpu_btn->setCheckable(true);
        gpu_btn->setCheckable(true);
        cpu_btn->setChecked(true);

        auto* backendBtnGroup = new QButtonGroup(this);
        backendBtnGroup->addButton(cpu_btn);
        backendBtnGroup->addButton(gpu_btn);
        backendBtnGroup->setExclusive(true);
        backLayout->addWidget(cpu_btn);
        backLayout->addWidget(gpu_btn);
        rightPanel->addWidget(backendGroup);

        // gamma group (slider)
        auto* gammaGroup = new QGroupBox("Gamma");
        auto* gammaLayout = new QVBoxLayout(gammaGroup);
        gammaValueLabel = new QLabel("2.20");          
        gammaSlider = new QSlider(Qt::Horizontal);
        gammaSlider->setRange(10, 300);
        gammaSlider->setValue(220);                      
        gammaLayout->addWidget(gammaValueLabel);
        gammaLayout->addWidget(gammaSlider);
        rightPanel->addWidget(gammaGroup);

        // process group
        auto* processGroup = new QGroupBox("Processing");
        auto* processLayout = new QVBoxLayout(processGroup);
        auto* pressedBtn = new QPushButton("Apply a demosaic to picture");
        processLayout->addWidget(pressedBtn);
        rightPanel->addWidget(processGroup);

        // export raw without processing
        auto* exportGroup = new QGroupBox("Export");
        auto* exportLayout = new QVBoxLayout(exportGroup);
        auto* saveBinBtn = new QPushButton("Save RAW to .Bin");
        auto* saveTiffBtn = new QPushButton("Save RAW to .TIFF");
        exportLayout->addWidget(saveBinBtn);
        exportLayout->addWidget(saveTiffBtn);
        rightPanel->addWidget(exportGroup);

        // save processing result
        auto* saveResGroup = new QGroupBox("Processing result");
        auto* saveResLayout = new QVBoxLayout(saveResGroup);
        auto* saveBtn = new QPushButton("Save to .png");
        saveResLayout->addWidget(saveBtn);
        rightPanel->addWidget(saveResGroup);

        // region group
        auto* regionGroup = new QGroupBox("Drag mouse to highlight region");
        auto* regionLayout = new QVBoxLayout(regionGroup);
        regionCoordsLabel = new QLabel("Region is not chosen");
        regionCoordsLabel->setObjectName("statsValue");
        regionCoordsLabel->setWordWrap(true);
        auto* processRegion = new QPushButton("Demosaic this region");
        regionLayout->addWidget(regionCoordsLabel);
        regionLayout->addWidget(processRegion);
        rightPanel->addWidget(regionGroup);             

        // stats for highlighted region
        auto* statsGroup = new QGroupBox("Stats of region");
        auto* statsLayout = new QVBoxLayout(statsGroup);
        statsText = new QLabel("—");
        statsText->setObjectName("statsValue");
        statsText->setWordWrap(true);
        statsLayout->addWidget(statsText);
        rightPanel->addWidget(statsGroup);

        rightPanel->addStretch();

        auto* rightWidget = new QWidget();
        rightWidget->setLayout(rightPanel);
        rightWidget->setFixedWidth(300);
        rootLayout->addWidget(rightWidget, 1);

        setCentralWidget(central);

        connect(loadbtn, &QPushButton::clicked, this, &MainWindow::onLoad);
        connect(pressedBtn, &QPushButton::clicked, this, &MainWindow::onProcess);
        connect(saveBinBtn, &QPushButton::clicked, this, &MainWindow::saveBin);
        connect(saveTiffBtn, &QPushButton::clicked, this, &MainWindow::saveTiff);
        connect(saveBtn, &QPushButton::clicked, this, &MainWindow::savePng);   // ИСПРАВЛЕНО: было onSavePng
        connect(processRegion, &QPushButton::clicked, this, &MainWindow::OnProcessRegion);
        connect(gammaSlider, &QSlider::valueChanged, this, &MainWindow::onGammaChanged);
    }

private:
    std::optional<RawImage> rawImage;
    std::optional<Image> linearResult;
    std::optional<Image> resultImage;
    QRect selectedRegion;

    ImageLabel* preview;
    QLabel* statsText;
    QLabel* regionCoordsLabel;
    QLabel* gammaValueLabel;
    QSlider* gammaSlider;
    QPushButton* cpu_btn;
    QPushButton* gpu_btn;

    Backend currentBack() const { return gpu_btn->isChecked() ? Backend::GPU : Backend::CPU; }

    void show_image(const Image& im) {
        QImage qi(im.data.data(), im.width, im.height,
                  im.width * im.channels, QImage::Format_RGB888);
        preview->setBaseImage(qi.copy());
    }

    void onLoad() {
        QString path = QFileDialog::getOpenFileName(this, "Choose RAW-file");
        if (path.isEmpty()) return;

        rawImage = load_raw(path.toStdString());
        if (!rawImage) {
            QMessageBox::warning(this, "Error", "Couldn't load file");
            return;
        }

        linearResult.reset();
        resultImage.reset();
        selectedRegion = QRect();

        regionCoordsLabel->setText("Region is not chosen");
        statsText->setText("—");

        std::vector<uint8_t> preview8(rawImage->data.size());
        float range = static_cast<float>(rawImage->white_level - rawImage->black_level);
        if (range <= 0.f) range = 1.f;

        for (size_t i = 0; i < rawImage->data.size(); ++i) {
            
            float v = (static_cast<float>(rawImage->data[i]) - rawImage->black_level) / range;
            v = std::min(1.f, std::max(0.f, v));
            preview8[i] = static_cast<uint8_t>(v * 255.f);
        }

        QImage qiPrew(preview8.data(), rawImage->width, rawImage->height,
                      rawImage->width, QImage::Format_Grayscale8);

        preview->setBaseImage(qiPrew.copy());
    }

    void onProcess() {
        if (!rawImage) {
            QMessageBox::warning(this, "Error", "Download RAW-file first");
            return;
        }

        try {
            linearResult = process_demosaic(*rawImage, currentBack());
        } catch (const std::exception& ex) {
            QMessageBox::critical(this, "Error", ex.what());
            return;
        }

        onGammaChanged(gammaSlider->value());
    }   

    void OnProcessRegion() {
        if (!rawImage) {
            QMessageBox::warning(this, "Error", "Download RAW-file first");
            return;
        }

        if (selectedRegion.isEmpty()) {
            QMessageBox::warning(this, "Error", "Highlight region by mouse first");
            return;
        }

        RawImage cropped_im = region_crop(*rawImage, selectedRegion.x(), selectedRegion.y(),
                                           selectedRegion.width(), selectedRegion.height());

        try {
            linearResult = process_demosaic(cropped_im, currentBack());
        } catch (const std::exception& ex) {
            QMessageBox::critical(this, "Error", ex.what());
            return;
        }

        onGammaChanged(gammaSlider->value());
    }

    void onGammaChanged(int sliderValue) {
        float gamma = sliderValue / 100.f;
        gammaValueLabel->setText(QString::number(gamma, 'f', 2));

        if (!linearResult) return;

        resultImage = apply_gamma(*linearResult, gamma, currentBack());
        show_image(*resultImage);
    }

    void onRegionSelected(QRect regionInRawCoords) {
        if (!rawImage || regionInRawCoords.isEmpty()) {
            regionCoordsLabel->setText("Region has not been chosen");
            return;
        }

        int x0 = std::clamp(regionInRawCoords.x(), 0, rawImage->width - 1);
        int y0 = std::clamp(regionInRawCoords.y(), 0, rawImage->height - 1);

        int w = std::min(regionInRawCoords.width(), rawImage->width - x0);
        int h = std::min(regionInRawCoords.height(), rawImage->height - y0);
        if (w <= 0 || h <= 0) return;

        selectedRegion = QRect(x0, y0, w, h);
        regionCoordsLabel->setText(QString("[%1, %2]  %3x%4 px").arg(x0).arg(y0).arg(w).arg(h));

        ImageStats stats = compute_region(*rawImage, x0, y0, w, h);
        statsText->setText(QString(
            "Min: %1  Max: %2\n"
            "Mean: %3\n"
            "RMS: %4\n"
            "ENOB: %5 bits")
            .arg(stats.min)
            .arg(stats.max)
            .arg(stats.mean, 0, 'f', 2)
            .arg(stats.stdDev, 0, 'f', 2)
            .arg(stats.effectBits, 0, 'f', 2));
    }

    void saveBin() {
        if (!rawImage) {
            QMessageBox::warning(this, "Error", "Upload RAW first");
            return;
        }

        QString path = QFileDialog::getSaveFileName(this, "Save bin", "raw_data.bin", "Binary (*.bin)");
        if (path.isEmpty()) return;

        if (!save_raw_binary(path.toStdString(), *rawImage)) {
            QMessageBox::critical(this, "Error", "Couldn't save binary");
        } else {
            QMessageBox::information(this, "Ready",
                QString("Saved. width=%1, height=%2")
                    .arg(rawImage->width).arg(rawImage->height));
        }
    }

    // ИСПРАВЛЕНО: return; стоял до основного кода — функция ничего не делала;
    // плюс не был закрыт if
    void saveTiff() {
        if (!rawImage) {
            QMessageBox::warning(this, "Error", "Upload RAW first");
            return;
        }

        QString path = QFileDialog::getSaveFileName(this, "Save TIFF", "raw_mosaic.tiff", "TIFF (*.tiff)");
        if (path.isEmpty()) return;

        if (!save_raw_in_tiff(path.toStdString(), *rawImage))
            QMessageBox::critical(this, "Error", "Couldn't save TIFF");
    }

    // ИСПРАВЛЕНО: не было (), getSavedFileName -> getSaveFileName, his -> this
    void savePng() {
        if (!resultImage) {
            QMessageBox::warning(this, "Error", "Apply demosaic first");
            return;
        }

        QString path = QFileDialog::getSaveFileName(this, "Save PNG", "output.png", "PNG (*.png)");
        if (path.isEmpty()) return;

        if (!save_image(path.toStdString(), *resultImage))
            QMessageBox::critical(this, "Error", "Couldn't save PNG");
    }
};   // ИСПРАВЛЕНО: была лишняя } перед этой строкой


int main(int argc, char** argv) {
    QApplication app(argc, argv);
    MainWindow window;
    window.show();
    return app.exec();
}