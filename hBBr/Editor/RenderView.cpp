#include "RenderView.h"

#pragma comment(lib , "RendererCore.lib")

RenderView::RenderView(QWidget *parent)
	: QWidget(parent)
{
	if (_vkRenderer == NULL)
		_vkRenderer = new VulkanRenderer((void*)this->winId(),true);
}

RenderView::~RenderView()
{
	if (_vkRenderer != NULL)
		delete _vkRenderer;
	_vkRenderer = NULL;
}