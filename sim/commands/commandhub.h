#ifndef COMMANDHUB_H
#define COMMANDHUB_H

#include <QObject>
#include <QMap>
#include "simapi.h"
#include <QAction>

namespace SIM
{
struct ActionDescriptor
{
    QString id;
    QString iconId;
    QString text;
    QAction* action;
};
class EXPORT CommandHub : public QObject
{
    Q_OBJECT
public:
    explicit CommandHub(QObject *parent = 0);

    void registerAction(const ActionDescriptor& action);
    void unregisterAction(const QString& id);
    QAction* action(const QString& id) const;
    QStringList actionsForTag(const QString& tag) const;

signals:

public slots:

private:
    QList<ActionDescriptor> m_commands;

};

EXPORT CommandHub* getCommandHub();
void EXPORT createCommandHub();
void EXPORT destroyCommandHub();
}

#endif // COMMANDHUB_H
