#include<QApplication>
#include<QMainWindow>
#include<QPushButton>
#include<QButtonGroup>
#include<QLabel>
#include<QVBoxLayout>
#include<QHBoxLayout>
#include<QGroupBox>
#include<QFileDialog>
#include<QFileInfo>
#include<QMessageBox>
#include<QImage>
#include<QSlider>
#include<QPixmap>
#include<QPainter>
#include<QPolygonF>
#include<QPainterPath>
#include<QFont>
#include<QIcon>
#include<QMimeData>
#include<QUrl>
#include<QDragEnterEvent>
#include<QDragLeaveEvent>
#include<QDropEvent>
#include<QKeyEvent>
#include<QTransform>
#include<QTableWidget>
#include<QTableWidgetItem>
#include<QHeaderView>
#include<QAbstractItemView>
#include<QPlainTextEdit>
#include<QJsonDocument>
#include<QJsonObject>
#include<QJsonArray>
#include<QTime>
#include<QVector>
#include<QGraphicsDropShadowEffect>
#include<QDockWidget>
#include<QMenuBar>
#include<QMenu>
#include<QToolBar>
#include<QToolButton>
#include<QDialog>
#include<QAction>
#include<QClipboard>
#include<QElapsedTimer>
#include<QUndoStack>
#include<QUndoCommand>
#include<QTabBar>
#include<QLineEdit>
#include<QAbstractSpinBox>
#include<QSet>
#include<QHash>
#include<QStackedWidget>
#include<QFrame>
#include<atomic>
#include<QRadialGradient>
#include<QLinearGradient>
#include<QScreen>
#include<QFontMetrics>

#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include<windows.h>
#include<windowsx.h>
#include<psapi.h>
#pragma comment(lib, "psapi.lib")
#endif

static const char* kAppVersion = "1.0";
#include<QSpinBox>
#include<QScrollArea>
#include<QStatusBar>
#include<QProgressBar>
#include<QTabWidget>
#include<QShortcut>
#include<QSettings>
#include<QFile>
#include<QTextStream>
#include<QCloseEvent>
#include<QWheelEvent>
#include<QStyleFactory>
#include<QTimer>
#include<QMouseEvent>
#include<functional>
#include<algorithm>
#include<cmath>
#include<thread>
#include<memory>
#include<array>

#include"im_read.hpp"
#include"gpu_process.hpp"
#include"process.hpp"
#include"image_stats.hpp"
#include"denoise.hpp"

static constexpr double kMinZoom = 1.0;
static constexpr double kMaxZoom = 30.0;
static constexpr int kHistogramBins = 256;

static const char* kDarkStyle = R"(

    QWidget {
        color: #d8e6f5;
        font-family: 'Segoe UI', sans-serif;
        font-size: 12px;
    }
    QMainWindow { background-color: #04050a; }
    QWidget#background { background: transparent; }
    QWidget#titleBar { background: transparent; }
    QLabel#titleText {
        color: #d8e6f5;
        font-size: 12px;
        font-weight: 700;
        background: transparent;
    }
    QLabel#titleFile {
        color: #5d6f8c;
        font-family: Consolas, monospace;
        font-size: 11px;
        background: transparent;
    }
    QPushButton#winBtn, QPushButton#winClose {
        background: transparent;
        border: none;
        border-radius: 4px;
        color: #5d6f8c;
        font-size: 13px;
        font-weight: 400;
        padding: 0px;
        min-width: 36px;
        max-width: 36px;
        min-height: 26px;
        max-height: 26px;
        text-align: center;
    }
    QPushButton#winBtn:hover {
        background: rgba(0, 229, 255, 30);
        color: #d8e6f5;
    }
    QPushButton#winClose:hover {
        background: #e23a5e;
        color: #ffffff;
    }
    QGroupBox {
        background-color: #0c0f18;
        border: 1px solid #16203a;
        border-radius: 8px;
        margin-top: 13px;
        padding: 0px;
        font-size: 10px;
        font-weight: 700;
        color: #5d6f8c;
    }
    QGroupBox::title {
        subcontrol-origin: margin;
        subcontrol-position: top left;
        left: 10px;
        padding: 0 6px;
        color: #00e5ff;
    }
    QLabel {
        background: transparent;
        color: #d8e6f5;
        font-size: 12px;
        font-weight: 400;
    }
    QLabel#statsValue {
        color: #7cf7c4;
        font-family: Consolas, monospace;
        font-size: 11px;
    }
    QLabel#hint {
        color: #4d5c75;
        font-size: 11px;
    }
    QPushButton {
        background-color: rgba(255, 255, 255, 6);
        color: #d8e6f5;
        border: 1px solid #16203a;
        border-radius: 4px;
        padding: 0 10px;
        min-height: 28px;
        max-height: 28px;
        font-size: 12px;
        font-weight: 600;
        text-align: left;
    }
    QPushButton:hover {
        background-color: rgba(0, 229, 255, 20);
        border-color: #26456e;
    }
    QPushButton:pressed { background-color: rgba(0, 229, 255, 34); }
    QPushButton:checked {
        background-color: rgba(0, 229, 255, 40);
        border-color: #00e5ff;
        color: #eaffff;
    }
    QPushButton:disabled {
        background-color: transparent;
        border-color: #101728;
        color: #38455c;
    }
    QPushButton#pairButton {
        text-align: center;
        padding: 0 6px;
    }
    QPushButton#pageButton {
        background: transparent;
        border: none;
        border-bottom: 2px solid transparent;
        border-radius: 0px;
        color: #5d6f8c;
        padding: 0 14px;
        min-height: 30px;
        max-height: 30px;
        font-size: 11px;
        font-weight: 700;
        text-align: center;
    }
    QPushButton#pageButton:hover {
        background: transparent;
        color: #d8e6f5;
    }
    QPushButton#pageButton:checked {
        background: transparent;
        color: #00e5ff;
        border-bottom: 2px solid #00e5ff;
    }
    QFrame#card {
        background-color: #0c0f18;
        border: 1px solid #16203a;
        border-radius: 8px;
    }
    QLabel#cardTitle {
        color: #5d6f8c;
        font-size: 10px;
        font-weight: 700;
    }
    QLabel#cardValue, QLabel#cardValueBefore {
        color: #00e5ff;
        font-family: Consolas, monospace;
        font-size: 22px;
        font-weight: 600;
    }
    QLabel#cardValueBefore { color: #ff2fb9; }
    QSpinBox {
        background-color: #06070d;
        color: #d8e6f5;
        border: 1px solid #16203a;
        border-radius: 4px;
        padding: 0 6px;
        min-height: 28px;
        max-height: 28px;
        font-size: 12px;
        selection-background-color: #00e5ff;
        selection-color: #04121a;
    }
    QSpinBox:focus { border-color: #00e5ff; }
    QSlider {
        min-height: 28px;
        max-height: 28px;
    }
    QSlider::groove:horizontal {
        height: 4px;
        background: #0d1424;
        border: 1px solid #16203a;
        border-radius: 2px;
    }
    QSlider::sub-page:horizontal {
        background: #00e5ff;
        border: 1px solid #00e5ff;
        border-radius: 2px;
    }
    QSlider::handle:horizontal {
        background: qradialgradient(cx:0.5, cy:0.5, radius:0.5,
                                    stop:0 #ffffff, stop:0.55 #b6f4ff, stop:1 #00e5ff);
        width: 13px;
        margin: -6px 0;
        border-radius: 6px;
    }
    QScrollArea {
        border: none;
        background: transparent;
    }
    QScrollArea > QWidget > QWidget { background: transparent; }
    QScrollBar:vertical {
        background: transparent;
        width: 10px;
        margin: 0px;
    }
    QScrollBar::handle:vertical {
        background: #1b2b47;
        border-radius: 5px;
        min-height: 30px;
    }
    QScrollBar::handle:vertical:hover { background: #26456e; }
    QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height: 0px; }
    QScrollBar::add-page:vertical, QScrollBar::sub-page:vertical { background: none; }
    QTabWidget::pane {
        border: 1px solid #16203a;
        border-radius: 4px;
        background-color: #06070d;
        top: -1px;
    }
    QTabBar::tab {
        background: transparent;
        color: #5d6f8c;
        border: 1px solid transparent;
        border-bottom: none;
        border-top-left-radius: 4px;
        border-top-right-radius: 4px;
        padding: 4px 12px;
        margin-right: 2px;
        font-size: 11px;
        font-weight: 600;
    }
    QTabBar::tab:hover { color: #d8e6f5; }
    QTabBar::tab:selected {
        background: #06070d;
        border-color: #16203a;
        color: #00e5ff;
    }
    QProgressBar {
        background-color: #0d1424;
        border: none;
        border-radius: 2px;
        min-height: 4px;
        max-height: 4px;
    }
    QProgressBar::chunk {
        background-color: #00e5ff;
        border-radius: 2px;
    }
    QTableWidget {
        background-color: #06070d;
        alternate-background-color: #090c15;
        color: #d8e6f5;
        border: 1px solid #16203a;
        border-radius: 4px;
        gridline-color: #101728;
        font-family: Consolas, monospace;
        font-size: 11px;
    }
    QTableWidget::item:selected {
        background-color: rgba(0, 229, 255, 46);
        color: #eaffff;
    }
    QHeaderView::section {
        background-color: #0b0e18;
        color: #5d6f8c;
        border: none;
        border-right: 1px solid #16203a;
        border-bottom: 1px solid #16203a;
        padding: 3px 4px;
        font-size: 10px;
        font-weight: 700;
    }
    QPlainTextEdit {
        background-color: #06070d;
        color: #5d6f8c;
        border: 1px solid #16203a;
        border-radius: 4px;
        font-family: Consolas, monospace;
        font-size: 11px;
    }
    QStatusBar {
        background-color: #07080f;
        border-top: 1px solid #16203a;
        color: #5d6f8c;
    }
    QStatusBar::item { border: none; }
    QStatusBar QLabel {
        color: #5d6f8c;
        font-family: Consolas, monospace;
        font-size: 11px;
    }
    QLabel#hud {
        background-color: rgba(4, 6, 12, 210);
        border: 1px solid rgba(0, 229, 255, 55);
        border-radius: 7px;
        padding: 7px 12px;
    }
    QToolButton#menuButton::menu-indicator { image: none; width: 0px; }
    QMenu {
        background-color: #0c0f18;
        border: 1px solid #16203a;
        border-radius: 6px;
        padding: 5px;
        color: #d8e6f5;
    }
    QMenu::item {
        padding: 6px 26px 6px 26px;
        border-radius: 4px;
    }
    QMenu::item:selected { background: rgba(0, 229, 255, 40); color: #eaffff; }
    QMenu::item:disabled { color: #38455c; }
    QMenu::separator { height: 1px; background: #16203a; margin: 5px 8px; }
    QToolBar {
        background: transparent;
        border: none;
        border-bottom: 1px solid #101728;
        spacing: 3px;
        padding: 3px 10px;
    }
    QToolBar::separator {
        background: #16203a;
        width: 1px;
        margin: 5px 6px;
    }
    QToolButton {
        background: transparent;
        border: 1px solid transparent;
        border-radius: 4px;
        padding: 4px;
        color: #d8e6f5;
    }
    QToolButton:hover { background: rgba(0, 229, 255, 24); border-color: #26456e; }
    QToolButton:checked { background: rgba(0, 229, 255, 44); border-color: #00e5ff; }
    QDockWidget {
        color: #00e5ff;
        font-size: 10px;
        font-weight: 700;
        titlebar-close-icon: none;
        titlebar-normal-icon: none;
    }
    QDockWidget::title {
        background: #0c0f18;
        border: 1px solid #16203a;
        border-bottom: none;
        border-top-left-radius: 6px;
        border-top-right-radius: 6px;
        padding: 5px 10px;
        text-align: left;
    }
    QDockWidget::close-button, QDockWidget::float-button {
        background: transparent;
        border: none;
        icon-size: 10px;
    }
    QMainWindow::separator {
        background: #101728;
        width: 4px;
        height: 4px;
    }
    QMainWindow::separator:hover { background: #26456e; }
    QTabBar::tab { min-width: 84px; }
    QTabBar#fileTabs::tab {
        background: #0a0d15;
        color: #5d6f8c;
        border: 1px solid #16203a;
        border-bottom: none;
        border-top-left-radius: 5px;
        border-top-right-radius: 5px;
        padding: 5px 14px;
        margin-right: 2px;
        font-size: 11px;
        font-weight: 600;
    }
    QTabBar#fileTabs::tab:selected {
        background: #0c0f18;
        color: #00e5ff;
        border-color: #26456e;
    }
    QDialog { background-color: #0a0d15; }
    QMessageBox { background-color: #0c0f18; }
    QToolTip {
        background-color: #0c0f18;
        color: #d8e6f5;
        border: 1px solid #00e5ff;
        padding: 3px;
    }
)";



static QPixmap paintGlyph(const QString& name, const QColor& color) {

    const qreal dpr = 2.0;
    QPixmap pixmap(QSize(32, 32));
    pixmap.setDevicePixelRatio(dpr);
    pixmap.fill(Qt::transparent);

    QPainter p(&pixmap);
    p.setRenderHint(QPainter::Antialiasing, true);

    QPen pen(color, 1.4);
    pen.setCapStyle(Qt::RoundCap);
    pen.setJoinStyle(Qt::RoundJoin);
    p.setPen(pen);
    p.setBrush(Qt::NoBrush);

    if (name == "open") {

        QPainterPath path;
        path.moveTo(2.2, 13.2);
        path.lineTo(2.2, 4.2);
        path.lineTo(6.2, 4.2);
        path.lineTo(7.8, 6.2);
        path.lineTo(13.8, 6.2);
        path.lineTo(13.8, 13.2);
        path.closeSubpath();
        p.drawPath(path);

    } else if (name == "layers") {

        p.drawRect(QRectF(2.4, 2.4, 8.4, 8.4));
        p.drawRect(QRectF(5.6, 5.6, 8.4, 8.4));

    } else if (name == "cpu") {

        p.drawRect(QRectF(4.2, 4.2, 7.6, 7.6));
        p.drawRect(QRectF(6.8, 6.8, 2.4, 2.4));

        for (double t : { 6.4, 8.0, 9.6 }) {
            p.drawLine(QPointF(t, 2.2), QPointF(t, 4.2));
            p.drawLine(QPointF(t, 11.8), QPointF(t, 13.8));
            p.drawLine(QPointF(2.2, t), QPointF(4.2, t));
            p.drawLine(QPointF(11.8, t), QPointF(13.8, t));
        }

    } else if (name == "gpu") {

        p.drawRoundedRect(QRectF(1.8, 4.6, 12.4, 7.2), 1.2, 1.2);
        p.drawEllipse(QPointF(6.2, 8.2), 2.2, 2.2);
        p.drawLine(QPointF(10.0, 7.0), QPointF(12.6, 7.0));
        p.drawLine(QPointF(10.0, 9.4), QPointF(12.6, 9.4));
        p.drawLine(QPointF(4.4, 11.8), QPointF(4.4, 13.8));
        p.drawLine(QPointF(9.8, 11.8), QPointF(9.8, 13.8));

    } else if (name == "grid") {

        p.drawRect(QRectF(2.4, 2.4, 11.2, 11.2));
        p.drawLine(QPointF(6.1, 2.4), QPointF(6.1, 13.6));
        p.drawLine(QPointF(9.9, 2.4), QPointF(9.9, 13.6));
        p.drawLine(QPointF(2.4, 6.1), QPointF(13.6, 6.1));
        p.drawLine(QPointF(2.4, 9.9), QPointF(13.6, 9.9));

    } else if (name == "clip") {

        QPainterPath path;
        path.moveTo(8.0, 2.4);
        path.lineTo(14.0, 13.4);
        path.lineTo(2.0, 13.4);
        path.closeSubpath();
        p.drawPath(path);
        p.drawLine(QPointF(8.0, 6.6), QPointF(8.0, 9.8));
        p.drawPoint(QPointF(8.0, 11.4));

    } else if (name == "fit") {

        p.drawPolyline(QPolygonF({ QPointF(2.2, 5.4), QPointF(2.2, 2.2), QPointF(5.4, 2.2) }));
        p.drawPolyline(QPolygonF({ QPointF(10.6, 2.2), QPointF(13.8, 2.2), QPointF(13.8, 5.4) }));
        p.drawPolyline(QPolygonF({ QPointF(13.8, 10.6), QPointF(13.8, 13.8), QPointF(10.6, 13.8) }));
        p.drawPolyline(QPolygonF({ QPointF(5.4, 13.8), QPointF(2.2, 13.8), QPointF(2.2, 10.6) }));

    } else if (name == "one") {

        p.drawEllipse(QPointF(7.0, 7.0), 4.6, 4.6);
        p.drawLine(QPointF(10.4, 10.4), QPointF(13.6, 13.6));

    } else if (name == "mosaic") {

        p.fillRect(QRectF(2.6, 2.6, 5.4, 5.4), color);
        p.fillRect(QRectF(8.0, 8.0, 5.4, 5.4), color);
        p.drawRect(QRectF(8.0, 2.6, 5.4, 5.4));
        p.drawRect(QRectF(2.6, 8.0, 5.4, 5.4));

    } else if (name == "save") {

        p.drawLine(QPointF(8.0, 2.4), QPointF(8.0, 10.2));
        p.drawPolyline(QPolygonF({ QPointF(4.8, 7.0), QPointF(8.0, 10.4), QPointF(11.2, 7.0) }));
        p.drawLine(QPointF(3.0, 13.4), QPointF(13.0, 13.4));

    } else if (name == "image") {

        p.drawRoundedRect(QRectF(2.4, 3.4, 11.2, 9.2), 1.4, 1.4);
        p.drawEllipse(QPointF(5.6, 6.6), 1.1, 1.1);
        p.drawPolyline(QPolygonF({ QPointF(3.2, 12.0), QPointF(6.8, 8.2),
                                   QPointF(9.4, 10.6), QPointF(11.0, 9.2), QPointF(13.2, 12.0) }));

    } else if (name == "region") {

        QPen dashed(color, 1.4);
        dashed.setDashPattern({ 2.4, 2.0 });
        p.setPen(dashed);
        p.drawRect(QRectF(2.6, 3.6, 10.8, 8.8));
        p.setPen(pen);
        p.drawLine(QPointF(2.6, 3.6), QPointF(5.0, 3.6));
        p.drawLine(QPointF(11.0, 12.4), QPointF(13.4, 12.4));

    } else if (name == "row") {

        p.drawLine(QPointF(2.2, 4.0), QPointF(13.8, 4.0));
        p.fillRect(QRectF(2.2, 7.0, 11.6, 2.2), color);
        p.drawLine(QPointF(2.2, 12.0), QPointF(13.8, 12.0));

    } else if (name == "column") {

        p.drawLine(QPointF(4.0, 2.2), QPointF(4.0, 13.8));
        p.fillRect(QRectF(7.0, 2.2, 2.2, 11.6), color);
        p.drawLine(QPointF(12.0, 2.2), QPointF(12.0, 13.8));

    } else if (name == "csv") {

        QPainterPath path;
        path.moveTo(3.4, 2.4);
        path.lineTo(9.4, 2.4);
        path.lineTo(12.8, 5.8);
        path.lineTo(12.8, 13.6);
        path.lineTo(3.4, 13.6);
        path.closeSubpath();
        p.drawPath(path);
        p.drawPolyline(QPolygonF({ QPointF(9.4, 2.4), QPointF(9.4, 5.8), QPointF(12.8, 5.8) }));
        p.drawLine(QPointF(5.6, 8.6), QPointF(10.6, 8.6));
        p.drawLine(QPointF(5.6, 11.0), QPointF(10.6, 11.0));

    } else if (name == "compare") {

        p.drawLine(QPointF(3.0, 5.6), QPointF(13.0, 5.6));
        p.drawPolyline(QPolygonF({ QPointF(10.4, 3.2), QPointF(13.0, 5.6), QPointF(10.4, 8.0) }));
        p.drawLine(QPointF(13.0, 10.4), QPointF(3.0, 10.4));
        p.drawPolyline(QPolygonF({ QPointF(5.6, 8.0), QPointF(3.0, 10.4), QPointF(5.6, 12.8) }));

    } else if (name == "filter") {

        p.drawPolygon(QPolygonF({ QPointF(2.4, 3.0), QPointF(13.6, 3.0), QPointF(9.3, 8.4),
                                  QPointF(9.3, 12.4), QPointF(6.7, 13.8), QPointF(6.7, 8.4) }));

    } else if (name == "minus") {

        p.drawEllipse(QPointF(8.0, 8.0), 5.6, 5.6);
        p.drawLine(QPointF(5.0, 8.0), QPointF(11.0, 8.0));

    } else if (name == "menu") {

        p.drawLine(QPointF(2.6, 4.4), QPointF(13.4, 4.4));
        p.drawLine(QPointF(2.6, 8.0), QPointF(13.4, 8.0));
        p.drawLine(QPointF(2.6, 11.6), QPointF(13.4, 11.6));

    } else if (name == "ruler") {

        p.drawRect(QRectF(2.2, 5.4, 11.6, 5.2));
        p.drawLine(QPointF(5.0, 5.4), QPointF(5.0, 8.2));
        p.drawLine(QPointF(8.0, 5.4), QPointF(8.0, 9.4));
        p.drawLine(QPointF(11.0, 5.4), QPointF(11.0, 8.2));

    } else if (name == "map") {

        p.drawRect(QRectF(2.2, 3.4, 11.6, 9.2));
        p.drawRect(QRectF(4.6, 5.6, 5.4, 4.4));

    } else if (name == "expand") {

        p.drawPolyline(QPolygonF({ QPointF(6.0, 2.4), QPointF(2.4, 2.4), QPointF(2.4, 6.0) }));
        p.drawPolyline(QPolygonF({ QPointF(10.0, 13.6), QPointF(13.6, 13.6), QPointF(13.6, 10.0) }));
        p.drawLine(QPointF(2.4, 2.4), QPointF(6.6, 6.6));
        p.drawLine(QPointF(13.6, 13.6), QPointF(9.4, 9.4));

    } else if (name == "copy") {

        p.drawRect(QRectF(5.4, 5.4, 8.2, 8.2));
        p.drawPolyline(QPolygonF({ QPointF(10.6, 2.4), QPointF(2.4, 2.4), QPointF(2.4, 10.6) }));

    } else if (name == "check") {

        p.drawPolyline(QPolygonF({ QPointF(3.0, 8.4), QPointF(6.6, 12.0), QPointF(13.0, 4.4) }));

    } else if (name == "plus") {

        p.drawEllipse(QPointF(8.0, 8.0), 5.6, 5.6);
        p.drawLine(QPointF(5.0, 8.0), QPointF(11.0, 8.0));
        p.drawLine(QPointF(8.0, 5.0), QPointF(8.0, 11.0));

    } else if (name == "trash") {

        p.drawLine(QPointF(2.6, 4.4), QPointF(13.4, 4.4));
        p.drawPolyline(QPolygonF({ QPointF(6.2, 4.4), QPointF(6.2, 2.6),
                                   QPointF(9.8, 2.6), QPointF(9.8, 4.4) }));
        p.drawPolyline(QPolygonF({ QPointF(4.0, 4.4), QPointF(4.8, 13.4),
                                   QPointF(11.2, 13.4), QPointF(12.0, 4.4) }));

    } else if (name == "rotate") {

        p.drawArc(QRectF(3.0, 3.2, 10.0, 10.0), 20 * 16, 290 * 16);
        p.drawPolyline(QPolygonF({ QPointF(9.6, 2.0), QPointF(12.8, 4.2),
                                   QPointF(10.4, 6.4) }));

    } else if (name == "revert") {

        p.drawArc(QRectF(3.0, 3.2, 10.0, 10.0), 250 * 16, 290 * 16);
        p.drawPolyline(QPolygonF({ QPointF(6.4, 2.0), QPointF(3.2, 4.2),
                                   QPointF(5.6, 6.4) }));

    } else if (name == "black") {

        p.setBrush(color);
        p.drawPie(QRectF(2.4, 2.4, 11.2, 11.2), 90 * 16, 180 * 16);
        p.setBrush(Qt::NoBrush);
        p.drawEllipse(QPointF(8.0, 8.0), 5.6, 5.6);
    }

    p.end();
    return pixmap;
}


static QIcon makeIcon(const QString& name) {

    QIcon icon;
    icon.addPixmap(paintGlyph(name, QColor("#cdd3dc")), QIcon::Normal, QIcon::Off);
    icon.addPixmap(paintGlyph(name, QColor("#131820")), QIcon::Normal, QIcon::On);
    icon.addPixmap(paintGlyph(name, QColor("#5b626d")), QIcon::Disabled, QIcon::Off);
    return icon;
}


class BackgroundWidget : public QWidget {
public:
    explicit BackgroundWidget(QWidget* parent = nullptr) : QWidget(parent) {
        setObjectName("background");
    }

protected:
    void paintEvent(QPaintEvent*) override {

        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing, true);
        painter.fillRect(rect(), QColor("#04050a"));

        QRadialGradient cool(QPointF(width() * 0.10, -height() * 0.10),
                             std::max(width(), height()) * 0.85);
        cool.setColorAt(0.0, QColor(0, 229, 255, 34));
        cool.setColorAt(1.0, QColor(0, 229, 255, 0));
        painter.fillRect(rect(), cool);

        QRadialGradient warm(QPointF(width() * 1.05, height() * 1.10),
                             std::max(width(), height()) * 0.80);
        warm.setColorAt(0.0, QColor(255, 47, 185, 30));
        warm.setColorAt(1.0, QColor(255, 47, 185, 0));
        painter.fillRect(rect(), warm);
    }
};


class TitleBar : public QWidget {
public:
    explicit TitleBar(QWidget* parent = nullptr) : QWidget(parent) {

        setObjectName("titleBar");
        setFixedHeight(38);

        auto* layout = new QHBoxLayout(this);
        layout->setContentsMargins(14, 0, 6, 0);
        layout->setSpacing(10);

        titleText = new QLabel("Raw Image Process");
        titleText->setObjectName("titleText");

        fileText = new QLabel();
        fileText->setObjectName("titleFile");

        minimizeBtn = new QPushButton(QString::fromUtf8("\u2013"));
        maximizeBtn = new QPushButton(QString::fromUtf8("\u25a1"));
        closeBtn = new QPushButton(QString::fromUtf8("\u2715"));

        minimizeBtn->setObjectName("winBtn");
        maximizeBtn->setObjectName("winBtn");
        closeBtn->setObjectName("winClose");

        layout->addSpacing(14);
        layout->addWidget(titleText);
        layout->addStretch();
        layout->addWidget(fileText);
        layout->addSpacing(12);
        layout->addWidget(minimizeBtn);
        layout->addWidget(maximizeBtn);
        layout->addWidget(closeBtn);
    }

    void setFileName(const QString& name) { fileText->setText(name); }

    bool overButton(const QPoint& local) const {

        return minimizeBtn->geometry().contains(local) ||
               maximizeBtn->geometry().contains(local) ||
               closeBtn->geometry().contains(local);
    }

    QPushButton* minimizeBtn;
    QPushButton* maximizeBtn;
    QPushButton* closeBtn;

protected:
    void paintEvent(QPaintEvent*) override {

        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing, true);

        QRectF dot(14.0, height() / 2.0 - 5.0, 10.0, 10.0);

        QRadialGradient glow(dot.center(), 14.0);
        glow.setColorAt(0.0, QColor(0, 229, 255, 150));
        glow.setColorAt(1.0, QColor(0, 229, 255, 0));
        painter.setPen(Qt::NoPen);
        painter.setBrush(glow);
        painter.drawEllipse(dot.center(), 14.0, 14.0);

        painter.setBrush(QColor("#00e5ff"));
        painter.drawRoundedRect(dot, 3, 3);

        painter.setPen(QPen(QColor(0, 229, 255, 45), 1));
        painter.drawLine(0, height() - 1, width(), height() - 1);
    }

private:
    QLabel* titleText;
    QLabel* fileText;
};


class HistogramWidget : public QWidget {
public:
    explicit HistogramWidget(QWidget* parent = nullptr) : QWidget(parent) {
        setMinimumHeight(150);
    }

    QSize sizeHint() const override { return QSize(260, 160); }

    void setData(const std::array<std::vector<uint32_t>, 4>& data,
                 const std::array<std::string, 4>& names) {

        hist = data;
        labels = names;
        update();
    }

    void clear() {
        for (auto& channel : hist) channel.clear();
        update();
    }

protected:
    void paintEvent(QPaintEvent*) override {

        QPainter painter(this);
        painter.fillRect(rect(), QColor("#06070d"));

        QRect plot = rect().adjusted(6, 6, -6, -18);

        uint32_t peak = 0;
        for (const auto& channel : hist)
            for (uint32_t value : channel) peak = std::max(peak, value);

        if (peak == 0 || plot.width() <= 2 || plot.height() <= 2) {

            painter.setPen(QColor("#4d5c75"));
            painter.drawText(rect(), Qt::AlignCenter, "no region selected");
            return;
        }

        painter.setPen(QPen(QColor(0, 229, 255, 26), 1));
        for (int i = 1; i < 4; ++i) {
            int x = plot.left() + plot.width() * i / 4;
            painter.drawLine(x, plot.top(), x, plot.bottom());
        }

        double logPeak = std::log10(static_cast<double>(peak) + 1.0);
        painter.setRenderHint(QPainter::Antialiasing, true);

        for (int k = 0; k < 4; ++k) {

            if (hist[k].size() < 2) continue;

            QPolygonF curve;
            int bins = static_cast<int>(hist[k].size());

            for (int i = 0; i < bins; ++i) {

                double norm = std::log10(static_cast<double>(hist[k][i]) + 1.0) / logPeak;

                curve << QPointF(plot.left() + plot.width() * i / double(bins - 1),
                                 plot.bottom() - norm * plot.height());
            }

            painter.setPen(QPen(channelColor(k), 1.3));
            painter.drawPolyline(curve);
        }

        painter.setRenderHint(QPainter::Antialiasing, false);

        int legendX = plot.left();
        painter.setFont(QFont("Consolas", 7));

        for (int k = 0; k < 4; ++k) {

            painter.setPen(channelColor(k));
            QString text = QString::fromStdString(labels[k].empty() ? "-" : labels[k]);

            painter.drawText(legendX, rect().bottom() - 4, text);
            legendX += 34;
        }

        painter.setPen(QColor("#4d5c75"));
        painter.drawText(plot.right() - 60, rect().bottom() - 4,
                         QString("peak %1").arg(peak));
    }

private:
    static QColor channelColor(int k) {

        static const QColor colors[4] = { QColor("#ff2fb9"), QColor("#38ff9b"),
                                          QColor("#b9ff4d"), QColor("#00e5ff") };
        return colors[k & 3];
    }

    std::array<std::vector<uint32_t>, 4> hist;
    std::array<std::string, 4> labels;
};


class ProfileWidget : public QWidget {
public:
    explicit ProfileWidget(QWidget* parent = nullptr) : QWidget(parent) {
        setMinimumHeight(150);
    }

    QSize sizeHint() const override { return QSize(260, 160); }

    void setData(const std::vector<double>& data, const QString& text) {
        values = data;
        caption = text;
        update();
    }

    void clear() {
        values.clear();
        caption.clear();
        update();
    }

protected:
    void paintEvent(QPaintEvent*) override {

        QPainter painter(this);
        painter.fillRect(rect(), QColor("#06070d"));

        QRect plot = rect().adjusted(6, 18, -6, -18);

        if (values.size() < 2 || plot.width() <= 2 || plot.height() <= 2) {

            painter.setPen(QColor("#4d5c75"));
            painter.drawText(rect(), Qt::AlignCenter, "no region selected");
            return;
        }

        double lo = *std::min_element(values.begin(), values.end());
        double hi = *std::max_element(values.begin(), values.end());
        double span = std::max(1e-6, hi - lo);

        painter.setPen(QPen(QColor(0, 229, 255, 24), 1));
        painter.drawLine(plot.left(), plot.top(), plot.right(), plot.top());
        painter.drawLine(plot.left(), plot.bottom(), plot.right(), plot.bottom());

        QPolygonF curve;
        int n = static_cast<int>(values.size());

        for (int i = 0; i < n; ++i) {

            double norm = (values[i] - lo) / span;

            curve << QPointF(plot.left() + plot.width() * i / double(n - 1),
                             plot.bottom() - norm * plot.height());
        }

        painter.setRenderHint(QPainter::Antialiasing, true);
        painter.setPen(QPen(QColor("#38ff9b"), 1.3));
        painter.drawPolyline(curve);
        painter.setRenderHint(QPainter::Antialiasing, false);

        painter.setFont(QFont("Consolas", 7));
        painter.setPen(QColor("#4d5c75"));
        painter.drawText(plot.left(), 12, caption);
        painter.drawText(plot.left(), rect().bottom() - 4,
                         QString("min %1   max %2   n %3")
                             .arg(lo, 0, 'f', 1).arg(hi, 0, 'f', 1).arg(n));
    }

private:
    std::vector<double> values;
    QString caption;
};


class NoiseCompareWidget : public QWidget {
public:
    explicit NoiseCompareWidget(QWidget* parent = nullptr) : QWidget(parent) {
        setMinimumHeight(120);
        setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    }

    QSize sizeHint() const override { return QSize(800, 420); }

    void setCurves(const std::vector<uint32_t>& before, const std::vector<uint32_t>& after,
                   const QString& beforeLabel, const QString& afterLabel, const QString& captionText) {

        beforeCurve = before;
        afterCurve = after;
        beforeText = beforeLabel;
        afterText = afterLabel;
        caption = captionText;

        update();
    }

    void setAxis(double from, double to, const QString& unitText) {

        axisFrom = from;
        axisTo = to > from ? to : from + 1.0;
        unit = unitText;

        update();
    }

    void setMarkers(double before, double after) {

        beforeSigma = before;
        afterSigma = after;

        update();
    }

    void setLogScale(bool on) {

        logScale = on;
        update();
    }

    void setEmptyText(const QString& text) {

        emptyText = text;
        update();
    }

protected:
    void paintEvent(QPaintEvent*) override {

        QPainter painter(this);
        painter.fillRect(rect(), QColor("#06070d"));
        painter.setPen(QColor("#16203a"));
        painter.drawRect(rect().adjusted(0, 0, -1, -1));

        uint32_t peak = 0;
        for (uint32_t value : beforeCurve) peak = std::max(peak, value);
        for (uint32_t value : afterCurve) peak = std::max(peak, value);

        QRect plot = rect().adjusted(66, 38, -22, -42);

        if (peak == 0 || plot.width() <= 40 || plot.height() <= 30) {

            QFont hintFont("Segoe UI");
            hintFont.setPixelSize(13);
            painter.setFont(hintFont);
            painter.setPen(QColor("#4d5c75"));
            painter.drawText(rect(), Qt::AlignCenter, emptyText.isEmpty() ? QString("no data") : emptyText);
            return;
        }

        QFont mono("Consolas");
        mono.setPixelSize(10);
        painter.setFont(mono);
        QFontMetrics metrics(mono);

        double top = logScale ? std::log10(static_cast<double>(peak) + 1.0) : static_cast<double>(peak);

        QPen gridPen(QColor(0, 229, 255, 22), 1);

        for (int i = 0; i <= 4; ++i) {

            int y = plot.bottom() - plot.height() * i / 4;
            double fraction = i / 4.0;
            double count = logScale ? std::pow(10.0, fraction * top) - 1.0 : fraction * top;

            painter.setPen(gridPen);
            painter.drawLine(plot.left(), y, plot.right(), y);

            painter.setPen(QColor("#4d5c75"));
            painter.drawText(QRect(20, y - 7, plot.left() - 28, 14),
                             Qt::AlignRight | Qt::AlignVCenter, compactCount(count));
        }

        bool wide = std::fabs(axisTo - axisFrom) >= 80.0;

        for (int i = 0; i <= 8; ++i) {

            int x = plot.left() + plot.width() * i / 8;
            double value = axisFrom + (axisTo - axisFrom) * i / 8.0;

            painter.setPen(gridPen);
            painter.drawLine(x, plot.top(), x, plot.bottom());

            painter.setPen(QColor("#4d5c75"));
            painter.drawText(QRect(x - 40, plot.bottom() + 5, 80, 14), Qt::AlignHCenter | Qt::AlignTop,
                             QString::number(value, 'f', wide ? 0 : 1));
        }

        auto toX = [&](double value) {
            return plot.left() + (value - axisFrom) / (axisTo - axisFrom) * plot.width();
        };

        if (axisFrom < 0.0 && axisTo > 0.0) {
            painter.setPen(QPen(QColor(0, 229, 255, 70), 1, Qt::DashLine));
            painter.drawLine(QPointF(toX(0.0), plot.top()), QPointF(toX(0.0), plot.bottom()));
        }

        painter.save();
        painter.setClipRect(plot.adjusted(0, -2, 1, 1));
        painter.setRenderHint(QPainter::Antialiasing, true);

        drawCurve(painter, plot, beforeCurve, top, QColor("#ff2fb9"), 60);
        drawCurve(painter, plot, afterCurve, top, QColor("#00e5ff"), 34);

        auto marker = [&](double sigma, const QColor& color, int row) {

            if (sigma <= 0.0 || axisFrom >= 0.0) return;

            painter.setPen(QPen(color, 1, Qt::DotLine));

            for (double value : { -sigma, sigma }) {

                if (value <= axisFrom || value >= axisTo) continue;

                double x = toX(value);
                painter.drawLine(QPointF(x, plot.top()), QPointF(x, plot.bottom()));
                painter.drawText(QPointF(x + 4, plot.top() + 12 + row * 13),
                                 value < 0.0 ? QString("-σ") : QString("+σ"));
            }
        };

        marker(beforeSigma, QColor(255, 47, 185, 170), 0);
        marker(afterSigma, QColor(0, 229, 255, 190), 1);

        painter.restore();

        int legendX = plot.left();

        auto legend = [&](const QString& text, const QColor& color) {

            if (text.isEmpty()) return;

            painter.fillRect(QRect(legendX, 17, 14, 3), color);
            painter.setPen(color);
            painter.drawText(legendX + 20, 22, text);
            legendX += 20 + metrics.horizontalAdvance(text) + 28;
        };

        legend(beforeText, QColor("#ff2fb9"));
        legend(afterText, QColor("#00e5ff"));

        painter.setPen(QColor("#5d6f8c"));
        painter.drawText(QRect(legendX, 10, std::max(0, plot.right() - legendX), 16),
                         Qt::AlignRight | Qt::AlignVCenter, caption);
        painter.drawText(QRect(plot.left(), rect().bottom() - 19, plot.width(), 16),
                         Qt::AlignRight | Qt::AlignVCenter, unit);

        painter.save();
        painter.translate(13, plot.center().y());
        painter.rotate(-90);
        painter.drawText(QRect(-70, -7, 140, 14), Qt::AlignCenter, logScale ? "count (log)" : "count");
        painter.restore();
    }

private:
    static QString compactCount(double count) {

        if (count >= 1e6) return QString::number(count / 1e6, 'f', 1) + "M";
        if (count >= 1e3) return QString::number(count / 1e3, 'f', 1) + "k";
        return QString::number(count, 'f', 0);
    }

    void drawCurve(QPainter& painter, const QRect& plot, const std::vector<uint32_t>& data,
                   double top, const QColor& color, int fillAlpha) const {

        if (data.size() < 2 || top <= 0.0) return;

        QPolygonF curve;
        int bins = static_cast<int>(data.size());

        for (int i = 0; i < bins; ++i) {

            double level = logScale ? std::log10(static_cast<double>(data[i]) + 1.0)
                                    : static_cast<double>(data[i]);

            curve << QPointF(plot.left() + plot.width() * i / double(bins - 1),
                             plot.bottom() - level / top * plot.height());
        }

        QPolygonF area = curve;
        area << QPointF(plot.left() + plot.width(), plot.bottom()) << QPointF(plot.left(), plot.bottom());

        QLinearGradient gradient(0, plot.top(), 0, plot.bottom());
        gradient.setColorAt(0.0, QColor(color.red(), color.green(), color.blue(), fillAlpha));
        gradient.setColorAt(1.0, QColor(color.red(), color.green(), color.blue(), fillAlpha / 5));

        painter.setPen(Qt::NoPen);
        painter.setBrush(gradient);
        painter.drawPolygon(area);

        painter.setBrush(Qt::NoBrush);
        painter.setPen(QPen(color, 1.6));
        painter.drawPolyline(curve);
    }

    std::vector<uint32_t> beforeCurve;
    std::vector<uint32_t> afterCurve;
    QString beforeText;
    QString afterText;
    QString caption;
    QString unit;
    QString emptyText;
    double axisFrom = 0.0;
    double axisTo = 1.0;
    double beforeSigma = 0.0;
    double afterSigma = 0.0;
    bool logScale = true;
};


struct NoiseSnapshot {

    std::array<std::vector<uint32_t>, 4> values;
    std::array<std::vector<uint32_t>, 4> noise;
    std::array<double, 4> mean = {0.0, 0.0, 0.0, 0.0};
    std::array<double, 4> sigma = {0.0, 0.0, 0.0, 0.0};
    std::array<std::string, 4> labels;
    double span = 0.0;
    int bins = 0;
    double valueCenter = 0.0;
    double valueSpan = 0.0;
    int valueBins = 0;
    QString title;
    bool valid = false;
};


class NoisePage : public QWidget {
public:
    explicit NoisePage(QWidget* parent = nullptr) : QWidget(parent) {

        setObjectName("noisePage");

        auto* layout = new QHBoxLayout(this);
        layout->setContentsMargins(0, 4, 0, 0);
        layout->setSpacing(10);

        auto* left = new QVBoxLayout();
        left->setSpacing(8);

        auto* topRow = new QHBoxLayout();
        topRow->setSpacing(6);

        const char* names[5] = { "ALL", "R", "G1", "G2", "B" };

        for (int i = 0; i < 5; ++i) {

            channelButtons[i] = makeToggle(names[i]);
            channelButtons[i]->setMinimumWidth(46);
            channelGroup.addButton(channelButtons[i], i);
            topRow->addWidget(channelButtons[i]);
        }

        channelButtons[0]->setChecked(true);
        channelGroup.setExclusive(true);

        topRow->addSpacing(18);

        noiseModeBtn = makeToggle("Noise");
        valueModeBtn = makeToggle("Values");
        noiseModeBtn->setChecked(true);
        noiseModeBtn->setToolTip("Histogram of deviations from the channel mean");
        valueModeBtn->setToolTip("Histogram of raw values");

        modeGroup.addButton(noiseModeBtn);
        modeGroup.addButton(valueModeBtn);
        modeGroup.setExclusive(true);

        topRow->addWidget(noiseModeBtn);
        topRow->addWidget(valueModeBtn);
        topRow->addSpacing(18);

        logBtn = makeToggle("Log scale");
        logBtn->setChecked(true);
        topRow->addWidget(logBtn);
        topRow->addStretch();

        auto* cardsRow = new QHBoxLayout();
        cardsRow->setSpacing(8);

        beforeValue = makeCard(cardsRow, "NOISE σ  BEFORE", "cardValueBefore");
        afterValue = makeCard(cardsRow, "NOISE σ  AFTER", "cardValue");
        changeValue = makeCard(cardsRow, "CHANGE", "cardValue");
        snrValue = makeCard(cardsRow, "SNR  AFTER", "cardValue");

        plot = new NoiseCompareWidget();
        plot->setEmptyText("open a RAW file to see noise histograms");

        table = new QTableWidget(4, 7);
        table->setHorizontalHeaderLabels({ "CH", "σ BEFORE", "σ AFTER", "CHANGE",
                                           "MEAN BEFORE", "MEAN AFTER", "SNR AFTER" });
        table->verticalHeader()->setVisible(false);
        table->verticalHeader()->setDefaultSectionSize(22);
        table->setEditTriggers(QAbstractItemView::NoEditTriggers);
        table->setSelectionMode(QAbstractItemView::NoSelection);
        table->setFocusPolicy(Qt::NoFocus);
        table->setAlternatingRowColors(true);
        table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
        table->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        table->setMinimumHeight(70);
        table->setMaximumHeight(126);

        left->addLayout(topRow);
        left->addLayout(cardsRow);
        left->addWidget(plot, 1);
        left->addWidget(table);

        auto* panel = new QWidget();
        panel->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);

        auto* panelLayout = new QVBoxLayout(panel);
        panelLayout->setContentsMargins(0, 0, 8, 0);
        panelLayout->setSpacing(10);

        auto* filterGroup = new QGroupBox("DENOISE FILTER");
        auto* filterLayout = new QVBoxLayout(filterGroup);
        filterLayout->setSpacing(6);

        gaussianBtn = makeToggle("Gaussian");
        medianBtn = makeToggle("Median");
        bilateralBtn = makeToggle("Bilateral");
        gaussianBtn->setChecked(true);

        methodGroup.addButton(gaussianBtn, 0);
        methodGroup.addButton(medianBtn, 1);
        methodGroup.addButton(bilateralBtn, 2);
        methodGroup.setExclusive(true);

        auto* methodRow = new QHBoxLayout();
        methodRow->setSpacing(4);
        methodRow->addWidget(gaussianBtn);
        methodRow->addWidget(medianBtn);
        methodRow->addWidget(bilateralBtn);

        methodHint = new QLabel();
        methodHint->setObjectName("hint");
        methodHint->setWordWrap(true);

        sigmaBox = new QWidget();
        auto* sigmaLayout = new QVBoxLayout(sigmaBox);
        sigmaLayout->setContentsMargins(0, 0, 0, 0);
        sigmaLayout->setSpacing(2);
        sigmaLabel = new QLabel();
        sigmaSlider = new QSlider(Qt::Horizontal);
        sigmaSlider->setRange(3, 30);
        sigmaSlider->setValue(10);
        sigmaLayout->addWidget(sigmaLabel);
        sigmaLayout->addWidget(sigmaSlider);

        sizeBox = new QWidget();
        auto* sizeLayout = new QVBoxLayout(sizeBox);
        sizeLayout->setContentsMargins(0, 0, 0, 0);
        sizeLayout->setSpacing(4);
        size3Btn = makeToggle("3 x 3");
        size5Btn = makeToggle("5 x 5");
        size3Btn->setChecked(true);
        sizeGroup.addButton(size3Btn, 3);
        sizeGroup.addButton(size5Btn, 5);
        sizeGroup.setExclusive(true);
        auto* sizeRow = new QHBoxLayout();
        sizeRow->setSpacing(4);
        sizeRow->addWidget(size3Btn);
        sizeRow->addWidget(size5Btn);
        sizeLayout->addWidget(new QLabel("Window per channel"));
        sizeLayout->addLayout(sizeRow);

        rangeBox = new QWidget();
        auto* rangeLayout = new QVBoxLayout(rangeBox);
        rangeLayout->setContentsMargins(0, 0, 0, 0);
        rangeLayout->setSpacing(2);
        auto* rangeHeader = new QHBoxLayout();
        rangeLabel = new QLabel();
        autoRangeBtn = new QPushButton("Auto");
        autoRangeBtn->setObjectName("pairButton");
        autoRangeBtn->setFixedWidth(56);
        autoRangeBtn->setToolTip("Range = 2.5 x current noise sigma (select a flat region first)");
        rangeHeader->addWidget(rangeLabel, 1);
        rangeHeader->addWidget(autoRangeBtn);
        rangeSlider = new QSlider(Qt::Horizontal);
        rangeSlider->setRange(1, 1000);
        rangeSlider->setValue(40);
        rangeLayout->addLayout(rangeHeader);
        rangeLayout->addWidget(rangeSlider);

        regionOnlyBtn = new QPushButton("Selected region only");
        regionOnlyBtn->setIcon(makeIcon("region"));
        regionOnlyBtn->setCheckable(true);
        regionOnlyBtn->setChecked(true);
        regionOnlyBtn->setToolTip("Filter only the selected region, without a selection the whole frame is filtered");

        applyBtn = new QPushButton("Apply filter");
        applyBtn->setIcon(makeIcon("filter"));

        progressBar = new QProgressBar();
        progressBar->setRange(0, 100);
        progressBar->setTextVisible(false);
        progressBar->setVisible(false);

        stopBtn = new QPushButton("Cancel");
        stopBtn->setVisible(false);

        auto* channelHint = new QLabel("R, G1, G2 and B are filtered separately, so colors never mix. Ctrl+Z undoes the filter");
        channelHint->setObjectName("hint");
        channelHint->setWordWrap(true);

        filterLayout->addLayout(methodRow);
        filterLayout->addWidget(methodHint);
        filterLayout->addWidget(sigmaBox);
        filterLayout->addWidget(sizeBox);
        filterLayout->addWidget(rangeBox);
        filterLayout->addWidget(regionOnlyBtn);
        filterLayout->addWidget(applyBtn);
        filterLayout->addWidget(progressBar);
        filterLayout->addWidget(stopBtn);
        filterLayout->addWidget(channelHint);

        auto* referenceGroup = new QGroupBox("BEFORE REFERENCE");
        auto* referenceLayout = new QVBoxLayout(referenceGroup);
        referenceLayout->setSpacing(6);

        referenceText = new QLabel("—");
        referenceText->setObjectName("statsValue");
        referenceText->setWordWrap(true);

        captureBtn = new QPushButton("Capture current as before");
        captureBtn->setIcon(makeIcon("image"));

        originalBtn = new QPushButton("Use original file as before");
        originalBtn->setIcon(makeIcon("revert"));

        lockBtn = new QPushButton("Lock reference");
        lockBtn->setIcon(makeIcon("check"));
        lockBtn->setCheckable(true);

        auto* lockHint = new QLabel("locked: every next filter is compared with the same reference, for example the original file");
        lockHint->setObjectName("hint");
        lockHint->setWordWrap(true);

        referenceLayout->addWidget(referenceText);
        referenceLayout->addWidget(captureBtn);
        referenceLayout->addWidget(originalBtn);
        referenceLayout->addWidget(lockBtn);
        referenceLayout->addWidget(lockHint);

        panelLayout->addWidget(filterGroup);
        panelLayout->addWidget(referenceGroup);
        panelLayout->addStretch();

        panelScroll = new QScrollArea();
        panelScroll->setWidget(panel);
        panelScroll->setWidgetResizable(true);
        panelScroll->setFrameShape(QFrame::NoFrame);
        panelScroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        panelScroll->setFixedWidth(296);

        layout->addLayout(left, 1);
        layout->addWidget(panelScroll);

        connect(&channelGroup, &QButtonGroup::idClicked, this, [this](int) { notifyView(); });
        connect(noiseModeBtn, &QPushButton::toggled, this, [this](bool) { notifyView(); });
        connect(logBtn, &QPushButton::toggled, this, [this](bool on) { plot->setLogScale(on); });

        connect(&methodGroup, &QButtonGroup::idToggled, this, [this](int, bool on) {
            if (on) updateFilterControls();
        });

        connect(&sizeGroup, &QButtonGroup::idToggled, this, [this](int, bool on) {
            if (on) updateFilterControls();
        });

        connect(sigmaSlider, &QSlider::valueChanged, this, [this](int) { updateFilterControls(); });
        connect(rangeSlider, &QSlider::valueChanged, this, [this](int) { updateFilterControls(); });

        connect(applyBtn, &QPushButton::clicked, this, [this]() { if (onApply) onApply(); });
        connect(captureBtn, &QPushButton::clicked, this, [this]() { if (onCapture) onCapture(); });
        connect(originalBtn, &QPushButton::clicked, this, [this]() { if (onUseOriginal) onUseOriginal(); });
        connect(autoRangeBtn, &QPushButton::clicked, this, [this]() { if (onAutoRange) onAutoRange(); });

        connect(stopBtn, &QPushButton::clicked, this, [this]() {
            stopBtn->setEnabled(false);
            if (onCancel) onCancel();
        });

        updateFilterControls();
        clearSummary();
    }

    int channel() const { return channelGroup.checkedId(); }
    bool noiseMode() const { return noiseModeBtn->isChecked(); }
    bool regionOnly() const { return regionOnlyBtn->isChecked(); }
    bool lockBefore() const { return lockBtn->isChecked(); }

    DenoiseParams params() const {

        DenoiseParams result;
        int id = methodGroup.checkedId();

        result.method = id == 1 ? DenoiseMethod::Median
                      : id == 2 ? DenoiseMethod::Bilateral : DenoiseMethod::Gaussian;
        result.sigma = sigmaSlider->value() / 10.0;
        result.medianSize = size5Btn->isChecked() ? 5 : 3;
        result.rangeSigma = rangeSlider->value();

        return result;
    }

    void setMethod(DenoiseMethod method) {

        if (method == DenoiseMethod::Median) medianBtn->setChecked(true);
        else if (method == DenoiseMethod::Bilateral) bilateralBtn->setChecked(true);
        else gaussianBtn->setChecked(true);
    }

    void setRangeSigma(double value) {
        rangeSlider->setValue(std::clamp(static_cast<int>(std::lround(value)), 1, rangeSlider->maximum()));
    }

    void setReferenceText(const QString& text) { referenceText->setText(text); }

    void setCards(double before, double after, double change, double snr) {

        beforeValue->setText(QString::number(before, 'f', 2));
        afterValue->setText(QString::number(after, 'f', 2));
        changeValue->setText(QString("%1%2%").arg(change > 0.0 ? "+" : "").arg(change, 0, 'f', 1));
        changeValue->setStyleSheet(change < -0.05 ? "color: #7cf7c4;" : change > 0.05 ? "color: #ff2fb9;" : "");
        snrValue->setText(snr > 0.0 ? QString("%1 dB").arg(20.0 * std::log10(snr), 0, 'f', 1) : QString("—"));
    }

    void setTable(const NoiseSnapshot& before, const NoiseSnapshot& after, double black, int highlighted) {

        for (int k = 0; k < 4; ++k) {

            double was = before.valid ? before.sigma[k] : 0.0;
            double now = after.sigma[k];
            double change = was > 0.0 ? (now - was) / was * 100.0 : 0.0;
            double signal = after.mean[k] - black;
            double snr = now > 0.0 && signal > 0.0 ? signal / now : 0.0;

            QStringList cells = {
                QString::fromStdString(after.labels[k]),
                QString::number(was, 'f', 2),
                QString::number(now, 'f', 2),
                QString("%1%2%").arg(change > 0.0 ? "+" : "").arg(change, 0, 'f', 1),
                QString::number(before.valid ? before.mean[k] : 0.0, 'f', 1),
                QString::number(after.mean[k], 'f', 1),
                snr > 0.0 ? QString("%1 dB").arg(20.0 * std::log10(snr), 0, 'f', 1) : QString("—")
            };

            QColor changeColor = change < -0.05 ? QColor("#7cf7c4")
                               : change > 0.05 ? QColor("#ff2fb9") : QColor("#5d6f8c");

            bool dimmed = highlighted >= 0 && k != highlighted;

            for (int column = 0; column < 7; ++column) {

                auto* item = new QTableWidgetItem(cells[column]);
                item->setTextAlignment(Qt::AlignCenter);

                if (dimmed) item->setForeground(QColor("#38455c"));
                else if (column == 1) item->setForeground(QColor("#ff2fb9"));
                else if (column == 2) item->setForeground(QColor("#00e5ff"));
                else if (column == 3) item->setForeground(changeColor);

                table->setItem(k, column, item);
            }
        }
    }

    void clearSummary() {

        for (QLabel* label : { beforeValue, afterValue, changeValue, snrValue }) label->setText("—");
        changeValue->setStyleSheet("");

        for (int k = 0; k < 4; ++k)
            for (int column = 0; column < 7; ++column)
                table->setItem(k, column, new QTableWidgetItem(column == 0 ? QString("—") : QString()));
    }

    void startProgress(bool cancellable) {

        progressBar->setValue(0);
        progressBar->setVisible(true);
        stopBtn->setEnabled(true);
        stopBtn->setVisible(cancellable);
    }

    void updateProgress(int percent) { progressBar->setValue(percent); }

    void stopProgress() {

        progressBar->setVisible(false);
        stopBtn->setVisible(false);
    }

    QList<QWidget*> wheelTargets() const { return { sigmaSlider, rangeSlider }; }
    QWidget* panelViewport() const { return panelScroll->viewport(); }

    void loadFrom(QSettings& settings) {

        int method = settings.value("denoiseMethod", 0).toInt();
        setMethod(method == 1 ? DenoiseMethod::Median
                : method == 2 ? DenoiseMethod::Bilateral : DenoiseMethod::Gaussian);

        sigmaSlider->setValue(std::clamp(settings.value("denoiseSigma", 10).toInt(), 3, 30));
        rangeSlider->setValue(std::clamp(settings.value("denoiseRange", 40).toInt(), 1, 1000));

        if (settings.value("denoiseMedian", 3).toInt() == 5) size5Btn->setChecked(true);
        else size3Btn->setChecked(true);

        regionOnlyBtn->setChecked(settings.value("denoiseRegionOnly", true).toBool());
        lockBtn->setChecked(settings.value("denoiseLock", false).toBool());

        updateFilterControls();
    }

    void saveTo(QSettings& settings) const {

        DenoiseParams current = params();

        settings.setValue("denoiseMethod", static_cast<int>(current.method));
        settings.setValue("denoiseSigma", sigmaSlider->value());
        settings.setValue("denoiseRange", rangeSlider->value());
        settings.setValue("denoiseMedian", current.medianSize);
        settings.setValue("denoiseRegionOnly", regionOnlyBtn->isChecked());
        settings.setValue("denoiseLock", lockBtn->isChecked());
    }

    std::function<void()> onViewChanged;
    std::function<void()> onApply;
    std::function<void()> onCapture;
    std::function<void()> onUseOriginal;
    std::function<void()> onAutoRange;
    std::function<void()> onCancel;

    NoiseCompareWidget* plot;
    QPushButton* applyBtn;
    QPushButton* captureBtn;
    QPushButton* originalBtn;

private:
    QPushButton* makeToggle(const QString& text) {

        auto* button = new QPushButton(text);
        button->setObjectName("pairButton");
        button->setCheckable(true);
        return button;
    }

    QLabel* makeCard(QHBoxLayout* row, const QString& title, const char* valueName) {

        auto* card = new QFrame();
        card->setObjectName("card");
        card->setMinimumWidth(110);

        auto* cardLayout = new QVBoxLayout(card);
        cardLayout->setContentsMargins(12, 8, 12, 8);
        cardLayout->setSpacing(2);

        auto* titleLabel = new QLabel(title);
        titleLabel->setObjectName("cardTitle");

        auto* value = new QLabel("—");
        value->setObjectName(valueName);

        cardLayout->addWidget(titleLabel);
        cardLayout->addWidget(value);
        row->addWidget(card, 1);

        return value;
    }

    void notifyView() { if (onViewChanged) onViewChanged(); }

    void updateFilterControls() {

        DenoiseParams current = params();

        sigmaBox->setVisible(current.method != DenoiseMethod::Median);
        sizeBox->setVisible(current.method == DenoiseMethod::Median);
        rangeBox->setVisible(current.method == DenoiseMethod::Bilateral);

        sigmaLabel->setText(QString("Spatial σ   %1 px").arg(current.sigma, 0, 'f', 1));
        rangeLabel->setText(QString("Range σ   %1 ADU").arg(current.rangeSigma, 0, 'f', 0));

        if (current.method == DenoiseMethod::Median)
            methodHint->setText("takes the middle value of the window: removes hot pixels and impulse noise, keeps hard edges");
        else if (current.method == DenoiseMethod::Bilateral)
            methodHint->setText("averages only neighbours with similar values: smooths flat areas, keeps edges. Range ≈ 2-3 x noise σ");
        else
            methodHint->setText("weighted average with a bell-shaped kernel: strongest smoothing, but edges get blurred too");

        applyBtn->setText(QString("Apply %1").arg(denoise_method_name(current.method)));
    }

    QPushButton* channelButtons[5];
    QPushButton* noiseModeBtn;
    QPushButton* valueModeBtn;
    QPushButton* logBtn;
    QPushButton* gaussianBtn;
    QPushButton* medianBtn;
    QPushButton* bilateralBtn;
    QPushButton* size3Btn;
    QPushButton* size5Btn;
    QPushButton* autoRangeBtn;
    QPushButton* regionOnlyBtn;
    QPushButton* lockBtn;
    QPushButton* stopBtn;
    QLabel* beforeValue;
    QLabel* afterValue;
    QLabel* changeValue;
    QLabel* snrValue;
    QLabel* methodHint;
    QLabel* sigmaLabel;
    QLabel* rangeLabel;
    QLabel* referenceText;
    QWidget* sigmaBox;
    QWidget* sizeBox;
    QWidget* rangeBox;
    QSlider* sigmaSlider;
    QSlider* rangeSlider;
    QProgressBar* progressBar;
    QScrollArea* panelScroll;
    QTableWidget* table;
    QButtonGroup channelGroup;
    QButtonGroup modeGroup;
    QButtonGroup methodGroup;
    QButtonGroup sizeGroup;
};


class ImageLabel : public QLabel {
public:
    enum Handle { HandleNone, HandleTopLeft, HandleTop, HandleTopRight, HandleRight,
                  HandleBottomRight, HandleBottom, HandleBottomLeft, HandleLeft };

    explicit ImageLabel(QWidget *parent = nullptr) : QLabel(parent) {

        setAlignment(Qt::AlignCenter);
        setMouseTracking(true);
        setAcceptDrops(true);
        setFocusPolicy(Qt::StrongFocus);

        antsTimer = new QTimer(this);
        antsTimer->setInterval(90);
        connect(antsTimer, &QTimer::timeout, this, [this]() {
            antsOffset = (antsOffset + 1) % 16;
            update();
        });
    }

    std::function<void(QRect)> onSelectionChanged;
    std::function<void(double)> onZoomChanged;
    std::function<void(QPoint, bool)> onCursorMoved;
    std::function<void(QString)> onFileDropped;
    std::function<void(int, int)> onNudge;
    std::function<QString(QPoint)> loupeText;

    static QColor markerColor(int index) {

        static const QColor colors[8] = { QColor("#ff2fb9"), QColor("#38ff9b"), QColor("#b9ff4d"),
                                          QColor("#a97bff"), QColor("#ffb02f"), QColor("#ff6b6b"),
                                          QColor("#4dd2ff"), QColor("#ff9ce0") };
        return colors[index % 8];
    }

    void setMarkedRegions(const QVector<QRect>& regions) {
        markedRegions = regions;
        update();
    }

    void setBaseImage(const QImage& image) {

        bool sameGeometry = !baseImage.isNull() &&
                            image.width() == baseImage.width() &&
                            image.height() == baseImage.height();

        baseImage = image;
        thumbnail = QImage();

        if (!sameGeometry) {
            selectionImage = QRect();
            dragRect = QRect();
            viewCenter = QPointF(baseImage.width() / 2.0, baseImage.height() / 2.0);
        }

        updateDisplay();
    }

    void setCompareImage(const QImage& image) {
        compareImage = image;
        updateDisplay();
    }

    QRect selectionInImageCoords() const { return selectionImage; }

    void setSelectionInImageCoords(const QRect& region) {

        selectionImage = clampToImage(region);
        dragRect = QRect();

        if (selectionImage.isEmpty()) antsTimer->stop();
        else if (!antsTimer->isActive()) antsTimer->start();

        update();
    }

    void focusOnRegion(const QRect& region) {

        QRect r = clampToImage(region);
        if (r.isEmpty()) return;

        viewCenter = QPointF(r.x() + r.width() / 2.0, r.y() + r.height() / 2.0);
        updateDisplay();
    }

    void setShowGrid(bool show) { showGrid = show; updateDisplay(); }
    void setZoom(double z) { zoomFactor = std::clamp(z, kMinZoom, kMaxZoom); updateDisplay(); }

    void setCompareEnabled(bool on) { compareEnabled = on; updateDisplay(); }
    void setLoupeEnabled(bool on) { loupeEnabled = on; update(); }
    void setThirdsEnabled(bool on) { thirdsEnabled = on; update(); }
    void setRulersEnabled(bool on) { rulersEnabled = on; update(); }
    void setMinimapEnabled(bool on) { minimapEnabled = on; update(); }

    bool compareIsEnabled() const { return compareEnabled; }

    double zoom() const { return zoomFactor; }
    int imageWidth() const { return baseImage.width(); }
    int imageHeight() const { return baseImage.height(); }

    double oneToOneZoom() const {
        if (fitScale <= 0.0) return kMinZoom;
        return std::clamp(1.0 / fitScale, kMinZoom, kMaxZoom);
    }

protected:
    void paintEvent(QPaintEvent*) override {

        QPainter painter(this);

        if (!canvasPixmap.isNull()) painter.drawPixmap(0, 0, canvasPixmap);
        else painter.fillRect(rect(), QColor("#05070c"));

        if (baseImage.isNull()) return;

        drawThirds(painter);
        drawRulers(painter);
        drawMarkers(painter);
        drawSelection(painter);
        drawSplitHandle(painter);
        drawMinimap(painter);
        drawLoupe(painter);
    }

    void resizeEvent(QResizeEvent* event) override { QLabel::resizeEvent(event); updateDisplay(); }

    void leaveEvent(QEvent* event) override {

        QLabel::leaveEvent(event);
        hasCursor = false;
        update();

        if (onCursorMoved) onCursorMoved(QPoint(), false);
    }

    void keyPressEvent(QKeyEvent* event) override {

        int step = (event->modifiers() & Qt::ShiftModifier) ? 10 : 1;
        int dx = 0;
        int dy = 0;

        switch (event->key()) {
            case Qt::Key_Left:  dx = -step; break;
            case Qt::Key_Right: dx =  step; break;
            case Qt::Key_Up:    dy = -step; break;
            case Qt::Key_Down:  dy =  step; break;
            default: QLabel::keyPressEvent(event); return;
        }

        if (onNudge) onNudge(dx, dy);
        event->accept();
    }

    void dragEnterEvent(QDragEnterEvent* event) override {

        if (!droppedPath(event->mimeData()).isEmpty()) {
            dropActive = true;
            updateDisplay();
            event->acceptProposedAction();
        }
    }

    void dragMoveEvent(QDragMoveEvent* event) override {

        if (!droppedPath(event->mimeData()).isEmpty()) event->acceptProposedAction();
    }

    void dragLeaveEvent(QDragLeaveEvent* event) override {
        QLabel::dragLeaveEvent(event);
        dropActive = false;
        updateDisplay();
    }

    void dropEvent(QDropEvent* event) override {

        QString path = droppedPath(event->mimeData());

        dropActive = false;
        updateDisplay();

        if (path.isEmpty()) return;

        event->acceptProposedAction();
        if (onFileDropped) onFileDropped(path);
    }

    void wheelEvent(QWheelEvent* event) override {

        if (baseImage.isNull() || displayedRect.isEmpty()) {
            QLabel::wheelEvent(event);
            return;
        }

        int delta = event->angleDelta().y();
        if (delta == 0) return;

        double target = std::clamp(zoomFactor * std::pow(1.0015, delta), kMinZoom, kMaxZoom);
        if (std::abs(target - zoomFactor) < 1e-9) { event->accept(); return; }

        QPoint cursor = event->position().toPoint();
        QPointF anchor = imagePointAt(cursor);

        zoomFactor = target;
        updateDisplay();

        QPointF landed = imagePointAt(cursor);
        viewCenter += anchor - landed;
        clampViewCenter();
        updateDisplay();

        if (onZoomChanged) onZoomChanged(zoomFactor);
        event->accept();
    }

    void mousePressEvent(QMouseEvent* event) override {

        if (baseImage.isNull()) return;

        if (event->button() == Qt::MiddleButton) {
            panning = true;
            panStart = event->pos();
            setCursor(Qt::ClosedHandCursor);
            return;
        }

        if (event->button() != Qt::LeftButton) return;

        if (compareEnabled && !compareImage.isNull() && overSplit(event->pos())) {
            draggingSplit = true;
            return;
        }

        Handle handle = handleAt(event->pos());

        if (handle != HandleNone) {
            activeHandle = handle;
            resizeRect = imageToWidget(selectionImage);
            return;
        }

        dragging = true;
        dragStart = event->pos();
        dragRect = QRect();
        selectionImage = QRect();
        antsTimer->stop();
        update();
    }

    void mouseMoveEvent(QMouseEvent* event) override {

        cursorPos = event->pos();
        hasCursor = true;

        if (draggingSplit) {

            if (!displayedRect.isEmpty()) {
                double ratio = double(event->pos().x() - displayedRect.left()) / displayedRect.width();
                splitRatio = std::clamp(ratio, 0.02, 0.98);
                updateDisplay();
            }
            return;
        }

        if (panning) {

            QPoint delta = event->pos() - panStart;
            panStart = event->pos();

            double sx = (double)sourceRect.width() / std::max(1, displayedRect.width());
            double sy = (double)sourceRect.height() / std::max(1, displayedRect.height());

            viewCenter -= QPointF(delta.x() * sx, delta.y() * sy);
            clampViewCenter();
            updateDisplay();
            return;
        }

        if (activeHandle != HandleNone) {
            applyHandleDrag(event->pos());
            return;
        }

        if (onCursorMoved && !baseImage.isNull()) {

            QPointF p = imagePointAt(event->pos());
            QPoint pixel(static_cast<int>(p.x()), static_cast<int>(p.y()));

            bool inside = displayedRect.contains(event->pos()) &&
                          pixel.x() >= 0 && pixel.y() >= 0 &&
                          pixel.x() < baseImage.width() && pixel.y() < baseImage.height();

            onCursorMoved(pixel, inside);
        }

        if (dragging) {
            dragRect = QRect(dragStart, event->pos()).normalized();
            update();
            return;
        }

        updateCursorShape(event->pos());

        if (loupeEnabled) update();
    }

    void mouseReleaseEvent(QMouseEvent* event) override {

        if (draggingSplit && event->button() == Qt::LeftButton) {
            draggingSplit = false;
            return;
        }

        if (panning && event->button() == Qt::MiddleButton) {
            panning = false;
            updateCursorShape(event->pos());
            return;
        }

        if (activeHandle != HandleNone && event->button() == Qt::LeftButton) {

            activeHandle = HandleNone;
            setSelectionInImageCoords(selectionImage);

            if (onSelectionChanged) onSelectionChanged(selectionImage);
            return;
        }

        if (!dragging || event->button() != Qt::LeftButton) return;

        dragging = false;
        dragRect = QRect(dragStart, event->pos()).normalized();
        selectionImage = widgetToImage(dragRect);
        dragRect = QRect();

        setSelectionInImageCoords(selectionImage);

        if (onSelectionChanged) onSelectionChanged(selectionImage);
    }

private:
    QRect clampToImage(const QRect& r) const {

        if (baseImage.isNull() || r.isEmpty()) return QRect();
        return r.intersected(QRect(0, 0, baseImage.width(), baseImage.height()));
    }

    void clampViewCenter() {

        if (baseImage.isNull()) return;

        viewCenter.setX(std::clamp(viewCenter.x(), 0.0, (double)baseImage.width()));
        viewCenter.setY(std::clamp(viewCenter.y(), 0.0, (double)baseImage.height()));
    }

    QPointF imagePointAt(const QPoint& p) const {

        if (displayedRect.isEmpty() || sourceRect.isEmpty()) return QPointF();

        double sx = (double)sourceRect.width() / displayedRect.width();
        double sy = (double)sourceRect.height() / displayedRect.height();

        return QPointF(sourceRect.left() + (p.x() - displayedRect.left()) * sx,
                       sourceRect.top() + (p.y() - displayedRect.top()) * sy);
    }

    QRect widgetToImage(const QRect& r) const {

        if (baseImage.isNull() || displayedRect.isEmpty() || sourceRect.isEmpty())
            return QRect();

        QRect sel = r.intersected(displayedRect);
        if (sel.isEmpty()) return QRect();

        double sx = (double)sourceRect.width() / displayedRect.width();
        double sy = (double)sourceRect.height() / displayedRect.height();

        int x0 = sourceRect.left() + static_cast<int>((sel.left() - displayedRect.left()) * sx);
        int y0 = sourceRect.top() + static_cast<int>((sel.top() - displayedRect.top()) * sy);

        int w = std::max(1, static_cast<int>(sel.width() * sx));
        int h = std::max(1, static_cast<int>(sel.height() * sy));

        return clampToImage(QRect(x0, y0, w, h));
    }

    QRect imageToWidget(const QRect& r) const {

        if (r.isEmpty() || displayedRect.isEmpty() || sourceRect.isEmpty())
            return QRect();

        double sx = (double)displayedRect.width() / sourceRect.width();
        double sy = (double)displayedRect.height() / sourceRect.height();

        int x = displayedRect.left() + static_cast<int>(std::lround((r.left() - sourceRect.left()) * sx));
        int y = displayedRect.top() + static_cast<int>(std::lround((r.top() - sourceRect.top()) * sy));

        int w = std::max(1, static_cast<int>(std::lround(r.width() * sx)));
        int h = std::max(1, static_cast<int>(std::lround(r.height() * sy)));

        return QRect(x, y, w, h).intersected(displayedRect);
    }

    static QString droppedPath(const QMimeData* mime) {

        if (!mime || !mime->hasUrls()) return QString();

        for (const QUrl& url : mime->urls())
            if (url.isLocalFile()) return url.toLocalFile();

        return QString();
    }

    int splitPosition() const {

        if (displayedRect.isEmpty()) return 0;
        return displayedRect.left() + static_cast<int>(displayedRect.width() * splitRatio);
    }

    bool overSplit(const QPoint& p) const {

        if (!compareEnabled || compareImage.isNull() || displayedRect.isEmpty()) return false;
        return std::abs(p.x() - splitPosition()) <= 6 && displayedRect.contains(QPoint(p.x(), p.y()));
    }

    QVector<QPoint> handlePoints() const {

        QRect r = activeHandle == HandleNone ? imageToWidget(selectionImage) : resizeRect;
        QVector<QPoint> points;

        if (r.isEmpty()) return points;

        points << r.topLeft() << QPoint(r.center().x(), r.top()) << r.topRight()
               << QPoint(r.right(), r.center().y()) << r.bottomRight()
               << QPoint(r.center().x(), r.bottom()) << r.bottomLeft()
               << QPoint(r.left(), r.center().y());

        return points;
    }

    Handle handleAt(const QPoint& p) const {

        QVector<QPoint> points = handlePoints();

        for (int i = 0; i < points.size(); ++i) {

            QRect box(points[i].x() - 5, points[i].y() - 5, 11, 11);
            if (box.contains(p)) return static_cast<Handle>(HandleTopLeft + i);
        }

        return HandleNone;
    }

    void applyHandleDrag(const QPoint& p) {

        QRect r = resizeRect;

        switch (activeHandle) {
            case HandleTopLeft:     r.setTopLeft(p); break;
            case HandleTop:         r.setTop(p.y()); break;
            case HandleTopRight:    r.setTopRight(p); break;
            case HandleRight:       r.setRight(p.x()); break;
            case HandleBottomRight: r.setBottomRight(p); break;
            case HandleBottom:      r.setBottom(p.y()); break;
            case HandleBottomLeft:  r.setBottomLeft(p); break;
            case HandleLeft:        r.setLeft(p.x()); break;
            default: return;
        }

        resizeRect = r.normalized();
        selectionImage = widgetToImage(resizeRect);
        update();
    }

    void updateCursorShape(const QPoint& p) {

        if (baseImage.isNull()) { unsetCursor(); return; }

        if (overSplit(p)) { setCursor(Qt::SplitHCursor); return; }

        switch (handleAt(p)) {
            case HandleTopLeft:
            case HandleBottomRight: setCursor(Qt::SizeFDiagCursor); return;
            case HandleTopRight:
            case HandleBottomLeft:  setCursor(Qt::SizeBDiagCursor); return;
            case HandleTop:
            case HandleBottom:      setCursor(Qt::SizeVerCursor); return;
            case HandleLeft:
            case HandleRight:       setCursor(Qt::SizeHorCursor); return;
            default: break;
        }

        setCursor(displayedRect.contains(p) ? Qt::CrossCursor : Qt::ArrowCursor);
    }

    void drawThirds(QPainter& painter) {

        if (!thirdsEnabled || displayedRect.isEmpty()) return;

        painter.setPen(QPen(QColor(255, 255, 255, 40), 1, Qt::DashLine));

        for (int i = 1; i < 3; ++i) {

            int x = displayedRect.left() + displayedRect.width() * i / 3;
            int y = displayedRect.top() + displayedRect.height() * i / 3;

            painter.drawLine(x, displayedRect.top(), x, displayedRect.bottom());
            painter.drawLine(displayedRect.left(), y, displayedRect.right(), y);
        }
    }

    void drawRulers(QPainter& painter) {

        if (!rulersEnabled || displayedRect.isEmpty() || sourceRect.isEmpty()) return;

        const int band = 18;

        painter.fillRect(QRect(0, 0, width(), band), QColor(4, 6, 12, 190));
        painter.fillRect(QRect(0, 0, band, height()), QColor(4, 6, 12, 190));

        QFont mono("Consolas");
        mono.setPixelSize(9);
        painter.setFont(mono);

        double scaleX = (double)displayedRect.width() / sourceRect.width();
        double step = 1.0;

        while (step * scaleX < 70.0) step *= (std::abs(step * 2 * scaleX - 70.0) < std::abs(step * 5 * scaleX - 70.0)) ? 2 : 5;

        painter.setPen(QColor(0, 229, 255, 130));

        for (double v = std::ceil(sourceRect.left() / step) * step; v <= sourceRect.right(); v += step) {

            int x = displayedRect.left() + static_cast<int>((v - sourceRect.left()) * scaleX);
            if (x < band || x > width()) continue;

            painter.drawLine(x, band - 5, x, band - 1);
            painter.drawText(x + 3, band - 6, QString::number(static_cast<int>(v)));
        }

        for (double v = std::ceil(sourceRect.top() / step) * step; v <= sourceRect.bottom(); v += step) {

            int y = displayedRect.top() + static_cast<int>((v - sourceRect.top()) * scaleX);
            if (y < band || y > height()) continue;

            painter.drawLine(band - 5, y, band - 1, y);
            painter.save();
            painter.translate(band - 6, y - 3);
            painter.rotate(-90);
            painter.drawText(0, 0, QString::number(static_cast<int>(v)));
            painter.restore();
        }
    }

    void drawMarkers(QPainter& painter) {

        for (int i = 0; i < markedRegions.size(); ++i) {

            QRect marker = imageToWidget(markedRegions[i]);
            if (marker.isEmpty()) continue;

            QColor color = markerColor(i);

            painter.setBrush(Qt::NoBrush);
            painter.setPen(QPen(QColor(color.red(), color.green(), color.blue(), 40), 4));
            painter.drawRect(marker);

            painter.setPen(QPen(color, 1));
            painter.setBrush(QColor(color.red(), color.green(), color.blue(), 24));
            painter.drawRect(marker);

            painter.setPen(color);
            painter.drawText(marker.adjusted(4, 2, 0, 0), Qt::AlignLeft | Qt::AlignTop,
                             QString::number(i + 1));
        }
    }

    void drawSelection(QPainter& painter) {

        QRect overlay = dragging ? dragRect.intersected(displayedRect)
                                 : (activeHandle == HandleNone ? imageToWidget(selectionImage)
                                                               : resizeRect.intersected(displayedRect));
        if (overlay.isEmpty()) return;

        painter.setRenderHint(QPainter::Antialiasing, true);
        painter.setBrush(Qt::NoBrush);

        for (int i = 4; i >= 1; --i) {
            painter.setPen(QPen(QColor(0, 229, 255, 14 + (4 - i) * 11), i * 2.0));
            painter.drawRect(overlay);
        }

        painter.setBrush(QColor(0, 229, 255, 26));
        painter.setPen(QPen(QColor("#8ff8ff"), 1.4));
        painter.drawRect(overlay);

        QPen ants(QColor("#04070c"), 1.4);
        ants.setDashPattern({ 4, 4 });
        ants.setDashOffset(antsOffset);

        painter.setBrush(Qt::NoBrush);
        painter.setPen(ants);
        painter.drawRect(overlay);

        if (!dragging) {

            painter.setPen(QPen(QColor("#04070c"), 1));
            painter.setBrush(QColor("#8ff8ff"));

            for (const QPoint& point : handlePoints())
                painter.drawRect(QRect(point.x() - 3, point.y() - 3, 7, 7));
        }

        painter.setRenderHint(QPainter::Antialiasing, false);
    }

    void drawSplitHandle(QPainter& painter) {

        if (!compareEnabled || compareImage.isNull() || displayedRect.isEmpty()) return;

        int x = splitPosition();

        painter.setPen(QPen(QColor(0, 229, 255, 70), 3));
        painter.drawLine(x, displayedRect.top(), x, displayedRect.bottom());

        painter.setPen(QPen(QColor("#8ff8ff"), 1));
        painter.drawLine(x, displayedRect.top(), x, displayedRect.bottom());

        painter.setRenderHint(QPainter::Antialiasing, true);
        painter.setBrush(QColor(4, 6, 12, 220));
        painter.setPen(QPen(QColor("#00e5ff"), 1));
        painter.drawEllipse(QPoint(x, displayedRect.center().y()), 11, 11);

        painter.setPen(QPen(QColor("#8ff8ff"), 1.4));
        painter.drawLine(x - 5, displayedRect.center().y(), x - 2, displayedRect.center().y());
        painter.drawLine(x + 2, displayedRect.center().y(), x + 5, displayedRect.center().y());
        painter.setRenderHint(QPainter::Antialiasing, false);

        QFont mono("Consolas");
        mono.setPixelSize(10);
        painter.setFont(mono);
        painter.setPen(QColor("#5d6f8c"));
        painter.drawText(displayedRect.left() + 8, displayedRect.top() + 16, "RAW");
        painter.drawText(displayedRect.right() - 62, displayedRect.top() + 16, "PROCESSED");
    }

    void drawMinimap(QPainter& painter) {

        if (!minimapEnabled || baseImage.isNull() || sourceRect.isEmpty()) return;
        if (sourceRect.width() >= baseImage.width() && sourceRect.height() >= baseImage.height()) return;

        if (thumbnail.isNull())
            thumbnail = baseImage.scaled(150, 150, Qt::KeepAspectRatio, Qt::FastTransformation);

        QRect box(width() - thumbnail.width() - 18, 18, thumbnail.width(), thumbnail.height());

        painter.setRenderHint(QPainter::Antialiasing, true);
        painter.setBrush(QColor(4, 6, 12, 220));
        painter.setPen(QPen(QColor(0, 229, 255, 90), 1));
        painter.drawRoundedRect(box.adjusted(-5, -5, 5, 5), 6, 6);
        painter.setRenderHint(QPainter::Antialiasing, false);

        painter.drawImage(box, thumbnail);

        double kx = (double)box.width() / baseImage.width();
        double ky = (double)box.height() / baseImage.height();

        QRect view(box.left() + static_cast<int>(sourceRect.left() * kx),
                   box.top() + static_cast<int>(sourceRect.top() * ky),
                   std::max(3, static_cast<int>(sourceRect.width() * kx)),
                   std::max(3, static_cast<int>(sourceRect.height() * ky)));

        painter.setBrush(QColor(0, 229, 255, 30));
        painter.setPen(QPen(QColor("#00e5ff"), 1));
        painter.drawRect(view);
    }

    void drawLoupe(QPainter& painter) {

        if (!loupeEnabled || !hasCursor || baseImage.isNull() || displayedRect.isEmpty()) return;
        if (!displayedRect.contains(cursorPos)) return;

        const int viewSize = 132;
        const int factor = 8;
        const int span = viewSize / factor;

        QPointF center = imagePointAt(cursorPos);

        QRect src(static_cast<int>(center.x()) - span / 2,
                  static_cast<int>(center.y()) - span / 2, span, span);

        QRect box(cursorPos.x() + 22, cursorPos.y() + 22, viewSize + 2, viewSize + 22);

        if (box.right() > width() - 8) box.moveRight(cursorPos.x() - 22);
        if (box.bottom() > height() - 8) box.moveBottom(cursorPos.y() - 22);

        painter.setRenderHint(QPainter::Antialiasing, true);
        painter.setBrush(QColor(4, 6, 12, 235));
        painter.setPen(QPen(QColor(0, 229, 255, 120), 1));
        painter.drawRoundedRect(box, 6, 6);
        painter.setRenderHint(QPainter::Antialiasing, false);

        QRect view(box.left() + 1, box.top() + 1, viewSize, viewSize);

        painter.setRenderHint(QPainter::SmoothPixmapTransform, false);
        painter.drawImage(view, baseImage, src);

        painter.setPen(QPen(QColor(0, 229, 255, 45), 1));

        for (int i = 1; i < span; ++i) {
            int offset = view.left() + i * factor;
            painter.drawLine(offset, view.top(), offset, view.bottom());
            painter.drawLine(view.left(), view.top() + i * factor, view.right(), view.top() + i * factor);
        }

        painter.setPen(QPen(QColor("#ff2fb9"), 1));
        painter.drawRect(QRect(view.left() + (span / 2) * factor, view.top() + (span / 2) * factor,
                               factor, factor));

        QFont mono("Consolas");
        mono.setPixelSize(10);
        painter.setFont(mono);
        painter.setPen(QColor("#8ff8ff"));

        QString text = loupeText ? loupeText(QPoint(static_cast<int>(center.x()),
                                                    static_cast<int>(center.y())))
                                 : QString("%1, %2").arg(static_cast<int>(center.x()))
                                                    .arg(static_cast<int>(center.y()));

        painter.drawText(QRect(box.left() + 6, view.bottom() + 2, box.width() - 12, 18),
                         Qt::AlignLeft | Qt::AlignVCenter, text);
    }

    void drawEmptyState() {

        QPixmap canvas(size());
        canvas.fill(QColor("#05070c"));

        QPainter painter(&canvas);
        painter.setRenderHint(QPainter::Antialiasing, true);

        int boxW = std::min(360, width() - 40);
        int boxH = std::min(200, height() - 40);

        if (boxW < 120 || boxH < 90) {
            painter.end();
            canvasPixmap = canvas;
            update();
            return;
        }

        QRect box((width() - boxW) / 2, (height() - boxH) / 2, boxW, boxH);

        QPen border(dropActive ? QColor("#00e5ff") : QColor("#1b2b47"), 2);
        border.setStyle(Qt::DashLine);
        border.setDashPattern({ 5, 4 });

        painter.setPen(border);
        painter.setBrush(dropActive ? QColor(0, 229, 255, 24) : QColor(0, 0, 0, 0));
        painter.drawRoundedRect(box, 8, 8);

        QPointF center(box.center().x(), box.center().y() - 26);

        QPen glyph(dropActive ? QColor("#00e5ff") : QColor("#26456e"), 2);
        glyph.setCapStyle(Qt::RoundCap);
        glyph.setJoinStyle(Qt::RoundJoin);
        painter.setPen(glyph);
        painter.setBrush(Qt::NoBrush);

        painter.drawLine(QPointF(center.x(), center.y() - 14), QPointF(center.x(), center.y() + 8));
        painter.drawPolyline(QPolygonF({ QPointF(center.x() - 8, center.y() + 1),
                                         QPointF(center.x(), center.y() + 9),
                                         QPointF(center.x() + 8, center.y() + 1) }));
        painter.drawPolyline(QPolygonF({ QPointF(center.x() - 13, center.y() + 12),
                                         QPointF(center.x() - 13, center.y() + 18),
                                         QPointF(center.x() + 13, center.y() + 18),
                                         QPointF(center.x() + 13, center.y() + 12) }));

        QFont titleFont = font();
        titleFont.setPixelSize(18);
        titleFont.setWeight(QFont::DemiBold);

        painter.setFont(titleFont);
        painter.setPen(dropActive ? QColor("#eaffff") : QColor("#5d6f8c"));
        painter.drawText(QRect(box.left(), box.center().y() + 18, box.width(), 26),
                         Qt::AlignCenter, "Drop RAW here");

        QFont hintFont = font();
        hintFont.setPixelSize(12);

        painter.setFont(hintFont);
        painter.setPen(QColor("#4d5c75"));
        painter.drawText(QRect(box.left(), box.center().y() + 46, box.width(), 20),
                         Qt::AlignCenter, "or press Ctrl+O to open a file");

        drawFrameGlow(painter);
        painter.end();

        canvasPixmap = canvas;
        update();
    }

    void updateDisplay() {

        if (width() <= 0 || height() <= 0) return;

        if (baseImage.isNull()) { drawEmptyState(); return; }

        fitScale = std::min((double)width() / baseImage.width(),
                            (double)height() / baseImage.height());

        double scale = fitScale * zoomFactor;
        if (scale <= 0.0) return;

        int srcW = std::clamp(static_cast<int>(std::lround(width() / scale)), 1, baseImage.width());
        int srcH = std::clamp(static_cast<int>(std::lround(height() / scale)), 1, baseImage.height());

        int srcX = std::clamp(static_cast<int>(std::lround(viewCenter.x() - srcW / 2.0)),
                              0, baseImage.width() - srcW);
        int srcY = std::clamp(static_cast<int>(std::lround(viewCenter.y() - srcH / 2.0)),
                              0, baseImage.height() - srcH);

        sourceRect = QRect(srcX, srcY, srcW, srcH);

        int dispW = std::clamp(static_cast<int>(std::lround(srcW * scale)), 1, width());
        int dispH = std::clamp(static_cast<int>(std::lround(srcH * scale)), 1, height());

        displayedRect = QRect((width() - dispW) / 2, (height() - dispH) / 2, dispW, dispH);

        QPixmap canvas(size());
        canvas.fill(QColor("#05070c"));

        QPainter painter(&canvas);
        painter.setRenderHint(QPainter::SmoothPixmapTransform, scale < 1.0);

        if (compareEnabled && !compareImage.isNull()) {

            int split = splitPosition();

            painter.setClipRect(QRect(displayedRect.left(), displayedRect.top(),
                                      split - displayedRect.left(), displayedRect.height()));
            painter.drawImage(displayedRect, baseImage, sourceRect);

            double kx = (double)compareImage.width() / baseImage.width();
            double ky = (double)compareImage.height() / baseImage.height();

            QRect other(static_cast<int>(sourceRect.left() * kx),
                        static_cast<int>(sourceRect.top() * ky),
                        std::max(1, static_cast<int>(sourceRect.width() * kx)),
                        std::max(1, static_cast<int>(sourceRect.height() * ky)));

            painter.setClipRect(QRect(split, displayedRect.top(),
                                      displayedRect.right() - split + 1, displayedRect.height()));
            painter.drawImage(displayedRect, compareImage, other);
            painter.setClipping(false);

        } else {

            painter.drawImage(displayedRect, baseImage, sourceRect);
        }

        if (showGrid) {

            double pixelSizeOnScreen = static_cast<double>(displayedRect.width()) / sourceRect.width();

            if (pixelSizeOnScreen >= 4.0) {

                painter.setPen(QPen(QColor(0, 229, 255, 42), 1));

                for (int px = 0; px <= sourceRect.width(); ++px) {

                    int sx = displayedRect.left() + static_cast<int>(std::lround(px * pixelSizeOnScreen));
                    if (sx > displayedRect.right()) break;
                    painter.drawLine(sx, displayedRect.top(), sx, displayedRect.bottom());
                }

                for (int py = 0; py <= sourceRect.height(); ++py) {

                    int sy = displayedRect.top() + static_cast<int>(std::lround(py * pixelSizeOnScreen));
                    if (sy > displayedRect.bottom()) break;
                    painter.drawLine(displayedRect.left(), sy, displayedRect.right(), sy);
                }
            }
        }

        drawFrameGlow(painter);
        painter.end();

        canvasPixmap = canvas;
        update();
    }

    void drawFrameGlow(QPainter& painter) {

        painter.setBrush(Qt::NoBrush);

        for (int i = 1; i <= 9; ++i) {
            painter.setPen(QPen(QColor(0, 229, 255, 18 - i * 2), 1));
            painter.drawRect(rect().adjusted(i, i, -i - 1, -i - 1));
        }

        painter.setPen(QPen(QColor(0, 229, 255, 60), 1));
        painter.drawRect(rect().adjusted(0, 0, -1, -1));
    }

    QImage baseImage;
    QImage compareImage;
    QImage thumbnail;
    QPixmap canvasPixmap;
    QRect displayedRect;
    QRect sourceRect;
    QRect selectionImage;
    QRect dragRect;
    QRect resizeRect;
    QVector<QRect> markedRegions;
    QPointF viewCenter;
    QPoint dragStart;
    QPoint panStart;
    QPoint cursorPos;
    QTimer* antsTimer;

    Handle activeHandle = HandleNone;

    bool dragging = false;
    bool panning = false;
    bool dropActive = false;
    bool hasCursor = false;
    bool draggingSplit = false;
    bool compareEnabled = false;
    bool loupeEnabled = false;
    bool thirdsEnabled = false;
    bool rulersEnabled = false;
    bool minimapEnabled = true;

    int antsOffset = 0;
    double splitRatio = 0.5;

    bool showGrid = false;
    double zoomFactor = 1.0;
    double fitScale = 1.0;
};


struct Document {

    std::optional<RawImage> raw;
    std::optional<RawImage> original;
    std::optional<RawImage> reference;
    std::optional<Image> linear;
    std::optional<Image> result;
    QRect selected;
    QVector<QRect> regions;
    int rotation = 0;
    double measuredBlack = -1.0;
    QString file;
    int gamma = 220;
    int zoom = 100;
};


class MainWindow;

class RegionsCommand : public QUndoCommand {
public:
    RegionsCommand(MainWindow* target, QVector<QRect> before, QVector<QRect> after,
                   const QString& text)
        : owner(target), previous(std::move(before)), next(std::move(after)) { setText(text); }

    void undo() override;
    void redo() override;

private:
    MainWindow* owner;
    QVector<QRect> previous;
    QVector<QRect> next;
};


class FrameCommand : public QUndoCommand {
public:
    FrameCommand(MainWindow* target, RawImage before, RawImage after, const QString& text)
        : owner(target), previous(std::move(before)), next(std::move(after)) { setText(text); }

    void undo() override;
    void redo() override;

private:
    MainWindow* owner;
    RawImage previous;
    RawImage next;
};


class MainWindow : public QMainWindow {
public:
    MainWindow() {
        setWindowTitle("Raw Image Process");
        resize(1150, 680);
        setStyleSheet(kDarkStyle);

        titleBar = new TitleBar(this);
        setMenuWidget(titleBar);

        auto* central = new BackgroundWidget(this);
        auto* centralLayout = new QVBoxLayout(central);
        centralLayout->setContentsMargins(0, 0, 0, 0);
        centralLayout->setSpacing(0);

        auto* content = new QWidget();
        auto* rootLayout = new QVBoxLayout(content);
        rootLayout->setContentsMargins(10, 6, 10, 10);
        rootLayout->setSpacing(6);
        centralLayout->addWidget(content, 1);

        fileTabs = new QTabBar();
        fileTabs->setObjectName("fileTabs");
        fileTabs->setExpanding(false);
        fileTabs->setTabsClosable(true);
        fileTabs->setDrawBase(false);
        fileTabs->setVisible(false);

        imagePageBtn = new QPushButton("IMAGE");
        noisePageBtn = new QPushButton("NOISE  ·  FILTERS");

        for (QPushButton* button : { imagePageBtn, noisePageBtn }) {
            button->setObjectName("pageButton");
            button->setCheckable(true);
            button->setFocusPolicy(Qt::NoFocus);
            button->setIconSize(QSize(14, 14));
        }

        imagePageBtn->setIcon(makeIcon("image"));
        noisePageBtn->setIcon(makeIcon("filter"));
        imagePageBtn->setToolTip("Photo  (Ctrl+1)");
        noisePageBtn->setToolTip("Noise histograms before / after and denoise filters  (Ctrl+2)");
        imagePageBtn->setChecked(true);

        auto* pageGroup = new QButtonGroup(this);
        pageGroup->addButton(imagePageBtn, 0);
        pageGroup->addButton(noisePageBtn, 1);
        pageGroup->setExclusive(true);

        auto* pageBar = new QHBoxLayout();
        pageBar->setContentsMargins(0, 0, 0, 0);
        pageBar->setSpacing(2);
        pageBar->addWidget(imagePageBtn);
        pageBar->addWidget(noisePageBtn);
        pageBar->addSpacing(18);
        pageBar->addWidget(fileTabs);
        pageBar->addStretch(1);
        rootLayout->addLayout(pageBar);

        preview = new ImageLabel();
        preview->setMinimumSize(320, 240);
        preview->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        auto* hudLayout = new QVBoxLayout(preview);
        hudLayout->setContentsMargins(16, 16, 16, 16);
        hudLayout->addStretch();

        hudLabel = new QLabel(preview);
        hudLabel->setObjectName("hud");
        hudLabel->setAttribute(Qt::WA_TransparentForMouseEvents);
        hudLabel->setTextFormat(Qt::RichText);
        hudLabel->hide();
        hudLayout->addWidget(hudLabel, 0, Qt::AlignLeft | Qt::AlignBottom);

        preview->onSelectionChanged = [this](QRect r) { onRegionSelected(displayToRaw(r)); };
        preview->onFileDropped = [this](QString path) { loadRawFile(path); };
        preview->onNudge = [this](int dx, int dy) { nudgeRegion(dx, dy); };
        noisePage = new NoisePage();

        pages = new QStackedWidget();
        pages->addWidget(preview);
        pages->addWidget(noisePage);
        rootLayout->addWidget(pages, 3);

        noisePage->onViewChanged = [this]() { redrawNoisePage(); };
        noisePage->onApply = [this]() { applyDenoise(); };
        noisePage->onAutoRange = [this]() { autoRangeSigma(); };
        noisePage->onUseOriginal = [this]() { useOriginalAsBefore(); };

        noisePage->onCapture = [this]() {
            captureNoiseBefore("manual snapshot");
            refreshNoisePage();
        };

        noisePage->onCancel = [this]() {
            cancelRequested = true;
            cancelBtn->setEnabled(false);
            logLine("cancel requested");
        };

        connect(pageGroup, &QButtonGroup::idClicked, this, [this](int id) { showPage(id); });

        auto* rightPanel = new QVBoxLayout();
        rightPanel->setSpacing(10);
        rightPanel->setContentsMargins(0, 0, 4, 0);

        // file group
        auto* fileGroup = new QGroupBox("FILE");
        auto* fileLayout = new QVBoxLayout(fileGroup);
        auto* loadbtn = new QPushButton("Open RAW");
        auto* loadRefBtn = new QPushButton("Open reference RAW");
        loadbtn->setIcon(makeIcon("open"));
        loadRefBtn->setIcon(makeIcon("layers"));
        fileLayout->addWidget(loadbtn);
        fileLayout->addWidget(loadRefBtn);
        rightPanel->addWidget(fileGroup);

        auto* metaGroup = new QGroupBox("FILE INFO");
        auto* metaLayout = new QVBoxLayout(metaGroup);
        metaText = new QLabel("—");
        metaText->setObjectName("statsValue");
        metaText->setWordWrap(true);
        metaLayout->addWidget(metaText);
        rightPanel->addWidget(metaGroup);

        // backend group
        auto* backendGroup = new QGroupBox("DEVICE");
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
        auto* gammaGroup = new QGroupBox("GAMMA");
        auto* gammaLayout = new QVBoxLayout(gammaGroup);
        gammaValueLabel = new QLabel("2.20");
        gammaSlider = new QSlider(Qt::Horizontal);
        gammaSlider->setRange(10, 300);
        gammaSlider->setValue(220);
        gammaLayout->addWidget(gammaValueLabel);
        gammaLayout->addWidget(gammaSlider);
        rightPanel->addWidget(gammaGroup);

        // timer
        gammaTimer = new QTimer(this);
        gammaTimer->setSingleShot(true);
        gammaTimer->setInterval(50);

        // view group
        auto* viewGroup = new QGroupBox("VIEW");
        auto* viewLayout = new QVBoxLayout(viewGroup);

        gridBtn = new QPushButton("View grid");
        gridBtn->setCheckable(true);

        clipBtn = new QPushButton("Show clipping");
        clipBtn->setCheckable(true);

        rotateBtn = new QPushButton("Rotate 90°");

        zoomValueLabel = new QLabel("1.00x");
        zoomSlider = new QSlider(Qt::Horizontal);
        zoomSlider->setRange(static_cast<int>(kMinZoom * 100), static_cast<int>(kMaxZoom * 100));
        zoomSlider->setValue(100);

        auto* zoomButtons = new QHBoxLayout();
        auto* fitBtn = new QPushButton("Fit (F)");
        auto* oneToOneBtn = new QPushButton("1:1");
        zoomButtons->addWidget(fitBtn);
        zoomButtons->addWidget(oneToOneBtn);

        auto* hintLabel = new QLabel("wheel = zoom to cursor, middle drag = pan");
        hintLabel->setWordWrap(true);
        hintLabel->setObjectName("hint");

        viewLayout->addWidget(gridBtn);
        viewLayout->addWidget(clipBtn);
        viewLayout->addWidget(rotateBtn);
        viewLayout->addWidget(zoomValueLabel);
        viewLayout->addWidget(zoomSlider);
        viewLayout->addLayout(zoomButtons);
        viewLayout->addWidget(hintLabel);
        rightPanel->addWidget(viewGroup);

        // process group
        auto* processGroup = new QGroupBox("PROCESSING");
        auto* processLayout = new QVBoxLayout(processGroup);
        binningBtn = new QPushButton("Binning 2x2");
        bilinearBtn = new QPushButton("Bilinear");
        malvarBtn = new QPushButton("Malvar");

        binningBtn->setCheckable(true);
        bilinearBtn->setCheckable(true);
        malvarBtn->setCheckable(true);
        binningBtn->setChecked(true);

        auto* methodGroup = new QButtonGroup(this);
        methodGroup->addButton(binningBtn);
        methodGroup->addButton(bilinearBtn);
        methodGroup->addButton(malvarBtn);
        methodGroup->setExclusive(true);

        auto* methodRow = new QHBoxLayout();
        methodRow->setSpacing(6);
        methodRow->addWidget(binningBtn);
        methodRow->addWidget(bilinearBtn);
        methodRow->addWidget(malvarBtn);

        auto* methodHint = new QLabel("binning halves the size, the others keep it");
        methodHint->setObjectName("hint");
        methodHint->setWordWrap(true);

        auto* pressedBtn = new QPushButton("Apply a demosaic to picture");

        progress = new QProgressBar();
        progress->setRange(0, 100);
        progress->setTextVisible(false);
        progress->setVisible(false);

        cancelBtn = new QPushButton("Cancel");
        cancelBtn->setVisible(false);

        processLayout->addLayout(methodRow);
        processLayout->addWidget(methodHint);
        processLayout->addWidget(pressedBtn);
        processLayout->addWidget(progress);
        processLayout->addWidget(cancelBtn);
        rightPanel->addWidget(processGroup);

        // export raw without processing
        auto* exportGroup = new QGroupBox("EXPORT RAW");
        auto* exportLayout = new QVBoxLayout(exportGroup);
        auto* saveBinBtn = new QPushButton("Save RAW to .Bin");
        auto* saveBinRegionBtn = new QPushButton("Save region to .Bin");
        auto* saveTiffBtn = new QPushButton("Save RAW to .TIFF");
        auto* saveTiffRegionBtn = new QPushButton("Save region to .TIFF");
        exportLayout->addWidget(saveBinBtn);
        exportLayout->addWidget(saveBinRegionBtn);
        exportLayout->addWidget(saveTiffBtn);
        exportLayout->addWidget(saveTiffRegionBtn);
        rightPanel->addWidget(exportGroup);

        // save processing result
        auto* saveResGroup = new QGroupBox("PROCESSING RESULT");
        auto* saveResLayout = new QVBoxLayout(saveResGroup);
        auto* saveBtn = new QPushButton("Save to .png");
        auto* saveTiff16Btn = new QPushButton("Save linear to 16-bit TIFF");
        saveResLayout->addWidget(saveBtn);
        saveResLayout->addWidget(saveTiff16Btn);
        rightPanel->addWidget(saveResGroup);

        // region group (mouse)
        auto* regionGroup = new QGroupBox("REGION BY MOUSE");
        auto* regionLayout = new QVBoxLayout(regionGroup);
        regionCoordsLabel = new QLabel("Region is not chosen");
        regionCoordsLabel->setObjectName("statsValue");
        regionCoordsLabel->setWordWrap(true);
        auto* processRegion = new QPushButton("Demosaic this region");
        regionLayout->addWidget(regionCoordsLabel);
        regionLayout->addWidget(processRegion);
        rightPanel->addWidget(regionGroup);

        // region group (coordinates)
        auto* numRegionGroup = new QGroupBox("REGION BY COORDINATES");
        auto* numRegionLayout = new QVBoxLayout(numRegionGroup);

        xSpin = new QSpinBox(); xSpin->setRange(0, 10000000); xSpin->setPrefix("X: ");
        ySpin = new QSpinBox(); ySpin->setRange(0, 10000000); ySpin->setPrefix("Y: ");
        wSpin = new QSpinBox(); wSpin->setRange(1, 10000000); wSpin->setValue(100); wSpin->setPrefix("W: ");
        hSpin = new QSpinBox(); hSpin->setRange(1, 10000000); hSpin->setValue(100); hSpin->setPrefix("H: ");

        auto* applyRegionBtn = new QPushButton("Apply region(coordinates)");

        auto* quickLayout = new QHBoxLayout();
        auto* wholeRowBtn = new QPushButton("Whole row");
        auto* wholeColBtn = new QPushButton("Whole column");
        quickLayout->addWidget(wholeRowBtn);
        quickLayout->addWidget(wholeColBtn);

        numRegionLayout->addWidget(xSpin);
        numRegionLayout->addWidget(ySpin);
        numRegionLayout->addWidget(wSpin);
        numRegionLayout->addWidget(hSpin);
        numRegionLayout->addWidget(applyRegionBtn);
        numRegionLayout->addLayout(quickLayout);
        rightPanel->addWidget(numRegionGroup);

        // stats for highlighted region
        auto* statsGroup = new QGroupBox("STATS OF REGION");
        auto* statsLayout = new QVBoxLayout(statsGroup);
        statsText = new QLabel("—");
        statsText->setObjectName("statsValue");
        statsText->setWordWrap(true);
        auto* exportStatsBtn = new QPushButton("Export stats to .csv");
        statsLayout->addWidget(statsText);
        statsLayout->addWidget(exportStatsBtn);
        rightPanel->addWidget(statsGroup);

        // analysis plots
        auto* analysisTabs = new QTabWidget();
        histogram = new HistogramWidget();
        profile = new ProfileWidget();
        analysisTabs->addTab(histogram, "Histogram");
        analysisTabs->addTab(profile, "Profile");
        analysisTabs->setDocumentMode(true);

        // two frame noise
        auto* noiseGroup = new QGroupBox("TWO-FRAME NOISE");
        auto* noiseLayout = new QVBoxLayout(noiseGroup);
        auto* noiseBtn = new QPushButton("Compare with reference");
        auto* darkBtn = new QPushButton("Subtract reference as dark");
        noiseText = new QLabel("—");
        noiseText->setObjectName("statsValue");
        noiseText->setWordWrap(true);
        auto* revertBtn = new QPushButton("Revert to original");

        noiseLayout->addWidget(noiseBtn);
        noiseLayout->addWidget(darkBtn);
        noiseLayout->addWidget(revertBtn);
        noiseLayout->addWidget(noiseText);
        rightPanel->addWidget(noiseGroup);

        auto* obGroup = new QGroupBox("OPTICAL BLACK");
        auto* obLayout = new QVBoxLayout(obGroup);
        auto* obMeasureBtn = new QPushButton("Measure black from optical area");
        auto* obApplyBtn = new QPushButton("Apply as black level");
        obText = new QLabel("—");
        obText->setObjectName("statsValue");
        obText->setWordWrap(true);
        obLayout->addWidget(obMeasureBtn);
        obLayout->addWidget(obApplyBtn);
        obLayout->addWidget(obText);
        rightPanel->addWidget(obGroup);

        auto* listPanel = new QWidget();
        auto* listLayout = new QVBoxLayout(listPanel);
        listLayout->setContentsMargins(8, 8, 8, 8);
        listLayout->setSpacing(6);

        regionTable = new QTableWidget(0, 7);
        regionTable->setHorizontalHeaderLabels({ "#", "X", "Y", "W", "H", "mean", "sigma" });
        regionTable->verticalHeader()->setVisible(false);
        regionTable->setSelectionBehavior(QAbstractItemView::SelectRows);
        regionTable->setSelectionMode(QAbstractItemView::SingleSelection);
        regionTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
        regionTable->setAlternatingRowColors(true);
        regionTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
        regionTable->setMinimumHeight(110);

        auto* listButtons = new QHBoxLayout();
        auto* addRegionBtn = new QPushButton("Add");
        auto* removeRegionBtn = new QPushButton("Remove");
        auto* clearRegionsBtn = new QPushButton("Clear");
        listButtons->addWidget(addRegionBtn);
        listButtons->addWidget(removeRegionBtn);
        listButtons->addWidget(clearRegionsBtn);

        auto* exportRegionsBtn = new QPushButton("Export all regions to .csv");

        listLayout->addWidget(regionTable, 1);
        listLayout->addLayout(listButtons);
        listLayout->addWidget(exportRegionsBtn);

        auto* presetGroup = new QGroupBox("PRESET");
        auto* presetLayout = new QHBoxLayout(presetGroup);
        auto* savePresetBtn = new QPushButton("Save");
        auto* loadPresetBtn = new QPushButton("Load");
        presetLayout->addWidget(savePresetBtn);
        presetLayout->addWidget(loadPresetBtn);
        rightPanel->addWidget(presetGroup);

        logView = new QPlainTextEdit();
        logView->setReadOnly(true);
        logView->setFrameShape(QFrame::NoFrame);

        cpu_btn->setIcon(makeIcon("cpu"));
        gpu_btn->setIcon(makeIcon("gpu"));
        gridBtn->setIcon(makeIcon("grid"));
        clipBtn->setIcon(makeIcon("clip"));
        fitBtn->setIcon(makeIcon("fit"));
        oneToOneBtn->setIcon(makeIcon("one"));
        pressedBtn->setIcon(makeIcon("mosaic"));
        saveBinBtn->setIcon(makeIcon("save"));
        saveBinRegionBtn->setIcon(makeIcon("region"));
        saveTiffBtn->setIcon(makeIcon("save"));
        saveTiffRegionBtn->setIcon(makeIcon("region"));
        saveBtn->setIcon(makeIcon("image"));
        processRegion->setIcon(makeIcon("region"));
        applyRegionBtn->setIcon(makeIcon("region"));
        wholeRowBtn->setIcon(makeIcon("row"));
        wholeColBtn->setIcon(makeIcon("column"));
        exportStatsBtn->setIcon(makeIcon("csv"));
        noiseBtn->setIcon(makeIcon("compare"));
        darkBtn->setIcon(makeIcon("minus"));
        rotateBtn->setIcon(makeIcon("rotate"));
        revertBtn->setIcon(makeIcon("revert"));
        saveTiff16Btn->setIcon(makeIcon("save"));
        obMeasureBtn->setIcon(makeIcon("black"));
        obApplyBtn->setIcon(makeIcon("check"));
        addRegionBtn->setIcon(makeIcon("plus"));
        removeRegionBtn->setIcon(makeIcon("minus"));
        clearRegionsBtn->setIcon(makeIcon("trash"));
        exportRegionsBtn->setIcon(makeIcon("csv"));
        savePresetBtn->setIcon(makeIcon("save"));
        loadPresetBtn->setIcon(makeIcon("open"));

        for (QPushButton* button : { cpu_btn, gpu_btn, fitBtn, oneToOneBtn,
                                     wholeRowBtn, wholeColBtn, addRegionBtn,
                                     removeRegionBtn, clearRegionsBtn,
                                     savePresetBtn, loadPresetBtn,
                                     binningBtn, bilinearBtn, malvarBtn })
            button->setObjectName("pairButton");

        gpuAvailable = cuda_device_available();

        if (!gpuAvailable) {
            gpu_btn->setEnabled(false);
            gpu_btn->setToolTip("CUDA device not found");
            cpu_btn->setChecked(true);
        }

        rightPanel->addStretch();

        auto* rightWidget = new QWidget();
        rightWidget->setLayout(rightPanel);

        for (QGroupBox* group : rightWidget->findChildren<QGroupBox*>()) {

            group->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);

            if (QLayout* groupLayout = group->layout()) {
                groupLayout->setContentsMargins(10, 8, 10, 10);
                groupLayout->setSpacing(6);
            }
        }

        for (QPushButton* button : rightWidget->findChildren<QPushButton*>())
            button->setIconSize(QSize(16, 16));

        for (QLayout* row : rightWidget->findChildren<QHBoxLayout*>())
            row->setSpacing(6);

        rightScroll = new QScrollArea();
        rightScroll->setWidget(rightWidget);
        rightScroll->setWidgetResizable(true);
        rightScroll->setFrameShape(QFrame::NoFrame);
        rightScroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        rightScroll->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
        rightScroll->setMinimumWidth(340);
        rightScroll->setMaximumWidth(420);

        controlsDock = new QDockWidget("CONTROLS", this);
        controlsDock->setObjectName("controlsDock");
        controlsDock->setWidget(rightScroll);
        controlsDock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
        addDockWidget(Qt::RightDockWidgetArea, controlsDock);

        analysisDock = new QDockWidget("ANALYSIS", this);
        analysisDock->setObjectName("analysisDock");
        analysisDock->setWidget(analysisTabs);
        addDockWidget(Qt::BottomDockWidgetArea, analysisDock);

        regionsDock = new QDockWidget("REGION LIST", this);
        regionsDock->setObjectName("regionsDock");
        regionsDock->setWidget(listPanel);
        addDockWidget(Qt::BottomDockWidgetArea, regionsDock);

        logDock = new QDockWidget("LOG", this);
        logDock->setObjectName("logDock");
        logDock->setWidget(logView);
        addDockWidget(Qt::BottomDockWidgetArea, logDock);

        tabifyDockWidget(analysisDock, regionsDock);
        tabifyDockWidget(regionsDock, logDock);
        analysisDock->raise();

        setDockNestingEnabled(true);
        resizeDocks({ analysisDock }, { 210 }, Qt::Vertical);
        analysisTabs->setCurrentIndex(0);

        for (QPushButton* button : { addRegionBtn, removeRegionBtn, clearRegionsBtn, exportRegionsBtn })
            button->setIconSize(QSize(16, 16));

        setCentralWidget(central);

        statusFileLabel = new QLabel("no file");
        statusCursorLabel = new QLabel(" ");
        statusBar()->setSizeGripEnabled(false);
        statusBar()->addWidget(statusFileLabel);

        for (QWidget* wheelTarget : { (QWidget*)xSpin, (QWidget*)ySpin, (QWidget*)wSpin,
                                      (QWidget*)hSpin, (QWidget*)gammaSlider, (QWidget*)zoomSlider }) {
            wheelTarget->setFocusPolicy(Qt::StrongFocus);
            wheelGuarded.insert(wheelTarget, rightScroll->viewport());
        }

        for (QWidget* wheelTarget : noisePage->wheelTargets()) {
            wheelTarget->setFocusPolicy(Qt::StrongFocus);
            wheelGuarded.insert(wheelTarget, noisePage->panelViewport());
        }

        taskWidgets = { loadbtn, loadRefBtn, pressedBtn, processRegion, applyRegionBtn,
                        wholeRowBtn, wholeColBtn, saveBinBtn, saveBinRegionBtn, saveTiffBtn, saveTiffRegionBtn, saveBtn,
                        noiseBtn, darkBtn, exportStatsBtn, cpu_btn, gpu_btn,
                        saveTiff16Btn, revertBtn, obMeasureBtn, obApplyBtn,
                        binningBtn, bilinearBtn, malvarBtn,
                        addRegionBtn, removeRegionBtn, clearRegionsBtn,
                        exportRegionsBtn, savePresetBtn, loadPresetBtn,
                        noisePage->applyBtn, noisePage->captureBtn, noisePage->originalBtn };

        connect(loadbtn, &QPushButton::clicked, this, &MainWindow::onLoad);
        connect(loadRefBtn, &QPushButton::clicked, this, &MainWindow::onLoadReference);
        connect(pressedBtn, &QPushButton::clicked, this, &MainWindow::onProcess);
        connect(binningBtn, &QPushButton::toggled, this, [this](bool on) {
            if (on) setDemosaicMethod(DemosaicMethod::Binning);
        });
        connect(bilinearBtn, &QPushButton::toggled, this, [this](bool on) {
            if (on) setDemosaicMethod(DemosaicMethod::Bilinear);
        });
        connect(malvarBtn, &QPushButton::toggled, this, [this](bool on) {
            if (on) setDemosaicMethod(DemosaicMethod::Malvar);
        });

        connect(cancelBtn, &QPushButton::clicked, this, [this]() {
            cancelRequested = true;
            cancelBtn->setEnabled(false);
            logLine("cancel requested");
        });
        connect(saveBinBtn, &QPushButton::clicked, this, &MainWindow::saveBin);
        connect(saveBinRegionBtn, &QPushButton::clicked, this, &MainWindow::saveRegionBin);
        connect(saveTiffBtn, &QPushButton::clicked, this, &MainWindow::saveTiff);
        connect(saveTiffRegionBtn, &QPushButton::clicked, this, &MainWindow::saveRegionTiff);
        connect(saveBtn, &QPushButton::clicked, this, &MainWindow::savePng);
        connect(processRegion, &QPushButton::clicked, this, &MainWindow::OnProcessRegion);
        connect(gammaSlider, &QSlider::valueChanged, this, &MainWindow::onGammaChanged);
        connect(gammaTimer, &QTimer::timeout, this, &MainWindow::applyPendingGamma);

        connect(applyRegionBtn, &QPushButton::clicked, this, &MainWindow::applyNumericRegion);
        connect(wholeRowBtn, &QPushButton::clicked, this, &MainWindow::onSelectedRow);
        connect(wholeColBtn, &QPushButton::clicked, this, &MainWindow::onSelectWholeColumn);
        connect(exportStatsBtn, &QPushButton::clicked, this, &MainWindow::exportStats);
        connect(noiseBtn, &QPushButton::clicked, this, &MainWindow::compareFrames);
        connect(darkBtn, &QPushButton::clicked, this, &MainWindow::subtractReference);
        connect(revertBtn, &QPushButton::clicked, this, &MainWindow::revertOriginal);
        connect(saveTiff16Btn, &QPushButton::clicked, this, &MainWindow::saveLinearTiff16);
        connect(obMeasureBtn, &QPushButton::clicked, this, &MainWindow::measureOpticalBlack);
        connect(obApplyBtn, &QPushButton::clicked, this, &MainWindow::applyMeasuredBlack);
        connect(addRegionBtn, &QPushButton::clicked, this, &MainWindow::addRegion);
        connect(removeRegionBtn, &QPushButton::clicked, this, &MainWindow::removeRegion);
        connect(clearRegionsBtn, &QPushButton::clicked, this, &MainWindow::clearRegions);
        connect(exportRegionsBtn, &QPushButton::clicked, this, &MainWindow::exportRegions);
        connect(savePresetBtn, &QPushButton::clicked, this, &MainWindow::savePreset);
        connect(loadPresetBtn, &QPushButton::clicked, this, &MainWindow::loadPreset);

        connect(rotateBtn, &QPushButton::clicked, this, [this]() {
            setRotation(viewRotation + 90);
        });

        connect(regionTable, &QTableWidget::itemSelectionChanged, this, [this]() {

            int row = regionTable->currentRow();
            if (row < 0 || row >= savedRegions.size()) return;

            onRegionSelected(savedRegions[row]);
            preview->focusOnRegion(rawToDisplay(savedRegions[row]));
        });

        connect(gridBtn, &QPushButton::toggled, this, [this](bool on) {
            preview->setShowGrid(on);
        });
        connect(clipBtn, &QPushButton::toggled, this, [this](bool on) {
            showClipping = on;
            rebuildPreview();
        });
        connect(zoomSlider, &QSlider::valueChanged, this, [this](int v) {
            double z = v / 100.0;
            zoomValueLabel->setText(QString::number(z, 'f', 2) + "x");
            preview->setZoom(z);
            updateHud();
        });

        connect(fileTabs, &QTabBar::currentChanged, this, [this](int index) {
            activateDocument(index);
        });

        connect(fileTabs, &QTabBar::tabCloseRequested, this, [this](int index) {
            closeDocument(index);
        });

        connect(titleBar->minimizeBtn, &QPushButton::clicked, this, &MainWindow::showMinimized);
        connect(titleBar->closeBtn, &QPushButton::clicked, this, &MainWindow::close);
        connect(titleBar->maximizeBtn, &QPushButton::clicked, this, [this]() {
            if (isMaximized()) showNormal();
            else showMaximized();
        });

        for (QPushButton* button : { cpu_btn, gpu_btn, gridBtn, clipBtn })
            connect(button, &QPushButton::toggled, this, [button](bool on) { applyGlow(button, on); });
        connect(fitBtn, &QPushButton::clicked, this, [this]() { setZoom(kMinZoom); });
        connect(oneToOneBtn, &QPushButton::clicked, this, [this]() {
            setZoom(preview->oneToOneZoom());
        });

        preview->onZoomChanged = [this](double z) {
            zoomSlider->blockSignals(true);
            zoomSlider->setValue(static_cast<int>(std::lround(z * 100)));
            zoomSlider->blockSignals(false);
            zoomValueLabel->setText(QString::number(z, 'f', 2) + "x");
        };

        preview->onCursorMoved = [this](QPoint p, bool inside) {

            if (!inside) {
                statusCursorLabel->setText(" ");
                hudCursor.clear();
                updateHud();
                return;
            }

            updateCursorReadout(p);
        };

        new QShortcut(QKeySequence("Ctrl+1"), this, [this]() { showPage(0); });
        new QShortcut(QKeySequence("Ctrl+2"), this, [this]() { showPage(1); });

        undoStack = new QUndoStack(this);
        undoStack->setUndoLimit(12);

        progressTimer = new QTimer(this);
        progressTimer->setInterval(120);

        connect(progressTimer, &QTimer::timeout, this, [this]() {
            progress->setValue(taskPercent.load());
            noisePage->updateProgress(taskPercent.load());
            updateHud();
        });

        buildActions();

        preview->loupeText = [this](QPoint p) -> QString {

            if (!rawImage) return QString();

            QRect mapped = displayToRaw(QRect(p, QSize(1, 1)));

            if (mapped.x() < 0 || mapped.y() < 0 ||
                mapped.x() >= rawImage->width || mapped.y() >= rawImage->height)
                return QString("%1, %2").arg(p.x()).arg(p.y());

            uint16_t value = rawImage->data[static_cast<std::size_t>(mapped.y())
                * rawImage->width + mapped.x()];

            std::array<std::string, 4> labels = cfa_labels(*rawImage);

            return QString("%1, %2   %3 %4").arg(mapped.x()).arg(mapped.y())
                       .arg(QString::fromStdString(labels[cfa_index(mapped.x(), mapped.y())]))
                       .arg(value);
        };

        statusGpuLabel = new QLabel(QString::fromStdString(cuda_device_name()));
        statusMemLabel = new QLabel("--");
        statusVramLabel = new QLabel("--");
        statusTimeLabel = new QLabel("--");

        statusBar()->addPermanentWidget(statusGpuLabel);
        statusBar()->addPermanentWidget(statusVramLabel);
        statusBar()->addPermanentWidget(statusMemLabel);
        statusBar()->addPermanentWidget(statusTimeLabel);
        statusBar()->addPermanentWidget(statusCursorLabel);

        auto* memTimer = new QTimer(this);
        memTimer->setInterval(2000);
        connect(memTimer, &QTimer::timeout, this, &MainWindow::updateMemoryUsage);
        memTimer->start();
        updateMemoryUsage();

        qApp->installEventFilter(this);

        loadSettings();

        for (QPushButton* button : { cpu_btn, gpu_btn, gridBtn, clipBtn })
            applyGlow(button, button->isChecked());
    }

    ~MainWindow() override {
        if (worker.joinable()) worker.join();
    }

    void openPath(const QString& path) { loadRawFile(path); }

    void applyRegionList(const QVector<QRect>& regions) {

        savedRegions = regions;
        rebuildRegionTable();
    }

    void applyRawFrame(const RawImage& frame) {

        rawImage = frame;

        linearResult.reset();
        resultImage.reset();

        rebuildPreview();
        refreshAnalysis();
        rebuildRegionTable();
        updateStatusFile();
        updateHud();
        refreshNoisePage();
    }

protected:
    void changeEvent(QEvent* event) override {

        QMainWindow::changeEvent(event);

        if (event->type() == QEvent::WindowStateChange && titleBar)
            titleBar->maximizeBtn->setText(isMaximized() ? QString::fromUtf8("\u2750")
                                                         : QString::fromUtf8("\u25a1"));
    }

#ifdef _WIN32
    bool nativeEvent(const QByteArray& eventType, void* message, qintptr* result) override {

        MSG* msg = static_cast<MSG*>(message);
        if (!msg) return QMainWindow::nativeEvent(eventType, message, result);

        if (msg->message == WM_NCCALCSIZE && msg->wParam == TRUE) {

            if (::IsZoomed(msg->hwnd)) {

                auto* params = reinterpret_cast<NCCALCSIZE_PARAMS*>(msg->lParam);
                int frame = ::GetSystemMetrics(SM_CXSIZEFRAME) + ::GetSystemMetrics(SM_CXPADDEDBORDER);

                params->rgrc[0].left += frame;
                params->rgrc[0].top += frame;
                params->rgrc[0].right -= frame;
                params->rgrc[0].bottom -= frame;
            }

            *result = 0;
            return true;
        }

        if (msg->message == WM_NCHITTEST) {

            RECT frame;
            ::GetWindowRect(msg->hwnd, &frame);

            int px = GET_X_LPARAM(msg->lParam) - frame.left;
            int py = GET_Y_LPARAM(msg->lParam) - frame.top;
            int pw = frame.right - frame.left;
            int ph = frame.bottom - frame.top;

            qreal ratio = devicePixelRatioF();
            int border = static_cast<int>(6 * ratio);

            if (!::IsZoomed(msg->hwnd)) {

                bool left = px >= 0 && px < border;
                bool right = px < pw && px >= pw - border;
                bool top = py >= 0 && py < border;
                bool bottom = py < ph && py >= ph - border;

                if (top && left) { *result = HTTOPLEFT; return true; }
                if (top && right) { *result = HTTOPRIGHT; return true; }
                if (bottom && left) { *result = HTBOTTOMLEFT; return true; }
                if (bottom && right) { *result = HTBOTTOMRIGHT; return true; }
                if (left) { *result = HTLEFT; return true; }
                if (right) { *result = HTRIGHT; return true; }
                if (top) { *result = HTTOP; return true; }
                if (bottom) { *result = HTBOTTOM; return true; }
            }

            QPoint local(static_cast<int>(px / ratio), static_cast<int>(py / ratio));

            if (local.y() >= 0 && local.y() < titleBar->height() && !titleBar->overButton(local)) {
                *result = HTCAPTION;
                return true;
            }

            *result = HTCLIENT;
            return true;
        }

        return QMainWindow::nativeEvent(eventType, message, result);
    }
#endif

    void dragEnterEvent(QDragEnterEvent* event) override {

        if (event->mimeData() && event->mimeData()->hasUrls()) event->acceptProposedAction();
    }

    void dropEvent(QDropEvent* event) override {

        if (!event->mimeData()) return;

        for (const QUrl& url : event->mimeData()->urls()) {

            if (!url.isLocalFile()) continue;

            event->acceptProposedAction();
            loadRawFile(url.toLocalFile());
            return;
        }
    }

    void closeEvent(QCloseEvent* event) override {
        saveSettings();
        QMainWindow::closeEvent(event);
    }

    bool eventFilter(QObject* watched, QEvent* event) override {

        if (event->type() == QEvent::Wheel) {

            QWidget* widget = qobject_cast<QWidget*>(watched);

            if (widget && wheelGuarded.contains(widget) && !widget->hasFocus()) {
                QCoreApplication::sendEvent(wheelGuarded.value(widget), event);
                return true;
            }
        }

        if (event->type() == QEvent::KeyPress && isActiveWindow()) {

            auto* keyEvent = static_cast<QKeyEvent*>(event);

            if (keyEvent->key() == Qt::Key_Tab && keyEvent->modifiers() == Qt::NoModifier) {

                QWidget* focus = QApplication::focusWidget();

                bool editing = qobject_cast<QLineEdit*>(focus) ||
                               qobject_cast<QAbstractSpinBox*>(focus) ||
                               qobject_cast<QPlainTextEdit*>(focus);

                if (!editing) {
                    panelsAction->toggle();
                    togglePanels(panelsAction->isChecked());
                    return true;
                }
            }
        }

        return QMainWindow::eventFilter(watched, event);
    }

private:
    std::optional<RawImage> rawImage;
    std::optional<RawImage> refRaw;
    std::optional<Image> linearResult;
    std::optional<Image> resultImage;
    QRect selectedRegion;

    ImageLabel* preview;
    QLabel* statsText;
    QLabel* noiseText;
    QLabel* regionCoordsLabel;
    QLabel* gammaValueLabel;
    QLabel* statusFileLabel;
    QLabel* statusCursorLabel;
    QLabel* statusGpuLabel;
    QLabel* statusMemLabel;
    QLabel* statusVramLabel;
    QLabel* statusTimeLabel;
    QToolBar* toolBar;
    QAction* gridAction;
    QAction* clipAction;
    QAction* thirdsAction;
    QAction* rulersAction;
    QAction* minimapAction;
    QAction* loupeAction;
    QAction* compareAction;
    QAction* fullScreenAction;
    QAction* panelsAction;
    QElapsedTimer taskTimer;
    QUndoStack* undoStack;
    QPushButton* cancelBtn;
    QPushButton* binningBtn;
    QPushButton* bilinearBtn;
    QPushButton* malvarBtn;
    DemosaicMethod demosaicMethod = DemosaicMethod::Binning;
    NoisePage* noisePage = nullptr;
    QStackedWidget* pages = nullptr;
    QPushButton* imagePageBtn = nullptr;
    QPushButton* noisePageBtn = nullptr;
    std::shared_ptr<const RawImage> beforeFrame;
    QString beforeTitle;
    QRect noiseBeforeRegion;
    NoiseSnapshot noiseBefore;
    NoiseSnapshot noiseAfter;
    std::optional<RawImage> pendingFrame;
    std::function<void()> taskDone;
    QTimer* progressTimer;
    std::atomic<int> taskPercent{0};
    std::atomic<bool> cancelRequested{false};
    QHash<QWidget*, QWidget*> wheelGuarded;
    std::vector<Document> documents;
    int activeDocument = -1;
    QTabBar* fileTabs;
    bool compareEnabled = false;
    QSlider* gammaSlider;
    QPushButton* cpu_btn;
    QPushButton* gpu_btn;
    QPushButton* gridBtn;
    QPushButton* clipBtn;
    QProgressBar* progress;
    HistogramWidget* histogram;
    ProfileWidget* profile;

    QTimer* gammaTimer;
    int pendingGammaValue = 220;

    QSpinBox* xSpin;
    QSpinBox* ySpin;
    QSpinBox* wSpin;
    QSpinBox* hSpin;

    QSlider* zoomSlider;
    QLabel* zoomValueLabel;
    QScrollArea* rightScroll;

    QList<QWidget*> taskWidgets;
    QString taskName;
    std::optional<RawImage> originalRaw;
    QVector<QRect> savedRegions;
    QTableWidget* regionTable;
    QPlainTextEdit* logView;
    QLabel* metaText;
    QLabel* obText;
    QPushButton* rotateBtn;
    QDockWidget* controlsDock;
    QDockWidget* analysisDock;
    QDockWidget* regionsDock;
    QDockWidget* logDock;
    TitleBar* titleBar = nullptr;
    QLabel* hudLabel;
    QString hudCursor;
    int viewRotation = 0;
    double measuredBlack = -1.0;
    bool gpuAvailable = true;
    std::thread worker;
    bool busy = false;
    bool showClipping = false;
    QString lastDir;
    QString currentFile;

    Backend currentBack() const { return gpu_btn->isChecked() ? Backend::GPU : Backend::CPU; }

    QSize shownSize() const {
        return QSize(preview->imageWidth(), preview->imageHeight());
    }

    QSize plainSize() const {

        QSize shown = shownSize();
        return (viewRotation % 180) ? QSize(shown.height(), shown.width()) : shown;
    }

    QRect displayToRaw(const QRect& r) const {

        if (!rawImage || r.isEmpty() || preview->imageWidth() <= 0 || preview->imageHeight() <= 0)
            return r;

        QRect plain = rotateRect(r, shownSize(), (360 - viewRotation) % 360);
        QSize size = plainSize();

        double sx = (double)rawImage->width / size.width();
        double sy = (double)rawImage->height / size.height();

        return QRect(static_cast<int>(std::lround(plain.x() * sx)),
                     static_cast<int>(std::lround(plain.y() * sy)),
                     std::max(1, static_cast<int>(std::lround(plain.width() * sx))),
                     std::max(1, static_cast<int>(std::lround(plain.height() * sy))));
    }

    QRect rawToDisplay(const QRect& r) const {

        if (!rawImage || r.isEmpty() || preview->imageWidth() <= 0 || preview->imageHeight() <= 0)
            return r;

        QSize size = plainSize();

        double sx = (double)size.width() / rawImage->width;
        double sy = (double)size.height() / rawImage->height;

        QRect scaled(static_cast<int>(std::lround(r.x() * sx)),
                     static_cast<int>(std::lround(r.y() * sy)),
                     std::max(1, static_cast<int>(std::lround(r.width() * sx))),
                     std::max(1, static_cast<int>(std::lround(r.height() * sy))));

        return rotateRect(scaled, size, viewRotation);
    }

    void setZoom(double z) {
        zoomSlider->setValue(static_cast<int>(std::lround(std::clamp(z, kMinZoom, kMaxZoom) * 100)));
    }

    void setControlsEnabled(bool enabled) {

        for (QWidget* w : taskWidgets)
            w->setEnabled(enabled && (w != gpu_btn || gpuAvailable));
    }

    static void applyGlow(QWidget* widget, bool on) {

        if (!on) { widget->setGraphicsEffect(nullptr); return; }

        auto* glow = new QGraphicsDropShadowEffect(widget);
        glow->setBlurRadius(26);
        glow->setColor(QColor(0, 229, 255, 170));
        glow->setOffset(0, 0);
        widget->setGraphicsEffect(glow);
    }

    void updateHud() {

        if (!rawImage) { hudLabel->hide(); return; }

        QString top = QString("%1     %2x%3     %4")
                          .arg(currentFile)
                          .arg(rawImage->width).arg(rawImage->height)
                          .arg(QString::fromStdString(cfa_pattern_name(*rawImage)));

        QString bottom = hudCursor;
        if (bottom.isEmpty()) bottom = QString("black %1   white %2")
                                           .arg(rawImage->black_level).arg(rawImage->white_level);

        bottom += QString("     %1x").arg(preview->zoom(), 0, 'f', 2);

        if (busy) bottom = QString("%1 %2%").arg(taskName).arg(taskPercent.load());

        hudLabel->setText(QString(
            "<div style='font-family:Consolas,monospace;font-size:11px;color:#d8e6f5'>%1</div>"
            "<div style='font-family:Consolas,monospace;font-size:11px;color:#5d6f8c'>%2</div>")
                .arg(top.toHtmlEscaped(), bottom.toHtmlEscaped()));

        hudLabel->show();
    }

    void logLine(const QString& text) {
        logView->appendPlainText(QTime::currentTime().toString("HH:mm:ss") + "  " + text);
    }

    static QRect rotateRect(const QRect& r, const QSize& size, int degrees) {

        if (degrees == 90)
            return QRect(size.height() - r.y() - r.height(), r.x(), r.height(), r.width());

        if (degrees == 180)
            return QRect(size.width() - r.x() - r.width(),
                         size.height() - r.y() - r.height(), r.width(), r.height());

        if (degrees == 270)
            return QRect(r.y(), size.width() - r.x() - r.width(), r.height(), r.width());

        return r;
    }

    QImage applyRotation(const QImage& image) const {

        if (viewRotation == 0) return image;
        return image.transformed(QTransform().rotate(viewRotation));
    }

    void setRotation(int degrees) {

        viewRotation = ((degrees % 360) + 360) % 360;
        viewRotation -= viewRotation % 90;

        rotateBtn->setText(QString("Rotate 90°   (%1°)").arg(viewRotation));
        rebuildPreview();
    }

    void syncPreviewRegions() {

        QVector<QRect> mapped;
        for (const QRect& region : savedRegions) mapped.append(rawToDisplay(region));

        preview->setMarkedRegions(mapped);
        preview->setSelectionInImageCoords(rawToDisplay(selectedRegion));
    }

    void startTask(const QString& title, std::function<void()> work, bool cancellable = false,
                   std::function<void()> done = {}) {

        if (busy) return;
        if (worker.joinable()) worker.join();

        busy = true;
        taskName = title;
        taskPercent = 0;
        cancelRequested = false;
        taskTimer.start();
        taskDone = std::move(done);

        setControlsEnabled(false);
        progress->setValue(0);
        progress->setVisible(true);
        cancelBtn->setEnabled(true);
        cancelBtn->setVisible(cancellable);
        progressTimer->start();
        noisePage->startProgress(cancellable);
        statusBar()->showMessage(title);
        logLine(title);

        worker = std::thread([this, work]() {

            QString error;

            try { work(); }
            catch (const std::exception& ex) { error = QString::fromUtf8(ex.what()); }
            catch (...) { error = "Unknown processing error"; }

            QMetaObject::invokeMethod(this, [this, error]() { finishTask(error); },
                                      Qt::QueuedConnection);
        });
    }

    void finishTask(const QString& result) {

        if (worker.joinable()) worker.join();

        QString error = result.isEmpty() && taskDone && cancelRequested.load() ? QString("cancelled") : result;

        busy = false;
        progressTimer->stop();
        progress->setVisible(false);
        cancelBtn->setVisible(false);
        noisePage->stopProgress();
        setControlsEnabled(true);
        updateHud();

        if (!error.isEmpty()) {
            taskDone = nullptr;
            pendingFrame.reset();
        }

        if (error == "cancelled") {
            statusBar()->showMessage("Cancelled", 3000);
            logLine("cancelled: " + taskName);
            return;
        }

        if (!error.isEmpty()) {
            statusBar()->showMessage("Failed", 4000);
            logLine("failed: " + error);
            QMessageBox::critical(this, "Error", error);
            return;
        }

        statusBar()->showMessage("Done", 3000);
        statusTimeLabel->setText(QString("last %1 s").arg(taskTimer.elapsed() / 1000.0, 0, 'f', 2));
        logLine(QString("done: %1  (%2 s)").arg(taskName).arg(taskTimer.elapsed() / 1000.0, 0, 'f', 2));
        std::function<void()> callback = std::move(taskDone);
        taskDone = nullptr;

        if (callback) callback();
        else onGammaChanged(gammaSlider->value());
    }

    QImage imageToQt(const Image& im) const {

        QImage source(im.data.data(), im.width, im.height,
                      im.width * im.channels, QImage::Format_RGB888);

        QImage result = source.copy();
        if (!showClipping) return applyRotation(result);

        for (int y = 0; y < result.height(); ++y) {

            uchar* line = result.scanLine(y);

            for (int x = 0; x < result.width(); ++x) {

                uchar* px = line + x * 3;

                if (px[0] == 255 || px[1] == 255 || px[2] == 255) {
                    px[0] = 255; px[1] = 48; px[2] = 48;
                } else if (px[0] == 0 && px[1] == 0 && px[2] == 0) {
                    px[0] = 40; px[1] = 90; px[2] = 255;
                }
            }
        }

        return applyRotation(result);
    }

    QImage rawToQt() const {

        float range = static_cast<float>(rawImage->white_level - rawImage->black_level);
        if (range <= 0.f) range = 1.f;

        if (!showClipping) {

            std::vector<uint8_t> preview8(rawImage->data.size());

            for (size_t i = 0; i < rawImage->data.size(); ++i) {

                float v = (static_cast<float>(rawImage->data[i]) - rawImage->black_level) / range;
                v = std::min(1.f, std::max(0.f, v));
                preview8[i] = static_cast<uint8_t>(v * 255.f);
            }

            QImage gray(preview8.data(), rawImage->width, rawImage->height,
                        rawImage->width, QImage::Format_Grayscale8);

            return applyRotation(gray.copy());
        }

        QImage marked(rawImage->width, rawImage->height, QImage::Format_RGB888);

        for (int y = 0; y < rawImage->height; ++y) {

            uchar* line = marked.scanLine(y);

            for (int x = 0; x < rawImage->width; ++x) {

                uint16_t raw = rawImage->data[static_cast<std::size_t>(y) * rawImage->width + x];
                uchar* px = line + x * 3;

                if (raw >= rawImage->white_level) {
                    px[0] = 255; px[1] = 48; px[2] = 48;
                } else if (raw <= rawImage->black_level) {
                    px[0] = 40; px[1] = 90; px[2] = 255;
                } else {

                    float v = (static_cast<float>(raw) - rawImage->black_level) / range;
                    v = std::min(1.f, std::max(0.f, v));

                    uint8_t g = static_cast<uint8_t>(v * 255.f);
                    px[0] = g; px[1] = g; px[2] = g;
                }
            }
        }

        return applyRotation(marked);
    }

    void rebuildPreview() {

        if (compareEnabled && rawImage && resultImage) {

            preview->setBaseImage(rawToQt());
            preview->setCompareImage(imageToQt(*resultImage));

        } else if (resultImage) {

            preview->setBaseImage(imageToQt(*resultImage));
            preview->setCompareImage(QImage());

        } else if (rawImage) {

            preview->setBaseImage(rawToQt());
            preview->setCompareImage(QImage());
        }

        syncPreviewRegions();
    }

    void updateMemoryUsage() {

#ifdef _WIN32
        PROCESS_MEMORY_COUNTERS counters;
        counters.cb = sizeof(counters);

        if (::GetProcessMemoryInfo(::GetCurrentProcess(), &counters, sizeof(counters)))
            statusMemLabel->setText(QString("RAM %1 MB")
                                        .arg(counters.WorkingSetSize / (1024 * 1024)));
#endif

        if (!gpuAvailable) { statusVramLabel->setText("VRAM --"); return; }

        std::size_t used = 0;
        std::size_t total = 0;

        if (cuda_memory_usage(used, total))
            statusVramLabel->setText(QString("VRAM %1 / %2 MB").arg(used).arg(total));
    }

    QAction* makeAction(QMenu* menu, const QString& text, const QString& shortcut,
                        const QString& icon, std::function<void(bool)> handler,
                        bool checkable = false, bool checked = false) {

        auto* action = new QAction(text, this);

        if (!shortcut.isEmpty()) action->setShortcut(QKeySequence(shortcut));
        if (!icon.isEmpty()) action->setIcon(makeIcon(icon));

        action->setCheckable(checkable);
        action->setChecked(checked);

        connect(action, &QAction::triggered, this, [handler](bool state) { handler(state); });

        if (menu) menu->addAction(action);
        addAction(action);

        return action;
    }

    void buildActions() {

        QMenu* fileMenu = new QMenu("File", this);
        makeAction(fileMenu, "Open RAW...", "Ctrl+O", "open", [this](bool) { onLoad(); });
        makeAction(fileMenu, "Open reference RAW...", "", "layers", [this](bool) { onLoadReference(); });
        fileMenu->addSeparator();
        makeAction(fileMenu, "Save RAW to .bin", "", "save", [this](bool) { saveBin(); });
        makeAction(fileMenu, "Save selected region to .bin", "", "region",
                   [this](bool) { saveRegionBin(); });
        makeAction(fileMenu, "Save selected region to .tiff", "", "region",
                   [this](bool) { saveRegionTiff(); });
        makeAction(fileMenu, "Save RAW to .tiff", "", "save", [this](bool) { saveTiff(); });
        makeAction(fileMenu, "Save result to .png", "Ctrl+S", "image", [this](bool) { savePng(); });
        makeAction(fileMenu, "Save linear to 16-bit TIFF", "", "save", [this](bool) { saveLinearTiff16(); });
        fileMenu->addSeparator();
        makeAction(fileMenu, "Export stats to .csv", "", "csv", [this](bool) { exportStats(); });
        makeAction(fileMenu, "Export regions to .csv", "", "csv", [this](bool) { exportRegions(); });
        fileMenu->addSeparator();
        makeAction(fileMenu, "Save preset...", "", "save", [this](bool) { savePreset(); });
        makeAction(fileMenu, "Load preset...", "", "open", [this](bool) { loadPreset(); });
        fileMenu->addSeparator();
        makeAction(fileMenu, "Close file", "Ctrl+W", "trash",
                   [this](bool) { closeDocument(activeDocument); });
        makeAction(fileMenu, "Quit", "Ctrl+Q", "", [this](bool) { close(); });

        QMenu* editMenu = new QMenu("Edit", this);

        QAction* undoAction = undoStack->createUndoAction(this, "Undo");
        QAction* redoAction = undoStack->createRedoAction(this, "Redo");

        undoAction->setShortcut(QKeySequence::Undo);
        redoAction->setShortcut(QKeySequence::Redo);
        undoAction->setIcon(makeIcon("revert"));
        redoAction->setIcon(makeIcon("rotate"));

        editMenu->addAction(undoAction);
        editMenu->addAction(redoAction);
        addAction(undoAction);
        addAction(redoAction);

        QMenu* viewMenu = new QMenu("View", this);

        gridAction = makeAction(viewMenu, "Pixel grid", "G", "grid",
                                [this](bool on) { gridBtn->setChecked(on); }, true);
        clipAction = makeAction(viewMenu, "Clipping", "C", "clip",
                                [this](bool on) { clipBtn->setChecked(on); }, true);
        thirdsAction = makeAction(viewMenu, "Rule of thirds", "T", "grid",
                                  [this](bool on) { preview->setThirdsEnabled(on); }, true);
        rulersAction = makeAction(viewMenu, "Rulers", "U", "ruler",
                                  [this](bool on) { preview->setRulersEnabled(on); }, true);
        minimapAction = makeAction(viewMenu, "Minimap", "M", "map",
                                   [this](bool on) { preview->setMinimapEnabled(on); }, true, true);
        loupeAction = makeAction(viewMenu, "Loupe", "L", "one",
                                 [this](bool on) { preview->setLoupeEnabled(on); }, true);
        compareAction = makeAction(viewMenu, "Compare before / after", "B", "compare",
                                   [this](bool on) { toggleCompare(on); }, true);

        viewMenu->addSeparator();
        makeAction(viewMenu, "Rotate 90", "R", "rotate", [this](bool) { setRotation(viewRotation + 90); });
        makeAction(viewMenu, "Fit to window", "F", "fit", [this](bool) { setZoom(kMinZoom); });
        makeAction(viewMenu, "Zoom 1:1", "1", "one", [this](bool) { setZoom(preview->oneToOneZoom()); });

        viewMenu->addSeparator();
        fullScreenAction = makeAction(viewMenu, "Full screen", "F11", "expand",
                                      [this](bool on) { toggleFullScreen(on); }, true);
        panelsAction = makeAction(viewMenu, "Hide panels", "", "map",
                                  [this](bool on) { togglePanels(on); }, true);
        viewMenu->addSeparator();
        makeAction(viewMenu, "Copy view to clipboard", "Ctrl+Shift+C", "copy",
                   [this](bool) { copyViewToClipboard(); });

        QMenu* analysisMenu = new QMenu("Analysis", this);
        makeAction(analysisMenu, "Apply demosaic", "Ctrl+D", "mosaic", [this](bool) { onProcess(); });

        QMenu* methodMenu = analysisMenu->addMenu("Demosaic method");

        makeAction(methodMenu, "Binning 2x2 (half size)", "", "mosaic",
                   [this](bool) { binningBtn->setChecked(true); });
        makeAction(methodMenu, "Bilinear (full size)", "", "mosaic",
                   [this](bool) { bilinearBtn->setChecked(true); });
        makeAction(methodMenu, "Malvar-He-Cutler (full size)", "", "mosaic",
                   [this](bool) { malvarBtn->setChecked(true); });
        makeAction(analysisMenu, "Demosaic selected region", "", "region", [this](bool) { OnProcessRegion(); });
        analysisMenu->addSeparator();
        makeAction(analysisMenu, "Apply region by coordinates", "", "region", [this](bool) { applyNumericRegion(); });
        makeAction(analysisMenu, "Select whole row", "", "row", [this](bool) { onSelectedRow(); });
        makeAction(analysisMenu, "Select whole column", "", "column", [this](bool) { onSelectWholeColumn(); });
        makeAction(analysisMenu, "Add region to list", "Ctrl+A", "plus", [this](bool) { addRegion(); });
        analysisMenu->addSeparator();
        makeAction(analysisMenu, "Compare with reference", "", "compare", [this](bool) { compareFrames(); });
        makeAction(analysisMenu, "Subtract reference as dark", "", "minus", [this](bool) { subtractReference(); });
        makeAction(analysisMenu, "Revert to original", "", "revert", [this](bool) { revertOriginal(); });
        analysisMenu->addSeparator();
        makeAction(analysisMenu, "Measure optical black", "", "black", [this](bool) { measureOpticalBlack(); });
        makeAction(analysisMenu, "Apply measured black level", "", "check", [this](bool) { applyMeasuredBlack(); });

        makeAction(analysisMenu, "Noise histogram (before / after)", "Ctrl+H", "compare",
                   [this](bool) { showPage(1); });

        QMenu* denoiseMenu = analysisMenu->addMenu("Denoise filter");

        makeAction(denoiseMenu, "Gaussian", "", "filter", [this](bool) {
            noisePage->setMethod(DenoiseMethod::Gaussian);
            showPage(1);
        });

        makeAction(denoiseMenu, "Median", "", "filter", [this](bool) {
            noisePage->setMethod(DenoiseMethod::Median);
            showPage(1);
        });

        makeAction(denoiseMenu, "Bilateral", "", "filter", [this](bool) {
            noisePage->setMethod(DenoiseMethod::Bilateral);
            showPage(1);
        });

        denoiseMenu->addSeparator();

        makeAction(denoiseMenu, "Apply current filter", "Ctrl+Shift+D", "filter", [this](bool) {
            showPage(1);
            applyDenoise();
        });

        QMenu* helpMenu = new QMenu("Help", this);
        makeAction(helpMenu, "About", "", "", [this](bool) { showAbout(); });

        auto* rootMenu = new QMenu(this);
        rootMenu->addMenu(fileMenu);
        rootMenu->addMenu(editMenu);
        rootMenu->addMenu(viewMenu);
        rootMenu->addMenu(analysisMenu);
        rootMenu->addSeparator();
        rootMenu->addMenu(helpMenu);

        auto* menuButton = new QToolButton();
        menuButton->setObjectName("menuButton");
        menuButton->setIcon(makeIcon("menu"));
        menuButton->setIconSize(QSize(16, 16));
        menuButton->setToolTip("Menu");
        menuButton->setPopupMode(QToolButton::InstantPopup);
        menuButton->setMenu(rootMenu);

        connect(gridBtn, &QPushButton::toggled, gridAction, &QAction::setChecked);
        connect(clipBtn, &QPushButton::toggled, clipAction, &QAction::setChecked);

        toolBar = addToolBar("Main");
        toolBar->setObjectName("mainToolBar");
        toolBar->setMovable(false);
        toolBar->setIconSize(QSize(16, 16));

        toolBar->addWidget(menuButton);
        toolBar->addSeparator();
        toolBar->addAction(fileMenu->actions().at(0));
        toolBar->addAction(analysisMenu->actions().at(0));
        toolBar->addSeparator();
        toolBar->addAction(undoAction);
        toolBar->addAction(redoAction);
        toolBar->addSeparator();
        toolBar->addAction(gridAction);
        toolBar->addAction(clipAction);
        toolBar->addAction(thirdsAction);
        toolBar->addAction(rulersAction);
        toolBar->addAction(minimapAction);
        toolBar->addAction(loupeAction);
        toolBar->addAction(compareAction);
        toolBar->addSeparator();
        toolBar->addAction(viewMenu->actions().at(8));
        toolBar->addAction(viewMenu->actions().at(9));
        toolBar->addAction(viewMenu->actions().at(10));
        toolBar->addSeparator();
        toolBar->addAction(analysisMenu->actions().at(6));
        toolBar->addAction(fullScreenAction);
        toolBar->addAction(viewMenu->actions().last());
    }

    QRect noiseRegion() const {

        if (!rawImage) return QRect();

        return selectedRegion.isEmpty() ? QRect(0, 0, rawImage->width, rawImage->height)
                                        : selectedRegion;
    }

    static void fitHistogramRange(double halfRange, double& span, int& bins) {

        double target = std::max(8.0, halfRange);
        double step = std::max(1.0, std::ceil(2.0 * target / (kHistogramBins - 1)));

        bins = std::min(kHistogramBins, static_cast<int>(std::ceil(2.0 * target / step)) + 1);
        span = step * (bins - 1) / 2.0;
    }

    NoiseSnapshot makeNoiseSnapshot(const RawImage& frame, const QRect& area, const NoiseSnapshot* layout) {

        NoiseSnapshot snapshot;

        QRect region = area.intersected(QRect(0, 0, frame.width, frame.height));
        if (region.isEmpty()) return snapshot;

        ChannelStats channels = compute_region_channels(frame, region.x(), region.y(),
                                                        region.width(), region.height());

        snapshot.labels = channels.labels;

        double widest = 0.0;
        double low = 1e300;
        double high = -1e300;

        for (int k = 0; k < 4; ++k) {

            snapshot.mean[k] = channels.stats[k].mean;
            snapshot.sigma[k] = channels.stats[k].stdDev;

            widest = std::max(widest, channels.stats[k].stdDev);
            low = std::min(low, channels.stats[k].mean - 4.0 * channels.stats[k].stdDev);
            high = std::max(high, channels.stats[k].mean + 4.0 * channels.stats[k].stdDev);
        }

        if (layout && layout->valid) {

            snapshot.span = layout->span;
            snapshot.bins = layout->bins;
            snapshot.valueCenter = layout->valueCenter;
            snapshot.valueSpan = layout->valueSpan;
            snapshot.valueBins = layout->valueBins;

        } else {

            fitHistogramRange(widest * 4.0, snapshot.span, snapshot.bins);
            fitHistogramRange((high - low) / 2.0, snapshot.valueSpan, snapshot.valueBins);
            snapshot.valueCenter = std::round((low + high) / 2.0);
        }

        std::array<double, 4> centers;
        centers.fill(snapshot.valueCenter);

        snapshot.values = compute_noise_histogram(frame, region.x(), region.y(),
                                                  region.width(), region.height(),
                                                  snapshot.valueBins, snapshot.valueSpan, centers);

        snapshot.noise = compute_noise_histogram(frame, region.x(), region.y(),
                                                 region.width(), region.height(),
                                                 snapshot.bins, snapshot.span, snapshot.mean);

        snapshot.valid = true;
        return snapshot;
    }

    void captureNoiseBefore(const QString& title) {

        if (!rawImage) return;

        beforeFrame = std::make_shared<const RawImage>(*rawImage);
        beforeTitle = title;
        noiseBefore = NoiseSnapshot();

        logLine("noise reference: " + title);
    }

    void autoCaptureBefore(const QString& title) {

        if (noisePage->lockBefore() && beforeFrame) return;

        captureNoiseBefore(title);
    }

    void useOriginalAsBefore() {

        if (!originalRaw) return;

        beforeFrame = std::make_shared<const RawImage>(*originalRaw);
        beforeTitle = "original file";
        noiseBefore = NoiseSnapshot();

        logLine("noise reference: original file");
        refreshNoisePage();
    }

    void resetNoiseReference() {

        beforeFrame.reset();
        beforeTitle.clear();
        noiseBeforeRegion = QRect();
        noiseBefore = NoiseSnapshot();
        noiseAfter = NoiseSnapshot();
    }

    bool noisePageShown() const {
        return pages && noisePage && pages->currentWidget() == noisePage;
    }

    void showPage(int index) {

        if (!pages) return;

        index = std::clamp(index, 0, 1);

        imagePageBtn->setChecked(index == 0);
        noisePageBtn->setChecked(index == 1);
        pages->setCurrentIndex(index);

        if (index == 1) refreshNoisePage();
        else preview->setFocus();
    }

    void refreshNoisePage() {

        if (!noisePageShown()) return;

        if (!rawImage) {
            noiseBefore = NoiseSnapshot();
            noiseAfter = NoiseSnapshot();
            redrawNoisePage();
            return;
        }

        auto sameSize = [this](const RawImage& frame) {
            return frame.width == rawImage->width && frame.height == rawImage->height;
        };

        if (!beforeFrame || !sameSize(*beforeFrame)) {

            bool fromOriginal = originalRaw && sameSize(*originalRaw);

            beforeFrame = std::make_shared<const RawImage>(fromOriginal ? *originalRaw : *rawImage);
            beforeTitle = fromOriginal ? "original file" : "state when opened";
            noiseBefore = NoiseSnapshot();
        }

        QRect region = noiseRegion();

        if (!noiseBefore.valid || noiseBeforeRegion != region) {

            noiseBefore = makeNoiseSnapshot(*beforeFrame, region, nullptr);
            noiseBefore.title = beforeTitle;
            noiseBeforeRegion = region;
        }

        noiseAfter = makeNoiseSnapshot(*rawImage, region, &noiseBefore);
        noiseAfter.title = "current";

        redrawNoisePage();
    }

    int noiseChannel() const {

        static const char* names[5] = { "ALL", "R", "G1", "G2", "B" };

        int selected = noisePage->channel();
        if (selected <= 0 || !noiseAfter.valid) return -1;

        for (int k = 0; k < 4; ++k)
            if (noiseAfter.labels[k] == names[selected]) return k;

        return -1;
    }

    void redrawNoisePage() {

        if (!noisePage) return;

        noisePage->setReferenceText(beforeFrame ? beforeTitle : QString("—"));

        if (!rawImage || !noiseAfter.valid) {

            noisePage->plot->setEmptyText(rawImage ? "no pixels in the region"
                                                   : "open a RAW file to see noise histograms");
            noisePage->plot->setCurves({}, {}, QString(), QString(), QString());
            noisePage->clearSummary();
            return;
        }

        bool noiseMode = noisePage->noiseMode();
        int wanted = noiseChannel();

        auto pick = [&](const NoiseSnapshot& snapshot) {

            std::vector<uint32_t> curve;
            if (!snapshot.valid) return curve;

            const std::array<std::vector<uint32_t>, 4>& source =
                noiseMode ? snapshot.noise : snapshot.values;

            std::size_t length = source[0].size();
            curve.assign(length, 0u);

            for (int k = 0; k < 4; ++k) {

                if (wanted >= 0 && k != wanted) continue;
                if (source[k].size() != length) continue;

                for (std::size_t i = 0; i < length; ++i) curve[i] += source[k][i];
            }

            return curve;
        };

        auto pooledSigma = [&](const NoiseSnapshot& snapshot) {

            if (!snapshot.valid) return 0.0;
            if (wanted >= 0) return snapshot.sigma[wanted];

            double sum = 0.0;
            for (double sigma : snapshot.sigma) sum += sigma * sigma;
            return std::sqrt(sum / 4.0);
        };

        auto pooledMean = [&](const NoiseSnapshot& snapshot) {

            if (wanted >= 0) return snapshot.mean[wanted];
            return (snapshot.mean[0] + snapshot.mean[1] + snapshot.mean[2] + snapshot.mean[3]) / 4.0;
        };

        double sigmaWas = pooledSigma(noiseBefore);
        double sigmaNow = pooledSigma(noiseAfter);
        double change = sigmaWas > 0.0 ? (sigmaNow - sigmaWas) / sigmaWas * 100.0 : 0.0;
        double signal = pooledMean(noiseAfter) - rawImage->black_level;
        double snr = sigmaNow > 0.0 && signal > 0.0 ? signal / sigmaNow : 0.0;

        QRect region = noiseRegion();

        QString channelName = wanted >= 0 ? QString::fromStdString(noiseAfter.labels[wanted])
                                          : QString("ALL");

        if (noiseMode)
            noisePage->plot->setAxis(-noiseAfter.span, noiseAfter.span, "deviation from channel mean, ADU");
        else
            noisePage->plot->setAxis(noiseAfter.valueCenter - noiseAfter.valueSpan,
                                     noiseAfter.valueCenter + noiseAfter.valueSpan, "raw value, ADU");

        noisePage->plot->setMarkers(noiseMode ? sigmaWas : 0.0, noiseMode ? sigmaNow : 0.0);
        noisePage->plot->setEmptyText(QString());
        noisePage->plot->setCurves(pick(noiseBefore), pick(noiseAfter),
                                   "BEFORE  " + noiseBefore.title,
                                   "AFTER  " + noiseAfter.title,
                                   QString("%1   region [%2, %3]  %4 x %5")
                                       .arg(channelName).arg(region.x()).arg(region.y())
                                       .arg(region.width()).arg(region.height()));

        noisePage->setCards(sigmaWas, sigmaNow, change, snr);
        noisePage->setTable(noiseBefore, noiseAfter, rawImage->black_level, wanted);
    }

    QString denoiseLabel(const DenoiseParams& params) const {

        if (params.method == DenoiseMethod::Median)
            return QString("median %1x%1").arg(params.medianSize);

        if (params.method == DenoiseMethod::Bilateral)
            return QString("bilateral σ%1 range %2").arg(params.sigma, 0, 'f', 1)
                                                    .arg(params.rangeSigma, 0, 'f', 0);

        return QString("gaussian σ%1").arg(params.sigma, 0, 'f', 1);
    }

    void applyDenoise() {

        if (!rawImage) {
            QMessageBox::warning(this, "Error", "Upload RAW first");
            return;
        }

        if (busy) return;

        DenoiseParams params = noisePage->params();

        QRect area = noisePage->regionOnly() && !selectedRegion.isEmpty()
            ? selectedRegion : QRect(0, 0, rawImage->width, rawImage->height);

        QString label = denoiseLabel(params);
        auto source = std::make_shared<RawImage>(*rawImage);

        pendingFrame.reset();

        startTask(QString("Denoise (%1)...").arg(label), [this, source, params, area]() {

            bool finished = denoise_frame(*source, area.x(), area.y(), area.width(), area.height(),
                                          params, [this](int done, int total) {
                taskPercent = total > 0 ? static_cast<int>(static_cast<long long>(done) * 100 / total) : 0;
                return !cancelRequested.load();
            });

            if (!finished) throw std::runtime_error("cancelled");

            pendingFrame = std::move(*source);

        }, true, [this, label, area]() {

            if (!pendingFrame || !rawImage) return;

            autoCaptureBefore("before " + label);

            RawImage filtered = std::move(*pendingFrame);
            pendingFrame.reset();

            undoStack->push(new FrameCommand(this, *rawImage, std::move(filtered), "denoise " + label));

            logLine(QString("denoise %1 on [%2, %3] %4 x %5").arg(label)
                        .arg(area.x()).arg(area.y()).arg(area.width()).arg(area.height()));

            logNoiseChange();
        });
    }

    void logNoiseChange() {

        if (!noisePageShown() || !noiseBefore.valid || !noiseAfter.valid) return;

        QString text = "noise σ";

        for (int k = 0; k < 4; ++k)
            text += QString("   %1 %2 -> %3").arg(QString::fromStdString(noiseAfter.labels[k]))
                        .arg(noiseBefore.sigma[k], 0, 'f', 2).arg(noiseAfter.sigma[k], 0, 'f', 2);

        logLine(text);
    }

    void autoRangeSigma() {

        if (!rawImage) return;

        NoiseSnapshot current = noiseAfter.valid ? noiseAfter
                                                 : makeNoiseSnapshot(*rawImage, noiseRegion(), nullptr);
        if (!current.valid) return;

        double widest = 0.0;
        for (double sigma : current.sigma) widest = std::max(widest, sigma);

        noisePage->setRangeSigma(std::max(1.0, widest * 2.5));

        logLine(QString("bilateral range = 2.5 x σ %1 = %2 ADU")
                    .arg(widest, 0, 'f', 2).arg(widest * 2.5, 0, 'f', 0));
    }

    void setDemosaicMethod(DemosaicMethod method) {

        if (demosaicMethod == method) return;

        demosaicMethod = method;
        logLine(QString("demosaic method: %1").arg(demosaic_method_name(method)));
    }

    void toggleCompare(bool on) {

        compareEnabled = on;
        preview->setCompareEnabled(on);
        rebuildPreview();

        logLine(on ? "compare view on" : "compare view off");
    }

    void toggleFullScreen(bool on) {

        if (on) showFullScreen();
        else showNormal();
    }

    void togglePanels(bool hidden) {

        controlsDock->setVisible(!hidden);
        analysisDock->setVisible(!hidden);
        regionsDock->setVisible(!hidden);
        logDock->setVisible(!hidden);
        toolBar->setVisible(!hidden);
        statusBar()->setVisible(!hidden);
    }

    void copyViewToClipboard() {

        QApplication::clipboard()->setPixmap(preview->grab());
        statusBar()->showMessage("View copied to clipboard", 2500);
        logLine("view copied to clipboard");
    }

    void showAbout() {

        QString cudaVersion = QString("%1.%2")
                                  .arg(CUDART_VERSION / 1000)
                                  .arg((CUDART_VERSION % 1000) / 10);

        QMessageBox::about(this, "About Raw Image Process",
            QString("<b>Raw Image Process</b> %1<br><br>"
                    "Build date: %2<br>"
                    "Qt %3<br>"
                    "CUDA runtime %4<br>"
                    "LibRaw %5<br><br>"
                    "Device: %6")
                .arg(kAppVersion)
                .arg(__DATE__)
                .arg(QT_VERSION_STR)
                .arg(cudaVersion)
                .arg(LibRaw::version())
                .arg(QString::fromStdString(cuda_device_name())));
    }

    void updateCursorReadout(QPoint p) {

        QRect mapped = displayToRaw(QRect(p, QSize(1, 1)));

        QString text = QString("x %1  y %2").arg(mapped.x()).arg(mapped.y());

        if (rawImage && mapped.x() >= 0 && mapped.y() >= 0 &&
            mapped.x() < rawImage->width && mapped.y() < rawImage->height) {

            uint16_t value = rawImage->data[static_cast<std::size_t>(mapped.y())
                * rawImage->width + mapped.x()];

            std::array<std::string, 4> labels = cfa_labels(*rawImage);

            text += QString("  %1 %2")
                        .arg(QString::fromStdString(labels[cfa_index(mapped.x(), mapped.y())]))
                        .arg(value);
        }

        statusCursorLabel->setText(text);

        hudCursor = text;
        updateHud();
    }

    void updateStatusFile() {

        if (!rawImage) { statusFileLabel->setText("no file"); return; }

        QString text = QString("%1   %2x%3 (%7 MP)   CFA %4   black %5   white %6")
                           .arg(currentFile)
                           .arg(rawImage->width).arg(rawImage->height)
                           .arg(QString::fromStdString(cfa_pattern_name(*rawImage)))
                           .arg(rawImage->black_level)
                           .arg(rawImage->white_level)
                           .arg(rawImage->width * (double)rawImage->height / 1e6, 0, 'f', 1);

        if (refRaw) text += QString("   ref: %1x%2").arg(refRaw->width).arg(refRaw->height);

        statusFileLabel->setText(text);
    }

    void onLoad() {

        QString path = QFileDialog::getOpenFileName(this, "Choose RAW-file", lastDir);
        loadRawFile(path);
    }

    void loadRawFile(const QString& path) {

        if (path.isEmpty()) return;

        std::optional<RawImage> loaded = load_raw(path.toStdString());
        if (!loaded) {
            QMessageBox::warning(this, "Error",
                QString("Couldn't load\n%1").arg(QFileInfo(path).fileName()));
            return;
        }

        stashDocument();

        documents.push_back(Document());
        activeDocument = static_cast<int>(documents.size()) - 1;

        refRaw.reset();
        undoStack->clear();
        resetNoiseReference();

        rawImage = std::move(loaded);

        lastDir = QFileInfo(path).absolutePath();
        currentFile = QFileInfo(path).fileName();

        xSpin->setRange(0, rawImage->width - 1);
        ySpin->setRange(0, rawImage->height - 1);
        wSpin->setRange(1, rawImage->width);
        hSpin->setRange(1, rawImage->height);

        originalRaw = rawImage;

        linearResult.reset();
        resultImage.reset();
        selectedRegion = QRect();
        savedRegions.clear();
        measuredBlack = -1.0;

        regionCoordsLabel->setText("Region is not chosen");
        statsText->setText("—");
        noiseText->setText("—");
        obText->setText(rawImage->obData.empty() ? "no optical black area in this file"
                                                 : "not measured");
        histogram->clear();
        profile->clear();
        rebuildRegionTable();
        updateMetaInfo();

        setRotation(rotationFromFlip(rawImage->flip));
        updateStatusFile();
        titleBar->setFileName(currentFile);
        hudCursor.clear();
        updateHud();

        fileTabs->blockSignals(true);
        int tabIndex = fileTabs->addTab(currentFile);
        fileTabs->setCurrentIndex(tabIndex);
        fileTabs->blockSignals(false);
        fileTabs->setVisible(fileTabs->count() > 1);

        logLine(QString("loaded %1  %2x%3  active [%4,%5]")
                    .arg(currentFile).arg(rawImage->width).arg(rawImage->height)
                    .arg(rawImage->activeX).arg(rawImage->activeY));

        refreshNoisePage();
    }

    void stashDocument() {

        if (activeDocument < 0 || activeDocument >= static_cast<int>(documents.size())) return;

        Document& document = documents[activeDocument];

        document.raw = std::move(rawImage);
        document.original = std::move(originalRaw);
        document.reference = std::move(refRaw);
        document.linear = std::move(linearResult);
        document.result = std::move(resultImage);
        document.selected = selectedRegion;
        document.regions = savedRegions;
        document.rotation = viewRotation;
        document.measuredBlack = measuredBlack;
        document.file = currentFile;
        document.gamma = gammaSlider->value();
        document.zoom = zoomSlider->value();
    }

    void activateDocument(int index) {

        if (index < 0 || index >= static_cast<int>(documents.size())) return;
        if (index == activeDocument) return;

        stashDocument();
        activeDocument = index;

        Document& document = documents[index];

        rawImage = std::move(document.raw);
        originalRaw = std::move(document.original);
        refRaw = std::move(document.reference);
        linearResult = std::move(document.linear);
        resultImage = std::move(document.result);
        selectedRegion = document.selected;
        savedRegions = document.regions;
        measuredBlack = document.measuredBlack;
        currentFile = document.file;

        undoStack->clear();
        resetNoiseReference();

        gammaSlider->blockSignals(true);
        gammaSlider->setValue(document.gamma);
        gammaSlider->blockSignals(false);
        gammaValueLabel->setText(QString::number(document.gamma / 100.0, 'f', 2));
        pendingGammaValue = document.gamma;

        zoomSlider->blockSignals(true);
        zoomSlider->setValue(document.zoom);
        zoomSlider->blockSignals(false);
        zoomValueLabel->setText(QString::number(document.zoom / 100.0, 'f', 2) + "x");
        preview->setZoom(document.zoom / 100.0);

        if (rawImage) {
            xSpin->setRange(0, rawImage->width - 1);
            ySpin->setRange(0, rawImage->height - 1);
            wSpin->setRange(1, rawImage->width);
            hSpin->setRange(1, rawImage->height);
        }

        regionCoordsLabel->setText(selectedRegion.isEmpty()
            ? QString("Region is not chosen")
            : QString("[%1, %2]  %3x%4 px").arg(selectedRegion.x()).arg(selectedRegion.y())
                  .arg(selectedRegion.width()).arg(selectedRegion.height()));

        obText->setText(rawImage && !rawImage->obData.empty() ? "not measured"
                                                              : "no optical black area in this file");

        setRotation(document.rotation);
        refreshAnalysis();
        rebuildRegionTable();
        updateMetaInfo();
        updateStatusFile();
        titleBar->setFileName(currentFile);

        hudCursor.clear();
        updateHud();

        logLine("switched to " + currentFile);
        refreshNoisePage();
    }

    void closeDocument(int index) {

        if (index < 0 || index >= static_cast<int>(documents.size())) return;

        if (index == activeDocument) activeDocument = -1;
        else stashDocument();

        documents.erase(documents.begin() + index);

        fileTabs->blockSignals(true);
        fileTabs->removeTab(index);
        fileTabs->blockSignals(false);
        fileTabs->setVisible(fileTabs->count() > 1);

        if (documents.empty()) {
            resetToEmpty();
            return;
        }

        int next = std::min(index, static_cast<int>(documents.size()) - 1);

        fileTabs->blockSignals(true);
        fileTabs->setCurrentIndex(next);
        fileTabs->blockSignals(false);

        activeDocument = -1;
        activateDocument(next);
    }

    void resetToEmpty() {

        activeDocument = -1;

        rawImage.reset();
        originalRaw.reset();
        refRaw.reset();
        linearResult.reset();
        resultImage.reset();

        selectedRegion = QRect();
        savedRegions.clear();
        measuredBlack = -1.0;
        currentFile.clear();

        undoStack->clear();
        resetNoiseReference();

        preview->setCompareImage(QImage());
        preview->setBaseImage(QImage());

        histogram->clear();
        profile->clear();
        rebuildRegionTable();

        statsText->setText("—");
        noiseText->setText("—");
        obText->setText("—");
        metaText->setText("—");
        regionCoordsLabel->setText("Region is not chosen");

        titleBar->setFileName(QString());
        updateStatusFile();
        updateHud();

        logLine("all files closed");
        refreshNoisePage();
    }

    static int rotationFromFlip(int flip) {

        if (flip == 3) return 180;
        if (flip == 5) return 270;
        if (flip == 6) return 90;
        return 0;
    }

    void updateMetaInfo() {

        if (!rawImage) { metaText->setText("—"); return; }

        QString camera = QString::fromStdString(rawImage->camera).trimmed();
        QString lens = QString::fromStdString(rawImage->lens).trimmed();

        QString shutter = "-";
        if (rawImage->shutter > 0.f) {
            shutter = rawImage->shutter >= 1.f
                ? QString("%1 s").arg(rawImage->shutter, 0, 'f', 1)
                : QString("1/%1 s").arg(qRound(1.f / rawImage->shutter));
        }

        metaText->setText(QString("%1\n%2\nISO %3   %4   f/%5   %6 mm\nactive %7x%8 @ [%9,%10]   flip %11")
            .arg(camera.isEmpty() ? "-" : camera)
            .arg(lens.isEmpty() ? "-" : lens)
            .arg(rawImage->iso, 0, 'f', 0)
            .arg(shutter)
            .arg(rawImage->aperture, 0, 'f', 1)
            .arg(rawImage->focal, 0, 'f', 0)
            .arg(rawImage->width).arg(rawImage->height)
            .arg(rawImage->activeX).arg(rawImage->activeY)
            .arg(rawImage->flip));
    }

    void nudgeRegion(int dx, int dy) {

        if (!rawImage || selectedRegion.isEmpty()) return;

        int rawDx = dx;
        int rawDy = dy;

        if (viewRotation == 90) { rawDx = dy; rawDy = -dx; }
        else if (viewRotation == 180) { rawDx = -dx; rawDy = -dy; }
        else if (viewRotation == 270) { rawDx = -dy; rawDy = dx; }

        QRect moved = selectedRegion.translated(rawDx, rawDy);

        moved.moveLeft(std::clamp(moved.x(), 0, std::max(0, rawImage->width - moved.width())));
        moved.moveTop(std::clamp(moved.y(), 0, std::max(0, rawImage->height - moved.height())));

        onRegionSelected(moved);
    }

    void measureOpticalBlack() {

        if (!rawImage) {
            QMessageBox::warning(this, "Error", "Upload RAW first");
            return;
        }

        if (rawImage->obData.empty()) {
            obText->setText("no optical black area in this file");
            return;
        }

        RawImage ob;
        ob.width = rawImage->obWidth;
        ob.height = rawImage->obHeight;
        ob.cfaPattern = rawImage->cfaPattern;
        ob.black_level = rawImage->black_level;
        ob.white_level = rawImage->white_level;
        ob.data = rawImage->obData;

        ImageStats total = compute_region(ob, 0, 0, ob.width, ob.height);
        ChannelStats channels = compute_region_channels(ob, 0, 0, ob.width, ob.height);

        measuredBlack = total.mean;

        QString text = QString("%1x%2 px\nblack %3   read %4 ADU")
                           .arg(ob.width).arg(ob.height)
                           .arg(total.mean, 0, 'f', 2)
                           .arg(total.stdDev, 0, 'f', 2);

        for (int k = 0; k < 4; ++k) {

            if (channels.counts[k] == 0) continue;

            text += QString("\n%1 m=%2 s=%3")
                        .arg(QString::fromStdString(channels.labels[k]), -2)
                        .arg(channels.stats[k].mean, 0, 'f', 2)
                        .arg(channels.stats[k].stdDev, 0, 'f', 2);
        }

        double fullScale = static_cast<double>(std::max(1, rawImage->white_level - rawImage->black_level));
        double dynamicBits = total.stdDev > 0.0 ? std::log2(fullScale / total.stdDev) : 0.0;

        text += QString("\ndynamic range %1 bits (%2 dB)")
                    .arg(dynamicBits, 0, 'f', 2)
                    .arg(6.0206 * dynamicBits, 0, 'f', 1);

        obText->setText(text);

        logLine(QString("optical black: %1 ADU, read noise %2 ADU")
                    .arg(total.mean, 0, 'f', 2).arg(total.stdDev, 0, 'f', 2));
    }

    void applyMeasuredBlack() {

        if (!rawImage || measuredBlack < 0.0) {
            QMessageBox::warning(this, "Error", "Measure the optical black first");
            return;
        }

        autoCaptureBefore("before black level change");

        RawImage updated = *rawImage;
        updated.black_level = static_cast<int>(std::lround(measuredBlack));

        undoStack->push(new FrameCommand(this, *rawImage, updated, "black level"));

        logLine(QString("black level set to %1").arg(updated.black_level));
    }

    void revertOriginal() {

        if (!originalRaw) {
            QMessageBox::warning(this, "Error", "Nothing to revert");
            return;
        }

        undoStack->push(new FrameCommand(this, *rawImage, *originalRaw, "revert frame"));

        measuredBlack = -1.0;
        noiseText->setText("Reverted to original");
        logLine("reverted to the original frame");
    }

    void saveLinearTiff16() {

        if (!rawImage) {
            QMessageBox::warning(this, "Error", "Upload RAW first");
            return;
        }

        QString path = QFileDialog::getSaveFileName(this, "Save linear TIFF",
                            lastDir.isEmpty() ? QString("linear16.tiff") : lastDir + "/linear16.tiff",
                            "TIFF (*.tiff)");
        if (path.isEmpty()) return;

        auto source = std::make_shared<RawImage>(*rawImage);
        std::string target = path.toStdString();

        startTask("Writing 16-bit TIFF...", [source, target]() {

            Image16 linear = demosaic_cpu16(*source);

            if (!save_rgb_tiff16(target, linear))
                throw std::runtime_error("Couldn't write the TIFF file");
        });

        logLine("16-bit TIFF: " + QFileInfo(path).fileName());
    }

    void addRegion() {

        if (!rawImage || selectedRegion.isEmpty()) {
            QMessageBox::warning(this, "Error", "Choose a region first");
            return;
        }

        QVector<QRect> next = savedRegions;
        next.append(selectedRegion);

        undoStack->push(new RegionsCommand(this, savedRegions, next, "add region"));

        logLine(QString("region %1 added: [%2, %3] %4x%5")
                    .arg(next.size())
                    .arg(selectedRegion.x()).arg(selectedRegion.y())
                    .arg(selectedRegion.width()).arg(selectedRegion.height()));
    }

    void removeRegion() {

        int row = regionTable->currentRow();
        if (row < 0 || row >= savedRegions.size()) return;

        QVector<QRect> next = savedRegions;
        next.remove(row);

        undoStack->push(new RegionsCommand(this, savedRegions, next, "remove region"));
        logLine(QString("region %1 removed").arg(row + 1));
    }

    void clearRegions() {

        if (savedRegions.isEmpty()) return;

        undoStack->push(new RegionsCommand(this, savedRegions, QVector<QRect>(), "clear regions"));
        logLine("region list cleared");
    }

    void rebuildRegionTable() {

        regionTable->blockSignals(true);
        regionTable->setRowCount(savedRegions.size());

        for (int row = 0; row < savedRegions.size(); ++row) {

            const QRect& region = savedRegions[row];

            ImageStats stats;
            if (rawImage)
                stats = compute_region(*rawImage, region.x(), region.y(),
                                       region.width(), region.height());

            const QString values[7] = {
                QString::number(row + 1),
                QString::number(region.x()), QString::number(region.y()),
                QString::number(region.width()), QString::number(region.height()),
                QString::number(stats.mean, 'f', 1),
                QString::number(stats.stdDev, 'f', 2)
            };

            for (int col = 0; col < 7; ++col) {

                auto* item = new QTableWidgetItem(values[col]);
                item->setTextAlignment(Qt::AlignCenter);

                if (col == 0) item->setForeground(ImageLabel::markerColor(row));

                regionTable->setItem(row, col, item);
            }
        }

        regionTable->blockSignals(false);
        syncPreviewRegions();
    }

    void writeCsvMeta(QTextStream& out) const {

        out << "file,camera,lens,iso,shutter,aperture,focal,black,white,active_x,active_y,width,height\n";
        out << currentFile << ",\"" << QString::fromStdString(rawImage->camera).trimmed()
            << "\",\"" << QString::fromStdString(rawImage->lens).trimmed() << "\","
            << QString::number(rawImage->iso, 'f', 0) << ","
            << QString::number(rawImage->shutter, 'f', 6) << ","
            << QString::number(rawImage->aperture, 'f', 2) << ","
            << QString::number(rawImage->focal, 'f', 1) << ","
            << rawImage->black_level << "," << rawImage->white_level << ","
            << rawImage->activeX << "," << rawImage->activeY << ","
            << rawImage->width << "," << rawImage->height << "\n\n";
    }

    void exportRegions() {

        if (!rawImage || savedRegions.isEmpty()) {
            QMessageBox::warning(this, "Error", "The region list is empty");
            return;
        }

        QString path = QFileDialog::getSaveFileName(this, "Save regions",
                            lastDir.isEmpty() ? QString("regions.csv") : lastDir + "/regions.csv",
                            "CSV (*.csv)");
        if (path.isEmpty()) return;

        QFile file(path);
        if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QMessageBox::critical(this, "Error", "Couldn't write the file");
            return;
        }

        QTextStream out(&file);
        writeCsvMeta(out);

        out << "region,x,y,w,h,channel,count,min,max,mean,rms,range_bits,useful_bits,snr_db\n";

        for (int i = 0; i < savedRegions.size(); ++i) {

            const QRect& region = savedRegions[i];

            ImageStats total = compute_region(*rawImage, region.x(), region.y(),
                                              region.width(), region.height());

            ChannelStats channels = compute_region_channels(*rawImage, region.x(), region.y(),
                                                            region.width(), region.height());

            QString prefix = QString("%1,%2,%3,%4,%5,")
                                 .arg(i + 1).arg(region.x()).arg(region.y())
                                 .arg(region.width()).arg(region.height());

            out << prefix << "ALL," << (qint64)region.width() * region.height() << ","
                << total.min << "," << total.max << ","
                << QString::number(total.mean, 'f', 4) << ","
                << QString::number(total.stdDev, 'f', 4) << ","
                << QString::number(total.rangeBits, 'f', 4) << ","
                << QString::number(total.usefulBits, 'f', 4) << ","
                << QString::number(total.snrDb, 'f', 4) << "\n";

            for (int k = 0; k < 4; ++k) {

                if (channels.counts[k] == 0) continue;

                const ImageStats& stats = channels.stats[k];

                out << prefix << QString::fromStdString(channels.labels[k]) << ","
                    << (qint64)channels.counts[k] << ","
                    << stats.min << "," << stats.max << ","
                    << QString::number(stats.mean, 'f', 4) << ","
                    << QString::number(stats.stdDev, 'f', 4) << ","
                    << QString::number(stats.rangeBits, 'f', 4) << ","
                    << QString::number(stats.usefulBits, 'f', 4) << ","
                    << QString::number(stats.snrDb, 'f', 4) << "\n";
            }
        }

        file.close();

        statusBar()->showMessage("Regions exported", 3000);
        logLine(QString("%1 regions exported to %2").arg(savedRegions.size())
                    .arg(QFileInfo(path).fileName()));
    }

    void savePreset() {

        QString path = QFileDialog::getSaveFileName(this, "Save preset",
                            lastDir.isEmpty() ? QString("preset.json") : lastDir + "/preset.json",
                            "JSON (*.json)");
        if (path.isEmpty()) return;

        QJsonObject root;
        root["gamma"] = gammaSlider->value();
        root["backend"] = gpu_btn->isChecked() ? "GPU" : "CPU";
        root["grid"] = gridBtn->isChecked();
        root["clipping"] = clipBtn->isChecked();
        root["rotation"] = viewRotation;
        root["zoom"] = zoomSlider->value();

        QJsonArray regions;

        for (const QRect& region : savedRegions) {

            QJsonObject item;
            item["x"] = region.x();
            item["y"] = region.y();
            item["w"] = region.width();
            item["h"] = region.height();
            regions.append(item);
        }

        root["regions"] = regions;

        if (!selectedRegion.isEmpty()) {

            QJsonObject item;
            item["x"] = selectedRegion.x();
            item["y"] = selectedRegion.y();
            item["w"] = selectedRegion.width();
            item["h"] = selectedRegion.height();
            root["selected"] = item;
        }

        QFile file(path);
        if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QMessageBox::critical(this, "Error", "Couldn't write the file");
            return;
        }

        file.write(QJsonDocument(root).toJson(QJsonDocument::Indented));
        file.close();

        logLine("preset saved: " + QFileInfo(path).fileName());
    }

    void loadPreset() {

        QString path = QFileDialog::getOpenFileName(this, "Load preset", lastDir, "JSON (*.json)");
        if (path.isEmpty()) return;

        QFile file(path);
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QMessageBox::critical(this, "Error", "Couldn't read the file");
            return;
        }

        QJsonDocument document = QJsonDocument::fromJson(file.readAll());
        file.close();

        if (!document.isObject()) {
            QMessageBox::warning(this, "Error", "This is not a preset file");
            return;
        }

        QJsonObject root = document.object();

        gammaSlider->setValue(root.value("gamma").toInt(220));

        if (root.value("backend").toString() == "GPU" && gpuAvailable) gpu_btn->setChecked(true);
        else cpu_btn->setChecked(true);

        gridBtn->setChecked(root.value("grid").toBool());
        clipBtn->setChecked(root.value("clipping").toBool());
        zoomSlider->setValue(root.value("zoom").toInt(100));
        setRotation(root.value("rotation").toInt(0));

        savedRegions.clear();

        for (const QJsonValue& value : root.value("regions").toArray()) {

            QJsonObject item = value.toObject();
            savedRegions.append(QRect(item.value("x").toInt(), item.value("y").toInt(),
                                      item.value("w").toInt(), item.value("h").toInt()));
        }

        rebuildRegionTable();

        QJsonObject selected = root.value("selected").toObject();

        if (!selected.isEmpty())
            onRegionSelected(QRect(selected.value("x").toInt(), selected.value("y").toInt(),
                                   selected.value("w").toInt(), selected.value("h").toInt()));

        logLine("preset loaded: " + QFileInfo(path).fileName());
    }

    void onLoadReference() {

        QString path = QFileDialog::getOpenFileName(this, "Choose reference RAW-file", lastDir);
        if (path.isEmpty()) return;

        std::optional<RawImage> loaded = load_raw(path.toStdString());
        if (!loaded) {
            QMessageBox::warning(this, "Error", "Couldn't load reference file");
            return;
        }

        if (rawImage && (loaded->width != rawImage->width || loaded->height != rawImage->height)) {
            QMessageBox::warning(this, "Error", "Reference frame has a different size");
            return;
        }

        refRaw = std::move(loaded);
        lastDir = QFileInfo(path).absolutePath();

        noiseText->setText("Reference loaded");
        updateStatusFile();
        logLine("reference loaded: " + QFileInfo(path).fileName());
    }

    void onProcess() {

        if (!rawImage) {
            QMessageBox::warning(this, "Error", "Download RAW-file first");
            return;
        }

        auto source = std::make_shared<RawImage>(*rawImage);
        Backend backend = currentBack();

        DemosaicMethod method = demosaicMethod;

        startTask(QString("Demosaic (%1)...").arg(demosaic_method_name(method)),
                  [this, source, backend, method]() {

            Image image = process_demosaic(*source, backend, method, [this](int done, int total) {
                taskPercent = total > 0 ? done * 100 / total : 0;
                return !cancelRequested.load();
            });

            if (image.data.empty()) throw std::runtime_error("cancelled");
            linearResult = std::move(image);

        }, true);
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

        auto source = std::make_shared<RawImage>(
            region_crop(*rawImage, selectedRegion.x(), selectedRegion.y(),
                        selectedRegion.width(), selectedRegion.height()));

        Backend backend = currentBack();

        DemosaicMethod method = demosaicMethod;

        startTask(QString("Demosaic of region (%1)...").arg(demosaic_method_name(method)),
                  [this, source, backend, method]() {

            Image image = process_demosaic(*source, backend, method, [this](int done, int total) {
                taskPercent = total > 0 ? done * 100 / total : 0;
                return !cancelRequested.load();
            });

            if (image.data.empty()) throw std::runtime_error("cancelled");
            linearResult = std::move(image);

        }, true);
    }

    void onGammaChanged(int sliderValue) {

        float v = sliderValue / 100.f;
        gammaValueLabel->setText(QString::number(v, 'f', 2));
        pendingGammaValue = sliderValue;
        gammaTimer->start();
    }

    void applyPendingGamma() {

        if (busy || !linearResult) return;

        float gamma = pendingGammaValue / 100.f;
        resultImage = apply_gamma(*linearResult, gamma, currentBack());
        rebuildPreview();
    }

    void onRegionSelected(QRect regionInRawCoords) {

        if (!rawImage || regionInRawCoords.isEmpty()) {
            regionCoordsLabel->setText("Region has not been chosen");
            preview->setSelectionInImageCoords(QRect());
            histogram->clear();
            profile->clear();
            return;
        }

        int x0 = std::clamp(regionInRawCoords.x(), 0, rawImage->width - 1);
        int y0 = std::clamp(regionInRawCoords.y(), 0, rawImage->height - 1);

        int w = std::min(regionInRawCoords.width(), rawImage->width - x0);
        int h = std::min(regionInRawCoords.height(), rawImage->height - y0);
        if (w <= 0 || h <= 0) return;

        selectedRegion = QRect(x0, y0, w, h);
        syncPreviewRegions();

        xSpin->blockSignals(true);  ySpin->blockSignals(true);
        wSpin->blockSignals(true);  hSpin->blockSignals(true);
        xSpin->setValue(x0);  ySpin->setValue(y0);
        wSpin->setValue(w);   hSpin->setValue(h);
        xSpin->blockSignals(false); ySpin->blockSignals(false);
        wSpin->blockSignals(false); hSpin->blockSignals(false);

        regionCoordsLabel->setText(QString("[%1, %2]  %3x%4 px").arg(x0).arg(y0).arg(w).arg(h));

        refreshAnalysis();
    }

    void refreshAnalysis() {

        if (!rawImage || selectedRegion.isEmpty()) {
            statsText->setText("—");
            histogram->clear();
            profile->clear();
            refreshNoisePage();
            return;
        }

        int x0 = selectedRegion.x(), y0 = selectedRegion.y();
        int w = selectedRegion.width(), h = selectedRegion.height();

        ImageStats total = compute_region(*rawImage, x0, y0, w, h);
        ChannelStats channels = compute_region_channels(*rawImage, x0, y0, w, h);

        auto describe = [](const QString& name, const ImageStats& s) {
            return QString("%1 %2..%3 m=%4 s=%5\n    range %6  useful %7  SNR %8 dB")
                       .arg(name, -3)
                       .arg(s.min).arg(s.max)
                       .arg(s.mean, 0, 'f', 1)
                       .arg(s.stdDev, 0, 'f', 2)
                       .arg(s.rangeBits, 0, 'f', 2)
                       .arg(s.usefulBits, 0, 'f', 2)
                       .arg(s.snrDb, 0, 'f', 1);
        };

        QString text = describe("ALL", total);

        for (int k = 0; k < 4; ++k) {

            if (channels.counts[k] == 0) continue;

            text += "\n" + describe(QString::fromStdString(channels.labels[k]), channels.stats[k]);
        }

        statsText->setText(text);

        histogram->setData(compute_histogram(*rawImage, x0, y0, w, h, kHistogramBins),
                           channels.labels);

        bool alongX = (w > 1);

        std::vector<double> values = compute_profile(*rawImage, x0, y0, w, h, alongX);

        QString caption = alongX
            ? QString("mean per column, x = %1..%2").arg(x0).arg(x0 + w - 1)
            : QString("mean per row, y = %1..%2").arg(y0).arg(y0 + h - 1);

        profile->setData(values, caption);
        refreshNoisePage();
    }

    void applyNumericRegion() {

        if (!rawImage) {
            QMessageBox::warning(this, "Error", "Upload RAW first");
            return;
        }
        onRegionSelected(QRect(xSpin->value(), ySpin->value(),
        wSpin->value(), hSpin->value()));

        preview->focusOnRegion(rawToDisplay(selectedRegion));
    }

    void onSelectedRow() {

        if (!rawImage) {
            QMessageBox::warning(this, "Error", "Upload RAW first");
            return;
        }

        onRegionSelected(QRect(0, ySpin->value(), rawImage->width, 1));
        preview->focusOnRegion(rawToDisplay(selectedRegion));
    }

    void onSelectWholeColumn() {

        if (!rawImage) {
            QMessageBox::warning(this, "Error", "Upload RAW first");
            return;
        }

        onRegionSelected(QRect(xSpin->value(), 0, 1, rawImage->height));
        preview->focusOnRegion(rawToDisplay(selectedRegion));
    }

    void compareFrames() {

        if (!rawImage || !refRaw) {
            QMessageBox::warning(this, "Error", "Load both RAW and reference RAW first");
            return;
        }

        QRect region = selectedRegion.isEmpty()
            ? QRect(0, 0, rawImage->width, rawImage->height)
            : selectedRegion;

        NoiseReport report = compute_two_frame(*rawImage, *refRaw, region.x(), region.y(),
                                               region.width(), region.height());

        if (!report.valid) {
            QMessageBox::warning(this, "Error", "Frames have different geometry");
            return;
        }

        QString text = QString("region %1x%2").arg(region.width()).arg(region.height());

        for (int k = 0; k < 4; ++k) {

            text += QString("\n%1 rd=%2 fpn=%3 tot=%4")
                        .arg(QString::fromStdString(report.labels[k]), -2)
                        .arg(report.temporal[k], 0, 'f', 2)
                        .arg(report.fpn[k], 0, 'f', 2)
                        .arg(report.total[k], 0, 'f', 2);
        }

        noiseText->setText(text);
    }

    void subtractReference() {

        if (!rawImage || !refRaw) {
            QMessageBox::warning(this, "Error", "Load both RAW and reference RAW first");
            return;
        }

        if (rawImage->width != refRaw->width || rawImage->height != refRaw->height) {
            QMessageBox::warning(this, "Error", "Frames have different geometry");
            return;
        }

        autoCaptureBefore("before dark subtraction");

        undoStack->push(new FrameCommand(this, *rawImage,
                                         subtract_dark(*rawImage, *refRaw), "dark subtraction"));

        noiseText->setText("Dark frame subtracted");
        logLine("dark frame subtracted");
    }

    void exportStats() {

        if (!rawImage || selectedRegion.isEmpty()) {
            QMessageBox::warning(this, "Error", "Choose a region first");
            return;
        }

        QString path = QFileDialog::getSaveFileName(this, "Save stats",
                            lastDir.isEmpty() ? QString("stats.csv") : lastDir + "/stats.csv",
                            "CSV (*.csv)");
        if (path.isEmpty()) return;

        QFile file(path);
        if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QMessageBox::critical(this, "Error", "Couldn't write the file");
            return;
        }

        int x0 = selectedRegion.x(), y0 = selectedRegion.y();
        int w = selectedRegion.width(), h = selectedRegion.height();

        ImageStats total = compute_region(*rawImage, x0, y0, w, h);
        ChannelStats channels = compute_region_channels(*rawImage, x0, y0, w, h);

        QTextStream out(&file);
        writeCsvMeta(out);

        out << "region_x,region_y,region_w,region_h\n";
        out << x0 << "," << y0 << "," << w << "," << h << "\n\n";

        out << "channel,count,min,max,mean,rms,range_bits,useful_bits,snr_db\n";
        out << "ALL," << (qint64)w * h << "," << total.min << "," << total.max << ","
            << QString::number(total.mean, 'f', 4) << ","
            << QString::number(total.stdDev, 'f', 4) << ","
            << QString::number(total.rangeBits, 'f', 4) << ","
            << QString::number(total.usefulBits, 'f', 4) << ","
            << QString::number(total.snrDb, 'f', 4) << "\n";

        for (int k = 0; k < 4; ++k) {

            if (channels.counts[k] == 0) continue;

            const ImageStats& s = channels.stats[k];

            out << QString::fromStdString(channels.labels[k]) << ","
                << (qint64)channels.counts[k] << "," << s.min << "," << s.max << ","
                << QString::number(s.mean, 'f', 4) << ","
                << QString::number(s.stdDev, 'f', 4) << ","
                << QString::number(s.rangeBits, 'f', 4) << ","
                << QString::number(s.usefulBits, 'f', 4) << ","
                << QString::number(s.snrDb, 'f', 4) << "\n";
        }

        file.close();
        statusBar()->showMessage("Stats exported", 3000);
    }

    QString exportFolder(const QString& path) {

        QFileInfo info(path);
        QDir dir(info.absolutePath());
        QString name = info.completeBaseName();

        if (!dir.mkpath(name)) return QString();

        return dir.absoluteFilePath(name + "/" + info.fileName());
    }

    bool writeExportNotes(const QString& path, const RawImage& frame, bool readbin) {

        QFileInfo info(path);
        QString suffix = readbin ? "_mathcad.txt" : "_info.txt";
        QString target = info.absolutePath() + "/" + info.completeBaseName() + suffix;
        QString source = readbin ? QString(path).replace('\\', '/') : QString();

        return save_raw_notes(target.toStdString(), frame, cfa_pattern_name(frame),
                              source.toStdString());
    }

    void saveBin() {
        if (!rawImage) {
            QMessageBox::warning(this, "Error", "Upload RAW first");
            return;
        }

        QString suggested = QString("raw_%1x%2_uint16.bin").arg(rawImage->width).arg(rawImage->height);
        QString start = lastDir.isEmpty() ? suggested : lastDir + "/" + suggested;

        QString chosen = QFileDialog::getSaveFileName(this, "Save bin", start, "Binary (*.bin)");
        if (chosen.isEmpty()) return;

        QString path = exportFolder(chosen);

        if (path.isEmpty()) {
            QMessageBox::critical(this, "Error", "Couldn't create the folder");
            return;
        }

        if (!save_raw_binary(path.toStdString(), *rawImage)) {
            QMessageBox::critical(this, "Error", "Couldn't save binary");
            return;
        }

        QFileInfo info(path);
        bool metaSaved = writeExportNotes(path, *rawImage, true);

        logLine(QString("saved %1/%2  %3x%4 uint16  %5 bytes")
                    .arg(info.dir().dirName()).arg(info.fileName())
                    .arg(rawImage->width).arg(rawImage->height)
                    .arg(static_cast<qint64>(rawImage->size())));

        QMessageBox::information(this, "Ready",
            QString("Folder %1\n\n%2 x %3 pixels, uint16, little-endian, %4 bytes.\n"
                    "Read the file with exactly these sizes, otherwise the picture is skewed.\n\n%5")
                .arg(info.dir().dirName())
                .arg(rawImage->width).arg(rawImage->height)
                .arg(static_cast<qint64>(rawImage->size()))
                .arg(metaSaved ? QString("Sizes, levels and the ready Mathcad line are in %1_mathcad.txt").arg(info.completeBaseName())
                               : QString("Couldn't write the notes file")));
    }

    void saveRegionBin() {

        if (!rawImage) {
            QMessageBox::warning(this, "Error", "Upload RAW first");
            return;
        }

        if (selectedRegion.isEmpty()) {
            QMessageBox::warning(this, "Error", "Choose a region first");
            return;
        }

        RawImage part = region_crop(*rawImage, selectedRegion.x(), selectedRegion.y(),
                                    selectedRegion.width(), selectedRegion.height());

        QString suggested = QString("region_%1x%2_uint16.bin").arg(part.width).arg(part.height);
        QString start = lastDir.isEmpty() ? suggested : lastDir + "/" + suggested;

        QString chosen = QFileDialog::getSaveFileName(this, "Save region bin", start, "Binary (*.bin)");
        if (chosen.isEmpty()) return;

        QString path = exportFolder(chosen);

        if (path.isEmpty()) {
            QMessageBox::critical(this, "Error", "Couldn't create the folder");
            return;
        }

        if (!save_raw_binary(path.toStdString(), part)) {
            QMessageBox::critical(this, "Error", "Couldn't save binary");
            return;
        }

        QFileInfo info(path);
        bool metaSaved = writeExportNotes(path, part, true);

        logLine(QString("saved %1/%2  region [%3, %4]  %5x%6 uint16  %7 bytes")
                    .arg(info.dir().dirName()).arg(info.fileName())
                    .arg(selectedRegion.x()).arg(selectedRegion.y())
                    .arg(part.width).arg(part.height)
                    .arg(static_cast<qint64>(part.size())));

        QMessageBox::information(this, "Ready",
            QString("Folder %1\n\n%2 x %3 numbers, uint16, little-endian, %4 bytes.\n"
                    "Read the file with exactly these sizes, otherwise the picture is skewed.\n\n%5")
                .arg(info.dir().dirName())
                .arg(part.width).arg(part.height)
                .arg(static_cast<qint64>(part.size()))
                .arg(metaSaved ? QString("Sizes, levels and the ready Mathcad line are in %1_mathcad.txt").arg(info.completeBaseName())
                               : QString("Couldn't write the notes file")));
    }

    void saveTiff() {
        if (!rawImage) {
            QMessageBox::warning(this, "Error", "Upload RAW first");
            return;
        }

        QString start = lastDir.isEmpty() ? QString("raw_mosaic.tiff") : lastDir + "/raw_mosaic.tiff";

        QString chosen = QFileDialog::getSaveFileName(this, "Save TIFF", start, "TIFF (*.tiff)");
        if (chosen.isEmpty()) return;

        QString path = exportFolder(chosen);

        if (path.isEmpty()) {
            QMessageBox::critical(this, "Error", "Couldn't create the folder");
            return;
        }

        if (!save_raw_in_tiff(path.toStdString(), *rawImage)) {
            QMessageBox::critical(this, "Error", "Couldn't save TIFF");
            return;
        }

        QFileInfo info(path);
        writeExportNotes(path, *rawImage, false);

        logLine(QString("saved %1/%2  %3x%4 uint16 tiff")
                    .arg(info.dir().dirName()).arg(info.fileName())
                    .arg(rawImage->width).arg(rawImage->height));
    }

    void saveRegionTiff() {

        if (!rawImage) {
            QMessageBox::warning(this, "Error", "Upload RAW first");
            return;
        }

        if (selectedRegion.isEmpty()) {
            QMessageBox::warning(this, "Error", "Choose a region first");
            return;
        }

        RawImage part = region_crop(*rawImage, selectedRegion.x(), selectedRegion.y(),
                                    selectedRegion.width(), selectedRegion.height());

        QString suggested = QString("region_%1x%2_uint16.tiff").arg(part.width).arg(part.height);
        QString start = lastDir.isEmpty() ? suggested : lastDir + "/" + suggested;

        QString chosen = QFileDialog::getSaveFileName(this, "Save region TIFF", start, "TIFF (*.tiff)");
        if (chosen.isEmpty()) return;

        QString path = exportFolder(chosen);

        if (path.isEmpty()) {
            QMessageBox::critical(this, "Error", "Couldn't create the folder");
            return;
        }

        if (!save_raw_in_tiff(path.toStdString(), part)) {
            QMessageBox::critical(this, "Error", "Couldn't save TIFF");
            return;
        }

        QFileInfo info(path);
        bool metaSaved = writeExportNotes(path, part, false);

        logLine(QString("saved %1/%2  region [%3, %4]  %5x%6 uint16 tiff")
                    .arg(info.dir().dirName()).arg(info.fileName())
                    .arg(selectedRegion.x()).arg(selectedRegion.y())
                    .arg(part.width).arg(part.height));

        QMessageBox::information(this, "Ready",
            QString("Folder %1\n\n%2 x %3 pixels, 16-bit grayscale mosaic without processing.\n\n%4")
                .arg(info.dir().dirName())
                .arg(part.width).arg(part.height)
                .arg(metaSaved ? QString("Sizes, levels and CFA pattern are in %1_info.txt").arg(info.completeBaseName())
                               : QString("Couldn't write the notes file")));
    }

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

    void loadSettings() {

        QSettings settings;

        lastDir = settings.value("lastDir").toString();

        int gamma = settings.value("gamma", 220).toInt();
        gammaSlider->setValue(std::clamp(gamma, 10, 300));

        if (settings.value("backend").toString() == "GPU" && gpuAvailable) gpu_btn->setChecked(true);
        else cpu_btn->setChecked(true);

        gridBtn->setChecked(settings.value("grid", false).toBool());
        clipBtn->setChecked(settings.value("clipping", false).toBool());

        int storedMethod = settings.value("demosaicMethod", 0).toInt();

        if (storedMethod == 1) bilinearBtn->setChecked(true);
        else if (storedMethod == 2) malvarBtn->setChecked(true);
        else binningBtn->setChecked(true);

        noisePage->loadFrom(settings);

        QByteArray geometry = settings.value("geometry").toByteArray();
        if (!geometry.isEmpty()) restoreGeometry(geometry);

        QByteArray dockState = settings.value("dockState").toByteArray();
        if (!dockState.isEmpty()) restoreState(dockState);
    }

    void saveSettings() {

        QSettings settings;

        settings.setValue("lastDir", lastDir);
        settings.setValue("gamma", gammaSlider->value());
        settings.setValue("backend", gpu_btn->isChecked() ? "GPU" : "CPU");
        settings.setValue("grid", gridBtn->isChecked());
        settings.setValue("clipping", clipBtn->isChecked());
        settings.setValue("demosaicMethod", static_cast<int>(demosaicMethod));
        noisePage->saveTo(settings);
        settings.setValue("geometry", saveGeometry());
        settings.setValue("dockState", saveState());
    }
};


void RegionsCommand::undo() { owner->applyRegionList(previous); }
void RegionsCommand::redo() { owner->applyRegionList(next); }

void FrameCommand::undo() { owner->applyRawFrame(previous); }
void FrameCommand::redo() { owner->applyRawFrame(next); }


int main(int argc, char** argv) {
    QApplication app(argc, argv);
    QApplication::setStyle(QStyleFactory::create("Fusion"));
    QApplication::setOrganizationName("RawImageProcess");
    QApplication::setApplicationName("RawImageProcess");

    MainWindow window;
    window.show();

    QStringList arguments = QApplication::arguments();
    for (int i = 1; i < arguments.size(); ++i) window.openPath(arguments.at(i));

    return app.exec();
}
