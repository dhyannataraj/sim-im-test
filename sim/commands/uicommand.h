#ifndef UICOMMAND_H
#define UICOMMAND_H

#include <QAction>
#include <QString>
#include <QStringList>
#include <QSharedPointer>
#include "simapi.h"
#include "contacts/contact.h"
#include "contacts/group.h"
#include "contacts/imcontact.h"
#include "contacts/imgroup.h"
#include "uicommandaction.h"
#include "uicommandcontextprovider.h"
#include "uicommandcontext.h"

namespace SIM
{

class EXPORT UiCommand;
typedef QSharedPointer<UiCommand> UiCommandPtr;

class EXPORT UiCommand : public QObject
{
    Q_OBJECT
public:
    explicit UiCommand(const QString& text, const QString& iconId, const QString& id, const QStringList& tags = QStringList());
    virtual ~UiCommand();

    QString text() const;
    QIcon icon() const;

    void setIconId(const QString& id);
    QString iconId() const;

    QString id() const;

    void addTag(const QString& tag);
    void clearTags();
    QStringList tags() const;

    virtual QList<SIM::UiCommandContext::UiCommandContext> getAvailableContexts() const = 0;

    virtual UiCommandAction* createAction(QWidget* parent);

    virtual void perform(UiCommandContextProvider* provider);

    void updateState(bool active);

signals:
    void stateChanged(bool active);

protected:
    bool state() const;

private:
    QString m_id;
    QStringList m_tags;
    QString m_iconId;
    QString m_text;
    QIcon m_icon;
    bool m_state;
};

}

#endif // UICOMMAND_H
