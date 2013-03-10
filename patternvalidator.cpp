#include "patternvalidator.h"

PatternValidator::PatternValidator(QObject *parent) :
    QValidator(parent)
{
}


QValidator::State PatternValidator::validate(QString &pattern, int &pos) const {
    if (pos == 0 || pattern.at(0) == '/')
        return QValidator::Invalid;

    QValidator::State retState = QValidator::Acceptable;

    /* check for double slashes */
    if (pattern.length() > 1 && (pattern.at(pos-1) == '/' && pattern.at(pos-2) == '/')) {
        return QValidator::Invalid;
    }

    /* this is stupid check!  */
    int dotpos = pattern.lastIndexOf(".");
    if (dotpos && pattern.endsWith(".ext")) {
        retState = QValidator::Acceptable;
    } else if (dotpos > 0 && (dotpos - pattern.length() - 1) == 3){
        retState = QValidator::Intermediate;
    } else if (dotpos > 0){
        retState = QValidator::Intermediate;
    } else {
        retState = QValidator::Acceptable;
    }

    /* now check the rest of the pattern for validity */
    for (int i=0; retState == QValidator::Acceptable && i < pattern.length(); ++i) {
        /* check for the pattern initiator */
        if (pattern.at(i) == '%') {
            /* the letter following the initiator is
             * the placeholder; but be aware of trailing %'s */
            if (++i >= pattern.length()) {
                retState = QValidator::Intermediate;
            } else {
                switch (pattern.at(i).toLatin1()) {
                case 'a': retState = QValidator::Acceptable; break;
                case 'r': retState = QValidator::Acceptable; break;
                case 't': retState = QValidator::Acceptable; break;
                case 'n': retState = QValidator::Acceptable; break;
                case 'y': retState = QValidator::Acceptable; break;

                default:  retState = QValidator::Invalid; break;
                }
            }
        }
    }

    return retState;
}

bool PatternValidator::isValid(QString &pat) {
    int pos = 2;
    return validate(pat,pos) == QValidator::Acceptable;
}
