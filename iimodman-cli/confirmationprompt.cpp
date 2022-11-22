#include "confirmationprompt.h"

#include <QTextStream>
#include <QThreadPool>

namespace iimodmanager {

ConfirmationPrompt::ConfirmationPrompt(QObject *parent)
    : QObject(parent)
{}

void ConfirmationPrompt::start(const QString &prompt, bool defaultYes)
{
    // Run on another thread, as std::getline blocks.
    QThreadPool::globalInstance()->start([this, prompt, defaultYes] {
        QTextStream cerr(stderr);
        cerr << prompt << (defaultYes ? " [Y/n] " : " [y/N] ") << Qt::flush;

        QTextStream cin(stdin);
        QString response = cin.readLine().toLower();
        if ((response.isEmpty() && defaultYes) || response == "y" || response == "yes")
            emit yes();
        else
            emit no();
    });
}

} // namespace iimodmanager
