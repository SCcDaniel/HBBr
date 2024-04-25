#include "SceneOutline.h"
#include "VulkanRenderer.h"
#include "Asset/World.h"
#include "Asset/Level.h"
#include "Component/GameObject.h"
#include <qheaderview.h>
#include <qaction.h>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QMimeData>
#include <QApplication>
#include <QDrag>
#include <QStyleOption>
#include <QPainter>
#include <qmenu.h>
#include "EditorCommonFunction.h"
#include "CustomSearchLine.h"
#include "Inspector.h"
#include "CheckBox.h"
#include "ComboBox.h"
SceneOutlineTree* SceneOutline::_treeWidget = nullptr;

SceneOutlineItem::SceneOutlineItem(std::weak_ptr<Level> level, std::weak_ptr<GameObject> gameObject, QTreeWidget* view)
    :QTreeWidgetItem(view)
{
    Init(level, gameObject);
}

SceneOutlineItem::SceneOutlineItem(std::weak_ptr<Level> level, std::weak_ptr<GameObject> gameObject, SceneOutlineItem* parent)
    :QTreeWidgetItem(parent)
{
    Init(level, gameObject);
}

SceneOutlineItem::~SceneOutlineItem()
{

}

void SceneOutlineItem::Init(std::weak_ptr<Level> level, std::weak_ptr<GameObject> gameObject)
{
    if (!level.expired())
    {
        _level = level;
        this->setText(0, _level.lock()->GetLevelName().c_str());
    }
    else if (!gameObject.expired())
    {
        _gameObject = gameObject;
        _gameObject.lock()->_editorObject = this;
        this->setText(0, _gameObject.lock()->GetObjectName().c_str());
    }
    setFlags(Qt::ItemIsEditable | Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled);
}

void SceneOutlineItem::Destroy()
{
    if (!_gameObject.expired())
    {
        _gameObject.lock()->_editorObject = nullptr;
        _gameObject.lock()->Destroy();
    }
    delete this;
}

SceneOutlineTree::SceneOutlineTree(class VulkanRenderer* renderer, QWidget* parent)
    :QTreeWidget(parent)
{
    _parent = (SceneOutline*)parent;
    setFocusPolicy(Qt::FocusPolicy::ClickFocus);
    setHeaderHidden(true);
    setIconSize({ 0,0 });
    //setDragDropMode(QAbstractItemView::DragDropMode::DragDrop);
    //�����Ϸ�ģʽΪ�ڲ��ƶ�
    setDragDropMode(QAbstractItemView::InternalMove);
    //�������drop����
    setAcceptDrops(true);
    setEditTriggers(EditTrigger::NoEditTriggers);//�ص�˫�������Ĺ���
    //setMouseTracking(true);

    setObjectName("SceneOutline");
    viewport()->setObjectName("SceneOutline");

    setIndentation(10);

    _renderer = renderer;
    _menu = new QMenu(this);
    _createNewGameObject    = new QAction(QString::fromLocal8Bit("����GameObject"), _menu);
    _deleteGameObject       = new QAction(QString::fromLocal8Bit("ɾ��"), _menu);
    _renameGameObject       = new QAction(QString::fromLocal8Bit("������"), _menu);

    _menu_createBasic = new QMenu(this);
    _menu_createBasic->setTitle(QString::fromLocal8Bit("����Ԥ��ģ��"));
    _createCube = new QAction(QString::fromLocal8Bit("Cube"), _menu_createBasic);
    _createSphere = new QAction(QString::fromLocal8Bit("Sphere"), _menu_createBasic);
    _createPlane = new QAction(QString::fromLocal8Bit("Plane"), _menu_createBasic);

    _createNewLevel = new QAction(QString::fromLocal8Bit("�����µĿճ���"), _menu_createBasic);
     _deleteLevel = new QAction(QString::fromLocal8Bit("ɾ������"), _menu_createBasic);
     _renameLevel = new QAction(QString::fromLocal8Bit("����������"), _menu_createBasic);
     _loadLevel = new QAction(QString::fromLocal8Bit("��������"), _menu_createBasic);
     _unloadLevel = new QAction(QString::fromLocal8Bit("����ж��"), _menu_createBasic);

    //setRootIsDecorated(false);
    connect(this, SIGNAL(itemDoubleClicked(QTreeWidgetItem*, int)),this,SLOT(ItemDoubleClicked(QTreeWidgetItem*, int)));
    connect(this, SIGNAL(sigEditFinished(QString)), this, SLOT(ItemEditFinished(QString)));
    connect(this, SIGNAL(itemSelectionChanged()), this, SLOT(ItemSelectionChanged()));
    connect(_createNewGameObject, &QAction::triggered, this, [this](bool bChecked) 
		{
            _renderer->ExecFunctionOnRenderThread([]() 
                {
                    GameObject::CreateGameObject();
                });           
		});
    connect(_renameGameObject, &QAction::triggered, this, [this](bool bChecked)
        {
            _renderer->ExecFunctionOnRenderThread([this]()
                {
                    if (this->currentItem())
                        editItem(this->currentItem());
                });
        });
    connect(_deleteGameObject, &QAction::triggered, this, [this](bool bChecked)
        {
            _renderer->ExecFunctionOnRenderThread([this]()
                {
                    if (this->currentItem())
                    {
                        ((SceneOutlineItem*)this->currentItem())->_gameObject.lock()->Destroy();
                    }
                });
        });
    connect(_createCube, &QAction::triggered, this, [this](bool bChecked)
        {
            _renderer->ExecFunctionOnRenderThread([]()
                {
                    QFileInfo fi("./Asset/Content/Core/Basic/Cube");
                    //GameObject::CreateModelGameObject(fi.absolutePath().toStdString().c_str());
                });
        });
    connect(_createSphere, &QAction::triggered, this, [this](bool bChecked)
        {
            _renderer->ExecFunctionOnRenderThread([]()
                {
                    QFileInfo fi("./Asset/Content/Core/Basic/Sphere");
                    //GameObject::CreateModelGameObject(fi.absolutePath().toStdString().c_str());
                });
        });
    connect(_createPlane, &QAction::triggered, this, [this](bool bChecked)
        {
            _renderer->ExecFunctionOnRenderThread([]()
                {
                    QFileInfo fi("./Asset/Content/Core/Basic/Plane");
                    //GameObject::CreateModelGameObject(fi.absolutePath().toStdString().c_str());
                });
        });
    //�����³���
    connect(_createNewLevel, &QAction::triggered, this, [this](bool bChecked)
        {
            HString newLevelName = "New Level";
            int index = 0;
            while (true)
            {
                bool bFound = false;
                for (auto& i : _parent->currentWorld.lock()->GetLevels())
                {
                    if (i->GetLevelName().IsSame(newLevelName))
                    {
                        bFound = true;
                        newLevelName = "New Level " + HString::FromInt(index);
                        index++;
                        break;
                    }
                }
                if (!bFound)
                    break;
            }
            _parent->currentWorld.lock()->AddNewLevel(newLevelName);
        });
    //ɾ������
    connect(_deleteLevel, &QAction::triggered, this, [this](bool bChecked)
        {

        });
    //����������
    connect(_renameLevel, &QAction::triggered, this, [this](bool bChecked)
        {

        });
    //��������
    connect(_loadLevel, &QAction::triggered, this, [this](bool bChecked)
        {
            for (auto& i : GetSelectionLevels())
            {
                i.lock()->Load();
            }
        });
    //����ж��
    connect(_unloadLevel, &QAction::triggered, this, [this](bool bChecked)
        {
            for (auto& i : GetSelectionLevels())
            {
                i.lock()->UnLoad();
            }
        });
    //��Item�����仯
    connect(this, &QTreeWidget::itemChanged, this, [this](QTreeWidgetItem* item, int column)
        {
            SceneOutlineItem* sceneItem = (SceneOutlineItem*)item;
            if (!sceneItem->_level.expired())
            {
                //����ֻ���б༭һ��Level������ͬʱ�༭���
                for (auto& i : _parent->_levelItems)
                {
                    if (i->_level.lock().get() == sceneItem->_level.lock().get() && i->checkState(0) == Qt::Checked)
                    {
                        _parent ->_currentLevelItem = sceneItem;
                        //�����༭��Level�����Զ�Load
                        i->_level.lock()->Load();
                    }
                    else
                    {
                        i->setCheckState(0, Qt::Unchecked);
                    }
                }
            }
        });
}

void SceneOutlineTree::contextMenuEvent(QContextMenuEvent* event)
{
    _menu->clear();
    _menu_createBasic->clear();
    //
    auto item = (SceneOutlineItem*)itemAt(event->pos());
    if (item)
    {
        if (!item->_gameObject.expired())
        {
            _menu_createBasic->addAction(_createCube);
            _menu_createBasic->addAction(_createSphere);
            _menu_createBasic->addAction(_createPlane);

            _menu->addAction(_createNewGameObject);
            _menu->addMenu(_menu_createBasic);
            _menu->addAction(_renameGameObject);
            _menu->addSeparator();
            _menu->addAction(_deleteGameObject);
        }
        else if (!item->_level.expired())
        {
            _menu->addAction(_createNewLevel);
            _menu->addAction(_renameLevel);
            _menu->addAction(_loadLevel);
            _menu->addAction(_unloadLevel);
            _menu->addSeparator();
            _menu->addAction(_deleteLevel);
        }
        _menu->exec(event->globalPos());
    }
}

void SceneOutlineTree::ItemSelectionChanged()
{
    auto objects = GetSelectionObjects();
    if (Inspector::_currentInspector != nullptr)
    {
        if (objects.size() > 0)
        {
            if (Inspector::_currentInspector != nullptr)
                Inspector::_currentInspector->LoadInspector_GameObject(objects[0].lock()->GetSelfWeekPtr());
        }
        else
        {
            Inspector::_currentInspector->ClearInspector();
        }
    }
}

void SceneOutlineTree::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton) {
        _curItem = this->itemAt(event->pos());
    }
    QTreeWidget::mousePressEvent(event);
}

void SceneOutlineTree::mouseMoveEvent(QMouseEvent* event)
{
    _mouseTouchItem = this->itemAt(event->pos());
    QTreeWidget::mouseMoveEvent(event);
}

void SceneOutlineTree::dropEvent(QDropEvent* event)
{
   /* QTreeWidget::dropEvent(event);
    if (_curItem != nullptr)
    {
        SceneOutlineItem* dragItem = (SceneOutlineItem*)_curItem;
        if (_mouseTouchItem == nullptr)
        {
            dragItem->_gameObject->SetParent(nullptr);
        }
        else
        {
            SceneOutlineItem* currentItem = (SceneOutlineItem*)(_mouseTouchItem);
            {
                if (dragItem->parent())
                {
                    dragItem->_gameObject->SetParent(((SceneOutlineItem*)dragItem->parent())->_gameObject);
                }
                else
                {
                    dragItem->_gameObject->SetParent(nullptr);
                }
            }
        }
        _curItem = nullptr;
    }*/
}

QList<std::weak_ptr<GameObject>> SceneOutlineTree::GetSelectionObjects()
{
    QList<std::weak_ptr<GameObject>> result;
    for (auto i : this->selectedItems())
    {
        auto item = dynamic_cast<SceneOutlineItem*>(i);
        if (item != nullptr && !item->_gameObject.expired())
        {
            result.append(item->_gameObject);
        }
    }
    return result;
}

QList<std::weak_ptr<Level>> SceneOutlineTree::GetSelectionLevels()
{
    QList<std::weak_ptr<Level>> result;
    for (auto i : this->selectedItems())
    {
        auto item = dynamic_cast<SceneOutlineItem*>(i);
        if (item != nullptr && !item->_level.expired())
        {
            result.append(item->_level);
        }
    }
    return result;
}

void SceneOutlineTree::ItemDoubleClicked(QTreeWidgetItem* item, int column)
{
    if (!item->isExpanded())
    {
        collapseItem(item);
    }
    else
    {
        expandItem(item);
    }
}

void SceneOutlineTree::ItemEditFinished(QString newText)
{
    SceneOutlineItem* objItem = (SceneOutlineItem*)currentItem();
    if(!objItem->_gameObject.expired() && objItem->_gameObject.lock()->GetObjectName() != newText.toStdString().c_str())
        objItem->_gameObject.lock()->SetObjectName(newText.toStdString().c_str());
}

SceneOutline::SceneOutline(VulkanRenderer* renderer, QWidget *parent)
    : QWidget(parent)
{
    setFocusPolicy(Qt::FocusPolicy::ClickFocus);
    setObjectName("SceneOutline");
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(1,1,1,1);
    mainLayout->setSpacing(1);
    this->setLayout(mainLayout);

    _search = new CustomSearchLine(this);
    _search->setMaximumHeight(30);
    _search->ui.comboBox->setHidden(true);
    mainLayout->addWidget(_search);
    connect(_search->ui.lineEdit, SIGNAL(returnPressed()), this, SLOT(TreeSearch()));

    _treeWidget = new SceneOutlineTree(renderer, this);
    mainLayout->addWidget(_treeWidget);

    _renderer = renderer;

    //World���ɵ�ʱ��ִ��
    auto spawnNewWorldCallBack = [this](std::weak_ptr<World> world)
    {
        if (!world.expired())
        {
            currentWorld = world;
            //Worldÿ֡����
            world.lock()->_editorWorldUpdate=
                [](std::vector<std::weak_ptr<Level>>&levels)
                {
                    
                };

            //World��Level���鷢���仯��ʱ��ִ��(���� ��������ɾ��Level)
            world.lock()->_editorLevelChanged =
                [this, world]()
            {
                if (!world.expired())
                for (auto& i : world.lock()->GetLevels())
                {
                    auto itemExist = FindLevel(i->GetLevelName().c_str());
                    if(itemExist == nullptr)
                    {
                        //����LevelĿ¼
                        auto item = new SceneOutlineItem(i, std::weak_ptr<GameObject>(), _treeWidget);
                        item->setFlags(item->flags() | Qt::ItemIsUserCheckable); //����ѡ��֧��CheckBox������ȷ����ǰ�����޸ĵ����ĸ�����
                        item->setCheckState(0, Qt::Unchecked);
                        _treeWidget->addTopLevelItem(item);
                        _levelItems.insert(i->GetLevelName().c_str(), item);
                        if (!i->IsLoaded())
                        {
                            item->setFlags(item->flags() & ~Qt::ItemIsEnabled);
                        }
                    }
                    ////Ĭ�ϵ�һ�����������༭
                    //_levelItems.first()->setCheckState(0, Qt::Checked);
                    //_treeWidget->expandItem(_levelItems.first());
                }
            };

            //�༭���� GameObject ����ί��
            world.lock()->_editorGameObjectAddFunc = [this]
            (std::shared_ptr<GameObject> object)
            {
                QString levelName = object->GetLevel()->GetLevelName().c_str();
                auto levelItem = FindLevel(levelName);
                if (levelItem)
                {
                    //�ڶ�Ӧ��LevelĿ¼�����Item
                    auto newObjectItem = new SceneOutlineItem(std::weak_ptr<Level>(), object, levelItem);
                }
            };

            //�༭���� GameObject SetParentί��
            world.lock()->_editorGameObjectSetParentFunc = [this]
            (std::shared_ptr<GameObject> object, std::shared_ptr<GameObject> newParent)
            {

            };
            

            //�༭���� GameObjectÿ֡����
            world.lock()->_editorGameObjectUpdateFunc = [this]
            (std::shared_ptr<GameObject> object)
            {
                if (object->_bEditorNeedUpdate)
                {
                    object->_bEditorNeedUpdate = false;
                    auto objects = SceneOutline::_treeWidget->GetSelectionObjects();
                    if (objects.size() > 0)
                    {
                        if (Inspector::_currentInspector != nullptr)
                            Inspector::_currentInspector->LoadInspector_GameObject(objects[0].lock()->GetSelfWeekPtr(), true);
                    }
                    if (object->_editorObject)
                    {
                        auto item = (SceneOutlineItem*)object->_editorObject;
                        //rename?
                        item->setText(0, object->GetObjectName().c_str());
                    }
                }
            };

            //�༭���� GameObject ����ί��
            world.lock()->_editorGameObjectRemoveFunc = [this]
            (std::shared_ptr<GameObject> object)
            {
                auto item = (SceneOutlineItem*)object->_editorObject;
                //QT5 QTreeWidgetItem ɾ���˸��ڵ�,�ӽڵ��ڴ�Ҳ�����һ������,
                //�ᵼ��object->_editorObject(�ڴ��ѱ�ɾ��)��ȡ�����쳣,
                //�������ٽڵ�֮ǰ�Ȱ��ӽڵ�ȡ������ֹ��ͻ��
                if (item->childCount() > 0)
                {
                    if (item->parent())
                    {
                        item->parent()->addChildren(item->takeChildren());
                    }
                    else
                    {
                        _treeWidget->addTopLevelItems(item->takeChildren());
                    }
                }
                item->Destroy();
            };
            
            for (auto &i : world.lock()->GetLevels())
            {

            }
        }
    };   
    _renderer->_spwanNewWorld.push_back(spawnNewWorldCallBack);

}

SceneOutline::~SceneOutline()
{
}

void SceneOutline::closeEvent(QCloseEvent* event)
{

}

void traverse(QTreeWidgetItem* item , QList<QTreeWidgetItem*>& list , bool bHide = false , QString findName = "")
{
    // ���������ӽڵ�
    for (int i = 0; i < item->childCount(); ++i) {
        traverse(item->child(i) , list , bHide);
        if (findName.length() > 0)
        {
            if (item->child(i)->text(0).contains(findName))
            {

            }
        }
        list.append(item->child(i));
        item->child(i)->setHidden(bHide);
    }
    item->setHidden(bHide);
}

bool searchTraverse(QTreeWidgetItem* item, QList<QTreeWidgetItem*>& list, QString findName = "")
{
    bool bHide = true;
    // ���������ӽڵ�
    for (int i = 0; i < item->childCount(); ++i) {
        bHide = bHide && searchTraverse(item->child(i), list);
        if (findName.length() > 0)
        {
            if (item->child(i)->text(0).contains(findName))
            {
                bHide = false;
                continue;
            }
        }
        item->child(i)->setHidden(bHide);
        list.append(item->child(i));
    }

    if (findName.length() > 0)
    {
        if (item->text(0).contains(findName))
        {
            return false;
        }
    }

    return bHide;
}

void SceneOutline::TreeSearch()
{
    //_treeWidget->collapseAll();
    QList<QTreeWidgetItem*> searchList;
    _treeWidget->selectionModel()->clearSelection();
    if (_search->ui.lineEdit->text().isEmpty())
    {
        for (int i = 0; i < _treeWidget->topLevelItemCount(); ++i) {
            traverse(_treeWidget->topLevelItem(i), searchList, false);
        }
        return;
    }
    QString input = _search->ui.lineEdit->text();
    _treeWidget->expandAll();
    //auto searchList = _treeWidget->findItems(input, Qt::MatchExactly)
    for (int i = 0; i < _treeWidget->topLevelItemCount(); ++i) {
        bool bHide = searchTraverse(_treeWidget->topLevelItem(i) , searchList , input);
        _treeWidget->topLevelItem(i)->setHidden(bHide);       
    }
}

void SceneOutline::focusInEvent(QFocusEvent* event)
{

}

void SceneOutline::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event);
    QStyleOption styleOpt;
    styleOpt.init(this);
    QPainter painter(this);
    style()->drawPrimitive(QStyle::PE_Widget, &styleOpt, &painter, this);
}

SceneOutlineItem* SceneOutline::FindLevel(QString levelName)
{
    SceneOutlineItem* result = nullptr; 
    auto it = _levelItems.find(levelName);
    if (it != _levelItems.end())
    {
        result = it.value();
    }
    return result;
}

