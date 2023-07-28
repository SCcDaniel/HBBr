#include "RenderView.h"
#include "qstylepainter.h"
#include "QStyleOption.h"
#pragma comment(lib , "RendererCore.lib")
RenderView::RenderView(QWidget* parent)
	: QWidget(parent)
{
	//����ʹ���û��Զ�����ƣ�����Ҫ����WA_PaintOnScreen
	setAttribute(Qt::WA_PaintOnScreen, true);
	//����ҪĬ�ϵ�Qt����
	setAttribute(Qt::WA_NoSystemBackground, true);
	//�ػ�ʱ������ȫ������
	//setAttribute(Qt::WA_OpaquePaintEvent, true);
	setAttribute(Qt::WA_NoBackground, true);
	setAttribute(Qt::WA_NativeWindow, true);

	setStyleSheet("background-color:rgb(0,0,0);");
}

RenderView::~RenderView()
{

}

void RenderView::showEvent(QShowEvent* event)
{
	QWidget::showEvent(event);
	if (_vkRenderer == NULL)
	{
		_vkRenderer = new VulkanRenderer((void*)this->winId(), "MainRenderer");

		_renderTimer = new QTimer(this);
		_renderTimer->setInterval(1);
		connect(_renderTimer, SIGNAL(timeout()), this, SLOT(UpdateRender()));
		_renderTimer->start();
	}
}

void RenderView::resizeEvent(QResizeEvent* event)
{
	QWidget::resizeEvent(event);
	if (_vkRenderer != NULL)
	{
		_vkRenderer->RendererResize(width(), height());
	}
}

void RenderView::closeEvent(QCloseEvent* event)
{
	QWidget::closeEvent(event);
	if (_vkRenderer != NULL) 
	{
		_vkRenderer->Release();
		_vkRenderer = NULL;
	}
}

void RenderView::paintEvent(QPaintEvent* event)
{
	QStylePainter painter(this);
	QStyleOption opt;
	opt.initFrom(this);
	opt.rect = rect();
	painter.drawPrimitive(QStyle::PE_Widget, opt);
}

void RenderView::UpdateRender()
{
	if (_vkRenderer != NULL)
	{
		_vkRenderer->Render();
		//SetFocus((HWND)winId());
	}
}