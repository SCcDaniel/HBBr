#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_EditorMain.h"

class EditorMain : public QMainWindow
{
    Q_OBJECT

public:
    EditorMain(QWidget *parent = nullptr);
    ~EditorMain();

    /* ����Ⱦ���� */
    class RenderView* _mainRenderView;

private:
    Ui::EditorMainClass ui;
};
