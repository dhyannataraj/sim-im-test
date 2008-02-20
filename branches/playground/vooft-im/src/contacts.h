#ifndef __CONTACTS_H__
#define __CONTACTS_H__

#include <QImage>
#include <QMap>
#include <QString>
#include <QObject> 

enum
{
	SOffline,
	SOnline,
	SAway,
	SNA,
	SBusy,
	
END_OF_STATUSES
};

// place your code here
class SContact: public QObject
{
	Q_OBJECT
	
	QString m_proto;
	QString m_ID;
	quint16 m_status;
	QMap<QString, QString> m_extras;
	QImage *m_img;
	
private slots:
	void contactChanged();
public:
	SContact(QString proto, QString id, quint16 status=SOffline, QImage *img=0);
	~SContact();
	QString getProto() { return m_proto; }
	QString getId() { return m_ID; }
	quint16 getStatus() { return m_status; }
	QMap<QString, QString> getExtras() const { return m_extras; }

public slots:
	void setImage(QImage*);
	void addExtraProps(QMap<QString, QString>);
	void clearExtraProps();
	void setExtraProps(QMap<QString, QString>);
	void setStatus(quint16);
	void clearImg();
	
signals:
	void statusChanged(quint16);
	void imgChanged(const QImage&);
	void extraPropsChanged(QMap<QString, QString>);
	void changed(SContact*);
	void changed(QString, QString);
};

class SGroup: public QObject
{
	Q_OBJECT
	
	QList<SContact *> contacts;
	QString m_ID;

public:
	SGroup(QString);
	~SGroup();
	SContact* getContact(QString proto, QString id);

public slots:
	void pushContact(SContact*);
	void deleteContact(QString proto, QString id);
	void contactChanged(SContact*);
	void contactChanged(QString proto, QString id);
	void changeName(QString);
	
signals:
	void nameChanged(QString);
	void contactAdded(SContact*);
	void groupStaffChanged();
	void contactChanged(int);
	
};

#endif // __CONTACTS_H__
