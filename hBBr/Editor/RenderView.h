#pragma once
#include <QWidget>
#include <qtimer.h>
#include "FormMain.h"

class RenderView  : public QWidget
{
	Q_OBJECT

public:

	RenderView(QWidget *parent);

	~RenderView();

	void Update();

	QWidget* _mainRendererWidget = nullptr;

	class VulkanForm* _mainRenderer = nullptr;

	static SDL_Keycode mapQtKeyToSdlKey(int qtKey);

protected:
	//不使用Qt默认的绘制引擎
	//virtual QPaintEngine* paintEngine() const { return 0; }

	bool event(QEvent* event) override;

	virtual void focusInEvent(QFocusEvent* event)override;

	virtual void focusOutEvent(QFocusEvent* event)override;

	virtual void mousePressEvent(QMouseEvent* event)override;

	virtual void mouseReleaseEvent(QMouseEvent* event)override;

	virtual void showEvent(QShowEvent* event)override;

	virtual void resizeEvent(QResizeEvent* event)override;

	virtual void closeEvent(QCloseEvent* event)override;

	virtual void paintEvent(QPaintEvent* event)override;

	virtual void keyPressEvent(QKeyEvent* event)override;

	virtual void keyReleaseEvent(QKeyEvent* event)override;

	virtual void dragEnterEvent(QDragEnterEvent* event)override;

	virtual void dropEvent(QDropEvent* event)override;

};
