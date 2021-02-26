#ifndef AUTHWRAPPER_H
#define AUTHWRAPPER_H

#include <QObject>
#include <QOAuth2AuthorizationCodeFlow>
#include <QOAuthHttpServerReplyHandler>
#include <QtNetwork>
#include <QWebEngineView>

class CustomOAuthReplyHandler : public QOAuthHttpServerReplyHandler
{
    Q_OBJECT
public:
    CustomOAuthReplyHandler(QObject *parent = nullptr)
        : QOAuthHttpServerReplyHandler(parent) {}

    QString callback() const {
        return QString("vlva://callback");
    }
};

class AuthWrapper : public QObject
{
    Q_OBJECT
public:
    explicit AuthWrapper(QObject *parent = nullptr);
    void grant();
    bool isAuthenticated() const;
    QString idToken() const;
    QString accessToken() const;
    QString refreshToken() const;

private slots:
    void open_webview(const QUrl & url);
    void exchangeTokenFromCode(const QString & code);
    void on_authenticated();

signals:
    void authenticated();

private:
    QWebEngineView *view;
    QNetworkAccessManager *mgr;
    QOAuth2AuthorizationCodeFlow oauth2;
    enum Token {ID, ACCESS, REFRESH};
    QString tokens[3];
    bool _authenticated = false;

    const QUrl authUrl = QUrl("https://sso.neanias.eu/auth/realms/neanias-development/protocol/openid-connect/auth");
    const QUrl tokenUrl = QUrl("https://sso.neanias.eu/auth/realms/neanias-development/protocol/openid-connect/token");
    const QString clientId = QString("vlkb");
    const QString clientSecret = QString("e1b4cda4-0ae5-46ba-86fd-1b05f58432e3");
    const QString scope = QString("openid profile email phone address");
};

#endif // AUTHWRAPPER_H
