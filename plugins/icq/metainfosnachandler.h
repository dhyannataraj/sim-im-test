/*
 * metainfosnachandler.h
 *
 *  Created on: Sep 15, 2011
 */

#ifndef METAINFOSNACHANDLER_H_
#define METAINFOSNACHANDLER_H_

#include "snac.h"
#include "icqcontact.h"
#include "bytearrayparser.h"
#include "bytearraybuilder.h"

#include <QList>
#include <QDateTime>

class ICQClient;
class MetaInfoSnacHandler : public SnacHandler
{
public:
    MetaInfoSnacHandler(ICQClient* client);
    virtual ~MetaInfoSnacHandler();

    virtual bool process(unsigned short subtype, const QByteArray& data, int flags, unsigned int requestId);

    void requestFullInfo(const ICQContactPtr& contact);

    void uploadBasicInfo();
    void uploadHomeInfo();
    void uploadWorkInfo();
    void uploadMoreInfo();
    void uploadAboutInfo();
    void uploadInterestsInfo();
    void uploadPastInfo();

    static const int SnacId = 0x15;

    static const int SnacMetaInfoRequest = 0x02;
    static const int SnacMetaInfoData = 0x03;

    static const int MetaInfoData = 0x7da;
    static const int MetaInfoRequest = 0x7d0;

    static const int MetaBasicUserInfo = 0xc8;
    static const int MetaWorkUserInfo = 0xd2;
    static const int MetaMoreUserInfo = 0xdc;
    static const int MetaAboutUserInfo = 0xe6;
    static const int MetaInterestsUserInfo = 0xf0;
    static const int MetaPastUserInfo = 0xfa;

    static const int MetaSetFullUserInfo = 0xc3a;

    static const int TlvFirstName = 0x140;
    static const int TlvLastName = 0x14a;
    static const int TlvNickname = 0x154;

    static const int TlvHomeCity = 0x190;
    static const int TlvHomeState = 0x19a;
    static const int TlvHomeCountry = 0x1a4;
    static const int TlvHomeAddress = 0x262;
    static const int TlvHomeZip = 0x26c;
    static const int TlvHomePhone = 0x276;
    static const int TlvHomeFax = 0x280;
    static const int TlvCellular = 0x28a;

    static const int TlvWorkCompany = 0x1ae;
    static const int TlvWorkDepartment = 0x1b8;
    static const int TlvWorkPosition = 0x1c2;
    static const int TlvWorkOccupation = 0x1cc;
    static const int TlvWorkAddress = 0x294;
    static const int TlvWorkCity = 0x29e;
    static const int TlvWorkState = 0x2a8;
    static const int TlvWorkCountry = 0x2b2;
    static const int TlvWorkZip = 0x2bc;
    static const int TlvWorkPhone = 0x2c6;
    static const int TlvWorkFax = 0x2d0;
    static const int TlvWorkHomepage = 0x2da;

    static const int TlvAge = 0x172;
    static const int TlvBirthday = 0x23a;
    static const int TlvLanguage = 0x186;

    static const int TlvAbout = 0x258;

    static const int TlvInterest = 0x1ea;

    static const int TlvBackground = 0x1fe;
    static const int TlvAffiliation = 0x1d6;


private:
    bool processMetaInfoData(const QByteArray& arr);

    bool parseBasicUserInfo(ByteArrayParser& parser, const ICQContactPtr& contact);
    bool parseWorkUserInfo(ByteArrayParser& parser, const ICQContactPtr& contact);
    bool parseMoreUserInfo(ByteArrayParser& parser, const ICQContactPtr& contact);
    bool parseAboutUserInfo(ByteArrayParser& parser, const ICQContactPtr& contact);
    bool parseInterestsUserInfo(ByteArrayParser& parser, const ICQContactPtr& contact);
    bool parsePastUserInfo(ByteArrayParser& parser, const ICQContactPtr& contact);

    void addMetaInfoRequest(int sqnum, const ICQContactPtr& contact);
    ICQContactPtr getMetaInfoRequestContact(int sqnum);

    QString readString(ByteArrayParser& parser);
    void appendString(ByteArrayBuilder& builder, const QString& str);

    QByteArray nullTerminatedStringWLength(const QString& str);
    QByteArray makeIcombo(int code, const QString& str);

private:
    int m_sqnum;

    struct MetaInfoRequestDescriptor
    {
        int sqnum;
        QDateTime timestamp;
        ICQContactPtr contact;
    };
    QList<MetaInfoRequestDescriptor> m_requests;
};

#endif /* METAINFOSNACHANDLER_H_ */
