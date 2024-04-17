﻿#pragma once
#include "CustomView.h"
#include "ui_ContentBrowser.h"
#include "Asset/ContentManager.h"

//--------------------------------------Virtual Folder Tree View-------------------
#pragma region VirtualFolderTreeView
class VirtualFolderTreeView : public CustomTreeView
{
	Q_OBJECT
	friend class ContentBrowser;
	friend class VirtualFileListView;
public:
	explicit VirtualFolderTreeView(class  ContentBrowser* contentBrowser , QWidget* parent = nullptr);

	void AddItem(CustomViewItem* newItem, CustomViewItem* parent = nullptr)override;

	//虚拟路径统一用“/”分割，不存在使用"\\"
	CustomViewItem* FindFolder(QString virtualPath);

	virtual void SelectionItem(QString vPath)override;

	class  ContentBrowser* _contentBrowser;

protected:
	virtual void selectionChanged(const QItemSelection& selected, const QItemSelection& deselected)override;
	void currentChanged(const QModelIndex& current, const QModelIndex& previous) override;
	QList<CustomViewItem*> _newSelectionItems;
	int _currentSelectionItem;
	bool _bSaveSelectionItem;
};
#pragma endregion

//--------------------------------------Virtual File List View-------------------
#pragma region VirtualFileListView
class VirtualFileListView :public CustomListView
{
	Q_OBJECT
	friend class ContentBrowser;
	friend class VirtualFolderTreeView;
public:
	explicit VirtualFileListView(QWidget* parent = nullptr);

	CustomListItem* AddFile(std::weak_ptr<struct AssetInfoBase> assetInfo);

	VirtualFolder _currentTreeViewSelection;

protected:

};
#pragma endregion

//--------------------------------------Repository Selection Widget-------------------
#pragma region RepositorySelectionWidget
class RepositorySelection : public QWidget
{
	Q_OBJECT
	friend class VirtualFileListView;
	friend class VirtualFolderTreeView;
	friend class ContentBrowser;
public:
	RepositorySelection(QWidget* parent);
	void Show();
	void Hide();
	std::function<void(QString repository)> _selectionCallBack;
	QString _currentRepositorySelection;
protected:
	virtual void paintEvent(QPaintEvent* event)override;
	virtual void resizeEvent(QResizeEvent* event)override;
	class ComboBox* combo = nullptr;
};

#pragma endregion

//--------------------------------------Content Browser Widget-------------------
#pragma region ContentBrowserWidget
class ContentBrowser : public QWidget
{
	Q_OBJECT
	friend class VirtualFileListView;
	friend class VirtualFolderTreeView;
	friend class RepositorySelection;
public:

	ContentBrowser(QWidget* parent = nullptr);
	~ContentBrowser();

	static void RefreshContentBrowsers();
	void Refresh();
	void RefreshFolderOnTreeView();
	void RefreshFileOnListView();

	static class RepositorySelection* _repositorySelection;

protected:

	virtual void focusInEvent(QFocusEvent* event);
	virtual void showEvent(QShowEvent* event);
	virtual void paintEvent(QPaintEvent* event);
	virtual void mouseMoveEvent(QMouseEvent* event) override;
	virtual void mousePressEvent(QMouseEvent* event);
	virtual void closeEvent(QCloseEvent* event) override;
	static QList<ContentBrowser*> _contentBrowser;

	class QSplitter*					_splitterBox = nullptr;
	class QWidget*					_listWidget = nullptr;
	class QWidget*					_treeWidget = nullptr;

	VirtualFolderTreeView*		_treeView = nullptr;
	VirtualFileListView*			_listView = nullptr;

	QStringList _importFileNames;

private:

	Ui::ContentBrowserClass ui;

};
#pragma endregion