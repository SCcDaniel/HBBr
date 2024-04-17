#include "EditorMain.h"
#include <QtWidgets/QApplication>
#include <QMouseEvent>
#include <qdebug.h>
#include "FormMain.h"
#include "RenderView.h"
#include "HInput.h"
#include "ContentBrowser.h"
#include <Windows.h> //Ϊ��֧��SetFocus(nullptr);

QWidget* currentFocusWidget = nullptr;
class MyEventFilter : public QObject
{
protected:
    bool eventFilter(QObject* watched, QEvent* event) override
    {
        QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(event);
        bool bChangeFocus = false;
        if (event->type() == QEvent::MouseButtonPress) 
        {
            QWidget* newCurrentFocusWidget = QApplication::widgetAt(mouseEvent->globalPos());
            if (currentFocusWidget && currentFocusWidget != newCurrentFocusWidget && currentFocusWidget != QApplication::focusWidget())
            {
                if (
                    !currentFocusWidget->objectName().contains("menu", Qt::CaseInsensitive)//
                    && !currentFocusWidget->objectName().contains("combo", Qt::CaseInsensitive)//
                    && !currentFocusWidget->objectName().contains("lineEdit", Qt::CaseInsensitive)
                    )
                    bChangeFocus = true;
                else if (currentFocusWidget->parent() || currentFocusWidget->parent()->parent())
                {
                    if(!currentFocusWidget->parent()->objectName().contains("combo", Qt::CaseInsensitive))
                        bChangeFocus = true;
                    if (!currentFocusWidget->parent()->parent()->objectName().contains("combo", Qt::CaseInsensitive))
                        bChangeFocus = true;
                }
                if (bChangeFocus)
                {
                    //ȡ�����н�����
                    SetFocus(nullptr);
                    //�����¸���QT����
                    currentFocusWidget->setFocus();
                }
            }
        }

        //if (event->type() == QEvent::MouseMove)
        //    qDebug() << QApplication::widgetAt(mouseEvent->globalPos())->objectName().toStdString().c_str();
        return false; // �¼�δ��������������
    }
};

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    MyEventFilter filter;
    a.installEventFilter(&filter); // ��װ�¼�������

    EditorMain w;
    w.show();
    w.resize(1024, 768);

    return a.exec();
}
