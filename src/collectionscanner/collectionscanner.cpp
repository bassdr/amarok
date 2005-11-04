/***************************************************************************
 *   Copyright (C) 2003-2005 by The amaroK Developers                      *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Steet, Fifth Floor, Boston, MA  02111-1307, USA.          *
 ***************************************************************************/

#define DEBUG_PREFIX "CollectionScanner"

#include "collectionscanner.h"
#include "debug.h"

#include <iostream>

#include <taglib/audioproperties.h>
#include <taglib/fileref.h>
#include <taglib/id3v1genres.h> //used to load genre list
#include <taglib/mpegfile.h>
#include <taglib/tag.h>
#include <taglib/tstring.h>

#include <qdir.h>
#include <qdom.h>
#include <qfile.h>
#include <qtimer.h>

#include <kglobal.h>
#include <klocale.h>

/**
 * Use this to const-iterate over QStringLists, if you like.
 * Watch out for the definition of last in the scope of your for.
 *
 *     QStringList strings;
 *     foreach( strings )
 *         debug() << *it << endl;
 */
#define foreach( x ) \
    for( QStringList::ConstIterator it = x.begin(), end = x.end(); it != end; ++it )



CollectionScanner::CollectionScanner( const QStringList& folders, bool recursive, bool importPlaylists )
        : KApplication()
        , m_importPlaylists( importPlaylists )
        , m_folders( folders )
        , m_recursively( recursive )
        , log( "~/collection_scanner.log" ) //FIXME
{
    QTimer::singleShot( 0, this, SLOT( doJob() ) );
}


CollectionScanner::~CollectionScanner()
{
    DEBUG_BLOCK
}


void
CollectionScanner::doJob() //SLOT
{
    log << "Collection Scan Log\n";
    log << "===================\n";
    log << i18n( "Report this file if amaroK crashes when building the Collection." ).local8Bit();
    log << "\n\n\n";

    // we need to create the temp tables before readDir gets called ( for the dir stats )

    std::cout << "<?xml version='1.0' encoding='utf-8' ?>";
    std::cout << "<scanner>";

    QStringList entries;
    foreach( m_folders ) {
        if( (*it).isEmpty() )
            //apparently somewhere empty strings get into the mix
            //which results in a full-system scan! Which we can't allow
            continue;

        QString dir = *it;
        if( !dir.endsWith( "/" ) )
            dir += '/';

        readDir( dir, entries );
    }

    if( !entries.isEmpty() ) {
        AttributeMap attributes;
        attributes["count"] = QString::number( entries.count() );
        writeElement( "itemcount", attributes );

        scanFiles( entries );
    }

    std::cout << "</scanner>" << std::endl;
    log.close();

    quit();
}


void
CollectionScanner::readDir( const QString& path, QStringList& entries )
{
    // linux specific, but this fits the 90% rule
    if( path.startsWith( "/dev" ) || path.startsWith( "/sys" ) || path.startsWith( "/proc" ) )
        return;
    // Protect against dupes and symlink loops
    if( m_processedFolders.contains( path ) )
        return;

    AttributeMap attributes;
    attributes["path"] = path;
    writeElement( "folder", attributes );

    m_processedFolders += path;
    QDir dir( path );


    // FILES:
    const QStringList files = dir.entryList( QDir::Files | QDir::Readable );

    // Append file paths to list
    for( QStringList::ConstIterator it = files.begin(); it != files.end(); ++it )
        entries += dir.absFilePath( *it );


    if( !m_recursively ) return;


    // FOLDERS:
    const QStringList dirs = dir.entryList( QDir::Dirs | QDir::Readable );

    // Recurse folders
    for( QStringList::ConstIterator it = dirs.begin(); it != dirs.end(); ++it ) {
        if( (*it).startsWith( "." ) ) continue;
        readDir( dir.absFilePath( *it ), entries );
    }
}


void
CollectionScanner::scanFiles( const QStringList& entries )
{
    DEBUG_BLOCK

    typedef QPair<QString, QString> CoverBundle;

    QStringList validImages; validImages << "jpg" << "png" << "gif" << "jpeg";

    QValueList<CoverBundle> covers;
    QStringList images;

    for( QStringList::ConstIterator it = entries.begin(); it != entries.end(); ++it ) {

        const QString path = *it;
        const QString ext  = extension( path );
        const QString dir  = directory( path );

        // Append path to logfile
        log << path.local8Bit() << std::endl;
        log.flush();

        readTags( path );

        if( validImages.contains( ext ) )
           images += path;
    }
}


void
CollectionScanner::readTags( const QString& path )
{
    // Tests reveal the following:
    //
    // TagLib::AudioProperties   Relative Time Taken
    //
    //  No AudioProp Reading        1
    //  Fast                        1.18
    //  Average                     Untested
    //  Accurate                    Untested

    TagLib::FileRef fileref;
    TagLib::Tag *tag = 0;

    fileref = TagLib::FileRef( QFile::encodeName( path ), true, TagLib::AudioProperties::Fast );

    if( !fileref.isNull() )
        tag = fileref.tag();

    if( fileref.isNull() || !tag ) {
        std::cout << "<dud/>";
        return;
    }

    AttributeMap attributes;

    #define strip( x ) TStringToQString( x ).stripWhiteSpace()
    attributes["path"]    = path;
    attributes["title"]   = strip( tag->title() );
    attributes["artist"]  = strip( tag->artist() );
    attributes["album"]   = strip( tag->album() );
    attributes["comment"] = strip( tag->comment() );
    attributes["genre"]   = strip( tag->genre() );
    attributes["year"]    = tag->year() ? QString::number( tag->year() ) : QString();
    attributes["track"]   = tag->track() ? QString::number( tag->track() ) : QString();
    #undef strip

    TagLib::AudioProperties* ap = fileref.audioProperties();
    if( ap ) {
        attributes["audioproperties"] = "true";
        attributes["bitrate"]         = QString::number( ap->bitrate() );
        attributes["length"]          = QString::number( ap->length() );
        attributes["samplerate"]      = QString::number( ap->sampleRate() );
    }
    else
        attributes["audioproperties"] = "false";


    writeElement( "tags", attributes );
}


void
CollectionScanner::writeElement( const QString& name, const AttributeMap& attributes )
{
    QDomDocument doc; // A dummy. We don't really use DOM, but SAX2
    QDomElement element = doc.createElement( name );

    AttributeMap::ConstIterator it;
    for( it = attributes.begin(); it != attributes.end(); ++it )
        element.setAttribute( it.key(), it.data() );

    QString text;
    QTextStream stream( &text, IO_WriteOnly );
    element.save( stream, 0 );

    std::cout << text.local8Bit() << std::endl;
}


#include "collectionscanner.moc"

