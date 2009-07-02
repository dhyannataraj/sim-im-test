#include "simfs.h"
#include "icons.h"

#include <QDateTime>

SimFileEngine::SimFileEngine( const QString &fileName )
    : m_buffer( &m_data )
{
    m_fileName = fileName.right( fileName.length() - fileName.indexOf( ':' ) - 1 );
}

SimFileEngine::~SimFileEngine() {
}

SimFileEngine::Iterator *SimFileEngine::beginEntryList( QDir::Filters filters, const QStringList & filterNames ) {
    int ggg = 0;
}

bool SimFileEngine::caseSensitive() const {
   return true;
}

bool SimFileEngine::close() {
    m_buffer.close();
    return true;
}

bool SimFileEngine::copy( const QString & newName ) {
    int ggg = 0;
}

QStringList SimFileEngine::entryList( QDir::Filters filters, const QStringList & filterNames ) const {
    int ggg = 0;
}

bool SimFileEngine::extension( Extension extension, const ExtensionOption * option /* = 0 */, ExtensionReturn * output /* = 0 */ ) {
    int ggg = 0;
}

SimFileEngine::FileFlags SimFileEngine::fileFlags( FileFlags type /* = FileInfoAll */ ) const {
    int ggg = 0;
}

QString SimFileEngine::fileName( FileName file ) const {
    return "sim:" + m_fileName;
}

QDateTime SimFileEngine::fileTime( FileTime time ) const {
    return QDateTime();
}

bool SimFileEngine::flush() {
    return true;
}

int SimFileEngine::handle() const {
    return 0;
}

bool SimFileEngine::isRelativePath() const {
    int ggg = 0;
}

bool SimFileEngine::isSequential() const {
    return false;
}

bool SimFileEngine::link( const QString & newName ) {
    int ggg = 0;
}

bool SimFileEngine::mkdir( const QString & dirName, bool createParentDirectories ) const {
    return false;
}

bool SimFileEngine::open( QIODevice::OpenMode mode ) {
    if( mode & QIODevice::WriteOnly )
        return false;

    QString sType = m_fileName.left( m_fileName.indexOf( '/' ) ).toLower();
    QString sName = m_fileName.right( m_fileName.length() - m_fileName.indexOf( '/' ) - 1);

    if( sType == "icons" ) {
        QPixmap pict = SIM::Pict( sName );
        if( pict.isNull() )
            return false;
        if( !m_buffer.open( QIODevice::WriteOnly ) )
            return false;
        if( !pict.save( &m_buffer, "PNG" ) )
            return false;
        m_buffer.close();
        return m_buffer.open( mode );
    }
    else
        return false;

    return m_buffer.open( mode );
}

QString SimFileEngine::owner( FileOwner owner ) const {
    int ggg = 0;
}

uint SimFileEngine::ownerId( FileOwner owner ) const {
    int ggg = 0;
}

qint64 SimFileEngine::pos() const {
    int ggg = 0;
}

qint64 SimFileEngine::read( char * data, qint64 maxlen ) {
    return m_buffer.read( data, maxlen );
}

qint64 SimFileEngine::readLine( char * data, qint64 maxlen ) {
    int ggg = 0;
}

bool SimFileEngine::remove() {
    return false;
}

bool SimFileEngine::rename( const QString & newName ) {
    int ggg = 0;
}

bool SimFileEngine::rmdir( const QString & dirName, bool recurseParentDirectories ) const {
    return false;
}

bool SimFileEngine::seek( qint64 offset ) {
    return m_buffer.seek( offset );
}

void SimFileEngine::setFileName( const QString &file ) {
    int ggg = 0;
}

bool SimFileEngine::setPermissions( uint perms ) {
    int ggg = 0;
}

bool SimFileEngine::setSize( qint64 size ) {
    return false;
}

qint64 SimFileEngine::size() const {
    return m_data.size();
}

bool SimFileEngine::supportsExtension( Extension extension ) const {
    return false;
}

qint64 SimFileEngine::write( const char * data, qint64 len ) {
    return 0;
}


SimFileEngineHandler::SimFileEngineHandler() {
}

SimFileEngineHandler::~SimFileEngineHandler() {
}

QAbstractFileEngine *SimFileEngineHandler::create( const QString &fileName ) const {
    if( fileName.toLower().startsWith( "sim:" ) )
        return new SimFileEngine( fileName );
    else
        return NULL;
}
