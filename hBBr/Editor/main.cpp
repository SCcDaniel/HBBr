#include "EditorMain.h"
#include <QtWidgets/QApplication>
#include <QMouseEvent>
#include <qdebug.h>
#include "FormMain.h"
#include "RenderView.h"
#include "HInput.h"
#include "ContentBrowser.h"
#include "qfontdatabase.h"
#include "ConsoleDebug.h"
#include "EditorCommonFunction.h"
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
            auto cursorPos = mouseEvent->globalPos();
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
            //ContentBrowser focus
            for (auto& cb : ContentBrowser::GetContentBrowsers())
            {
                if (cb)
                {
                    QPoint localCursorPos = cb->mapFromGlobal(mouseEvent->globalPos());
                    if (cb->rect().contains(localCursorPos))
                    {
                        ContentBrowser::SetCurrentBrowser(cb);
                        break;
                    }
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

    //���촰��
    EditorMain w;

    //��������
    auto allFonts = FileSystem::GetAllFilesExceptFolders(FileSystem::Append(FileSystem::GetConfigAbsPath(), "Theme/Fonts").c_str());
    for (auto& i : allFonts)
    {
        QString fontPath = QString::fromStdWString(i.absPath.c_wstr());
        auto fontId = QFontDatabase::addApplicationFont(fontPath);
        if (fontId != -1)
        {
            QStringList fonts = QFontDatabase::applicationFontFamilies(fontId);
            foreach(QString font, fonts)
            {
                ConsoleDebug::printf_endl("Loaded font:%s", font.toStdString().c_str());
            }
            QString fontFamily = fonts.at(0);
            QFont defaultFont = a.font();
            defaultFont.setFamily(fontFamily);
            a.setFont(defaultFont);
        }
        else
        {
            ConsoleDebug::printf_endl("Failed to load the font: %s", fontPath.toStdString().c_str());
        }
    }

    //ˢ��һ����ʽ
    w.setStyleSheet(GetWidgetStyleSheetFromFile("EditorMain"));

    //��ʾ����
    w.show();
    w.resize(1024, 768);

    return a.exec();
}
