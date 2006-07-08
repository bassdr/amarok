#ifndef AMAROK_DAAPCLIENT_H
#define AMAROK_DAAPCLIENT_H

#include "mediabrowser.h"
#include "daapreader/reader.h"

#include <dnssd/remoteservice.h> //for DNSSD::RemoteService::Ptr

namespace DNSSD {
    class ServiceBrowser;
}

class QString;
class MediaItem;

class DaapClient : public MediaDevice
{

    Q_OBJECT

    public:
         DaapClient();
         virtual ~DaapClient();
         bool isConnected();
    protected:
         bool getCapacity( KIO::filesize_t *total, KIO::filesize_t *available );
         bool lockDevice( bool tryOnly = false );
         void unlockDevice();
         bool openDevice( bool silent=false );
         bool closeDevice();
         void synchronizeDevice();
         MediaItem* copyTrackToDevice(const MetaBundle& bundle);
         MediaItem* trackExists( const MetaBundle& );
         int    deleteItemFromDevice( MediaItem *item, bool onlyPlayed = false, bool deleteTrack = true );
   private slots:
         void foundDaap( DNSSD::RemoteService::Ptr );
         void resolvedDaap( bool );
         void createTree( const QString& host, Daap::SongList bundles );
   private:
        DNSSD::ServiceBrowser* m_browser;
        bool    m_connected;
        QMap<const char*, MediaItem*> m_servers;
};

#endif /*AMAROK_DAAPCLIENT_H*/
