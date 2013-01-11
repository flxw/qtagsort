#ifndef PATTERNVALIDATOR_H
#define PATTERNVALIDATOR_H

#include <QValidator>

class PatternValidator : public QValidator
{
    Q_OBJECT

public:
    explicit PatternValidator(QObject *parent = 0);

    QValidator::State validate(QString &pattern, int &pos) const;
    bool isValid(QString &pat);
};

#endif // PATTERNVALIDATOR_H
