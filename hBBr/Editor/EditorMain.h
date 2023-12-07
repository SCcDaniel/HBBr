#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_EditorMain.h"
#include <qdockwidget.h>
#include <qtimer.h>
#include <qthread.h>

class EditorMain : public QMainWindow
{
    Q_OBJECT

public:

    static  EditorMain* _self;

    EditorMain(QWidget *parent = nullptr);
    ~EditorMain();

    /* ����Ⱦ���� */
    class RenderView* _mainRenderView;

    //�������
    class SceneOutline* _sceneOutline = nullptr;
    class QDockWidget* _sceneOutline_dock = nullptr;

    //���ݹ�����
    class ContentBrowser* _contentBrowser = nullptr;
    class QDockWidget* _contentBrowser_dock = nullptr;

    //�����/�����༭��
    class Inspector* _inspector = nullptr;
    class QDockWidget* _inspector_dock = nullptr;

    virtual void closeEvent(QCloseEvent* event);

    virtual void resizeEvent(QResizeEvent* event)override;

    virtual void showEvent(QShowEvent* event)override;

    virtual void focusInEvent(QFocusEvent* event);

    virtual void focusOutEvent(QFocusEvent* event);

private:

    Ui::EditorMainClass ui;

    QTimer* _renderTimer;

private slots:

    void UpdateRender();
};

