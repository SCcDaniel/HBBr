#pragma once
#include "QString.h"
#include <QFileInfo>

QString GetWidgetStyleSheetFromFile(QString objectName, QString path = "Config/Theme/ThemeMain.qss");

bool DeleteAllFile(QString path, QList<QString>*allAssets = NULL);

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
