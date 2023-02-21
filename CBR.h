#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_CBR.h"
#include "currentView.h"
#include <QMouseEvent>
#include <QGraphicsView>
#include <QPoint>
#include <QEvent>
#include <QGraphicsSceneMouseEvent>

class CBR : public QMainWindow
{
    Q_OBJECT

public:
    CBR(QWidget *parent = nullptr);
    ~CBR();
    void extractArchive();
    void PageSuivante();
    void PagePrecedante();
    void createArchive();
private slots:
    void onViewDoubleClicked(QMouseEvent* event);

protected:
    bool eventFilter(QObject* obj, QEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;


private:
    Ui::CBRClass ui;
    QPoint m_lastMousePos;
    bool m_dragging;
public:
    currentView v;

};
