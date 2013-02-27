# include "brainzlineedit.h"

# include <QKeyEvent>

# include "versioninfo.h"

#include <iostream>
# include <musicbrainz5/Query.h>
# include <musicbrainz5/Metadata.h>
# include <musicbrainz5/ReleaseList.h>
# include <musicbrainz5/Release.h>
# include <musicbrainz5/ArtistList.h>
# include <musicbrainz5/Artist.h>
# include <musicbrainz5/HTTPFetch.h>
# include <musicbrainz5/Disc.h>

BrainzLineEdit::BrainzLineEdit(QWidget *parent) : QLineEdit(parent) {
    this->suggestionModel = new QStringListModel(this);
    this->_completer = new QCompleter(this->suggestionModel, this);
    this->setCompleter(this->_completer);
}

void BrainzLineEdit::keyPressEvent(QKeyEvent *ev) {
    if (ev->key() == Qt::Key_C && ev->modifiers() == Qt::ControlModifier) {
        ev->ignore();
        this->prepareCompleterData();
        this->suggestionModel->setStringList(this->suggestionList);
        this->completer()->complete();
        //connect(this, SIGNAL(textChanged(QString)), this->_completer, SLOT(complete()));
    } else if (ev->key() != Qt::Key_Down && ev->key() != Qt::Key_Up) {
        QLineEdit::keyPressEvent(ev);
        //disconnect(this->completer());
    } else {
        QLineEdit::keyPressEvent(ev);
    }
}

void BrainzLineEdit::setActiveColumn(int col) {
    this->activeColumn = col;
}

void BrainzLineEdit::setAdditionalInfo(const QString &a, const QString &r) {
    this->artist  = a;
    this->release = r;
}

void BrainzLineEdit::prepareCompleterData(void) {
    MusicBrainz5::CQuery Query(PROGNAME "-" VERSION);
    MusicBrainz5::CQuery::tParamMap pmap;

    /* TODO: write function that generates the pmap
     *       write functions that parse the data for the different query types
     *       write function to give the data to the completer */
    pmap = this->generateParameterMap();

    try {
        MusicBrainz5::CMetadata metaData = Query.Query(queryType.toStdString(),"","",pmap);

        /* act according to current query type */
        this->suggestionList.clear();

        switch (this->activeColumn) {
        /* artists */
        case 0:
            if (metaData.ArtistList()) this->loadArtistList(metaData.ArtistList());
            break;
        case 1:
            if (metaData.ReleaseList()) this->loadReleaseList(metaData.ReleaseList());
            break;
/*        case 2:
            if (metaData.LabelList()) this->loadTitleList(metaData.LabelList());
            break;*/
        }
    }

    catch (MusicBrainz5::CConnectionError& Error) {
        std::cout << "Connection Exception: '" << Error.what() << "'" << std::endl;
        std::cout << "LastResult: " << Query.LastResult() << std::endl;
        std::cout << "LastHTTPCode: " << Query.LastHTTPCode() << std::endl;
        std::cout << "LastErrorMessage: " << Query.LastErrorMessage() << std::endl;
    }

    catch (MusicBrainz5::CTimeoutError& Error) {
        std::cout << "Timeout Exception: '" << Error.what() << "'" << std::endl;
        std::cout << "LastResult: " << Query.LastResult() << std::endl;
        std::cout << "LastHTTPCode: " << Query.LastHTTPCode() << std::endl;
        std::cout << "LastErrorMessage: " << Query.LastErrorMessage() << std::endl;
    }

    catch (MusicBrainz5::CAuthenticationError& Error) {
        std::cout << "Authentication Exception: '" << Error.what() << "'" << std::endl;
        std::cout << "LastResult: " << Query.LastResult() << std::endl;
        std::cout << "LastHTTPCode: " << Query.LastHTTPCode() << std::endl;
        std::cout << "LastErrorMessage: " << Query.LastErrorMessage() << std::endl;
    }

    catch (MusicBrainz5::CFetchError& Error) {
        std::cout << "Fetch Exception: '" << Error.what() << "'" << std::endl;
        std::cout << "LastResult: " << Query.LastResult() << std::endl;
        std::cout << "LastHTTPCode: " << Query.LastHTTPCode() << std::endl;
        std::cout << "LastErrorMessage: " << Query.LastErrorMessage() << std::endl;
    }

    catch (MusicBrainz5::CRequestError& Error) {
        std::cout << "Request Exception: '" << Error.what() << "'" << std::endl;
        std::cout << "LastResult: " << Query.LastResult() << std::endl;
        std::cout << "LastHTTPCode: " << Query.LastHTTPCode() << std::endl;
        std::cout << "LastErrorMessage: " << Query.LastErrorMessage() << std::endl;
    }

    catch (MusicBrainz5::CResourceNotFoundError& Error) {
        std::cout << "ResourceNotFound Exception: '" << Error.what() << "'" << std::endl;
        std::cout << "LastResult: " << Query.LastResult() << std::endl;
        std::cout << "LastHTTPCode: " << Query.LastHTTPCode() << std::endl;
        std::cout << "LastErrorMessage: " << Query.LastErrorMessage() << std::endl;
    }
}


/* private methods =========================== */
std::map<std::string, std::string> BrainzLineEdit::generateParameterMap(void) {
    std::map<std::string, std::string> pmap;
    QString queryBuffer;

    switch (this->activeColumn) {
    /* artist field */
    case 0:
        this->queryType = QString("artist");
        queryBuffer = this->queryType;
        queryBuffer.append(this->text());
        break;

    /* release field */
    case 1:
        this->queryType = QString("release");
        queryBuffer = QString("artist:");
        queryBuffer.append(this->artist);
        queryBuffer.append("&release");
        queryBuffer.append(this->text());
        break;

    /* title field */
    case 2:
        this->queryType = QString("title");
        queryBuffer = QString("artist:");
        queryBuffer.append(this->artist);
        queryBuffer.append("&release:");
        queryBuffer.append(this->release);
        queryBuffer.append("&title:");
        queryBuffer.append(this->text());
        break;
    }

    /* now insert the buffers into the pmap */
    pmap.insert(std::pair<std::string,std::string>(std::string("query"), queryBuffer.toStdString()));
    pmap.insert(std::pair<std::string, std::string>(std::string("limit"), std::string("25")));

    return pmap;
}

void BrainzLineEdit::loadArtistList(const MusicBrainz5::CArtistList *alist) {
    MusicBrainz5::CArtist *art;

    int max = alist->NumItems()-1;
    for (int i = 0; i <= max; ++i) {
        art = alist->Item(i);
        this->suggestionList.append(QString::fromStdString(art->Name()));
    }
}

void BrainzLineEdit::loadReleaseList(const MusicBrainz5::CReleaseList *rlist) {
    MusicBrainz5::CRelease* rel;

    int max = rlist->NumItems()-1;
    for (int i = 0; i<=max; ++i) {
        rel = rlist->Item(i);
        this->suggestionList.append(QString::fromStdString(rel->Title()));
    }
}
