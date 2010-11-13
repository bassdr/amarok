/****************************************************************************************
 * Copyright (c) 2007 Leo Franchi <lfranchi@gmail.com>                                  *
 * Copyright (c) 2009 simon.esneault <simon.esneault@gmail.com>                         *
 * Copyright (c) 2010 Daniel Faust <hessijames@gmail.com>                               *
 *                                                                                      *
 * This program is free software; you can redistribute it and/or modify it under        *
 * the terms of the GNU General Public License as published by the Free Software        *
 * Foundation; either version 2 of the License, or (at your option) any later           *
 * version.                                                                             *
 *                                                                                      *
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY      *
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A      *
 * PARTICULAR PURPOSE. See the GNU General Public License for more details.             *
 *                                                                                      *
 * You should have received a copy of the GNU General Public License along with         *
 * this program.  If not, see <http://www.gnu.org/licenses/>.                           *
 ****************************************************************************************/

#ifndef LABELS_APPLET_H
#define LABELS_APPLET_H

#include "context/Applet.h"
#include "context/DataEngine.h"

#include <ui_labelsSettings.h>

#include <QWeakPointer>

class LabelGraphicsItem;
class TextScrollingWidget;
class KComboBox;
class QGraphicsProxyWidget;


class LabelsApplet : public Context::Applet
{
    Q_OBJECT

public:
    LabelsApplet( QObject *parent, const QVariantList &args );
    ~LabelsApplet();

    void constraintsEvent( Plasma::Constraints constraints = Plasma::AllConstraints );

public slots:
    virtual void init();
    void dataUpdated( const QString &name, const Plasma::DataEngine::Data &data );
    void connectSource( const QString &source );
    void toggleLabel( const QString &label );
    void listLabel( const QString &label );
    void blacklistLabel( const QString &label );
    void addLabelPressed();
    void saveSettings();
    
protected:
    void createConfigurationInterface( KConfigDialog *parent );

private:
    void updateLabels();
    void setStoppedState( bool stopped );

    QWeakPointer<Plasma::IconWidget>   m_reloadIcon;
    QWeakPointer<Plasma::IconWidget>   m_settingsIcon;
    QString                            m_titleText;
    QWeakPointer<TextScrollingWidget>  m_titleLabel;
    QWeakPointer<QGraphicsProxyWidget> m_addLabelProxy;
    QWeakPointer<KComboBox>            m_addLabel;

    QStringList                     m_allLabels;
    QStringList                     m_userLabels;
    QMap < QString, QVariant >      m_webLabels;

    // the list of the active label items and their animations - both lists have to be in sync
    QList < LabelGraphicsItem * >   m_labelItems;
    QList < QPropertyAnimation * >  m_labelAnimations;

    // the list of the label items that are about to be delete and are "flying out" and their animations - both lists have to be in sync
    QList < LabelGraphicsItem * >   m_labelItemsToDelete;
    QList < QPropertyAnimation * >  m_labelAnimationsToDelete;

    // configuration values
    int                     m_numLabels;
    int                     m_minCount;
    int                     m_personalCount;
    bool                    m_autoAdd;
    int                     m_minAutoAddCount;
    bool                    m_matchArtist;
    bool                    m_matchTitle;
    bool                    m_matchAlbum;
    QStringList             m_blacklist;
    QColor                  m_selectedColor;
    
    bool                    m_stoppedstate;
    QString                 m_artist;
    QString                 m_title;
    QString                 m_album;

    // if the user adds a label through the combobox, the animation should start at the bottom
    bool                    m_selfAdded;
    
    Ui::labelsSettings      ui_Settings;

private slots:
    void reload();
    void animationFinished();

};

K_EXPORT_AMAROK_APPLET( labels, LabelsApplet )

#endif
