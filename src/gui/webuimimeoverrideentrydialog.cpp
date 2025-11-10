#include "webuimimeoverrideentrydialog.h"

#include <QMessageBox>

#include "base/webuimimetypestore.h"
#include "ui_webuimimeoverrideentrydialog.h"

WebUIMimeOverrideEntryDialog::WebUIMimeOverrideEntryDialog(QWidget *parent)
    : QDialog(parent)
    , m_ui(new Ui::WebUIMimeOverrideEntryDialog)
{
    m_ui->setupUi(this);
    setWindowModality(Qt::WindowModal);
}

WebUIMimeOverrideEntryDialog::~WebUIMimeOverrideEntryDialog()
{
    delete m_ui;
}

void WebUIMimeOverrideEntryDialog::setEntry(const WebUIMimeOverridesModel::Entry &entry)
{
    m_ui->extensionLineEdit->setText(entry.extension);
    m_ui->contentTypeLineEdit->setText(entry.contentType);
}

WebUIMimeOverridesModel::Entry WebUIMimeOverrideEntryDialog::entry() const
{
    WebUIMimeOverridesModel::Entry result;
    result.extension = m_ui->extensionLineEdit->text();
    result.contentType = m_ui->contentTypeLineEdit->text();
    return result;
}

void WebUIMimeOverrideEntryDialog::accept()
{
    const QString extension = WebUIMimeTypeStore::normalizeExtension(m_ui->extensionLineEdit->text());
    const QString contentType = m_ui->contentTypeLineEdit->text().trimmed();

    if (extension.isEmpty())
    {
        QMessageBox::warning(this, tr("Invalid extension"), tr("Please enter a valid file extension."));
        return;
    }

    if (contentType.isEmpty())
    {
        QMessageBox::warning(this, tr("Invalid content type"), tr("Please enter a valid content type."));
        return;
    }

    QDialog::accept();
}
