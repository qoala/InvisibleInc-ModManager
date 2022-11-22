#ifndef IIMODMANAGER_CONFIRMATIONPROMPT_H
#define IIMODMANAGER_CONFIRMATIONPROMPT_H

#include <QObject>

namespace iimodmanager {

class ConfirmationPrompt : public QObject
{
    Q_OBJECT
public:
    explicit ConfirmationPrompt(QObject *parent = nullptr);

    void start(const QString &prompt = QStringLiteral("Do you want to continue?"), bool defaultYes = true);

signals:
    void yes();
    void no();
};

} // namespace iimodmanager

#endif // IIMODMANAGER_CONFIRMATIONPROMPT_H
