#ifndef AuthWrapper_h
#define AuthWrapper_h

#include <QObject>
#include <QPointer>

class OIDCAuthorizationCodeFlow;
class QNetworkRequest;

enum class AuthService { VLKB };

class AuthWrapper : public QObject
{
    Q_OBJECT

public:
    explicit AuthWrapper(QObject *parent = nullptr);

    bool hasTokens(AuthService service) const;
    void putAuthorizationHeader(AuthService service, QNetworkRequest &req);

public slots:
    void grant(AuthService service);
    void logout(AuthService service);

signals:
    void granted(AuthService service);
    void loggedOut(AuthService service);

private:
    QPointer<OIDCAuthorizationCodeFlow> vlkb;

    OIDCAuthorizationCodeFlow *authObject(AuthService service) const;
};

#endif
