#ifndef UICOMMAND_H
#define UICOMMAND_H

#include <QAction>
#include <QString>
#include <QStringList>
#include <QSharedPointer>
#include "simapi.h"

namespace SIM
{

class EXPORT UiCommand;
typedef QSharedPointer<UiCommand> UiCommandPtr;

class EXPORT CommandSet;
typedef QSharedPointer<CommandSet> CommandSetPtr;

class EXPORT UiCommand : public QObject
{
    Q_OBJECT
public:
    static UiCommandPtr create(const QString& text, const QString& iconId, const QString& id, const QStringList& tags = QStringList());

    QString text() const;
    QIcon icon() const;

    void setIconId(const QString& id);
    QString iconId() const;

    QString id() const;

    void addTag(const QString& tag);
    void clearTags();
    QStringList tags() const;

    bool isCheckable() const;
    void setCheckable(bool b);

    bool isChecked() const;

signals:

public slots:

protected:
    explicit UiCommand(const QString& text, const QString& iconId, const QString& id, const QStringList& tags = QStringList());

private:
    QString m_id;
    QStringList m_tags;
    QString m_iconId;
    CommandSetPtr m_subcommands;
    UiCommand* m_parent;
    QString m_text;
    QIcon m_icon;
    bool m_checkable;
    bool m_checked;
    bool m_autoExclusive;
};
}

#endif // UICOMMAND_H
