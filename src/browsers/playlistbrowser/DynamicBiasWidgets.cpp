/***************************************************************************
 * copyright         : (C) 2008 Daniel Caleb Jones <danielcjones@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License or (at your option) version 3 or any later version
 * accepted by the membership of KDE e.V. (or its successor approved
 * by the membership of KDE e.V.), which shall act as a proxy
 * defined in Section 14 of version 3 of the license.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 **************************************************************************/

#include "DynamicBiasWidgets.h"

#include "App.h"
#include "Bias.h"
#include "BiasedPlaylist.h"
#include "BlockingQuery.h"
#include "CollectionManager.h"
#include "Debug.h"
#include "DynamicPlaylist.h"
#include "DynamicBiasModel.h"
#include "DynamicModel.h"
#include "MetaQueryMaker.h"
#include "QueryMaker.h"
#include "SliderWidget.h"
#include "SvgHandler.h"
#include "collection/support/XmlQueryWriter.h"
#include "widgets/kratingwidget.h"

#include <typeinfo>

#include <QDateTimeEdit>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QListView>
#include <QPaintEvent>
#include <QPainter>
#include <QSpinBox>
#include <QTimeEdit>
#include <QToolButton>

#include <KComboBox>
#include <KHistoryComboBox>
#include <KIcon>
#include <KToolBar>
#include <KVBox>

PlaylistBrowserNS::BiasBoxWidget::BiasBoxWidget( QWidget* parent )
    : QWidget( parent ), m_alternate(false)
{
}

void
PlaylistBrowserNS::BiasBoxWidget::paintEvent( QPaintEvent* e )
{
    Q_UNUSED(e)


    // is it showing ?
    QListView* parentList = dynamic_cast<QListView*>(parent());
    if( parentList )
    {
        PlaylistBrowserNS::DynamicBiasModel* model =
            dynamic_cast<PlaylistBrowserNS::DynamicBiasModel*>( parentList->model() );
        if( model )
        {
            QRect rect = parentList->visualRect( model->indexOf( this ) );
            if( rect.x() < 0 || rect.y() < 0 || rect.height() < height() )
            {
                hide();
                return;
            }
        }
    }

    QPainter painter(this);
    painter.setRenderHint( QPainter::Antialiasing, true );

    QPixmap body;

    if( alternate() )
        body = The::svgHandler()->renderSvgWithDividers( "alt_body", width(), height(), "alt_body" );
    else
        body = The::svgHandler()->renderSvgWithDividers( "body", width(), height(), "body" );

    painter.drawPixmap( 0, 0, body );

    painter.end();
}

void
PlaylistBrowserNS::BiasBoxWidget::resizeEvent( QResizeEvent* )
{
    emit widgetChanged( this );
}



PlaylistBrowserNS::BiasAddWidget::BiasAddWidget( QWidget* parent )
    : BiasBoxWidget( parent )
{
    DEBUG_BLOCK

    QHBoxLayout* mainLayout = new QHBoxLayout( this );

    QWidget* m_addToolbarWidget = new QWidget( this );
    m_addToolbarWidget->setMinimumSize( 30, 30 );

    m_addToolbar = new KToolBar( m_addToolbarWidget );
    m_addToolbar->setFixedHeight( 30 );
    m_addToolbar->setContentsMargins( 0, 0, 0, 0 );
    m_addToolbar->setToolButtonStyle( Qt::ToolButtonIconOnly );
    m_addToolbar->setSizePolicy( QSizePolicy::Preferred, QSizePolicy::Preferred );
    m_addToolbar->setIconDimensions( 22 );
    m_addToolbar->setMovable( false );
    m_addToolbar->setFloatable ( false );

    m_addButton = new QToolButton( m_addToolbar );
    m_addButton->setIcon( KIcon( "list-add-amarok" ) );
    m_addButton->setToolTip( i18n( "Add a new bias." ) );
    m_addToolbar->addWidget( m_addButton );
    m_addToolbar->adjustSize();
    connect( m_addButton, SIGNAL( clicked() ), SLOT( addBias() ) );

    m_addLabel = new QLabel( "New Bias", this );
    m_addLabel->setAlignment( Qt::AlignCenter | Qt::AlignVCenter );
    QFont font;
    font.setPointSize( 16 );
    m_addLabel->setFont( font );

    mainLayout->addWidget( m_addToolbarWidget );
    mainLayout->setStretchFactor( m_addToolbarWidget, 0 );
    mainLayout->setAlignment( m_addToolbarWidget, Qt::AlignLeft | Qt::AlignVCenter );
    mainLayout->addWidget( m_addLabel );
    mainLayout->setStretchFactor( m_addLabel, 1 );

    connect( this, SIGNAL( clicked() ), SLOT( addBias() ) );

    setLayout( mainLayout );
}

void
PlaylistBrowserNS::BiasAddWidget::mouseReleaseEvent( QMouseEvent* )
{
    emit clicked();
}

void
PlaylistBrowserNS::BiasAddWidget::addBias()
{
    DEBUG_BLOCK

    Dynamic::GlobalBias* gb = new Dynamic::GlobalBias( 0.0, XmlQueryReader::Filter() );
    gb->setActive( false );

    emit addBias( gb );
}




PlaylistBrowserNS::BiasWidget::BiasWidget( Dynamic::Bias* b, QWidget* parent )
    : BiasBoxWidget( parent ), m_bias(b)
{
    DEBUG_BLOCK

    QHBoxLayout* hLayout = new QHBoxLayout( this );

    QWidget* removeToolbarWidget = new QWidget(this);
    removeToolbarWidget->setMinimumSize( 30, 30 );

    m_removeToolbar = new KToolBar(removeToolbarWidget);
    m_removeToolbar->setFixedHeight( 30 );
    m_removeToolbar->setContentsMargins( 0, 0, 0, 0 );
    m_removeToolbar->setToolButtonStyle( Qt::ToolButtonIconOnly );
    m_removeToolbar->setSizePolicy( QSizePolicy::Preferred, QSizePolicy::Preferred );
    m_removeToolbar->setIconDimensions( 22 );
    m_removeToolbar->setMovable( false );
    m_removeToolbar->setFloatable ( false );

    QToolButton* removeButton = new QToolButton( m_removeToolbar );
    removeButton->setIcon( KIcon( "list-remove-amarok" ) );
    removeButton->setToolTip( i18n( "Remove this bias." ) );
    connect( removeButton, SIGNAL( clicked() ), SLOT( biasRemoved() ), Qt::QueuedConnection );
    m_removeToolbar->addWidget( removeButton );
    m_removeToolbar->adjustSize();


    m_mainLayout = new KVBox( this );

    hLayout->addWidget( removeToolbarWidget );
    hLayout->setStretchFactor( removeToolbarWidget, 0 );
    hLayout->setAlignment( removeToolbarWidget, Qt::AlignLeft | Qt::AlignVCenter );

    hLayout->addWidget( m_mainLayout );

    setLayout( hLayout );
}

void
PlaylistBrowserNS::BiasWidget::biasRemoved()
{
    emit biasRemoved( m_bias );
}


PlaylistBrowserNS::BiasGlobalWidget::BiasGlobalWidget(
        Dynamic::GlobalBias* bias, QWidget* parent )
    : BiasWidget( bias, parent )
    , m_withLabel(0)
    , m_valueSelection(0)
    , m_compareSelection(0)
    , m_gbias( bias )
    , m_filter( bias->filter() )
{
    DEBUG_BLOCK

    m_controlFrame = new QFrame( m_mainLayout );
    m_controlLayout = new QGridLayout( m_controlFrame );
    m_controlFrame->setLayout( m_controlLayout );

    QHBoxLayout* sliderLayout = new QHBoxLayout();
    m_controlLayout->addLayout( sliderLayout, 0, 1 );

    m_weightLabel = new QLabel( " 0%", m_controlFrame );
    m_weightSelection = new Amarok::Slider( Qt::Horizontal, m_controlFrame, 100 );
    m_weightSelection->setToolTip(
            i18n( "This controls what portion of the playlist should match the criteria" ) );
    connect( m_weightSelection, SIGNAL(valueChanged(int)),
            this, SLOT(weightChanged(int)) );

    m_fieldSelection = new KComboBox( m_controlFrame );
    m_fieldSelection->setPalette( QApplication::palette() );

    m_controlLayout->addWidget( new QLabel( "Proportion:", m_controlFrame ), 0, 0 );
    m_controlLayout->addWidget( new QLabel( "Match:", m_controlFrame ), 1, 0 );

    m_controlLayout->addWidget( m_weightSelection, 0, 1 );
    m_controlLayout->addWidget( m_weightLabel, 0, 1 );
    m_controlLayout->addWidget( m_fieldSelection, 1, 1 );

    sliderLayout->addWidget( m_weightSelection );
    sliderLayout->addWidget( m_weightLabel );

    popuplateFieldSection();
    connect( m_fieldSelection, SIGNAL(currentIndexChanged(int)),
            this, SLOT(fieldChanged(int)) );
    m_fieldSelection->setCurrentIndex( 0 );
    
    syncControlsToBias();
}

void
PlaylistBrowserNS::BiasGlobalWidget::syncBiasToControls()
{
    m_gbias->setQuery( m_filter );
    m_gbias->setActive( true );

    emit biasChanged( m_bias );
}

    void
PlaylistBrowserNS::BiasGlobalWidget::syncControlsToBias()
{
    int index = m_fieldSelection->findData( m_filter.field );
    m_fieldSelection->setCurrentIndex( index == -1 ? 0 : index );

    m_weightSelection->setValue( (int)(m_gbias->weight() * 100.0) );
}


    void
PlaylistBrowserNS::BiasGlobalWidget::popuplateFieldSection()
{
    m_fieldSelection->addItem( "", (qint64)0 );
    m_fieldSelection->addItem( "Artist", QueryMaker::valArtist );
    m_fieldSelection->addItem( "Composer", QueryMaker::valComposer );
    m_fieldSelection->addItem( "Album",  QueryMaker::valAlbum );
    m_fieldSelection->addItem( "Title",  QueryMaker::valTitle );
    m_fieldSelection->addItem( "Genre",  QueryMaker::valGenre );
    m_fieldSelection->addItem( "Year",   QueryMaker::valYear );
    m_fieldSelection->addItem( "Play Count", QueryMaker::valPlaycount );
    m_fieldSelection->addItem( "Rating", QueryMaker::valRating );
    m_fieldSelection->addItem( "Score", QueryMaker::valScore );
    m_fieldSelection->addItem( "Length", QueryMaker::valLength );
    m_fieldSelection->addItem( "Track #", QueryMaker::valTrackNr );
    m_fieldSelection->addItem( "Disc #", QueryMaker::valDiscNr );
    m_fieldSelection->addItem( "First Played", QueryMaker::valFirstPlayed );
    m_fieldSelection->addItem( "Last Played", QueryMaker::valLastPlayed );
    m_fieldSelection->addItem( "Comment", QueryMaker::valComment );
}

    void
PlaylistBrowserNS::BiasGlobalWidget::weightChanged( int ival )
{
    double fval = (double)ival;
    m_weightLabel->setText( QString().sprintf( "%2.0f%%", fval ) );

    m_gbias->setWeight( fval / 100.0 );

    emit biasChanged( m_bias );
}

    void
PlaylistBrowserNS::BiasGlobalWidget::fieldChanged( int i )
{
    qint64 field = qvariant_cast<qint64>( m_fieldSelection->itemData( i ) );
    m_filter.field = field;
    if( field != m_gbias->filter().field )
        m_filter.value.clear();

    if( field == 0 )
    {
        delete m_compareSelection;
        m_compareSelection = 0;
        delete m_withLabel;
        m_withLabel = 0;
        delete m_valueSelection;
        m_valueSelection = 0;
    }
    else if( field == QueryMaker::valArtist )
        makeArtistSelection();
    else if( field == QueryMaker::valComposer )
        makeComposerSelection();
    else if( field == QueryMaker::valAlbum )
        makeAlbumSelection();
    else if( field == QueryMaker::valTitle )
        makeTitleSelection();
    else if( field == QueryMaker::valGenre )
        makeGenreSelection();
    else if( field == QueryMaker::valYear )
        makeGenericNumberSelection( 0, 3000, 0 );
    else if( field == QueryMaker::valPlaycount )
        makeGenericNumberSelection( 0, 10000, 0 );
    else if( field == QueryMaker::valRating )
        makeRatingSelection();
    else if( field == QueryMaker::valScore )
        makeGenericNumberSelection( 0, 100, 0 );
    else if( field == QueryMaker::valLength )
        makeLengthSelection();
    else if( field == QueryMaker::valTrackNr )
        makeGenericNumberSelection( 0, 1000, 0 );
    else if( field == QueryMaker::valDiscNr )
        makeGenericNumberSelection( 0, 1000, 0 );
    else if( field == QueryMaker::valFirstPlayed )
        makeDateTimeSelection();
    else if( field == QueryMaker::valLastPlayed )
        makeDateTimeSelection();
    else if( field == QueryMaker::valComment )
        makeGenericComboSelection( true, 0 );

    //etc
}

    void
PlaylistBrowserNS::BiasGlobalWidget::compareChanged( int index )
{
    if( index < 0 )
        m_filter.compare = -1;
    else if( index < m_compareSelection->count() )
    {
        m_filter.compare = 
            qvariant_cast<int>(
                    m_compareSelection->itemData( index ) );
    }

    syncBiasToControls();
}


void
PlaylistBrowserNS::BiasGlobalWidget::setValueSection( QWidget* w )
{
    if( m_withLabel == 0 )
    {
        m_withLabel = new QLabel( "With:", m_controlFrame );
        m_controlLayout->addWidget( m_withLabel, 2, 0 );
    }

    delete m_valueSelection;
    m_valueSelection = w;
    m_controlLayout->addWidget( m_valueSelection, 2, 1 );

}

void
PlaylistBrowserNS::BiasGlobalWidget::valueChanged( int value )
{
    m_filter.value = QString::number( value );
    syncBiasToControls();
}

void
PlaylistBrowserNS::BiasGlobalWidget::valueChanged( const QString& value )
{
    m_filter.value = value;
    syncBiasToControls();
}

void
PlaylistBrowserNS::BiasGlobalWidget::valueChanged( const QDateTime& value )
{
    m_filter.value = QString::number( value.toTime_t() );
    syncBiasToControls();
}

void
PlaylistBrowserNS::BiasGlobalWidget::valueChanged( const QTime& value )
{
    m_filter.value = QString::number( qAbs( value.secsTo( QTime(0,0,0) ) ) );
    syncBiasToControls();
}


void
PlaylistBrowserNS::BiasGlobalWidget::makeCompareSelection( QWidget* parent )
{
    m_compareSelection = new KComboBox( parent );
    m_compareSelection->setPalette( QApplication::palette() );
    m_compareSelection->addItem( "", -1 );
    m_compareSelection->addItem( "less than",    (int)QueryMaker::LessThan );
    m_compareSelection->addItem( "equal to",     (int)QueryMaker::Equals );
    m_compareSelection->addItem( "greater than", (int)QueryMaker::GreaterThan );

    connect( m_compareSelection, SIGNAL(currentIndexChanged(int)),
            SLOT(compareChanged(int)) );

    int val = m_gbias->filter().compare;

    if( val == -1 )
        m_compareSelection->setCurrentIndex( 0 );
    if( val == (int)QueryMaker::LessThan )
        m_compareSelection->setCurrentIndex( 1 );
    else if( val == (int)QueryMaker::Equals )
        m_compareSelection->setCurrentIndex( 2 );
    else if( val == (int)QueryMaker::GreaterThan )
        m_compareSelection->setCurrentIndex( 3 );

}

void
PlaylistBrowserNS::BiasGlobalWidget::makeGenericComboSelection( bool editable, QueryMaker* populateQuery )
{
    KComboBox* combo = new KComboBox( m_controlFrame );
    combo->setPalette( QApplication::palette() );
    combo->setSizePolicy( QSizePolicy::Ignored, QSizePolicy::Preferred );
    combo->setEditable( editable );

    if( populateQuery != 0 )
    {
        populateQuery->returnResultAsDataPtrs( true );
        BlockingQuery bq( populateQuery );

        bq.startQuery();

        QSet<QString> dataSet;
        foreach( Meta::DataList as, bq.data() )
        {
            foreach( Meta::DataPtr a, as )
            {
                dataSet.insert( a->name() );
            }
        }

        QStringList dataList = dataSet.toList();
        dataList.sort();
        foreach( QString item, dataList )
            combo->addItem( item );
    }

    connect( combo, SIGNAL(currentIndexChanged( const QString& )),
            SLOT(valueChanged(const QString&)) );
    connect( combo, SIGNAL(editTextChanged( const QString& ) ),
            SLOT(valueChanged(const QString&)) );

    combo->setEditText( m_gbias->filter().value );

    combo->setAutoCompletion( true );
    setValueSection( combo );
}

void
PlaylistBrowserNS::BiasGlobalWidget::makeArtistSelection()
{
    QueryMaker* qm = new MetaQueryMaker( CollectionManager::instance()->queryableCollections() );
    qm->setQueryType( QueryMaker::Artist );
    makeGenericComboSelection( true, qm );
}


void
PlaylistBrowserNS::BiasGlobalWidget::makeComposerSelection()
{
    QueryMaker* qm = new MetaQueryMaker( CollectionManager::instance()->queryableCollections() );
    qm->setQueryType( QueryMaker::Composer );
    makeGenericComboSelection( true, qm );
}


void
PlaylistBrowserNS::BiasGlobalWidget::makeAlbumSelection()
{
    QueryMaker* qm = new MetaQueryMaker( CollectionManager::instance()->queryableCollections() );
    qm->setQueryType( QueryMaker::Album );
    makeGenericComboSelection( true, qm );
}


void
PlaylistBrowserNS::BiasGlobalWidget::makeTitleSelection()
{
    // We,re not going to populate this. There tends to be too many titles.
    makeGenericComboSelection( true, 0 );
}


void
PlaylistBrowserNS::BiasGlobalWidget::makeGenreSelection()
{
    QueryMaker* qm = new MetaQueryMaker( CollectionManager::instance()->queryableCollections() );
    qm->setQueryType( QueryMaker::Genre );
    makeGenericComboSelection( true, qm );
}


void
PlaylistBrowserNS::BiasGlobalWidget::makeRatingSelection()
{
    KHBox* hLayout = new KHBox( m_controlFrame );

    makeCompareSelection( hLayout );

    KRatingWidget* ratingWidget = new KRatingWidget( hLayout );

    connect( ratingWidget, SIGNAL(ratingChanged(int)),
             this, SLOT(valueChanged(int)) );
    
    ratingWidget->setRating( m_gbias->filter().value.toInt() );
    setValueSection( hLayout );
}


void
PlaylistBrowserNS::BiasGlobalWidget::makeLengthSelection()
{
    KHBox* hLayout = new KHBox( m_controlFrame );

    makeCompareSelection( hLayout );

    QTimeEdit* timeSpin = new QTimeEdit( hLayout );
    timeSpin->setDisplayFormat( "m:ss" );
    QTime min( 0, 0, 0 );
    QTime max( 0, 60, 59 );
    timeSpin->setMinimumTime( min );
    timeSpin->setMaximumTime( max );

    connect( timeSpin, SIGNAL(timeChanged(const QTime&)),
            SLOT(valueChanged(const QTime&)) );

    timeSpin->setTime( QTime().addSecs( m_gbias->filter().value.toInt() ) );

    setValueSection( hLayout );
}

void
PlaylistBrowserNS::BiasGlobalWidget::makeGenericNumberSelection( int min, int max, int def )
{
    KHBox* hLayout = new KHBox( m_controlFrame );

    makeCompareSelection( hLayout );

    QSpinBox* spin = new QSpinBox( hLayout );
    spin->setMinimum( min );
    spin->setMaximum( max );

    connect( spin, SIGNAL(valueChanged( const QString& )),
            this, SLOT(valueChanged(const QString&)) );

    if( m_gbias->filter().value.isEmpty() )
        spin->setValue( def );
    else
        spin->setValue( m_gbias->filter().value.toInt() );

    setValueSection( hLayout );
}


void
PlaylistBrowserNS::BiasGlobalWidget::makeDateTimeSelection()
{
    KHBox* hLayout = new KHBox( m_controlFrame );

    makeCompareSelection( hLayout );

    QDateTimeEdit* dateTimeSpin = new QDateTimeEdit( hLayout );

    connect( dateTimeSpin, SIGNAL(dateTimeChanged(const QDateTime&)),
             SLOT(valueChanged(const QDateTime&)) );

    QDateTime dt;
    if( m_gbias->filter().value.isEmpty() )
        dt = QDateTime::currentDateTime();
    else
        dt.setTime_t( m_gbias->filter().value.toUInt() );

    dateTimeSpin->setDateTime( dt );

    setValueSection( hLayout );
}


#include "DynamicBiasWidgets.moc"

