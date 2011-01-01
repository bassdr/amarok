/****************************************************************************************
 * Copyright (c) 2008 Daniel Jones <danielcjones@gmail.com>                             *
 * Copyright (c) 2009 Leo Franchi <lfranchi@kde.org>                                    *
 * Copyright (c) 2010, 2011 Ralf Engels <ralf-engels@gmx.de>                                  *
 *                                                                                      *
 * This program is free software; you can redistribute it and/or modify it under        *
 * the terms of the GNU General Public License as published by the Free Software        *
 * Foundation; either version 2 of the License, or (at your option) version 3 or        *
 * any later version accepted by the membership of KDE e.V. (or its successor approved  *
 * by the membership of KDE e.V.), which shall act as a proxy defined in Section 14 of  *
 * version 3 of the license.                                                            *
 *                                                                                      *
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY      *
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A      *
 * PARTICULAR PURPOSE. See the GNU General Public License for more details.             *
 *                                                                                      *
 * You should have received a copy of the GNU General Public License along with         *
 * this program.  If not, see <http://www.gnu.org/licenses/>.                           *
 ****************************************************************************************/

#ifndef AMAROK_METATAGBIAS_H
#define AMAROK_METATAGBIAS_H

#include "Bias.h"
#include <widgets/MetaQueryWidget.h>

namespace Dynamic
{
    class TagMatchBias : public AbstractBias
    {
        Q_OBJECT

        public:
            TagMatchBias();
            TagMatchBias( QXmlStreamReader *reader );

            void toXml( QXmlStreamWriter *writer ) const;

            static QString sName();
            virtual QString name() const;

            virtual QWidget* widget( QWidget* parent = 0 );

            virtual TrackSet matchingTracks( int position,
                                             const Meta::TrackList& playlist, int contextCount,
                                             const TrackCollectionPtr universe ) const;

            virtual bool trackMatches( int position,
                                       const Meta::TrackList& playlist,
                                       int contextCount ) const;

            MetaQueryWidget::Filter filter() const;
            void setFilter( const MetaQueryWidget::Filter &filter );

        public slots:
            virtual void invalidate();

        protected slots:
            /** Called when we get new uids from the query maker */
            void updateReady( QString collectionId, QStringList uids );

            /** Called when the querymaker is finished */
            void updateFinished();

            /** Creates a new query to get matching tracks. */
            void newQuery() const;

        protected:

            static QString nameForCondition( MetaQueryWidget::FilterCondition cond );
            static MetaQueryWidget::FilterCondition conditionForName( const QString &name );

            bool matches( const Meta::TrackPtr &track ) const;

            MetaQueryWidget::Filter m_filter;

            mutable QScopedPointer<Collections::QueryMaker> m_qm;

            /** The result from the current query manager are buffered in the m_uids set. */
            bool m_tracksValid;
            mutable TrackSet m_tracks;

        private:
            Q_DISABLE_COPY(TagMatchBias)
    };

}

#endif
