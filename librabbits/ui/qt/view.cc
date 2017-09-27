#include "view.h"

QtUiView::QtUiView(QWidget *parent, const std::string &name)
    : QWidget(parent), m_name(name)
{
}

void QtUiView::qt_ui_set_name(const std::string &name)
{
    m_name = name;
    emit name_changed(QString::fromStdString(name));
}
