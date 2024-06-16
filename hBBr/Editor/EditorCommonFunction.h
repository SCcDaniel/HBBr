#pragma once
#include "QString.h"
#include <QFileInfo>

QString GetWidgetStyleSheetFromFile(QString objectName, QString path = "Config/Theme/ThemeMain.qss");

QString GetSingleStyleFromFile(QString Name, QString path = "Config/Theme/ThemeMain.qss");

#define ActionConnect(action,func) connect(action, &QAction::triggered, this, func);

struct SFileSearch
{
	QString path;
	QFileInfo fileInfo;
	bool bLastFile = false;
};
//ֻ����Ŀ¼
bool SearchDir(QString path, QString searchText ,QList<SFileSearch>& ResultOutput);
//������ǰĿ¼�µ��ƶ��ļ�
bool SearchFile(QString path, QString searchText, QList<SFileSearch>& ResultOutput);
//��ȡTGAͼ��
QImage GetImageFromTGA(QString path);

bool GetPreviewImage(QString resourceFilePath , QPixmap& pixmap);

QString GetEditorInternationalization(QString Group, QString name);
