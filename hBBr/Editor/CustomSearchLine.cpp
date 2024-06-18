#include "CustomSearchLine.h"
#include "HString.h"
#include "EditorCommonFunction.h"
#include <QPixmap>
#include <qabstractitemview.h>
#include <QPainter>
#include <QStyleOption>
CustomSearchLine::CustomSearchLine(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);

	this->setObjectName("SearchLine");
	ui.lineEdit->setObjectName("SearchLine_LineEdit");
	ui.comboBox->setObjectName("SearchLine_ComboBox");
	ui.comboBox->view()->viewport()->setObjectName("SearchLine_ComboBoxViewport");
	ui.label->setMinimumSize(20, 20);
	ui.label->setMaximumSize(20, 20);
	this->setMaximumHeight(20);
	this->setMinimumHeight(20);
	//QPixmap image((FileSystem::GetProgramPath() + "Config/Theme/Icons/ICON_SEARCH.png").c_str());
	//image.scaled(4,4);
	//ui.label->setPixmap(image);

}

CustomSearchLine::~CustomSearchLine()
{

}

void CustomSearchLine::paintEvent(QPaintEvent* event)
{
	Q_UNUSED(event);
	QStyleOption styleOpt;
	styleOpt.init(this);
	QPainter painter(this);
	style()->drawPrimitive(QStyle::PE_Widget, &styleOpt, &painter, this);
}