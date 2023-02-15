#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_CBR.h"

class CBR : public QMainWindow
{
    Q_OBJECT

public:
    CBR(QWidget *parent = nullptr);
    ~CBR();
    void extractArchive();

private:
    Ui::CBRClass ui;
};
