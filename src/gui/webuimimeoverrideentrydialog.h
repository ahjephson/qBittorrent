#pragma once

#include <QDialog>

#include "gui/webuimimeoverridesmodel.h"

namespace Ui
{
    class WebUIMimeOverrideEntryDialog;
}

class WebUIMimeOverrideEntryDialog final : public QDialog
{
    Q_OBJECT
    Q_DISABLE_COPY_MOVE(WebUIMimeOverrideEntryDialog)

public:
    explicit WebUIMimeOverrideEntryDialog(QWidget *parent = nullptr);
    ~WebUIMimeOverrideEntryDialog() override;

    void setEntry(const WebUIMimeOverridesModel::Entry &entry);
    WebUIMimeOverridesModel::Entry entry() const;

protected:
    void accept() override;

private:
    Ui::WebUIMimeOverrideEntryDialog *m_ui = nullptr;
};
