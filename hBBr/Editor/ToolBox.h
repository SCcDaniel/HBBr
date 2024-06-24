#pragma once
#include <QWidget>
#include "ui_ToolBox.h"
#include "ui_ToolPage.h"

class QFormLayout;
class QLabel;
class ToolPage : public QWidget
{
    Q_OBJECT
public:
    explicit ToolPage(bool isExpanded = false, QWidget* parent = nullptr);
    ~ToolPage();
    Ui::PageClassTool ui;
public slots:
    void addWidget(const QString& title, QWidget* widget);
    void expand();
    void collapse();
private slots:
    void onPushButtonFoldClicked();
private:
    bool m_bIsExpanded;
    class QLabel* m_pLabel;
};

class QVBoxLayout;
class ToolBox : public QWidget
{
    Q_OBJECT

public:
    explicit ToolBox(QWidget* parent = nullptr);
    explicit ToolBox(QString title, bool isExpanded = false, QWidget* parent = nullptr);
    ~ToolBox();
    //���ҳ��,bAdditive: �Ƿ�ǿ�Ƽ�һ����ҳ��
    void addPage(const QString& title, bool isExpanded = false, bool bAdditive = false);
    //�����µ�ҳ�������
    void addSubWidget(QWidget* widget);
    //���ݱ������ֻ�ȡ��ҳ�棬Ȼ�����
    void addSubWidget(const QString& title, QWidget* widget);
    //page��mianWidget
    std::map<QString, class QWidget*> _widgets;

    QWidget* getWidget()const {return m_widget;}
    Ui::ToolBoxClass ui;
private:
    QVBoxLayout* m_pContentVBoxLayout;
    QWidget* m_widget = nullptr;
};