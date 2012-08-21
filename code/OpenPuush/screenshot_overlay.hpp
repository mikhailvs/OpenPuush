#ifndef SCREENSHOT_OVERLAY_HPP
#define SCREENSHOT_OVERLAY_HPP

#include <QMainWindow>

class QKeyEvent;
class QMouseEvent;
class QPaintEvent;
class QShowEvent;
class QTimer;

class screenshot_overlay : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit screenshot_overlay(QWidget *parent = 0);
    ~screenshot_overlay();
    
private:
    QWidget * selected_area;
    bool grabbing_window;
    bool taking_ss;

    QPoint initial_selected;
    QPoint current_selected;

    void update_selected();

private slots:
    void keyPressEvent(QKeyEvent *);
    void mousePressEvent(QMouseEvent *);
    void mouseMoveEvent(QMouseEvent *);
    void mouseReleaseEvent(QMouseEvent *);
    void showEvent(QShowEvent *);

    void get_screenshot();

signals:
    void got_screenshot(QPixmap);
};

#endif // SCREENSHOT_OVERLAY_HPP
